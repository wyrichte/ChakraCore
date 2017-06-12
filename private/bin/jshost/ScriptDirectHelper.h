/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once

// IActiveScriptDirect helper
class ScriptDirect
{
private:
    CComPtr<IActiveScriptDirect> m_pScriptDirect;
    CComPtr<ITypeOperations> m_pTypeOperations;
    CComPtr<IJavascriptOperations> m_pJavascriptOperators;

    template<class T, class Func>
    HRESULT GetProperty(Var obj, LPCWSTR name, T* value, const Func& convert);

    template<class T, class Func>
    HRESULT GetOwnProperty(Var obj, LPCWSTR name, T* value, const Func& convert);

public:
    static HRESULT JsVarToScriptDirect(Var instance, IActiveScriptDirect** ppScriptDirect);
    static HRESULT JsVarAddRef(Var instance);
    static HRESULT JsVarRelease(Var instance);
    static HRESULT JsVarToExtension(Var instance, void** extensionRef);    

    HRESULT From(IActiveScript* pActiveScript);
    HRESULT From(Var var);

    IActiveScriptDirect* operator->()
    {
        return m_pScriptDirect;
    }

    Var GetUndefined();
    Var GetNull();

    HRESULT GetProperty(Var obj, PropertyId propertyId, Var* value);
    HRESULT GetProperty(Var obj, LPCWSTR name, Var* value);
    HRESULT GetProperty(Var obj, LPCWSTR name, int* value);
    HRESULT GetProperty(Var obj, LPCWSTR name, double* value);
    HRESULT GetProperty(Var obj, LPCWSTR name, ULONGLONG* value);
    HRESULT GetProperty(Var obj, LPCWSTR name, BSTR* value);

    HRESULT GetOwnProperty(Var obj, PropertyId propertyId, Var* value);
    HRESULT GetOwnProperty(Var obj, LPCWSTR name, Var* value);
    HRESULT GetOwnProperty(Var obj, LPCWSTR name, int* value);
    HRESULT GetOwnProperty(Var obj, LPCWSTR name, double* value);
    HRESULT GetOwnProperty(Var obj, LPCWSTR name, ULONGLONG* value);
    HRESULT GetOwnProperty(Var obj, LPCWSTR name, BSTR* value);

    HRESULT SetProperty(Var obj, PropertyId propertyId, Var value);
    HRESULT GetItem(Var obj, Var index, Var* value);
    HRESULT GetItem(Var obj, int index, Var* value);
    HRESULT SetItem(Var obj, Var index, Var value);
    HRESULT SetItem(Var obj, int index, Var value);

    HRESULT AddProperty(Var obj, LPCWSTR name, Var value);
    HRESULT AddMethod(Var obj, LPCWSTR name, ScriptMethod entryPoint);
    HRESULT HasProperty(Var obj, LPCWSTR name, BOOL* result);

    HRESULT StringToVar(LPCWSTR str, Var* value);
    HRESULT VarToString(Var value, BSTR* str);
    HRESULT NewDate(double time, Var* pValue);

    void ThrowIfFailed(HRESULT hr, LPCWSTR msg = _u(""));
    HRESULT CheckRecordedException(HRESULT hr);

    template <class T>
    HRESULT ReadArray(Var data, T* container);
};

//
// Read array-like data and add elements into a container.
//
template <class T>
HRESULT ScriptDirect::ReadArray(Var data, T* container)
{
    HRESULT hr = S_OK;

    int len;
    IfFailGo(GetProperty(data, _u("length"), &len));

    for (int i = 0; i < len; i++) {
        Var index, value;
        IfFailGo(m_pScriptDirect->IntToVar(i, &index));
        IfFailGo(GetItem(data, index, &value));

        IfFailGo(container->Add(*this, value));
    }

Error:
    return hr;
}

//
// A container for reading bytes from a JavaScript int array into a buffer.
//
class ByteBufferContainer
{
private:
    BYTE* m_pBuffer;
    UINT m_length;
    UINT m_cur;

public:
    ByteBufferContainer(BYTE* pBuffer, UINT length)
        : m_pBuffer(pBuffer), m_length(length), m_cur(0)
    {
    }

    HRESULT Add(ScriptDirect& pScriptDirect, Var value);
};