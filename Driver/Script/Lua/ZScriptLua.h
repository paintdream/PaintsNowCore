#pragma once
#include "../../../Interface/IScript.h"

struct lua_State;

namespace PaintsNow {
	// A general interface of lua
	class ZScriptLua final : public IScript {
	public:
		ZScriptLua(IThread& threadApi, lua_State* L = nullptr);
		~ZScriptLua() override;

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
			~Request() override;
			TObject<IReflect>& operator () (IReflect& reflect) override;
			IScript* GetScript() override;
			IScript::RequestPool* GetRequestPool() override;
			void SetRequestPool(IScript::RequestPool* requestPool) override;
			bool Call(const AutoWrapperBase& defer, const Request::Ref& g) override;
			std::vector<Key> Enumerate() override;
			IScript::Request::TYPE GetCurrentType() override;
			Request::Ref Load(const String& script, const String& pathname) override;
			IScript::Request& Push() override;
			IScript::Request& Pop() override;
			IScript::Request& operator >> (IScript::Request::Arguments&) override;
			IScript::Request& operator >> (IScript::Request::Ref&) override;
			IScript::Request& operator << (const IScript::Request::Ref&) override;
			IScript::Request& operator << (const IScript::Request::Nil&) override;
			IScript::Request& operator << (const IScript::BaseDelegate&) override;
			IScript::Request& operator >> (IScript::BaseDelegate&) override;
			IScript::Request& operator << (const IScript::Request::Global&) override;
			IScript::Request& operator << (const IScript::Request::TableStart&) override;
			IScript::Request& operator >> (IScript::Request::TableStart&) override;
			IScript::Request& operator << (const IScript::Request::TableEnd&) override;
			IScript::Request& operator >> (const IScript::Request::TableEnd&) override;
			IScript::Request& operator << (const IScript::Request::ArrayStart&) override;
			IScript::Request& operator >> (IScript::Request::ArrayStart&) override;
			IScript::Request& operator << (const IScript::Request::ArrayEnd&) override;
			IScript::Request& operator >> (const IScript::Request::ArrayEnd&) override;
			IScript::Request& operator << (const IScript::Request::Key&) override;
			IScript::Request& operator >> (const IScript::Request::Key&) override;
			IScript::Request& operator << (double value) override;
			IScript::Request& operator >> (double& value) override;
			IScript::Request& operator << (const String& str) override;
			IScript::Request& operator >> (String& str) override;
			IScript::Request& operator << (Unique unique) override;
			IScript::Request& operator >> (Unique& unique) override;
			IScript::Request& operator << (const char* str) override;
			IScript::Request& operator >> (const char*& str) override;
			IScript::Request& operator << (int64_t value) override;
			IScript::Request& operator >> (int64_t& value) override;
			IScript::Request& operator << (bool b) override;
			IScript::Request& operator >> (bool& b) override;
			IScript::Request& operator << (const AutoWrapperBase& wrapper) override;
			IScript::Request& MoveVariables(IScript::Request& target, size_t count) override;
			void Dereference(IScript::Request::Ref& ref) override;
			IScript::Request::Ref Reference(const IScript::Request::Ref& d) override;
			IScript::Request::TYPE GetReferenceType(const IScript::Request::Ref& d) override;
			int GetCount() override;

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
		void Reset() override;
		IScript::Request& GetDefaultRequest() override;
		IScript* NewScript() const override;
		IScript::Request* NewRequest(const String& entry) override;
		const char* GetFileExt() const override;

		bool IsClosing() const override;
		bool IsHosting() const override;

	private:
		void Init();
		void Clear();

		lua_State* state;
		lua_State* rawState;
		ZScriptLua::Request* defaultRequest;
		IThread::Event* runningEvent;

		std::atomic<uint32_t> closing;
		uint32_t totalReference;
		uint32_t callCounter;

	private:
#ifdef _DEBUG
		std::map<size_t, std::pair<uint32_t, String> > debugReferences;
#endif
	};
}

