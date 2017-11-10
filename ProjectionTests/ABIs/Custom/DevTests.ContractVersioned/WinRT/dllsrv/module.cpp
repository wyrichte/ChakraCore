//  Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdafx.h>
#include <wrl\dllexports.h>
#include "DevTests.ContractVersioned.h"
#include "ContractVersioning.h"

namespace DevTests
{
    namespace ContractVersioned
    {
            // FUTURE: Once IDL extensions for Windows Runtime are available, this
            // can be defined as a cpp_quote of a constant in the IDL file.
            ActivatableClass(XyzServer)
            ActivatableClass(XyzPlatformVersionedServer)
    }
}
