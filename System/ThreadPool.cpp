#define MAX_YIELD_COUNT 8
#ifdef _WIN32
#define MAX_WAIT_MILLISECONDS 200
#else
// reduce to 1s due to low performance on gettimeofday
#define MAX_WAIT_MILLISECONDS 1000
#endif
#include "ThreadPool.h"
#include "../Template/TProxy.h"
#include <cassert>
#include <ctime>
#include <cstdlib>

using namespace PaintsNow;

ThreadPool::ThreadPool(IThread& t, uint32_t tc) : ISyncObject(t), threadCount(tc) {
	eventPump = threadApi.NewEvent();
	Initialize();
}

bool ThreadPool::IsInitialized() const {
	return !threadInfos.empty();
}

void ThreadPool::Initialize() {
	// Initialize thread pool states.
	queuedTaskCount.store(0, std::memory_order_relaxed);
	readCritical.store(0, std::memory_order_relaxed);
	writeCritical.store(0, std::memory_order_relaxed);
	liveThreadCount.store(0, std::memory_order_relaxed);
	activeThreadCount.store(0, std::memory_order_relaxed);
	runningToken.store(1, std::memory_order_release);
	waitEventCounter = 0;

	assert(threadInfos.empty());
	threadInfos.resize(threadCount);

	// create thread workers
	for (size_t i = 0; i < threadInfos.size(); i++) {
		ThreadInfo& info = threadInfos[i];
		info.threadHandle = threadApi.NewThread(Wrap(this, &ThreadPool::Run), i, true);
		info.stockNode = new ThreadTaskQueue::Node();
		info.context = nullptr;
	}
}

uint32_t ThreadPool::GetThreadCount() const {
	return safe_cast<uint32_t>(threadInfos.size());
}

thread_local uint32_t localThreadIndex = ~(uint32_t)0;

uint32_t ThreadPool::GetCurrentThreadIndex() const {
	return localThreadIndex;
}

IThread& ThreadPool::GetThreadApi() {
	return threadApi;
}

void ThreadPool::SetThreadContext(uint32_t id, void* context) {
	assert(id < threadInfos.size());
	threadInfos[id].context = context;
}

void* ThreadPool::GetThreadContext(uint32_t id) const {
	assert(id < threadInfos.size());
	return threadInfos[id].context;
}

void ThreadPool::Uninitialize() {
	// force kill all pending tasks
	size_t threadCount = threadInfos.size();
	assert(threadCount != 0);

	DoLock();
	runningToken.store(0, std::memory_order_release);

	if (threadCount != 0) {
		uint32_t yieldCount = 0;
		while (liveThreadCount.load(std::memory_order_acquire) != 0) {
			if (yieldCount++ < MAX_YIELD_COUNT) {
				YieldThread();
			} else {
				++waitEventCounter;
				threadApi.Wait(eventPump, mutex, MAX_WAIT_MILLISECONDS);
				--waitEventCounter;
				yieldCount = 0;
			}
		}

		// abort remaining tasks
		ThreadTaskQueue::Node* deleted = nullptr;
		while (!taskQueue.Empty()) {
			ITask* task = taskQueue.Top();
			deleted = taskQueue.QuickPop();
			queuedTaskCount.fetch_sub(1, std::memory_order_release);
			task->Abort(nullptr);

			if (deleted != nullptr) {
				delete deleted;
			}
		}

		for (size_t k = 0; k < threadCount; k++) {
			ThreadInfo& info = threadInfos[k];
			threadApi.DeleteThread(info.threadHandle);
			delete info.stockNode;
		}

		threadInfos.clear();
	}

	UnLock();
}

ThreadPool::~ThreadPool() {
	if (!threadInfos.empty()) {
		Uninitialize();
	}

	threadApi.DeleteEvent(eventPump);
}

bool ThreadPool::Push(ITask* task) {
	assert(task != nullptr);
	if (runningToken.load(std::memory_order_acquire) != 0) {
		uint32_t index = GetCurrentThreadIndex();
		if (index != (uint32_t)~0) {
			// Avoid memory allocation inside SpinLock
			ThreadInfo& threadInfo = threadInfos[index];
			SpinLock(writeCritical);
			threadInfo.stockNode = taskQueue.QuickPush(task, threadInfo.stockNode);
			SpinUnLock(writeCritical);
			queuedTaskCount.fetch_add(1, std::memory_order_release);

			if (threadInfo.stockNode == nullptr) {
				threadInfo.stockNode = new ThreadTaskQueue::Node();
			}
		} else {
			SpinLock(writeCritical);
			taskQueue.Push(task);
			SpinUnLock(writeCritical);
			queuedTaskCount.fetch_add(1, std::memory_order_release);
		}

		if (waitEventCounter > threadCount / 4) {
			threadApi.Signal(eventPump, false);
		}

		return true;
	} else {
		task->Abort(nullptr);
		return false;
	}
}

bool ThreadPool::PollRoutine(uint32_t index, bool realtime) {
	for (uint32_t k = 0; k < MAX_YIELD_COUNT; k++) {
		if (queuedTaskCount.load(std::memory_order_acquire) == 0) {
			YieldThread();
		} else {
			break;
		}
	}

	ITask* p = nullptr;
	ThreadTaskQueue::Node* deleted = nullptr;
	SpinLock(readCritical);
	if (queuedTaskCount.load(std::memory_order_acquire) != 0) {
		p = taskQueue.Top();
		deleted = taskQueue.QuickPop();
		queuedTaskCount.fetch_sub(1, std::memory_order_release);
	}
	SpinUnLock(readCritical);

	if (deleted != nullptr) {
		delete deleted;
	}

	if (p != nullptr) {
		if (realtime && (p->flag & ITask::TASK_PRIORITY_BACKGROUND) && activeThreadCount.load(std::memory_order_relaxed) > (int32_t)threadCount - 1) {
			// Not suitable for running here, repost
			Push(p);
		} else {
			if (!realtime) {
				activeThreadCount.fetch_add(1, std::memory_order_acquire);
			}

			void* context = threadInfos[index].context;
			// Exited?
			if (runningToken.load(std::memory_order_relaxed) == 0) {
				p->Abort(context);
			} else {
				p->Execute(context);
			}

			if (!realtime) {
				activeThreadCount.fetch_sub(1, std::memory_order_release);
			}
		}

		return true;
	} else {
		return false;
	}
}

bool ThreadPool::Run(IThread::Thread* thread, size_t index) {
	// set thread local
	localThreadIndex = safe_cast<uint32_t>(index);
	// fetch one and execute
	liveThreadCount.fetch_add(1, std::memory_order_acquire);
	while (runningToken.load(std::memory_order_acquire) != 0) {
		if (!PollRoutine(safe_cast<uint32_t>(index), false) && runningToken.load(std::memory_order_acquire) != 0) {
			threadApi.DoLock(mutex);
			++waitEventCounter;
			threadApi.Wait(eventPump, mutex, MAX_WAIT_MILLISECONDS);
			--waitEventCounter;
			threadApi.UnLock(mutex);
		}
	}

	liveThreadCount.fetch_sub(1, std::memory_order_release);
	return false; // manages IThread::Thread* by ourself
}
