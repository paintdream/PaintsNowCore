// TaskGraph.h
// PaintDream (paintdream@paintdream.com)
// 2018-10-30
//

#ifndef __TASKGRAPH_H__
#define __TASKGRAPH_H__

#include "Kernel.h"
#include "Tiny.h"

namespace PaintsNow {
	class ThreadPool;
	class TaskGraph : public TReflected<TaskGraph, Tiny> {
	public:
		TaskGraph(Kernel& kernel);
		virtual ~TaskGraph();
		uint32_t Insert(WarpTiny* host, ITask* task);
		void Next(uint32_t from, uint32_t to);
		void Commit(const TWrapper<void>& completion = TWrapper<void>());

	protected:
		class TaskNode : public ITask {
		public:
			virtual void Execute(void* context) override;
			virtual void Suspend(void* context) override final;
			virtual void Resume(void* context) override final;
			virtual void Abort(void* context) override;
			virtual bool Continue() const override final;

			TaskGraph* taskGraph;
			WarpTiny* host;
			ITask* task;
			uint32_t refCount;
			std::vector<TaskNode*> nextNodes;
		};

		friend class TaskNode;
		void Complete();

		Kernel& kernel;
		std::vector<TaskNode> taskNodes;
		std::atomic<uint32_t> completedCount;
		TWrapper<void> completion;
	};
}

#endif // __TASKGRAPH_H__