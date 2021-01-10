#include "ITask.h"
#include "../Template/TAllocator.h"
using namespace PaintsNow;

static TAllocator<64>* globalTaskAllocator64;
static TAllocator<128>* globalTaskAllocator128;
static TAllocator<256>* globalTaskAllocator256;

class TaskInitializer {
public:
	TaskInitializer() {
		globalTaskAllocator64 = new TAllocator<64>();
		globalTaskAllocator64->GetRootAllocator().Pin(globalTaskAllocator64);
		globalTaskAllocator64->ReleaseObject();
		globalTaskAllocator128 = new TAllocator<128>();
		globalTaskAllocator128->GetRootAllocator().Pin(globalTaskAllocator128);
		globalTaskAllocator128->ReleaseObject();
		globalTaskAllocator256 = new TAllocator<256>();
		globalTaskAllocator256->GetRootAllocator().Pin(globalTaskAllocator256);
		globalTaskAllocator256->ReleaseObject();
	}
};

ITask::ITask() : next(nullptr) {
	queued = 0;
}

ITask::~ITask() {}

void* ITask::Allocate(size_t size) {
	TSingleton<TaskInitializer>::Get();

	switch ((size + 63) / 64)
	{
	case 0:
		assert(false);
	case 1:
		return globalTaskAllocator64->Allocate();
	case 2:
		return globalTaskAllocator128->Allocate();
	case 3:
	case 4:
		return globalTaskAllocator256->Allocate();
	default:
		return ::operator new (size);
	}
}

void ITask::Deallocate(void* p, size_t size) {
	switch ((size + 63) / 64) {
	case 0:
		assert(false);
	case 1:
		globalTaskAllocator64->Deallocate(p);
		break;
	case 2:
		globalTaskAllocator128->Deallocate(p);
		break;
	case 3:
	case 4:
		globalTaskAllocator256->Deallocate(p);
		break;
	default:
		::operator delete (p);
		break;
	}
}

void TaskOnce::Suspend(void* context) {}
void TaskOnce::Resume(void* context) {}
void TaskOnce::Abort(void* context) {
	delete this;
}

bool TaskOnce::Continue() const { return true; }

void TaskRepeat::Suspend(void* context) {}
void TaskRepeat::Resume(void* context) {}
bool TaskRepeat::Continue() const { return true; }
