//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "StdAfx.h"
#include <comdef.h>
#include "ComObject.h"
#include "ScriptDebugSite.h"

ScriptDebugSite::ScriptDebugSite(void)
{
}

ScriptDebugSite::~ScriptDebugSite(void)
{
}

HRESULT ScriptDebugSite::Init(LPCWSTR pModule, ExtExtension* pExtention,
                        IDebugSymbols* pDebugSymbols, IDebugSymbols3* pDebugSymbols3,
                        IDebugDataSpaces* pDebugDataSpaces, IDebugDataSpaces4* pDebugDataSpaces4)
{
    m_module = pModule;
    m_module += _u("!");

    m_pExtention = pExtention;
    m_pDebugSymbols = pDebugSymbols;
    m_pDebugSymbols3 = pDebugSymbols3;
    m_pDebugDataSpaces = pDebugDataSpaces;
    m_pDebugDataSpaces4 = pDebugDataSpaces4;

    return S_OK;
}

STDMETHODIMP ScriptDebugSite::QueryInterface(REFIID riid, void** ppvObj)
{
    IfNullReturnError(ppvObj, E_POINTER);

    QI_IMPL_INTERFACE(IUnknown);
    QI_IMPL_INTERFACE(IScriptDebugSite);

    *ppvObj = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP ScriptDebugSite::Out(LPCWSTR msg)
{
    m_pExtention->Out(msg);
    return S_OK;
}

STDMETHODIMP ScriptDebugSite::FindSymbol(LPCWSTR name, const void** addr)
{
    IfNullReturnError(name, E_INVALIDARG);
    IfNullReturnError(addr, E_INVALIDARG);

    std::wstring symbol = m_module + name;
    ULONG64 offset;

    HRESULT hr = m_pDebugSymbols3->GetOffsetByNameWide(symbol.c_str(), &offset);
    if (SUCCEEDED(hr))
    {
        *addr = reinterpret_cast<LPVOID>(offset);
    }
    return hr;
}

STDMETHODIMP ScriptDebugSite::ReadVirtual(const void* addr,
    _Out_writes_bytes_to_(bufferSize, *bytesRead) void* buffer, ULONG bufferSize, ULONG* bytesRead)
{
    ULONG64 offset = reinterpret_cast<ULONG64>(addr);
    return m_pDebugDataSpaces->ReadVirtual(offset, buffer, bufferSize, bytesRead);
}

STDMETHODIMP ScriptDebugSite::ReadString(const void* addr,
    _Out_writes_(bufferSize) LPWSTR buffer, ULONG bufferSize)
{
    ULONG64 offset = reinterpret_cast<ULONG64>(addr);
    ULONG maxBytes = bufferSize * sizeof(WCHAR);
    HRESULT hr = m_pDebugDataSpaces4->ReadUnicodeStringVirtualWide(offset, maxBytes, buffer, bufferSize, NULL);

    // If no NULL terminater found yet, terminate the string and return S_FALSE.
    if (hr == E_INVALIDARG)
    {
        buffer[bufferSize - 1] = 0;
        hr = S_FALSE;
    }
    return hr;
}

STDMETHODIMP ScriptDebugSite::AddSyntheticModule(const void* addr, ULONG size)
{
    ULONG64 offset = reinterpret_cast<ULONG64>(addr);

    // Remove previously added synthetic module first. This discards its synthetic symbols as well.
    m_pDebugSymbols3->RemoveSyntheticModule(offset);

    ULONG flags = DEBUG_ADDSYNTHMOD_ZEROBASE;
    HRESULT hr = m_pDebugSymbols3->AddSyntheticModule(offset, size, "_js_", "_js_", flags);
    if (FAILED(hr))
    {
        ULONG64 end = offset + size;
#ifdef _M_X64
        m_pExtention->Err("*** ERROR: Failed to add synthetic module: %08x`%08x - %08x`%08x\n",
            HILONG(offset), LOLONG(offset), HILONG(end), LOLONG(end));
#else
        m_pExtention->Err("*** ERROR: Failed to add synthetic module: %p - %p\n", offset, end);
#endif
        _com_error err(hr);
        m_pExtention->Err("    0x%08X: %S\n", hr, err.ErrorMessage());
    }

    return hr;
}

STDMETHODIMP ScriptDebugSite::AddSyntheticSymbol(const void* addr, ULONG size, LPCWSTR name)
{
    // On ARM the least significant bit indicates thumb mode. Ignore it here.
#ifdef _M_ARM
    addr = (LPCVOID)((UINT_PTR)addr & (((UINT_PTR)-1) - 1));
#endif

    ULONG64 offset = reinterpret_cast<ULONG64>(addr);

    HRESULT hr = m_pDebugSymbols3->AddSyntheticSymbolWide(offset, size, name, DEBUG_ADDSYNTHSYM_DEFAULT, NULL);
    if (hr == 0x8007007E)
    {
        // When adding the first synthetic symbol into a synthetic module, we'll receive error 0x8007007E:
        // The specified module could not be found. Workaround is to add again.
        hr = m_pDebugSymbols3->AddSyntheticSymbolWide(offset, size, name, DEBUG_ADDSYNTHSYM_DEFAULT, NULL);
    }

    if (FAILED(hr))
    {
        ULONG64 end = offset + size;
#ifdef _M_X64
        m_pExtention->Err("*** ERROR: Failed to add synthetic symbol: %08x`%08x - %08x`%08x %S\n",
            HILONG(offset), LOLONG(offset), HILONG(end), LOLONG(end), name);
#else
        m_pExtention->Err("*** ERROR: Failed to add synthetic symbol: %p - %p %S\n", offset, end, name);
#endif
        _com_error err(hr);
        m_pExtention->Err("    0x%08X: %S\n", hr, err.ErrorMessage());
    }

    return hr;
}

EXT_COMMAND(ldsym,
    "Load JavaScript symbols",
    "{p;x;path;Path to the jscript debugger helper}")
{
    // Find jscript9 module
    ULONG index = DEBUG_ANY_ID;
    ULONG64 base;
    IfFailThrow(FindJScriptModuleByName<JD_IS_PUBLIC>(m_Symbols, &index, &base),
        "Failed to find the jscript module in the process");

    WCHAR moduleName[MAX_PATH], imageName[MAX_PATH];
    IfFailThrow(m_Symbols3->GetModuleNameStringWide(DEBUG_MODNAME_MODULE, index, base, moduleName, _countof(moduleName), NULL),
        "Failed to find the jscript module name");
    if (HasArg("p"))
    {
        CA2W path = GetArgStr("p", false);
        wcscpy_s(imageName, path);
    }
    else
    {
        IfFailThrow(m_Symbols3->GetModuleNameStringWide(DEBUG_MODNAME_IMAGE, index, base, imageName, _countof(imageName), NULL),
            "Failed to find the jscript module path");
    }
    Out(_u("Use %s\n"), imageName);

    const CLSID CLSID_JScript9DAC = { 0x197060cb, 0x5efb, 0x4a53, 0xb0, 0x42, 0x93, 0x9d, 0xbb, 0x31, 0x62, 0x7c };
    CComPtr<IScriptDAC> pDAC;
    IfFailThrow(PrivateCoCreate(imageName, CLSID_JScript9DAC, IID_PPV_ARGS(&pDAC)),
        "Failed to create the jscript debug helper object; ensure the platform architecture of the debugger and jsd matches that of the process being debugged, and the version and path of jscript being loaded by jsd is correct.");

    CComPtr<ScriptDebugSite> pScriptDebugSite;
    IfFailThrow(ComObject<ScriptDebugSite>::CreateInstance(&pScriptDebugSite));
    IfFailThrow(pScriptDebugSite->Init(moduleName, this, m_Symbols, m_Symbols3, m_Data, m_Data4));

    IfFailThrow(pDAC->LoadScriptSymbols(pScriptDebugSite));
}