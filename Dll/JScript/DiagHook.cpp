//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "stdafx.h"

extern HANDLE g_hInstance;

namespace JsDiag
{
    DiagHook::DiagHook() {}

  
    STDMETHODIMP DiagHook::GetGlobals(
        /* [size_is][out] */ __RPC__out_ecount_full(globalsCount) void **globals,
        /* [in] */ ULONG globalsCount)
    {
        const INT_PTR s_globals[] =
        {
#define ENTRY(field, name) reinterpret_cast<INT_PTR>(&##field##),
#include "DiagGlobalList.h"
#undef ENTRY
        };

        if(globalsCount != _countof(s_globals))
        {
            AssertMsg(false, "Mismatched runtime ~ diag?");
            return E_INVALIDARG;
        }

        INT_PTR baseAddr = reinterpret_cast<INT_PTR>(g_hInstance);
        for (ULONG i = 0; i < globalsCount; i++)
        {
            globals[i] = (void*)(s_globals[i] - baseAddr);
        }
        return S_OK;
    }

    STDMETHODIMP DiagHook::GetVTables(
        /* [size_is][out] */ __RPC__out_ecount_full(bufferSize) void **vtables,
        /* [in] */ ULONG bufferSize)
    {
        // NOTE: This table can't be static -- it may be resolved earlier than static ...::Address.
        const INT_PTR s_diagVTables[] =
        {
#define ENTRY(s) VirtualTableInfo<Js::##s##>::Address,
#define PROJECTION_ENTRY(s) VirtualTableInfo<Projection::##s##>::Address,
#include "DiagVTableList.h"
#undef ENTRY
#undef PROJECTION_ENTRY
        };

        if (bufferSize != _countof(s_diagVTables))
        {
            AssertMsg(false, "Mismatched runtime ~ diag?");
            return E_INVALIDARG;
        }

        const INT_PTR baseAddr = (INT_PTR)g_hInstance;
        for (int i = 0; i < _countof(s_diagVTables); i++)
        {
            vtables[i] = reinterpret_cast<void*>(s_diagVTables[i] - baseAddr);
        }

        return S_OK;
    }

    STDMETHODIMP DiagHook::GetErrorString( 
            /* [in] */ HRESULT errorCode,
            /* [out] */ __RPC__deref_out_opt BSTR *bsResource)
    {
        LCID lcid = GetUserLocale();

        HRESULT hr = errorCode;

        // FACILITY_CONTROL is used for internal (activscp.idl) and legacy errors
        // FACILITY_JSCRIPT is used for newer public errors
        if (FACILITY_CONTROL == HRESULT_FACILITY(hr) || FACILITY_JSCRIPT == HRESULT_FACILITY(hr))
        {
            HRESULT hrAdjusted = Js::JavascriptError::GetAdjustedResourceStringHr(hr, /* isFormatString */ true);

			BSTR message = BstrGetResourceString(hrAdjusted, lcid);
            if (!message)
            {
				hrAdjusted = Js::JavascriptError::GetAdjustedResourceStringHr(hr, /* isFormatString */ false);

                message = BstrGetResourceString(hrAdjusted, lcid);
            }

            if (message)
            {
                *bsResource = message;
                return S_OK;
            }
        }

        *bsResource = nullptr;
        return E_FAIL;
    }
} // namespace JsDiag.
