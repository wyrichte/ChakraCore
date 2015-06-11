//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

extern const int MachPtr;

enum IRType : BYTE
{
#define IRTYPE(Name, BaseType, Bytes, Align, Bits, EnRegOk, DumpName)  Ty ## Name,
#include "IRTypeList.h"
#undef IRTYPE

};

enum IRBaseType : BYTE
{
#define IRBASETYPE(Name, BaseType, Bytes, Align, Bits, EnRegOk, DumpName)  TyBase ## Name,
#include "IRBaseTypeList.h"
#undef IRTYPE

};

extern int const TySize[];

extern bool IRType_IsSignedInt(IRType type);
extern bool IRType_IsUnsignedInt(IRType type);
extern bool IRType_IsFloat(IRType type);
extern bool IRType_IsNative(IRType type);
extern bool IRType_IsNativeInt(IRType type);

#ifdef SIMD_JS_ENABLED
extern bool IRType_IsSimd(IRType type);
extern bool IRType_IsSimd128(IRType type);
#endif

#if DBG_DUMP || defined(ENABLE_IR_VIEWER)
extern void IRType_Dump(IRType type);
#endif
