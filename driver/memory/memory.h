#pragma once
#include "../utils/defs.h"
#include "../utils/message.h"
#include "process.h"
#include "stdint.h"

uint64_t get_kernel_module(const char* module_name);

NTSTATUS get_guarded_region(int pid, uintptr_t* p_buffer);

ULONG64 get_module_imagebase(int pid);

intptr_t get_process_id(const char* process_name);

uintptr_t get_module_base(int pid,UNICODE_STRING module_name);

NTSTATUS read_virtual_memory(HANDLE pid, unsigned __int64 address, void* buffer, unsigned __int64 size);

BOOLEAN is_guarded(uintptr_t pointer);

uintptr_t read_guardered(uintptr_t pointer, uintptr_t guarded_region);
