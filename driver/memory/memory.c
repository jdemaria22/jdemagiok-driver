#include "memory.h"

uint64_t get_kernel_module(const char* module_name) {
	uint64_t module_base = 0;
	ULONG module_size = 0;
	PRTL_PROCESS_MODULES modules = NULL;
	NTSTATUS status = ZwQuerySystemInformation(0x0B, 0, 0, &module_size);

	if (status != STATUS_INFO_LENGTH_MISMATCH)
		return 0;


	modules = (PRTL_PROCESS_MODULES)ExAllocatePool2(POOL_FLAG_NON_PAGED, module_size, KM_POOL_TAG);
	if (!modules)
		return 0;

	status = ZwQuerySystemInformation(0x0B, modules, module_size, &module_size);
	if (!NT_SUCCESS(status)) {
		ExFreePoolWithTag(modules, KM_POOL_TAG);
		return 0;
	}

	PRTL_PROCESS_MODULE_INFORMATION module = modules->Modules;
	for (ULONG i = 0; i < modules->NumberOfModules; i++) {
		message("Module Name: %s\n", module[i].FullPathName + module[i].OffsetToFileName);
		message("Module Base: %p\n", module[i].ImageBase);
		message("Module Size: %d\n", module[i].ImageSize);
		if (strcmp((char*)module[i].FullPathName + module[i].OffsetToFileName, module_name) == 0) {
			module_base = (uint64_t)module[i].ImageBase;
			break;
		}
	}

	ExFreePoolWithTag(modules, KM_POOL_TAG);
	return module_base;
}

BOOL isguarded(uintptr_t pointer)
{
	uintptr_t filter = 0xFFFFFFF000000000;
	uintptr_t result = pointer & filter;
	return result == 0x8000000000 || result == 0x10000000000;
}


uintptr_t readguarded(uintptr_t src, uintptr_t guardedregion)
{
	uintptr_t buffer;
	read_virtual_memory(-1, src, &buffer, sizeof(uintptr_t));
	uintptr_t val = guardedregion + (*(uintptr_t*)&buffer & 0xFFFFFF);
	return &val;
}

uint64_t get_guarded_region() {
	uint64_t module_vkg = get_kernel_module("vgk.sys");
	message("module_vkg: %p\n", module_vkg);
	uint64_t vgk_pool_offset = module_vkg + 0x80CE0;
	message("vgk_pool_offset: %p\n", vgk_pool_offset);
	uint64_t pool_offset;
	uint64_t u_world;
	uint64_t gameInstance;
	uint64_t localplayer;
	uint64_t localplayerarray;
	NTSTATUS status = read_virtual_memory(-1, vgk_pool_offset, &pool_offset, sizeof(uint64_t));
	if (!NT_SUCCESS(status)) {
		message("get pool failed");
		return (uint64_t)-1;
	}
	status = read_virtual_memory(-1, pool_offset+0x60, &u_world, sizeof(uint64_t));
	if (!NT_SUCCESS(status)) {
		message("get uworld failed");
		return (uint64_t)-1;
	}
	u_world = u_world - 0x10000000000;
	message("pool_offset: %p\n", pool_offset);
	message("u_world: %p\n", pool_offset + u_world);
	status = read_virtual_memory(-1, (pool_offset + u_world) + 0x1A0, &gameInstance, sizeof(uint64_t));
	message("gameInstance: %p\n", gameInstance);
	uint64_t merca = readguarded(gameInstance, pool_offset);
	message("merca de la rica: %p\n", merca);
	status = read_virtual_memory(-1, gameInstance + 0x40, &localplayerarray, sizeof(uint64_t));
	status = read_virtual_memory(-1, localplayerarray, &localplayer, sizeof(uint64_t));
	message("Localplayer: %p\n", localplayer);

	return (uint64_t)pool_offset;
}

ULONG64 get_module_imagebase(int pid) {
	PEPROCESS proc;
	if (PsLookupProcessByProcessId((HANDLE)pid, &proc) != STATUS_SUCCESS)
		return 0;

	return (ULONG64)PsGetProcessSectionBaseAddress(proc);

}

// driver built for windows 10 22H2, change the offsets if you're using a different version
//https://www.vergiliusproject.com/kernels/x64/Windows%2010%20%7C%202016

intptr_t get_process_id(const char* process_name) {
	PEPROCESS proc;
	PEPROCESS sysproc = PsInitialSystemProcess;
	PLIST_ENTRY list = (PLIST_ENTRY)((char*)sysproc + ActiveProcessLinks); // _EPROCESS.ActiveProcessLinks
	PLIST_ENTRY head = list;
	do {
		proc = (PEPROCESS)((char*)list - ActiveProcessLinks); // _EPROCESS.ActiveProcessLinks
		if (strstr((char*)proc + ImageFileName, process_name)) { // _EPROCESS.ImageFileName
			return (intptr_t)PsGetProcessId(proc);
		}
		list = list->Flink;
	} while (list != head);

	return 0;
}

uintptr_t get_module_base(int pid, UNICODE_STRING module_name) {

	PEPROCESS proc;
	if (PsLookupProcessByProcessId((HANDLE)pid, &proc) != STATUS_SUCCESS)
		return 0;

	PPEB p_peb = (PPEB)PsGetProcessPeb(proc); 

	if (!p_peb)
		return 0; 

	KAPC_STATE state;

	KeStackAttachProcess(proc, &state);

	PPEB_LDR_DATA pLdr = (PPEB_LDR_DATA)p_peb->Ldr;

	if (!pLdr) {
		KeUnstackDetachProcess(&state);
		return 0; 
	}

	
	for (PLIST_ENTRY list = (PLIST_ENTRY)pLdr->InLoadOrderModuleList.Flink;
		list != &pLdr->InLoadOrderModuleList; list = (PLIST_ENTRY)list->Flink)
	{
		PLDR_DATA_TABLE_ENTRY pEntry =
			CONTAINING_RECORD(list, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);


		if (RtlCompareUnicodeString(&pEntry->BaseDllName, &module_name, TRUE) == 0) {
			message("Module Name: %wZ\n", pEntry->BaseDllName);
			message("Module Base: %p\n", pEntry->DllBase);
			message("Module Size: %d\n", pEntry->SizeOfImage);
			uintptr_t module_base = (uintptr_t)pEntry->DllBase;
			KeUnstackDetachProcess(&state);
		
			return module_base;
		}


	}

	KeUnstackDetachProcess(&state);
	message("Failed to find module\n");
	return 0; 
}
