//---------------------------------------------------------------------------
// Copyright (C) 2011 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// Description: Represents the base class that would be used for all the that are wrapper around the js objects

struct ProfilerHeapObjectOptionalInfo;

namespace Projection
{
    class ProjectionContext;
    class CriticalSectionForUnknownImpl;

    typedef HRESULT (__stdcall * const PFN_VTABLE_ENTRY)();

    Declare_UnknownImpl_Extern_VTable(g_DelegateVtable);

    // *******************************************************
    // Represents the base class that would be used for all the that are wrapper around the js objects
    // *******************************************************
    class __declspec(novtable) CUnknownImpl
    {
        friend class ABIMarshalingContext;
        friend class ProjectionWriter;
        friend class ArrayAsIterable;

    protected:
        //
        //  Note:
        //  The order of m_pvtbl and CallIndirect is important and needs to be maintained.
        //  These are used in asm code for generating the thunks and calculating actual this ptr so make sure to update them incase you really need to change the order
        //

        // VTable for this object
        const PFN_VTABLE_ENTRY* m_pvtbl;

#if _M_AMD64 || defined(_M_ARM)
        // CallIndirect which calls the method based on method id
        virtual HRESULT __stdcall CallIndirect(ULONG iMethod, __in_bcount(*pcbArgs) void* pvArgs, ULONG* pcbArgs);
#endif

        // WeakReference is going to maintain the refcount for us
        CExternalWeakReferenceImpl *m_pWeakReference;
        CExternalWeakReferenceSourceImpl *m_pWeakReferenceSource;

        // We shouldnt be accessing scriptSite from projectionContext as Interface implementations that we give away to winrt can come back when callrootlevel = 0
        // (that javascript function doesnt exist on the current stack) and hence the projectionContext might have been destroyed
        // Hence the IsClosed check on the entry should use the directly stored scriptsite.
        //
        // After the entry all the queries for scriptContext or scriptEngine should always go through projectionContext as once inside the delegate fn call,
        // scriptSite can be closed and the GetScriptSiteContext() or GetScriptEngine() would return null in these scenarios.
        // ProjectionContext would be able to provide us scriptContext irrespective of scriptSite close and we shouldnt be using scriptEngine if scriptSite is closed.

        ScriptSite *m_pScriptSite;
        ProjectionContext *projectionContext;

        // If it is inspectable the typeName to return as GetRuntimeClassName()
        LPCWSTR m_typeName;

        // Our IID
        IID m_iid;

        // ThreadId
        DWORD m_threadId;

#if DBG_DUMP
        // Store the ProjectionMemoryInformation when this object is created - when it is disposed the ProjectionContext may be garbage so we won't be able to get it.
        ProjectionMemoryInformation* projectionMemoryInformation;
#endif

        CriticalSectionForUnknownImpl *criticalSectionForUnknownImpl;

        // Get IIDs with iid and m_iid
        HRESULT GetTwoIids(__in IID iid, __RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids);

    protected:
        CUnknownImpl::CUnknownImpl(ProjectionContext *projectionContext, PFN_VTABLE_ENTRY *pvtbl
#if DBG_DUMP
            , UnknownImplType unknownImplType
#endif
            );
        virtual ~CUnknownImpl();

        // Does class have inspectable support
        HRESULT SupportsInspectable() { Assert(m_pvtbl != nullptr); return (m_pvtbl != g_DelegateVtable) ? S_OK : E_UNEXPECTED; }

        virtual void MarkScriptContextDependentResources(Recycler *recycler, bool inPartialCollect) { }

        HRESULT OwnQueryInterface(__in REFIID riid, __out void **ppv);

        ActiveScriptProfilerHeapEnum* GetHeapEnum() const;

    public:
        // Initialize
        HRESULT Initialize(IID iid, LPCWSTR typeName, bool fOwnRefCounting = true);

        // Helpers for the object

        //
        // IUnknown members
        //

        CUnknownMethodNoErrorImpl_Def(CUnknownImpl, QueryInterface, REFIID riid, void **ppv);
        CUnknownMethodImpl_NoArgs_ULONGReturn_Def(CUnknownImpl, AddRef);
        CUnknownMethodImpl_NoArgs_ULONGReturn_Def(CUnknownImpl, Release);

        //
        // IInspectable Methods
        //
        CUnknownMethodNoErrorImpl_Def(CUnknownImpl, GetIids, __RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids);
        CUnknownMethodNoErrorImpl_Def(CUnknownImpl, GetRuntimeClassName, __RPC__deref_out_opt HSTRING *className) sealed;
        CUnknownMethodNoErrorImpl_Def(CUnknownImpl, GetTrustLevel, __RPC__out TrustLevel *trustLevel) sealed;

        virtual IUnknown *GetUnknown() { return (IUnknown *)&m_pvtbl; }
        IID GetOwnIID() { return m_iid; }

        HRESULT GetWeakReference(IWeakReference **weakReference);

        LPCWSTR GetFullTypeName() { return m_typeName; }
        virtual USHORT GetWinrtTypeFlags() { return 0; }
        virtual UINT GetHeapObjectRelationshipInfoSize() { return 0; }
        virtual void FillHeapObjectRelationshipInfo(ProfilerHeapObjectOptionalInfo*) { Assert(false); }
        virtual bool CanBeManagedByGC() { return false; }
        virtual void MarkForClose() { m_pScriptSite = nullptr; }

#ifdef WINRTFINDPREMATURECOLLECTION
        virtual void VerifyNotDisconnected() { };
#endif

        CExternalWeakReferenceImpl *GetPrivateWeakReference()
        {
            Assert(m_pWeakReference != nullptr);
            m_pWeakReference->AddRef();
            return m_pWeakReference;
        }
    };

    class CriticalSectionForUnknownImpl
    {
        friend class ProjectionWriter;

    private:
        CRITICAL_SECTION m_cs;
        unsigned long refCount;
        bool fCanUnregister;

        void EnterCriticalSection();
        void LeaveCriticalSection();

    public:
        CriticalSectionForUnknownImpl();
        ~CriticalSectionForUnknownImpl();

        ULONG AddRef();
        ULONG Release();

        void DisableUnregister();
        void RegisterUnknown(ProjectionContext *projectionContext, CUnknownImpl *unknownImpl);
        void UnregisterUnknown(ProjectionContext *projectionContext, CUnknownImpl *unknownImpl);
    };
}
