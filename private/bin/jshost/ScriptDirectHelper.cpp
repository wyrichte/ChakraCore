/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"

HRESULT ScriptDirect::JsVarToScriptDirect(Var instance, IActiveScriptDirect** ppScriptDirect)
{
    return JScript9Interface::JsVarToScriptDirect(instance, ppScriptDirect);
}

HRESULT ScriptDirect::JsVarAddRef(Var instance)
{
    return JScript9Interface::JsVarAddRef(instance);
}

HRESULT ScriptDirect::JsVarRelease(Var instance)
{
    return JScript9Interface::JsVarRelease(instance);
}

HRESULT ScriptDirect::JsVarToExtension(Var instance, void** extensionRef)
{
    return JScript9Interface::JsVarToExtension(instance, extensionRef);
}

HRESULT ScriptDirect::From(IActiveScript* pActiveScript)
{
    HRESULT hr = S_OK;

    IfFailGo(pActiveScript->QueryInterface(&m_pScriptDirect));
    IfFailGo(m_pScriptDirect->GetDefaultTypeOperations(&m_pTypeOperations));
    IfFailGo(m_pScriptDirect->GetJavascriptOperations(&m_pJavascriptOperators));
Error:
    return hr;
}

HRESULT ScriptDirect::From(Var var)
{
    HRESULT hr = S_OK;

    IfFailGo(JsVarToScriptDirect(var, &m_pScriptDirect));
    IfFailGo(m_pScriptDirect->GetDefaultTypeOperations(&m_pTypeOperations));
    IfFailGo(m_pScriptDirect->GetJavascriptOperations(&m_pJavascriptOperators));
Error:
    return hr;
}

Var ScriptDirect::GetUndefined()
{
    Var undefined = NULL;
    if (m_pScriptDirect)
    {
        m_pScriptDirect->GetUndefined(&undefined);
    }
    return undefined;
}

Var ScriptDirect::GetNull()
{
    Var value = NULL;
    if (m_pScriptDirect)
    {
        m_pScriptDirect->GetNull(&value);
    }
    return value;
}

HRESULT ScriptDirect::GetProperty(Var obj, PropertyId propertyId, Var* value)
{
    return (SUCCEEDED(m_pJavascriptOperators->GetProperty(m_pScriptDirect, obj, propertyId, value))) ? S_OK : E_FAIL;
}

HRESULT ScriptDirect::GetOwnProperty(Var obj, PropertyId propertyId, Var* value)
{
    BOOL result;
    return (SUCCEEDED(m_pTypeOperations->GetOwnProperty(m_pScriptDirect, obj, propertyId, value, &result)) && result) ? S_OK : E_FAIL;
}

HRESULT ScriptDirect::SetProperty(Var obj, PropertyId propertyId, Var value)
{
    BOOL result;
    return (SUCCEEDED(m_pTypeOperations->SetProperty(m_pScriptDirect, obj, propertyId, value, &result)) && result) ? S_OK : E_FAIL;
}

HRESULT ScriptDirect::GetItem(Var obj, Var index, Var* value)
{
    BOOL result;
    return (SUCCEEDED(m_pTypeOperations->GetOwnItem(m_pScriptDirect, obj, index, value, &result)) && result) ? S_OK : E_FAIL;
}

HRESULT ScriptDirect::GetItem(Var obj, int index, Var* value)
{
    BOOL result;
    Var indexVar = nullptr;
    this->m_pScriptDirect->IntToVar(index, &indexVar);
    return (SUCCEEDED(m_pTypeOperations->GetOwnItem(m_pScriptDirect, obj, indexVar, value, &result)) && result) ? S_OK : E_FAIL;
}

HRESULT ScriptDirect::SetItem(Var obj, Var index, Var value)
{
    BOOL result;
    return (SUCCEEDED(m_pTypeOperations->SetItem(m_pScriptDirect, obj, index, value, &result)) && result) ? S_OK : E_FAIL;
}

HRESULT ScriptDirect::SetItem(Var obj, int index, Var value)
{
    BOOL result;
    Var indexVar = nullptr;
    this->m_pScriptDirect->IntToVar(index, &indexVar);
    return (SUCCEEDED(m_pTypeOperations->SetItem(m_pScriptDirect, obj, indexVar, value, &result)) && result) ? S_OK : E_FAIL;
}

