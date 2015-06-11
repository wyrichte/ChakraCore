/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"

//----------------------------------------
// Default operator new/delete overrides
//----------------------------------------

void * __cdecl
operator new(size_t byteSize)
{
    return HeapNewNoThrowArray(char, byteSize);
}

void * __cdecl
operator new[](size_t byteSize)
{
    return HeapNewNoThrowArray(char, byteSize);
}

void __cdecl
operator delete(void * obj)
{
    HeapAllocator::Instance.Free(obj, (size_t)-1);
}

void __cdecl
operator delete[](void * obj)
{
    HeapAllocator::Instance.Free(obj, (size_t)-1);
}