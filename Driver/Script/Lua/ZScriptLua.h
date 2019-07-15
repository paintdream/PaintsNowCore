#ifndef __ZSCRIPTLUA_H__
#define __ZSCRIPTLUA_H__

#include "../../../../Core/Interface/IScript.h"

struct lua_State;

namespace PaintsNow {
	// A general interface of lua
	class ZScriptLua final : public IScript {
	public:
		ZScriptLua(IThread& threadApi);
		virtual ~ZScriptLua();

		bool BeforeCall();
		void AfterCall();

		struct IndexState {
			IndexState(lua_State* L);
			IndexState() : idx(1), initCount(0), tableLevel(0) {}
			int idx;
			int initCount;
			int tableLevel;
		};

		class Request final : public TReflected<Request, IScript::Request>, public IndexState {
		public:
			Request(ZScriptLua* s, lua_State* L);
			static IScript::Request::TYPE ConvertType(int type);
			virtual ~Request() override;
			virtual TObject<IReflect>& operator () (IReflect& reflect) override;
			virtual IScript* GetScript() override;
			virtual bool Call(const AutoWrapperBase& defer, const Request::Ref& g) override;
			virtual std::vector<Key> Enumerate() override;
			virtual IScript::Request::TYPE GetCurrentType() override;
			virtual Request::Ref Load(const String& script, const String& pathname) override;
			virtual IScript::Request& Push() override;
			virtual IScript::Request& Pop() override;
			virtual IScript::Request& operator >> (IScript::Request::Ref&) override;
			virtual IScript::Request& operator << (const IScript::Request::Ref&) override;
			virtual IScript::Request& operator << (const IScript::Request::Nil&) override;
			virtual IScript::Request& operator << (const IScript::BaseDelegate&) override;
			virtual IScript::Request& operator >> (IScript::BaseDelegate&) override;
			virtual IScript::Request& operator << (const IScript::Request::Global&) override;
			virtual IScript::Request& operator << (const IScript::Request::Local&) override;
			virtual IScript::Request& operator << (const IScript::Request::TableStart&) override;
			virtual IScript::Request& operator >> (IScript::Request::TableStart&) override;
			virtual IScript::Request& operator << (const IScript::Request::TableEnd&) override;
			virtual IScript::Request& operator >> (const IScript::Request::TableEnd&) override;
			virtual IScript::Request& operator << (const IScript::Request::ArrayStart&) override;
			virtual IScript::Request& operator >> (IScript::Request::ArrayStart&) override;
			virtual IScript::Request& operator << (const IScript::Request::ArrayEnd&) override;
			virtual IScript::Request& operator >> (const IScript::Request::ArrayEnd&) override;
			virtual IScript::Request& operator << (const IScript::Request::Key&) override;
			virtual IScript::Request& operator >> (const IScript::Request::Key&) override;
			virtual IScript::Request& operator << (double value) override;
			virtual IScript::Request& operator >> (double& value) override;
			virtual IScript::Request& operator << (const String& str) override;
			virtual IScript::Request& operator >> (String& str) override;
			virtual IScript::Request& operator << (const char* str) override;
			virtual IScript::Request& operator >> (const char*& str) override;
			virtual IScript::Request& operator << (int64_t value) override;
			virtual IScript::Request& operator >> (int64_t& value) override;
			virtual IScript::Request& operator << (bool b) override;
			virtual IScript::Request& operator >> (bool& b) override;
			virtual IScript::Request& operator << (const AutoWrapperBase& wrapper) override;
			virtual IScript::Request& operator >> (const Skip& skip) override;
			virtual IScript::Request& MoveVariables(IScript::Request& target, size_t count) override;
			virtual void Dereference(IScript::Request::Ref& ref) override;
			virtual IScript::Request::Ref Reference(const IScript::Request::Ref& d) override;
			virtual IScript::Request::TYPE GetReferenceType(const IScript::Request::Ref& d) override;
			virtual int GetCount() override;

			void SetIndex(int i);
			IScript::Request& CleanupIndex();
			lua_State* GetRequestState();
			void AddReference(IScript::Request::Ref ref);
			void DelReference(IScript::Request::Ref ref);
			IndexState ExchangeState(const IndexState& w);

		private:
			ZScriptLua* state;
			IScript::Request::Ref ref;
			String key;
			lua_State* L;
		};

		friend class Request;

		lua_State* GetState() const;
		lua_State* GetDeferState() const;
		virtual void DoLock();
		virtual void UnLock();
		virtual bool TryLock();
		virtual void Reset();
		virtual IScript::Request& GetDefaultRequest();
		virtual IScript* NewScript() const;
		virtual IScript::Request* NewRequest(const String& entry);
		virtual const char* GetFileExt() const;
		virtual void SetDebugHandler(const TWrapper<void, IScript::Request&, int, int>& handler, int mask);
		virtual const char* QueryUniformResource(const String& path, size_t& length);

		int GetInitDeferCount() const;
		void SetInitDeferCount(int count);
		bool IsClosing() const;

	private:
		void Init();
		void Clear();

		lua_State* state;
		lua_State* deferState;
		TAtomic<uint32_t> closing;
		ZScriptLua::Request* defaultRequest;
		size_t totalReference;
		int initCountDefer;
		size_t callCounter;
		IThread::Event* runningEvent;
		static std::map<String, std::pair<const char*, size_t> > builtinModules;
	private:
#ifdef _DEBUG
		std::map<size_t, String> debugReferences;
#endif
	};
}

#endif // __ZSCRIPTLUA_H__