HRESULT ScriptDirect::AddProperty(Var obj, LPCWSTR name, Var value)
{
    HRESULT hr = S_OK;

    PropertyId propertyId;
    IfFailGo(m_pScriptDirect->GetOrAddPropertyId(name, &propertyId));
    IfFailGo(SetProperty(obj, propertyId, value));

Error:
    return hr;
}

HRESULT ScriptDirect::AddMethod(Var obj, LPCWSTR name, ScriptMethod entryPoint)
{
    HRESULT hr = S_OK;

    PropertyId propertyId;
    IfFailGo(m_pScriptDirect->GetOrAddPropertyId(name, &propertyId));

    Var func;
    IfFailGo(m_pScriptDirect->BuildDOMDirectFunction(NULL, entryPoint, propertyId, -1, 0, &func));
    IfFailGo(SetProperty(obj, propertyId, func));

Error:
    return hr;
}

HRESULT ScriptDirect::HasProperty(Var obj, LPCWSTR name, BOOL* result)
{
    HRESULT hr = S_OK;

    PropertyId propertyId;
    IfFailGo(m_pScriptDirect->GetOrAddPropertyId(name, &propertyId));
    IfFailGo(m_pTypeOperations->HasOwnProperty(m_pScriptDirect, obj, propertyId, result));

Error:
    return hr;
}

HRESULT ScriptDirect::GetProperty(Var obj, LPCWSTR name, Var* value)
{
    HRESULT hr = S_OK;

    PropertyId propertyId;
    IfFailGo(m_pScriptDirect->GetOrAddPropertyId(name, &propertyId));
    IfFailGo(GetProperty(obj, propertyId, value));

Error:
    return hr;
}

template<class T, class Func>
HRESULT ScriptDirect::GetProperty(Var obj, LPCWSTR name, T* value, const Func& convert)
{
    HRESULT hr = S_OK;

    Var valueVar;
    IfFailGo(GetProperty(obj, name, &valueVar));
    IfFailGo(convert(*this, valueVar, value));

Error:
    return hr;
}

HRESULT ScriptDirect::GetProperty(Var obj, LPCWSTR name, int* value)
{
    return GetProperty(obj, name, value,
        [](ScriptDirect& pScriptDirect, Var var, int* value) {
            return pScriptDirect->VarToInt(var, value);
    });
}

HRESULT ScriptDirect::GetProperty(Var obj, LPCWSTR name, double* value)
{
    return GetProperty(obj, name, value,
        [](ScriptDirect& pScriptDirect, Var var, double* value) {
            return pScriptDirect->VarToDouble(var, value);
    });
}

HRESULT ScriptDirect::GetProperty(Var obj, LPCWSTR name, ULONGLONG* value)
{
    return GetProperty(obj, name, value,
        [](ScriptDirect& pScriptDirect, Var var, ULONGLONG* value) {
            return pScriptDirect->VarToUInt64(var, value);
    });
}

HRESULT ScriptDirect::GetProperty(Var obj, LPCWSTR name, BSTR* value)
{
    return GetProperty(obj, name, value,
        [](ScriptDirect& pScriptDirect, Var var, BSTR* value) {
            return pScriptDirect->VarToString(var, value);
    });
}

HRESULT ScriptDirect::GetOwnProperty(Var obj, LPCWSTR name, Var* value)
{
    HRESULT hr = S_OK;

    PropertyId propertyId;
    IfFailGo(m_pScriptDirect->GetOrAddPropertyId(name, &propertyId));
    IfFailGo(GetOwnProperty(obj, propertyId, value));

Error:
    return hr;
}

template<class T, class Func>
HRESULT ScriptDirect::GetOwnProperty(Var obj, LPCWSTR name, T* value, const Func& convert)
{
    HRESULT hr = S_OK;

    Var valueVar;
    IfFailGo(GetOwnProperty(obj, name, &valueVar));
    IfFailGo(convert(*this, valueVar, value));

Error:
    return hr;
}

