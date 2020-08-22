// IThread.h
// By PaintDream (paintdream@paintdream.com)
// 2014-12-14
//

#pragma once

#include "../PaintsNow.h"
#include "../Interface/IType.h"
#include "../Template/TProxy.h"
#include "IDevice.h"

#if defined(_MSC_VER) && _MSC_VER <= 1200
#include <windows.h>

inline void YieldThread() {
	if (!SwitchToThread()) {
		for (int i = 0; i < 16; i++)
		{
			YieldProcessor();
		}
	}
}

inline void YieldThreadFast() {
	YieldProcessor();
}

#else
#include <thread>
inline void YieldThread() {
	std::this_thread::yield();
}

inline void YieldThreadFast() {
	std::this_thread::yield();
}

#endif


namespace PaintsNow {
	class IThread : public IDevice {
	public:
		virtual ~IThread();
		class Lock {};
		class Semaphore {};
		class Thread {};
		class Event {};
		
		virtual void AttachLocalThread() const = 0;
		virtual void DetachLocalThread() const = 0;

		virtual Thread* NewThread(const TWrapper<bool, Thread*, size_t>& wrapper, size_t index, bool realtime) = 0;
		virtual bool IsThreadRunning(Thread* thread) const = 0;
		virtual void Wait(Thread* thread) = 0;
		virtual void DeleteThread(Thread* thread) = 0;
		virtual IThread::Thread* OpenCurrentThread() const = 0;
		virtual bool EqualThread(Thread* lhs, Thread* rhs) const = 0;

		virtual Lock* NewLock() = 0;
		virtual bool DoLock(Lock* lock) = 0;
		virtual bool UnLock(Lock* lock) = 0;
		virtual bool TryLock(Lock* lock) = 0;
		virtual size_t GetLockCount(Lock* lock) = 0;
		virtual void DeleteLock(Lock* lock) = 0;

		virtual Semaphore* NewSemaphore(int initValue) = 0;
		virtual void Signal(Semaphore* semaphore) = 0;
		virtual void Wait(Semaphore* semaphore) = 0;
		virtual void DeleteSemaphore(Semaphore* semphore) = 0;

		virtual Event* NewEvent() = 0;
		virtual void Signal(Event* event, bool broadcast) = 0;
		virtual void Wait(Event* event, Lock* lock) = 0;
		virtual void Wait(Event* event, Lock* lock, size_t timeout) = 0;
		virtual void DeleteEvent(Event* event) = 0;
	};

	class LockGuard {
	public:
		LockGuard(IThread& thread, IThread::Lock* mutex);
		~LockGuard();
	private:
		IThread& thread;
		IThread::Lock* mutex;
	};

	class ISyncObject {
	public:
		ISyncObject(IThread& threadApi);
		virtual ~ISyncObject();
		virtual void DoLock();
		virtual void UnLock();
		virtual bool TryLock();
		size_t GetLockCount() const;
		IThread& GetThreadApi();

	protected:
		IThread& threadApi;
		IThread::Lock* mutex;
	};

}

