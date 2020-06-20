#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "ZScriptLua.h"
#include <cassert>
#include <cstring>

extern "C" {
	#include "Core/lua.h"
	#include "Core/lualib.h"
	#include "Core/lauxlib.h"
	#include "Core/lobject.h"
}

#ifdef _MSC_VER
#pragma warning(disable:4800)
#endif

using namespace PaintsNow;

// static int g_global = 0;
// static int g_scriptKey = 1;
static int g_bindKey = 2;
static int g_dummyKey = 3;
static int g_refKey = 4;
static int g_objKey = 5;
static int g_wrapKey = 6;
static int g_localKey = 7;

std::map<String, std::pair<const char*, size_t> > ZScriptLua::builtinModules;

static thread_local IScript::RequestPool* _CurrentRequestPool = nullptr;

static int lua_typex(lua_State* L, int index) {
	int type = lua_type(L, index); // maybe faster ? ttypetag()
	if (type == LUA_TNUMBER) {
		type = lua_isinteger(L, index) ? LUA_VNUMINT : LUA_VNUMFLT;
	}

	return type;
}

inline ZScriptLua* GetScript(lua_State* L) {
	return *reinterpret_cast<ZScriptLua**>(lua_getextraspace(L));
	/*
	lua_rawgetp(L, LUA_REGISTRYINDEX, (void*)&g_scriptKey);
	ZScriptLua* pRet = reinterpret_cast<ZScriptLua*>(lua_touserdata(L, -1));
	lua_pop(L, 1);*/
}

static void DebugHandler(lua_State* L, lua_Debug* ar) {
	ZScriptLua* pRet = GetScript(L);
	ZScriptLua::Request s(pRet, L);
	s.SetRequestPool(_CurrentRequestPool);

	TWrapper<void, IScript::Request&, int, int> handler = pRet->GetDebugHandler();
	if (handler) {
		int m = 0;
		switch (ar->event) {
			case LUA_HOOKCALL:
				m = IScript::DEBUG_CALL;
				break;
			case LUA_HOOKTAILCALL:
				m = IScript::DEBUG_TAILCALL;
				break;
			case LUA_HOOKRET:
				m = IScript::DEBUG_RETURN;
				break;
			case LUA_HOOKLINE:
				m = IScript::DEBUG_LINE;
				break;
		}

		assert(pRet->GetLockCount() == 1);
		// pRet->UnLock(); // currently not unlock it by now

		if (handler) {
			handler(s, m, ar->currentline);
		}
		assert(pRet->GetLockCount() == 1);

		// pRet->DoLock();
	}
}

static void HandleError(ZScriptLua* script, lua_State* L) {
	// error
	ZScriptLua::Request s(script, L);
	s.SetRequestPool(_CurrentRequestPool);

	const char* errMsg = lua_tostring(L, -1);
	assert(script->GetLockCount() == 1);
	script->UnLock();
	s.Error(errMsg);
	if (strcmp(errMsg, "attempt to yield across a C-call boundary") == 0) {
		s.Error("Please use asynchronized calls in your script handler while calling lua routines that may yield themselves.");
	}
	script->DoLock();
	lua_pop(L, 1);
}

static int ContinueProxy(lua_State* L, int status, lua_KContext ctx);

static int FunctionProxyEx(ZScriptLua* pRet, const IScript::Request::AutoWrapperBase* wrapper, lua_State* L) {
	int valsize = 0;
	ZScriptLua::Request s(pRet, L);
	assert(_CurrentRequestPool != nullptr);
	s.SetRequestPool(_CurrentRequestPool);

	int ptr = lua_gettop(L);
	// popup all locks
	assert(pRet->GetLockCount() == 1);
	// pRet->UnLock(); // currently not locked in this way
	(*wrapper).Execute(s);
	// pRet->DoLock();
	assert(pRet->GetLockCount() == 1);
	valsize = lua_gettop(L) - ptr;
	return valsize;
}

static int ContinueProxy(lua_State* L, int status, lua_KContext ctx) {
	IScript::Request::AutoWrapperBase* wrapper = reinterpret_cast<IScript::Request::AutoWrapperBase*>(ctx);
	ZScriptLua* pRet = GetScript(L);
	if (status != LUA_OK && status != LUA_YIELD) {
		HandleError(pRet, L);
		return 0;
	} else {
		if (wrapper != nullptr) {
			FunctionProxyEx(pRet, wrapper, L);
			delete wrapper;
		}

		int count = pRet->GetInitDeferCount();
		lua_State* state = pRet->GetDeferState();
		int savedValsize = (int)lua_tointeger(state, -1);
		lua_pop(state, 1);

		if (count != 0) {
			int paramsize = lua_gettop(state) - count;

			lua_xmove(state, L, paramsize);
			wrapper = reinterpret_cast<IScript::Request::AutoWrapperBase*>(lua_touserdata(L, -1));
			lua_pop(L, 1);

			pRet->SetInitDeferCount((int)lua_tointeger(state, -1));
			lua_pop(state, 1);
			lua_pushinteger(state, savedValsize); // repush

			return ContinueProxy(L, lua_pcallk(L, paramsize - 2, LUA_MULTRET, 0, (lua_KContext)wrapper, ContinueProxy), (lua_KContext)wrapper);
		} else {
			return savedValsize;
		}
	}
}

