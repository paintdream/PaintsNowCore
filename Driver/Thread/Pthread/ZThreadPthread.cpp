#include "ZThreadPthread.h"

#if defined(_DEBUG) && defined(_MSC_VER)
#include <Windows.h>
#endif

#if !defined(_MSC_VER) || _MSC_VER > 1200
#define USE_STD_THREAD
#include <thread>
#include <mutex>
#include <condition_variable>
#else
#include <windows.h>
#include <process.h>
#if _WIN32_WINNT >= 0x0600 // Windows Vista
#define HAS_CONDITION_VARIABLE 1
#else
#define HAS_CONDITION_VARIABLE 0
#endif
#endif

using namespace PaintsNow;

class LockImpl : public IThread::Lock {
public:
	#ifdef USE_STD_THREAD
	std::mutex cs;
#if defined(_DEBUG) && defined(_MSC_VER)
	DWORD owner;
#endif
	#else
	CRITICAL_SECTION cs;
#if defined(_DEBUG)
	DWORD owner;
#endif
	#endif
	size_t lockCount;
};

class ThreadImpl : public IThread::Thread {
public:
#ifdef USE_STD_THREAD
	std::thread thread;
#else
	HANDLE thread;
#endif

	TWrapper<bool, IThread::Thread*, size_t> proxy;
	size_t index;
	bool running;
	bool managed;
	bool reserved[2];
};

class EventImpl : public IThread::Event {
public:
#ifdef USE_STD_THREAD
	std::condition_variable cond;
#else
#if HAS_CONDITION_VARIABLE
	CONDITION_VARIABLE cond;
#else
	HANDLE cond;
#endif
#endif
};

#ifdef USE_STD_THREAD
static void
#else
static UINT _stdcall
#endif
_ThreadProc(void* param)
{
	ThreadImpl* thread = reinterpret_cast<ThreadImpl*>(param);
	thread->running = true;
	bool deleteSelf = thread->proxy(thread, thread->index);

	if (deleteSelf) {
		thread->running = false;
		delete thread;
	}

#ifndef USE_STD_THREAD
	::_endthreadex(0);
	return 0;
#endif
}


ZThreadPthread::ZThreadPthread() {

}

ZThreadPthread::~ZThreadPthread() {
}


void ZThreadPthread::Wait(Thread* th) {
	ThreadImpl* t = static_cast<ThreadImpl*>(th);
#ifdef USE_STD_THREAD
	t->thread.join();
#else
	::WaitForSingleObject(t->thread, INFINITE);
#endif
	t->managed = false;
}

IThread::Thread* ZThreadPthread::NewThread(const TWrapper<bool, IThread::Thread*, size_t>& wrapper, size_t index) {
	ThreadImpl* t = new ThreadImpl();
	t->proxy = wrapper;
	t->index = index;
	t->managed = true;

#ifdef USE_STD_THREAD
	t->thread = std::thread(_ThreadProc, t);
#else
	t->thread = (HANDLE)::_beginthreadex(nullptr, 0, _ThreadProc, t, 0, nullptr);
#endif

	return t;
}

bool ZThreadPthread::IsThreadRunning(Thread* th) const {
	assert(th != nullptr);
	ThreadImpl* thread = static_cast<ThreadImpl*>(th);
	return thread->running;
}

void ZThreadPthread::DeleteThread(Thread* thread) {
	assert(thread != nullptr);
	ThreadImpl* t = static_cast<ThreadImpl*>(thread);
	if (t->managed) {
#ifdef USE_STD_THREAD
		t->thread.detach();
#else
		::CloseHandle(t->thread);
#endif
	}

	delete t;
}

IThread::Lock* ZThreadPthread::NewLock() {
	LockImpl* lock = new LockImpl();
	lock->lockCount = 0;
#if defined(_DEBUG) && defined(_MSC_VER)
	lock->owner = 0;
#endif

#ifndef USE_STD_THREAD
	::InitializeCriticalSection(&lock->cs);
#endif

	return lock;
}

void ZThreadPthread::DoLock(Lock* l) {
	assert(l != nullptr);
	LockImpl* lock = static_cast<LockImpl*>(l);
#ifdef USE_STD_THREAD
#if defined(_DEBUG) && defined(_MSC_VER)
	assert(lock->owner != ::GetCurrentThreadId());
#endif
	lock->cs.lock();
#if defined(_DEBUG) && defined(_MSC_VER)
	lock->owner = ::GetCurrentThreadId();
	// printf("Thread %d, takes: %p\n", lock->owner, lock);
#endif
#else
	::EnterCriticalSection(&lock->cs);
#endif
	assert(lock->lockCount == 0);
	lock->lockCount++;
}

