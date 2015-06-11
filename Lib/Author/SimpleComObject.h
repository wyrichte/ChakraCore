//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2009 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// Description: Basic factory-less COM object implementation.
// Release operation is customizable by overriding Delete method, allowing creation of the object using allocators such as ArenaAllocator.

#pragma once

namespace Authoring
{
    void ScheduleRelease(IUnknown *unk, ThreadContextId threadId);
    void PerformScheduledReleases();

    struct HeapFreePolicy
    {
        void Free(void* obj) { delete obj; }
    };

    struct NoFreePolicy
    {
        void Free(void* obj) { }
    };

    struct SafeRelease
    {
    private:
        ThreadContextId m_threadId;        
    protected:
        SafeRelease() : m_threadId(GetCurrentThreadContextId()) { }
        bool ShouldScheduleRelease() { return m_threadId != GetCurrentThreadContextId(); }
        ThreadContextId GetThreadId() { return m_threadId; }
    };

    struct UnsafeRelease
    {
    protected:
        bool ShouldScheduleRelease() { return false; }
        ThreadContextId GetThreadId() { return nullptr; }
    };

    template <class TPrimaryInterface, typename FreePolicy = HeapFreePolicy, typename ReleasePolicy = SafeRelease>
    class SimpleComObject: public TPrimaryInterface, public FreePolicy, public ReleasePolicy
    {

#if TRACK_TYPE_STATS
        // Note: ignore dependent objects (when m_outer is provided.) as they are allocated using outer object allocator
        // and are not explicitly deleted. 
        static TypeStats _typeStats;
#endif

    public:
        SimpleComObject() : m_outer(nullptr), m_refCount(1) 
        { 
#if TRACK_TYPE_STATS
            if(!m_outer)
                _typeStats.IncrementInstances();
#endif
        }

        SimpleComObject(IUnknown* outer) 
            : m_outer(outer) 
        { 
            // The initial reference count should be zero for a dependent object since it is not directly creatable 
            // and is given out by outer object methods/properties which will AddRef it when providing a reference to it. 
            m_refCount = m_outer != nullptr ? 0 : 1;

#if TRACK_TYPE_STATS
            if(!m_outer)
                _typeStats.IncrementInstances();
#endif
        }

        //
        // IUnknown
        //

        virtual HRESULT STDMETHODCALLTYPE QueryInterface(__in REFIID riid, __out void **ppvObject)
        {
            if(ppvObject == NULL)
            {
                return E_POINTER;
            }

            #define __QI_IMPL(intf)\
            if (IsEqualIID(riid, __uuidof(intf)))\
            {   \
                *ppvObject = static_cast<intf *>(this); \
                AddRef();\
                return NOERROR;\
            }

            __QI_IMPL(IUnknown);
            __QI_IMPL(TPrimaryInterface);

            #undef __QI_IMPL

            // GetInterface may be overriden in the derived class to provide additional interfaces.
            // Note: GetInterface implementation is responsible for the proper AddRef of the returned interface.
            HRESULT hr = GetInterface(riid, ppvObject);
            if(SUCCEEDED(hr))
            {
                Assert(*ppvObject);
            }

            return hr;
        }

        virtual ULONG STDMETHODCALLTYPE AddRef()
        {
            ULONG refCount = (ULONG)InterlockedIncrement(&m_refCount);

            if(m_outer != nullptr)
            {
                return m_outer->AddRef();
            }

            return refCount;
        }

        virtual ULONG STDMETHODCALLTYPE Release()
        {
            // Keep the outer object reference in case this object is deleted.
            auto outer = m_outer;

            LONG refCount = (ULONG)InterlockedDecrement(&m_refCount);

            if(refCount == 0)
            {
                if (ShouldScheduleRelease())
                {
                    // Schedule the release to occur on the language service thread instead of this one.
                    ScheduleRelease(this, GetThreadId());
                    
                    // No need for an interlocked update here since all other references are now released.
                    m_refCount = 1;

                    // Let the caller know we are going to be freeing this.
                    return 0;
                }

#ifdef EXCEPTION_CHECK
                AutoFilterExceptionRegion filter((ExceptionType)(ExceptionType_OutOfMemory | ExceptionType_StackOverflow));
#endif
                HRESULT hr;
                BEGIN_TRANSLATE_OOM_TO_HRESULT
                {
                    Delete();
                }
                END_TRANSLATE_OOM_TO_HRESULT(hr);
                // Ignore OOM exceptions during release.
            }

            if(outer != nullptr)
            {
                refCount = outer->Release();
            }

            return refCount;
        }

    protected:

        // May be overriden on derived class
        virtual void OnDelete() { }

        virtual HRESULT GetInterface(REFIID iid, __out void** ppvObject)
        {
            // Default implementation
            return E_NOINTERFACE;
        }

    private:

        void Delete()
        {
            auto outer = m_outer;

            OnDelete();
            // invoke the FreePolicy
            Free(this);

#if TRACK_TYPE_STATS
            if(!outer)
                _typeStats.DecrementInstances();
#endif
        }

        long        m_refCount;
        IUnknown*   m_outer;
    };

    template <class TPrimaryInterface>
    class SimpleComObjectWithAlloc: public SimpleComObject<TPrimaryInterface>
    {
    private:
        ThreadContextId m_threadId;
        ArenaAllocator* m_alloc;
    public:
        SimpleComObjectWithAlloc(PageAllocator* pageAlloc, LPCWSTR arenaName, IUnknown* outer = nullptr)
            :SimpleComObject<TPrimaryInterface>(outer), 
            m_alloc(HeapNew(ArenaAllocator, arenaName, pageAlloc, Js::Throw::OutOfMemory)),
            m_threadId(GetCurrentThreadContextId())
        {
        }

