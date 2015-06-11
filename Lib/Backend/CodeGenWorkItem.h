//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

class Func;

struct CodeGenWorkItem
{
protected:
    CodeGenWorkItem(CodeGenWorkItemType type, Js::FunctionBody *const functionBody)
        : codeAddress(null)
        , functionBody(functionBody)
        , type(type)
        , jitMode(ExecutionMode::Interpreter)
    {
        Assert(functionBody);
    }

    Js::FunctionBody *const functionBody;
    size_t codeAddress;
    ptrdiff_t codeSize;
    ushort pdataCount;
    ushort xdataSize;

    CodeGenWorkItemType type;
    ExecutionMode jitMode;

public:
    bool IsInMemoryWorkItem() const { return type != JsFunctionSerializedType; }
#if DBG
    virtual bool DbgIsInMemoryWorkItem() const abstract;
#endif

    virtual uint GetByteCodeCount() const abstract; 
    virtual uint GetFunctionNumber() const abstract; 
    virtual size_t GetDisplayName(_Out_writes_opt_z_(sizeInChars) WCHAR* displayName, _In_ size_t sizeInChars) abstract;
    virtual void GetEntryPointAddress(void** entrypoint, ptrdiff_t *size) abstract;
    virtual uint GetInterpretedCount() const abstract;
    virtual void Delete() abstract;
#if DBG_DUMP | defined(VTUNE_PROFILING)
    virtual void RecordNativeMap(uint32 nativeOffset, uint32 statementIndex) abstract;
#endif
    virtual void RecordNativeThrowMap(Js::SmallSpanSequenceIter& iter, uint32 nativeOffset, uint32 statementIndex) abstract;
#if DBG_DUMP
    virtual void DumpNativeOffsetMaps() abstract;
    virtual void DumpNativeThrowSpanSequence() abstract;
#endif
#if _M_X64 || _M_ARM
    virtual size_t RecordUnwindInfo(size_t offset, BYTE *unwindInfo, size_t size) abstract;
#endif
#if _M_ARM
    virtual void RecordPdataEntry(int index, DWORD beginAddress, DWORD unwindData) abstract;
    virtual void FinalizePdata() abstract;
#endif
    virtual void FinalizeNativeCode(Func *func) abstract;
    virtual void InitializeReader(Js::ByteCodeReader &reader, Js::StatementReader &statementReader) abstract;
    virtual void RecordNativeCodeSize(Func *func, size_t bytes, ushort pdataCount, ushort xdataSize) abstract;
    virtual void RecordNativeCode(Func *func, const BYTE* sourceBuffer) abstract;
    virtual void RecordNativeRelocation(size_t offset) abstract;
    virtual void OnWorkItemProcessFail(NativeCodeGenerator *codeGen) abstract;

    ExecutionMode GetJitMode()
    {
        return jitMode;
    }

    CodeGenWorkItemType Type() const { return type; }

    Js::ScriptContext* GetScriptContext() 
    {
        return functionBody->GetScriptContext();
    }

    Js::FunctionBody* GetFunctionBody() const
    {
        return functionBody;
    }

    void SetCodeAddress(size_t codeAddress) { this->codeAddress = codeAddress; }
    size_t GetCodeAddress() { return codeAddress; }

    void SetCodeSize(ptrdiff_t codeSize) { this->codeSize = codeSize; }
    ptrdiff_t GetCodeSize() { return codeSize; }

    void SetPdataCount(ushort pdataCount) { this->pdataCount = pdataCount; }
    ushort GetPdataCount() { return pdataCount; }

    void SetXdataSize(ushort xdataSize) { this->xdataSize = xdataSize; }
    ushort GetXdataSize() { return xdataSize; }

    InMemoryCodeGenWorkItem *AsInMemoryWorkItem()
    {
        Assert(IsInMemoryWorkItem());
        return (InMemoryCodeGenWorkItem *)this;
    }
};

