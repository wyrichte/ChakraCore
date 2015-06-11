//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

class ScriptDebugSite:
    public ComObjectRoot,
    public IScriptDebugSite
{
private:
    std::wstring m_module;

    ExtExtension* m_pExtention;
    CComPtr<IDebugSymbols> m_pDebugSymbols;
    CComPtr<IDebugSymbols3> m_pDebugSymbols3;
    CComPtr<IDebugDataSpaces> m_pDebugDataSpaces;
    CComPtr<IDebugDataSpaces4> m_pDebugDataSpaces4;

public:
    ScriptDebugSite(void);
    ~ScriptDebugSite(void);

    HRESULT Init(LPCWSTR pModule, ExtExtension* pExtention,
        IDebugSymbols* pDebugSymbols, IDebugSymbols3* pDebugSymbols3,
        IDebugDataSpaces* pDebugDataSpaces, IDebugDataSpaces4* pDebugDataSpaces4);

    // *** IUnknown ***
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj);

    // *** IScriptDebugSite ***
    STDMETHODIMP Out(LPCWSTR msg);
    STDMETHODIMP FindSymbol(LPCWSTR name, const void** addr);
    STDMETHODIMP ReadVirtual(const void* addr,
        _Out_writes_bytes_to_(bufferSize, *bytesRead) void* buffer, ULONG bufferSize, ULONG* bytesRead);
    STDMETHODIMP ReadString(const void* addr,
        _Out_writes_(bufferSize) LPWSTR buffer, ULONG bufferSize);
    STDMETHODIMP AddSyntheticModule(const void* addr, ULONG size);
    STDMETHODIMP AddSyntheticSymbol(const void* addr, ULONG size, LPCWSTR name);
};