        ArenaAllocator* Alloc() { return m_alloc; }

        virtual void OnDelete() override
        {
            Assert(this->m_threadId == GetCurrentThreadContextId());

            if (this->m_threadId == GetCurrentThreadContextId())
                HeapDelete(m_alloc);
        }
    };

    template <class TPrimaryInterface>
    class InnerComObject: public SimpleComObject<TPrimaryInterface, NoFreePolicy>
    {
    protected:
        InnerComObject(IUnknown* outer)
            :SimpleComObject<TPrimaryInterface, NoFreePolicy>(outer)
        {
            Assert(outer != nullptr);
        }
    };

    template<class TType, class TItemType>
    class CollectionBase: public InnerComObject<TType>
    {
    protected:
        JsUtil::List<TItemType*, ArenaAllocator> m_items;
    public:
        CollectionBase(IUnknown* outer, ArenaAllocator* alloc)
            :InnerComObject<TType>(outer), m_items(alloc)	{ }

        //
        //	Internal interface to populate the collection
        //

        void Add(TItemType* item) { m_items.Add(item); }
        int Count() { return m_items.Count(); }
        TItemType* Item(int index) { return m_items.Item(index); }
   
        //
        // TType COM interface implementation
        //

        virtual HRESULT STDMETHODCALLTYPE get_Count(__out int* count)
        {
            IfNullReturnError(count, E_POINTER);
            *count = m_items.Count();
            return S_OK;
        }
    };

    template<class TType, class TItemType>
    class Collection: public CollectionBase<TType, TItemType>
    {
    public:
        Collection(IUnknown* outer, ArenaAllocator* alloc)
            :CollectionBase<TType, TItemType>(outer, alloc)	{ }

        virtual HRESULT STDMETHODCALLTYPE GetItems(int startIndex, int count, __out TItemType** items)
        {
            IfNullReturnError(items, E_POINTER);
            IfNullReturnError(startIndex + count <= m_items.Count(), E_INVALIDARG);
        
            for (int i = 0; i < count; i++)
            {
                auto item = m_items.Item(startIndex + i);
                if (item) item->AddRef();
                items[i] = item;
            }

            return S_OK;
        }
    };

    template<class TType, class TItemType>
    class StructCollection: public CollectionBase<TType, TItemType>
    {
    public:
        StructCollection(IUnknown* outer, ArenaAllocator* alloc)
            :CollectionBase<TType, TItemType>(outer, alloc)	{ }

        virtual HRESULT STDMETHODCALLTYPE GetItems(int startIndex, int count, __out TItemType* items)
        {
            IfNullReturnError(items, E_POINTER);
            IfNullReturnError(startIndex + count <= m_items.Count(), E_INVALIDARG);
        
            for (int i = 0; i < count; i++)
            {
                auto item = m_items.Item(startIndex + i);
                Assert(item);
                memcpy_s(&items[i], sizeof(TItemType), item, sizeof(TItemType));
            }

            return S_OK;
        }
    };

    // A string field helper for SimpleComObject derived classes.
    // Takes care of initialization, allocation, reallocation, 
    // access to the underlying buffer, BSTR conversion and proper null/empty string handling.
    class ComStringField
    {
    private:
        Js::InternalString* m_value;
    public:
        ComStringField() : m_value(nullptr) { }

        void Set(ArenaAllocator* alloc, LPCWSTR s)
        {
            if(m_value != nullptr)
            {
                // Assuming the value was allocated using the same allocator
                Adelete(alloc, m_value);
                m_value = nullptr;
            }

            if(s != nullptr && s[0] != 0)
            {
                m_value = AllocInternalString(alloc, s);
            }
        }

        LPCWSTR Sz()
        {
            if(m_value == nullptr)
            {
                return L"";
            }

            return m_value->GetBuffer();
        }

        BSTR BSTR()
        {
            if(m_value == nullptr)
            {
                return nullptr;
            }

            return AllocBSTR(m_value);
        }
    };

    #define SIMPLE_PROPERTY_IMPL(PropName, PropType, PropValue) \
        virtual HRESULT STDMETHODCALLTYPE get_##PropName(__out PropType* value) \
        { \
            IfNullReturnError(value, E_POINTER); \
            *value = PropValue; \
            return S_OK; \
        } 

    #define STRING_PROPERTY_IMPL(PropName, Member) \
        virtual HRESULT STDMETHODCALLTYPE get_##PropName(__out BSTR* value) \
        { \
            IfNullReturnError(value, E_POINTER); \
            *value = Member.BSTR(); \
            return S_OK; \
        } 

    #define VARIANT_BOOL_PROPERTY_IMPL(PropName, Member) \
        virtual HRESULT STDMETHODCALLTYPE get_##PropName(__out VARIANT_BOOL* value) \
        { \
            IfNullReturnError(value, E_POINTER); \
            if (Member) \
            { \
                *value = VARIANT_TRUE; \
            } \
            else \
            { \
                *value = VARIANT_FALSE; \
            } \
            return S_OK; \
        } 

    #define INTERFACE_PROPERTY_IMPL(PropName, PropType, PropValue) \
        virtual HRESULT STDMETHODCALLTYPE get_##PropName(__out PropType** value) \
        { \
            IfNullReturnError(value, E_POINTER); \
            *value = nullptr; \
            if((PropValue) != nullptr) \
            { \
                (PropValue)->AddRef(); \
                *value = PropValue; \
            } \
            return S_OK; \
        } 

    #define COLLECTION_PROPERTY_IMPL(PropName, CollType, Member) \
        HRESULT STDMETHODCALLTYPE get_##PropName(__out CollType** value) \
        { \
            IfNullReturnError(value, E_POINTER); \
            (Member)->AddRef(); \
            *value = Member; \
            return S_OK; \
        } 
}