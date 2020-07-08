// TaskQueue.h
// PaintDream (paintdream@paintdream.com)
// 2018-10-30
//

#ifndef __TASKQUEUE_H__
#define __TASKQUEUE_H__

#include "../Template/TAtomic.h"
#include "../Template/TQueue.h"
#include "../Interface/ITask.h"
#include "../Interface/IType.h"
#include <vector>

namespace PaintsNow {
	// TaskQueue is single-thread read + single-thread/write task ring buffer
	// All tasks in the same task queue are scheduled in sequence. So no lock is needed for themselves ...
	// TaskQueue is also an ITask.
	class ThreadPool;
	class TaskQueue : public ITask {
	public:
		TaskQueue(uint32_t idCount);
		virtual ~TaskQueue();
		void Push(uint32_t id, ITask* task, void* tag);
		void Flush(ThreadPool& threadPool);
		typedef TQueueList<std::pair<ITask*, void*>, 8> RingBuffer;
		const std::vector<RingBuffer>& GetRingBuffers() const;

	protected:
		virtual bool InvokeOperation(std::pair<ITask*, void*>& task, void (ITask::*operation)(void*), void* context);

		virtual void Execute(void* context) override;
		virtual void Suspend(void* context) override final;
		virtual void Resume(void* context) override final;
		virtual void Abort(void* context) override;
		virtual bool Continue() const override final;

	protected:
		void OnOperation(void (ITask::*operation)(void*), void* context);

		// Wait-free Ring Buffer
		std::vector<RingBuffer> ringBuffers;
		size_t threadIndex;
	};
}

#endif // __TASKQUEUE_H__