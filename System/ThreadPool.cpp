#define MAX_YIELD_COUNT 8
#define MAX_WAIT_MILLISECONDS 200

#include "ThreadPool.h"
#include "../Template/TProxy.h"
#include "../Driver/Profiler/Optick/optick.h"
#include <cassert>
#include <ctime>
#include <cstdlib>
#include <sstream>

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
	temperature.store(0, std::memory_order_relaxed);
	runningToken.store(1, std::memory_order_release);
	waitEventCounter = 0;

	assert(threadInfos.empty());
	threadInfos.resize(threadCount);

	// create thread workers
	for (size_t i = 0; i < threadInfos.size(); i++) {
		ThreadInfo& info = threadInfos[i];
		info.threadHandle = threadApi.NewThread(Wrap(this, &ThreadPool::Run), i);
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
		ITask* p = taskHead.exchange(nullptr, std::memory_order_acquire);
		while (p != nullptr) {
			p->Abort(nullptr);
			ITask* q = p;
			p = p->next;
			q->next = nullptr;
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
	if (runningToken.load(std::memory_order_acquire) != 0) {
		std::atomic<size_t>& queued = *reinterpret_cast<std::atomic<size_t>*>(&task->queued);
		if (queued.exchange(1, std::memory_order_acquire) == 1) // already pushed
			return true;

		// Chain task
		assert(task != taskHead.load(std::memory_order_acquire));
		assert(task->next == nullptr);
		task->next = taskHead.load(std::memory_order_acquire);
		while (!taskHead.compare_exchange_weak(task->next, task, std::memory_order_release)) {}

		std::atomic_thread_fence(std::memory_order_acquire);
		if (waitEventCounter != 0) {
			threadApi.Signal(eventPump);
		}

		return true;
	} else {
		task->Abort(nullptr);
		return false;
	}
}

bool ThreadPool::Poll(uint32_t index) {
	OPTICK_EVENT();
	// Wait for a moment
	ITask* probe = nullptr;
	for (uint32_t k = 0; k < MAX_YIELD_COUNT; k++) {
		if ((probe = (ITask*)taskHead.load(std::memory_order_acquire)) == nullptr) {
			YieldThreadFast();
		} else {
			break;
		}
	}

	if (probe != nullptr) {
		ITask* p = taskHead.exchange(nullptr, std::memory_order_acquire);

		// Has task?
		if (p != nullptr) {
			ITask* next = p->next;

			if (next != nullptr) {
				p->next = nullptr;
				ITask* t = taskHead.exchange(next, std::memory_order_release);
				// Someone has pushed some new tasks at the same time.
				// So rechain remaining tasks proceeding to the current one to new task head atomically.
				while (t != nullptr) {
					ITask* q = t;
					assert(q->queued == 1);
					t = t->next;
					q->next = taskHead.load(std::memory_order_acquire);
					while (!taskHead.compare_exchange_weak(q->next, q, std::memory_order_release)) {}
				}

				temperature.store(threadCount, std::memory_order_release);
			}

			assert(p->next == nullptr);
			std::atomic<size_t>& queued = *reinterpret_cast<std::atomic<size_t>*>(&p->queued);
			queued.store(0, std::memory_order_release);

			// OK. now we can execute the task
			void* context = threadInfos[index].context;

			OPTICK_PUSH("Execute");

			// Exited?
			if (runningToken.load(std::memory_order_relaxed) == 0) {
				p->Abort(context);
			} else {
				p->Execute(context);
			}

			OPTICK_POP();
			return true;
		}

		return true;
	} else if ((long)temperature.load(std::memory_order_acquire) > 0) {
		return (long)temperature.fetch_sub(1, std::memory_order_relaxed) >= 1;
	} else {
		return false;
	}
}

bool ThreadPool::IsRunning() const {
	return runningToken.load(std::memory_order_acquire) != 0;
}

bool ThreadPool::PollDelay(uint32_t index, uint32_t delay) {
	if (delay == 0) {
		return Poll(safe_cast<uint32_t>(index));
	} else {
		if (!Poll(safe_cast<uint32_t>(index)) && runningToken.load(std::memory_order_acquire) != 0) {
			threadApi.DoLock(mutex);
			++waitEventCounter;
			std::atomic_thread_fence(std::memory_order_release);
			// OPTICK_CATEGORY("Sleep", Optick::Category::Wait);
			threadApi.Wait(eventPump, mutex, delay);
			--waitEventCounter;
			threadApi.UnLock(mutex);

			return false;
		} else {
			return true;
		}
	}
}

bool ThreadPool::PollWait(std::atomic<uint32_t>& variable, uint32_t mask, uint32_t flag, uint32_t delay) {
	uint32_t threadIndex = GetCurrentThreadIndex();
	bool result;
	while ((result = ((variable.load(std::memory_order_acquire) & mask) != flag)) && IsRunning()) {
		PollDelay(threadIndex, delay);
	}

	return !result;
}

uint32_t ThreadPool::PollExchange(std::atomic<uint32_t>& variable, uint32_t value, uint32_t delay) {
	uint32_t threadIndex = GetCurrentThreadIndex();
	uint32_t target;
	while ((target = variable.exchange(value, std::memory_order_acq_rel)) == value && IsRunning()) {
		PollDelay(threadIndex, delay);
	}

	return target;
}

bool ThreadPool::PollCompareExchange(std::atomic<uint32_t>& variable, uint32_t mask, uint32_t flag, uint32_t delay) {
	uint32_t threadIndex = GetCurrentThreadIndex();
	uint32_t current = variable.load(std::memory_order_acquire);
	uint32_t target;
	bool result = false;

	while (IsRunning()) {
		target = (current & ~mask) | ((current & mask) & flag);
		if (!(result = variable.compare_exchange_strong(current, target, std::memory_order_release))) {
			PollDelay(threadIndex, delay);
		} else {
			break;
		}
	}

	return result;
}

bool ThreadPool::Run(IThread::Thread* thread, size_t index) {
	std::stringstream ss;
	ss << "Worker " << index;
	OPTICK_THREAD(ss.str().c_str());

	// set thread local
	localThreadIndex = safe_cast<uint32_t>(index);
	// fetch one and execute
	liveThreadCount.fetch_add(1, std::memory_order_acquire);
	while (runningToken.load(std::memory_order_acquire) != 0) {
		PollDelay(safe_cast<uint32_t>(index), MAX_WAIT_MILLISECONDS);
	}

	liveThreadCount.fetch_sub(1, std::memory_order_release);
	return false; // manages IThread::Thread* by ourself
}