HRESULT ScriptDirect::GetOwnProperty(Var obj, LPCWSTR name, int* value)
{
    return GetOwnProperty(obj, name, value,
        [](ScriptDirect& pScriptDirect, Var var, int* value) {
        return pScriptDirect->VarToInt(var, value);
    });
}

HRESULT ScriptDirect::GetOwnProperty(Var obj, LPCWSTR name, double* value)
{
    return GetOwnProperty(obj, name, value,
        [](ScriptDirect& pScriptDirect, Var var, double* value) {
        return pScriptDirect->VarToDouble(var, value);
    });
}

HRESULT ScriptDirect::GetOwnProperty(Var obj, LPCWSTR name, ULONGLONG* value)
{
    return GetOwnProperty(obj, name, value,
        [](ScriptDirect& pScriptDirect, Var var, ULONGLONG* value) {
        return pScriptDirect->VarToUInt64(var, value);
    });
}

HRESULT ScriptDirect::GetOwnProperty(Var obj, LPCWSTR name, BSTR* value)
{
    return GetOwnProperty(obj, name, value,
        [](ScriptDirect& pScriptDirect, Var var, BSTR* value) {
        return pScriptDirect->VarToString(var, value);
    });
}

HRESULT ScriptDirect::StringToVar(LPCWSTR str, Var* value)
{
    size_t len = wcslen(str);
    return m_pScriptDirect->StringToVar(str, static_cast<int>(len), value);
}
HRESULT ScriptDirect::VarToString(Var value, BSTR* str)
{
    return m_pScriptDirect->VarToString(value, str);
}

HRESULT ScriptDirect::NewDate(double time, Var* pValue)
{
    HRESULT hr = S_OK;

    Var global;
    IfFailGo(m_pScriptDirect->GetGlobalObject(&global));

    Var date;
    IfFailGo(GetProperty(global, _u("Date"), &date));

    Var timeVar;
    IfFailGo(m_pScriptDirect->DoubleToVar(time, &timeVar));

    Var args[] = {date, timeVar};
    CallInfo callInfo;
    callInfo.Flags = CallFlags_New;
    callInfo.Count = _countof(args);
    IfFailGo(m_pScriptDirect->Execute(date, callInfo, args, NULL, pValue));

Error:
    return hr;
}

void ScriptDirect::ThrowIfFailed(HRESULT hr, LPCWSTR msg)
{
    // Check SCRIPT_E_RECORDED
    hr = CheckRecordedException(hr);

    CComPtr<IJavascriptOperations> pJavascriptOperations;
    if (FAILED(hr) && m_pScriptDirect
        && SUCCEEDED(m_pScriptDirect->GetJavascriptOperations(&pJavascriptOperations)))
    {
        const size_t bufsize = 256;
        static WCHAR buf[bufsize];

        Var err;
        if (SUCCEEDED(StringCchPrintf(buf, bufsize, _u("%8X %s"), hr, msg))
            && SUCCEEDED(m_pScriptDirect->CreateErrorObject(JsErrorType::CustomError, hr, buf, &err)))
        {
            pJavascriptOperations->ThrowException(m_pScriptDirect, err, FALSE);
        }
    }
}

//
// Check for SCRIPT_E_RECORDED and rethrow after calling into a script engine.
//
HRESULT ScriptDirect::CheckRecordedException(HRESULT hr)
{
    if (hr == SCRIPT_E_RECORDED && m_pScriptDirect)
    {
        m_pTypeOperations = NULL;

        // Detach m_pScriptDirect
        IActiveScriptDirect* pScriptDirect = m_pScriptDirect;
        pScriptDirect->AddRef(); // Keep a ref
        m_pScriptDirect = NULL;

        pScriptDirect->ReleaseAndRethrowException(hr);
    }

    return hr;
}


//
// Add a new byte value into the buffer.
//
HRESULT ByteBufferContainer::Add(ScriptDirect& pScriptDirect, Var value)
{
    HRESULT hr = S_OK;

    int byte;
    IfFailGo(pScriptDirect->VarToInt(value, &byte));

    if (m_cur < m_length)
    {
        m_pBuffer[m_cur++] = static_cast<BYTE>(byte);
    }
    else
    {
        hr = E_FAIL;
    }

Error:
    return hr;
}