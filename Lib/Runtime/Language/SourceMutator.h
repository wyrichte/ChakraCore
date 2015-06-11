/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once

#ifdef MUTATORS
#include "edgemutatorshost.h"

namespace Js
{
    class ScriptContext;
    class SourceMutator
    {
        ScriptContext *scriptContext;
        IMutatorsHost *pMHO;
    public:
        SourceMutator(ScriptContext *scriptContext_);
        LPCOLESTR ModifySource(LPCOLESTR in);
        LPCOLESTR ModifySourceWithMutators(LPCOLESTR src, LPCOLESTR mutators);
    };
}
#endif