static int FunctionProxy(lua_State* L) {
	// const TProxy<void, IScript::Request&>* proxy = reinterpret_cast<TProxy<void, IScript::Request&>* const>(lua_touserdata(L, lua_upvalueindex(1)));
	// IHost* handler = reinterpret_cast<IHost*>(lua_touserdata(L, lua_upvalueindex(2)));
	const IScript::Request::AutoWrapperBase* wrapper = *reinterpret_cast<const IScript::Request::AutoWrapperBase**>(lua_touserdata(L, lua_upvalueindex(1)));

	ZScriptLua* pRet = GetScript(L);
	if (pRet->IsClosing()) return 0;

	int valsize = FunctionProxyEx(pRet, wrapper, L);
	lua_State* defer = pRet->GetDeferState();
	lua_pushinteger(defer, valsize);
	return ContinueProxy(L, LUA_OK, (lua_KContext)nullptr);
}


static int FreeMem(lua_State* L) {
	void* mem = lua_touserdata(L, -1);
	if (mem == nullptr) // we only free user data
		return 0;

	IScript::Object* obj = *(IScript::Object**)mem;
	ZScriptLua::Request s(GetScript(L), L);
	s.SetRequestPool(_CurrentRequestPool);

	IScript* script = s.GetScript();
	assert(script->GetLockCount() == 1);
	script->UnLock();

	// printf("<<<< DELETED OBJ: %p Type: %s\n", obj, obj->GetUnique()->typeName.c_str());
	if (obj != nullptr) {
		obj->ScriptUninitialize(s);
	}

	script->DoLock();
	assert(script->GetLockCount() == 1);
	return 0;
}

static int FreeWrapper(lua_State* L) {
	IScript::Request::AutoWrapperBase** base = reinterpret_cast<IScript::Request::AutoWrapperBase**>(lua_touserdata(L, -1));
	delete *base;
	// printf("Freed!");
	return 0;
}

static int SetIndexer(lua_State* L) {
	// const char* s = lua_tostring(L, 1);
	luaL_checktype(L, 1, LUA_TSTRING);
	lua_newtable(L);
	lua_pushliteral(L, "__index");
	lua_pushvalue(L, -3);
	lua_rawset(L, -3);
	lua_pushliteral(L, "__gc");
	lua_pushcfunction(L, FreeMem);
	lua_rawset(L, -3);
	lua_replace(L, -2);
	lua_rawset(L, LUA_REGISTRYINDEX);

	return 0;
}

ZScriptLua::ZScriptLua(IThread& threadApi) : IScript(threadApi), callCounter(0), runningEvent(nullptr) {
	Init();
}

bool ZScriptLua::IsClosing() const {
	return closing.load(std::memory_order_relaxed) != 0;
}

void ZScriptLua::Clear() {
	// Wait for all active routines finished.
	DoLock();

	if (callCounter != 0) {
		IThread::Event* e = threadApi.NewEvent();
		runningEvent = e;
		threadApi.Wait(runningEvent, mutex);
		assert(runningEvent == nullptr);
		threadApi.DeleteEvent(e);
	}

	closing.store(1, std::memory_order_relaxed);
	lua_close(state);
	delete defaultRequest;
	UnLock();

#ifdef _DEBUG
	if (totalReference != 0) {
		for (std::map<size_t, String>::iterator it = debugReferences.begin(); it != debugReferences.end(); ++it) {
			printf("Leak lua object where: %s\n", (*it).second.c_str());
		}
	}
#endif

	assert(totalReference == 0);
}

void ZScriptLua::Init() {
	DoLock();
	closing.store(0, std::memory_order_relaxed);
	state = luaL_newstate();
	defaultRequest = new ZScriptLua::Request(this, state);
	luaL_openlibs(state);
	callCounter = 0;

	ZScriptLua** s = reinterpret_cast<ZScriptLua**>(lua_getextraspace(state));
	*s = this;

	// make __gc metatable for default values
	lua_newtable(state);
	lua_pushliteral(state, "__gc");
	lua_pushcfunction(state, FreeMem);
	lua_rawset(state, -3);
	lua_rawsetp(state, LUA_REGISTRYINDEX, (void*)&g_bindKey);

	// make a dummy node for empty table accesses
	lua_newtable(state);
	lua_rawsetp(state, LUA_REGISTRYINDEX, (void*)&g_dummyKey);

	// make a reference table for references
	lua_newtable(state);
	lua_rawsetp(state, LUA_REGISTRYINDEX, (void*)&g_refKey);

	// make a table for delegates
	lua_newtable(state);
	lua_pushliteral(state, "__mode");
	lua_pushliteral(state, "v");
	lua_rawset(state, -3);
	lua_pushvalue(state, -1);
	lua_setmetatable(state, -2);
	lua_rawsetp(state, LUA_REGISTRYINDEX, (void*)&g_objKey);

	// make a table for local storage
	lua_newtable(state);
	lua_pushliteral(state, "__mode");
	lua_pushliteral(state, "k");
	lua_rawset(state, -3);
	lua_pushvalue(state, -1);
	lua_setmetatable(state, -2);
	lua_rawsetp(state, LUA_REGISTRYINDEX, (void*)&g_localKey);

	// make a table for wrappers
	lua_newtable(state);
	lua_pushliteral(state, "__gc");
	lua_pushcfunction(state, FreeWrapper);
	lua_rawset(state, -3);
	lua_rawsetp(state, LUA_REGISTRYINDEX, (void*)&g_wrapKey);

	// const char* type = lua_typename(state, lua_typex(state, -1));

	/*
	luaL_loadbuffer(state, (const char*)Core_lua, Core_lua_len, "Core");
	lua_call(state, 0, 1); // get returned core lib
	lua_setglobal(state, "Core");*/

	// enable setmetatable for userdata
	lua_pushcfunction(state, SetIndexer);
	lua_setglobal(state, "setindexer");

	totalReference = 0;
	initCountDefer = 0;

	deferState = lua_newthread(state); // don't pop it from stack unless the state was closed.

	if (debugHandler) {
		// set hook
		SetDebugHandler(debugHandler, debugMask);
	}

	UnLock();
}


