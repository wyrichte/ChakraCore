//----------------------------------------------------------------------------
// Copyright (C) by Microsoft Corporation.  All rights reserved.
// 
//  Implements simple, fast property getter/setter for DOM object.
//----------------------------------------------------------------------------
#pragma once

class DOMFastPathInfo
{
public:
    static Js::FunctionInfo getterTable[];
    static Js::FunctionInfo setterTable[];
    static Js::FunctionInfo* GetGetterInfo(unsigned int slotIndex) { return &(getterTable[slotIndex]); }
    static Js::FunctionInfo* GetSetterInfo(unsigned int slotIndex) { return &(setterTable[slotIndex]); }
    static IR::JnHelperMethod getterHelperIDTable[];
    static IR::JnHelperMethod setterHelperIDTable[];
    static IR::JnHelperMethod GetGetterIRHelper(unsigned int slotIndex) { return getterHelperIDTable[slotIndex]; }
    static IR::JnHelperMethod GetSetterIRHelper(unsigned int slotIndex) { return setterHelperIDTable[slotIndex]; }

    static Js::Var __cdecl CrossSiteSimpleSlotAccessorThunk(Js::RecyclableObject*, Js::CallInfo, ...);
#if DBG
    static bool VerifyObjectSize(Js::RecyclableObject* obj, size_t objSize);
#endif
};


