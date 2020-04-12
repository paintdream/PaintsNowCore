#include "TaskGraph.h"
#include "ThreadPool.h"
using namespace PaintsNow;

void TaskGraph::TaskNode::Execute(void* context) {
	task->Execute(context);

	for (size_t i = 0; i < nextNodes.size(); i++) {
		TaskNode* node = nextNodes[i];
		TAtomic<uint32_t>& refCount = (TAtomic<uint32_t>&)node->refCount;
		if (refCount.fetch_sub(1, std::memory_order_relaxed) == 1) {
			taskGraph->kernel.QueueRoutine(host, node);
		}
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
		TAtomic<uint32_t>& refCount = (TAtomic<uint32_t>&)node->refCount;
		if (refCount.fetch_sub(1, std::memory_order_relaxed) == 1) {
			node->task->Abort(context);
		}
	}

	taskGraph->Complete();
}

bool TaskGraph::TaskNode::Continue() const {
	return task->Continue();
}

TaskGraph::TaskGraph(Kernel& k) : kernel(k) {
	completedCount.store(0, std::memory_order_relaxed);
}

TaskGraph::~TaskGraph() {}

void TaskGraph::Complete() {
	if (completedCount.fetch_add(1, std::memory_order_relaxed) + 1 == taskNodes.size()) {
		if (completion) {
			completion();
		}

		ReleaseObject();
	}
}

uint32_t TaskGraph::Insert(WarpTiny* host, ITask* task) {
	TaskNode node;
	node.taskGraph = this;
	node.host = host;
	node.task = task;
	node.refCount = 0;

	uint32_t id = safe_cast<uint32_t>(taskNodes.size());
	taskNodes.emplace_back(node);
	return id;
}

void TaskGraph::Next(uint32_t from, uint32_t to) {
	assert(from < taskNodes.size());
	assert(to < taskNodes.size());

	TaskNode* nextTask = &taskNodes[to];
	taskNodes[from].nextNodes.emplace_back(nextTask);
	nextTask->refCount++;
}

void TaskGraph::Commit(const TWrapper<void>& w) {
	completion = w;

	for (size_t i = 0; i < taskNodes.size(); i++) {
		TaskNode& node = taskNodes[i];
		if (node.refCount == 0) {
			kernel.QueueRoutine(node.host, &node);
		}
	}
}
