#include "IThread.h"

using namespace PaintsNow;

IThread::~IThread() {}

LockGuard::LockGuard(IThread& t, IThread::Lock* l) : thread(t), mutex(l) {
	thread.DoLock(mutex);
}

LockGuard::~LockGuard() {
	thread.UnLock(mutex);
}

ISyncObject::~ISyncObject() {
	threadApi.DeleteLock(mutex);
}

ISyncObject::ISyncObject(IThread& thread) : threadApi(thread) {
	mutex = threadApi.NewLock();
}

void ISyncObject::DoLock() {
	threadApi.DoLock(mutex);
}
void ISyncObject::UnLock() {
	threadApi.UnLock(mutex);
}

bool ISyncObject::TryLock() {
	return threadApi.TryLock(mutex);
}

bool ISyncObject::IsLocked() const {
	return threadApi.IsLocked(mutex);
}

IThread& ISyncObject::GetThreadApi() {
	return threadApi;
}
