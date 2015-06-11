//---------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation.  All rights reserved.
//
// Macros to be used in TSGen
//---------------------------------------------------------------------------

#pragma once

#define ThrowIfFalse(condition, message) \
    if (!(condition)) { throw TSGenException(message); }

#define DeleteIfNotNull(alloc, objectPtr) \
    if(objectPtr) { Adelete(alloc, objectPtr); objectPtr = nullptr; }

#define DeleteListIfNotNull(alloc, listPtr, T) \
    if(listPtr) { listPtr->Iterate([&](T element){ Adelete(alloc, element); }); Adelete(alloc, listPtr); listPtr = nullptr; }