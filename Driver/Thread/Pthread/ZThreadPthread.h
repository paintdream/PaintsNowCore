// ZThreadPthread.h
// By PaintDream (paintdream@paintdream.com)
// 2014-12-14
//

#pragma once
#include "../../../Interface/IThread.h"

namespace PaintsNow {
	class ZThreadPthread : public IThread {
	public:
		ZThreadPthread(bool initOnWin32 = true);
		virtual ~ZThreadPthread();
		virtual void AttachLocalThread() const;
		virtual void DetachLocalThread() const;
		virtual IThread::Thread* NewThread(const TWrapper<bool, IThread::Thread*, size_t>& wrapper, size_t index, bool realtime);
		virtual bool IsThreadRunning(Thread* thread) const;
		virtual void Wait(Thread* thread);
		virtual void DeleteThread(Thread* thread);
		virtual IThread::Thread* OpenCurrentThread() const;
		virtual bool EqualThread(Thread* lhs, Thread* rhs) const;

		virtual IThread::Lock* NewLock();
		virtual bool DoLock(Lock* lock);
		virtual bool UnLock(Lock* lock);
		virtual bool TryLock(Lock* lock);
		virtual void DeleteLock(Lock* lock);
		virtual size_t GetLockCount(Lock* lock);

		virtual IThread::Semaphore* NewSemaphore(int initValue);
		virtual void Signal(Semaphore* semaphore);
		virtual void Wait(Semaphore* semaphore);
		virtual void DeleteSemaphore(Semaphore* semphore);

		virtual Event* NewEvent();
		virtual void Signal(Event* event, bool broadcast);
		virtual void Wait(Event* event, Lock* lock);
		virtual void Wait(Event* event, Lock* lock, size_t timeout);
		virtual void DeleteEvent(Event* event);

	protected:
		int maxPriority;
		int minPriority;
		bool initOnWin32;
	};
}

