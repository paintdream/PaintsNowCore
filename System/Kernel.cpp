#include "Kernel.h"
using namespace PaintsNow;

void WarpTiny::SetWarpIndex(uint32_t warpIndex) {
	FLAG old, value;
	
	// Update warp index atomically
	do {
		old = value = flag.load(std::memory_order_acquire);
		value = (value & ~(WARP_INDEX_END - WARP_INDEX_BEGIN)) | (warpIndex << Math::Log2((uint32_t)WARP_INDEX_BEGIN));
	} while (!flag.compare_exchange_weak(old, value, std::memory_order_release));
}

uint32_t WarpTiny::GetWarpIndex() const {
	return (flag.load(std::memory_order_relaxed) & (WARP_INDEX_END - WARP_INDEX_BEGIN)) >> Math::Log2((uint32_t)WARP_INDEX_BEGIN);
}

void WarpTiny::AssertWarp(Kernel& kernel) const {
	assert(GetWarpIndex() == kernel.GetCurrentWarpIndex());
}

Kernel::Kernel(ThreadPool& tp, uint32_t warpCount) : threadPool(tp) {
	uint32_t threadCount = (uint32_t)threadPool.GetThreadCount();
	if (warpCount == 0) {
		// set warp count automatically if not specified.
		warpCount = threadCount * 2;
	}

	// but not exceeds the warp bits restriction.
	warpCount = Math::Min(warpCount, safe_cast<uint32_t>(1 << WarpTiny::WARP_BITS) - 1);
	taskQueueGrid.resize(warpCount, SubTaskQueue(this, safe_cast<uint32_t>(threadCount)));

#ifdef _DEBUG
	activeTaskCount.store(0, std::memory_order_relaxed);
#endif
}

Kernel::~Kernel() { Clear(); }

uint32_t Kernel::GetWarpCount() const {
	return safe_cast<uint32_t>(taskQueueGrid.size());
}

// Warp index is thread_local
thread_local uint32_t WarpIndex = ~0;

uint32_t Kernel::GetCurrentWarpIndex() const {
	return WarpIndex;
}

void Kernel::Clear() {
	// Suspend all warps so we can take over tasks
	for (uint32_t i = 0; i < GetWarpCount(); i++) {
		SuspendWarp(i);
	}

	// Just abort them
	uint32_t threadIndex = threadPool.GetCurrentThreadIndex();
	threadIndex = threadIndex == ~(uint32_t)0 ? 0 : threadIndex;
	for (uint32_t j = 0; j < GetWarpCount(); j++) {
		SubTaskQueue& q = taskQueueGrid[j];

		if (threadPool.GetThreadCount() != 0) {
			while (!q.PreemptExecution()) {
				if (!threadPool.PollRoutine(threadIndex)) {
					YieldThread();
				}
			}
		}

		q.Abort(nullptr);
		q.YieldExecution();
	}

	// Resume warps
	for (uint32_t k = 0; k < GetWarpCount(); k++) {
		ResumeWarp(k);
	}

#ifdef _DEBUG
	assert(activeTaskCount.load(std::memory_order_acquire) == 0);
	taskQueueGrid.clear();
#endif
}

// Make VC6 Happy.
class ForwardRoutine : public TaskOnce {
public:
	ForwardRoutine(Kernel& k, WarpTiny* tn, ITask* tk) : kernel(k), tiny(tn), task(tk) {}
	void Execute(void* context) override {
		assert(next == nullptr);
		assert(queued == 0);
		// requeue it
		uint32_t warpIndex = tiny->GetWarpIndex();
		kernel.QueueRoutine(tiny, task);
		tiny->ReleaseObject();
		delete this;
	}

	void Abort(void* context) override {
		assert(next == nullptr);
		assert(queued == 0);
		// force
		WarpIndex = tiny->GetWarpIndex();
		task->Abort(context);
		tiny->ReleaseObject();
		delete this;
	}

private:
	Kernel& kernel;
	WarpTiny* tiny;
	ITask* task;
};

