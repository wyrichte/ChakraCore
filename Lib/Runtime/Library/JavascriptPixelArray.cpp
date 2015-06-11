//----------------------------------------------------------------------------
// Copyright (C) 2008 - 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "JavascriptPixelArray.h"

namespace Js
{
    // Limit the size to 1GB to stay away from integer overflow conditions.
    // A single 1900x1200 32bpp bitmap takes less than 10MB, so this should be plenty of room.
    
    const uint32 JavascriptPixelArray::MaxPixelArrayLength = 1 << 30;   

    JavascriptPixelArray::JavascriptPixelArray(DynamicType * type) : DynamicObject(type), buffer(null), bufferlength(0) { }

    JavascriptPixelArray::JavascriptPixelArray(uint32 length, DynamicType * type)
        : DynamicObject(type)
    {                         
        if ((length % 4) != 0)
        {
            // The length must be a multiple of 4 because the array stores RGBA quads.
            // The code doesn't have any hard dependency on this, but enforcing it reduces the 
            // surface area for problems
            
            JavascriptError::ThrowTypeError(GetScriptContext(), JSERR_FunctionArgument_Invalid);
        }       
        else if (length > MaxPixelArrayLength)
        {
            JavascriptError::ThrowTypeError(GetScriptContext(), JSERR_FunctionArgument_Invalid);
        }            
        else if (length > 0)
        {            
            buffer = (BYTE *) malloc(length);               

            if (buffer == null)
            {
                JavascriptError::ThrowOutOfMemoryError(GetScriptContext());
            }

            bufferlength = length;
            ZeroMemory(buffer, bufferlength);            
        }
        else
        {
            // The requested length will be zero for objects created for the type system
            buffer = null;
            bufferlength = 0;
        }
    }

    void JavascriptPixelArray::Dispose(bool isShutdown)
    {
        if (buffer != null)
        {
            free(buffer);
            buffer = null;
        }        
    }

    DynamicObject* JavascriptPixelArray::MakeCopyOnWriteObject(ScriptContext* scriptContext)
    {
        VERIFY_COPY_ON_WRITE_ENABLED_RET();

        Recycler* recycler = scriptContext->GetRecycler();
        CopyOnWriteObject<JavascriptPixelArray> *result = RecyclerNew(recycler, CopyOnWriteObject<JavascriptPixelArray>, scriptContext->GetLibrary()->GetPixelArrayType(), this, scriptContext);
        result->bufferlength = bufferlength;

        if (bufferlength > 0)
        {
            result->buffer = (BYTE *)malloc(bufferlength);
            js_memcpy_s(result->buffer, bufferlength, buffer, bufferlength);

            // Force the pixel array to detach now so that the GetItem() doesn't diverge when DirectSetItem() is used.
            result->Detach();
        }
        else
            result->buffer = null;

        return result;
    }

    Var JavascriptPixelArray::NewInstance(RecyclableObject* function, CallInfo callInfo, ...)
    {
        // Other browsers do not support creating a CanvasPixelArray directly from script               
        JavascriptError::ThrowTypeError(function->GetScriptContext(), VBSERR_ActionNotSupported);
    }

    void JavascriptPixelArray::DirectSetItem(JavascriptPixelArray *array, uint32 index, Var value)
    {
        array->DirectSetItem(index, value);
    }

    
}