void ZThreadPthread::UnLock(Lock* l) {
	assert(l != nullptr);
	LockImpl* lock = static_cast<LockImpl*>(l);
	lock->lockCount--;

#ifdef USE_STD_THREAD
#if defined(_DEBUG) && defined(_MSC_VER)
	// printf("Thread %d, releases: %p\n", lock->owner, lock);
	lock->owner = ::GetCurrentThreadId();
	lock->owner = 0;
#endif
	lock->cs.unlock();
#else
	::LeaveCriticalSection(&lock->cs);
#endif
}

bool ZThreadPthread::TryLock(Lock* l) {
	assert(l != nullptr);
	LockImpl* lock = static_cast<LockImpl*>(l);
#ifdef USE_STD_THREAD
	bool success = lock->cs.try_lock();
#else
	bool success = ::TryEnterCriticalSection(&lock->cs) != 0;
#endif
	if (success) {
		lock->lockCount++;
	}

	return success;
}

void ZThreadPthread::DeleteLock(Lock* l) {
	assert(l != nullptr);
	LockImpl* lock = static_cast<LockImpl*>(l);
#ifndef USE_STD_THREAD
	::DeleteCriticalSection(&lock->cs);
#endif

	delete lock;
}

bool ZThreadPthread::IsLocked(Lock* l) {
	assert(l != nullptr);
	LockImpl* lock = static_cast<LockImpl*>(l);
	return lock->lockCount != 0;
}

IThread::Event* ZThreadPthread::NewEvent() {
	EventImpl* ev = new EventImpl();
#ifndef USE_STD_THREAD
#if HAS_CONDITION_VARIABLE
	::InitializeConditionVariable(&ev->cond);
#else
	ev->cond = ::CreateEvent(NULL, FALSE, FALSE, NULL);
#endif
#endif
	return ev;
}

void ZThreadPthread::Signal(Event* event) {
	assert(event != nullptr);
	EventImpl* ev = static_cast<EventImpl*>(event);

#ifdef USE_STD_THREAD
	ev->cond.notify_one();
#else
#if HAS_CONDITION_VARIABLE
	::WakeConditionVariable(&ev->cond);
#else
	::SetEvent(ev->cond);
#endif
#endif
}

void ZThreadPthread::Wait(Event* event, Lock* lock, size_t timeout) {
	assert(event != nullptr);
	assert(lock != nullptr);
	EventImpl* ev = static_cast<EventImpl*>(event);
	LockImpl* l = static_cast<LockImpl*>(lock);
	assert(l->lockCount != 0);
	l->lockCount--; // it's safe because we still hold this lock

#ifdef USE_STD_THREAD
	std::unique_lock<std::mutex> u(l->cs, std::adopt_lock);
	ev->cond.wait_for(u, std::chrono::microseconds(timeout));
	u.release();
#else
#if HAS_CONDITION_VARIABLE
	::SleepConditionVariableCS(&ev->cond, &l->cs, (DWORD)timeout);
#else
	::LeaveCriticalSection(&l->cs);
	::WaitForSingleObject(ev->cond, (DWORD)timeout);
	::EnterCriticalSection(&l->cs);
#endif
#endif
	l->lockCount++; // it's also safe because we has already take lock before returning from pthread_cond_wait
}

void ZThreadPthread::Wait(Event* event, Lock* lock) {
	assert(event != nullptr);
	assert(lock != nullptr);
	EventImpl* ev = static_cast<EventImpl*>(event);
	LockImpl* l = static_cast<LockImpl*>(lock);
	l->lockCount--; // it's safe because we still hold this lock
#ifdef USE_STD_THREAD
	std::unique_lock<std::mutex> u(l->cs, std::adopt_lock);
#if defined(_DEBUG) && defined(_MSC_VER)
	l->owner = 0;
#endif
	ev->cond.wait(u);
#if defined(_DEBUG) && defined(_MSC_VER)
	l->owner = ::GetCurrentThreadId();
#endif
	u.release();
#else
#if HAS_CONDITION_VARIABLE
	::SleepConditionVariableCS(&ev->cond, &l->cs, INFINITE);
#else
	::LeaveCriticalSection(&l->cs);
	::WaitForSingleObject(ev->cond, INFINITE);
	::EnterCriticalSection(&l->cs);
#endif
#endif
	l->lockCount++; // it's also safe because we has already take lock before returning from pthread_cond_wait
}

void ZThreadPthread::DeleteEvent(Event* event) {
	assert(event != nullptr);
	EventImpl* ev = static_cast<EventImpl*>(event);
#ifndef USE_STD_THREAD
#if !HAS_CONDITION_VARIABLE
	::CloseHandle(ev->cond);
#endif
#endif

	delete ev;
}
