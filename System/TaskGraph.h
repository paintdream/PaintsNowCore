// TaskGraph.h
// PaintDream (paintdream@paintdream.com)
// 2018-10-30
//

#pragma once
#include "Kernel.h"
#include "Tiny.h"

namespace PaintsNow {
	class ThreadPool;
	class TaskGraph : public TReflected<TaskGraph, Tiny> {
	public:
		TaskGraph(Kernel& kernel);
		~TaskGraph() override;
		size_t Insert(WarpTiny* host, ITask* task);
		void Next(size_t from, size_t to);
		void Commit(const TWrapper<void>& completion = TWrapper<void>());

	protected:
		class TaskNode : public ITask {
		public:
			void Execute(void* context) override;
			void Suspend(void* context) final;
			void Resume(void* context) final;
			void Abort(void* context) override;
			bool Continue() const final;

			TaskGraph* taskGraph;
			WarpTiny* host;
			ITask* task;
			size_t refCount;
			std::vector<TaskNode*> nextNodes;
		};

		friend class TaskNode;
		void Complete();

		Kernel& kernel;
		std::vector<TaskNode> taskNodes;
		std::atomic<size_t> completedCount;
		TWrapper<void> completion;
	};
}

