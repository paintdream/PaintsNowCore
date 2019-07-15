#include "ITask.h"

using namespace PaintsNow;

ITask::ITask() : flag(0) {}
ITask::~ITask() {}

void TaskOnce::Suspend(void* context) {}
void TaskOnce::Resume(void* context) {}
void TaskOnce::Abort(void* context) {
	delete this;
}
bool TaskOnce::Continue() const { return true; }

void TaskRepeat::Suspend(void* context) {}
void TaskRepeat::Resume(void* context) {}
bool TaskRepeat::Continue() const { return true; }

ITask* ITask::UpdateFlag(uint32_t f) {
	flag = f;
	return this;
}