bool ZScriptLua::BeforeCall() {
	assert(GetLockCount() == 1);
	callCounter++;

	return true;
}

void ZScriptLua::AfterCall() {
	assert(GetLockCount() == 1);
	if (--callCounter == 0 && runningEvent != nullptr) {
		threadApi.Signal(runningEvent, false);
		runningEvent = nullptr;
	}
}

void ZScriptLua::Reset() {
	Clear();
	IScript::Reset();
	Init();
}

ZScriptLua::~ZScriptLua() {
	Clear();
}

IScript::Request::TYPE ZScriptLua::Request::GetReferenceType(const IScript::Request::Ref& g) {
	*this << g;
	int t = lua_typex(L, -1);
	lua_pop(L, 1);

	return ZScriptLua::Request::ConvertType(t);
}


bool ZScriptLua::Request::Call(const AutoWrapperBase& defer, const IScript::Request::Ref& g) {
	assert(GetScript()->GetLockCount() == 1);
	assert(tableLevel == 0);
	if (!state->BeforeCall())
		return false;

	int pos = lua_gettop(L);
	int in = pos - initCount;
	assert(in >= 0);
	// lua_State* s = state->GetState();
	if (defer.IsSync()) {
		*this << g;
		// printf("Type : %s\n", lua_typename(L, lua_typex(L, -1)));
		if (!lua_isfunction(L, -1)) {
			state->AfterCall();
			return false;
		}

		lua_insert(L, initCount + 1);
		assert(lua_isfunction(L, -in - 1));
		// lua_KContext context = (lua_KContext)defer.Clone();
		// dispatch deferred calls after the next sync call.
		// int status = lua_pcallk(L, in, LUA_MULTRET, 0, nullptr, ContinueProxy);
		RequestPool* p = _CurrentRequestPool;
		_CurrentRequestPool = GetRequestPool();
		assert(_CurrentRequestPool != nullptr);
		int status = lua_pcall(L, in, LUA_MULTRET, 0);
		_CurrentRequestPool = p;

		if (status != LUA_OK && status != LUA_YIELD) {
			HandleError(static_cast<ZScriptLua*>(GetScript()), L);
			state->AfterCall();
			return false;
		} else {
			assert(lua_gettop(L) >= initCount);
			// important!
			SetIndex(initCount + 1);
			state->AfterCall();
			return true;
		}
	} else {
		// Invoke deferred call
		lua_State* host = state->GetDeferState();
		lua_pushinteger(host, state->GetInitDeferCount());
		state->SetInitDeferCount(lua_gettop(host));
		*this << g;
		lua_xmove(L, host, 1);
		if (in != 0) {
			lua_xmove(L, host, in);
		}

		lua_pushlightuserdata(host, (void*)defer.Clone()); // as lightuserdata
		state->AfterCall();
		return true;
	}
}

const char* ZScriptLua::GetFileExt() const {
	static const String ext = "lua";
	return ext.c_str();
}

void ZScriptLua::SetDebugHandler(const TWrapper<void, IScript::Request&, int, int>& handler, int mask) {
	assert(GetLockCount() != 0);
	if (handler) {
		int m = 0;
		if ((mask & IScript::DEBUG_CALL) == IScript::DEBUG_CALL) m |= LUA_MASKCALL;
		if ((mask & IScript::DEBUG_RETURN) == IScript::DEBUG_RETURN) m |= LUA_MASKRET;
		if ((mask & IScript::DEBUG_LINE) == IScript::DEBUG_LINE) m |= LUA_MASKLINE;

		lua_sethook(state, DebugHandler, m, 1);
	} else {
		lua_sethook(state, nullptr, 0, 0);
	}

	IScript::SetDebugHandler(handler, mask);
}

