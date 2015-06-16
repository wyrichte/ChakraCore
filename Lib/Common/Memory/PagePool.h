//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------


// PagePool caches freed pages in a pool for reuse, and more importantly,
// defers freeing them until ReleaseFreePages is called.
// This allows us to free the pages when we know it is multi-thread safe to do so,
// e.g. after all parallel marking is completed.

namespace Memory
{
class PagePoolPage
{
private:
    PageAllocator * pageAllocator;
    PageSegment * pageSegment;

public:
    static PagePoolPage * New(PageAllocator * pageAllocator)
    {
        PageSegment * pageSegment;
        PagePoolPage * newPage = (PagePoolPage *)pageAllocator->AllocPages(1, &pageSegment);
        if (newPage == null)
        {
            return null;
        }

        newPage->pageAllocator = pageAllocator;
        newPage->pageSegment = pageSegment;
        return newPage;
    }

    void Free()
    {
        this->pageAllocator->ReleasePages(this, 1, this->pageSegment);
    }
};


class PagePool
{
private:
    class PagePoolFreePage : public PagePoolPage
    {
    public:
        PagePoolFreePage * nextFreePage;
    };

    PageAllocator pageAllocator;
    PagePoolFreePage * freePageList;

public:
    PagePool(Js::ConfigFlagsTable& flagsTable) :
        pageAllocator(NULL, flagsTable, PageAllocatorType_GCThread, PageAllocator::DefaultMaxFreePageCount, false, null, PageAllocator::DefaultMaxAllocPageCount, 0, true),
        freePageList(null)
    {
    }

    ~PagePool()
    {
        Assert(freePageList == null);
    }

    PageAllocator * GetPageAllocator() { return &this->pageAllocator; }
    
    PagePoolPage * GetPage()
    {
        if (freePageList != null)
        {
            PagePoolPage * page = freePageList;
            freePageList = freePageList->nextFreePage;
            return page;
        }

        return PagePoolPage::New(&pageAllocator);
    }

    void FreePage(PagePoolPage * page)
    {
        PagePoolFreePage * freePage = (PagePoolFreePage *)page;
        freePage->nextFreePage = freePageList;
        freePageList = freePage;
    }

    void ReleaseFreePages()
    {
        while (freePageList != null)
        {
            PagePoolFreePage * page = freePageList;
            freePageList = freePageList->nextFreePage;
            page->Free();
        }
    }

    void Decommit()
    {
        pageAllocator.DecommitNow();
    }

#if DBG
    bool IsEmpty() const { return (freePageList == null); }
#endif
};
}
