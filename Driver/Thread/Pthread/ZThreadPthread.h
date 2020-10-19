// ZThreadPthread.h
// By PaintDream (paintdream@paintdream.com)
// 2014-12-14
//

#pragma once
#include "../../../Interface/IThread.h"

namespace PaintsNow {
	class ZThreadPthread : public IThread {
	public:
		ZThreadPthread();
		~ZThreadPthread() override;
		void AttachLocalThread() const override;
		void DetachLocalThread() const override;
		IThread::Thread* NewThread(const TWrapper<bool, IThread::Thread*, size_t>& wrapper, size_t index, bool realtime) override;
		bool IsThreadRunning(Thread* thread) const override;
		void Wait(Thread* thread) override;
		void DeleteThread(Thread* thread) override;
		IThread::Thread* OpenCurrentThread() const override;
		bool EqualThread(Thread* lhs, Thread* rhs) const override;

		IThread::Lock* NewLock() override;
		bool DoLock(Lock* lock) override;
		bool UnLock(Lock* lock) override;
		bool TryLock(Lock* lock) override;
		void DeleteLock(Lock* lock) override;
		size_t GetLockCount(Lock* lock) override;

		IThread::Semaphore* NewSemaphore(int initValue) override;
		void Signal(Semaphore* semaphore) override;
		void Wait(Semaphore* semaphore) override;
		void DeleteSemaphore(Semaphore* semphore) override;

		Event* NewEvent() override;
		void Signal(Event* event, bool broadcast) override;
		void Wait(Event* event, Lock* lock) override;
		void Wait(Event* event, Lock* lock, size_t timeout) override;
		void DeleteEvent(Event* event) override;

	protected:
		int maxPriority;
		int minPriority;
	};
}

