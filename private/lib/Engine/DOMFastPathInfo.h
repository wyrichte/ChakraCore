//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------
//  Implements simple, fast property getter/setter for DOM object.
//----------------------------------------------------------------------------
#pragma once

#ifdef ENABLE_DOM_FAST_PATH

class DOMFastPathInfo
{
public:
    static Js::FunctionInfo objectGetterTable[];
    static Js::FunctionInfo objectSetterTable[];
    static Js::FunctionInfo* GetObjectGetterInfo(unsigned int slotIndex) { return &(objectGetterTable[slotIndex]); }
    static Js::FunctionInfo* GetObjectSetterInfo(unsigned int slotIndex) { return &(objectSetterTable[slotIndex]); }
    static IR::JnHelperMethod const objectGetterHelperIDTable[];
    static IR::JnHelperMethod const objectSetterHelperIDTable[];
    static IR::JnHelperMethod GetObjectGetterIRHelper(unsigned int slotIndex) { return objectGetterHelperIDTable[slotIndex]; }
    static IR::JnHelperMethod GetObjectSetterIRHelper(unsigned int slotIndex) { return objectSetterHelperIDTable[slotIndex]; }

    static Js::FunctionInfo typeGetterTable[];
    static Js::FunctionInfo typeSetterTable[];
    static Js::FunctionInfo* GetTypeGetterInfo(unsigned int slotIndex) { return &(typeGetterTable[slotIndex]); }
    static Js::FunctionInfo* GetTypeSetterInfo(unsigned int slotIndex) { return &(typeSetterTable[slotIndex]); }
    static IR::JnHelperMethod const typeGetterHelperIDTable[];
    static IR::JnHelperMethod const typeSetterHelperIDTable[];
    static IR::JnHelperMethod GetTypeGetterIRHelper(unsigned int slotIndex) { return typeGetterHelperIDTable[slotIndex]; }
    static IR::JnHelperMethod GetTypeSetterIRHelper(unsigned int slotIndex) { return typeSetterHelperIDTable[slotIndex]; }

    static Js::Var __cdecl CrossSiteSimpleSlotAccessorThunk(Js::RecyclableObject*, Js::CallInfo, ...);
#if DBG
    static bool VerifyObjectSize(Js::RecyclableObject* obj, size_t objSize);
#endif
};

#endif


