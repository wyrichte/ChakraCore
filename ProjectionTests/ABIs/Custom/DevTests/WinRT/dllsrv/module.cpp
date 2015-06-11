//  Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdafx.h>
#include <wrl\dllexports.h>
#include "DevTests.h"

#include "DateTimeAndTimeSpan.h"
#include "Arm.h"
#include "SimpleTestNamespace.h"
#include "GCPressure.h"
#include "Versioning.h"
#include "Delegates.h"

namespace DevTests
{
    namespace DateTimeAndTimeSpan
    {
        // FUTURE: Once IDL extensions for Windows Runtime are available, this
        // can be defined as a cpp_quote of a constant in the IDL file.
        ActivatableClass(TestsServer)
    }

    namespace Arm
    {
        // FUTURE: Once IDL extensions for Windows Runtime are available, this
        // can be defined as a cpp_quote of a constant in the IDL file.
        ActivatableClass(ArmTestsServer)
    }

    namespace SimpleTestNamespace
    {
        ActivatableClass(SimpleClassServer)
    }

    namespace GCPressure
    {
        ActivatableClass(SmallClassServer)
        ActivatableClass(MediumClassServer)
        ActivatableClass(LargeClassServer)
    }

    namespace Versioning
    {
        ActivatableClassWithFactory(MinVersionClass, MinVersionFactory)
        ActivatableClassWithFactory(Win8Class, Win8Factory)
        ActivatableClassWithFactory(Win8SP1Class, Win8SP1Factory)
        ActivatableClassWithFactory(Win9Class, Win9Factory)
        ActivatableClass(MaxVersionClass)
        ActivatableClass(MarshalVersionedTypes)
        ActivatableClass(VectorInt)
        ActivatableClass(RequiresVectorInt)
        ActivatableClass(VersionedVectorInt)
        ActivatableClass(VectorVersionedT)
        ActivatableClass(ObservableVectorInt)
        ActivatableClass(RequiresObservableVectorInt)
        ActivatableClass(VersionedObservableVectorInt)
        ActivatableClass(ObservableVectorVersionedT)
    }

    namespace Delegates
    {
        ActivatableClass(TestClassServer)
        ActivatableClassWithFactory(StaticTestClassServer, MethodStaticsFactory)
    }
}

BOOL
WINAPI
QuirkIsEnabled(__in ULONG QuirkId)
{
    UNREFERENCED_PARAMETER(QuirkId);
    return FALSE;
}
