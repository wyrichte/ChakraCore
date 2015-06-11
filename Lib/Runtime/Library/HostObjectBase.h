//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    class HostObjectBase : public DynamicObject
    {
    public:
        virtual Js::ModuleRoot * GetModuleRoot(ModuleID moduleID) = 0; 

    protected:
        DEFINE_VTABLE_CTOR(HostObjectBase, DynamicObject);
        HostObjectBase(DynamicType * type) : DynamicObject(type) {};
    };
};