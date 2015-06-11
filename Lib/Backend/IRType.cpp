//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "BackEnd.h"

int const TySize[] = {
#define IRTYPE(ucname, baseType, size, align, bitSize, enRegOk, dname) size,
#include "IRTypeList.h"
#undef IRTYPE
};

enum IRBaseTypes : BYTE {
    IRBaseType_Illegal,
    IRBaseType_Int,
    IRBaseType_Uint,
    IRBaseType_Float,
#ifdef SIMD_JS_ENABLED
    IRBaseType_Simd,
#endif
    IRBaseType_Var,
    IRBaseType_Condcode,
    IRBaseType_Misc
};

int const TyBaseType[] = {
#define IRTYPE(ucname, baseType, size, align, bitSize, enRegOk, dname) IRBaseType_ ## baseType,
#include "IRTypeList.h"
#undef IRTYPE
};

wchar_t * const TyDumpName[] = {
#define IRTYPE(ucname, baseType, size, align, bitSize, enRegOk, dname) L# dname,
#include "IRTypeList.h"
#undef IRTYPE
};

bool IRType_IsSignedInt(IRType type) { return TyBaseType[type] == IRBaseType_Int; }
bool IRType_IsUnsignedInt(IRType type) { return TyBaseType[type] == IRBaseType_Uint; }
bool IRType_IsFloat(IRType type) { return TyBaseType[type] == IRBaseType_Float; }
bool IRType_IsNative(IRType type) 
{ 
    return TyBaseType[type] > IRBaseType_Illegal && TyBaseType[type] < IRBaseType_Var; 
}
bool IRType_IsNativeInt(IRType type)
{
    return TyBaseType[type] > IRBaseType_Illegal && TyBaseType[type] < IRBaseType_Float; 
}

#ifdef SIMD_JS_ENABLED
bool IRType_IsSimd(IRType type)
{
    return TyBaseType[type] == IRBaseType_Simd;
}

bool IRType_IsSimd128(IRType type)
{
    return type == TySimd128;
}
#endif
#if DBG_DUMP || defined(ENABLE_IR_VIEWER)
void IRType_Dump(IRType type)
{
    Output::Print(TyDumpName[type]);
}
#endif