//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

class JDByteCodeCachedData
{
public:
    JDByteCodeCachedData() : initialized(false) {}
    ~JDByteCodeCachedData() { Clear(); }
    void Ensure();
    void Clear();

    uint * layoutTable;
    uint * extendedLayoutTable;
    int * attributesTable;
    int * extendedAttributesTable;

    int OpcodeAttr_OpHasMultiSizeLayout;
    int LayoutSize_SmallLayout;
    int LayoutSize_MediumLayout;
    int LayoutSize_LargeLayout;
    int TotalNumberOfBuiltInProperties;

    bool initialized;

private:
    template <typename T>
    static T * ReadTable(ExtRemoteTyped remoteTyped)
    {
        ULONG size = remoteTyped.GetTypeSize();
        ULONG count = size / sizeof(T);
        T * table = new T[count];
        remoteTyped.ReadBuffer(table, count * sizeof(T), true);
        return table;
    }
};