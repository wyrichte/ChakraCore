// Copyright (C) Microsoft. All rights reserved. 

#include "StdAfx.h"

namespace Js
{
    CodeGenRecyclableData::CodeGenRecyclableData(const FunctionCodeGenJitTimeData *const jitTimeData) : jitTimeData(jitTimeData)
    {
        Assert(jitTimeData);
    }

    const FunctionCodeGenJitTimeData *CodeGenRecyclableData::JitTimeData() const
    {
        return jitTimeData;
    }
}