const char* ZScriptLua::QueryUniformResource(const String& path, size_t& length) {
	std::map<String, std::pair<const char*, size_t> >::const_iterator it = builtinModules.find(path);
	if (it != builtinModules.end()) {
		length = it->second.second;
		return it->second.first;
	} else {
		return nullptr;
	}
}

IScript::Request* ZScriptLua::NewRequest(const String& entry) {
	assert(GetLockCount() == 1);
	return new ZScriptLua::Request(this, nullptr);
}

IScript::Request& ZScriptLua::Request::MoveVariables(IScript::Request& target, size_t count) {
	assert(GetScript()->GetLockCount() == 1);
	lua_xmove(L, (static_cast<ZScriptLua::Request&>(target)).L, (int)count);
	return *this;
}

IScript* ZScriptLua::Request::GetScript() {
	return state;
}

int ZScriptLua::Request::GetCount() {
	assert(GetScript()->GetLockCount() == 1);
	// return lua_gettop(L);
	return lua_gettop(L) - initCount;
}

IScript::Request& PaintsNow::ZScriptLua::Request::CleanupIndex() {
	idx = initCount + 1;
	return *this;
}

void ZScriptLua::Request::SetIndex(int i) {
	assert(GetScript()->GetLockCount() == 1);
	idx = i;
}

lua_State* ZScriptLua::Request::GetRequestState() {
	return L;
}

static int IncreaseTableIndex(lua_State* L, int count = 1) {
	void* t = (lua_touserdata(L, -1));
	int index = *(int*)&t;
	if (count == 0) {
		index = 0;
	} else {
		index += count;
	}
	lua_pop(L, 1);
	lua_pushlightuserdata(L, (void*)(size_t)index);
	return index;
}

inline void refget(lua_State* L, const IScript::Request::Ref& ref) {
	lua_rawgetp(L, LUA_REGISTRYINDEX, &g_refKey);
	lua_rawgeti(L, -1, ref.value);
	lua_replace(L, -2);
}


template <class F, class C>
inline void Write(lua_State* L, int& tableLevel, int& idx, String& key, F f, const C& value) {
	if (tableLevel != 0) {
		if (key.empty()) {
			int index = IncreaseTableIndex(L);
			f(L, value);
			lua_rawseti(L, -3, index);
		} else {
			lua_pushvalue(L, lua_upvalueindex(0));
			bool isGlobal = lua_compare(L, -1, -3, LUA_OPEQ);
			lua_pop(L, 1);
			if (isGlobal) {
				f(L, value);
				lua_setglobal(L, key.c_str());
			} else {
				lua_pushstring(L, key.c_str());
				f(L, value);
				lua_rawset(L, -4);
			}
			key = "";
		}
	} else {
		f(L, value);
	}
}


template <class F, class C>
inline void Read(lua_State* L, int& tableLevel, int& idx, String& key, F f, C& value) {
	if (tableLevel != 0) {
		assert(lua_isuserdata(L, -1));
		assert(lua_istable(L, -2));
		if (key.empty()) {
			int index = IncreaseTableIndex(L);
			lua_rawgeti(L, -2, index);
		} else {
			lua_pushstring(L, key.c_str());
			key = "";
			assert(lua_istable(L, -3));
			lua_rawget(L, -3);
		}
		if (!lua_isnil(L, -1)) {
			value = f(L, -1);
		}
		lua_pop(L, 1);
	} else {
		value = f(L, idx++);
	}
}

IScript::Request& ZScriptLua::Request::operator >> (Arguments& args) {
	args.count = initCount - idx + 1;
	assert(args.count >= 0);
	return *this;
}

IScript::Request& ZScriptLua::Request::operator >> (const Skip& skip) {
	assert(GetScript()->GetLockCount() == 1);
	if (tableLevel != 0) {
		assert(lua_isuserdata(L, -1));
		assert(lua_istable(L, -2));
		if (key.empty()) {
			IncreaseTableIndex(L, skip.count);
		}
	} else {
		idx += skip.count;
		assert(idx <= lua_gettop(L));
	}

	return *this;
}


IScript::Request& ZScriptLua::Request::operator << (const IScript::Request::Local&) {
	lua_rawgetp(L, LUA_REGISTRYINDEX, &g_localKey);
	lua_pushthread(L);
	assert(lua_istable(L, -2));
	if (lua_rawget(L, -2) == LUA_TNIL) {
		assert(lua_isnil(L, -1));
		lua_pop(L, 1);
		assert(lua_istable(L, -1));
		lua_newtable(L);
		lua_pushthread(L);
		lua_pushvalue(L, -2); // make a copy
		lua_rawset(L, -4);
		lua_replace(L, -2);
	}

	lua_remove(L, -2);
	lua_pushlightuserdata(L, (void*)0);
	tableLevel++;

	return *this;
}


IScript::Request& ZScriptLua::Request::operator << (const IScript::Request::Global&) {
	assert(GetScript()->GetLockCount() == 1);
	//	lua_pushvalue(L, lua_upvalueindex(0));
	lua_pushvalue(L, lua_upvalueindex(0));
	// now the top of stack is object table
	// push index
	lua_pushlightuserdata(L, (void*)0);
	tableLevel++;
	return *this;
}


