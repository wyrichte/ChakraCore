/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"

#ifdef MUTATORS

#include "initguid.h"

// {077B6E85-4DFF-4c4c-A616-528044B67AA3}
DEFINE_GUID(CLSID_MutatorsHostObject, 0x77b6e85, 0x4dff, 0x4c4c, 0xa6, 0x16, 0x52, 0x80, 0x44, 0xb6, 0x7a, 0xa3);

// {5540AC81-3DF8-442b-82A5-6DD37039D386}
DEFINE_GUID(IID_IMutatorsHost, 0x5540ac81, 0x3df8, 0x442b, 0x82, 0xa5, 0x6d, 0xd3, 0x70, 0x39, 0xd3, 0x86);



namespace Js
{
    SourceMutator::SourceMutator(ScriptContext* scriptContext_) : scriptContext(scriptContext_), pMHO(NULL)
    {
        // COM already initialized here

        if(CoCreateInstance(CLSID_MutatorsHostObject, NULL, CLSCTX_INPROC_SERVER, IID_IMutatorsHost, (void**)&pMHO) != S_OK)
        {
            Throw::FatalInternalError();
        }
    }

    LPCOLESTR SourceMutator::ModifySource(LPCOLESTR in)
    {
        LPCWSTR out;
        if(pMHO->ModifySource(in, &out) != S_OK)
        {
            Throw::FatalInternalError();
        }
        return out;
    }

    LPCOLESTR SourceMutator::ModifySourceWithMutators(LPCOLESTR src, LPCOLESTR mutators)
    {
        LPCWSTR out;
        if(pMHO->ModifySourceWithMutators(src, mutators, &out) != S_OK)
        {
            Throw::FatalInternalError();
        }
        return out;
    }
}
#endif