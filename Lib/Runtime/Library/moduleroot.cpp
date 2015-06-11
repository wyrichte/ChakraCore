//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    ModuleRoot::ModuleRoot(DynamicType * type):
        RootObjectBase(type)
    {
    }

    void ModuleRoot::SetHostObject(ModuleID moduleID, HostObjectBase * hostObject)
    {
        this->moduleID = moduleID;
        __super::SetHostObject(hostObject);
    }
}
