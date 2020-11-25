// IThread.h
// By PaintDream (paintdream@paintdream.com)
// 2014-12-14
//

#pragma once

#include "../PaintsNow.h"
#include "../Interface/IType.h"
#include "../Template/TProxy.h"
#include "../Template/TAtomic.h"
#include "IDevice.h"

namespace PaintsNow {
	class pure_interface IThread : public IDevice {
	public:
		~IThread() override;
		class Lock {};
		class Semaphore {};
		class Thread {};
		class Event {};
		
		virtual Thread* NewThread(const TWrapper<bool, Thread*, size_t>& wrapper, size_t index) = 0;
		virtual bool IsThreadRunning(Thread* thread) const = 0;
		virtual void Wait(Thread* thread) = 0;
		virtual void DeleteThread(Thread* thread) = 0;

		virtual Lock* NewLock() = 0;
		virtual void DoLock(Lock* lock) = 0;
		virtual void UnLock(Lock* lock) = 0;
		virtual bool TryLock(Lock* lock) = 0;
		virtual bool IsLocked(Lock* lock) = 0;
		virtual void DeleteLock(Lock* lock) = 0;

		virtual Event* NewEvent() = 0;
		virtual void Signal(Event* event) = 0;
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
		void DoLock();
		void UnLock();

		bool TryLock();
		bool IsLocked() const;
		IThread& GetThreadApi();

	protected:
		IThread& threadApi;
		IThread::Lock* mutex;
	};
}