inline void Kernel::QueueRoutineInternal(uint32_t toWarpIndex, uint32_t fromThreadIndex, WarpTiny* warpTiny, ITask* task) {
	// Different warp-thread pair indicates different queue grid, so it's thread safe.
	SubTaskQueue& queue = taskQueueGrid[toWarpIndex];
	queue.Push(fromThreadIndex, task, warpTiny);
	queue.Flush(threadPool);
}

// QueueRoutine from any thread in threadPool
void Kernel::QueueRoutine(WarpTiny* warpTiny, ITask* task) {
	assert(warpTiny != nullptr);
	uint32_t fromThreadIndex = threadPool.GetCurrentThreadIndex();
	uint32_t toWarpIndex = warpTiny->GetWarpIndex();
	SubTaskQueue& q = taskQueueGrid[toWarpIndex];
	if (fromThreadIndex == ~(uint32_t)0) {
		// external thread?
		// since we may not in any thread of thread pool
		// locking is required because thread pool could be resetting at this time.
		threadPool.DoLock();
		assert(threadPool.IsLocked());
		// forward to threadPool directly
		warpTiny->ReferenceObject();
		threadPool.Push(new ForwardRoutine(*this, warpTiny, task));
		threadPool.UnLock();
	} else if (WarpIndex == toWarpIndex && (size_t)q.threadWarp.load(std::memory_order_acquire) == (size_t)&WarpIndex && q.suspendCount.load(std::memory_order_acquire) == 0) {
		// Just the same warp? Execute at once.
		task->Execute(threadPool.GetThreadContext(fromThreadIndex));
	} else {
		// Must be asynchronized
#ifdef _DEBUG
		activeTaskCount.fetch_add(1, std::memory_order_relaxed);
#endif
		warpTiny->ReferenceObject(); // hold reference in case of invalid memory access.
		QueueRoutineInternal(toWarpIndex, fromThreadIndex, warpTiny, task);
	}
}

void Kernel::QueueRoutinePost(WarpTiny* warpTiny, ITask* task) {
	assert(warpTiny != nullptr);
	uint32_t fromThreadIndex = threadPool.GetCurrentThreadIndex();
	uint32_t toWarpIndex = warpTiny->GetWarpIndex();
#ifdef _DEBUG
	activeTaskCount.fetch_add(1, std::memory_order_relaxed);
#endif
	warpTiny->ReferenceObject(); // hold reference in case of invalid memory access.
	QueueRoutineInternal(toWarpIndex, fromThreadIndex, warpTiny, task);
}

void Kernel::YieldCurrentWarp() {
	taskQueueGrid[WarpIndex].YieldExecution();
}

void Kernel::SuspendWarp(uint32_t warpIndex) {
	assert(warpIndex < taskQueueGrid.size());
	SubTaskQueue& taskQueue = taskQueueGrid[warpIndex];
	taskQueue.Suspend();
}

bool Kernel::ResumeWarp(uint32_t warpIndex) {
	assert(warpIndex < taskQueueGrid.size());
	SubTaskQueue& taskQueue = taskQueueGrid[warpIndex];
	return taskQueue.Resume();
}

// SubTaskQueue
Kernel::SubTaskQueue::SubTaskQueue(Kernel* e, uint32_t idCount) : kernel(e), TaskQueue(idCount) {
	threadWarp.store(nullptr, std::memory_order_relaxed);
	suspendCount.store(0, std::memory_order_relaxed);
	queueing.store(0, std::memory_order_release);

	YieldExecution();
}

Kernel::SubTaskQueue::SubTaskQueue(const SubTaskQueue& rhs) : TaskQueue(safe_cast<uint32_t>(rhs.ringBuffers.size())) {
	kernel = rhs.kernel;
	threadWarp.store(nullptr, std::memory_order_relaxed);
	suspendCount.store(0, std::memory_order_relaxed);
	queueing.store(0, std::memory_order_release);

	YieldExecution();
}

inline void Kernel::SubTaskQueue::Suspend() {
	suspendCount.fetch_add(1, std::memory_order_acquire);
}

