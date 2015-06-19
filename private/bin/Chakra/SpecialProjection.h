//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------
#pragma once

namespace Projection
{
    struct ThisInfo;

    struct VectorProjectionFunctions
    {
        Js::JavascriptWinRTFunction * getAtFunction;
        Js::JavascriptWinRTFunction * setAtFunction;
        Js::JavascriptWinRTFunction * appendFunction;
        Js::JavascriptWinRTFunction * removeAtEndFunction;

        VectorProjectionFunctions() : getAtFunction(nullptr), setAtFunction(nullptr), appendFunction(nullptr), removeAtEndFunction(nullptr)
        {
        }
    };

    struct SpecialProjection
    {
        ThisInfo *thisInfo;
        ProjectionContext *projectionContext;
        Js::DynamicObject *prototypeObject;
        VectorProjectionFunctions * abiFunctions;
        SpecialProjection(ThisInfo *thisInfo, Js::DynamicObject *prototypeObject, ProjectionContext *projectionContext, VectorProjectionFunctions * abiFunctions)
            : thisInfo(thisInfo), prototypeObject(prototypeObject), projectionContext(projectionContext), abiFunctions(abiFunctions)
        {
            Assert(thisInfo);
            Assert(prototypeObject);
            Assert(projectionContext);
        }
    };
}