struct SerializedCodeGenWorkItem sealed : public CodeGenWorkItem
{
public:
    SerializedCodeGenWorkItem(Js::FunctionBody *const functionBody, PEWriter *writer)
        : CodeGenWorkItem(JsFunctionSerializedType, functionBody)
        , writer(writer)
    {
        // TODO [paulv]: Consider how we can use this.
        this->jitMode = ExecutionMode::FullJit;
    }
#if DBG
    virtual bool DbgIsInMemoryWorkItem() const override { return false; }
#endif    
    uint GetByteCodeCount() const override
    {
        return functionBody->GetByteCodeCount() +  functionBody->GetConstantCount();
    }

    uint GetFunctionNumber() const override 
    {
        return functionBody->GetFunctionNumber();
    }

    size_t GetDisplayName(_Out_writes_opt_z_(sizeInChars) WCHAR* displayName, _In_ size_t sizeInChars) override
    {
        const WCHAR* name = functionBody->GetExternalDisplayName();
        size_t nameSizeInChars = wcslen(name) + 1;
        size_t sizeInBytes = nameSizeInChars * sizeof(WCHAR);
        if(displayName == NULL || sizeInChars < nameSizeInChars)
        {
           return nameSizeInChars;
        }
        js_memcpy_s(displayName, sizeInChars * sizeof(WCHAR), name, sizeInBytes);
        return nameSizeInChars;
    }

    void GetEntryPointAddress(void** entrypoint, ptrdiff_t *size) override
    {
         Assert(entrypoint);
         *entrypoint = (void *)GetCodeAddress();
         *size = GetCodeSize();
    }

    uint GetInterpretedCount() const override
    {
        return this->functionBody->interpretedCount;
    }

    void Delete() override
    {
        HeapDelete(this);
    }

    void InitializeReader(Js::ByteCodeReader &reader, Js::StatementReader &statementReader) override
    {
        reader.Create(this->functionBody);
        statementReader.Create(this->functionBody);
    }

#if DBG_DUMP | defined(VTUNE_PROFILING)
    void RecordNativeMap(uint32 nativeOffset, uint32 statementIndex) override
    {
        writer->RecordNativeMap(nativeOffset, statementIndex);
    }
#endif

#if DBG_DUMP
    void DumpNativeOffsetMaps() override
    {
        writer->DumpNativeOffsetMaps();
    }

    void DumpNativeThrowSpanSequence() override
    {
        writer->DumpNativeThrowSpanSequence();
    }
#endif

    void RecordNativeThrowMap(Js::SmallSpanSequenceIter& iter, uint32 nativeOffset, uint32 statementIndex) override
    {
        writer->RecordNativeThrowMap(nativeOffset, statementIndex);
    }

#if _M_X64 || _M_ARM
    size_t RecordUnwindInfo(size_t offset, BYTE *unwindInfo, size_t size) override
    {
#if _M_X64
        Assert(offset == 0);
#endif
        return writer->RecordUnwindInfo(offset, unwindInfo, size);
    }
#endif
#if _M_ARM
    void RecordPdataEntry(int index, DWORD beginAddress, DWORD unwindData) override
    {
        writer->RecordPdataEntry(index, beginAddress, unwindData);
    }

    void FinalizePdata() override
    {
        // Nothing to do.
    }
#endif

    void RecordNativeCodeSize(Func *func, size_t bytes, ushort pdataCount, ushort xdataSize) override
    {
        SetCodeAddress(writer->RecordNativeCodeSize(bytes, pdataCount, xdataSize));
        SetCodeSize(bytes);
        SetPdataCount(pdataCount);
        SetXdataSize(xdataSize);
    }

    void RecordNativeCode(Func *func, const BYTE* sourceBuffer) override
    {
        writer->RecordNativeCode(GetCodeSize(), sourceBuffer);
    }

    void RecordNativeRelocation(size_t offset) override
    {
        writer->RecordNativeRelocation(offset);
    }

    void FinalizeNativeCode(Func *func) override
    {
        writer->FinalizeNativeCode();
    }

    void OnWorkItemProcessFail(NativeCodeGenerator *codeGen) override
    {
        // Nothing to do.
    }

private:
    PEWriter *writer;
};

