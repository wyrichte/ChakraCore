/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once

// SCA test hook
class SCA
{
private:
    static HTYPE htypeImageData;
    static CComPtr<ITypeOperations> s_pImageDataTypeOperations;

private:
    static HRESULT EnsureImageDataType(ScriptDirect& pScriptDirect);
    static HRESULT GetOptions(ScriptDirect& pScriptDirect, CallInfo callInfo, Var* args,
        ISCAContext** ppSCAContext, _Outptr_opt_result_maybenull_ BSTR* pFileName = NULL, unsigned int argIndex = 2);
    static HRESULT GetInStream(ScriptDirect& pScriptDirect, Var data, IStream** ppStream);

    static HRESULT CreateMemStream(_In_reads_bytes_opt_(len) const BYTE* pb, UINT len, IStream** ppStream);
    static HRESULT CreateMemStream(IStream** ppStream);

    // EntryPoints
    static Var MakeInt64(Var function, CallInfo callInfo, ...);
    static Var MakeUint64(Var function, CallInfo callInfo, ...);
    static Var __cdecl ImageDataConstructor(Var function, CallInfo callInfo, Var _this, ...);
    static Var Serialize(Var function, CallInfo callInfo, ...);
    static Var Deserialize(Var function, CallInfo callInfo, ...);
    template <class APIFuncType>
    static Var LookupHelper(_In_ APIFuncType ReadIndexableProperty, _In_ Var function, _In_ CallInfo callInfo, _In_ Var* args); //Common method for LookupEx and Lookup, since both just differ by func
    static Var LookupEx(_In_ Var function, _In_ CallInfo callInfo, ...);
    static Var Lookup(_In_ Var function, _In_ CallInfo callInfo, ...);
    static Var DataToKey(_In_ Var function, _In_ CallInfo callInfo, ...);

    static HRESULT VariantArrayToVar(_In_ ScriptDirect& pScriptDirect, _In_ const VARIANT &value, _Out_ Var* outVar);
    static HRESULT VariantToVar(_In_ ScriptDirect& pScriptDirect, _In_ const VARIANT &value, _Out_ Var* outVar);
public:
    static HRESULT Initialize(IActiveScript* pActiveScript);
    static HRESULT CreateImageDataObject(
        ScriptDirect& pScriptDirect, Var* value, MockImageData** ppImageData = NULL);
};

//
// A container for reading bytes from a JavaScript int array, using a stream as backstore.
//
class ByteStreamContainer
{
private:
    IStream* m_pStream;

public:
    ByteStreamContainer(IStream* pStream)
        : m_pStream(pStream)
    {
    }

    HRESULT Add(ScriptDirect& pScriptDirect, Var value);
};

//
// A container for reading strings from a JavaScript string array.
//
class BSTRContainer
{
private:
    std::vector<BSTR> m_bstrArr;

public:
    ~BSTRContainer();

    UINT Count() const
    {
        return static_cast<UINT>(m_bstrArr.size());
    }

    BSTR* Items()
    {
        return &(*m_bstrArr.begin());
    }

    HRESULT Add(ScriptDirect& pScriptDirect, Var value);
};
