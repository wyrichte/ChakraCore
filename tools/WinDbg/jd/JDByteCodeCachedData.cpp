//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

#ifdef JD_PRIVATE
#include "JDByteCodeCachedData.h"

void
JDByteCodeCachedData::Ensure()
{
    if (initialized)
    {
        return;
    }

    layoutTable = ReadTable<uint>(ExtRemoteTyped(GetExtension()->FillModule("%s!Js::OpCodeUtil::OpCodeLayouts")));
    extendedLayoutTable = ReadTable<uint>(ExtRemoteTyped(GetExtension()->FillModule("%s!Js::OpCodeUtil::ExtendedOpCodeLayouts")));
    if (GetExtension()->CanResolveSymbol(GetExtension()->FillModule("%s!TotalNumberOfBuiltInProperties")))
    {
        TotalNumberOfBuiltInProperties = ExtRemoteTyped(GetExtension()->FillModule("%s!TotalNumberOfBuiltInProperties")).GetLong();
    }
    else
    {
        TotalNumberOfBuiltInProperties = ExtRemoteTyped(GetExtension()->FillModule("(int)%s!_countJSOnlyProperty")).GetLong();
    }

    bool fSearchEnumOpHasMultiSizeLayout = false;
    if (GetExtension()->CanResolveSymbol(GetExtension()->FillModule("%s!OpcodeAttr::OpHasMultiSizeLayout")))
    {
        OpcodeAttr_OpHasMultiSizeLayout = ExtRemoteTyped(GetExtension()->FillModule("%s!OpcodeAttr::OpHasMultiSizeLayout")).GetLong();
    }
    else
    {
        fSearchEnumOpHasMultiSizeLayout = true;
    }

    if (GetExtension()->CanResolveSymbol(GetExtension()->FillModule("%s!OpcodeAttr::OpcodeAttributes")))
    {
        attributesTable = ReadTable<int>(ExtRemoteTyped(GetExtension()->FillModule("%s!OpcodeAttr::OpcodeAttributes")));
        extendedAttributesTable = ReadTable<int>(ExtRemoteTyped(GetExtension()->FillModule("%s!OpcodeAttr::ExtendedOpcodeAttributes")));
        
        LayoutSize_SmallLayout = ExtRemoteTyped(GetExtension()->FillModule("%s!Js::SmallLayout")).GetLong();
        LayoutSize_MediumLayout = ExtRemoteTyped(GetExtension()->FillModule("%s!Js::MediumLayout")).GetLong();
        LayoutSize_LargeLayout = ExtRemoteTyped(GetExtension()->FillModule("%s!Js::LargeLayout")).GetLong();
        // CanResolveSymbol doesn't work on enum. Need to guess that it is 3 here.
        char * name = JDUtil::GetEnumString(ExtRemoteTyped(GetExtension()->FillModule("(%s!Js::LayoutSize)3")));
        extendedOpCodesWith2Bytes = ENUM_EQUAL(name, LayoutCount);
    }
    else
    {
        // Dev12 tool set doesn't add namespace to statics and enum
        attributesTable = ReadTable<int>(GetExtension()->FillModule("%s!OpcodeAttributes"));
        extendedAttributesTable = ReadTable<int>(GetExtension()->FillModule("%s!ExtendedOpcodeAttributes"));

        // There are two enum of the same name OpCodeAttr and OpCodeAttrAsmJs versions.  Need to go thru them to find the one we want
        fSearchEnumOpHasMultiSizeLayout = true;

        LayoutSize_SmallLayout = ExtRemoteTyped(GetExtension()->FillModule("%s!SmallLayout")).GetLong();
        LayoutSize_MediumLayout = ExtRemoteTyped(GetExtension()->FillModule("%s!MediumLayout")).GetLong();
        LayoutSize_LargeLayout = ExtRemoteTyped(GetExtension()->FillModule("%s!LargeLayout")).GetLong();
    }

    if (fSearchEnumOpHasMultiSizeLayout)
    {
        OpcodeAttr_OpHasMultiSizeLayout = 0;
        for (uint i = 0; i < 32; i++)
        {
            char * name = JDUtil::GetEnumString(ExtRemoteTyped(GetExtension()->FillModule("(%s!OpCodeAttr::OpCodeAttrEnum)@$extin)"), (ULONG64)1 << i));
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

#endif