struct InMemoryCodeGenWorkItem : public CodeGenWorkItem, public JsUtil::Job
{
protected:
    InMemoryCodeGenWorkItem(
        JsUtil::JobManager *const manager,
        Js::FunctionBody *const functionBody,
        Js::EntryPointInfo* entryPointInfo,
        bool isJitInDebugMode,
        CodeGenWorkItemType type);
    ~InMemoryCodeGenWorkItem();

    virtual uint GetLoopNumber() const
    {
        return Js::LoopHeader::NoLoop;
    }   

protected:
    // This reference does not keep the entry point alive, and it's not expected to
    // The entry point is kept alive only if it's in the JIT queue, in which case recyclableData will be allocated and will keep the entry point alive
    // If the entry point is getting collected, it'll actually removed itself from the work item list so this work item will get deleted when the EntryPointInfo goes away
    Js::EntryPointInfo* entryPointInfo;
    Js::CodeGenRecyclableData *recyclableData;

private:
    bool isInJitQueue;                  // indicates if the work item has been added to the global jit queue
    bool isJitInDebugMode;              // Whether JIT is in debug mode for this work item.
    bool isAllocationCommitted;         // Whether the EmitBuffer allocation has been committed

    QueuedFullJitWorkItem *queuedFullJitWorkItem;
    EmitBufferAllocation *allocation;

#ifdef IR_VIEWER
public:  // FIXME (t-doilij) this isn't how it should be
    bool isRejitIRViewerFunction;           // re-JIT function for IRViewer object generation
    Js::DynamicObject *irViewerOutput;      // hold results of IRViewer APIs
    Js::ScriptContext *irViewerRequestContext;  // FIXME (t-doilij) keep track of the request context (unneeded)

    Js::DynamicObject * GetIRViewerOutput(Js::ScriptContext *scriptContext)
    {
        if (!irViewerOutput)
        {
            irViewerOutput = scriptContext->GetLibrary()->CreateObject();
        }

        return irViewerOutput;
    }

    void SetIRViewerOutput(Js::DynamicObject *output)
    {
        Assert(IsInMemoryWorkItem());
        irViewerOutput = output;
    }

#endif
private:

    void SetAllocation(EmitBufferAllocation *allocation) 
    { 
        Assert(IsInMemoryWorkItem());
        this->allocation = allocation; 
    }
    EmitBufferAllocation *GetAllocation() { return allocation; }

public:
    Js::EntryPointInfo* GetEntryPoint() const
    {
        return this->entryPointInfo;
    }
#if DBG
    virtual bool DbgIsInMemoryWorkItem() const { return true; }
#endif
    void RecordNativeCodeSize(Func *func, size_t bytes, ushort pdataCount, ushort xdataSize) override;
    void RecordNativeCode(Func *func, const BYTE* sourceBuffer) override;

    void RecordNativeRelocation(size_t offset) override
    {
        // Nothing to do.
    }

    Js::CodeGenRecyclableData *RecyclableData() const
    {
        return recyclableData;
    }

    void SetRecyclableData(Js::CodeGenRecyclableData *const recyclableData)
    {
        Assert(recyclableData);
        Assert(!this->recyclableData);

        this->recyclableData = recyclableData;
    }

    void SetEntryPointInfo(Js::EntryPointInfo* entryPointInfo) 
    { 
        this->entryPointInfo = entryPointInfo;
    }

public:
    void ResetJitMode()
    {
        jitMode = ExecutionMode::Interpreter;
    }

    void SetJitMode(const ExecutionMode jitMode)
    {
        this->jitMode = jitMode;
        VerifyJitMode();
    }

    void VerifyJitMode() const
    {
        Assert(jitMode == ExecutionMode::SimpleJit || jitMode == ExecutionMode::FullJit);
        Assert(jitMode != ExecutionMode::SimpleJit || GetFunctionBody()->DoSimpleJit());
        Assert(jitMode != ExecutionMode::FullJit || GetFunctionBody()->DoFullJit());
    }

