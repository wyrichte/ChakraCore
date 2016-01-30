//  Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdafx.h>
#include <wrl\dllexports.h>
#include "Animals.h"
#include "AnimalServer.h"
#include "FishServer.h"
#include "DinoServer.h"
#include "CollectionsServer.h"
#include "InterfaceWithEventServer.h"
#include "PropertyValueTests.h"
#include "HiddenServer.h"
#include "VariableProjectionServer.h"


namespace Animals
{
            // FUTURE: Once IDL extensions for Windows Runtime are available, this
            // can be defined as a cpp_quote of a constant in the IDL file.
            ActivatableClassWithFactory(AnimalServer, AnimalFactory)
            ActivatableClass(SimplestClassServer)
            ActivatableClass(FishServer)
            ActivatableClass(TurkeyServer)
            ActivatableClass(ElephantServer)
            ActivatableClass(HiddenClassServer)
            ActivatableClass(VisibleClassWithHiddenInterfaceOnlyServer)
            ActivatableClass(VisibleClassWithDefaultHiddenInterfaceServer)
            ActivatableClass(VisibleClassWithDefaultVisibleInterfaceServer)
            ActivatableClassWithFactory(VisibleClassWithVisibleInterfaceAndHiddenStaticInterfaceServer, VisibleClassWithVisibleInterfaceAndHiddenStaticInterfaceFactory)
            ActivatableClassWithFactory(DinoServer, DinoFactory)
            ActivatableClassWithFactory(PomapoodleServer, PomapoodleFactory)
            ActivatableClassWithFactory(EmptyClassServer, EmptyFactory)
            ActivatableClass(SingleIVectorServer)
            ActivatableClass(DoubleIVectorServer)
            ActivatableClass(MultipleIVectorServer)
            ActivatableClass(InterfaceWithSingleIVectorServer)
            ActivatableClass(InterfaceWithDoubleIVectorServer)
            ActivatableClass(InterfaceWithMultipleIVectorServer)
            ActivatableClass(RCIObservableServer)
            ActivatableClass(RCISingleObservableServer)
            ActivatableClass(RCIDoubleObservableServer)
            ActivatableClass(RCIDoubleObservableMapServer)
            ActivatableClass(RC1WithEventServer)
            ActivatableClassWithFactory(RC2WithEventServer, RC2WithEventFactory)
            ActivatableClass(RC3WithEventServer)
            ActivatableClass(RC4WithEventServer)
            ActivatableClassWithFactory(RC5WithEventServer, RC5WithEventFactory)
            ActivatableClass(RC6WithEventServer)
            ActivatableClass(RC7WithEventServer)
            ActivatableClass(RC8WithEventServer)
            ActivatableClass(CPropertyValueTests)
            ActivatableClass(CRCPropertyValue1)
            ActivatableClass(CRCPropertyValue2)
            ActivatableClass(CRCPropertyValue3)
            ActivatableClass(CRCPropertyValue4)
            ActivatableClass(CRCPropertyValue5)
            ActivatableClass(CRCPropertyValue6)
            ActivatableClass(RCStringMapServer)
            ActivatableClass(RCStringMapWithIterableServer)
            ActivatableClass(RCStringMapWithDefaultIterableServer)
            
            
            namespace VariableProjection
            {
                ActivatableClass(TestingClass)
                ActivatableClass(PartialAndMissingInterfaceClass)
                ActivatableClass(ExtendsInterfaceClass)
                ActivatableClass(MissingAndPartialInterfaceClass)
                ActivatableClass(ExtendsReverseInterfaceClass)
            }
            
}

BOOL
WINAPI
QuirkIsEnabled(__in ULONG QuirkId)
{
    UNREFERENCED_PARAMETER(QuirkId);
    return FALSE;
}
