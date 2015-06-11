//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    class IWalkPropertyCallback abstract
    {
    public:
        virtual void OnBeginInstance(DynamicObject* dynamicObject) abstract;
        virtual void OnEndInstance(DynamicObject* dynamicObject) abstract;
        virtual void OnBeginPrototype(DynamicObject* dynamicObject, DynamicObject* psoPrototype) abstract;
        virtual void OnEndPrototype(DynamicObject* dynamicObject, DynamicObject* psoPrototype) abstract;
        virtual void OnProperty(PropertyId propertyId, Var aValue) abstract;
    };
}
