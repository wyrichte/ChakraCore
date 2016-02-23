//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

//
// This file represents Manual DAC (debug access component) which consists of proxies to read
// blocks of target process memory and treat them as Runtime objects (access fields, etc).

// Notes:
// - Runtime class can have the following fields:
//   - pointer:                struct Foo { Bar* bar; }; // the field 'bar'.
//   - embedded struct(class): struct Foo { Bar  bar; }; // the field 'bar'.
//   - simple data type (like int)
//
// Guidelines:
// - Never call runtime functions (such as field getters) - don't depend on runtime functionality
//   which can change without updating the DAC. Just use fields.
// - You get aceess to all fields, as we compile runtime headers with everything public.
// - For simple data types or pointers, access the fields directly via overloaded operator->(),
//   or ToTargetPtr() method in case you have pointer to the proxy (rather than ugly (*proxy)->fieldFoo),
//   and you don't have to define a method to get the field in the proxy:
//   RemoteFoo foo(reader, addr); foo->fieldBar;
// - For embedded struct fields, use GetFieldAddr together with offsetof:
//   struct Foo { Bar bar; };
//   RemoteFoo foo(reader, addr);
//   RemoteBar var(reader, foo->GetFieldAddr<Bar>(offsetof(Foo, bar)));
// - For algorithms in runtime (e.g. ScriptContext::IsNativeAddress) encapsulate the logic into corresponding proxy
//   (in this case that would be RemoteScriptContext)
// - Naming: in this file proxy for runtime type 'Foo' is called using 'Remote' prefix, i.e. 'RemoteFoo'.
//

void DebugHeap_OOM_fatal_error();

namespace JsDiag
{
    using namespace Js;

    //
    // Forward declarations.
    //
    struct RemoteScriptContext;

    //
    // Base class for buffer of sizeof(T).
    //
    template <typename T>
    class DataBuffer
    {
    public:
        ULONG GetBufferSize() const
        {
            CompileAssert(sizeof(T) < ULONG_MAX);
            return static_cast<ULONG>(sizeof(T));
        }
    };

    //
    // Static on-stack buffer of sizeof(T).
    //
    template <typename T>
    class StaticDataBuffer: public DataBuffer<T>
    {
    private:
        BYTE m_data[sizeof(T)];

    public:
        LPBYTE GetBuffer()
        {
#if defined(_M_X64) || defined(_M_ARM64)
            CompileAssert(sizeof(T) <= 1024); // Otherwise use DynamicDataBuffer
#else
            CompileAssert(sizeof(T) <= 512); // Otherwise use DynamicDataBuffer
#endif
            return m_data;
        }
    };

    //
    // Dynamic allocated-from-heap buffer of sizeof(T).
    //
    template <typename T>
    class DynamicDataBuffer: public DataBuffer<T>
    {
    private:
        AutoArrayPtr<BYTE> m_data;

    public:
        LPBYTE GetBuffer()
        {
            if (m_data == NULL)
            {
                m_data = new(oomthrow) BYTE[sizeof(T)];
            }
            return m_data;
        }
    };

    //
    // Wrapper for accessing remote debugging target data.
    // Supports delayed read (we don't read target memory until we actually need to access fields that require read operation).
    // Template parameters:
    // - T: the remote type to wrap
    // Usage example:
    // - RemoteData<ScriptContext> remoteScriptContext(reader, remoteAddr);
    //
    template <typename T, template <typename V> class Buffer = StaticDataBuffer>
    class RemoteData
    {
    private:
        Buffer<T> m_data;
        bool m_isInitialized;

    protected:
        IVirtualReader* m_reader;
        const T* m_remoteAddr;

    public:
        typedef T TargetType;

        // Returns strongly typed pointer to the target data buffer.
        T* ToTargetPtr()
        {
            return this->ToTargetPtrImpl();
        }

        const T* ToTargetPtr() const
        {
            return const_cast<T*>(const_cast<RemoteData*>(this)->ToTargetPtrImpl());
        }

        T* operator->()
        {
            return this->ToTargetPtr();
        }

        const T* operator->() const
        {
            return this->ToTargetPtr();
        }

        operator T*()
        {
            return operator->();
        }

        operator const T*() const
        {
            return operator->();
        }

        RemoteData(IVirtualReader* reader, const T* remoteAddr) : m_reader(reader), m_remoteAddr(remoteAddr), m_isInitialized(false) {}

        // Returns address of field at specified offset in remote type, as pointer to the specified type.
        // This is useful for structs/classes embedded into target type (i.e. when the field is struct, and not struct*).
        // Parameters:
        // - TFieldType: actual type of the field as defined in the .h file.
        // - offset: offset off m_remoteAddr for the field.
        template <typename TFieldType>
        TFieldType* GetFieldAddr(size_t offset) const
        {
            return reinterpret_cast<TFieldType*>(reinterpret_cast<BYTE*>(const_cast<T*>(m_remoteAddr)) + offset);
        }
        const void* ReadVTable() const { return ReadField<const void*>(0); }
        IVirtualReader* GetReader() const { return m_reader; }
        T* GetRemoteAddr() const { return const_cast<T*>(m_remoteAddr); }

        void Flush()
        {
            Assert(m_isInitialized);
            HRESULT hr = m_reader->WriteMemory(m_remoteAddr, m_data.GetBuffer(), m_data.GetBufferSize());
            CheckHR(hr, DiagErrorCode::WRITE_VIRTUAL);
        }

        template<typename TFieldType>
        void WriteField(size_t offset, TFieldType data) const
        {
            TFieldType* remoteFieldAddr = GetFieldAddr<TFieldType>(offset);
            HRESULT hr = m_reader->WriteMemory(remoteFieldAddr, &data, sizeof(TFieldType));
            CheckHR(hr, DiagErrorCode::WRITE_VIRTUAL);
        }
    private:
        // Returns a pointer to m_data (which is in local address space) interpreted as T*,
        // with data pointed to containing an instance of T read from target process.
        T* ToTargetPtrImpl()
        {
            if (!m_isInitialized)
            {
                this->Read();   // Read remote memory into m_data;
                m_isInitialized = true;
            }
            return reinterpret_cast<T*>(m_data.GetBuffer());
        }

        void Read()
        {
            ULONG bytesRead;
            HRESULT hr = m_reader->ReadVirtual(m_remoteAddr, m_data.GetBuffer(), m_data.GetBufferSize(), &bytesRead);
            CheckHR(hr, DiagErrorCode::READ_VIRTUAL);
            if (bytesRead != m_data.GetBufferSize())
            {
                DiagException::Throw(E_UNEXPECTED, DiagErrorCode::READ_VIRTUAL_MISMATCH);
            }
        }

    protected:
        template <typename T> T ReadVirtual(const void* addr) const
        {
            return VirtualReader::ReadVirtual<T>(m_reader, addr);
        }

        template <typename T> T ReadField(size_t offset) const
        {
            return ReadVirtual<T>(GetFieldAddr<T>(offset));
        }
    }; // RemoteData.

    template <typename T, typename ListType = JsUtil::List<T>>
    struct RemoteList : public RemoteData<ListType>
    {
        RemoteList(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}

        int Count()
        {
            return this->ToTargetPtr()->count;
        }

