// Copyright (C) Microsoft. All rights reserved.

#pragma once

#define NATVIS_API __declspec(dllexport)

namespace NatVis
{
    typedef struct tagDEBUGHELPER
    {
        DWORD dwVersion;
        BOOL (WINAPI *ReadDebuggeeMemory)(struct tagDEBUGHELPER *pThis, DWORD dwAddr, DWORD nWant, VOID* pWhere, DWORD *nGot);
        // from here only when dwVersion >= 0x20000
        unsigned __int64 (WINAPI *GetRealAddress)(struct tagDEBUGHELPER *pThis);
        BOOL (WINAPI *ReadDebuggeeMemoryEx)(struct tagDEBUGHELPER *pThis, unsigned __int64 qwAddr, DWORD nWant, VOID* pWhere, DWORD *nGot);
        int (WINAPI *GetProcessorType)(struct tagDEBUGHELPER *pThis);
    } DEBUGHELPER;

    template<class T> bool Read(DEBUGHELPER *const debugHelper, T *const t);

    #pragma prefast(suppress: 28718, "Unannotated buffer - Intentional, annotation is not a requirement")
    NATVIS_API HRESULT NatVis_ValueType(const DWORD address, DEBUGHELPER *const debugHelper, const int base, const BOOL uniStrings, char *const str, const size_t strSize, const DWORD /* reserved */);

    #pragma prefast(suppress: 28718, "Unannotated buffer - Intentional, annotation is not a requirement")
    NATVIS_API HRESULT NatVis_BVSparseNode(const DWORD address, DEBUGHELPER *const debugHelper, const int base, const BOOL uniStrings, char *const str, const size_t strSize, const DWORD /* reserved */);

    #pragma prefast(suppress: 28718, "Unannotated buffer - Intentional, annotation is not a requirement")
    NATVIS_API HRESULT NatVis_BVSparse_JitArenaAllocator(const DWORD address, DEBUGHELPER *const debugHelper, const int base, const BOOL uniStrings, char *const str, const size_t strSize, const DWORD /* reserved */);
}