void ZScriptLua::Request::Dereference(IScript::Request::Ref& ref) {
	assert(GetScript()->GetLockCount() == 1);
	if (ref.value != 0) {
		lua_rawgetp(L, LUA_REGISTRYINDEX, &g_refKey);
		// refget(L, ref);
		// printf("UNREF: %s\n", lua_typename(L, lua_typex(L, -1)));
		// lua_pop(L, 1);

		luaL_unref(L, -1, (int)ref.value);
		lua_pop(L, 1);
		DelReference(ref);
		ref.value = 0;
	}
}

inline IScript::Request::Ref refadd(lua_State* L, int index) {
	if (lua_isnil(L, index) || index > lua_gettop(L)) {
		return IScript::Request::Ref(0);
	}

	lua_rawgetp(L, LUA_REGISTRYINDEX, &g_refKey);
	lua_pushvalue(L, index > 0 ? index : index - 1);
	// printf("Val type: %s\n", lua_typename(L, lua_typex(L, -1)));
	int ref = luaL_ref(L, -2);
	lua_pop(L, 1);
	return IScript::Request::Ref(ref == -1 ? 0 : ref);
}

inline void wrapget(lua_State* L, const IScript::Request::AutoWrapperBase& wrapper) {
	// make a reference in wrap table
	IScript::Request::AutoWrapperBase** ptr = reinterpret_cast<IScript::Request::AutoWrapperBase**>(lua_newuserdata(L, sizeof(IScript::Request::AutoWrapperBase*)));
	*ptr = wrapper.Clone();

	lua_rawgetp(L, LUA_REGISTRYINDEX, &g_wrapKey);
	lua_setmetatable(L, -2);

	lua_pushcclosure(L, FunctionProxy, 1);
}

IScript::Request& ZScriptLua::Request::Push() {
	// save state
	assert(key.empty());
	lua_pushinteger(L, idx);
	lua_pushinteger(L, tableLevel);
	lua_pushinteger(L, initCount);
	initCount = lua_gettop(L);
	idx = initCount + 1;
	tableLevel = 0;
	return *this;
}

IScript::Request& ZScriptLua::Request::Pop() {
	// save state
	assert(key.empty());
	assert(tableLevel == 0);
	assert(lua_gettop(L) >= 3);
	lua_settop(L, initCount);
	initCount = (int)lua_tointeger(L, -1);
	tableLevel = (int)lua_tointeger(L, -2);
	idx = (int)lua_tointeger(L, -3);
	lua_pop(L, 3);
	return *this;
}

void ZScriptLua::Request::AddReference(IScript::Request::Ref ref) {
	if (ref.value == 0)
		return;

	state->totalReference++;
}

void ZScriptLua::Request::DelReference(IScript::Request::Ref ref) {
	if (ref.value == 0)
		return;

	state->totalReference--;
#ifdef _DEBUG
	state->debugReferences.erase(ref.value);
#endif
}

IScript::Request& ZScriptLua::Request::operator << (const AutoWrapperBase& wrapper) {
	assert(GetScript()->GetLockCount() == 1);
	Write(L, tableLevel, idx, key, wrapget, wrapper);
	return *this;
}


IScript::Request& ZScriptLua::Request::operator >> (IScript::Request::Ref& ref) {
	assert(GetScript()->GetLockCount() == 1);

	Read(L, tableLevel, idx, key, refadd, ref);
	AddReference(ref);
	return *this;
}

IScript::Request& ZScriptLua::Request::operator << (const IScript::Request::Ref& ref) {
	assert(GetScript()->GetLockCount() == 1);
	Write(L, tableLevel, idx, key, refget, ref);
	return *this;
}


IScript::Request& ZScriptLua::GetDefaultRequest() {
	return *defaultRequest;
}

IScript::Request& ZScriptLua::Request::operator >> (IScript::Request::TableStart& start) {
	assert(GetScript()->GetLockCount() == 1);
	if (tableLevel == 0) {
		lua_pushvalue(L, idx++); // table
	} else {
		if (key.empty()) {
			int index = IncreaseTableIndex(L);
			lua_rawgeti(L, -2, index);
		} else {
			lua_pushvalue(L, lua_upvalueindex(0));
			bool isGlobal = lua_compare(L, -1, -3, LUA_OPEQ);
			lua_pop(L, 1);

			if (isGlobal) {
				lua_getglobal(L, key.c_str());
			} else {
				lua_pushstring(L, key.c_str());
				lua_rawget(L, -3);
			}

			key = "";
		}
	}

	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		lua_rawgetp(L, LUA_REGISTRYINDEX, &g_dummyKey);
	}

	lua_pushliteral(L, "n");
	if (lua_rawget(L, -2) == LUA_TNUMBER) {
		start.count = lua_tointeger(L, -1);
	} else {
		start.count = (size_t)lua_rawlen(L, -2);
	}
	lua_pop(L, 1);

	lua_pushlightuserdata(L, (void*)0); // index
	tableLevel++;

	assert(lua_istable(L, -2));
	assert(lua_isuserdata(L, -1));

	return *this;
}