    void OnAddToJitQueue();
    void OnRemoveFromJitQueue(NativeCodeGenerator* generator);

public:
    bool ShouldSpeculativelyJit(uint byteCodeSizeGenerated) const;
private:
    bool ShouldSpeculativelyJitBasedOnProfile() const;

public:
    bool IsInJitQueue() const
    {
        return isInJitQueue;
    }

    bool IsJitInDebugMode() const
    {
        return isJitInDebugMode;
    }

#if _M_X64 || _M_ARM
    size_t RecordUnwindInfo(size_t offset, BYTE *unwindInfo, size_t size) override
    {
#if _M_X64
        Assert(offset == 0);
        Assert(XDATA_SIZE >= size);
        js_memcpy_s(GetAllocation()->allocation->xdata.address, XDATA_SIZE, unwindInfo, size);
        return 0;
#else
        BYTE *xdataFinal = GetAllocation()->allocation->xdata.address + offset;
    
        Assert(xdataFinal);
        Assert(((DWORD)xdataFinal & 0x3) == 0); // 4 byte aligned
        js_memcpy_s(xdataFinal, size, unwindInfo, size); 
        return (size_t)xdataFinal;
#endif
    }
#endif

#if _M_ARM
    void RecordPdataEntry(int index, DWORD beginAddress, DWORD unwindData) override
    {
        RUNTIME_FUNCTION *function = GetAllocation()->allocation->xdata.GetPdataArray() + index;
        function->BeginAddress = beginAddress;
        function->UnwindData = unwindData;
    }

    void FinalizePdata() override
    {
        GetAllocation()->allocation->RegisterPdata((ULONG_PTR)GetCodeAddress(), GetCodeSize());
    }
#endif

    void OnWorkItemProcessFail(NativeCodeGenerator *codeGen) override;

    void FinalizeNativeCode(Func *func) override;

    void RecordNativeThrowMap(Js::SmallSpanSequenceIter& iter, uint32 nativeOffset, uint32 statementIndex) override
    {
        this->functionBody->RecordNativeThrowMap(iter, nativeOffset, statementIndex, this->GetEntryPoint(), GetLoopNumber());
    }

    QueuedFullJitWorkItem *GetQueuedFullJitWorkItem() const;
    QueuedFullJitWorkItem *EnsureQueuedFullJitWorkItem();

private:
    bool ShouldSpeculativelyJit() const;
};

struct JsFunctionCodeGen sealed : public InMemoryCodeGenWorkItem
{
    JsFunctionCodeGen(
        JsUtil::JobManager *const manager,
        Js::FunctionBody *const functionBody,
        Js::EntryPointInfo* entryPointInfo,
        bool isJitInDebugMode)
        : InMemoryCodeGenWorkItem(manager, functionBody, entryPointInfo, isJitInDebugMode, JsFunctionType)
    {
    }

public:
    uint GetByteCodeCount() const override
    {
        return functionBody->GetByteCodeCount() +  functionBody->GetConstantCount();
    }

    uint GetFunctionNumber() const override 
    {
        return functionBody->GetFunctionNumber();
    }

    size_t GetDisplayName(_Out_writes_opt_z_(sizeInChars) WCHAR* displayName, _In_ size_t sizeInChars) override
    {
        const WCHAR* name = functionBody->GetExternalDisplayName();
        size_t nameSizeInChars = wcslen(name) + 1;
        size_t sizeInBytes = nameSizeInChars * sizeof(WCHAR);
        if(displayName == NULL || sizeInChars < nameSizeInChars)
        {
           return nameSizeInChars;
        }
        js_memcpy_s(displayName, sizeInChars * sizeof(WCHAR), name, sizeInBytes);
        return nameSizeInChars;
    }

    void GetEntryPointAddress(void** entrypoint, ptrdiff_t *size) override
    {
         Assert(entrypoint);
         *entrypoint = this->GetEntryPoint()->address;
         *size = this->GetEntryPoint()->GetCodeSize();
    }

    uint GetInterpretedCount() const override
    {
        return this->functionBody->interpretedCount;
    }

