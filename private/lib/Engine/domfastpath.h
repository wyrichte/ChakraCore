//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------
//  Implements simple, fast property getter/setter for DOM object.
//----------------------------------------------------------------------------

#include "edgescriptdirect.h"

#pragma once

#define DECLARE_SIMPLEGETTER_INFO(kind, x) Js::FunctionInfo DOMFastPath<x>::EntryInfo::Simple##kind##SlotGetter(FORCE_NO_WRITE_BARRIER_TAG(DOMFastPath<x>::EntrySimple##kind##SlotGetter),  \
        (Js::FunctionInfo::Attributes)(Js::FunctionInfo::NeedCrossSiteSecurityCheck | Js::FunctionInfo::HasNoSideEffect | Js::FunctionInfo::CanBeHoisted | Js::FunctionInfo::BuiltInInlinableAsLdFldInlinee), \
        Js::JavascriptBuiltInFunction::DOMFastPathGetter);

#define DECLARE_SIMPLESETTER_INFO(kind, x) Js::FunctionInfo DOMFastPath<x>::EntryInfo::Simple##kind##SlotSetter(FORCE_NO_WRITE_BARRIER_TAG(DOMFastPath<x>::EntrySimple##kind##SlotSetter),  \
        (Js::FunctionInfo::Attributes)(Js::FunctionInfo::NeedCrossSiteSecurityCheck | Js::FunctionInfo::BuiltInInlinableAsLdFldInlinee), Js::JavascriptBuiltInFunction::DOMFastPathSetter);

#define DECLARE_SIMPLEACCESSOR_INFO(kind, x) \
    DECLARE_SIMPLESETTER_INFO(kind, x) \
    DECLARE_SIMPLEGETTER_INFO(kind, x) \

#define DECLARE_TEN_SIMPLEACCESS_INFO(kind, x) \
    DECLARE_SIMPLEACCESSOR_INFO(kind, x##0) \
    DECLARE_SIMPLEACCESSOR_INFO(kind, x##1) \
    DECLARE_SIMPLEACCESSOR_INFO(kind, x##2) \
    DECLARE_SIMPLEACCESSOR_INFO(kind, x##3) \
    DECLARE_SIMPLEACCESSOR_INFO(kind, x##4) \
    DECLARE_SIMPLEACCESSOR_INFO(kind, x##5) \
    DECLARE_SIMPLEACCESSOR_INFO(kind, x##6) \
    DECLARE_SIMPLEACCESSOR_INFO(kind, x##7) \
    DECLARE_SIMPLEACCESSOR_INFO(kind, x##8) \
    DECLARE_SIMPLEACCESSOR_INFO(kind, x##9)

#define DECLARE_OBJECT_SIMPLEACCESS_INFO \
    DECLARE_SIMPLEACCESSOR_INFO(Object, 0) \
    DECLARE_SIMPLEACCESSOR_INFO(Object, 1) \
    DECLARE_SIMPLEACCESSOR_INFO(Object, 2) \
    DECLARE_SIMPLEACCESSOR_INFO(Object, 3) \
    DECLARE_SIMPLEACCESSOR_INFO(Object, 4) \
    DECLARE_SIMPLEACCESSOR_INFO(Object, 5) \
    DECLARE_SIMPLEACCESSOR_INFO(Object, 6) \
    DECLARE_SIMPLEACCESSOR_INFO(Object, 7) \
    DECLARE_SIMPLEACCESSOR_INFO(Object, 8) \
    DECLARE_SIMPLEACCESSOR_INFO(Object, 9) \
    DECLARE_TEN_SIMPLEACCESS_INFO(Object, 1) \

#define DECLARE_TYPE_SIMPLEACCESS_INFO \
    DECLARE_SIMPLEACCESSOR_INFO(Type, 0) \
    DECLARE_SIMPLEACCESSOR_INFO(Type, 1) \
    DECLARE_SIMPLEACCESSOR_INFO(Type, 2) \
    DECLARE_SIMPLEACCESSOR_INFO(Type, 3) \

template <unsigned int slotIndex>
class DOMFastPath
{
public:
    class EntryInfo
    {
    public:
        static Js::FunctionInfo SimpleObjectSlotSetter;
        static Js::FunctionInfo SimpleObjectSlotGetter;
        static Js::FunctionInfo SimpleTypeSlotSetter;
        static Js::FunctionInfo SimpleTypeSlotGetter;
    };

    static Js::Var EntrySimpleObjectSlotGetter(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntrySimpleObjectSlotSetter(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntrySimpleTypeSlotGetter(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntrySimpleTypeSlotSetter(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
};
