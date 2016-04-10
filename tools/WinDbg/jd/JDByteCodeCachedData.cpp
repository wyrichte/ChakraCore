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

    EXT_CLASS_BASE * ext = GetExtension();
    
    layoutTable = ReadTable<uint>(ExtRemoteTyped(ext->FillModule("%s!Js::OpCodeUtil::OpCodeLayouts")));
    extendedLayoutTable = ReadTable<uint>(ExtRemoteTyped(ext->FillModule("%s!Js::OpCodeUtil::ExtendedOpCodeLayouts")));
    if (ext->CanResolveSymbol(ext->FillModule("%s!TotalNumberOfBuiltInProperties")))
    {
        TotalNumberOfBuiltInProperties = ExtRemoteTyped(ext->FillModule("%s!TotalNumberOfBuiltInProperties")).GetLong();
    }
    else
    {
        TotalNumberOfBuiltInProperties = ExtRemoteTyped(ext->FillModule("(int)%s!_countJSOnlyProperty")).GetLong();
    }

    if (ext->CanResolveSymbol(ext->FillModule("%s!OpcodeAttr::OpcodeAttributes")))
    {
        attributesTable = ReadTable<int>(ExtRemoteTyped(ext->FillModule("%s!OpcodeAttr::OpcodeAttributes")));
        extendedAttributesTable = ReadTable<int>(ExtRemoteTyped(ext->FillModule("%s!OpcodeAttr::ExtendedOpcodeAttributes")));
        OpcodeAttr_OpHasMultiSizeLayout = ExtRemoteTyped(ext->FillModule("%s!OpcodeAttr::OpHasMultiSizeLayout")).GetLong();
        LayoutSize_SmallLayout = ExtRemoteTyped(ext->FillModule("%s!Js::SmallLayout")).GetLong();
        LayoutSize_MediumLayout = ExtRemoteTyped(ext->FillModule("%s!Js::MediumLayout")).GetLong();
        LayoutSize_LargeLayout = ExtRemoteTyped(ext->FillModule("%s!Js::LargeLayout")).GetLong();
    }
    else
    {
        // Dev12 tool set doesn't add namespace to statics and enum
        attributesTable = ReadTable<int>(ext->FillModule("%s!OpcodeAttributes"));
        extendedAttributesTable = ReadTable<int>(ext->FillModule("%s!ExtendedOpcodeAttributes"));

        // There are two enum of the same name OpCodeAttr and OpCodeAttrAsmJs versions.  Need to go thru them to find the one we want
        OpcodeAttr_OpHasMultiSizeLayout = 0;
        for (uint i = 0; i < 32; i++)
        {
            char * name = JDUtil::GetEnumString(ExtRemoteTyped(ext->FillModule("(%s!OpCodeAttr::OpCodeAttrEnum)@$extin)"), (ULONG64)1 << i));
            if (ENUM_EQUAL(name, OpHasMultiSizeLayout))
            {
                OpcodeAttr_OpHasMultiSizeLayout = 1 << i;
                break;
            }
        }
        LayoutSize_SmallLayout = ExtRemoteTyped(ext->FillModule("%s!SmallLayout")).GetLong();
        LayoutSize_MediumLayout = ExtRemoteTyped(ext->FillModule("%s!MediumLayout")).GetLong();
        LayoutSize_LargeLayout = ExtRemoteTyped(ext->FillModule("%s!LargeLayout")).GetLong();
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