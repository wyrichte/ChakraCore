// Copyright (C) Microsoft. All rights reserved.

#include "StdAfx.h"
#include "NativeVisualizerExtension.h"

namespace NatVis
{
    template<class T> bool Read(DEBUGHELPER *const debugHelper, const uint64 address, T *const t)
    {
        const DWORD sizeRequired = sizeof(T);
        DWORD sizeRead;
        #pragma prefast(suppress: 6216, "Cast between semantically different integer types: a Boolean type to HRESULT. - This is what the native visualizer extension sample suggests doing");
        return
            debugHelper->ReadDebuggeeMemoryEx(debugHelper, address, sizeRequired, t, &sizeRead) == S_OK &&
            sizeRead == sizeRequired;
    }

    #pragma prefast(suppress: 28718, "Unannotated buffer - Intentional, annotation is not a requirement")
    NATVIS_API HRESULT NatVis_ValueType(
        const DWORD address,            // low 32-bits of address
        DEBUGHELPER *const debugHelper, // callback pointer to access helper functions
        const int base,                 // decimal or hex
        const BOOL uniStrings,          // not used
        char *const str,                // where the result needs to go
        const size_t strSize,           // how large the above buffer is
        const DWORD /* reserved */)     // always pass zero
    {
        ValueType valueType;
        if(!Read(debugHelper, debugHelper->GetRealAddress(debugHelper), &valueType))
            return E_FAIL;

        valueType.ToStringDebug(str, strSize);
        return S_OK;
    }

    #pragma prefast(suppress: 28718, "Unannotated buffer - Intentional, annotation is not a requirement")
    NATVIS_API HRESULT NatVis_BVSparseNode(
        const DWORD address,            // low 32-bits of address
        DEBUGHELPER *const debugHelper, // callback pointer to access helper functions
        const int base,                 // decimal or hex
        const BOOL uniStrings,          // not used
        char *const str,                // where the result needs to go
        const size_t strSize,           // how large the above buffer is
        const DWORD /* reserved */)     // always pass zero
    {
        byte nodeData[sizeof(BVSparseNode)];
        BVSparseNode *const node = reinterpret_cast<BVSparseNode *>(nodeData);
        if(!Read(debugHelper, debugHelper->GetRealAddress(debugHelper), node))
            return E_FAIL;

        node->ToString(str, strSize);
        return S_OK;
    }

    #pragma prefast(suppress: 28718, "Unannotated buffer - Intentional, annotation is not a requirement")
    NATVIS_API HRESULT NatVis_BVSparse_JitArenaAllocator(
        const DWORD address,            // low 32-bits of address
        DEBUGHELPER *const debugHelper, // callback pointer to access helper functions
        const int base,                 // decimal or hex
        const BOOL uniStrings,          // not used
        char *const str,                // where the result needs to go
        const size_t strSize,           // how large the above buffer is
        const DWORD /* reserved */)     // always pass zero
    {
        byte bvData[sizeof(BVSparse<JitArenaAllocator>)];
        BVSparse<JitArenaAllocator> *const bv = reinterpret_cast<BVSparse<JitArenaAllocator> *>(bvData);
        if(!Read(debugHelper, debugHelper->GetRealAddress(debugHelper), bv))
            return E_FAIL;

        bv->ToString(
            str,
            strSize,
            [&](BVSparseNode *const nodePtr, bool *const successRef) -> BVSparseNode
            {
                Assert(nodePtr);
                Assert(successRef);

                byte nodeData[sizeof(BVSparseNode)];
                BVSparseNode *const node = reinterpret_cast<BVSparseNode *>(nodeData);
                *successRef = Read(debugHelper, reinterpret_cast<uint64>(nodePtr), node);
                return *node;
            });
        return S_OK;
    }
}
