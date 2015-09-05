//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// *******************************************************
// Represents a marshaled Javascript array as a IIterator in JavaScript
// *******************************************************


#include "ProjectionPch.h"

namespace Projection
{
    CUnknownImpl::CUnknownImpl(ProjectionContext *projectionContext, PFN_VTABLE_ENTRY *pvtbl
#if DBG_DUMP
        , UnknownImplType unknownImplType
#endif
        )
        : m_pWeakReference(nullptr),
        m_pWeakReferenceSource(nullptr),
        projectionContext(projectionContext),
        m_pScriptSite(projectionContext->GetScriptSite()),
        m_pvtbl(pvtbl),
        m_typeName(NULL),
        m_iid(GUID_NULL)
    {
        Assert(pvtbl != NULL);
        Assert(projectionContext != nullptr);
        Assert(m_pScriptSite != NULL);
        criticalSectionForUnknownImpl = projectionContext->GetProjectionWriter()->GetCriticalSectionForUnknownsToMark();
        criticalSectionForUnknownImpl->AddRef();
        m_threadId = ::GetCurrentThreadId();

#if DBG_DUMP
        this->projectionMemoryInformation = (ProjectionMemoryInformation*) projectionContext->GetThreadContext()->GetProjectionContextMemoryInformation();
        this->projectionMemoryInformation->AddUnknown(this, unknownImplType);
#endif
    }

    HRESULT CUnknownImpl::Initialize(IID iid, LPCWSTR typeName, bool fOwnRefCounting)
    {
        m_typeName = typeName;
        m_iid = iid;

        if (fOwnRefCounting)
        {
            m_pWeakReference = new CExternalWeakReferenceImpl(this);
            IfNullReturnError(m_pWeakReference, E_OUTOFMEMORY);
            
            // Supports inspectable  
            if (g_DelegateVtable != m_pvtbl)
            {
                m_pWeakReferenceSource = new CExternalWeakReferenceSourceImpl(this);
                IfNullReturnError(m_pWeakReferenceSource, E_OUTOFMEMORY);
            }

            criticalSectionForUnknownImpl->RegisterUnknown(projectionContext, this);
        }

        return S_OK;
    }

    CUnknownImpl::~CUnknownImpl()
    {
        if (m_pWeakReferenceSource)
        {
            delete m_pWeakReferenceSource;
        }

        if (m_pWeakReference)
        {
            m_pWeakReference->Release();
        }

        criticalSectionForUnknownImpl->Release();

#if DBG_DUMP
        this->projectionMemoryInformation->DestructUnknown(this);
#endif
    }

    HRESULT CUnknownImpl::OwnQueryInterface(__in REFIID riid, __out void **ppv)
    {
        IfNullReturnError(ppv, E_POINTER);
        *ppv = nullptr;

        if (((g_DelegateVtable != m_pvtbl) && IsEqualGUID(riid, IID_IInspectable))
            || IsEqualGUID(riid, m_iid))
        {
            *ppv = &m_pvtbl;
            reinterpret_cast<IUnknown*>(*ppv)->AddRef();
            return S_OK;
        }

        *ppv = NULL;
        return E_NOINTERFACE;
    }

    ActiveScriptProfilerHeapEnum* CUnknownImpl::GetHeapEnum() const
    {
        Js::ScriptContext* scriptContext = this->projectionContext->GetScriptContext();
        Assert(scriptContext);
        return reinterpret_cast<ActiveScriptProfilerHeapEnum*>(scriptContext->GetHeapEnum());
    }

    CUnknownMethodNoErrorImpl_Prolog(CUnknownImpl, QueryInterface, (riid, ppv), REFIID riid, void **ppv)
    {
        // Querying for IInspectable / m_iid
        HRESULT hr = OwnQueryInterface(riid, ppv);
        if (hr != E_NOINTERFACE)
        {
            return hr;
        }

        // Querying rest of the suppoted interfaces
        if (IsEqualGUID(riid, IID_IUnknown)) 
        {
            *ppv = &m_pvtbl;
            reinterpret_cast<IUnknown*>(*ppv)->AddRef();
            return S_OK;
        }

        if (IsEqualGUID(riid, IID_IWeakReferenceSource) 
            && m_pWeakReferenceSource != NULL)
        {
            *ppv = m_pWeakReferenceSource;
            m_pWeakReferenceSource->AddRef();
            return S_OK;
        }

        *ppv = NULL;
        return E_NOINTERFACE;
    }
    CUnknownMethodNoErrorImpl_Epilog()

    CUnknownMethodImpl_NoArgs_ULONGReturn_Prolog(CUnknownImpl, AddRef)
    { 
        Assert(m_pWeakReference != NULL);
        uRetVal = m_pWeakReference->StrongAddRef(); 
    }
    CUnknownMethodImpl_NoArgs_ULONGReturn_Epilog()

    CUnknownMethodImpl_NoArgs_ULONGReturn_Prolog(CUnknownImpl, Release)
    {
        Assert(m_pWeakReference != NULL);
        uRetVal = m_pWeakReference->StrongRelease(); 

        if (uRetVal == 0)
        {
            criticalSectionForUnknownImpl->UnregisterUnknown(projectionContext, this);
            delete this;
        }
    }
    CUnknownMethodImpl_NoArgs_ULONGReturn_Epilog()

