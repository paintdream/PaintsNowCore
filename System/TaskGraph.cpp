#include "TaskGraph.h"
#include "ThreadPool.h"
using namespace PaintsNow;

void TaskGraph::TaskNode::Execute(void* context) {
	task->Execute(context);

	// release the proceedings
	for (size_t i = 0; i < nextNodes.size(); i++) {
		TaskNode* node = nextNodes[i];
		std::atomic<size_t>& refCount = (std::atomic<size_t>&)node->refCount;

		// can be executed?
		if (refCount.fetch_sub(1, std::memory_order_relaxed) == 1) {
			if (node->host == nullptr) {
				taskGraph->kernel.GetThreadPool().Push(node);
			} else {
				taskGraph->kernel.QueueRoutine(node->host, node);
			}
		}
	}

	if (host != nullptr) {
		host->ReleaseObject();
		host = nullptr;
	}

	taskGraph->Complete();
}

void TaskGraph::TaskNode::Suspend(void* context) {
	task->Suspend(context);
}

void TaskGraph::TaskNode::Resume(void* context) {
	task->Resume(context);
}

void TaskGraph::TaskNode::Abort(void* context) {
	task->Abort(context);

	for (size_t i = 0; i < nextNodes.size(); i++) {
		TaskNode* node = nextNodes[i];
		std::atomic<size_t>& refCount = (std::atomic<size_t>&)node->refCount;
		if (refCount.fetch_sub(1, std::memory_order_relaxed) == 1) {
			node->task->Abort(context);
		}
	}

	if (host != nullptr) {
		host->ReleaseObject();
		host = nullptr;
	}

	taskGraph->Complete();
}

bool TaskGraph::TaskNode::Continue() const {
	return task->Continue();
}

TaskGraph::TaskGraph(Kernel& k) : kernel(k) {
	completedCount.store(0, std::memory_order_release);
}

TaskGraph::~TaskGraph() {
	for (size_t i = 0; i < taskNodes.size(); i++) {
		WarpTiny* host = taskNodes[i].host;
		if (host != nullptr) {
			host->ReleaseObject();
		}
	}
}

bool TaskGraph::IsRunning() const {
	return running.load(std::memory_order_acquire) != 0;
}

void TaskGraph::Complete() {
	// all tasks finished
	if (completedCount.fetch_add(1, std::memory_order_relaxed) + 1 == taskNodes.size()) {
		if (completion) {
			completion();
		}

		// reset completed count for next dispatch (if exists)
		completedCount.store(0, std::memory_order_relaxed);
		size_t r = running.exchange(0, std::memory_order_release);
		assert(r != 0);
		ReleaseObject();
	}
}

size_t TaskGraph::Insert(WarpTiny* host, ITask* task) {
	assert(running.load(std::memory_order_acquire) == 0);
	TaskNode node;
	node.taskGraph = this;
	node.host = host;
	node.task = task;
	node.refCount = 0;
	node.totalRefCount = 0;

	if (host != nullptr) {
		host->ReferenceObject();
	}

	size_t id = safe_cast<size_t>(taskNodes.size());
	taskNodes.emplace_back(std::move(node));
	return id;
}

void TaskGraph::Next(size_t from, size_t to) {
	assert(running.load(std::memory_order_acquire) == 0);
	assert(from < taskNodes.size());
	assert(to < taskNodes.size());

	TaskNode* nextTask = &taskNodes[to];
	taskNodes[from].nextNodes.emplace_back(nextTask);
	nextTask->totalRefCount++;
}

bool TaskGraph::Dispatch(const TWrapper<void>& w) {
	size_t r = running.exchange(1, std::memory_order_acq_rel);
	assert(r == 0);
	completion = w;

	bool committed = false;
	for (size_t i = 0; i < taskNodes.size(); i++) {
		TaskNode& node = taskNodes[i];
		node.refCount = node.totalRefCount;
		if (node.refCount == 0) {
			if (node.host == nullptr) {
				kernel.GetThreadPool().Push(&node);
			} else {
				kernel.QueueRoutine(node.host, &node);
			}

			committed = true;
		}
	}

	// check cycle and empty tasks
	if (committed) {
		ReferenceObject();
	}

	return committed;
}
