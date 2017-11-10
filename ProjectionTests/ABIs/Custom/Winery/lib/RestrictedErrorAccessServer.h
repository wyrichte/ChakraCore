//  Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdafx.h>
#include "winery.h"

class AutoHSTRING
{
protected:
    HSTRING hs;
    
public:
    AutoHSTRING(HSTRING hs = nullptr) : hs(hs) { }
    ~AutoHSTRING() { this->Release(); }
    
private:
    void Release()
    {
        if (this->hs)
        {
            WindowsDeleteString(hs);
            hs = nullptr;
        }
    }
};

namespace Winery
{
    namespace WinRTErrorTests
    {
        class RestrictedErrorAccessErrorCodes
        {
        protected:
            // This is here instead of directly in RestrictedErrorAccessImpl to avoid problems due to the template parameter
            static Errors m_Errors;
        };

        template <class BaseClass>
        class RestrictedErrorAccessImpl : public BaseClass, RestrictedErrorAccessErrorCodes
        {
        public:
            IFACEMETHOD(get_ErrorCodes)(__out Errors * value)
            {
                *value = m_Errors;
                return S_OK;
            }

            IFACEMETHOD(get_SuccessCode)(__out HRESULT * value)
            {
                *value = S_OK;
                return S_OK;
            }

            // Originate Error Utility
            // description: originates a winrt error with the given hresult and message
            // parameters:  hr - HRESULT of error to originate
            //              str - message to originate error with
            // returns:     S_FALSE - if failed to originate error
            //              hr otherwise
            IFACEMETHOD(OriginateError)(HRESULT hr, HSTRING str)
            {
                HRESULT hrReport = RoSetErrorReportingFlags(Windows::Foundation::Diagnostics::UseSetErrorInfo);
                if (SUCCEEDED(hrReport))
                {
                    HSTRING errorMessage = nullptr;
                    
                    WindowsDuplicateString(str, &errorMessage);
                    
                    AutoHSTRING autoReleaseMessage(errorMessage);
                    
                    BOOL succeeded = RoOriginateError(hr, errorMessage);
                    
                    if (succeeded)
                    {
                        return hr;
                    }
                }
                return S_FALSE;
            }


            IFACEMETHOD(ReturnHr)(HRESULT hr)
            {
                return hr;
            }

            IFACEMETHOD(InvokeDelegate)(IDelegateToInvoke * func, HSTRING message)
            {
                return func->Invoke(this, message);
            }

            IFACEMETHOD(TransformError)(HRESULT hr, HRESULT hrNew, HSTRING str, HSTRING strNew)
            {
                HRESULT hrReport = RoSetErrorReportingFlags(Windows::Foundation::Diagnostics::UseSetErrorInfo);
                if (SUCCEEDED(hrReport))
                {
                    hrReport = OriginateError(hr, str);
                    if (hrReport != hr) { return S_FALSE; }
                    if (FAILED(hrReport))
                    {
                        HSTRING errorMessage;
                        hrReport = WindowsDuplicateString(strNew, &errorMessage);
                        BOOL succeeded = RoTransformError(hr, hrNew, errorMessage);
                        if (SUCCEEDED(hrReport))
                        {
                            WindowsDeleteString(errorMessage);
                        }
                        if (succeeded)
                        {
                            return hrNew;
                        }
                    }
                    return hr;
                }
                return S_FALSE;
            }

            IFACEMETHOD(TransformDelegateError)(IDelegateToInvoke * failingFunc, HSTRING message, HRESULT hrNew, HSTRING strNew)
            {
                HRESULT hrReport = RoSetErrorReportingFlags(Windows::Foundation::Diagnostics::UseSetErrorInfo);
                if (SUCCEEDED(hrReport))
                {
                    HRESULT hr = failingFunc->Invoke(this, message);
                    if (FAILED(hr))
                    {
                        HSTRING errorMessage;
                        hrReport = WindowsDuplicateString(strNew, &errorMessage);
                        BOOL succeeded = RoTransformError(hr, hrNew, errorMessage);
                        if (SUCCEEDED(hrReport))
                        {
                            WindowsDeleteString(errorMessage);
                        }
                        if (succeeded)
                        {
                            return hrNew;
                        }
                    }
                    return hr;
                }
                return S_FALSE;
            }

            IFACEMETHOD(OriginateErrorReturnHr)(HRESULT hr, HSTRING str, HRESULT hrToReturn)
            {
                HRESULT hrReport = OriginateError(hr, str);
                if (FAILED(hrReport))
                {
                    return hrToReturn;
                }
                return S_FALSE;
            }

