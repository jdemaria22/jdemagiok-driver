#include "process.h"

NTSTATUS read_virtual_memory(HANDLE pid, unsigned __int64 address, void* buffer, unsigned __int64 size) {
	if (!pid || !address || !buffer || !size)
		return STATUS_UNSUCCESSFUL;

	SIZE_T Result = 0;
	NTSTATUS status = STATUS_SUCCESS;
	PEPROCESS process;
	PsLookupProcessByProcessId((HANDLE)pid, &process);

	status = MmCopyVirtualMemory(process, (void*)address, (PEPROCESS)PsGetCurrentProcess(), (void*)buffer, size, KernelMode, &Result);

	if (!NT_SUCCESS(status))
		return status;
	else
		return status;
}


NTSTATUS read_physical_memory(int pid, void* address, void* buffer, unsigned __int64 size) {
	if (!pid || !address || !buffer || !size)
		return STATUS_UNSUCCESSFUL;

	PEPROCESS process = 0;
	PsLookupProcessByProcessId(pid, &process);
	if (!process)
		return STATUS_UNSUCCESSFUL;

	KAPC_STATE state;
	KeStackAttachProcess(process, &state);

	MM_COPY_ADDRESS physical_address = { 0 };

	physical_address.PhysicalAddress = MmGetPhysicalAddress(address);
	KeUnstackDetachProcess(&state);

	SIZE_T bytes = 0;

	NTSTATUS status = MmCopyMemory(buffer, physical_address, size, MM_COPY_MEMORY_PHYSICAL, &bytes);
	if (!NT_SUCCESS(status))
		return status;
	else
		return status;
}