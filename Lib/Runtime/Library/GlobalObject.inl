//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    inline bool GlobalObject::Is(Var aValue)
    {
        return RecyclableObject::Is(aValue) && (RecyclableObject::FromVar(aValue)->GetTypeId() == TypeIds_GlobalObject);
    }

    inline GlobalObject* GlobalObject::FromVar(Var aValue)
    {
        AssertMsg(Is(aValue), "Ensure var is actually a 'GlobalObject'");
        return static_cast<GlobalObject*>(aValue);
    }
}
