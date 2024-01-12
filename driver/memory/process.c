#include "process.h"

NTSTATUS read_virtual_memory(int pid, PVOID source_addr, PVOID target_addr, SIZE_T size) {
	SIZE_T bytes;
	NTSTATUS status = STATUS_SUCCESS;
	PEPROCESS process;

	if (pid == -1) {
		process = PsGetCurrentProcess();
	}
	else {
		if (!NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)pid, &process)))
			return STATUS_INVALID_PARAMETER;
	}

	status = MmCopyVirtualMemory(process, source_addr, process, target_addr, size, KernelMode, &bytes);
	if (!NT_SUCCESS(status))
		return status;

	return status;

}