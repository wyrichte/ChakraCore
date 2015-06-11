//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

//
// Represents an interval [begin, end).
//
template <class T>
struct Interval
{
    T begin;
    T end;

    Interval(T begin, T end)
        : begin(begin), end(end)
    {
        ASSERT(begin <= end);
    }

    bool overlaps(const Interval& i) const
    {
        return !(this->begin >= i.end || this->end <= i.begin);
    }

    bool contains(const Interval& i) const
    {
        return this->begin <= i.begin && this->end >= i.end;
    }
};

//
// This implements and verifies IScriptDebugSite.
//
class SinkDebugSite:
    public CComObjectRoot,
    public DummyTestGroup,
    public IScriptDebugSite
{
private:
    typedef Interval<LPCVOID> interval_type;
    typedef std::vector<interval_type> module_info_list;

    struct symbol_info: public interval_type
    {
        string name;

        symbol_info(LPCVOID begin, LPCVOID end, const string& name)
            : interval_type(begin, end), name(name)
        {
        }
    };
    typedef std::vector<symbol_info> symbol_info_list;

    module_info_list m_modules;
    symbol_info_list m_symbols;

    EXT_CLASS* m_ext;
    string m_module;
    CComPtr<IDebugSymbols3> m_pDebugSymbols;
    CComPtr<IDebugDataSpaces4> m_pDebugDataSpaces;

public:
    SinkDebugSite();
    ~SinkDebugSite();

    BEGIN_COM_MAP(SinkDebugSite)
        COM_INTERFACE_ENTRY(IScriptDebugSite)
    END_COM_MAP()

    // *** IScriptDebugSite ***
    STDMETHODIMP Out(LPCWSTR msg);
    STDMETHODIMP FindSymbol(LPCWSTR name, const void** addr);
    STDMETHODIMP ReadVirtual(const void* addr,
        _Out_writes_bytes_to_(bufferSize, *bytesRead) void* buffer, ULONG bufferSize, ULONG* bytesRead);
    STDMETHODIMP ReadString(const void* addr,
        _Out_writes_(bufferSize) LPWSTR buffer, ULONG bufferSize);
    STDMETHODIMP AddSyntheticModule(const void* addr, ULONG size);
    STDMETHODIMP AddSyntheticSymbol(const void* addr, ULONG size, LPCWSTR name);

    HRESULT Init(EXT_CLASS* ext, LPCTSTR pModule, IDebugSymbols3* pDebugSymbols, IDebugDataSpaces4* pDebugDataSpaces);
    void PrintSymbolNames();
};
