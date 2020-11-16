#include "IMemory.h"
#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#endif

using namespace PaintsNow;

void IMemory::SetHardwareBreakpoint(void* address, size_t length, BREAK_TYPE type, size_t slot) {
#ifdef _WIN32
	assert(slot < 4);
	slot = slot % 4;
	size_t flag = (1
		| (type == EXECUTE ? 0 : type == WRITE ? 0x10000 : 0x30000)
		| (length == 8 ? 0x2000000 : length == 4 ? 0x3000000 : length == 2 ? 0x1000000 : 0)) << (slot * 2);
	size_t mask = (0x3 | 0x30000 | 0x3000000) << (slot * 2);

	HANDLE h = ::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	DWORD currentThreadID = ::GetCurrentThreadId();
	DWORD currentProcessID = ::GetCurrentProcessId();
	if (h != INVALID_HANDLE_VALUE) {
		THREADENTRY32 te;
		te.dwSize = sizeof(THREADENTRY32);
		if (::Thread32First(h, &te)) {
			do {
				if (te.th32OwnerProcessID == currentProcessID) {
					HANDLE hThread = ::OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
					if (hThread != INVALID_HANDLE_VALUE) {
						if (te.th32ThreadID != currentThreadID) {
							::SuspendThread(hThread);
						}

						CONTEXT context;
						context.ContextFlags = CONTEXT_DEBUG_REGISTERS;

						if (::GetThreadContext(hThread, &context)) {
							context.ContextFlags = CONTEXT_DEBUG_REGISTERS;
							((void**)&context.Dr0)[slot] = address;
							context.Dr7 = (context.Dr7 & ~mask) | flag;
							::SetThreadContext(hThread, &context);
						}

						if (te.th32ThreadID != currentThreadID) {
							::ResumeThread(hThread);
						}

						::CloseHandle(hThread);
					}
				}
			} while (::Thread32Next(h, &te));
		}

		::CloseHandle(h);
	}
#endif
}