            IFACEMETHOD(OriginateErrorWithCapabilitySid)(HRESULT hr, HSTRING str, TestSids sidEnum)
            {
                HRESULT hrReport = RoSetErrorReportingFlags(Windows::Foundation::Diagnostics::UseSetErrorInfo);
                if (SUCCEEDED(hrReport))
                {
                    DWORD SidSize;
                    PSID TheSID;

                    SidSize = SECURITY_MAX_SID_SIZE;
                    // Allocate enough memory for the largest possible SID.
                    TheSID = LocalAlloc(LMEM_FIXED, SidSize);
                    if(!TheSID)
                    {    
                        return E_OUTOFMEMORY;
                    }
                    // Create a CapabalitySid.
                    if(!CreateWellKnownSid((WELL_KNOWN_SID_TYPE)sidEnum, NULL, TheSID, &SidSize))
                    {
                        LocalFree(TheSID);
                        return E_INVALIDARG;
                    }
                    else
                    {
                        UINT32 strlen;
                        PCWSTR pszStr = WindowsGetStringRawBuffer(str, &strlen);
                        RoReportCapabilityCheckFailure(
                            hr,
                            strlen,
                            pszStr,
                            TheSID);

                        // When done, free the memory used.
                        LocalFree(TheSID);
                        return hr;
                    }
                }

                return S_FALSE;
            }

            IFACEMETHOD(GetSidString)(TestSids sidEnum, __out HSTRING * sidStr)
            {
                DWORD SidSize;
                PSID TheSID;

                SidSize = SECURITY_MAX_SID_SIZE;
                // Allocate enough memory for the largest possible SID.
                TheSID = LocalAlloc(LMEM_FIXED, SidSize);
                if(!TheSID)
                {    
                    return E_OUTOFMEMORY;
                }
                // Create a CapabalitySid.
                if(!CreateWellKnownSid((WELL_KNOWN_SID_TYPE)sidEnum, NULL, TheSID, &SidSize))
                {
                    LocalFree(TheSID);
                    return E_INVALIDARG;
                }
                else
                {
                    WCHAR* pszSidString = nullptr;
                    if (nullptr != TheSID)
                    {
                        BOOL fReturn = FALSE;
                        fReturn = ConvertSidToStringSidW(TheSID, &pszSidString);
                        if (FALSE == fReturn)
                        {
                            LocalFree(TheSID);
                            return E_FAIL;
                        }
                        WindowsCreateString(pszSidString, (UINT32)wcslen(pszSidString), sidStr);
                        LocalFree(pszSidString);
                    }
                }

                LocalFree(TheSID);
                return S_OK;
            }
        };

        class RestrictedErrorAccessServer :
            public Microsoft::WRL::RuntimeClass<IEmpty>
        {
            InspectableClass(L"Winery.WinRTErrorTests.RestrictedErrorAccess", BaseTrust);
        };

        class RestrictedErrorAccessFactory :
            public RestrictedErrorAccessImpl<Microsoft::WRL::ActivationFactory<IRestrictedErrorAccess>>
        {
        public:
            // IActivationFactory
            IFACEMETHOD(ActivateInstance)(__deref_out IInspectable **ppInspectable)
            {
                *ppInspectable = nullptr;
                return E_NOTIMPL;
            }
        };

        class RestrictedErrorAccessInstanceServer :
            public RestrictedErrorAccessImpl<Microsoft::WRL::RuntimeClass<IRestrictedErrorAccess>>
        {
            InspectableClass(L"Winery.WinRTErrorTests.RestrictedErrorAccessInstance", BaseTrust);
        };

        class RestrictedErrorAccessInstanceFactory :
            public Microsoft::WRL::ActivationFactory<IEmpty>
        {
        public:
            // IActivationFactory
            IFACEMETHOD(ActivateInstance)(__deref_out IInspectable **ppInspectable)
            {
                HRESULT hr = S_OK;
                Microsoft::WRL::ComPtr<IRestrictedErrorAccess> spIInstance;
                *ppInspectable = nullptr;

                Microsoft::WRL::ComPtr<RestrictedErrorAccessInstanceServer> spObj =
                    Microsoft::WRL::Make<RestrictedErrorAccessInstanceServer>();

                hr = spObj.As(&spIInstance);
                if (SUCCEEDED(hr))
                {
                    *ppInspectable = spIInstance.Detach();
                }

                return hr;
            }
        };
    }
}