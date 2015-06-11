//
//    Copyright (C) Microsoft.  All rights reserved.
//
namespace Authoring
{
    class RegionSet : public SimpleComObjectWithAlloc<IAuthorRegionSet>
    {
    private:
        typedef JsUtil::List<AuthorFileRegion, ArenaAllocator> Regions;

        Regions m_regions;

    public:
        RegionSet(PageAllocator* pageAlloc) : SimpleComObjectWithAlloc<IAuthorRegionSet>(pageAlloc, L"ls: RegionSet"), m_regions(Alloc())  { }

        void Add(long offset, long length);
        STDMETHOD(get_Count)(int *result);
        STDMETHOD(GetItems)(int startIndex, int count, AuthorFileRegion *regions);
    };

}
