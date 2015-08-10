
#include <windows.h>

EXTERN_C BOOL WINAPI ChakraDllMain(HINSTANCE hmod, DWORD dwReason, PVOID pvReserved);

EXTERN_C BOOL WINAPI DllMain(HINSTANCE hmod, DWORD dwReason, PVOID pvReserved)
{
    return ChakraDllMain(hmod, dwReason, pvReserved);
}