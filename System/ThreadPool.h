// ThreadPool.h
// By PaintDream (paintdream@paintdream.com)
// 2014-12-14
//

#pragma once
#include "../Interface/IThread.h"
#include "../Interface/ITask.h"
#include "../Template/TProxy.h"
#include "../Template/TAtomic.h"
#include "../Template/TQueue.h"
#include <vector>

namespace PaintsNow {
	// ThreadPool with fixed worker count
	class ThreadPool : public ISyncObject {
	public:
		ThreadPool(IThread& threadApi, uint32_t threadCount);

	public:
		virtual ~ThreadPool();
		bool Push(ITask* task);
		uint32_t GetThreadCount() const;
		uint32_t GetCurrentThreadIndex() const;

		// Custom thread context
		void SetThreadContext(uint32_t index, void* context);
		void* GetThreadContext(uint32_t index) const;
		bool PollRoutine(uint32_t index);

		IThread& GetThreadApi();
		void Initialize();
		void Uninitialize();
		bool IsInitialized() const;

	protected:
		void Init();
		bool Run(IThread::Thread* thread, size_t threadID);

	protected:
		typedef TQueueList<ITask*> ThreadTaskQueue;
		ThreadTaskQueue taskQueue;
		std::atomic<int32_t> queuedTaskCount;
		std::atomic<int32_t> readCritical;
		std::atomic<int32_t> writeCritical;

		std::atomic<int32_t> liveThreadCount;
		std::atomic<int32_t> runningToken;
		std::atomic<int32_t> activeThreadCount;
		uint32_t waitEventCounter;
		uint32_t threadCount;
		IThread::Event* eventPump;

		struct ThreadInfo {
			IThread::Thread* threadHandle;
			void* context;
			ThreadTaskQueue::Node* stockNode;
		};

		std::vector<ThreadInfo> threadInfos;
	};
}
