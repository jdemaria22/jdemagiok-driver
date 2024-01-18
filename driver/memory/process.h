#pragma once
#include "../utils/defs.h"

NTSTATUS read_virtual_memory(HANDLE pid, unsigned __int64 address, void* buffer, unsigned __int64 size);
NTSTATUS read_physical_memory(int pid, void* address, void* buffer, unsigned __int64 size);
