//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

namespace Js
{
    //
    // Implements IScriptDAC.
    //
    class ScriptDAC:
        public ComObjectBase<IScriptDAC, ScriptDAC>
    {
        friend class ComObjectBase<IScriptDAC, ScriptDAC>;
    private:
        ScriptDAC();

    public:
        // *** IScriptDAC ***
        STDMETHODIMP LoadScriptSymbols(IScriptDebugSite* debugSite);

        static HRESULT Read(IScriptDebugSite* debugSite, LPCVOID addr, void* buffer, ULONG bufferSize);

        template <class T>
        static HRESULT ReadPointer(IScriptDebugSite* debugSite, LPCVOID addr, const T** p)
        {
            return Read(debugSite, addr, p, sizeof(const T*));
        }
    };

    //
    // Helper class for accessing remote debugging target data.
    //
    template <class T>
    class RemoteData
    {
    private:
        BYTE m_data[sizeof(T)];
        const T* remoteAddress;

    public:
        RemoteData():
            remoteAddress(nullptr)
        {
        }

        const T* GetRemoteAddress()
        {
            return remoteAddress;
        }

        HRESULT Read(IScriptDebugSite* debugSite, const T* addr)
        {
            HRESULT hr = ScriptDAC::Read(debugSite, addr, m_data, sizeof(m_data));

            if (SUCCEEDED(hr))
            {
                remoteAddress = addr;
            }

            return hr;
        }

        T* operator->()
        {
            return reinterpret_cast<T*>(&m_data[0]);
        }

        const T* operator->() const
        {
            return reinterpret_cast<const T*>(&m_data[0]);
        }

        operator T*()
        {
            return operator->();
        }

        operator const T*() const
        {
            return operator->();
        }
    };

    template <class T>
    class RemoteWeakReference: public RemoteData<RecyclerWeakReference<T>>
    {
    public:
        T* Get()
        {
            return (T*)((*this)->strongRef);
        }
    };

    //
    // Helper class for accessing remote debugging target array data.
    //
    template <class T>
    class RemoteArray
    {
    private:
        int m_count;
        AutoArrayPtr<T> m_array;

    public:
        RemoteArray(int count)
            : m_count(count), m_array(HeapNewNoThrowArray(T, count), count)
        {
        }

        HRESULT Read(IScriptDebugSite* debugSite, const T* addr)
        {
            IfNullReturnError(m_array, E_OUTOFMEMORY);
            return ScriptDAC::Read(debugSite, addr, m_array, m_count * sizeof(T));
        }

        operator T*()
        {
            return m_array;
        }

        template<class TMapFunction, class TMapPredicate>
        void Map(TMapFunction map, TMapPredicate predicate)
        {
            for (int i = 0; i < m_count; i++)
            {
                if (predicate(m_array[i]))
                {
                    map(i, m_array[i]);
                }
            }
        }

        // Map each element of this remote array.
        template<class TMapFunction>
        void Map(TMapFunction map)
        {
            for (int i = 0; i < m_count; i++)
            {
                map(i, m_array[i]);
            }
        }

    };

    // Map each remote array element with known array address and element count.
    template<class T, class TMapFunction, class TMapPredicate>
    static HRESULT Map(IScriptDebugSite* debugSite, const T* addr, int count, TMapFunction map, TMapPredicate mapPredicate)
    {
        HRESULT hr = S_OK;

        if (addr && count > 0)
        {
            RemoteArray<T> pBuffer(count);
            IfFailGo(pBuffer.Read(debugSite, addr));

            pBuffer.Map(map, mapPredicate);
        }
Error:
        return hr;
    }

    // Map each remote array element with known array address and element count.
    template<class T, class TMapFunction>
    static HRESULT Map(IScriptDebugSite* debugSite, const T* addr, int count, TMapFunction map)
    {
        return Map(debugSite, addr, count, map, [&](const T& address) -> bool 
                { 
                    return true;
                });
    }

    // Map each remote List element.
    template<class List, class TMapFunction>
    static HRESULT Map(IScriptDebugSite* debugSite, const List* addr, TMapFunction map)
    {
        HRESULT hr = S_OK;

        if (addr)
        {
            RemoteData<List> list;
            IfFailGo(list.Read(debugSite, addr));

            IfFailGo(list->Map(debugSite, map));
        }
Error:
        return hr;
    }

    // Map each remote linked list element.
    template<class LinkedListItem, class TMapFunction>
    static HRESULT MapLinkedList(IScriptDebugSite* debugSite, const LinkedListItem* addr, TMapFunction map)
    {
        HRESULT hr = S_OK;

        while (addr)
        {
            RemoteData<LinkedListItem> item;
            IfFailGo(item.Read(debugSite, addr));

            IfFailGo(map(item, &addr)); // Process item and advance to next
        }
Error:
        return hr;
    }

    template <typename Dictionary>
    class RemoteDictionary: public RemoteData<Dictionary>
    {
    public:
        template<class TMapFunction>
        HRESULT Map(IScriptDebugSite* debugSite, TMapFunction mapFunction)
        {
            HRESULT hr = S_OK;

            RemoteArray<Dictionary::EntryType> entries((*this)->count);
            IfFailGo(entries.Read(debugSite, (*this)->entries));

            entries.Map([debugSite, mapFunction] (int entryIndex, Dictionary::EntryType entry) {
                mapFunction(entry.Value());
            });
Error:
            return hr;
        }

    };

