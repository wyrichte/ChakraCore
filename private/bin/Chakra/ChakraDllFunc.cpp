
#include <windows.h>

EXTERN_C BOOL WINAPI ChakraDllMain(HINSTANCE hmod, DWORD dwReason, PVOID pvReserved);

EXTERN_C BOOL WINAPI DllMain(HINSTANCE hmod, DWORD dwReason, PVOID pvReserved)
{
    return ChakraDllMain(hmod, dwReason, pvReserved);
}

extern "C" const __declspec(selectany) CLSID CLSID_StdGlobalInterfaceTable = { 0x00000323,0x0000,0x0000,{ 0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 } };