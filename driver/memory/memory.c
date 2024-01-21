#include "memory.h"

static const uint64_t mask = (~0xfull << 8) & 0xfffffffffull;

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
		if (strcmp((char*)module[i].FullPathName + module[i].OffsetToFileName, module_name) == 0) {
			module_base = (uint64_t)module[i].ImageBase;
			break;
		}
	}

	ExFreePoolWithTag(modules, KM_POOL_TAG);
	return module_base;
}

NTSTATUS get_guarded_region(int pid, uintptr_t* p_buffer) {
	uintptr_t module_vkg = get_kernel_module("vgk.sys");
	//message("module_vgk: %p\n", module_vkg);
	uintptr_t vgk_pool_offset = module_vkg + 0x80CE0;
	//message("vgk_pool_offset: %p\n", vgk_pool_offset);
	NTSTATUS status;
	uintptr_t guardedregion;
	status = read_virtual_memory(pid, vgk_pool_offset, &guardedregion, sizeof(uintptr_t));
	if (!NT_SUCCESS(status)) {
		message("get pool failed %p:", status);
		return status;
	}
	message("guarded region value: %p\n", guardedregion);
	*p_buffer = guardedregion;
	//uintptr_t uwrold;
	//status = read_virtual_memory(pid, guardedregion+0x60, &uwrold, sizeof(uintptr_t));
	//if (!NT_SUCCESS(status)) {
	//	message("uwrold failed %p:", status);
	//	return status;
	//}
	//if (is_guarded(uwrold)) {
	//	read_guardered(uwrold, guardedregion);
	//}	
	//unsigned long long uworld_offset;
	//if (uwrold > 0x10000000000)
	//{
	//	uworld_offset = uwrold - 0x10000000000;
	//}
	//else {
	//	uworld_offset = uwrold - 0x8000000000;
	//}
	//uwrold = (uintptr_t)guardedregion + uworld_offset;
	//message("uwrold value: %p\n", uwrold);
	//uintptr_t gameinstance;
	//status = read_virtual_memory(pid, uwrold+0x1A0, &gameinstance, sizeof(uintptr_t));
	//if (!NT_SUCCESS(status)) {
	//	message("gameinstance failed %p:", status);
	//	return status;
	//}
	//if (is_guarded(gameinstance)) {
	//	read_guardered(gameinstance, guardedregion);
	//}
	//message("gameinstance value: %p\n", gameinstance);
	//uintptr_t ULocalPlayerArray;
	//status = read_virtual_memory(pid, gameinstance + 0x40, &ULocalPlayerArray, sizeof(uintptr_t));
	//if (!NT_SUCCESS(status)) {
	//	message("ULocalPlayerArray failed %p:", status);
	//	return status;
	//}
	//if (is_guarded(ULocalPlayerArray)) {
	//	read_guardered(ULocalPlayerArray, guardedregion);
	//}
	//message("ULocalPlayerArray value: %p\n", ULocalPlayerArray);
	//uintptr_t ULocalPlayer;
	//status = read_virtual_memory(pid, ULocalPlayerArray, &ULocalPlayer, sizeof(uintptr_t));
	//if (!NT_SUCCESS(status)) {
	//	message("ULocalPlayer failed %p:", status);
	//	return status;
	//}
	//if (is_guarded(ULocalPlayer)) {
	//	ULocalPlayer = read_guardered(ULocalPlayer, guardedregion);
	//}
	//message("ULocalPlayer value: %p\n", ULocalPlayer);

	//uintptr_t APlayerControllerPtr;
	//status = read_virtual_memory(pid, ULocalPlayer + 0x38, &APlayerControllerPtr, sizeof(uintptr_t));
	//if (!NT_SUCCESS(status)) {
	//	message("APlayerControllerPtr failed %p:", status);
	//	return status;
	//}
	//message("APlayerControllerPtr pre value: %p\n", APlayerControllerPtr);
	//if (is_guarded(APlayerControllerPtr)) {
	//	APlayerControllerPtr = read_guardered(APlayerControllerPtr, guardedregion);
	//}
	//message("APlayerControllerPtr final value: %p\n", APlayerControllerPtr);
	//uintptr_t APawn;
	//status = read_virtual_memory(pid, APlayerControllerPtr + 0x468, &APawn, sizeof(uintptr_t));
	//if (!NT_SUCCESS(status)) {
	//	message("APawn failed %p:", status);
	//	return status;
	//}
	//if (is_guarded(APawn)) {
	//	APawn = read_guardered(APawn, guardedregion);
	//}
	//message("APawn value: %p\n", APawn);
	//uintptr_t DamageHandler;
	//status = read_virtual_memory(pid, APawn + 0x9F0, &DamageHandler, sizeof(uintptr_t));
	//if (!NT_SUCCESS(status)) {
	//	message("DamageHandler failed %p:", status);
	//	return status;
	//}
	//if (is_guarded(DamageHandler)) {
	//	DamageHandler = read_guardered(DamageHandler, guardedregion);
	//}
	//message("DamageHandler value: %p\n", DamageHandler);

	//float Health;
	//status = read_virtual_memory(pid, DamageHandler + 0x1B0, &Health, sizeof(float));
	//if (!NT_SUCCESS(status)) {
	//	message("Health failed %p:", status);
	//	return status;
	//}
	//message("Health valu %d", (int)Health);
	return STATUS_SUCCESS;
}

BOOLEAN is_guarded(uintptr_t pointer) {
	uintptr_t filter = 0xFFFFFFF000000000;
	uintptr_t result = pointer & filter;
	return result == 0x8000000000 || result == 0x10000000000;
}

uintptr_t read_guardered(uintptr_t pointer, uintptr_t guarded_region) {
	unsigned long long pointer_offset;
	if (pointer > 0x10000000000)
	{
		pointer_offset = pointer - 0x10000000000;
	}
	else {
		pointer_offset = pointer - 0x8000000000;
	}
	return (uintptr_t)guarded_region + pointer_offset;
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
