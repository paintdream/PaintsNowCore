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
		bool Poll(uint32_t index);
		bool PollDelay(uint32_t index, uint32_t delayMilliseconds);
		bool PollWait(std::atomic<uint32_t>& variable, uint32_t mask = ~(uint32_t)0, uint32_t flag = 0, uint32_t delay = 20);
		uint32_t PollExchange(std::atomic<uint32_t>& variable, uint32_t value, uint32_t delay = 20);
		bool PollCompareExchange(std::atomic<uint32_t>& variable, uint32_t mask, uint32_t flag, uint32_t delay = 20);
		bool IsRunning() const;

		IThread& GetThreadApi();
		void Initialize();
		void Uninitialize();
		bool IsInitialized() const;

	protected:
		bool Run(IThread::Thread* thread, size_t threadID);

	protected:
		alignas(64) std::atomic<size_t> critical;
		alignas(64) std::atomic<ITask*> taskHead;
		alignas(64) std::atomic<size_t> liveThreadCount;
		alignas(64) std::atomic<size_t> runningToken;
		alignas(64) std::atomic<size_t> temperature;
		alignas(64) size_t waitEventCounter;

		uint32_t threadCount;
		IThread::Event* eventPump;

		struct ThreadInfo {
			IThread::Thread* threadHandle;
			void* context;
		};

		std::vector<ThreadInfo> threadInfos;
	};
}