    //
    // Represents remote UTF8SourceInfo data.
    //
    class RemoteUtf8SourceInfo: public RemoteData<Utf8SourceInfo>
    {
    public:
        template <typename TMapFunction>
        HRESULT MapFunctions(IScriptDebugSite* debugSite, TMapFunction mapFunction)
        {
            HRESULT hr = S_OK;
            RemoteDictionary<Utf8SourceInfo::FunctionBodyDictionary> functionBodyDictionary;
            IfFailGo(functionBodyDictionary.Read(debugSite, (*this)->functionBodyDictionary));

            IfFailGo(functionBodyDictionary.Map(debugSite, mapFunction));
Error:
            return hr;
        }
    };

    //
    // Represents remote ScriptContext data.
    //
    class RemoteScriptContext: public RemoteData<ScriptContext>
    {
    private:
        HRESULT ReadEmitBufferAllocations(IScriptDebugSite* debugSite);
        static HRESULT AddSyntheticModules(IScriptDebugSite* debugSite, const EmitBufferAllocation* allocation);
        template <typename TMapFunction>
        HRESULT MapFunctions(IScriptDebugSite* debugSite, TMapFunction mapper)
        {
            HRESULT hr = S_OK;

            if ((*this)->cache)
            {
                auto processUtf8SourceContextInfo = [debugSite, mapper] (int, RecyclerWeakReference<Utf8SourceInfo>* entry) -> HRESULT
                {
                    HRESULT hr = S_OK;

                    RemoteWeakReference<Utf8SourceInfo> sourceInfoWeakReference;
                    IfFailedReturn(sourceInfoWeakReference.Read(debugSite, entry));

                    if (sourceInfoWeakReference.Get())
                    {
                        RemoteUtf8SourceInfo sourceInfo;
                        IfFailedReturn(sourceInfo.Read(debugSite, sourceInfoWeakReference.Get()));
                        IfFailedReturn(sourceInfo.MapFunctions(debugSite, mapper));
                    }
                    
                    return hr;
                };

                IfFailGo(Map(debugSite, (ScriptContext::SourceList *)(*this)->sourceList, processUtf8SourceContextInfo));
            }
Error:
            return hr;
        }

    public:
        HRESULT LoadScriptSymbols(IScriptDebugSite* debugSite);
    };

    //
    // Represents remote FunctionBody data.
    //
    class RemoteFunctionBody: public RemoteData<FunctionBody>
    {
    private:
        static const int MAX_FUNCTION_NAME = 256;
        static const int MAX_URL = 256;
        static const int MAX_SYMBOL_NAME = MAX_FUNCTION_NAME + MAX_URL;

        // All the FunctionBody info needed for FunctionBody::GetExternalDisplayName
        class GetFunctionBodyNameData
        {
        private:
            const RemoteFunctionBody& m_funcBody;
            const char16* m_displayName;
            BOOL m_isDynamicScript;
            BOOL m_isGlobalFunc;

        public:
            GetFunctionBodyNameData(const RemoteFunctionBody& funcBody, const char16* displayName, BOOL isDynamicScript, BOOL isGlobalFunc)
                : m_funcBody(funcBody), m_displayName(displayName),m_isDynamicScript(isDynamicScript), m_isGlobalFunc(isGlobalFunc)
            {
            }

            const char16* GetDisplayName() const { return m_displayName; }
            BOOL IsDynamicScript() const { return m_isDynamicScript; }
            uint GetScriptId() const { return m_funcBody->GetScriptId(); }
            uint GetFunctionNumber() const { return m_funcBody->GetFunctionNumber(); }
            BOOL GetIsGlobalFunc() const { return m_isGlobalFunc; }
        };

        const char16* GetExternalDisplayName(const char16* displayName, BOOL isDynamicScript, BOOL isGlobalFunc) const
        {
            GetFunctionBodyNameData funcBody(*this, displayName, isDynamicScript, isGlobalFunc);
            return FunctionBody::GetExternalDisplayName(&funcBody);
        }

        HRESULT GetFunctionBodyInfo(
            IScriptDebugSite* debugSite,
            _Out_writes_z_(nameBufferSize) LPWSTR nameBuffer, ULONG nameBufferSize,
            _Out_writes_z_(urlBufferSize) LPWSTR urlBuffer, ULONG urlBufferSize,
            ULONG* line, ULONG* column) const;

        void* GetAuxPtrs(IScriptDebugSite* debugSite, FunctionProxy::AuxPointerType type) const;
        uint GetCounter(IScriptDebugSite* debugSite, FunctionBody::CounterFields counterType) const;

        static HRESULT AddSymbol(
            IScriptDebugSite* debugSite, LPCVOID addr, ULONG size, LPCWSTR name,
            LPCWSTR url = NULL, ULONG line = 0, ULONG column = 0);

    public:
        HRESULT LoadSymbols(IScriptDebugSite* debugSite, const RemoteScriptContext& scriptContext) const;
    };
}