    void Delete() override
    {
        HeapDelete(this);
    }

#if DBG_DUMP | defined(VTUNE_PROFILING)
    void RecordNativeMap(uint32 nativeOffset, uint32 statementIndex) override
    {
        Js::FunctionEntryPointInfo* info = (Js::FunctionEntryPointInfo*) this->GetEntryPoint();

        info->RecordNativeMap(nativeOffset, statementIndex);
    }
#endif

#if DBG_DUMP
    virtual void DumpNativeOffsetMaps() override
    {
        this->GetEntryPoint()->DumpNativeOffsetMaps();
    }

    virtual void DumpNativeThrowSpanSequence() override
    {
        this->GetEntryPoint()->DumpNativeThrowSpanSequence();
    }
#endif

    virtual void InitializeReader(Js::ByteCodeReader &reader, Js::StatementReader &statementReader) override
    {
        reader.Create(this->functionBody, 0, IsJitInDebugMode());
        statementReader.Create(this->functionBody, 0, IsJitInDebugMode());
    }
};

struct JsLoopBodyCodeGen sealed : public InMemoryCodeGenWorkItem
{
    JsLoopBodyCodeGen(
        JsUtil::JobManager *const manager,
        Js::FunctionBody *const functionBody,
        Js::EntryPointInfo* entryPointInfo,
        bool isJitInDebugMode)
        : InMemoryCodeGenWorkItem(manager, functionBody, entryPointInfo, isJitInDebugMode, JsLoopBodyWorkItemType)
        , symIdToValueTypeMap(null)
    { 
    }

    Js::LoopHeader * loopHeader;
    typedef JsUtil::BaseDictionary<uint, ValueType, HeapAllocator> SymIdToValueTypeMap;
    SymIdToValueTypeMap *symIdToValueTypeMap;

    uint GetLoopNumber() const override
    {
        return functionBody->GetLoopNumber(loopHeader);
    }   

    uint GetByteCodeCount() const override
    {
        return (loopHeader->endOffset - loopHeader->startOffset) + functionBody->GetConstantCount();
    }

    uint GetFunctionNumber() const override
    {
        return functionBody->GetFunctionNumber();
    }

    size_t GetDisplayName(_Out_writes_opt_z_(sizeInChars) WCHAR* displayName, _In_ size_t sizeInChars) override
    {
         return EtwTrace::GetLoopBodyName(this->functionBody, this->loopHeader, displayName, sizeInChars);
    }

    void GetEntryPointAddress(void** entrypoint, ptrdiff_t *size) override
    {
        Assert(entrypoint);
        Js::EntryPointInfo * entryPoint = this->GetEntryPoint();
        *entrypoint = entryPoint->address;
        *size = entryPoint->GetCodeSize();
    }

    uint GetInterpretedCount() const override
    {
        return loopHeader->interpretCount;
    }

#if DBG_DUMP | defined(VTUNE_PROFILING)
    void RecordNativeMap(uint32 nativeOffset, uint32 statementIndex) override
    {
        this->GetEntryPoint()->RecordNativeMap(nativeOffset, statementIndex);        
    }
#endif

#if DBG_DUMP
    virtual void DumpNativeOffsetMaps() override
    {
        this->GetEntryPoint()->DumpNativeOffsetMaps();
    }

    virtual void DumpNativeThrowSpanSequence() override
    {
        this->GetEntryPoint()->DumpNativeThrowSpanSequence();
    }
#endif

    void Delete() override
    {
        HeapDelete(this);
    }

    virtual void InitializeReader(Js::ByteCodeReader &reader, Js::StatementReader &statementReader) override
    {
        reader.Create(this->functionBody, this->loopHeader->startOffset, IsJitInDebugMode());
        statementReader.Create(this->functionBody, this->loopHeader->startOffset, IsJitInDebugMode());
    }

    ~JsLoopBodyCodeGen()
    {
        if (this->symIdToValueTypeMap)
        {
            HeapDelete(this->symIdToValueTypeMap);
            this->symIdToValueTypeMap = NULL;
        }
    }
};
