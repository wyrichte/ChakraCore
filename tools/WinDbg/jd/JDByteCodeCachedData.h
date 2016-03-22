//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

class JDByteCodeCachedData
{
public:
    JDByteCodeCachedData() : initialized(false) {}
    void Ensure();
    void Clear();

    ExtRemoteTyped layoutTable;
    ExtRemoteTyped extendedLayoutTable;
    ExtRemoteTyped attributesTable;
    ExtRemoteTyped extendedAttributesTable;

    int OpcodeAttr_OpHasMultiSizeLayout;
    int LayoutSize_SmallLayout;
    int LayoutSize_MediumLayout;
    int LayoutSize_LargeLayout;
    int TotalNumberOfBuiltInProperties;

    bool initialized;
};