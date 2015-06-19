#pragma once

class HostSystemInfo
{
public:
    static bool SupportsOnlyMultiThreadedCOM()
    {
        return Data.deviceInfoRetrieved
            && (Data.deviceFamily == 0x00000004 /*DEVICEFAMILYINFOENUM_MOBILE*/); //TODO: pick some other platform to the list
    }

    HostSystemInfo();

private:
    static HostSystemInfo Data;
    bool   deviceInfoRetrieved; 
    ULONG  deviceFamily;
    ULONGLONG uapInfo;
    ULONG  deviceForm;
};
