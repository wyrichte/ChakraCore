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
    layoutTable = ext->FillModule("%s!Js::OpCodeUtil::OpCodeLayouts");
    extendedLayoutTable = ext->FillModule("%s!Js::OpCodeUtil::ExtendedOpCodeLayouts");
    TotalNumberOfBuiltInProperties = ExtRemoteTyped(ext->FillModule("%s!TotalNumberOfBuiltInProperties")).GetLong();

    try
    {
        attributesTable = ext->FillModule("%s!OpcodeAttr::OpcodeAttributes");
        extendedAttributesTable = ext->FillModule("%s!OpcodeAttr::ExtendedOpcodeAttributes");
        OpcodeAttr_OpHasMultiSizeLayout = ExtRemoteTyped(ext->FillModule("%s!OpcodeAttr::OpHasMultiSizeLayout")).GetLong();
        LayoutSize_SmallLayout = ExtRemoteTyped(ext->FillModule("%s!Js::SmallLayout")).GetLong();
        LayoutSize_MediumLayout = ExtRemoteTyped(ext->FillModule("%s!Js::MediumLayout")).GetLong();
        LayoutSize_LargeLayout = ExtRemoteTyped(ext->FillModule("%s!Js::LargeLayout")).GetLong();
    }
    catch (...)
    {
        // Dev12 tool set doesn't add namespace to statics and enum
        attributesTable = ext->FillModule("%s!OpcodeAttributes");
        extendedAttributesTable = ext->FillModule("%s!ExtendedOpcodeAttributes");

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
    initialized = false;
}

#endif