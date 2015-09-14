//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "stdafx.h"

namespace JsDiag 
{
    // Private unit testing only hook
    STDMETHODIMP JsDebug::OpenVirtualProcess(            
        /* [in] */ __RPC__in_opt IUnknown *pTestDataTarget,
        /* [in] */ bool validateDebugMode,
        /* [in] */ DWORD processId,
        /* [in] */ UINT64 baseAddress,
        /* [in] */ __RPC__in_opt IJsDebugDataTarget *pDataTarget,
        /* [out] */ __RPC__deref_out_opt IJsDebugProcess **ppProcess)
    {
        
        if(!ppProcess)
        {
            return E_POINTER;
        }

        if(baseAddress == 0 || processId == 0)
        {
            return E_INVALIDARG;
        }
        return JsDebugApiWrapper([=]
        {
            HRESULT hr = S_OK;
#if VALIDATE_RUNTIME_VERSION
            DWORD diagMajorVersion, diagMinorVersion;
            hr = Module::GetFileVersion(&diagMajorVersion, &diagMinorVersion);
            Assert(SUCCEEDED(hr));
            if(FAILED(hr))
            {
                return E_UNEXPECTED;
            }

            HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, /*inheritHandle*/ false, processId);
            if(processHandle == nullptr)
            {
                return HRESULT_FROM_WIN32(GetLastError());
            }

            DWORD majorVersion, minorVersion;
            hr = Module::GetFileVersion(processHandle, (HINSTANCE)baseAddress, &majorVersion, &minorVersion); 

            CloseHandle(processHandle);
            if(FAILED(hr) || majorVersion !=  diagMajorVersion || minorVersion != diagMinorVersion)
            {   
                return E_JsDEBUG_MISMATCHED_RUNTIME;
            }
#endif
            CComPtr<IStackProviderDataTarget> spTestDataTarget;
            if (pTestDataTarget != nullptr)
            {
                IfFailRet(pTestDataTarget->QueryInterface(&spTestDataTarget));
            }

            CreateComObject<JsDebugProcess>(baseAddress, pDataTarget, spTestDataTarget, processId, validateDebugMode, ppProcess);
            return S_OK;
        });
    }

    STDMETHODIMP JsDebug::OpenVirtualProcess(
        /* [in] */ DWORD processId,
        /* [in] */ UINT64 runtimeJsBaseAddress,
        /* [in] */ __RPC__in_opt IJsDebugDataTarget *pDataTarget,
        /* [out] */ __RPC__deref_out_opt IJsDebugProcess **ppProcess)
    {
        Assert(pDataTarget);
        if(!pDataTarget)
        {
            return E_INVALIDARG;
        }
        // This public API always create a debug mode inspection support
        return JsDebug::OpenVirtualProcess(nullptr, /*validateDebugMode*/ true, processId, runtimeJsBaseAddress, pDataTarget, ppProcess);
    }
}
