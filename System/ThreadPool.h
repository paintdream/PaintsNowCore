// ThreadPool.h
// By PaintDream (paintdream@paintdream.com)
// 2014-12-14
//

#pragma once
#include "../Interface/IThread.h"
#include "../Interface/ITask.h"
#include "../Template/TProxy.h"
#include "../Template/TAtomic.h"
#include <vector>

namespace PaintsNow {
	// ThreadPool with fixed worker count
	class ThreadPool : public ISyncObject {
	public:
		ThreadPool(IThread& threadApi, uint32_t threadCount);

	public:
		~ThreadPool() override;

		// Push a task. Notice that a task will be ignored if it has been pushed before and not be executed yet.
		bool Push(ITask* task);
		uint32_t GetThreadCount() const;
		uint32_t GetCurrentThreadIndex() const;

		// Custom thread context
		void SetThreadContext(uint32_t index, void* context);
		void* GetThreadContext(uint32_t index) const;

		// Pool specified thread task on current thread. (can be run with non-pooled thread)
		bool PollRoutine(uint32_t index);

		IThread& GetThreadApi();
		void Initialize();
		void Uninitialize();
		bool IsInitialized() const;

	protected:
		void Init();
		bool Run(IThread::Thread* thread, size_t threadID);

	protected:
		std::atomic<ITask*> taskHead;
		std::atomic<int32_t> liveThreadCount;
		std::atomic<int32_t> runningToken;
		uint32_t waitEventCounter;
		uint32_t threadCount;
		IThread::Event* eventPump;
#if defined(_MSC_VER) && _MSC_VER <= 1200
		std::atomic<int32_t> critical;
#endif

		struct ThreadInfo {
			IThread::Thread* threadHandle;
			void* context;
		};

		std::vector<ThreadInfo> threadInfos;
	};
}
