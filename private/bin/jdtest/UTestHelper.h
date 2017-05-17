//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

//
// A dummy test group to use with UT_ASSERT helpers.
//
class DummyTestGroup
{
protected:
    bool m_fLastTestCaseFailed;
    bool m_fVerboseEnabled;

public:
    DummyTestGroup()
        : m_fLastTestCaseFailed(false),
        m_fVerboseEnabled(false)
    {
    }

    bool HasFailure() const
    {
        return m_fLastTestCaseFailed;
    }

    void MergeFailure(const DummyTestGroup* g)
    {
        m_fLastTestCaseFailed |= g->HasFailure();
    }

    bool TraceVerbose() const
    {
        return m_fVerboseEnabled;
    }
};

// Use a dummy test group for current scope.
#define UT_USE_DUMMY_TEST_GROUP() \
    bool m_fLastTestCaseFailed = false; \

// Ensure com hr succeeded. Otherwise fail and print error message.
#define UT_COM_SUCCEEDED(e) \
{ \
    HRESULT _hr_ = (e); \
    if (FAILED(_hr_)) \
    { \
        m_fLastTestCaseFailed = true; \
        wprintf(_u("Error 0x%X: %s\n"), _hr_, _u(#e)); \
        wprintf(_u("Function %s at %s, line %d\n"), __FUNCTIONW__, __FILEW__, __LINE__); \
        __debugbreak(); \
        return; \
    } \
} \

inline ULONG HILONG(ULONG64 value)
{
    return (ULONG)(value >> 32);
}

inline ULONG LOLONG(ULONG64 value)
{
    return (ULONG)(value & 0xffffffff);
}

template <class T>
HRESULT CreateComObject(T** ppInstance)
{
    CComObject<T>* p;
    HRESULT hr = CComObject<T>::CreateInstance(&p);
    if (SUCCEEDED(hr))
    {
        p->AddRef();
        *ppInstance = p;
    }
    return hr;
}

//
// Auto release resources contained in JsDebugPropertyInfo.
//
class AutoJsDebugPropertyInfo:
    public JsDebugPropertyInfo
{
public:
    AutoJsDebugPropertyInfo()
    {
        name = NULL;
        type = NULL;
        value = NULL;
        fullName = NULL;
        attr = JS_PROPERTY_ATTRIBUTE_NONE;
    }

    ~AutoJsDebugPropertyInfo()
    {
        ::SysFreeString(name);
        ::SysFreeString(type);
        ::SysFreeString(value);
        ::SysFreeString(fullName);
    }

    BSTR FullName() const { return fullName; }
    BSTR Name() const { return name; }
    BSTR Name(boolean full) const { return full ? FullName() : Name(); }
    bool HasFullName() const { return true; }
    BSTR Type() const { return type; }
    BSTR Value() const { return value; }
    DWORD Attr() const { return static_cast<DWORD>(attr); }
    bool IsExpandable() const { return attr & JS_PROPERTY_HAS_CHILDREN; }
};

//
// Helper for finding the jscript module in the debuggee process
//
template <bool IsPublic>
struct JsModuleList
{
    static PCSTR moduleList[];
};
PCSTR JsModuleList<true>::moduleList[] = { "chakra", "chakracore", "jscript9" }; // Only support jscript9 and chakra for public tool
PCSTR JsModuleList<false>::moduleList[] = { "chakratest", "chakra", "chakracore", "jscript9test", "jscript9", "chakralstest", "chakrals", "jc" };

template <bool IsPublic>
HRESULT FindJScriptModuleByName(_In_ IDebugSymbols* pSymbols, _Out_ ULONG* pIndex, _Out_ ULONG64* pBase)
{
    HRESULT hr = E_FAIL;

    for (int i = 0; i < _countof(JsModuleList<IsPublic>::moduleList); i++)
    {
        hr = pSymbols->GetModuleByModuleName(JsModuleList<IsPublic>::moduleList[i], 0, pIndex, pBase);
        if (SUCCEEDED(hr))
        {
            break;
        }
    }

    return hr;
}
