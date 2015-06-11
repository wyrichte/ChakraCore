//----------------------------------------------------------------------------
// Copyright (C) 2008 - 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    JavascriptPixelArrayEnumerator::JavascriptPixelArrayEnumerator(JavascriptPixelArray* pixelArrayObjectIn) :
        JavascriptEnumerator(pixelArrayObjectIn->GetScriptContext()),
        pixelArrayObject(pixelArrayObjectIn)
    {
        // Must start at -1 so that the first iteration bumps it to zero
        index = -1;
    }

    Var JavascriptPixelArrayEnumerator::GetCurrentIndex()
    {
        ScriptContext *scriptContext = pixelArrayObject->GetScriptContext();

        if (index < pixelArrayObject->GetLengthAsSignedInt())
        {
            return scriptContext->GetIntegerString(index);
        }
        else
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }
    }

    BOOL JavascriptPixelArrayEnumerator::MoveNext(PropertyAttributes* attributes)
    {
        if (++index < pixelArrayObject->GetLengthAsSignedInt())
        {
            if (attributes != nullptr)
            {
                *attributes = PropertyEnumerable;
            }

            return true;
        }
        else
        {
            index = pixelArrayObject->GetLengthAsSignedInt();
            return false;
        }
    }
} 