    HRESULT CUnknownImpl::GetTwoIids(__in IID iid, __RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids)
    {
        IfNullReturnError(iidCount, E_POINTER);
        *iidCount = 0;
        IfNullReturnError(iids, E_POINTER);
        *iids = nullptr;

        HRESULT hr = SupportsInspectable();
        IfFailedReturn(hr);

        *iids = (IID *)CoTaskMemAlloc(sizeof(IID) * 2);
        IfNullReturnError(*iids, E_OUTOFMEMORY);

        (*iids)[0] = m_iid;
        (*iids)[1] = iid;

        *iidCount = 2;
        return hr;
    }

    CUnknownMethodNoErrorImpl_Prolog(CUnknownImpl, GetIids, (iidCount, iids), __RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids)
    {
        IfNullReturnError(iidCount, E_POINTER);
        *iidCount = 0;
        IfNullReturnError(iids, E_POINTER);
        *iids = nullptr;

        HRESULT hr = SupportsInspectable();
        IfFailedReturn(hr);

        *iids = (IID *)CoTaskMemAlloc(sizeof(IID));
        IfNullReturnError(*iids, E_OUTOFMEMORY);

        (*iids)[0] = m_iid;
        *iidCount = 1;
        return hr;
    }
    CUnknownMethodNoErrorImpl_Epilog()
        
    CUnknownMethodNoErrorImpl_Prolog(CUnknownImpl, GetRuntimeClassName, (className), __RPC__deref_out_opt HSTRING *className)
    {
        IfNullReturnError(className, E_POINTER);
        *className = nullptr;

        HRESULT hr = SupportsInspectable();
        IfFailedReturn(hr);

        if (m_pScriptSite == nullptr)
        {
            return E_ACCESSDENIED;
        }
        
        return projectionContext->GetThreadContext()->GetWinRTStringLibrary()->WindowsCreateString(m_typeName, wcslen(m_typeName), className);
    }
    CUnknownMethodNoErrorImpl_Epilog()

    CUnknownMethodNoErrorImpl_Prolog(CUnknownImpl, GetTrustLevel, (trustLevel), __RPC__out TrustLevel *trustLevel)
    {
        IfNullReturnError(trustLevel, E_POINTER);
        *trustLevel = BaseTrust;

        HRESULT hr = SupportsInspectable();
        IfFailedReturn(hr);

        return S_OK;
    }
    CUnknownMethodNoErrorImpl_Epilog()

#if _M_AMD64 || defined(_M_ARM) || defined (_M_ARM64)
    // CallIndirect which calls the method based on method id
    HRESULT __stdcall CUnknownImpl::CallIndirect(ULONG iMethod, __in_bcount(*pcbArgs) void* pvArgs, ULONG* pcbArgs)
    {
        CallIndirectImpl_CUnknownMethod(Delegate, Invoke)
        CallIndirectImpl_CUnknownMethod(ArrayAsVector, IndexOf)
        CallIndirectImpl_CUnknownMethod(ArrayAsVector, SetAt)
        CallIndirectImpl_CUnknownMethod(ArrayAsVector, InsertAt)
        CallIndirectImpl_CUnknownMethod(ArrayAsVector, Append)

        return E_UNEXPECTED;
    }
#endif

    HRESULT CUnknownImpl::GetWeakReference(IWeakReference **weakReference)
    {
        IfNullReturnError(weakReference, E_POINTER);

        HRESULT hr = SupportsInspectable();
        IfFailedReturn(hr);

        Assert(m_pWeakReference != NULL);

        *weakReference = m_pWeakReference;
        m_pWeakReference->AddRef();
        return S_OK;
    }

    CriticalSectionForUnknownImpl::CriticalSectionForUnknownImpl()
    {
        fCanUnregister = true;
        refCount = 1;
        InitializeCriticalSection(&m_cs);
    }

    CriticalSectionForUnknownImpl::~CriticalSectionForUnknownImpl()
    {
        DeleteCriticalSection(&m_cs);
    }

    ULONG CriticalSectionForUnknownImpl::AddRef()
    {
        return InterlockedIncrement(&refCount); 
    }
        
    ULONG CriticalSectionForUnknownImpl::Release()
    {
        ULONG uRetVal = InterlockedDecrement(&refCount);

        if (uRetVal == 0)
        {
            delete this;
        }

        return uRetVal;
    }

    void CriticalSectionForUnknownImpl::EnterCriticalSection()
    {
        ::EnterCriticalSection(&m_cs);
    }

    void CriticalSectionForUnknownImpl::LeaveCriticalSection()
    {
        ::LeaveCriticalSection(&m_cs);
    }

    void CriticalSectionForUnknownImpl::DisableUnregister()
    {
        Assert(fCanUnregister);
        fCanUnregister = false;
    }

    void CriticalSectionForUnknownImpl::RegisterUnknown(ProjectionContext *projectionContext, CUnknownImpl *unknownImpl)
    {
        EnterCriticalSection();
        Assert(fCanUnregister);

        projectionContext->GetProjectionWriter()->RegisterUnknownToCleanOnClose(unknownImpl);

        LeaveCriticalSection();
    }

    void CriticalSectionForUnknownImpl::UnregisterUnknown(ProjectionContext *projectionContext, CUnknownImpl *unknownImpl)
    {
        EnterCriticalSection();
        if (fCanUnregister)
        {
            projectionContext->GetProjectionWriter()->UnRegisterUnknownToCleanOnClose(unknownImpl);
        }
        LeaveCriticalSection();
    }
}
