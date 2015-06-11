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
    UTest::UTCOMMANDARGS m_CommandArgs;

public:
    DummyTestGroup()
        : m_fLastTestCaseFailed(false)
    {
        ZeroMemory(&m_CommandArgs, sizeof(m_CommandArgs));
        m_CommandArgs.m_fDebugBreakEnabled = true; // Enable DebugBreak
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
        return m_CommandArgs.m_fVerboseEnabled;
    }
};

// Use a dummy test group for current scope so that we can use IEUT helpers.
#define UT_USE_DUMMY_TEST_GROUP() \
    bool m_fLastTestCaseFailed = false; \
    UTest::UTCOMMANDARGS m_CommandArgs = {0}; \

// Ensure com hr succeeded. Otherwise fail and print error message.
#define UT_COM_SUCCEEDED(e) \
{ \
    HRESULT _hr_ = (e); \
    if (FAILED(_hr_)) \
    { \
        m_fLastTestCaseFailed = true; \
        _com_error err(_hr_); \
        UT_COLOR_TRACE(UT_COLOR_ERROR, UT_T(UT_PREFIX_ERROR) UT_T("Error 0x%X: %s %s"), _hr_, err.ErrorMessage(), UT_T(#e)); \
        UTEST_PRINT_FILE_LINE_FUNC; \
        UTEST_DEBUG_BREAK; \
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
// Helper for finding jscript9 module in the debuggee process
//
template <bool IsPublic>
struct JsModuleList
{
    static PCSTR moduleList[];
};
PCSTR JsModuleList<true>::moduleList[] = { "chakra", "jscript9" }; // Only support jscript9 and chakra for public tool
PCSTR JsModuleList<false>::moduleList[] = { "chakratest", "chakra", "jscript9test", "jscript9", "chakralstest", "chakrals", "jc" };

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