IScript::Request& ZScriptLua::Request::operator >> (IScript::Request::ArrayStart& start) {
	TableStart ts;
	*this >> ts;
	start.count = ts.count;
	return *this;
}

IScript::Request& ZScriptLua::Request::operator << (const IScript::Request::TableStart& t) {
	assert(GetScript()->GetLockCount() == 1);
	// add table
	if (tableLevel != 0) {
		if (key.empty()) {
			int index = IncreaseTableIndex(L);
			lua_newtable(L);
			lua_pushvalue(L, -1);
			lua_rawseti(L, -4, index);
		} else {
			lua_pushvalue(L, lua_upvalueindex(0));
			bool isGlobal = lua_compare(L, -1, -3, LUA_OPEQ);
			lua_pop(L, 1);

			lua_newtable(L);
			if (isGlobal) {
				lua_pushvalue(L, -1);
				lua_setglobal(L, key.c_str());
			} else {
				lua_pushstring(L, key.c_str());
				lua_pushvalue(L, -2);
				lua_rawset(L, -5);
			}
			key = "";
		}
	} else {
		lua_newtable(L);
		lua_pushvalue(L, -1);
	}

	// load table
	assert(lua_istable(L, -1));
	lua_pushlightuserdata(L, (void*)0);
	tableLevel++;

	return *this;
}

IScript::Request& ZScriptLua::Request::operator << (const IScript::Request::ArrayStart& t) {
	return *this << TableStart();
}

IScript::Request& ZScriptLua::Request::operator << (const IScript::Request::TableEnd& d) {
	assert(GetScript()->GetLockCount() == 1);
	assert(tableLevel != 0);
	tableLevel--;
	lua_pop(L, 2);
	return *this;
}

IScript::Request& ZScriptLua::Request::operator << (const IScript::Request::ArrayEnd& d) {
	return *this << TableEnd();
}

IScript::Request& ZScriptLua::Request::operator >> (const IScript::Request::TableEnd& d) {
	assert(GetScript()->GetLockCount() == 1);
	assert(tableLevel != 0);
	tableLevel--;
	lua_pop(L, 2);
	return *this;
}

IScript::Request& ZScriptLua::Request::operator >> (const IScript::Request::ArrayEnd& d) {
	return *this >> TableEnd();
}

IScript::Request& ZScriptLua::Request::operator << (double value) {
	assert(GetScript()->GetLockCount() == 1);
	Write(L, tableLevel, idx, key, lua_pushnumber, value);
	return *this;
}

inline double tonumber(lua_State* L, int index) {
	return lua_tonumber(L, index);
}

IScript::Request& ZScriptLua::Request::operator >> (double& value) {
	assert(GetScript()->GetLockCount() == 1);
	Read(L, tableLevel, idx, key, tonumber, value);

	return *this;
}


inline void strwrite(lua_State* L, const String& v) {
	lua_pushlstring(L, v.c_str(), v.size());
}

String strget(lua_State* L, int index) {
	size_t length;
	const char* ptr = lua_tolstring(L, index, &length);
	return String(ptr, length);
}

IScript::Request& ZScriptLua::Request::operator << (const String& v) {
	assert(GetScript()->GetLockCount() == 1);
	Write(L, tableLevel, idx, key, strwrite, v);
	return *this;
}

IScript::Request& ZScriptLua::Request::operator >> (String& v) {
	assert(GetScript()->GetLockCount() == 1);
	Read(L, tableLevel, idx, key, strget, v);

	return *this;
}

// lua 5.3 now supports 64bit integer
IScript::Request& ZScriptLua::Request::operator << (int64_t value) {
	assert(GetScript()->GetLockCount() == 1);
	Write(L, tableLevel, idx, key, lua_pushinteger, value);
	return *this;
}


inline int64_t tointeger(lua_State* L, int index) {
	int isnumber;
	int64_t v = lua_tointegerx(L, index, &isnumber);
	if (isnumber) {
		return v;
	} else {
		return (int64_t)lua_tonumber(L, index);
	}
}

IScript::Request& ZScriptLua::Request::operator >> (int64_t& value) {
	assert(GetScript()->GetLockCount() == 1);
	Read(L, tableLevel, idx, key, tointeger, value);

	return *this;
}

IScript::Request& ZScriptLua::Request::operator << (bool value) {
	assert(GetScript()->GetLockCount() == 1);
	Write(L, tableLevel, idx, key, lua_pushboolean, value);
	return *this;
}

IScript::Request& ZScriptLua::Request::operator >> (bool& value) {
	assert(GetScript()->GetLockCount() == 1);
	Read(L, tableLevel, idx, key, lua_toboolean, value);

	return *this;
}

IScript::Request& ZScriptLua::Request::operator << (const char* value) {
	assert(GetScript()->GetLockCount() == 1);
	Write(L, tableLevel, idx, key, lua_pushstring, value);

	return *this;
}

inline const char* tostrptr(lua_State* L, int i) {
	return lua_tostring(L, i);
}

