//  Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdafx.h>
#include "DevTests.h"

using namespace Microsoft::WRL;

namespace DevTests
{
    namespace Delegates
    {
        class CInvalidGRCNString : public Microsoft::WRL::RuntimeClass<IInspectable>
        {
            InspectableClass(L"DevTests.Delegates.InvalidGRCNString", BaseTrust);

        public:
            CInvalidGRCNString() { }
            ~CInvalidGRCNString() { }
        };

        class MethodStaticsFactory :
            public Microsoft::WRL::ActivationFactory<IMethodStatics>
        {
        public:
            HRESULT STDMETHODCALLTYPE OperationOutStatic(long* result) {*result = 20; return S_OK; }
            HRESULT STDMETHODCALLTYPE OperationOutStaticNotFastPath(double num1,  double num2, double num3, double* sum)
            {
                *sum = num1+ num2 + num3;
                return S_OK;
            }

            HRESULT STDMETHODCALLTYPE put_ExceptionalPropStatic(HSTRING ) { return S_OK; }
            HRESULT STDMETHODCALLTYPE get_ExceptionalPropStatic( HSTRING *exceptionallyMeaningless)
            {
                return WindowsCreateString(L"hello", (UINT32)wcslen(L"hello"), exceptionallyMeaningless);
            }
        };

        class StaticTestClassServer :
            public Microsoft::WRL::RuntimeClass<IBaseInterface>
        {
            InspectableClass(L"DevTests.Delegates.StaticTestClass", BaseTrust);
        };

        class TestClassServer :
            public Microsoft::WRL::RuntimeClass<ITestClass>
        {
            InspectableClass(L"DevTests.Delegates.TestClass", BaseTrust);

        public:
            TestClassServer() : m_inspectable(nullptr) { }
            ~TestClassServer() {
                if (m_inspectable)
                {
                    m_inspectable->Release();
                    m_inspectable = nullptr;
                }
            }

            IFACEMETHOD(VerifyTestDelegateFailure)(__in FailureCondition failure, __in Params invalidParams, __in ITestDelegate* func) 
            {
                if (func == nullptr) { return E_POINTER; }

                HSTRING str;
                int num = -4321;
                ITestClass* rc = (ITestClass*)42;
                ITestClass* iface = (ITestClass*)123;

                HRESULT hr = S_OK;
                LPCWSTR message = nullptr;
                switch (failure)
                {
                case FailureCondition_JSException:
                    {
                        hr = func->Invoke(&str, &num, this, &rc, &iface);
                        if (FAILED(hr))
                        {
                            if (str == nullptr && num == 0 && rc == nullptr && iface == nullptr)
                            {
                                message = L"All out parameters initialized";
                            }
                            else 
                            {
                                message = L"NOT all out parameters initialized";
                            }
                        }
                    }
                    break;
                case FailureCondition_FailToMarshal:
                    {
                        if (invalidParams & Params_Input)
                        {
                            if (m_inspectable == nullptr)
                            {
                                ComPtr<IInspectable> spDuplicate = Make<CInvalidGRCNString>();
                                spDuplicate.CopyTo(&m_inspectable);

                                if (m_inspectable == nullptr)
                                {
                                    return E_OUTOFMEMORY;
                                }
                            }
                            hr = func->Invoke(&str, &num, m_inspectable, &rc, &iface);
                            if (FAILED(hr))
                            {
                                if (str == nullptr && num == 0 && rc == nullptr && iface == nullptr)
                                {
                                    message = L"All out parameters initialized";
                                }
                                else 
                                {
                                    message = L"NOT all out parameters initialized";
                                }
                            }
                        }
                        else if (invalidParams & (Params_Class | Params_IFace))
                        {
                            hr = func->Invoke(&str, &num, this, &rc, &iface);
                            if (FAILED(hr))
                            {
                                if (str == nullptr && rc == nullptr && iface == nullptr)
                                {
                                    message = L"All out parameters released";
                                }
                                else 
                                {
                                    message = L"NOT all out parameters released";
                                }
                            }
                        }
                    }
                    break;
                case FailureCondition_NullOutParam:
                    {
                        HSTRING * strOut = &str;
                        int * numOut = &num;
                        ITestClass ** rcOut = &rc;
                        ITestClass ** ifaceOut = &iface;

                        if (invalidParams & Params_IFace)
                        {
                            ifaceOut = nullptr;
                        }
                        if (invalidParams & Params_Class)
                        {
                            rcOut = nullptr;
                        }
                        if (invalidParams & Params_Num)
                        {
                            numOut = nullptr;
                        }
                        if (invalidParams & Params_Str)
                        {
                            strOut = nullptr;
                        }

                        hr = func->Invoke(strOut, numOut, this, rcOut, ifaceOut);
                        if (FAILED(hr))
                        {
                            if ((!ifaceOut || iface == nullptr) 
                                && (!rcOut || rc == nullptr)
                                && (!numOut || num == 0)
                                && (!strOut || str == nullptr))
                            {
                                message = L"All valid out parameters initialized";
                            }
                            else 
                            {
                                message = L"NOT all valid out parameters initialized";
                            }
                        }
                    }
                }
                if (FAILED(hr))
                {
                    HRESULT hrReport = RoSetErrorReportingFlags(Windows::Foundation::Diagnostics::UseSetErrorInfo);
                    if (SUCCEEDED(hrReport))
                    {
                        HSTRING errorMessage;
                        hrReport = WindowsCreateString(message, (UINT32)wcslen(message), &errorMessage);
                        BOOL succeeded = RoOriginateError(hr, errorMessage);
                        if (SUCCEEDED(hrReport))
                        {
                            WindowsDeleteString(errorMessage);
                        }
                        if (succeeded)
                        {
                            return hr;
                        }
                    }
                    return S_FALSE;
                }
                return hr;
            }
            IFACEMETHOD(VerifyTestDelegateOutParams)(__in Params undef, __in Params nul, __in ITestDelegate* func)
            {
                HSTRING str;
                int num;
                ITestClass* rc;
                ITestClass* iface;

                HRESULT hr = func->Invoke(&str, &num, this, &rc, &iface);
                if (FAILED(hr))
                {
                    return hr;
                }

                if (undef & Params_Str)
                {
                    if (wcscmp(WindowsGetStringRawBuffer(str, nullptr), L"undefined") != 0)
                    {
                        hr = E_FAIL;
                    }
                }
                else if (nul & Params_Str)
                {
                    if (wcscmp(WindowsGetStringRawBuffer(str, nullptr), L"null") != 0)
                    {
                        hr = E_FAIL;
                    }
                }

                if (SUCCEEDED(hr))
                {
                    if ((undef & Params_Num) || (nul & Params_Num))
                    {
                        if (num != 0)
                        {
                            hr = E_FAIL;
                        }
                    }
                }

                if (SUCCEEDED(hr))
                {
                    if ((undef & Params_Class) || (nul & Params_Class))
                    {
                        if (rc != nullptr)
                        {
                            hr = E_FAIL;
                        }
                    }
                    else if (rc == nullptr)
                    {
                        hr = E_FAIL;
                    }
                }

                if (SUCCEEDED(hr))
                {
                    if ((undef & Params_IFace) || (nul & Params_IFace))
                    {
                        if (iface != nullptr)
                        {
                            hr = E_FAIL;
                        }
                    }
                    else if (iface == nullptr)
                    {
                        hr = E_FAIL;
                    }
                }

                WindowsDeleteString(str);
                if (rc) {
                    rc->Release();
                }
                if (iface)
                {
                    iface->Release();
                }

                return hr;
            }

        private:
            IInspectable * m_inspectable;
        };

    }
}
