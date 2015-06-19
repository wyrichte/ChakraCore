/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"

namespace Projection
{
#if _M_AMD64
    byte *StackVarArgReader::Read(int paramIndex, RtCONCRETETYPE paramType, StackVarArg* stackArgs)
    {
        bool fDouble = false;
        bool fFloat = false;
        if(ProjectionModel::BasicType::Is(paramType))
        {
            auto baseType = ProjectionModel::BasicType::From(paramType)->typeCor;
            if (baseType == ELEMENT_TYPE_R4)
            {
                fFloat = true;
            }
            else if(baseType == ELEMENT_TYPE_R8)
            {
                fDouble = true;
            }
        }

        return StackVarArgReader::Read(paramIndex, stackArgs, fDouble, fFloat);
    }

    byte *StackVarArgReader::Read(int paramIndex, StackVarArg* stackArgs, bool fDouble, bool fFloat)
    {
        byte* value = nullptr;

        // Always add 1 for This ptr
        paramIndex++;

        if(paramIndex < REG_PARAMS)
        {
            // Choose the parameter value according to parameter type. 
            // Double and float are special cases. 
            if(fDouble)
            {
                value = (byte*)&stackArgs->pFloatRegisters[paramIndex];
            }
            else if(fFloat)
            {
                value = (byte*)&stackArgs->pFloatRegisters[paramIndex];
            }

            if(value == nullptr)
            {
                // Any other type is passed as int
                value = (byte*)&stackArgs->pStack[paramIndex];
            }
        }
        else
        {
            // The rest of arguments are on the stack
            value = (byte*)&stackArgs->pStack[paramIndex];
        }

        return value;
    }
#elif defined _M_ARM
    using namespace ProjectionModel;

    byte *StackVarArgReader::Read(const ParameterLocation* loc, StackVarArg* callLayout)    // static.
    {
        // TODO: Projection/Evanesco: make sure that HFP data is filled at this point.
        Assert(loc);
        Assert(callLayout);
        return callLayout->GetParameterLocation(loc);
    }
#elif defined _M_ARM64
    using namespace ProjectionModel;

    byte *StackVarArgReader::Read(const ParameterLocation* loc, StackVarArg* callLayout)    // static.
    {
        // TODO: Projection/Evanesco: make sure that HFP data is filled at this point.
        Assert(loc);
        Assert(callLayout);
        return callLayout->GetParameterLocation(loc);
    }
#endif
}
