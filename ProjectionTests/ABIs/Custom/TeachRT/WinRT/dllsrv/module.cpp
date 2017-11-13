//  Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdafx.h>
#include <wrl\dllexports.h>
#include "Fabrikam.h"

#include "ToasterServer.h"
#include "ChefServer.h"
#include "OvenServer.h"
#include "KitchenServer.h"

namespace Fabrikam
{
    namespace Kitchen
    {
        ActivatableClass(ToasterServer)
        ActivatableClassWithFactory(ChefServer, ChefFactory)
        ActivatableClass(OvenServer)
        ActivatableClass(KitchenServer)
    }
}

BOOL
WINAPI
QuirkIsEnabled(__in ULONG QuirkId)
{
    UNREFERENCED_PARAMETER(QuirkId);
    return FALSE;
}
