#include "HostCommonPch.h"
#include "HostSysInfo.h"

HostSystemInfo HostSystemInfo::Data;

HostSystemInfo::HostSystemInfo()
{
    HMODULE hModNtDll = GetModuleHandleW(L"ntdll.dll");
    if (hModNtDll == nullptr)
    {
        RaiseException(0, EXCEPTION_NONCONTINUABLE, 0, 0);
    }
    typedef void(*PFNRTLGETDEVICEFAMILYINFOENUM)(ULONGLONG*, ULONG*, ULONG*);
    PFNRTLGETDEVICEFAMILYINFOENUM pfnRtlGetDeviceFamilyInfoEnum =
        reinterpret_cast<PFNRTLGETDEVICEFAMILYINFOENUM>(GetProcAddress(hModNtDll, "RtlGetDeviceFamilyInfoEnum"));

    if (pfnRtlGetDeviceFamilyInfoEnum)
    {
        pfnRtlGetDeviceFamilyInfoEnum(&this->uapInfo, &this->deviceFamily, &this->deviceForm);
        deviceInfoRetrieved = true;
    }
    else
    {
        deviceInfoRetrieved = false;
    }
}

