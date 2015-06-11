//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "stdafx.h"

__inline void js_memcpy_s(__bcount(sizeInBytes) void *dst, size_t sizeInBytes, __in_bcount(count) const void *src, size_t count)
{
    Assert((count) <= (sizeInBytes));
    if ((count) <= (sizeInBytes))
        memcpy((dst), (src), (count));
    else                              
        Js::Throw::FatalInternalError();   
}

__inline void js_wmemcpy_s(__ecount(sizeInWords) wchar_t *dst, size_t sizeInWords, __in_ecount(count) const wchar_t *src, size_t count)
{
    //Multiplication Overflow check
    Assert(count <= sizeInWords && count <= SIZE_MAX/sizeof(wchar_t));
    if(!(count <= sizeInWords && count <= SIZE_MAX/sizeof(wchar_t)))
    {
        Js::Throw::FatalInternalError();
    }

    memcpy(dst, src, count * sizeof(wchar_t));
}