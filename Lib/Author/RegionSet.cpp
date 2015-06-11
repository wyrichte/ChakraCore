//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

namespace Authoring
{
    TYPE_STATS(RegionSet, L"RegionSet")

    void RegionSet::Add(long offset, long length)
    {
        AuthorFileRegion region;
        region.offset = offset;
        region.length = length;
        m_regions.Add(region);
    }

    STDMETHODIMP RegionSet::get_Count(int *result)
    {
        if (result) *result = m_regions.Count();
        return S_OK;
    }

    STDMETHODIMP RegionSet::GetItems(int startIndex, int count, AuthorFileRegion *regions)
    {
        STDMETHOD_PREFIX;

        ValidateArg(startIndex + count <= m_regions.Count());

        for (int i = 0; i < count; i++)
        {
            int sourceIndex = startIndex + i; 
            regions[i] = m_regions.Item(sourceIndex);
        }
        
        STDMETHOD_POSTFIX;
    }

}
