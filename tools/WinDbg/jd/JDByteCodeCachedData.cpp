//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

#include "JDByteCodeCachedData.h"

void
JDByteCodeCachedData::Ensure()
{
    if (initialized)
    {
        return;
    }

    layoutTable = ReadTable<uint>(GetExtension()->FillModule("%s!Js::OpCodeUtil::OpCodeLayouts"));
    extendedLayoutTable = ReadTable<uint>(GetExtension()->FillModule("%s!Js::OpCodeUtil::ExtendedOpCodeLayouts"));
    if (GetExtension()->CanResolveSymbol(GetExtension()->FillModule("%s!TotalNumberOfBuiltInProperties")))
    {
        TotalNumberOfBuiltInProperties = JDRemoteTyped(GetExtension()->FillModule("%s!TotalNumberOfBuiltInProperties")).GetLong();
    }
    else
    {
        TotalNumberOfBuiltInProperties = JDRemoteTyped(GetExtension()->FillModule("(int)%s!_countJSOnlyProperty")).GetLong();
    }

    bool fSearchEnumOpHasMultiSizeLayout = false;
    if (GetExtension()->CanResolveSymbol(GetExtension()->FillModule("%s!OpcodeAttr::OpHasMultiSizeLayout")))
    {
        OpcodeAttr_OpHasMultiSizeLayout = JDRemoteTyped(GetExtension()->FillModule("%s!OpcodeAttr::OpHasMultiSizeLayout")).GetLong();
    }
    else
    {
        fSearchEnumOpHasMultiSizeLayout = true;
    }

    if (GetExtension()->CanResolveSymbol(GetExtension()->FillModule("%s!OpcodeAttr::OpcodeAttributes")))
    {
        attributesTable = ReadTable<int>(GetExtension()->FillModule("%s!OpcodeAttr::OpcodeAttributes"));
        extendedAttributesTable = ReadTable<int>(GetExtension()->FillModule("%s!OpcodeAttr::ExtendedOpcodeAttributes"));
        
        LayoutSize_SmallLayout = JDRemoteTyped(GetExtension()->FillModule("%s!Js::SmallLayout")).GetLong();
        LayoutSize_MediumLayout = JDRemoteTyped(GetExtension()->FillModule("%s!Js::MediumLayout")).GetLong();
        LayoutSize_LargeLayout = JDRemoteTyped(GetExtension()->FillModule("%s!Js::LargeLayout")).GetLong();
        // CanResolveSymbol doesn't work on enum. Need to guess that it is 3 here.
        char const * name = JDRemoteTyped(GetExtension()->FillModule("(%s!Js::LayoutSize)3")).GetEnumString();
        extendedOpCodesWith2Bytes = ENUM_EQUAL(name, LayoutCount);
    }
    else
    {
        // Dev12 tool set doesn't add namespace to statics and enum
        attributesTable = ReadTable<int>(GetExtension()->FillModule("%s!OpcodeAttributes"));
        extendedAttributesTable = ReadTable<int>(GetExtension()->FillModule("%s!ExtendedOpcodeAttributes"));

        // There are two enum of the same name OpCodeAttr and OpCodeAttrAsmJs versions.  Need to go thru them to find the one we want
        fSearchEnumOpHasMultiSizeLayout = true;

        LayoutSize_SmallLayout = JDRemoteTyped(GetExtension()->FillModule("%s!SmallLayout")).GetLong();
        LayoutSize_MediumLayout = JDRemoteTyped(GetExtension()->FillModule("%s!MediumLayout")).GetLong();
        LayoutSize_LargeLayout = JDRemoteTyped(GetExtension()->FillModule("%s!LargeLayout")).GetLong();
    }

    if (fSearchEnumOpHasMultiSizeLayout)
    {
        OpcodeAttr_OpHasMultiSizeLayout = 0;
        for (uint i = 0; i < 32; i++)
        {
            char const * name = JDRemoteTyped(GetExtension()->FillModule("(%s!OpCodeAttr::OpCodeAttrEnum)@$extin)"), (ULONG64)1 << i).GetEnumString();
            if (ENUM_EQUAL(name, OpHasMultiSizeLayout))
            {
                OpcodeAttr_OpHasMultiSizeLayout = 1 << i;
                break;
            }
        }
    }

    initialized = true;
}

void
JDByteCodeCachedData::Clear()
{
    if (initialized)
    {
        initialized = false;
        delete[] layoutTable;
        delete[] extendedLayoutTable;
        delete[] attributesTable;
        delete[] extendedAttributesTable;
    }
}