IScript::Request& ZScriptLua::Request::operator >> (const char*& value) {
	assert(GetScript()->GetLockCount() == 1);
	Read(L, tableLevel, idx, key, tostrptr, value);

	return *this;
}

IScript::Request& ZScriptLua::Request::operator << (const IScript::Request::Key& k) {
	assert(GetScript()->GetLockCount() == 1);
	key = k.GetKey();
	return *this;
}


IScript::Request::TYPE ZScriptLua::Request::ConvertType(int type) {
	IScript::Request::TYPE target = NIL;
	switch (type) {
	case LUA_TNIL:
		target = NIL;
		break;
	case LUA_TBOOLEAN:
		target = BOOLEAN;
		break;
	case LUA_VNUMFLT:
		target = NUMBER;
		break;
	case LUA_VNUMINT:
		target = INTEGER;
		break;
	case LUA_TSTRING:
		target = STRING;
		break;
	case LUA_TTABLE:
		target = TABLE;
		break;
	case LUA_TFUNCTION:
		target = FUNCTION;
		break;
	case LUA_TUSERDATA:
		target = OBJECT;
		break;
	default:
		target = NIL;
		break;
	}

	return target;
}

IScript::Request& ZScriptLua::Request::operator >> (const IScript::Request::Key& y) {
	IScript::Request::Key& k = const_cast<IScript::Request::Key&>(y);
	assert(GetScript()->GetLockCount() == 1);
	int type;
	if (tableLevel != 0) {
		// try to read the value
		lua_pushvalue(L, lua_upvalueindex(0));
		bool isGlobal = lua_compare(L, -1, -3, LUA_OPEQ);
		lua_pop(L, 1);

		if (isGlobal) {
			lua_getglobal(L, k.GetKey());
		} else {
			lua_pushstring(L, k.GetKey());
			lua_rawget(L, -3);
		}

		type = lua_typex(L, -1);
		lua_pop(L, 1);
		key = k.GetKey();
	} else {
		type = lua_typex(L, idx); // read the current one
	}

	k.SetType(ConvertType(type));

	return *this;
}

inline void PushUserdata(lua_State* L, const IScript::BaseDelegate& b) {
	IScript::BaseDelegate& d = const_cast<IScript::BaseDelegate&>(b);
	IScript::Object* ptr = d.GetRaw();
	int t = lua_gettop(L);
	// check if already exists
	lua_rawgetp(L, LUA_REGISTRYINDEX, &g_objKey);
	lua_rawgetp(L, -1, ptr);
	lua_replace(L, -2);

	if (lua_isnil(L, -1)) {
		d = IScript::BaseDelegate(nullptr);
		lua_pop(L, 1);
		void* p = lua_newuserdata(L, sizeof(void*));
		memcpy(p, &ptr, sizeof(void*));
		lua_pushstring(L, ptr->GetUnique()->typeName.c_str());

		if (lua_rawget(L, LUA_REGISTRYINDEX) == LUA_TNIL) {
			lua_pop(L, 1);
			lua_rawgetp(L, LUA_REGISTRYINDEX, &g_bindKey);
		}
		
		lua_setmetatable(L, -2);
		lua_rawgetp(L, LUA_REGISTRYINDEX, &g_objKey);
		lua_pushvalue(L, -2);
		lua_rawsetp(L, -2, ptr);
		lua_pop(L, 1);
	}

	assert(lua_typex(L, -1) == LUA_TUSERDATA);
	int m = lua_gettop(L);
	assert(m == t + 1);
}

inline IScript::Object* ToUserdata(lua_State* L, int idx) {
	IScript::Object** ptr = reinterpret_cast<IScript::Object**>(lua_touserdata(L, idx));
	if (ptr == nullptr) {
		return nullptr;
	} else {
		return *ptr;
	}
}

IScript::Request::Ref ZScriptLua::Request::Reference(const IScript::Request::Ref& d) {
	assert(GetScript()->GetLockCount() == 1);
	int idx = GetCount();
	*this << d;
	IScript::Request::Ref ref = refadd(L, -1);
	lua_pop(L, 1);

	assert(idx == GetCount());
	AddReference(ref);
	return ref;
}

inline void fake_pushnil(lua_State* L, const IScript::Request::Nil&) {
	lua_pushnil(L);
}

IScript::Request& ZScriptLua::Request::operator << (const IScript::BaseDelegate& d) {
	assert(GetScript()->GetLockCount() == 1);
	if (d.GetRaw() != nullptr) {
		// write
		IScript::BaseDelegate& m = const_cast<IScript::BaseDelegate&>(d);
		IScript::Object* ptr = m.GetRaw();
		Write(L, tableLevel, idx, key, PushUserdata, m);

		if (m.GetRaw() == nullptr) {
			UnLock();
			ptr->ScriptInitialize(*this);
			DoLock();
		}
	} else {
		Write(L, tableLevel, idx, key, fake_pushnil, Nil());
	}

	return *this;
}

