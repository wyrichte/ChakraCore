//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "StdAfx.h"

SinkDebugSite::SinkDebugSite()
{
}

SinkDebugSite::~SinkDebugSite()
{
}

HRESULT SinkDebugSite::Init(EXT_CLASS* ext, LPCTSTR pModule, IDebugSymbols3* pDebugSymbols, IDebugDataSpaces4* pDebugDataSpaces)
{
    try
    {
        m_module = pModule;
        m_module += _T('!');
    }
    catch(const std::bad_alloc&)
    {
        return E_OUTOFMEMORY;
    }
 
    m_ext = ext;
    m_pDebugSymbols = pDebugSymbols;
    m_pDebugDataSpaces = pDebugDataSpaces;
    return S_OK;
}

STDMETHODIMP SinkDebugSite::Out(LPCWSTR msg)
{
    m_ext->Out(L"%s\n", msg);
    return S_OK;
}

STDMETHODIMP SinkDebugSite::FindSymbol(LPCWSTR name, const void** addr)
{
    IfNullReturnError(name, E_INVALIDARG);
    IfNullReturnError(addr, E_INVALIDARG);
    HRESULT hr = S_OK;

    string symbol = m_module + name;
    ULONG64 offset;

    hr = m_pDebugSymbols->GetOffsetByNameT(symbol.c_str(), &offset);
    if (SUCCEEDED(hr))
    {
        *addr = reinterpret_cast<LPVOID>(offset);
    }
    return hr;
}

STDMETHODIMP SinkDebugSite::ReadVirtual(const void* addr,
    _Out_writes_bytes_to_(bufferSize, *bytesRead) void* buffer, ULONG bufferSize, ULONG* bytesRead)
{
    IfNullReturnError(addr, E_INVALIDARG);
    IfNullReturnError(buffer, E_INVALIDARG);
    IfNullReturnError(bytesRead, E_INVALIDARG);
    
    ULONG64 offset = reinterpret_cast<ULONG64>(addr);
    return m_pDebugDataSpaces->ReadVirtual(offset, buffer, bufferSize, bytesRead);
}

STDMETHODIMP SinkDebugSite::ReadString(const void* addr,
    _Out_writes_(bufferSize) LPWSTR buffer, ULONG bufferSize)
{
    IfNullReturnError(addr, E_INVALIDARG);
    IfNullReturnError(buffer, E_INVALIDARG);
    
    ULONG64 offset = reinterpret_cast<ULONG64>(addr);
    ULONG maxBytes = bufferSize * sizeof(WCHAR);
    HRESULT hr = m_pDebugDataSpaces->ReadUnicodeStringVirtualWide(offset, maxBytes, buffer, bufferSize, NULL);

    // If no NULL terminater found yet, terminate the string and return S_FALSE.
    if (hr == E_INVALIDARG)
    {
        buffer[bufferSize - 1] = 0;
        hr = S_FALSE;
    }
    return hr;
}

STDMETHODIMP SinkDebugSite::AddSyntheticModule(const void* addr, ULONG size)
{
    HRESULT hr = S_OK;

    // Verify the module is valid
    IfFalseAssertReturnEFAIL(addr != NULL);
    IfFalseAssertReturnEFAIL(size > 0);

    LPCVOID end = reinterpret_cast<const BYTE*>(addr) + size;
    interval_type m(addr, end);
    IfFalseAssertReturnEFAIL(m.begin <= m.end);

    // Verify the module doesn't overlap with any existing module
    module_info_list::const_iterator i = std::find_if(m_modules.begin(), m_modules.end(),
        std::bind2nd(std::mem_fun_ref(&interval_type::overlaps), m));
    if (i != m_modules.end())
    {
        wprintf(L"Error: Synthetic module %p-%p overlaps with previous module %p-%p\n", m.begin, m.end, i->begin, i->end);
        IfFalseAssertReturnEFAIL(false);
    }

    // Good to go
    try
    {
        m_modules.push_back(m);
    }
    catch(const std::bad_alloc&)
    {
        hr = E_OUTOFMEMORY;
    }

    if (TraceVerbose())
    {
        wprintf(L"Synthetic module %p-%p\n", m.begin, m.end);
    }

    return hr;
}

STDMETHODIMP SinkDebugSite::AddSyntheticSymbol(const void* addr, ULONG size, LPCWSTR name)
{
    HRESULT hr = S_OK;

    // Verify the symbol is valid
    IfFalseAssertReturnEFAIL(addr != NULL);
    IfFalseAssertReturnEFAIL(size > 0);
    IfFalseAssertReturnEFAIL(name != NULL);

    LPCVOID end = reinterpret_cast<const BYTE*>(addr) + size;
    CW2T tname(name);
    symbol_info sym(addr, end, string(tname));
    IfFalseAssertReturnEFAIL(sym.begin <= sym.end);
    IfFalseAssertReturnEFAIL(!sym.name.empty());

    // Verify the symbol doesn't overlap with any existing symbol
    symbol_info_list::const_iterator i = std::find_if(m_symbols.begin(), m_symbols.end(),
        std::bind2nd(std::mem_fun_ref(&symbol_info::overlaps), sym));
    if (i != m_symbols.end())
    {
        wprintf(L"Error: Synthetic symbol %p-%p (%s) overlaps with previous symbol %p-%p (%s)\n", sym.begin, sym.end, sym.name.c_str(), i->begin, i->end, i->name.c_str());

        IfFalseAssertReturnEFAIL(false);
    }

    // Verify the symbol is contained by some existing module
    module_info_list::const_iterator m = std::find_if(m_modules.begin(), m_modules.end(),
        std::bind2nd(std::mem_fun_ref(&interval_type::contains), sym));
    if (m == m_modules.end())
    {
        wprintf(L"Error: Synthetic symbol %p-%p (%s) isn't contained by any existing synthetic module.\n", sym.begin, sym.end, sym.name.c_str());
        IfFalseAssertReturnEFAIL(false);
    }

    // Good to go
    try
    {
        m_symbols.push_back(sym);
    }
    catch(const std::bad_alloc&)
    {
        hr = E_OUTOFMEMORY;
    }

    if (TraceVerbose())
    {
       wprintf(L"Synthetic symbol %p-%p %s\n", sym.begin, sym.end, sym.name.c_str());
    }
    return hr;
}

void SinkDebugSite::PrintSymbolNames()
{
    // Use a set to print sorted unique symbol names
    std::set<string> names;

    std::for_each(m_symbols.begin(), m_symbols.end(), [&](const symbol_info& sym){
        string name = sym.name;

        // Trim paths so that we can compare with baseline
        string::size_type i = name.find(_T('('));
        if (i != string::npos)
        {
            string::size_type end = name.find(_T(')'), i);
            if (end != string::npos)
            {
                end = name.rfind(_T('\\'), end);
                if (end != string::npos)
                {
                    name = name.substr(0, i + 1) + name.substr(end + 1);
                }
            }
        }

        ExpressionNoThrow(names.insert(name));
    });

    std::for_each(names.begin(), names.end(), [&](const string& name){
        m_ext->Out(_T("%s\n"), name.c_str());
    });
}
