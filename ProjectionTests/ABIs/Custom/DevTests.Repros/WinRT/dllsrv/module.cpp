//  Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdafx.h>
#include <wrl\dllexports.h>
#include "DevTests.Repros.h"

#include "WebUIRepro.h"
#include "PerformanceRepro.h"
#include "InterfaceOutRepro.h"
#include "VersionedPropertiesRepro.h"
#include "Stringables.h"

namespace DevTests
{
    namespace Repros
    {
            // FUTURE: Once IDL extensions for Windows Runtime are available, this
            // can be defined as a cpp_quote of a constant in the IDL file.

        namespace WebUI
        {
            ActivatableClassWithFactory(WebUIActivationServer, WebUIActivationFactory)
        }

        namespace Performance
        {
            ActivatableClass(RefClass)
        }

        namespace InterfaceOutFastPath
        {
            ActivatableClassWithFactory(TestsServer, TestsFactory)
        }

        namespace VersionedProperties
        {
            ActivatableClassWithFactory(VersionedProperty, VersionedPropertyFactory)
            ActivatableClass(ConflictWithVersionedProperty)
            ActivatableClass(ReadOnlyVersionedProperty)
        }

        namespace Stringables
        {
            ActivatableClass(SimpleStringable)
        }
    }
}

BOOL
WINAPI
QuirkIsEnabled(__in ULONG QuirkId)
{
    UNREFERENCED_PARAMETER(QuirkId);
    return FALSE;
}
