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
	taskHead.store(nullptr, std::memory_order_relaxed);
	liveThreadCount.store(0, std::memory_order_relaxed);
	runningToken.store(1, std::memory_order_release);
	waitEventCounter = 0;

	assert(threadInfos.empty());
	threadInfos.resize(threadCount);

	// create thread workers
	for (size_t i = 0; i < threadInfos.size(); i++) {
		ThreadInfo& info = threadInfos[i];
		info.threadHandle = threadApi.NewThread(Wrap(this, &ThreadPool::Run), i, true);
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
		// wait for live thread exit
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
		ITask* p = (ITask*)taskHead.exchange(nullptr, std::memory_order_acquire);
		while (p != nullptr) {
			p->Abort(nullptr);
			ITask* q = p;
			p = p->next;
			q->next = (ITask*)(size_t)~0;
		}

		for (size_t k = 0; k < threadCount; k++) {
			ThreadInfo& info = threadInfos[k];
			threadApi.DeleteThread(info.threadHandle);
		}

		threadInfos.clear();
		std::atomic_thread_fence(std::memory_order_release);
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
	std::atomic<ITask*>& next = *reinterpret_cast<std::atomic<ITask*>*>(&task->next);
	if (next.load(std::memory_order_acquire) != (ITask*)(size_t)~0) // already pushed
		return true;

	if (runningToken.load(std::memory_order_relaxed) != 0) {
		ITask* expected = (ITask*)(size_t)~0;
		
		// guard for repush the same task
		if (next.compare_exchange_strong(expected, taskHead.load(std::memory_order_acquire), std::memory_order_acq_rel)) {
			// Chain task
			while (!taskHead.compare_exchange_weak(task->next, task, std::memory_order_release)) {
				YieldThreadFast();
			}

			std::atomic_thread_fence(std::memory_order_acquire);
			if (waitEventCounter != 0) {
				threadApi.Signal(eventPump, false);
			}
		}

		return true;
	} else {
		task->Abort(nullptr);
		return false;
	}
}

bool ThreadPool::PollRoutine(uint32_t index) {
	// Wait for a moment
	for (uint32_t k = 0; k < MAX_YIELD_COUNT; k++) {
		if ((ITask*)taskHead.load(std::memory_order_acquire) == nullptr) {
			YieldThread();
		} else {
			break;
		}
	}

	ITask* p = (ITask*)taskHead.exchange(nullptr, std::memory_order_acquire);
	// Has task?
	if (p != nullptr) {
		ITask* next = p->next;
		assert(next != (ITask*)(size_t)~0);
		p->next = (ITask*)(size_t)~0;

		if (next != nullptr) {
			ITask* t = (ITask*)taskHead.exchange(next, std::memory_order_release);
			// Someone has pushed some new tasks at the same time.
			// So rechain remaining tasks proceeding to the current one to new task head atomically.
			while (t != nullptr) {
				ITask* q = t;
				t = t->next;
				q->next = taskHead.load(std::memory_order_acquire);
				while (!taskHead.compare_exchange_weak(q->next, q, std::memory_order_release)) {
					YieldThreadFast();
				}
			}
		}

		std::atomic_thread_fence(std::memory_order_release);

		// OK. now we can execute the task
		void* context = threadInfos[index].context;

		// Exited?
		if (runningToken.load(std::memory_order_relaxed) == 0) {
			p->Abort(context);
		} else {
			p->Execute(context);
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
		if (!PollRoutine(safe_cast<uint32_t>(index)) && runningToken.load(std::memory_order_acquire) != 0) {
			threadApi.DoLock(mutex);
			++waitEventCounter;
			std::atomic_thread_fence(std::memory_order_release);
			threadApi.Wait(eventPump, mutex, MAX_WAIT_MILLISECONDS);
			--waitEventCounter;
			threadApi.UnLock(mutex);
		}
	}

	liveThreadCount.fetch_sub(1, std::memory_order_release);
	return false; // manages IThread::Thread* by ourself
}
