#include "communication/dispatch.h"
#include "utils/message.h"
#include "memory/memory.h"
#include "memory/process.h"

UNICODE_STRING device_name = RTL_CONSTANT_STRING(L"\\Device\\djdemagiok");
UNICODE_STRING device_link = RTL_CONSTANT_STRING(L"\\DosDevices\\dlinkjdemagiok");
PDEVICE_OBJECT device_object;

NTSTATUS driver_initialize(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path) {
    UNREFERENCED_PARAMETER(registry_path);

    IoCreateDevice(driver_object, 0, &device_name, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &device_object);
    IoCreateSymbolicLink(&device_link, &device_name);

    driver_object->MajorFunction[IRP_MJ_CREATE] = on_create;
    driver_object->MajorFunction[IRP_MJ_CLOSE] = on_close;
    driver_object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = on_message;

    device_object->Flags |= DO_DIRECT_IO;
    device_object->Flags &= ~DO_DEVICE_INITIALIZING;

    return STATUS_SUCCESS;
}

NTSTATUS driver_entry(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path) {
    UNREFERENCED_PARAMETER(driver_object);
    UNREFERENCED_PARAMETER(registry_path);

    __try {
        UNICODE_STRING driver_name = RTL_CONSTANT_STRING(L"\\Driver\\djdemagiok82");

        return IoCreateDriver(&driver_name, &driver_initialize);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }
}