        T Item(int index)
        {
#if DBG
            int count = this->Count();
            if (index < 0 || index >= count)
            {
                // Note that List.h for this case just returns dummy instance of T (created by default ctor).
                DiagException::Throw(E_INVALIDARG, DiagErrorCode::LIST_INDEX_OUTOFBOUND);
            }
#endif
            T* bufferAddr = this->ToTargetPtr()->buffer;
            RemoteData<T> val(m_reader, bufferAddr + index);
            return *val.ToTargetPtr();
        }

        template <typename TMapFunction>
        void Map(TMapFunction mapFunction)
        {
            MapUntil( [=] (uint i, T item) -> bool
            {
                mapFunction(i, item);
                return false;
            });
        }

        template <typename TMapFunction>
        bool MapUntil(TMapFunction mapFunction)
        {
            int count = this->Count();
            for (int i = 0; i < count; ++i)
            {
                T item = this->Item(i);
                if(ListType::TRemovePolicyType::IsItemValid(item))
                {
                    if(mapFunction(i, item))
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        template <typename TMapFunction>
        void MapReverse(TMapFunction mapFunction)
        {
            MapUntilReverse( [=] (uint i, T item) -> bool
            {
                mapFunction(i, item);
                return false;
            });
        }

        template <typename TMapFunction>
        bool MapUntilReverse(TMapFunction mapFunction)
        {
            int count = this->Count();

            if (count == 0) return false;

            for (int i = count - 1; i >= 0; --i)
            {
                T item = this->Item(i);
                if(ListType::TRemovePolicyType::IsItemValid(item))
                {
                    if(mapFunction(i, item))
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        // Performs a binary search on a range of elements in the list (assumes the list is sorted).
        template <typename TComparisonFunction>
        int BinarySearch(TComparisonFunction compare, int fromIndex, int toIndex)
        {
            AssertMsg(fromIndex >= 0, "Invalid starting index for binary searching.");
            AssertMsg(toIndex < this->Count(), "Invalid ending index for binary searching.");

            while (fromIndex <= toIndex)
            {
                int midIndex = fromIndex + (toIndex - fromIndex) / 2;
                T item = this->Item(midIndex);
                int compareResult = compare(item, midIndex);
                if (compareResult > 0)
                {
                    toIndex = midIndex - 1;
                }
                else if (compareResult < 0)
                {
                    fromIndex = midIndex + 1;
                }
                else
                {
                    return midIndex;
                }
            }
            return -1;
        }

        // Performs a binary search on the elements in the list (assumes the list is sorted).
        template <typename TComparisonFunction>
        int BinarySearch(TComparisonFunction compare)
        {
            return BinarySearch<TComparisonFunction>(compare, 0, this->Count() - 1);
        }

    };

    struct RemoteInterval : public RemoteData<regex::Interval>
    {
        RemoteInterval(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
        bool Includes (int value);
    };

    template <typename T>
    struct RemoteDListNodeBase : public RemoteData<DListNodeBase<T>>
    {
        RemoteDListNodeBase(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}

        DListNodeBase<T>* Next()
        {
            return this->ToTargetPtr()->next.base;
        }

        T* GetData()
        {
            return this->GetFieldAddr<T>(offsetof(DListNode<T>, data));
        }
    };

    template <typename T>
    class RemoteDListIterator
    {
        IVirtualReader* m_reader;
        DListNodeBase<T>* m_head;
        DListNodeBase<T>* m_current;
    public:
        RemoteDListIterator<T>(IVirtualReader* reader, DListBase<T>* head) : m_reader(reader), m_head((DListNodeBase<T>*)head), m_current((DListNodeBase<T>*)head) {}

        bool Next()
        {
            RemoteDListNodeBase<T> currentNode(m_reader, m_current);
            DListNodeBase<T>* nextNode = currentNode.Next();
            if (nextNode == m_head)
            {
                m_current = NULL;
                return false;
            }

            m_current = nextNode;
            return true;
        }

        T* Current()
        {
            Assert(m_current && m_current != m_head);    // IsValid.
            RemoteDListNodeBase<T> currentNode(m_reader, m_current);
            return currentNode.GetData();
        }
    };

    template <typename T>
    struct RemoteArray : public RemoteData<T>
    {
        RemoteArray(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}

        T Item(const uint index) const
        {
            return ReadVirtual<T>(m_remoteAddr + index);
        }

        T operator[] (const int index) const
        {
            return Item(index);
        }
    };

    template <typename T>
    struct RemoteWeaklyReferencedKeyDictionary : public RemoteData<T>
    {
        typedef typename T::EntryType EntryType;
        typedef typename T::KeyType TKey;
        typedef typename T::ValueType TValue;

    private:
        RemoteArray<EntryType> m_entries;
        RemoteArray<int> m_buckets;

    public:
        RemoteWeaklyReferencedKeyDictionary(IVirtualReader* reader, const TargetType* addr):
            RemoteData<TargetType>(reader, addr),
            m_entries(reader, ToTargetPtr()->entries),
            m_buckets(reader, ToTargetPtr()->buckets)
        {}

        template<class Fn>
        void Map(Fn fn)
        {
            for(int i = 0; i < ToTargetPtr()->size; i++)
            {
                if(m_buckets[i] != -1)
                {
                    for(int currentIndex = m_buckets[i]; currentIndex != -1;)
                    {
                        EntryType currentEntry = m_entries[currentIndex];
                        RemoteRecyclerWeakReference<TKey> weakKey(m_reader, currentEntry.key);
                        TKey * key = weakKey.Get();
                        if(key != nullptr)
                        {
                            fn(key, currentEntry.value);
                        }

                        currentIndex = currentEntry.next;
                    }
                }
            }
        }
    };

    template <typename T>
    struct RemoteDictionary : public RemoteData<T>
    {
        typedef typename T::EntryType EntryType;
        typedef typename T::ValueType TValue;

    private:
        RemoteArray<EntryType> m_entries;
        RemoteArray<int> m_buckets;
    public:
        RemoteDictionary(IVirtualReader* reader, const TargetType* addr):
            RemoteData<TargetType>(reader, addr),
            m_entries(reader, ToTargetPtr()->entries),
            m_buckets(reader, ToTargetPtr()->buckets)
        {}

        EntryType Item(uint index)
        {
            return m_entries.Item(index);
        }

        template <typename TLookup>
        bool TryGetValue(const TLookup& key, TValue* value)
        {
            int i = FindEntryWithKey(key);
            if (i >= 0)
            {
                *value = m_entries[i].Value();
                return true;
            }
            return false;
        }

        template<typename Fn>
        void MapUntil(Fn fn)
        {
            MapEntryUntil([fn](EntryType const& entry) -> bool
            {
                return fn(entry.Value());
            });
        }

        int GetLastIndex()
        {

            return ToTargetPtr()->GetLastIndex();
        }

    private:
        template <typename LookupType>
        int FindEntryWithKey(const LookupType& key) const
        {
            typedef T::ComparerType<LookupType>::Type LookupComparer;

            if (m_buckets.GetRemoteAddr() != NULL)
            {
                uint hashCode = ToTargetPtr()->GetHashCodeWithKey<LookupType>(key);
                uint targetBucket = ToTargetPtr()->GetBucket(hashCode, ToTargetPtr()->bucketCount);
                for (int i = m_buckets[targetBucket]; i >= 0; i = m_entries[i].next)
                {
                    if (m_entries[i].KeyEquals<LookupComparer>(key, hashCode))
                    {
                        return i;
                    }
                }
            }

            return -1;
        }

        template<typename Fn>
        void MapEntryUntil(Fn fn) const
        {
            for (uint i=0; i < ToTargetPtr()->bucketCount; i++)
            {
                if(m_buckets[i] != -1)
                {
                    for (int currentIndex = m_buckets[i] ; currentIndex != -1 ; currentIndex = m_entries[currentIndex].next)
                    {
                        if (fn(m_entries[currentIndex]))
                        {
                            return;
                        }
                    }
                }
            }
        }
    };

    typedef RemoteData<BVSparseNode> RemoteBVSparseNode;

    template <class TAllocator>
    class RemoteBVSparse: RemoteData<BVSparse<TAllocator>>
    {
    private:
        BVIndex m_prevFloor;
        RemoteBVSparseNode m_cur;

    public:
        RemoteBVSparse(IVirtualReader* reader, const TargetType* addr):
            RemoteData<TargetType>(reader, addr),
            m_prevFloor(BVInvalidIndex),
            m_cur(reader, ToTargetPtr()->head)
        {
        }

        bool Test(BVIndex i)
        {
            const BVIndex searchIndex = BVUnit32::Floor(i);

            // If index possibly exists prior to m_cur, reset to beginning
            if (searchIndex <= m_prevFloor && m_prevFloor != BVInvalidIndex)
            {
                m_prevFloor = BVInvalidIndex;
                m_cur = RemoteBVSparseNode(m_reader, ToTargetPtr()->head);
            }

            while (m_cur.GetRemoteAddr())
            {
                if (searchIndex == m_cur->startIndex)
                {
                    return m_cur->data.Test(BVUnit32::Offset(i)) != 0;
                }
                if (searchIndex < m_cur->startIndex)
                {
                    break;
                }

                m_prevFloor = m_cur->startIndex;
                m_cur = RemoteBVSparseNode(m_reader, m_cur->next);
            }

            return false;
        }
    };

    struct RemoteThreadContextTLSEntry : public RemoteData<ThreadContextTLSEntry>
    {
        RemoteThreadContextTLSEntry(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}

        // Returns address of ThreadContext in remote address space.
        ThreadContext* GetThreadContext();
    };

    template <typename TTargetType>
    struct RemoteEntryPointInfo : public RemoteData<TTargetType>
    {
        RemoteEntryPointInfo(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
        bool IsCodeGenDone();
        bool HasNativeAddress();
        DWORD_PTR GetNativeAddress();
        SmallSpanSequence* GetNativeThrowSpanSequence();
        ptrdiff_t GetCodeSize();
        EntryPointInfo::State GetState();
    };

    typedef RemoteList<DebuggerScopeProperty> RemoteDebuggerScopePropertyList;
    typedef RemoteList<RecyclerWeakReference<Utf8SourceInfo>*, ScriptContext::SourceList> RemoteSourceList;
    typedef RemoteData<ScriptEntryExitRecord> RemoteScriptEntryExitRecord;
    typedef RemoteData<Type> RemoteType;
    typedef RemoteData<DynamicType> RemoteDynamicType;
    typedef RemoteData<AsyncBreakController> RemoteAsyncBreakController;
    typedef RemoteList<ReturnedValue*, JsUtil::List<ReturnedValue*>> RemoteReturnedValueList;
    typedef RemoteData<ReturnedValue> RemoteReturnedValue;
    typedef RemoteData<InterpreterHaltState> RemoteInterpreterHaltState;
    typedef RemoteData<StepController> RemoteStepController;

    struct RemoteJavascriptLibrary: public RemoteData<JavascriptLibrary>
    {
        RemoteJavascriptLibrary(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
        RemoteJavascriptLibrary(IVirtualReader* reader, const ScriptContext* scriptContext);

        ScriptContext* GetScriptContext() const { return this->ReadField<ScriptContext*>(offsetof(TargetType, scriptContext)); }
        bool IsUndeclBlockVar(Js::Var var) const { return var == GetUndeclBlockVar(); }
        RecyclableObject* GetUndeclBlockVar() const { return this->ReadField<RecyclableObject*>(offsetof(TargetType, undeclBlockVarSentinel)); }
        RecyclableObject* GetUndefined() const { return this->ReadField<RecyclableObject*>(offsetof(TargetType, undefinedValue)); }
        RecyclableObject* GetNull() const { return this->ReadField<RecyclableObject*>(offsetof(TargetType, nullValue)); }
        RecyclableObject* GetTrue() const { return this->ReadField<RecyclableObject*>(offsetof(TargetType, booleanTrue)); }
        RecyclableObject* GetFalse() const { return this->ReadField<RecyclableObject*>(offsetof(TargetType, booleanFalse)); }
        ObjectPrototypeObject* GetObjectPrototype() const { return this->ReadField<ObjectPrototypeObject*>(offsetof(TargetType, objectPrototype)); }
        DynamicObject* GetFunctionPrototype() const { return this->ReadField<DynamicObject*>(offsetof(TargetType, functionPrototype)); }
        DynamicObject* GetBooleanPrototype() const { return this->ReadField<DynamicObject*>(offsetof(TargetType, booleanPrototype)); }
        DynamicObject* GetSymbolPrototype() const { return this->ReadField<DynamicObject*>(offsetof(TargetType, symbolPrototype)); }
        DynamicObject* GetNumberPrototype() const { return this->ReadField<DynamicObject*>(offsetof(TargetType, numberPrototype)); }
        DynamicObject* GetStringPrototype() const { return this->ReadField<DynamicObject*>(offsetof(TargetType, stringPrototype)); }
        DynamicObject* GetMapConstructor() { return this->ReadField<DynamicObject*>(offsetof(TargetType, mapConstructor)); }
        DynamicObject* GetSetConstructor() { return this->ReadField<DynamicObject*>(offsetof(TargetType, setConstructor)); }
        DynamicObject* GetWeakMapConstructor() { return this->ReadField<DynamicObject*>(offsetof(TargetType, weakMapConstructor)); }
        RecyclableObject* GetDebuggerDeadZoneBlockVariableString() const { Assert(this->ReadField<RecyclableObject*>(offsetof(TargetType, debuggerDeadZoneBlockVariableString))); return this->ReadField<RecyclableObject*>(offsetof(TargetType, debuggerDeadZoneBlockVariableString)); }
    };

    typedef RemoteData<Js::CallInfo> RemoteCallInfo;
    typedef RemoteData<NativeCodeGenerator> RemoteNativeCodeGenerator;
    typedef RemoteData<FunctionBody::StatementMap> RemoteFunctionBody_StatementMap;

    typedef RemoteData<ByteBlock> RemoteByteBlock;
    typedef RemoteEntryPointInfo<FunctionEntryPointInfo> RemoteFunctionEntryPointInfo;
    typedef RemoteEntryPointInfo<LoopEntryPointInfo> RemoteLoopEntryPointInfo;
    typedef RemoteDictionary<Utf8SourceInfo::FunctionBodyDictionary> RemoteFunctionBodyDicionary;

    struct RemotePropertyRecord: public RemoteData<PropertyRecord>
    {
        RemotePropertyRecord(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
        Js::PropertyId GetPropertyId() const { return ReadField<Js::PropertyId>(offsetof(TargetType, pid)); }
    };

    struct RemoteThreadContext : public RemoteData<ThreadContext, DynamicDataBuffer>
    {
    private:
        AutoPtr<RemoteDictionary<ThreadContext::PropertyMap>> m_propertyMap;

    public:
        RemoteThreadContext(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType, DynamicDataBuffer>(reader, addr) {}
        bool DoInterruptProbe();
        bool GetIsThreadBound();
        BOOL TestThreadContextFlag(ThreadContextFlags contextFlag);
        DebugManager* GetDebugManager() const { return this->ReadField<DebugManager*>(offsetof(TargetType, debugManager)); }
        ScriptContext* GetScriptContextList() const { return this->ReadField<ScriptContext*>(offsetof(TargetType, scriptContextList)); }
        bool IsAllJITCodeInPreReservedRegion() const{ return this->ReadField<bool>(offsetof(TargetType, isAllJITCodeInPreReservedRegion)); }
        PreReservedVirtualAllocWrapper * GetPreReservedVirtualAllocator() { return (this->GetFieldAddr<PreReservedVirtualAllocWrapper>(offsetof(TargetType, preReservedVirtualAllocator))); }
        CustomHeap::CodePageAllocators * GetCodePageAllocators() { return this->GetFieldAddr<CustomHeap::CodePageAllocators>(offsetof(TargetType, codePageAllocators));}
        DWORD GetCurrentThreadId() const { return this->ReadField<DWORD>(offsetof(TargetType, currentThreadId)); }
        const PropertyRecord* GetPropertyName(Js::PropertyId propertyId);
        Js::JavascriptExceptionObject* GetUnhandledExceptionObject() const;
    };

    struct RemoteDebugManager : public RemoteData <DebugManager>
    {
        RemoteDebugManager(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
        DebuggingFlags* GetDebuggingFlags() const { return this->GetFieldAddr<DebuggingFlags>(offsetof(TargetType, debuggingFlags)); }
        AsyncBreakController* GetAsyncBreakController() const { return this->GetFieldAddr<AsyncBreakController>(offsetof(TargetType, asyncBreakController)); }
        StepController* GetStepController() const { return this->GetFieldAddr<StepController>(offsetof(TargetType, stepController)); }
        InterpreterHaltState* GetInterpreterHaltState() const { return this->ReadField<InterpreterHaltState*>(offsetof(TargetType, pCurrentInterpreterLocation)); }
        bool GetIsAtDispatchHalt() const { return this->ReadField<bool>(offsetof(TargetType, isAtDispatchHalt)); }
    };

    struct RemoteFunctionInfo : public RemoteData<FunctionInfo>
    {
        RemoteFunctionInfo(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
        FunctionBody* GetFunction();
    };

    struct RemoteParseableFunctionInfo : public RemoteData<ParseableFunctionInfo>
    {
        RemoteParseableFunctionInfo(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
    };

    struct RemoteLoopHeader : public RemoteData<Js::LoopHeader>
    {
        RemoteLoopHeader(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}

        template<typename TMapFunction> void MapEntryPoints(TMapFunction mapFunction)
        {
            // typedef SynchronizableList<LoopEntryPointInfo*, JsUtil::List<LoopEntryPointInfo*>> EntryPointList;
            if (this->ToTargetPtr()->entryPoints)
            {
                RemoteList<LoopEntryPointInfo*> entryPoints(m_reader, this->ToTargetPtr()->entryPoints);
                entryPoints.Map([&](int index, LoopEntryPointInfo* entryPoint){
                    if (entryPoint != NULL)
                    {
                        mapFunction(index, entryPoint);
                    }
                });
            }
        }
    };

    struct RemoteGrowingUint32HeapArray : public RemoteData<JsUtil::GrowingUint32HeapArray>
    {
        RemoteGrowingUint32HeapArray(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
        uint32 ItemInBuffer(uint32 index);
    };

    struct RemoteSmallSpanSequence : public RemoteData<SmallSpanSequence>
    {
        RemoteSmallSpanSequence(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
        BOOL GetMatchingStatementFromBytecode(int bytecode, SmallSpanSequenceIter& iter, StatementData& data);
        template <typename TFilterFn>
        bool GetMatchingStatement(SmallSpanSequenceIter& iter, TFilterFn filterFn, StatementData& data);
        uint32 Count();
        void ResetIterator(SmallSpanSequenceIter &iter);
        BOOL GetRangeAt(int index, SmallSpanSequenceIter& iter, int* pCountOfMissed, StatementData& data);
        BOOL Item(int index, SmallSpanSequenceIter& iter, StatementData& data);
    };

    template <class T>
    struct RemoteRecyclableObjectBase : public RemoteData<T>
    {
        RemoteRecyclableObjectBase(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
        ScriptContext* GetScriptContext();
        RecyclableObject* GetPrototype();
        JavascriptLibrary* GetLibrary();
        JavascriptMethod GetEntrypoint();
    };

    typedef RemoteRecyclableObjectBase<RecyclableObject> RemoteRecyclableObject;

    struct RemoteScriptConfiguration : public RemoteData<ScriptConfiguration>
    {
        RemoteScriptConfiguration(IVirtualReader* reader, const TargetType* addr) :
            RemoteData<TargetType>(reader, addr),
            m_threadConfig(m_reader, ToTargetPtr()->threadConfig)
        {
        }

        bool IsES6TypedArrayExtensionsEnabled() { return m_threadConfig->IsES6TypedArrayExtensionsEnabled(); }
        bool IsES6UnicodeExtensionsEnabled() { return m_threadConfig->IsES6UnicodeExtensionsEnabled(); }
        bool IsES6RegExStickyEnabled() { return m_threadConfig->IsES6RegExStickyEnabled(); }

    private:
        ThreadConfiguration *GetThreadConfig()
        {
            const ThreadConfiguration * const threadConfigAddr = ToTargetPtr()->threadConfig;
            RemoteData<ThreadConfiguration> remoteThreadConfig(m_reader, threadConfigAddr);
            return remoteThreadConfig.ToTargetPtr();
        }

        RemoteData<ThreadConfiguration> m_threadConfig;
    };

    struct RemoteJavascriptFunction : public RemoteRecyclableObjectBase<JavascriptFunction>
    {
        RemoteJavascriptFunction(IVirtualReader* reader, const TargetType* addr) : RemoteRecyclableObjectBase<TargetType>(reader, addr) {}
        JavascriptLibrary* GetLibrary();
        FunctionBody* GetFunction() const;
        bool_result TryReadDisplayName(_Out_ CString* name);
        static bool_result TryReadDisplayName(IVirtualReader* reader, Js::Var var, _Out_ CString* name);
        Js::Var GetSourceString(const InspectionContext* context) const;
        CString GetDisplayNameString(InspectionContext* context);

        bool GetCaller(const InspectionContext* context, Js::Var* value, CString& error);
        bool GetArguments(const InspectionContext* context, Js::Var* value, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty, CString& error);
        uint16 GetLength();

        RecyclableObject* FindCaller(
            const InspectionContext* inspectionContext,
            RemoteThreadContext* remoteThreadContext,
            RemoteScriptContext* remoteRequestContext,
            JavascriptFunction* nullObject,
            bool& foundThis);

        bool IsLibraryCode() const;
        bool IsScriptFunction() const;
        bool IsBoundFunction(const InspectionContext* inspectionContext) const;
        bool IsStrictMode() const;
        bool HasRestrictedProperties() const;
    };

    struct RemoteBoundFunction : public RemoteRecyclableObjectBase<BoundFunction>
    {
        RemoteBoundFunction(IVirtualReader* reader, const TargetType* addr) : RemoteRecyclableObjectBase<TargetType>(reader, addr) {}
        uint16 GetLength(InspectionContext* inspectionContext, PROPERTY_INFO* propInfo);
        static const uint16 TARGETS_RUNTIME_FUNCTION = 0xffff;
    };

    struct RemoteRuntimeFunction : public RemoteData<RuntimeFunction>
    {
        RemoteRuntimeFunction(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
    };

    struct RemoteJavascriptRegExpConstructor : public RemoteData<JavascriptRegExpConstructor>
    {
        RemoteJavascriptRegExpConstructor(IVirtualReader* reader, const TargetType* addr)
            : RemoteData<TargetType>(reader, addr)
        {}
    };

    struct RemoteScriptFunction : public RemoteData<ScriptFunction>
    {
        RemoteScriptFunction(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
        FunctionBody* GetFunction();
        uint32 GetFrameHeight(FunctionEntryPointInfo* entryPoint);
        FrameDisplay* GetEnvironment();
    };

    class InternalStackFrame;
    struct RemoteInlinedFrameLayout : public RemoteData<InlinedFrameLayout>
    {
        RemoteInlinedFrameLayout(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
        InlinedFrameLayout* Next();
        static InlinedFrameLayout* FromPhysicalFrame(
            IVirtualReader* reader, InternalStackFrame* physicalFrame, void* entry, Js::ScriptFunction* parent, FunctionEntryPointInfo* entryPoint);
    };

    struct RemoteInterpreterStackFrame : public RemoteData<InterpreterStackFrame>
    {
        RemoteInterpreterStackFrame(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
        bool IsCurrentLoopNativeAddr(void* addr);
        ByteCodeReader* GetReader();
        Js::Var GetReg(RegSlot reg);
        FrameDisplay* GetFrameDisplay(RegSlot frameDisplayRegister);
        Js::Var GetRootObject();
        Js::Var GetInnerScope(RegSlot scopeLocation);
    };

    struct RemoteByteCodeReader : public RemoteData<ByteCodeReader>
    {
        RemoteByteCodeReader(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
        int GetCurrentOffset();
    };

    struct RemoteDynamicObject: public RemoteRecyclableObjectBase<DynamicObject>
    {
        RemoteDynamicObject(IVirtualReader* reader, const TargetType* addr) : RemoteRecyclableObjectBase(reader, addr) {}
        DynamicTypeHandler* GetTypeHandler() { return RemoteDynamicType(m_reader, ToTargetPtr()->GetDynamicType())->GetTypeHandler(); }
        bool HasObjectArray(InspectionContext* context);
    };

    struct RemoteEmitBufferManager : public RemoteData<EmitBufferManager<CriticalSection>>
    {
        RemoteEmitBufferManager(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
        CustomHeap::Heap* GetAllocationHeap();
    };

    struct RemoteHeap : public RemoteData<CustomHeap::Heap>
    {
        RemoteHeap(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
        HeapPageAllocator<VirtualAllocWrapper>* GetHeapPageAllocator();
        HeapPageAllocator<PreReservedVirtualAllocWrapper>* GetPreReservedHeapPageAllocator();
    };

    struct RemoteSegment : public RemoteData<Segment>
    {
        RemoteSegment(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
        bool IsInSegment(void* addr);

    private:
        char* GetEndAddress();
        size_t GetAvailablePageCount();
    };

    struct RemotePageSegment : public RemoteData<PageSegment, DynamicDataBuffer>
    {
        RemotePageSegment(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType, DynamicDataBuffer>(reader, addr) {}

        bool IsInSegment(void* address);
        bool IsFreeOrDecommitted(void* addr);

    private:
        uint GetBitRangeBase(void* addr);
    };

    struct RemotePreReservedVirtualAllocWrapper : public RemoteData<PreReservedVirtualAllocWrapper>
    {
        RemotePreReservedVirtualAllocWrapper(IVirtualReader * reader, const TargetType * addr) : RemoteData<TargetType>(reader, addr) {}

        bool IsInRange(void * address);
        bool IsPreReservedRegionPresent();
        LPVOID GetPreReservedStartAddress();
        LPVOID GetPreReservedEndAddress();
    };

    struct RemotePreReservedHeapPageAllocator : public RemoteData<HeapPageAllocator<PreReservedVirtualAllocWrapper>>
    {
        RemotePreReservedHeapPageAllocator(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}

        PreReservedVirtualAllocWrapper * GetVirtualAllocator() { return this->ReadField<PreReservedVirtualAllocWrapper*>(offsetof(TargetType, virtualAllocator)); }
        bool IsInRange(void * address)
        {
            RemotePreReservedVirtualAllocWrapper preReservedVirtualAllocWrapper(m_reader, GetVirtualAllocator());
            return preReservedVirtualAllocWrapper.IsInRange(address);
        }
    };

    struct RemoteHeapPageAllocator : public RemoteData<HeapPageAllocator<>>
    {
        RemoteHeapPageAllocator(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}

        bool IsAddressFromAllocator(void* address);
        bool IsAddressInSegment(void* address, const PageSegment* segmentAddr);
        bool IsAddressInSegment(void* address, const Segment* segmentAddr);
        static void GetSegmentOffsets(size_t* segments, size_t* fullSegments, size_t* decommitSegments, size_t* largeSegments);
    };

    struct RemoteCodePageAllocators : public RemoteData<CustomHeap::CodePageAllocators>
    {
        RemoteCodePageAllocators(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}

        HeapPageAllocator<VirtualAllocWrapper> * GetHeapPageAllocator();
    };
    struct RemoteCodeGenAllocators : public RemoteData<CodeGenAllocators>
    {
        RemoteCodeGenAllocators(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}

        EmitBufferManager<CriticalSection>* GetEmitBufferManager();
    };

    struct RemoteDebugContext : public RemoteData<DebugContext>
    {
        RemoteDebugContext(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}

        ProbeContainer* GetProbeContainer() const { return this->ReadField<ProbeContainer*>(offsetof(DebugContext, diagProbesContainer)); }

        bool IsInDebugMode() const
        {
            DebuggerMode mode = this->ReadField<DebuggerMode>(offsetof(DebugContext, debuggerMode));
            return mode == DebuggerMode::Debugging;
        }
    };

    struct RemoteScriptContext : public RemoteData<ScriptContext, DynamicDataBuffer>
    {
        RemoteScriptContext(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType, DynamicDataBuffer>(reader, addr) {}
        RemoteScriptContext(IVirtualReader* reader, const RecyclableObject* instance):
            RemoteData<TargetType, DynamicDataBuffer>(reader, RemoteRecyclableObject(reader, instance).GetScriptContext()) {}

        bool IsNativeAddress(void* address);    // Check current script context and all contexts from its thread context.s
        JavascriptLibrary* GetLibrary() const;
        ThreadContext* GetThreadContext() const;
        const PropertyRecord* GetPropertyName(Js::PropertyId propertyId) const;
        ProbeContainer* GetProbeContainer() const;
        ScriptConfiguration* GetConfig() const { return this->GetFieldAddr<ScriptConfiguration>(offsetof(ScriptContext, config)); }
        DaylightTimeHelper* GetDaylightTimeHelper() const { return this->GetFieldAddr<DaylightTimeHelper>(offsetof(ScriptContext, daylightTimeHelper)); }
        DebugContext* GetDebugContext() const { return this->ReadField<DebugContext*>(offsetof(ScriptContext, debugContext)); }
        bool IsInDebugMode() const
        {
            if (this->GetDebugContext() != nullptr)
            {
                RemoteDebugContext remoteDebugContext(m_reader, this->GetDebugContext());
                return remoteDebugContext.IsInDebugMode();
            }
            return false;
        }
    };

    struct RemoteProbeContainer: public RemoteData<ProbeContainer>
    {
        RemoteProbeContainer(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
        Js::Var GetExceptionObject() const { return ReadField<Js::Var>(offsetof(ProbeContainer, jsExceptionObject)); }
    };

    template <typename T>
    struct RemoteRecyclerWeakReference : public RemoteData<RecyclerWeakReference<T>>
    {
        RemoteRecyclerWeakReference(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}

        T* Get()
        {
            // Note: Runtime accounts for scenario when Recycler is exiting which does not seems to be of our interest.
            return reinterpret_cast<T*>(this->ToTargetPtr()->strongRef);
        }
    };

    struct RemoteSourceContextInfo : public RemoteData<SourceContextInfo>
    {
        RemoteSourceContextInfo(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
        bool IsDynamic() const;
    };

    struct RemoteSRCINFO : public RemoteData<SRCINFO>
    {
        RemoteSRCINFO(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
        ULONG ConvertInternalOffsetToHost(ULONG charOffsetInScriptBuffer) const;
    };

    typedef JsUtil::LineOffsetCache<Recycler> LineOffsetCache;
    typedef LineOffsetCache::LineOffsetCacheItem LineOffsetCacheItem;
    typedef JsUtil::ReadOnlyList<LineOffsetCacheItem> LineOffsetCacheReadOnlyList;

    struct RemoteLineOffsetCache : public RemoteData<LineOffsetCache>
    {
        RemoteLineOffsetCache(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}

        int GetLineForCharacterOffset(charcount_t characterOffset, charcount_t *outLineCharOffset, charcount_t *outByteOffset);

    private:
        const LineOffsetCacheReadOnlyList* GetLineOffsetCacheList();
    };

    struct RemoteFunctionBody;

    struct RemoteFunctionBody_SourceInfo : public RemoteData<FunctionBody::SourceInfo>
    {
        RemoteFunctionBody_SourceInfo(IVirtualReader* reader, const RemoteFunctionBody* functionBody);

        ByteBlock* GetProbeBackingBlock();
        void SetProbeBackingBlock(ByteBlock* block);
        void IncrementProbeCount();
        void DecrementProbeCount();
        bool GetLineCharOffset(int byteCodeOffset, ULONG* _line, LONG* _colOffset);
        void GetStatementOffsets(int byteCodeOffset, ULONG* startOffset, ULONG* endOffset);
        bool HasLineBreak(charcount_t start, charcount_t end);
        LineOffsetCache* GetLineOffsetCache();
        FunctionBody::StatementMap* GetEnclosingStatementMapFromByteCode(int byteCodeOffset);
        int GetEnclosingStatementIndexFromByteCode(int byteCodeOffset);
        FunctionBody::StatementMapList* GetStatementMaps();
        SourceContextInfo* GetSourceContextInfo();
        const SRCINFO* GetHostSrcInfo();
        LPCWSTR GetSourceName(const RemoteSourceContextInfo* remoteSourceContextInfo);
        ULONG GetHostStartLine();
        LPCUTF8 GetStartOfDocument();
        size_t StartOffset();
        size_t LengthInBytes();
        bool EndsAfter(size_t offset);

    private:
        const FunctionBody* GetFunctionBody();
        bool GetLineCharOffsetFromStartChar(charcount_t startCharOfStatement, ULONG* _line, LONG* _colOffset);
        ULONG GetInternalOffsetForStatementStart(int byteCodeOffset);
        bool GetInternalOffsetForStatementEnd(ULONG statementStartSourceOffset, ULONG* statementEndCharOffset);

        const RemoteFunctionBody* m_functionBody;
    };

    struct RemoteFunctionBody : public RemoteData<FunctionBody>
    {
        RemoteFunctionBody(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}

    private:
        // All the FunctionBody info needed for FunctionBody::GetExternalDisplayName
        class GetFunctionBodyNameData
        {
        private:
            const RemoteFunctionBody& m_funcBody;
            const wchar_t* m_displayName;
            BOOL m_isDynamicScript;
            BOOL m_isGlobalFunc;

        public:
            GetFunctionBodyNameData(const RemoteFunctionBody& funcBody, const wchar_t* displayName, BOOL isDynamicScript, BOOL isGlobalFunc)
                : m_funcBody(funcBody), m_displayName(displayName),  m_isDynamicScript(isDynamicScript), m_isGlobalFunc(isGlobalFunc)
            {
            }

            const wchar_t* GetDisplayName() const { return m_displayName; }
            BOOL IsDynamicScript() const { return m_isDynamicScript; }
            uint GetScriptId() const { return m_funcBody->m_uScriptId; }
            uint GetFunctionNumber() const { return m_funcBody->m_functionNumber; }
            BOOL GetIsGlobalFunc() const { return m_isGlobalFunc; }
        }; // GetFunctionBodyNameData.

        struct RowColumn
        {
            ULONG row;
            LONG column;
        };
        CAtlMap<int, RowColumn> m_rowColumnMap; // bytecode offset -> row/column

    public:
        bool_result TryReadDisplayName(_Out_ CString* name) const;
        void GetFunctionName(_Out_writes_z_(nameBufferElementCount) LPWSTR nameBuffer, ULONG nameBufferElementCount) const;
        void GetUri(_Out_writes_z_(nameBufferElementCount) LPWSTR nameBuffer, ULONG nameBufferElementCount) const;

        Utf8SourceInfo* GetUtf8SourceInfo() const;
        UINT64 GetDocumentId() const;
        FunctionBody::SourceInfo* GetSourceInfo() const;
        FunctionEntryPointInfo* GetEntryPointFromNativeAddress(DWORD_PTR codeAddress);
        LoopEntryPointInfo* GetLoopEntryPointFromNativeAddress(DWORD_PTR codeAddress, uint loopNum);
        LoopHeader* GetLoopHeader(uint index);
        uint32 GetFrameHeight(FunctionEntryPointInfo* entryPoint);
        FunctionEntryPointInfo* GetEntryPointInfo(int index);
        template <typename Fn> void MapEntryPoints(Fn fn);
        BOOL GetMatchingStatementMapFromNativeOffset(StatementData* statementMap, DWORD_PTR codeAddress, uint32 offset, uint loopNum, FunctionBody* inlinee = NULL);
        void FindClosestStatements(long characterOffset, StatementLocation *firstStatementLocation, StatementLocation *secondStatementLocation);
        bool InstallProbe(int offset, RemoteAllocator* allocator);
        void UninstallProbe(int offset);
        static bool Is(InspectionContext* context, void* ptr);
        void GetRowColumn(int byteCodeOffset, ULONG* pRow, ULONG* pColumn);
        RegSlot GetFrameDisplayRegister();
        Js::RootObjectBase* GetRootObject() const;
        bool GetNonTempSlotOffset(RegSlot slotId, __out int32 * slotOffset) const;

    private:
        static const wchar_t* GetExternalDisplayName(const GetFunctionBodyNameData* funcBody);
        const wchar_t* GetExternalDisplayName(const wchar_t* displayName, BOOL isDynamicScript, BOOL isGlobalFunc) const;
        template <typename Fn> void GetFunctionBodyInfo(Fn fn) const;
        BOOL GetMatchingStatementMapFromNativeAddress(StatementData* statementMap, DWORD_PTR codeAddress, uint loopNum, FunctionBody* inlinee = NULL);
        BOOL GetMatchingStatementMapFromNativeOffset(StatementData* statementMap, DWORD_PTR codeAddress, uint32 offset, FunctionBody* inlinee = NULL);
        int GetStatementIndexFromNativeAddress(SmallSpanSequence* throwSpanSequence, DWORD_PTR codeAddress, DWORD_PTR nativeBaseAddress);
        int GetStatementIndexFromNativeOffset(SmallSpanSequence* throwSpanSequence, uint32 nativeOffset);
        BOOL GetMatchingStatementMap(StatementData* statementMap, int statementIndex, FunctionBody* inlinee);
        bool IsNonTempLocalVar(uint32 varIndex) const;
        uint32 GetFirstNonTempLocalIndex() const;
        uint32 GetEndNonTempLocalIndex() const;
        RegSlot GetLocalsCount() const;

    public:
        const void* GetAuxPtrs(FunctionProxy::AuxPointerType e) const;
    }; // RemoteFunctionBody.

    struct RemoteScriptDebugDocument : public RemoteData<ScriptDebugDocument>
    {
        RemoteScriptDebugDocument(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
        void* GetDocumentId() const { return this->ReadField<void*>(offsetof(TargetType, m_documentText)); }
    };

    struct RemoteUtf8SourceInfo : RemoteData<Utf8SourceInfo>
    {
        RemoteUtf8SourceInfo(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}

        LPCUTF8 GetDebugModeSource() const
        {
            AssertMsg(this->ToTargetPtr()->debugModeSource != nullptr || this->ToTargetPtr()->debugModeSourceIsEmpty, "Source code wasn't mapped for debug mode.");
            return this->ToTargetPtr()->debugModeSource;
        }

        size_t GetDebugModeSourceLength() const
        {
            AssertMsg(this->ToTargetPtr()->debugModeSource != nullptr || this->ToTargetPtr()->debugModeSourceIsEmpty, "Source code wasn't mapped for debug mode.");
            return this->ToTargetPtr()->debugModeSourceLength; //Assert above checks debugModeSource pointer for null which is better than checking a size_t for 0, or some other number.
        }

        template<class TMapFunction>
        void MapFunctionUntil(TMapFunction mapFunction) const
        {
            RemoteFunctionBodyDicionary functionBodyDictionary(m_reader, this->ToTargetPtr()->functionBodyDictionary);
            functionBodyDictionary.MapUntil( [=] (FunctionBody* body) -> bool
            {
                RemoteFunctionBody remoteFunctionBody(m_reader, body);
                return mapFunction(remoteFunctionBody);
            });
        }

        UINT64 GetDocumentId() const
        {
            DebugDocument* debugDocument = this->ToTargetPtr()->m_debugDocument;

            if (debugDocument != nullptr)
            {
                RemoteScriptDebugDocument remoteScriptDebugDocument(m_reader, (ScriptDebugDocument*)debugDocument);
                return (UINT64)remoteScriptDebugDocument.GetDocumentId();
            }
            return 0;
        }

        bool Contains(DWORD offset) const
        {
            RemoteSRCINFO srcInfo(this->m_reader, this->ToTargetPtr()->m_srcInfo);
            uint begin = srcInfo->ulCharOffset;
            size_t end = srcInfo->ulCharOffset + this->ToTargetPtr()->m_cchLength;
            return (offset >= begin && offset < end);
        }
    };

    struct RemotePropertyIdOnRegSlotsContainer:
        public RemoteData<PropertyIdOnRegSlotsContainer>
    {
    private:
        RemoteArray<Js::PropertyId> m_propertyIdsForRegSlots;

    public:
        RemotePropertyIdOnRegSlotsContainer(IVirtualReader* reader, const TargetType* addr):
            RemoteData<TargetType>(reader, addr),
            m_propertyIdsForRegSlots(reader, ToTargetPtr()->propertyIdsForRegSlots)
        {}

        void FetchItemAt(uint index, RemoteFunctionBody* pFuncBody, _Out_ Js::PropertyId* pPropId, _Out_ RegSlot* pRegSlot) const;
    };

    struct RemoteFrameDisplay: public RemoteData<FrameDisplay>
    {
    private:
        RemoteArray<Js::Var> m_scopes;

    public:
        RemoteFrameDisplay(IVirtualReader* reader, const TargetType* addr):
            RemoteData<TargetType>(reader, addr),
            m_scopes(reader, GetFieldAddr<const Js::Var>(FrameDisplay::GetOffsetOfScopes()))
        {
        }

        void* GetItem(uint index) const;
    };

    struct RemoteTypePath: public RemoteData<TypePath, DynamicDataBuffer>
    {
        RemoteTypePath(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType, DynamicDataBuffer>(reader, addr) {}
        bool TryLookup(Js::PropertyId propId, int typePathLength, PropertyIndex* index);
        RemotePropertyRecord operator[] (const int index) const;
    };

    struct RemoteJavascriptBoolean:
        public RemoteData<JavascriptBoolean>
    {
        RemoteJavascriptBoolean(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
        static bool GetValue(IVirtualReader* reader, Js::Var var);
    };

    struct RemoteJavascriptSymbol:
        public RemoteData<JavascriptSymbol>
    {
        RemoteJavascriptSymbol(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
        static CString GetValue(IVirtualReader* reader, Js::Var var);
    };

    struct RemoteJavascriptNumber
#if !FLOATVAR
        : public RemoteData<JavascriptNumber>
#endif
    {
#if !FLOATVAR
        RemoteJavascriptNumber(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
#endif
        static double GetValue(IVirtualReader* reader, Js::Var var);
    };

    struct RemoteJavascriptBooleanObject: public RemoteData<JavascriptBooleanObject>
    {
        RemoteJavascriptBooleanObject(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
        bool GetValue();
        static bool GetValue(IVirtualReader* reader, Js::Var var);
    };

    struct RemoteJavascriptSymbolObject : public RemoteData<JavascriptSymbolObject>
    {
        RemoteJavascriptSymbolObject(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
        CString GetValue();
        static CString GetValue(IVirtualReader* reader, Js::Var var);
    };

    struct RemoteJavascriptNumberObject: public RemoteData<JavascriptNumberObject>
    {
        RemoteJavascriptNumberObject(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
        double GetValue();
        static double GetValue(IVirtualReader* reader, Js::Var var);
    };

    struct RemoteJavascriptStringObject: public RemoteData<JavascriptStringObject>
    {
        RemoteJavascriptStringObject(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
        JavascriptString* GetValue() { return ReadField<JavascriptString*>(offsetof(TargetType, value)); }
    };

    struct RemoteArgumentsObject: public RemoteData<ArgumentsObject>
    {
        RemoteArgumentsObject(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
        Js::Var GetCaller(
            const InspectionContext* inspectionContext,
            RemoteThreadContext* threadContext,
            RemoteScriptContext* scriptContext);

    private:
        bool AdvanceWalkerToArgsFrame(const InspectionContext* inspectionContext, RemoteStackWalker* walker);

        Js::Var GetCaller(
            const InspectionContext* inspectionContext,
            RemoteScriptContext* scriptContext,
            RemoteStackWalker* walker,
            JavascriptFunction* nullObject,
            bool skipGlobal);
    };

    typedef RemoteData<Js::JavascriptError> RemoteJavascriptError;
    typedef RemoteData<Js::JavascriptExceptionObject> RemoteJavascriptExceptionObject;
    typedef RemoteData<ThreadContext::RecyclableData> RemoteRecyclableData;
    typedef RemoteData<JavascriptDate> RemoteJavascriptDate;
    typedef RemoteRecyclableObjectBase<JavascriptVariantDate> RemoteJavascriptVariantDate;

    struct RemoteRegexPattern: public RemoteData<UnifiedRegex::RegexPattern>
    {
        RemoteRegexPattern(IVirtualReader* reader, const TargetType* addr)
            : RemoteData<TargetType>(reader, addr)
        {
            // Cache the flags for the regex pattern so we don't have to read them
            // from the remote process multiple times when checking if they're set.

            RemoteData<UnifiedRegex::Program> program(GetReader(), ToTargetPtr()->rep.unified.program);
            this->cachedFlags = program->flags;
        }

        bool IsGlobal() const;
        bool IsMultiline() const;
        bool IsIgnoreCase() const;
        bool IsUnicode() const;
        bool IsSticky() const;

    private:
        bool IsFlagSet(UnifiedRegex::RegexFlags flag) const;

        UnifiedRegex::RegexFlags cachedFlags;
    };

    struct RemoteJavascriptRegExp: public RemoteData<JavascriptRegExp>
    {
        RemoteJavascriptRegExp(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
        void GetSource(PCWSTR* pSource, charcount_t* pLength);
        CString GetSource();
        CString GetOptions(bool IsES6UnicodeExtensionsEnabled = false, bool isEs6RegExpStickyFlagEnabled = false) const;

        Js::Var GetLastIndex() const
        {
            return ToTargetPtr()->lastIndexVar;
        }

        bool IsGlobal() const
        {
            RemoteRegexPattern pattern = RemoteRegexPattern(GetReader(), ToTargetPtr()->pattern);
            return pattern.IsGlobal();
        }

        bool IsMultiline() const
        {
            RemoteRegexPattern pattern = RemoteRegexPattern(GetReader(), ToTargetPtr()->pattern);
            return pattern.IsMultiline();
        }

        bool IsIgnoreCase() const
        {
            RemoteRegexPattern pattern = RemoteRegexPattern(GetReader(), ToTargetPtr()->pattern);
            return pattern.IsIgnoreCase();
        }

        bool IsUnicode() const
        {
            RemoteRegexPattern pattern = RemoteRegexPattern(GetReader(), ToTargetPtr()->pattern);
            return pattern.IsUnicode();
        }

        bool IsSticky() const
        {
            RemoteRegexPattern pattern = RemoteRegexPattern(GetReader(), ToTargetPtr()->pattern);
            return pattern.IsSticky();
        }
    };

    struct RemoteExternalType: public RemoteData<ExternalType>
    {
        RemoteExternalType(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
        Js::PropertyId GetNameId() { return ReadField<Js::PropertyId>(offsetof(TargetType, nameId)); }
    };

    struct RemoteExternalObject: public RemoteRecyclableObjectBase<ExternalObject>
    {
        RemoteExternalObject(IVirtualReader* reader, const TargetType* addr) : RemoteRecyclableObjectBase(reader, addr) {}
        const PropertyRecord* GetClassName();
        bool IsProjectionObjectInstance(DebugClient* debugClient) const;
    };

    typedef RemoteData<Projection::ProjectionObjectInstance> RemoteProjectionObjectInstance;

    struct RemoteCustomExternalObject: public RemoteData<CustomExternalObject>
    {
        RemoteCustomExternalObject(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
        void* ReadExtensionObject() { return ReadField<void*>(sizeof(TargetType)); }
    };

    template<typename T>
    struct RemoteSparseArraySegment: public RemoteData<SparseArraySegment<T>>
    {
    private:
        RemoteArray<T> m_items;

    public:
        RemoteSparseArraySegment(IVirtualReader* reader, const TargetType* addr):
            RemoteData<TargetType>(reader, addr),
            m_items(reader, GetFieldAddr<T>(offsetof(TargetType, elements)))
        {}

        T Item(uint i) const
        {
            return m_items[i];
        }
    };

    typedef RemoteData<JavascriptArray> RemoteJavascriptArray;
    typedef RemoteData<JavascriptNativeIntArray> RemoteJavascriptNativeIntArray;
    typedef RemoteData<JavascriptNativeFloatArray> RemoteJavascriptNativeFloatArray;
    typedef RemoteData<ES5Array> RemoteES5Array;

    template <class T, bool clamped, class Target>
    struct RemoteBufferArray: RemoteData<Target>
    {
    private:
        RemoteArray<T> m_buffer;

    public:
        RemoteBufferArray(IVirtualReader* reader, const TargetType* addr):
            RemoteData<TargetType>(reader, addr),
            m_buffer(reader, reinterpret_cast<T*>(ToTargetPtr()->buffer))
        {}

        T Item(uint i) const
        {
            return m_buffer[i];
        }
    };

    template <class T, bool clamped = false, class Target = TypedArray<T, clamped>>
    struct RemoteTypedArray: public RemoteBufferArray<T, clamped, Target>
    {
    public:
        RemoteTypedArray(IVirtualReader* reader, const TargetType* addr):
            RemoteBufferArray<T, clamped, Target>(reader, addr)
        {}

        uint GetLength()
        {
            return ToTargetPtr()->length;
        }
    };

    struct RemoteArrayBuffer: public RemoteData<ArrayBuffer>
    {
        RemoteArrayBuffer(IVirtualReader* reader, const TargetType* addr)
            : RemoteData<ArrayBuffer>(reader, addr) {}
    };

    struct RemoteGlobalObject: public RemoteData<GlobalObject>
    {
        RemoteGlobalObject(IVirtualReader* reader, const TargetType* addr) : RemoteData<TargetType>(reader, addr) {}
        Js::Var ToThis();
    };

    struct RemoteDebuggerScope: public RemoteData<DebuggerScope>
    {
        RemoteDebuggerScope(IVirtualReader* reader, const TargetType* addr) : RemoteData<DebuggerScope>(reader, addr) {}

        // Check if the property is there in the scope
        bool ContainsProperty(RemoteStackFrame* frame, Js::PropertyId propertyId, Js::RegSlot location, DebuggerScopeProperty* outProperty = nullptr);
        bool ContainsValidProperty(RemoteStackFrame* frame, Js::PropertyId propertyId, Js::RegSlot location, int offset, bool* isInDeadZone);
    };

    struct RemoteDebuggingFlags : public RemoteData<DebuggingFlags>
    {
        RemoteDebuggingFlags(IVirtualReader* reader, const TargetType* addr) : RemoteData<DebuggingFlags>(reader, addr) {}
        void SetForceInterpreter(bool value);
    };

    //
    // Remote proxy for Js::Configuration (the one that encapsulates phases, flags, switches, etc.)
    //
    class RemoteConfiguration
    {
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    private:
        static RemoteConfiguration s_instance;
        BYTE m_data[sizeof(Configuration)];
        bool m_isInitialized;
        void EnsureInitialize(IVirtualReader* reader, const Configuration* addr);
    public:
        RemoteConfiguration() : m_isInitialized(false) {}
        static RemoteConfiguration* GetInstance();
        static void EnsureInstance(IVirtualReader* reader, const Configuration* addr);
        Configuration* ToTargetPtr() { return reinterpret_cast<Configuration*>(m_data); }

        bool PhaseOff1(Phase phase);
        bool PhaseOn1(Phase phase);
#endif ENABLE_DEBUG_CONFIG_OPTIONS
    public:
        static void SetHybridDebugging(IVirtualReader* reader, const Configuration* addr);
    };

} // namespace JsDiag.
