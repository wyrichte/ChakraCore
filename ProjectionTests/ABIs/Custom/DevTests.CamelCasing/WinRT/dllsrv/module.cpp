//  Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdafx.h>
#include <wrl\dllexports.h>
#include "DevTests.CamelCasing.h"

#include "StringVariations.h"
#include "SimpleNameCollisions.h"
#include "CrossMemberCollisions.h"

namespace DevTests
{
    namespace CamelCasing
    {
            // FUTURE: Once IDL extensions for Windows Runtime are available, this
            // can be defined as a cpp_quote of a constant in the IDL file.
            ActivatableClassWithFactory(StringVariationsServer, StringVariationsFactory)
            ActivatableClassWithFactory(OverloadStringVariationsServer, OverloadStringVariationsFactory)

            namespace SimpleNameCollisions
            {
                ActivatableClass(InternalConflictServer)
                ActivatableClass(ExternalConflictSameCaseServer)
                ActivatableClass(ExternalConflictDifferentCaseServer)
                ActivatableClass(InternalConflictWithExternalConflictServer)
                ActivatableClass(InternalConflictWithExternalConflict2Server)
                ActivatableClass(DoubleConflictServer)

                ActivatableClassWithFactory(StaticInternalConflictServer, StaticInternalConflictFactory)
                ActivatableClassWithFactory(StaticExternalConflictDifferentCaseServer, StaticExternalConflictDifferentCaseFactory)
            }

            namespace CrossMemberCollisions
            {
                ActivatableClass(BuiltInConflictsServer)
                ActivatableClassWithFactory(BuiltInConflictsStaticServer, BuiltInConflictsStaticFactory)
                ActivatableClassWithFactory(CamelLengthConflictServer, CamelLengthConflictFactory)
                ActivatableClassWithFactory(PascalLengthConflictServer, PascalLengthConflictFactory)
                ActivatableClass(VectorLengthConflictServer)
                ActivatableClass(InternalCrossMemberConflictServer)
                ActivatableClass(ExternalPropPropConflictServer)
                ActivatableClass(ExternalPropMethodConflictServer)
                ActivatableClass(ExternalMethodMethodConflictServer)
                ActivatableClass(ExternalMethodPropConflictServer)
                ActivatableClass(DoubleCrossMemberConflictServer)
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
