#include "ZThreadPthread.h"
#if USE_STATIC_THIRDPARTY_LIBRARIES
#define PTW32_STATIC_LIB
#endif

#if !defined(_MSC_VER) || _MSC_VER > 1200
#define USE_STD_THREAD
#endif

#ifdef USE_STD_THREAD
#include <thread>
#include <mutex>
#include <condition_variable>
#else
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>
#include <time.h>
#endif

using namespace PaintsNow;

class LockImpl : public IThread::Lock {
public:
	#ifdef USE_STD_THREAD
	std::mutex cs;
	#else
	pthread_mutex_t lock;
	#endif
	size_t lockCount;
};

class ThreadImpl : public IThread::Thread {
public:
#ifdef USE_STD_THREAD
	std::thread thread;
#else
	pthread_t thread;
#endif

	TWrapper<bool, IThread::Thread*, size_t> proxy;
	size_t index;
	bool running;
	bool managed;
};

class EventImpl : public IThread::Event {
public:
#ifdef USE_STD_THREAD
	std::condition_variable cond;
#else
	pthread_cond_t cond;
#endif
};

#ifdef USE_STD_THREAD
static void
#else
static void*
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
	pthread_exit(nullptr);
	return nullptr;
#endif
}

#if defined(_WIN32) || defined(WIN32)
class Win32ThreadHelper {
public:
	Win32ThreadHelper() {
#ifndef USE_STD_THREAD
		pthread_win32_process_attach_np();
#endif
	}

	~Win32ThreadHelper() {
#ifndef USE_STD_THREAD
		pthread_win32_process_detach_np();
#endif
	}
};
#endif

ZThreadPthread::ZThreadPthread() {
#ifndef USE_STD_THREAD
	static Win32ThreadHelper threadHelper;
#endif
	AttachLocalThread();
}

ZThreadPthread::~ZThreadPthread() {
	DetachLocalThread();
}

void ZThreadPthread::AttachLocalThread() const {
#ifndef USE_STD_THREAD
	pthread_win32_thread_attach_np();
#endif
}

void ZThreadPthread::DetachLocalThread() const {
#ifndef USE_STD_THREAD
	pthread_win32_thread_detach_np();
#endif
}

void ZThreadPthread::Wait(Thread* th) {
	ThreadImpl* t = static_cast<ThreadImpl*>(th);
#ifdef USE_STD_THREAD
	t->thread.join();
#else
	void* param;
	pthread_join(t->thread, &param);
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
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
//	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&t->thread, nullptr, _ThreadProc, t);
	pthread_attr_destroy(&attr);
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
		pthread_detach(t->thread);
#endif
	}

	delete t;
}

IThread::Lock* ZThreadPthread::NewLock() {
	LockImpl* lock = new LockImpl();
	lock->lockCount = 0;

#ifndef USE_STD_THREAD
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	// pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	// pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
	pthread_mutex_init(&lock->lock, &attr);
	pthread_mutexattr_destroy(&attr);
#endif

	return lock;
}

void ZThreadPthread::DoLock(Lock* l) {
	assert(l != nullptr);
	LockImpl* lock = static_cast<LockImpl*>(l);
#ifdef USE_STD_THREAD
	lock->cs.lock();
#else
	pthread_mutex_lock(&lock->lock);
#endif
	lock->lockCount++;
}

void ZThreadPthread::UnLock(Lock* l) {
	assert(l != nullptr);
	LockImpl* lock = static_cast<LockImpl*>(l);
	lock->lockCount--;

#ifdef USE_STD_THREAD
	lock->cs.unlock();
#else
	pthread_mutex_unlock(&lock->lock);
#endif
}

bool ZThreadPthread::TryLock(Lock* l) {
	assert(l != nullptr);
	LockImpl* lock = static_cast<LockImpl*>(l);
#ifdef USE_STD_THREAD
	bool success = lock->cs.try_lock();
#else
	bool success = pthread_mutex_trylock(&lock->lock) == 0;
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
	pthread_mutex_destroy(&lock->lock);
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
	pthread_cond_init(&ev->cond, nullptr);
#endif
	return ev;
}

void ZThreadPthread::Signal(Event* event, bool broadcast) {
	assert(event != nullptr);
	EventImpl* ev = static_cast<EventImpl*>(event);

#ifdef USE_STD_THREAD
	if (broadcast) {
		ev->cond.notify_all();
	} else {
		ev->cond.notify_one();
	}
#else
	if (broadcast) {
		pthread_cond_broadcast(&ev->cond);
	} else {
		pthread_cond_signal(&ev->cond);
	}
#endif
}

#if !defined(USE_STD_THREAD) && _WIN32
#include <Windows.h>
#define CLOCK_MONOTONIC 0
int clock_gettime(int, struct timespec *spec) //C-file part
{
	__int64 wintime; GetSystemTimeAsFileTime((FILETIME*)&wintime);
	wintime -= (__int64)116444736000000000; //1jan1601 to 1jan1970
	spec->tv_sec = wintime / 10000000; //seconds
	spec->tv_nsec = wintime % 10000000 * 100; //nano-seconds
	return 0;
}
#endif

void ZThreadPthread::Wait(Event* event, Lock* lock, size_t timeout) {
	assert(event != nullptr);
	assert(lock != nullptr);
	EventImpl* ev = static_cast<EventImpl*>(event);
	LockImpl* l = static_cast<LockImpl*>(lock);
	l->lockCount--; // it's safe because we still hold this lock

#ifdef USE_STD_THREAD
	std::unique_lock<std::mutex> u(l->cs, std::adopt_lock);
	ev->cond.wait_for(u, std::chrono::microseconds(timeout));
	u.release();
#else
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);

	ts.tv_sec = ts.tv_sec + (long)(timeout / 1000);
	ts.tv_nsec = ts.tv_nsec + (timeout % 1000) * 1000000;
	ts.tv_sec += ts.tv_nsec / 1000000000;
	ts.tv_nsec = ts.tv_nsec % 1000000000;
	
	pthread_cond_timedwait(&ev->cond, &l->lock, &ts);
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
	ev->cond.wait(u);
	u.release();
#else
	pthread_cond_wait(&ev->cond, &l->lock);
#endif
	l->lockCount++; // it's also safe because we has already take lock before returning from pthread_cond_wait
}

void ZThreadPthread::DeleteEvent(Event* event) {
	assert(event != nullptr);
	EventImpl* ev = static_cast<EventImpl*>(event);
#ifndef USE_STD_THREAD
	pthread_cond_destroy(&ev->cond);
#endif

	delete ev;
}
