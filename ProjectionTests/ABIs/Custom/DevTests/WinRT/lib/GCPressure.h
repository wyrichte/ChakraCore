//  Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdafx.h>
#include "DevTests.h"

namespace DevTests
{
    namespace GCPressure
    {
        class SmallClassServer :
            public Microsoft::WRL::RuntimeClass<IEmptyInterface>
        {
            InspectableClass(L"DevTests.GCPressure.SmallClass", BaseTrust);
        };

        class MediumClassServer :
            public Microsoft::WRL::RuntimeClass<IEmptyInterface>
        {
            InspectableClass(L"DevTests.GCPressure.MediumClass", BaseTrust);
        };

        class LargeClassServer :
            public Microsoft::WRL::RuntimeClass<IEmptyInterface>
        {
            InspectableClass(L"DevTests.GCPressure.LargeClass", BaseTrust);
        };
    }
}