inline bool Kernel::SubTaskQueue::Resume() {
	bool ret = suspendCount.fetch_sub(1, std::memory_order_release) == 1;

	if (ret) {
		queueing.store(0, std::memory_order_relaxed);
		Flush(kernel->threadPool);
	}

	return ret;
}

Kernel::SubTaskQueue& Kernel::SubTaskQueue::operator = (const SubTaskQueue& rhs) {
	ringBuffers.resize(rhs.ringBuffers.size());
	kernel = rhs.kernel;

	return *this;
}

Kernel::SubTaskQueue::~SubTaskQueue() {}

void Kernel::SubTaskQueue::Flush(ThreadPool& threadPool) {
	// avoid duplicated flushes
	if (queueing.exchange(1, std::memory_order_relaxed) == 0) {
		TaskQueue::Flush(threadPool);
	}
}

bool Kernel::SubTaskQueue::PreemptExecution() {
	uint32_t* expected = nullptr;
	uint32_t thisWarpIndex = safe_cast<uint32_t>(this - &kernel->taskQueueGrid[0]);

	if (threadWarp.compare_exchange_strong(expected, &WarpIndex, std::memory_order_acquire)) {
		WarpIndex = thisWarpIndex;
		return true;
	} else {
		return expected == &WarpIndex;
	}
}

bool Kernel::SubTaskQueue::YieldExecution() {
	uint32_t* exp = &WarpIndex;
	if (threadWarp.compare_exchange_strong(exp, (uint32_t*)nullptr, std::memory_order_relaxed)) {
		WarpIndex = ~(uint32_t)0;
		if (queueing.exchange(0, std::memory_order_release) == 1) {
			Flush(kernel->threadPool);
		}

		return true;
	} else {
		return false;
	}
}

void Kernel::SubTaskQueue::Execute(void* context) {
	if (suspendCount.load(std::memory_order_acquire) == 0) {
		if (PreemptExecution()) {
			queueing.store(2, std::memory_order_relaxed);

			TaskQueue::Execute(context);
			if (!YieldExecution()) {
				Flush(kernel->threadPool);
			}
		}
	}
}

void Kernel::SubTaskQueue::Abort(void* context) {
	TaskQueue::Abort(context);
}

bool Kernel::SubTaskQueue::InvokeOperation(std::pair<ITask*, void*>& task, void (ITask::*operation)(void*), void* context) {
	uint32_t thisWarpIndex = safe_cast<uint32_t>(this - &kernel->taskQueueGrid[0]);
	assert(operation == &ITask::Abort || WarpIndex == thisWarpIndex);

	WarpTiny* warpTiny = reinterpret_cast<WarpTiny*>(task.second);
	// It's possible that tiny's warp changed before Invoking.
	if (warpTiny == nullptr || thisWarpIndex == warpTiny->GetWarpIndex()) {
		// Not changed, call it at once
		(task.first->*operation)(context);

		// we have hold tiny on enqueuing, release it after calling.
		if (warpTiny != nullptr) {
#ifdef _DEBUG
			kernel->activeTaskCount.fetch_sub(1, std::memory_order_relaxed);
#endif
			warpTiny->ReleaseObject();
		}
	} else {
		// requeue it
		kernel->QueueRoutineInternal(warpTiny->GetWarpIndex(), kernel->threadPool.GetCurrentThreadIndex(), warpTiny, task.first);
	}

	// recheck suspend and yield status
	return operation == &ITask::Abort || (suspendCount.load(std::memory_order_acquire) == 0 && (uint32_t*)threadWarp.load(std::memory_order_relaxed) == &WarpIndex && WarpIndex == thisWarpIndex);
}

void Kernel::SubTaskQueue::Push(uint32_t fromThreadIndex, ITask* task, WarpTiny* warpTiny) {
	// Just forward
	TaskQueue::Push(fromThreadIndex, task, warpTiny);
}

const std::vector<TaskQueue::RingBuffer>& Kernel::SubTaskQueue::GetRingBuffers() const {
	return TaskQueue::GetRingBuffers();
}