IScript::Request& ZScriptLua::Request::operator >> (IScript::BaseDelegate& d) {
	assert(GetScript()->GetLockCount() == 1);
	Object* p = nullptr;
	Read(L, tableLevel, idx, key, ToUserdata, p);
	d = IScript::BaseDelegate(p);

	return *this;
}


IScript::Request& ZScriptLua::Request::operator << (const Nil& n) {
	assert(GetScript()->GetLockCount() == 1);
	Write(L, tableLevel, idx, key, fake_pushnil, n);
	return *this;
}

IScript::Request::TYPE ZScriptLua::Request::GetCurrentType() {
	int type = LUA_TNIL;
	if (tableLevel != 0) {
		assert(lua_isuserdata(L, -1));
		assert(lua_istable(L, -2));
		if (key.empty()) {
			void* t = (lua_touserdata(L, -1));
			int index = *(int*)&t + 1;
			lua_rawgeti(L, -2, index);
		} else {
			lua_pushstring(L, key.c_str());
			assert(lua_istable(L, -3));
			lua_rawget(L, -3);
		}
		if (!lua_isnil(L, -1)) {
			type = lua_typex(L, -1);
		}
		lua_pop(L, 1);
	} else {
		type = lua_typex(L, idx);
	}

	return ConvertType(type);
}

std::vector<IScript::Request::Key> ZScriptLua::Request::Enumerate() {
	std::vector<Key> keys;
	assert(GetScript()->GetLockCount() == 1);
	// from lua document
	lua_pushnil(L);  // first key
	while (lua_next(L, -3) != 0) { // it's -3, [table, index, frontKey]
		int type = lua_typex(L, -2);
		if (type == LUA_TSTRING) {
			keys.emplace_back(std::move(Key(lua_tostring(L, -2), ConvertType(lua_typex(L, -1)))));
		}
		lua_pop(L, 1);
	}

	return keys;
}

static int lastlevel(lua_State *L) {
	lua_Debug ar;
	int li = 1, le = 1;
	/* find an upper bound */
	while (lua_getstack(L, le, &ar)) { li = le; le *= 2; }
	/* do a binary search */
	while (li < le) {
		int m = (li + le) / 2;
		if (lua_getstack(L, m, &ar)) li = m + 1;
		else le = m;
	}
	return le - 1;
}

void Assign(String& target, const char* str) {
	if (target != str) {
		target = str;
	}
}

ZScriptLua::Request::Request(ZScriptLua* s, lua_State* l) : state(s), key(""), L(l) {
	assert(GetScript()->GetLockCount() == 1);
	if (L == nullptr) {
		L = lua_newthread(s->GetState());

		// make reference so gc won't collect this request
		ref = refadd(s->GetState(), -1);
		lua_pop(s->GetState(), 1);

		AddReference(ref);
	} else {
		ref.value = 0;
	}

	ExchangeState(L);
}

ZScriptLua::IndexState::IndexState(lua_State* L) {
	idx = 1;
	tableLevel = 0;
	initCount = lua_gettop(L);
}

inline ZScriptLua::IndexState ZScriptLua::Request::ExchangeState(const IndexState& w) {
	IndexState state = *this;
	static_cast<IndexState&>(*this) = w;
	return state;
}

IScript::Request::Ref ZScriptLua::Request::Load(const String& script, const String& path) {
	assert(GetScript()->GetLockCount() == 1);
	// lua_pushlightuserdata(L, &g_refKey);
	// lua_rawget(L, LUA_REGISTRYINDEX);
	// assert(lua_istable(L, -1));
	IScript::Request::Ref ref;
	int error = luaL_loadbuffer(L, script.c_str(), script.length(), path.c_str());

	if (error != 0) {
		Error(lua_tostring(L, -1));
		lua_pop(L, 1);
		return ref;
	}

	assert(lua_isfunction(L, -1));
	ref = refadd(L, -1);
	lua_pop(L, 1);

	AddReference(ref);
	return ref;
}


ZScriptLua::Request::~Request() {
	assert(GetScript()->GetLockCount() == 1);
	if (ref.value != 0) {
		Dereference(ref);
	}
}

lua_State* ZScriptLua::GetState() const {
	return state;
}

TObject<IReflect>& ZScriptLua::Request::operator () (IReflect& reflect) {
	BaseClass::operator () (reflect);

	return *this;
}

inline int ZScriptLua::GetInitDeferCount() const {
	return initCountDefer;
}

void ZScriptLua::SetInitDeferCount(int count) {
	initCountDefer = count;
}


lua_State* ZScriptLua::GetDeferState() const {
	return deferState;
}

bool ZScriptLua::TryLock() {
	assert(GetLockCount() == 1);
	return ISyncObject::TryLock();
}

#if defined(_WIN32) && defined(_DEBUG)
#include <windows.h>
DWORD HoldingThread = 0;
#endif

void ZScriptLua::DoLock() {
	ISyncObject::DoLock();
	assert(GetLockCount() == 1);
}

void ZScriptLua::UnLock() {
	assert(GetLockCount() == 1);
	ISyncObject::UnLock();
}

IScript* ZScriptLua::NewScript() const {
	return new ZScriptLua(threadApi);
}