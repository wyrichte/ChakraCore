//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#include "stdafx.h"

namespace Js
{
    const char16 Constants::AnonymousFunction[] = _u("Anonymous function");
    const char16 Constants::Anonymous[] = _u("anonymous");
    const char16 Constants::FunctionCode[] = _u("Function code");
    const char16 Constants::GlobalFunction[] = _u("glo");
    const char16 Constants::GlobalCode[] = _u("Global code");
    const char16 Constants::EvalCode[] = _u("eval code");
    const char16 Constants::UnknownScriptCode[] = _u("Unknown script code");
}
// --- Dummy definitions - to satisfy the linker ---
__declspec(noinline) void DebugHeap_OOM_fatal_error()
{
    Assert(false);
};
// another dummy definition for linker
hash_t PrimePolicy::ModPrime(hash_t key, uint prime, int modFunctionIndex)
{
    return key & prime;
}
NoCheckHeapAllocator NoCheckHeapAllocator::Instance;
HANDLE NoCheckHeapAllocator::processHeap;

#if defined(RECYCLER_WRITE_BARRIER) && ENABLE_DEBUG_CONFIG_OPTIONS
namespace Memory
{
    FN_VerifyIsNotBarrierAddress* g_verifyIsNotBarrierAddress = nullptr;
}
#endif
// -- end of dummy definitions ----

namespace JsDiag
{
    using namespace Js;
    using namespace JsUtil;

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    //static
    RemoteConfiguration RemoteConfiguration::s_instance;
#endif ENABLE_DEBUG_CONFIG_OPTIONS


    bool RemoteInterval::Includes(int value)
    {
        return this->ToTargetPtr()->begin <= value && value <= this->ToTargetPtr()->end;
    }

    // Returns address of ThreadContext in remote address space.
    ThreadContext* RemoteThreadContextTLSEntry::GetThreadContext()
    {
        // Note: it's important to do static_cast here as e.g. reinterpret_cast doesn't take care of presense of vtable,
        //       while staic_cast perfoms the cast correctly.
        return static_cast<ThreadContext*>(this->ToTargetPtr()->threadContext);
    }

    template <class T>
    ScriptContext* RemoteRecyclableObjectBase<T>::GetScriptContext()
    {
        // Idea: obj->GetType()->GetLibrary()->GetScriptContext().
        Js::Type* typeAddr = this->ToTargetPtr()->type;
        Assert(typeAddr);
        if (typeAddr)
        {
            RemoteType type(m_reader, typeAddr);
            Js::JavascriptLibrary* libAddr = type->javascriptLibrary;
            Assert(libAddr);
            if (libAddr)
            {
                RemoteJavascriptLibrary lib(m_reader, libAddr);
                return lib.GetScriptContext();
            }
        }

        return nullptr;
    }

    template <class T>
    RecyclableObject* RemoteRecyclableObjectBase<T>::GetPrototype()
    {
        RemoteType type(m_reader, ToTargetPtr()->GetType());
        return type->GetPrototype();
    }

    template <class T>
    JavascriptMethod RemoteRecyclableObjectBase<T>::GetEntrypoint()
    {
        RemoteType type(m_reader, ToTargetPtr()->GetType());
        return type->GetEntryPoint();
    }

    template <class T>
    JavascriptLibrary* RemoteRecyclableObjectBase<T>::GetLibrary()
    {
        RemoteType type(m_reader, ToTargetPtr()->GetType());
        return type->GetLibrary();
    }

    template struct RemoteRecyclableObjectBase<RecyclableObject>;
    template struct RemoteRecyclableObjectBase<DynamicObject>;

    RemoteJavascriptLibrary::RemoteJavascriptLibrary(IVirtualReader* reader, const ScriptContext* scriptContext):
        RemoteData(reader, RemoteScriptContext(reader, scriptContext)->GetLibrary())
    {
    }

    bool RemoteThreadContext::DoInterruptProbe()
    {
        return
            (this->TestThreadContextFlag(ThreadContextFlagCanDisableExecution) && !DIAG_PHASE_OFF1(Js::InterruptProbePhase)) ||
            DIAG_PHASE_ON1(Js::InterruptProbePhase);
    }

    bool RemoteThreadContext::GetIsThreadBound()
    {
        return this->ToTargetPtr()->isThreadBound;
    }

    BOOL RemoteThreadContext::TestThreadContextFlag(ThreadContextFlags contextFlag)
    {
        return (this->ToTargetPtr()->threadContextFlags & contextFlag) != 0;
    }

    const PropertyRecord* RemoteThreadContext::GetPropertyName(Js::PropertyId propertyId)
    {
        if (!m_propertyMap)
        {
            const ThreadContext::PropertyMap * addr = ReadVirtual<ThreadContext::PropertyMap *>(
                GetFieldAddr<ThreadContext::PropertyMap *>(offsetof(ThreadContext, propertyMap)));
            m_propertyMap = new(oomthrow) RemoteDictionary<ThreadContext::PropertyMap>(m_reader, addr);
        }

        if (propertyId >= 0 && Js::IsInternalPropertyId(propertyId))
        {
            AssertMsg(false, "Would we ever see internal property id in DAC?");
        }

        int propertyIndex = propertyId - Js::PropertyIds::_none;
        if (propertyIndex < 0 || propertyIndex > m_propertyMap->GetLastIndex())
        {
            propertyIndex = 0;
        }

        return m_propertyMap->Item(propertyIndex).Value();
    }

    Js::JavascriptExceptionObject* RemoteThreadContext::GetUnhandledExceptionObject() const
    {
        // This method is in sync with ThreadContext::GetUnhandledExceptionObject() on the runtime side.
        RemoteRecyclableData recyclableData = RemoteRecyclableData(m_reader, this->ToTargetPtr()->recyclableData);
        return recyclableData->unhandledExceptionObject;
    }

    FunctionBody* RemoteFunctionInfo::GetFunction()
    {
        return (FunctionBody *)PointerValue(this->ToTargetPtr()->functionBodyImpl);
    }

    template <typename TTargetType>
    EntryPointInfo::State RemoteEntryPointInfo<TTargetType>::GetState()
    {
        Assert(this->ToTargetPtr()->state >= EntryPointInfo::NotScheduled && this->ToTargetPtr()->state <= EntryPointInfo::CleanedUp);
        return this->ToTargetPtr()->state;
    }

    template <typename TTargetType>
    bool RemoteEntryPointInfo<TTargetType>::IsCodeGenDone()
    {
        return this->GetState() == EntryPointInfo::CodeGenDone;
    }

    template <typename TTargetType>
    bool RemoteEntryPointInfo<TTargetType>::HasNativeAddress()
    {
        return this->ToTargetPtr()->nativeAddress != nullptr;
    }

    template <typename TTargetType>
    DWORD_PTR RemoteEntryPointInfo<TTargetType>::GetNativeAddress()
    {
        Assert(this->GetState() == EntryPointInfo::CodeGenRecorded || this->GetState() == EntryPointInfo::CodeGenDone);
        return (DWORD_PTR)this->ToTargetPtr()->nativeAddress;
    }

    template <typename TTargetType>
    SmallSpanSequence* RemoteEntryPointInfo<TTargetType>::GetNativeThrowSpanSequence()
    {
        Assert(this->GetState() != EntryPointInfo::State::NotScheduled);
        Assert(this->GetState() != EntryPointInfo::State::CleanedUp);
        return this->ToTargetPtr()->nativeThrowSpanSequence;
    }

    template <typename TTargetType>
    ptrdiff_t RemoteEntryPointInfo<TTargetType>::GetCodeSize()
    {
        Assert(this->GetState() == EntryPointInfo::CodeGenRecorded || this->GetState() == EntryPointInfo::CodeGenDone);
        return this->ToTargetPtr()->codeSize;
    }

    JavascriptLibrary* RemoteJavascriptFunction::GetLibrary()
    {
        RemoteType type(m_reader, ToTargetPtr()->GetType());
        return type->GetLibrary();
    }

    FunctionBody* RemoteJavascriptFunction::GetFunction() const
    {
        FunctionInfo* functionInfoAddr = this->ToTargetPtr()->functionInfo;
        Assert(functionInfoAddr);

        RemoteFunctionInfo functionInfo(m_reader, functionInfoAddr);
        return functionInfo.GetFunction();
    }

    bool RemoteJavascriptFunction::IsLibraryCode() const
    {
        if (!IsScriptFunction())
        {
            return true;
        }

        RemoteFunctionBody functionBody(this->GetReader(), this->GetFunction());
        return RemoteUtf8SourceInfo(this->GetReader(), functionBody.GetUtf8SourceInfo())->GetIsLibraryCode();
    }

    bool RemoteJavascriptFunction::IsScriptFunction() const
    {
        FunctionInfo* functionInfoAddr = this->ToTargetPtr()->functionInfo;
        Assert(functionInfoAddr);

        RemoteFunctionInfo functionInfo(m_reader, functionInfoAddr);
        return !!functionInfo->HasBody();
    }

    uint16 RemoteJavascriptFunction::GetLength()
    {
        FunctionBody* functionBodyAddr = GetFunction();
        Assert(functionBodyAddr != nullptr); // This is a script function, so the body will be there.

        RemoteFunctionBody functionBody(m_reader, functionBodyAddr);
        return functionBody->GetInParamsCount() - 1;
    }

    bool RemoteJavascriptFunction::IsStrictMode() const
    {
        // This method is in sync with JavascriptFunction::IsStrictMode() on the runtime side.
        FunctionInfo* functionInfoAddr = this->ToTargetPtr()->functionInfo;
        Assert(functionInfoAddr);

        RemoteFunctionInfo functionInfo(m_reader, functionInfoAddr);
        if (functionInfo->HasBody())
        {
            RemoteParseableFunctionInfo parseableFunctionInfo = RemoteParseableFunctionInfo(m_reader, functionInfo.GetFunction());
            return parseableFunctionInfo->GetIsStrictMode();
        }

        return false;
    }

    bool RemoteJavascriptFunction::HasRestrictedProperties() const
    {
        // This method is in sync with JavascriptFunction::HasRestrictedProperties() on the runtime side.
        FunctionInfo* functionInfoAddr = this->ToTargetPtr()->functionInfo;
        Assert(functionInfoAddr);

        RemoteFunctionInfo functionInfo(m_reader, functionInfoAddr);
        if (functionInfo->HasBody())
        {
            RemoteFunctionProxy parseableFunctionInfo = RemoteFunctionProxy(m_reader, functionInfo.GetFunction());
            return !(parseableFunctionInfo->IsClassMethod() || parseableFunctionInfo->IsClassConstructor() || parseableFunctionInfo->IsLambda());
        }

        return true;
    }

    FunctionBody* RemoteScriptFunction::GetFunction()
    {
        FunctionInfo* functionInfoAddr = this->ToTargetPtr()->functionInfo;
        Assert(functionInfoAddr);

        RemoteFunctionInfo functionInfo(m_reader, functionInfoAddr);
        return functionInfo.GetFunction();
    }

    //static
    InlinedFrameLayout* RemoteInlinedFrameLayout::FromPhysicalFrame(
        IVirtualReader* reader, InternalStackFrame* physicalFrame, void* entry, Js::ScriptFunction* parent, FunctionEntryPointInfo* entryPoint)
    {
        Assert(physicalFrame);
        Assert(parent);

        InlinedFrameLayout* first = nullptr;
        if (!physicalFrame->IsInStackCheckCode(entry))
        {
            void *frameBase = physicalFrame->FrameBase;
            first = (InlinedFrameLayout*)(((uint8*)frameBase) - RemoteScriptFunction(reader, parent).GetFrameHeight(entryPoint));
            Assert(first);
            first = RemoteInlinedFrameLayout(reader, first)->callInfo.Count ? first : nullptr;
        }

        return first;
    }

    InlinedFrameLayout* RemoteInlinedFrameLayout::Next()
    {
        InlinedFrameLayout* nextAddr = (InlinedFrameLayout*)(
            reinterpret_cast<BYTE*>(const_cast<InlinedFrameLayout*>(this->m_remoteAddr)) +  // start
            sizeof(InlinedFrameLayout) +                                                    // now we point to the start of argv
            this->ToTargetPtr()->callInfo.Count * sizeof(Js::Var));
        return RemoteInlinedFrameLayout(m_reader, nextAddr)->callInfo.Count ? nextAddr : nullptr;
    }

    uint32 RemoteScriptFunction::GetFrameHeight(FunctionEntryPointInfo* entryPointInfoAddr)
    {
        FunctionBody* functionBodyAddr = this->GetFunction();
        Assert(functionBodyAddr != nullptr);

        RemoteFunctionBody functionBody(m_reader, functionBodyAddr);
        return functionBody.GetFrameHeight(entryPointInfoAddr);
    }

    FrameDisplay* RemoteScriptFunction::GetEnvironment()
    {
        return ReadField<FrameDisplay*>(offsetof(ScriptFunction, environment));
    }

    bool RemoteInterpreterStackFrame::IsCurrentLoopNativeAddr(void* addr)
    {
        return this->ToTargetPtr()->currentLoopNum != LoopHeader::NoLoop;
    }

    ByteCodeReader* RemoteInterpreterStackFrame::GetReader()
    {
        return this->GetFieldAddr<ByteCodeReader>(offsetof(InterpreterStackFrame, m_reader));
    }

    Js::Var RemoteInterpreterStackFrame::GetReg(RegSlot reg)
    {
        const Js::Var* localSlots = GetFieldAddr<Js::Var>(offsetof(TargetType, m_localSlots));
        return ReadVirtual<Js::Var>(localSlots + reg);
    }

    FrameDisplay* RemoteInterpreterStackFrame::GetFrameDisplay(RegSlot frameDisplayRegister)
    {
        if (frameDisplayRegister != Constants::NoRegister && frameDisplayRegister != 0)
        {
            return ReadField<Js::FrameDisplay*>(offsetof(InterpreterStackFrame, localFrameDisplay));
        }
        else
        {
            RemoteScriptFunction func(m_reader, ToTargetPtr()->GetJavascriptFunction());
            return func.GetEnvironment();
        }
    }

    Js::Var RemoteInterpreterStackFrame::GetInnerScope(RegSlot scopeLocation)
    {
        RemoteFunctionBody functionBody(m_reader, ToTargetPtr()->GetFunctionBody());
        uint32 index = scopeLocation - functionBody.GetCounter(FunctionBody::CounterFields::FirstInnerScopeRegister);
        Js::Var* innerScopeArray = ReadField<Js::Var*>(offsetof(TargetType, innerScopeArray));
        return ReadVirtual<Js::Var>(innerScopeArray + index);
    }

    Js::Var RemoteInterpreterStackFrame::GetRootObject()
    {
        return GetReg(FunctionBody::RootObjectRegSlot);
    }

    int RemoteByteCodeReader::GetCurrentOffset()
    {
        const BYTE* currentLocation = this->ToTargetPtr()->m_currentLocation;
        const BYTE* startLocation = this->ToTargetPtr()->m_startLocation;
        int currentOffset =  (int)(currentLocation - startLocation);
        return currentOffset;
    }

    CustomHeap::InProcHeap* RemoteEmitBufferManager::GetAllocationHeap()
    {
        return this->GetFieldAddr<CustomHeap::InProcHeap>(offsetof(InProcEmitBufferManagerWithlock, allocationHeap));
    }

    bool RemoteSegment::IsInSegment(void* addr)
    {
        void* start = this->ToTargetPtr()->address;
        void* end = this->GetEndAddress();
        return (addr >= start && addr < end);
    }

    char* RemoteSegment::GetEndAddress()
    {
        return this->ToTargetPtr()->address + this->GetAvailablePageCount() * AutoSystemInfo::PageSize;
    }

    size_t RemoteSegment::GetAvailablePageCount()
    {
        return this->ToTargetPtr()->segmentPageCount - this->ToTargetPtr()->secondaryAllocPageCount;
    }

    bool RemotePageSegment::IsInSegment(void* address)
    {
        RemoteSegment segment(m_reader, m_remoteAddr);
        return segment.IsInSegment(address);
    }

    bool RemotePageSegment::IsFreeOrDecommitted(void* addr)
    {
        Assert(this->IsInSegment(addr));
        uint base = GetBitRangeBase(addr);
        return this->ToTargetPtr()->decommitPages.Test(base) || this->ToTargetPtr()->freePages.Test(base);
    }

    uint RemotePageSegment::GetBitRangeBase(void* addr)
    {
        uint base = ((uint)(((char *)addr) - this->ToTargetPtr()->address)) / AutoSystemInfo::PageSize;
        return base;
    }

    HeapPageAllocator<PreReservedVirtualAllocWrapper>* RemoteHeap::GetPreReservedHeapPageAllocator()
    {
        CustomHeap::InProcCodePageAllocators * codePageAllocators = this->ReadField<CustomHeap::InProcCodePageAllocators *>(offsetof(CustomHeap::InProcHeap, codePageAllocators));
        RemoteData<CustomHeap::InProcCodePageAllocators> remoteCodePageAllocators(this->m_reader, codePageAllocators);
        return remoteCodePageAllocators.GetFieldAddr<HeapPageAllocator<PreReservedVirtualAllocWrapper>>(offsetof(CustomHeap::InProcCodePageAllocators, preReservedHeapAllocator));
    }

    bool RemoteHeapPageAllocator::IsAddressFromAllocator(void* address)
    {
        size_t pageSegmentListOffsets[3];
        size_t segmentListOffset;
        RemoteHeapPageAllocator::GetSegmentOffsets(
            &pageSegmentListOffsets[0], &pageSegmentListOffsets[1], &pageSegmentListOffsets[2], &segmentListOffset);

        // Iterate over 3 PageSegments and 1 Segment.
        int pageSegmentCount = sizeof(pageSegmentListOffsets) / sizeof(size_t);
        for (int i = 0; i < pageSegmentCount; ++i)
        {
            RemoteDListIterator<PageSegment> iter(m_reader, this->GetFieldAddr<DListBase<PageSegment>>(pageSegmentListOffsets[i]));
            while (iter.Next())
            {
                if (this->IsAddressInSegment(address, iter.Current()))
                {
                    return true;
                }
            }
        }
        RemoteDListIterator<Segment> iter(m_reader, this->GetFieldAddr<DListBase<Segment>>(segmentListOffset));
        while (iter.Next())
        {
            if (this->IsAddressInSegment(address, iter.Current()))
            {
                return true;
            }
        }

        return false;
    }

    bool RemoteHeapPageAllocator::IsAddressInSegment(void* address, const PageSegment* segmentAddr)
    {
        RemotePageSegment segment(m_reader, segmentAddr);
        if (segment.IsInSegment(address))
        {
            return !segment.IsFreeOrDecommitted(address);
        }
        return false;
    }

    bool RemoteHeapPageAllocator::IsAddressInSegment(void* address, const Segment* segmentAddr)
    {
        RemoteSegment segment(m_reader, segmentAddr);
        return segment.IsInSegment(address);
    }

    //static
    void RemoteHeapPageAllocator::GetSegmentOffsets(size_t* segments, size_t* fullSegments, size_t* decommitSegments, size_t* largeSegments)
    {
        *segments = offsetof(PageAllocator, segments);
        *fullSegments = offsetof(PageAllocator, fullSegments);
        *decommitSegments = offsetof(PageAllocator, decommitSegments);
        *largeSegments = offsetof(PageAllocator, largeSegments);
    }

    InProcEmitBufferManagerWithlock* RemoteCodeGenAllocators::GetEmitBufferManager()
    {
        return this->GetFieldAddr<InProcEmitBufferManagerWithlock>(offsetof(InProcCodeGenAllocators, emitBufferManager));
    }

    bool RemotePreReservedVirtualAllocWrapper::IsPreReservedRegionPresent()
    {
        return this->GetPreReservedStartAddress() != nullptr;
    }

    LPVOID RemotePreReservedVirtualAllocWrapper::GetPreReservedStartAddress()
    {
        return this->ReadField<LPVOID>(offsetof(TargetType, preReservedStartAddress));
    }

    uint GetPreReservedRegionSize()
    {
        return (PreReservedVirtualAllocWrapper::PreReservedAllocationSegmentCount * AutoSystemInfo::Data.GetAllocationGranularityPageCount() * AutoSystemInfo::PageSize);
    }

    LPVOID  RemotePreReservedVirtualAllocWrapper::GetPreReservedEndAddress()
    {
        return (char*) GetPreReservedStartAddress() + GetPreReservedRegionSize();
    }

    LPVOID  RemotePreReservedVirtualAllocWrapper::GetPreReservedEndAddress(void *regionStart)
    {
        return (char*)regionStart + GetPreReservedRegionSize();
    }

    bool RemotePreReservedVirtualAllocWrapper::IsInRange(void * regionStart, void * address)
    {
        return regionStart && (address >= regionStart && address < GetPreReservedEndAddress(regionStart));
    }

    bool RemotePreReservedVirtualAllocWrapper::IsInRange(void * address)
    {
        if (this->GetRemoteAddr() == nullptr || !IsPreReservedRegionPresent())
        {
            return false;
        }

        return (address >= GetPreReservedStartAddress() && address < GetPreReservedEndAddress());
    }

    HeapPageAllocator<VirtualAllocWrapper> * RemoteCodePageAllocators::GetHeapPageAllocator()
    {
        return this->GetFieldAddr<HeapPageAllocator<VirtualAllocWrapper>>(offsetof(CustomHeap::InProcCodePageAllocators, pageAllocator));
    }

    // Check current script context and all contexts from its thread context.
    bool RemoteScriptContext::IsNativeAddress(void* address)
    {
        RemoteThreadContext threadContext(m_reader, this->ToTargetPtr()->threadContext);

        PreReservedVirtualAllocWrapper * preReservedVirtualAllocator = threadContext.GetPreReservedVirtualAllocator();
        intptr_t preReservedRegionAddr = threadContext.GetPreReservedRegionAddr();

        RemotePreReservedVirtualAllocWrapper preReservedVirtualAllocWrapper(m_reader, preReservedVirtualAllocator);

        if (preReservedRegionAddr != 0)
        {
            if (RemotePreReservedVirtualAllocWrapper::IsInRange((void*)preReservedRegionAddr, address))
            {
                return true;
            }
        }
        else if (preReservedVirtualAllocWrapper.IsPreReservedRegionPresent())
        {
            if (preReservedVirtualAllocWrapper.IsInRange(address))
            {
                return true;
            }
            else if (threadContext.IsAllJITCodeInPreReservedRegion())
            {
                return false;
            }
        }
        ScriptContext * scriptContext = threadContext.GetScriptContextList();
        
        bool isNativeAddr = false;
        
        while (scriptContext != nullptr && !isNativeAddr)
        {
            RemoteScriptContext scriptContextWrapper(m_reader, scriptContext);

            RemoteJITPageAddrToFuncRangeCacheWrapper jitPageAddrMapWrapper(this->m_reader, scriptContextWrapper.GetJitPageAddrMapWrapper());

            if (jitPageAddrMapWrapper.GetRemoteAddr() != nullptr)
            {
                AutoPtr<RemoteDictionary<JITPageAddrToFuncRangeCache::JITPageAddrToFuncRangeMap>> jitPageAddrToFuncRangeMap = nullptr;
                AutoPtr<RemoteDictionary<JITPageAddrToFuncRangeCache::LargeJITFuncAddrToSizeMap>> jitLargePageAddrMap = nullptr;

                if (jitPageAddrMapWrapper.GetJitPageAddrToFuncCountMap() != nullptr)
                {
                    jitPageAddrToFuncRangeMap = new(oomthrow) RemoteDictionary<JITPageAddrToFuncRangeCache::JITPageAddrToFuncRangeMap>(m_reader, jitPageAddrMapWrapper.GetJitPageAddrToFuncCountMap());
                }

                if (jitPageAddrMapWrapper.GetLargeJitFuncAddrToSizeMap() != nullptr)
                {
                    jitLargePageAddrMap = new(oomthrow) RemoteDictionary<JITPageAddrToFuncRangeCache::LargeJITFuncAddrToSizeMap>(m_reader, jitPageAddrMapWrapper.GetLargeJitFuncAddrToSizeMap());
                }

                void * pageAddr = (void*)((uintptr_t)address & ~(AutoSystemInfo::PageSize - 1));

                AutoPtr<RemoteDictionary<JITPageAddrToFuncRangeCache::RangeMap>> remoteRangeMap = nullptr;
                JITPageAddrToFuncRangeCache::RangeMap * rangeMap = nullptr;
                if (jitPageAddrToFuncRangeMap && jitPageAddrToFuncRangeMap->GetRemoteAddr() != nullptr && jitPageAddrToFuncRangeMap->TryGetValue(pageAddr, &rangeMap))
                {
                    remoteRangeMap = new(oomthrow) RemoteDictionary<JITPageAddrToFuncRangeCache::RangeMap>(jitPageAddrToFuncRangeMap->GetReader(), rangeMap);

                    if (remoteRangeMap->MapUntil(
                        [&](void* key, uint value) {
                        return (key <= address && (uintptr_t)address < ((uintptr_t)key + value));
                    }))
                    {
                        return true;
                    }
                }

                isNativeAddr = jitLargePageAddrMap && jitLargePageAddrMap->GetRemoteAddr() != nullptr && jitLargePageAddrMap->MapUntil(
                    [&](void* key, uint value) {
                    return (key <= address && (uintptr_t)address < ((uintptr_t)key + value));
                });
            }

            scriptContext = scriptContextWrapper.GetNextScriptContext();
        }

        return isNativeAddr;
    }

    JavascriptLibrary* RemoteScriptContext::GetLibrary() const
    {
        return this->ReadField<JavascriptLibrary*>(offsetof(ScriptContext, javascriptLibrary));
    }

    ThreadContext* RemoteScriptContext::GetThreadContext() const
    {
        return this->ReadField<ThreadContext*>(offsetof(ScriptContext, threadContext));
    }

    const PropertyRecord* RemoteScriptContext::GetPropertyName(Js::PropertyId propertyId) const
    {
        return RemoteThreadContext(m_reader, GetThreadContext()).GetPropertyName(propertyId);
    }

    ProbeContainer* RemoteScriptContext::GetProbeContainer() const
    {
        if (this->GetDebugContext() != nullptr)
        {
            RemoteDebugContext remoteDebugContext(m_reader, this->GetDebugContext());
            return remoteDebugContext.GetProbeContainer();
        }
        return nullptr;
    }

    FunctionBody::SourceInfo* RemoteFunctionBody::GetSourceInfo() const
    {
        return this->GetFieldAddr<FunctionBody::SourceInfo>(offsetof(FunctionBody, m_sourceInfo));
    }

    FunctionEntryPointInfo* RemoteFunctionBody::GetEntryPointFromNativeAddress(DWORD_PTR codeAddress)
    {
        FunctionEntryPointInfo* entryPoint = nullptr;

        this->MapEntryPoints([&entryPoint, &codeAddress, this](int index, FunctionEntryPointInfo* currentEntryPointAddr)
        {
            RemoteFunctionEntryPointInfo currentEntryPoint(m_reader, currentEntryPointAddr);
            if (currentEntryPoint.HasNativeAddress() &&
                currentEntryPoint.GetNativeAddress() <= codeAddress &&
                codeAddress < currentEntryPoint.GetNativeAddress() + currentEntryPoint.GetCodeSize())
            {
                entryPoint = currentEntryPointAddr;
            }
        });

        return entryPoint;
    }

    LoopEntryPointInfo* RemoteFunctionBody::GetLoopEntryPointFromNativeAddress(DWORD_PTR codeAddress, uint loopNum)
    {
        LoopHeader* loopHeaderAddr = this->GetLoopHeader(loopNum);
        Assert(loopHeaderAddr);

        LoopEntryPointInfo* entryPoint = nullptr;
        RemoteLoopHeader loopHeader(m_reader, loopHeaderAddr);
        loopHeader.MapEntryPoints([&](int index, LoopEntryPointInfo* currentEntryPointAddr)
        {
            RemoteLoopEntryPointInfo currentEntryPoint(m_reader, currentEntryPointAddr);
            if (currentEntryPoint.IsCodeGenDone() &&
                codeAddress >= currentEntryPoint.GetNativeAddress() &&
                codeAddress < currentEntryPoint.GetNativeAddress() + currentEntryPoint.GetCodeSize())
            {
                entryPoint = currentEntryPointAddr;
            }
        });

        return entryPoint;
    }

    FunctionEntryPointInfo* RemoteFunctionBody::GetEntryPointInfo(int index)
    {
        //typedef SynchronizableList<FunctionEntryPointWeakRef*, JsUtil::List<FunctionEntryPointWeakRef*>> FunctionEntryPointList;
        RemoteList<FunctionEntryPointWeakRef*> entryPoints(m_reader, this->ToTargetPtr()->entryPoints);
        RemoteRecyclerWeakReference<FunctionEntryPointInfo> weakEntryPoint(m_reader, entryPoints.Item(index));
        FunctionEntryPointInfo* entryPoint = weakEntryPoint.Get();
        Assert(entryPoint);
        return entryPoint;
    }

    uint32 RemoteFunctionBody::GetFrameHeight(FunctionEntryPointInfo* entryPointInfoAddr)
    {
        RemoteFunctionEntryPointInfo entryPointInfo(m_reader, entryPointInfoAddr);
        return entryPointInfo->frameHeight;
    }

    template <typename Fn>
    void RemoteFunctionBody::MapEntryPoints(Fn fn)
    {
        if (this->ToTargetPtr()->entryPoints)
        {
            //typedef SynchronizableList<FunctionEntryPointWeakRef*, JsUtil::List<FunctionEntryPointWeakRef*>> FunctionEntryPointList;
            RemoteList<FunctionEntryPointWeakRef*> entryPoints(m_reader, this->ToTargetPtr()->entryPoints);
            entryPoints.Map([&fn, this](int index, RecyclerWeakReference<FunctionEntryPointInfo>* weakEntryPointAddr)
            {
                RemoteRecyclerWeakReference<FunctionEntryPointInfo> weakEntryPoint(m_reader, weakEntryPointAddr);
                FunctionEntryPointInfo* strongRef = weakEntryPoint.Get();
                if (strongRef)
                {
                    fn(index, strongRef);
                }
            });
        }
    }

    LoopHeader* RemoteFunctionBody::GetLoopHeader(uint index)
    {
        auto loopHeaderArray = static_cast<const Js::LoopHeader*>(this->GetAuxPtrs(Js::FunctionProxy::AuxPointerType::LoopHeaderArray));
        Assert(loopHeaderArray != nullptr);
        Assert(index < this->GetCounter(FunctionBody::CounterFields::LoopCount));

        return const_cast<LoopHeader*>(loopHeaderArray) + index;
    }

    BOOL RemoteFunctionBody::GetMatchingStatementMapFromNativeOffset(
        StatementData* statementMap, DWORD_PTR codeAddress, uint loopNum, uint32 inlineeOffset, FunctionBody *inlinee /* = nullptr */)
    {
        AssertMsg(loopNum == LoopHeader::NoLoop || inlineeOffset == 0 && inlinee == nullptr,
            "Interpreter Jit loop body frame can't have inlinees.");
        return inlineeOffset != 0 ?
            this->GetMatchingStatementMapFromNativeOffset(statementMap, codeAddress, inlineeOffset, inlinee) :
            this->GetMatchingStatementMapFromNativeAddress(statementMap, codeAddress, loopNum, inlinee);
    }

    BOOL RemoteFunctionBody::GetMatchingStatementMapFromNativeAddress(
        StatementData* statementMap, DWORD_PTR codeAddress, uint loopNum, FunctionBody *inlinee /* = nullptr */)
    {
        SmallSpanSequence* spanSequence = nullptr;
        DWORD_PTR nativeBaseAddress = NULL;

        FunctionEntryPointInfo* entryPointAddr = this->GetEntryPointFromNativeAddress(codeAddress);
        if (entryPointAddr != nullptr)
        {
            RemoteFunctionEntryPointInfo entryPoint(m_reader, entryPointAddr);
            spanSequence = entryPoint.GetNativeThrowSpanSequence();
            nativeBaseAddress = entryPoint.GetNativeAddress();
        }
        else
        {
            LoopEntryPointInfo* entryPointAddr = this->GetLoopEntryPointFromNativeAddress(codeAddress, loopNum);
            if (entryPointAddr != nullptr)
            {
                RemoteLoopEntryPointInfo entryPoint(m_reader, entryPointAddr);
                spanSequence = entryPoint.GetNativeThrowSpanSequence();
                nativeBaseAddress = entryPoint.GetNativeAddress();
            }
        }

        int statementIndex = this->GetStatementIndexFromNativeAddress(spanSequence, codeAddress, nativeBaseAddress);
        return this->GetMatchingStatementMap(statementMap, statementIndex, inlinee);
    }

    BOOL RemoteFunctionBody::GetMatchingStatementMapFromNativeOffset(
        StatementData* statementMap, DWORD_PTR codeAddress, uint32 offset, FunctionBody *inlinee /* = nullptr */)
    {
        FunctionEntryPointInfo* entryPointAddr = this->GetEntryPointFromNativeAddress(codeAddress);

        SmallSpanSequence * spanSequence = nullptr;
        if (entryPointAddr != nullptr)
        {
            RemoteFunctionEntryPointInfo entryPoint(m_reader, entryPointAddr);
            spanSequence = entryPoint.GetNativeThrowSpanSequence();
        }

        int statementIndex = this->GetStatementIndexFromNativeOffset(spanSequence, offset);
        return this->GetMatchingStatementMap(statementMap, statementIndex, inlinee);
    }

    int RemoteFunctionBody::GetStatementIndexFromNativeAddress(
        SmallSpanSequence* throwSpanSequence, DWORD_PTR codeAddress, DWORD_PTR nativeBaseAddress)
    {
        uint32 nativeOffset = (uint32)(codeAddress - nativeBaseAddress);
        return this->GetStatementIndexFromNativeOffset(throwSpanSequence, nativeOffset);
    }

    int RemoteFunctionBody::GetStatementIndexFromNativeOffset(SmallSpanSequence* throwSpanSequenceAddr, uint32 nativeOffset)
    {
        int statementIndex = -1;
        if (throwSpanSequenceAddr)
        {
            RemoteSmallSpanSequence spanSequence(m_reader, throwSpanSequenceAddr);
            SmallSpanSequenceIter iter; // Note: the itetator contains simple data and is fine to use for both local and remote.
            StatementData statementData;
            if (spanSequence.GetMatchingStatementFromBytecode(nativeOffset, iter, statementData))
            {
                statementIndex = statementData.sourceBegin; // sourceBegin represents statementIndex here
            }
            else
            {
                // We've checked that codeAddress is within our range, so we consider it matches the last span.
                statementIndex = iter.accumulatedSourceBegin;
            }
        }

        return statementIndex;
    }

    void RemoteFunctionBody::FindClosestStatements(long characterOffset, StatementLocation *firstStatementLocation, StatementLocation *secondStatementLocation)
    {
        RemoteFunctionBody_SourceInfo remoteSourceInfo(this->m_reader, this);
        auto statementMaps = remoteSourceInfo.GetStatementMaps();

        if (statementMaps)
        {
            RemoteList<FunctionBody::StatementMap*> remoteStatementMaps(m_reader, statementMaps);
            for(int i = 0; i < remoteStatementMaps->Count(); i++)
            {
                RemoteFunctionBody_StatementMap statementMap(m_reader, remoteStatementMaps.Item(i));
                regex::Interval* pSourceSpan = &statementMap->sourceSpan;
                if (pSourceSpan->begin == 0 && pSourceSpan->end == 0)
                {
                    // Workaround for handling global return, which is a empty range.
                    // Ideal fix should be in bytecode generator, which should not emit the bytecode for global return.
                    // Once the fix done, this 'if' statement should be changed to an Assert
                    continue;
                }

                if (pSourceSpan->begin < characterOffset
                    && (firstStatementLocation->function == nullptr || firstStatementLocation->statement.begin < pSourceSpan->begin))
                {
                    firstStatementLocation->function = this->GetRemoteAddr();
                    firstStatementLocation->statement = *pSourceSpan;
                    firstStatementLocation->bytecodeSpan = statementMap->byteCodeSpan;
                }
                else if (pSourceSpan->begin >= characterOffset
                    && (secondStatementLocation->function == nullptr || secondStatementLocation->statement.begin > pSourceSpan->begin))
                {
                    secondStatementLocation->function = this->GetRemoteAddr();
                    secondStatementLocation->statement = *pSourceSpan;
                    secondStatementLocation->bytecodeSpan = statementMap->byteCodeSpan;
                }
            }
        }
    }

    BOOL RemoteFunctionBody::GetMatchingStatementMap(StatementData* data, int statementIndex, FunctionBody* inlineeAddr)
    {
        Assert(data);

        Js::FunctionBody::SourceInfo* siAddr = this->GetSourceInfo();
        RemoteFunctionBody inlinee(m_reader, inlineeAddr);
        if (inlineeAddr)
        {
            siAddr = inlinee.GetSourceInfo();
            Assert(siAddr);
        }

        if (statementIndex >= 0)
        {
            RemoteFunctionBody_SourceInfo si(m_reader, inlineeAddr ? &inlinee : this);
            SmallSpanSequence* spanSequenceAddr = si->pSpanSequence;
            if (spanSequenceAddr)
            {
                SmallSpanSequenceIter iter;
                RemoteSmallSpanSequence spanSequence(m_reader, spanSequenceAddr);
                spanSequence.ResetIterator(iter);

                if (spanSequence.Item(statementIndex, iter, *data))
                {
                    return TRUE;
                }
            }
            else
            {
                // typedef JsUtil::List<Js::FunctionBody::StatementMap*> StatementMapList;
                RemoteFunctionBody_SourceInfo sourceInfo(m_reader, this);
                Js::FunctionBody::StatementMapList* pStatementMaps = (Js::FunctionBody::StatementMapList*)this->GetAuxPtrs(FunctionProxy::AuxPointerType::StatementMaps);
                Assert(pStatementMaps);

                RemoteList<Js::FunctionBody::StatementMap*> statementMaps(m_reader, pStatementMaps);
                if (statementIndex >= statementMaps.Count())
                {
                    return FALSE;
                }

                RemoteFunctionBody_StatementMap map(m_reader, statementMaps.Item(statementIndex));
                data->sourceBegin = map->sourceSpan.begin;
                data->bytecodeBegin = map->byteCodeSpan.begin;
                return TRUE;
            }
        }

        return FALSE;
    }

    const uint32 RemoteFunctionBody::GetCounter(FunctionBody::CounterFields fieldEnum) const
    {

        // for registers, it's using UINT32_MAX to represent NoRegister
        if ((fieldEnum == FunctionBody::CounterFields::LocalClosureRegister && !this->ToTargetPtr()->m_hasLocalClosureRegister)
            || (fieldEnum == FunctionBody::CounterFields::LocalFrameDisplayRegister && !this->ToTargetPtr()->m_hasLocalFrameDisplayRegister)
            || (fieldEnum == FunctionBody::CounterFields::EnvRegister && !this->ToTargetPtr()->m_hasEnvRegister)
            || (fieldEnum == FunctionBody::CounterFields::ThisRegisterForEventHandler && !this->ToTargetPtr()->m_hasThisRegisterForEventHandler)
            || (fieldEnum == FunctionBody::CounterFields::FirstInnerScopeRegister && !this->ToTargetPtr()->m_hasFirstInnerScopeRegister)
            || (fieldEnum == FunctionBody::CounterFields::FuncExprScopeRegister && !this->ToTargetPtr()->m_hasFuncExprScopeRegister)
            || (fieldEnum == FunctionBody::CounterFields::FirstTmpRegister && !this->ToTargetPtr()->m_hasFirstTmpRegister)
            )
        {
            return Constants::NoRegister;
        }

        uint8 fieldEnumVal = static_cast<uint8>(fieldEnum);
        const auto& remoteCounters = this->ToTargetPtr()->counters;
        uint8 fieldSize = remoteCounters.fieldSize;

        if (fieldSize == 1)
        {
            return ReadVirtual<uint8>(&remoteCounters.fields.ptr->u8Fields[fieldEnumVal]);
        }
        else if (fieldSize == 2)
        {
            return ReadVirtual<uint16>(&remoteCounters.fields.ptr->u16Fields[fieldEnumVal]);
        }
        else if (fieldSize == 4)
        {
            return ReadVirtual<uint32>(&remoteCounters.fields.ptr->u32Fields[fieldEnumVal]);
        }
        else
        {
            Assert(false);
            return 0;
        }
    }
    const void* RemoteFunctionBody::GetAuxPtrs(FunctionProxy::AuxPointerType pointerType) const
    {
        void* auxPtrsRaw = static_cast<void*>(this->ToTargetPtr()->auxPtrs);
        uint8* auxPtrs = static_cast<uint8*>(auxPtrsRaw);
        uint8 pointerTypeValue = static_cast<uint8>(pointerType);
        if (auxPtrs != nullptr)
        {
            uint8 count = ReadVirtual<uint8>(auxPtrs);
            if (count == FunctionProxy::AuxPtrsT::AuxPtrs16::MaxCount)
            {
                for (uint8 i = 0; i < count; i++)
                {
                    uint8 type = ReadVirtual<uint8>(auxPtrs + offsetof(FunctionProxy::AuxPtrsT::AuxPtrs16, type) + i);
                    if (type == pointerTypeValue)
                    {
                        return ReadVirtual<void*>(auxPtrs + offsetof(FunctionProxy::AuxPtrsT::AuxPtrs16, ptr) + sizeof(void*) * i);
                    }
                }
            }
            else if (count == FunctionProxy::AuxPtrsT::AuxPtrs32::MaxCount)
            {
                for (uint8 i = 0; i < count; i++)
                {
                    uint8 type = ReadVirtual<uint8>(auxPtrs + offsetof(FunctionProxy::AuxPtrsT::AuxPtrs32, type) + i);
                    if (type == pointerTypeValue)
                    {
                        return ReadVirtual<void*>(auxPtrs + offsetof(FunctionProxy::AuxPtrsT::AuxPtrs32, ptr) + sizeof(void*) * i);
                    }
                }
            }
            else if (count > FunctionProxy::AuxPtrsT::AuxPtrs32::MaxCount)
            {
                uint8 offset = ReadVirtual<uint8>(auxPtrs + offsetof(FunctionProxy::AuxPtrsT, offsets) + pointerTypeValue);
                if (offset != (uint8)FunctionProxy::AuxPointerType::Invalid)
                {
                    return ReadVirtual<void*>(auxPtrs + offsetof(FunctionProxy::AuxPtrsT, ptrs) + sizeof(void*) * offset);
                }
            }
        }
        return nullptr;
    }

    uint32 RemoteGrowingUint32HeapArray::ItemInBuffer(uint32 index)
    {
        if (index >= this->ToTargetPtr()->Count())
        {
            return 0;
        }

        // Read index's item into RemoteData and return the value read.
        RemoteData<uint32> item(m_reader, this->ToTargetPtr()->buffer + index);
        return *static_cast<uint32*>(item);
    }

    uint32 RemoteSmallSpanSequence::Count()
    {
        GrowingUint32HeapArray* statementBufferAddr = this->ToTargetPtr()->pStatementBuffer;
        if (statementBufferAddr)
        {
            return RemoteGrowingUint32HeapArray(m_reader, statementBufferAddr)->count;
        }
        return 0;
    }

    BOOL RemoteSmallSpanSequence::Item(int index, SmallSpanSequenceIter& iter, StatementData& data)
    {
        GrowingUint32HeapArray* statementBufferAddr = this->ToTargetPtr()->pStatementBuffer;
        if (!statementBufferAddr || index < 0)
        {
            return FALSE;
        }

        RemoteGrowingUint32HeapArray statementBuffer(m_reader, statementBufferAddr);
        if ((uint32)index >= statementBuffer->Count())
        {
            return FALSE;
        }

        if (iter.accumulatedIndex <= 0 || iter.accumulatedIndex > index)
        {
            this->ResetIterator(iter);
        }

        while (iter.accumulatedIndex <= index)
        {
            Assert((uint32)iter.accumulatedIndex < statementBuffer->Count());

            int countOfMissed = 0;
            if (!this->GetRangeAt(iter.accumulatedIndex, iter, &countOfMissed, data))
            {
                Assert(FALSE);
                break;
            }

            // We store the next index
            iter.accumulatedSourceBegin = data.sourceBegin;
            iter.accumulatedBytecodeBegin = data.bytecodeBegin;

            iter.accumulatedIndex++;

            if (countOfMissed)
            {
                iter.indexOfActualOffset += countOfMissed;
            }

            if ((iter.accumulatedIndex - 1) == index)
            {
                return TRUE;
            }
        }

        return FALSE;
    }

    void RemoteSmallSpanSequence::ResetIterator(SmallSpanSequenceIter &iter)
    {
        iter.accumulatedIndex = 0;
        iter.accumulatedSourceBegin = this->ToTargetPtr()->baseValue;
        iter.accumulatedBytecodeBegin = 0;
        iter.indexOfActualOffset = 0;
    }

    template <typename TFilterFn>
    bool RemoteSmallSpanSequence::GetMatchingStatement(SmallSpanSequenceIter& iter, TFilterFn filterFn, StatementData& data)
    {
        while ((uint32)iter.accumulatedIndex < this->Count())
        {
            int countOfMissed = 0;
            if (!this->GetRangeAt(iter.accumulatedIndex, iter, &countOfMissed, data))
            {
                Assert(FALSE);
                break;
            }

            if (filterFn(iter, data))
            {
                return true;
            }

            // Look for the next
            iter.accumulatedSourceBegin = data.sourceBegin;
            iter.accumulatedBytecodeBegin = data.bytecodeBegin;
            iter.accumulatedIndex++;

            if (countOfMissed)
            {
                iter.indexOfActualOffset += countOfMissed;
            }
        }

        return false;
    }

    BOOL RemoteSmallSpanSequence::GetMatchingStatementFromBytecode(int bytecode, SmallSpanSequenceIter& iter, StatementData& data)
    {
        if (this->Count() > 0 && bytecode >= 0)
        {
            // Support only in forward direction
            if (bytecode < iter.accumulatedBytecodeBegin
                || iter.accumulatedIndex <= 0 || (uint32)iter.accumulatedIndex >= this->Count())
            {
                // Re-initialize the accumulator.
                this->ResetIterator(iter);
            }

            auto filterFn = [bytecode](SmallSpanSequenceIter& iter, StatementData& data)
            {
                if (data.bytecodeBegin >= bytecode)
                {
                    if (data.bytecodeBegin > bytecode)
                    {
                        // Not exactly at the current bytecode, so it falls in between previous statement.
                        data.sourceBegin = iter.accumulatedSourceBegin;
                        data.bytecodeBegin = iter.accumulatedBytecodeBegin;
                    }
                    return TRUE;
                }
                return FALSE;
            };

            if (this->GetMatchingStatement(iter, filterFn, data))
            {
                return TRUE;
            }
        }

        // Failed to give the correct one, init to default
        iter.accumulatedIndex = -1;
        return FALSE;
    }

    // Get Values of the begining of the statement at particular index.
    BOOL RemoteSmallSpanSequence::GetRangeAt(int index, SmallSpanSequenceIter& iter, int* pCountOfMissed, StatementData& data)
    {
        Assert((uint32)index < this->Count());
        RemoteGrowingUint32HeapArray statementBuffer(m_reader, this->ToTargetPtr()->pStatementBuffer);
        SmallSpan span(statementBuffer.ItemInBuffer(index));  // Note: SmallSpan is fine to use as a data class.
        int countOfMissed = 0;

        if ((short)span.sourceBegin == SHRT_MAX)
        {
            // Look in ActualOffset store
            Assert(this->ToTargetPtr()->pActualOffsetList);

            RemoteGrowingUint32HeapArray actualOffsetList(m_reader, this->ToTargetPtr()->pActualOffsetList);
            Assert(actualOffsetList->Count() > 0);
            Assert(actualOffsetList->Count() > (uint32)iter.indexOfActualOffset);

            data.sourceBegin = actualOffsetList.ItemInBuffer(iter.indexOfActualOffset);
            countOfMissed++;
        }
        else
        {
            data.sourceBegin = iter.accumulatedSourceBegin + (short)span.sourceBegin;
        }

        if (span.bytecodeBegin == SHRT_MAX)
        {
            // Look in ActualOffset store
            Assert(this->ToTargetPtr()->pActualOffsetList);

            RemoteGrowingUint32HeapArray actualOffsetList(m_reader, this->ToTargetPtr()->pActualOffsetList);
            Assert(actualOffsetList->Count() > 0);
            Assert(actualOffsetList->Count() > (uint32)(iter.indexOfActualOffset + countOfMissed));

            data.bytecodeBegin = actualOffsetList.ItemInBuffer(iter.indexOfActualOffset + countOfMissed);
            countOfMissed++;
        }
        else
        {
            data.bytecodeBegin = iter.accumulatedBytecodeBegin + span.bytecodeBegin;
        }

        if (pCountOfMissed)
        {
            *pCountOfMissed = countOfMissed;
        }

        return TRUE;
    }

    bool RemoteSourceContextInfo::IsDynamic() const
    {
        return this->ToTargetPtr()->dwHostSourceContext == Js::Constants::NoHostSourceContext || this->ToTargetPtr()->isHostDynamicDocument;
    }

    // Adjust document offset (normally for start/end of statement) for scenarios when JS adds scriptlet code,
    // such as for event handlers/onclick, etc.
    // Returns offset in unmodified host buffer.
    // See also: ScriptDebugDocument::GetDocumentContext.
    ULONG RemoteSRCINFO::ConvertInternalOffsetToHost(ULONG charOffsetInScriptBuffer) const
    {
        ULONG charOffsetInHostBuffer = charOffsetInScriptBuffer - this->ToTargetPtr()->ichMinHost;
        return charOffsetInHostBuffer;
    }

    int RemoteLineOffsetCache::GetLineForCharacterOffset(charcount_t characterOffset, charcount_t *outLineCharOffset, charcount_t *outByteOffset)
    {
        const LineOffsetCacheReadOnlyList* lineOffsetCacheList = this->GetLineOffsetCacheList();
        RemoteList<LineOffsetCacheItem, LineOffsetCacheReadOnlyList> remoteLineOffsetCacheList(m_reader, lineOffsetCacheList);

        Assert(remoteLineOffsetCacheList.Count() > 0);

        // The list is sorted, so binary search to find the line info.
        int closestIndex = -1;
        int minRange = INT_MAX;

        int resultIndex = remoteLineOffsetCacheList.BinarySearch([&](const LineOffsetCacheItem& item, int index)
        {

            int offsetRange = characterOffset - item.characterOffset;
            if (offsetRange >= 0)
            {
                if (offsetRange < minRange)
                {
                    // There are potentially many lines with starting offsets greater than the one we're searching
                    // for.  As a result, we should track which index we've encountered so far that is the closest
                    // to the offset we're looking for without going under.  This will find the line that contains
                    // the offset.
                    closestIndex = index;
                    minRange = offsetRange;
                }

                // Search lower to see if we can find a closer index.
                return -1;
            }
            else
            {
                // Search higher to get into a range that is greater than the offset.
                return 1;
            }

            // Note that we purposely don't return 0 (==) here.  We want the search to end in failure (-1) because
            // we're searching for the closest element, not necessarily an exact element offset.  Exact offsets
            // are possible when the offset we're searching for is the first character of the line, but that will
            // be handled by the if statement above.
        });

        if (closestIndex >= 0)
        {
            LineOffsetCacheItem lastItem = remoteLineOffsetCacheList.Item(closestIndex);

            if (outLineCharOffset != nullptr)
            {
                *outLineCharOffset = lastItem.characterOffset;
            }

            if (outByteOffset != nullptr)
            {
                *outByteOffset = lastItem.byteOffset;
            }
        }

        return closestIndex;
    }

    const LineOffsetCacheReadOnlyList* RemoteLineOffsetCache::GetLineOffsetCacheList()
    {
        return this->ToTargetPtr()->lineOffsetCacheList;
    }

    RemoteFunctionBody_SourceInfo::RemoteFunctionBody_SourceInfo(IVirtualReader* reader, const RemoteFunctionBody* functionBody) :
        RemoteData<TargetType>(reader, functionBody->GetSourceInfo()),
        m_functionBody(functionBody)
    {}

    const FunctionBody* RemoteFunctionBody_SourceInfo::GetFunctionBody()
    {
        return m_functionBody->operator const Js::FunctionBody *();
    }

    // Get char offsets for statement start and end.
    void RemoteFunctionBody_SourceInfo::GetStatementOffsets(int byteCodeOffset, ULONG* startOffset, ULONG* endOffset)
    {
        *startOffset = this->GetInternalOffsetForStatementStart(byteCodeOffset);
        if (!this->GetInternalOffsetForStatementEnd(*startOffset, endOffset))
        {
            *endOffset = *startOffset;    // set to same source offset, the best we can do.
        }

        RemoteSRCINFO hostSrcInfo(m_reader, this->GetHostSrcInfo());
        *startOffset = hostSrcInfo.ConvertInternalOffsetToHost(*startOffset);
        *endOffset = hostSrcInfo.ConvertInternalOffsetToHost(*endOffset);
    }

    // The function determines the char offset for a bytecode offset within the current script buffer
    // (in order to convert it to actual offset, use ConvertInternalOffsetToHost).
    ULONG RemoteFunctionBody_SourceInfo::GetInternalOffsetForStatementStart(int byteCodeOffset)
    {
        int startCharOfStatement = this->GetFunctionBody()->m_cchStartOffset; // Default to the start of this function

        if (this->ToTargetPtr()->pSpanSequence)
        {
            SmallSpanSequenceIter iter;
            RemoteSmallSpanSequence spanSequence(m_reader, this->ToTargetPtr()->pSpanSequence);
            spanSequence.ResetIterator(iter);

            StatementData data;
            if (spanSequence.GetMatchingStatementFromBytecode(byteCodeOffset, iter, data)
                && this->EndsAfter(data.sourceBegin))
            {
                startCharOfStatement = data.sourceBegin;
            }
        }
        else
        {
            FunctionBody::StatementMap* mapAddr = this->GetEnclosingStatementMapFromByteCode(byteCodeOffset);
            RemoteFunctionBody_StatementMap map(m_reader, mapAddr);
            if (map && this->EndsAfter(map->sourceSpan.begin))
            {
                startCharOfStatement = map->sourceSpan.begin;
            }
        }

        return startCharOfStatement;
    }

    // Returns source offset for corresponding end of statement, or current offset if cannot find it.
    // The return value is offset within the current script buffer
    // (in order to convert it to actual offset, use ConvertInternalOffsetToHost).
    bool RemoteFunctionBody_SourceInfo::GetInternalOffsetForStatementEnd(ULONG statementStartSourceOffset, ULONG* statementEndSourceOffset)
    {
        Assert(statementEndSourceOffset);

        if (this->ToTargetPtr()->pSpanSequence)
        {
            // Find the span that corresponds to next statement and subtract 1 from its sourceBegin.
            SmallSpanSequenceIter iter;
            RemoteSmallSpanSequence spanSequence(m_reader, this->ToTargetPtr()->pSpanSequence);
            spanSequence.ResetIterator(iter);

            auto filterFn = [statementStartSourceOffset](SmallSpanSequenceIter& iter, StatementData& data)
            {
                return data.sourceBegin >= 0 &&
                       static_cast<ULONG>(data.sourceBegin) > statementStartSourceOffset;
            };

            StatementData data;
            if (spanSequence.GetMatchingStatement(iter, filterFn, data))
            {
                int prevStatementEnd = data.sourceBegin - 1;
                *statementEndSourceOffset = prevStatementEnd >= 0 && static_cast<ULONG>(prevStatementEnd) > statementStartSourceOffset ?
                    prevStatementEnd :
                    statementStartSourceOffset;
                return true;
            }
        }
        else
        {
            FunctionBody::StatementMapList* mapsAddr = this->GetStatementMaps();
            Assert(mapsAddr);
            if (mapsAddr)
            {
                // Find statement map that corresponds to statement with this source.
                // Well, not precise as there could be newlines and closing brackets in between, but that seems to be the best we can.
                RemoteList<Js::FunctionBody::StatementMap*> maps(m_reader, mapsAddr);
                for (int i = 0; i < maps.Count(); ++i)
                {
                    FunctionBody::StatementMap* mapAddr = maps.Item(i);
                    Assert(mapAddr);

                    RemoteFunctionBody_StatementMap map(m_reader, mapAddr);
                    RemoteInterval span(m_reader, map.GetFieldAddr<regex::Interval>(offsetof(FunctionBody::StatementMap, sourceSpan)));
                    if (span.Includes(statementStartSourceOffset))
                    {
                        *statementEndSourceOffset = span->end;
                        return true;
                    }
                }
            }
        }
        return false;
    }

    // Returns the StatementMap for the offset.
    // 1. Current statementMap if bytecodeoffset falls within bytecode's span
    // 2. Previous if the bytecodeoffset is in between previous's end to current's begin

    FunctionBody::StatementMap* RemoteFunctionBody_SourceInfo::GetEnclosingStatementMapFromByteCode(int byteCodeOffset)
    {
        int index = GetEnclosingStatementIndexFromByteCode(byteCodeOffset);
        if (index != -1)
        {
            RemoteList<FunctionBody::StatementMap*> statementMaps(m_reader, this->GetStatementMaps());
            return statementMaps.Item(index);
        }
        return nullptr;
    }

    // Returns the index of StatementMap for
    // 1. Current statementMap if bytecodeoffset falls within bytecode's span
    // 2. Previous if the bytecodeoffset is in between previous's end to current's begin
    // 3. -1 of the failures.

    int RemoteFunctionBody_SourceInfo::GetEnclosingStatementIndexFromByteCode(int byteCodeOffset)
    {
        FunctionBody::StatementMapList* statementMapsAddr = this->GetStatementMaps();
        if (statementMapsAddr == nullptr)
        {
            // eg. internal library.
            return -1;
        }

        Assert(this->ToTargetPtr()->pSpanSequence == nullptr);

        RemoteList<FunctionBody::StatementMap*> statementMaps(m_reader, statementMapsAddr);

        for (int index = 0; index < statementMaps.Count(); index++)
        {
            FunctionBody::StatementMap* mapAddr = statementMaps.Item(index);
            RemoteFunctionBody_StatementMap map(m_reader, mapAddr);
            FunctionBody::StatementMap* stMap = map;
            RemoteInterval byteCodeSpan(m_reader,
                map.GetFieldAddr<regex::Interval>(offsetof(FunctionBody::StatementMap, byteCodeSpan)));

            if (byteCodeSpan.Includes(byteCodeOffset))
            {
                return index;
            }
            else if (!stMap->isSubexpression && byteCodeOffset < byteCodeSpan->begin)
            {
                return index > 0 ? index - 1 : 0;
            }
        }

        return statementMaps.Count() - 1;
    }

    // For given bytecode gets offsets of row and column in (host buffer corresponsing to) the actual user file.
    bool RemoteFunctionBody_SourceInfo::GetLineCharOffset(int byteCodeOffset, ULONG* line, LONG* colOffset)
    {
        ULONG startOfStatement = this->GetInternalOffsetForStatementStart(byteCodeOffset);
        return this->GetLineCharOffsetFromStartChar(startOfStatement, line, colOffset);
    }

    bool RemoteFunctionBody_SourceInfo::GetLineCharOffsetFromStartChar(charcount_t startCharOfStatement, ULONG* _line, LONG* _colOffset)
    {
        // First check if we have a pre-built cache (in the case of a WWA app, we will have it deserialized fromt the source code).
        LineOffsetCache* lineOffsetCache = this->GetLineOffsetCache();

        if (lineOffsetCache != nullptr)
        {
            RemoteLineOffsetCache remoteLineOffsetCache(m_reader, lineOffsetCache);

            charcount_t ignore = 0;
            charcount_t lineOffset = 0;
            int result = remoteLineOffsetCache.GetLineForCharacterOffset(startCharOfStatement, &lineOffset, &ignore /* byteOffset */);
            if (result == -1)
            {
                return false;
            }
            else
            {
                *_colOffset = (LONG)(startCharOfStatement - lineOffset);
                *_line = result;
                return true;
            }
        }

        // The following adjusts for where the script is within the document
        ULONG line = this->GetHostStartLine();
        LONG charOffset = 0;
        LPCUTF8 allSourceAddr = this->GetStartOfDocument();    // Note: this is in target process address space.
        int extraLines = 0;
        LONG lastNewLine = -1;
        // TODO: What about surrogate pairs?
        utf8::DecodeOptions options = utf8::doAllowThreeByteSurrogates;

        // TODO: it seem that we can make this more efficient by reading from the start of the function as opposed to start of the document.
        //       We should know the line number at the start of the function and we should be able to add the offset of the statement to that.
        //       It seems that product code can do the same it just does not currently.
        // Count newlines from start of document
        // Note: m_cbLength is length in bytes for script, and we need from doc start.
        RemoteUtf8SourceInfo funcUtf8SourceInfo(m_reader, this->GetFunctionBody()->m_utf8SourceInfo);
        const size_t bufByteCount = funcUtf8SourceInfo.GetDebugModeSourceLength() + (BYTE*)funcUtf8SourceInfo.GetDebugModeSource() - (BYTE*)allSourceAddr;
        const int cacheSize = 32;
        RemoteBuffer<utf8char_t> allSource(m_reader, const_cast<LPUTF8>(allSourceAddr), bufByteCount, cacheSize);

        // skip any byte order mark (assuming 4 bytes at a minimum)
        const ULONG availableBytes = allSource.EnsurePtr(4 * sizeof(CUTF8)); // Make sure that we have enough bytes copied locally.
        LPCUTF8 tmpPtrEnd = allSource.Ptr + availableBytes;
        const size_t byteOrderMarkCharOffset = FunctionBody::SkipByteOrderMark(allSource.Ptr);
        if (allSource.Ptr > tmpPtrEnd)
        {
            DiagException::Throw(E_UNEXPECTED, DiagErrorCode::RUNTIME_READ_PAST_SOURCE); // Read past available bytes
        }

        LONG cbCol = 0;

        for(size_t offsetCh = byteOrderMarkCharOffset; offsetCh < (size_t)startCharOfStatement;)
        {
            const ULONG availableBytes = allSource.EnsurePtr(4 * sizeof(CUTF8)); // Make sure that we have enough bytes copied locally.
            char16 ch = utf8::Decode(allSource.Ptr, allSource.Ptr + availableBytes, options);

            switch (ch)
            {
            case _u('\r'):
                if ((offsetCh + 1) < (size_t)startCharOfStatement)
                {
                    allSource.EnsurePtr(sizeof(CUTF8));
                    if (*allSource.Ptr == '\n')
                    {
                        // Consume the next char also
                        offsetCh++;
                        allSource.Ptr++;
                    }
                }
                // Falls-through
            case _u('\n'):
                extraLines++;
                lastNewLine = offsetCh;
                break;
            }

            // update offset
            offsetCh++;
        }

        line += extraLines;

        if (this->GetFunctionBody()->m_isDynamicFunction)
        {
            RemoteSourceContextInfo sourceContextInfo(m_reader, this->GetSourceContextInfo());
            if (sourceContextInfo.IsDynamic())
            {
                line -= JavascriptFunction::numberLinesPrependedToAnonymousFunction;
            }
        }

        if(_line)
        {
            *_line = line;
        }

        if(_colOffset)
        {
            *_colOffset = startCharOfStatement - lastNewLine - 1;

            // if we are on the 1st line, account for the byte order mark character offset
            if (lastNewLine == -1)
            {
                *_colOffset -= byteOrderMarkCharOffset;
            }
        }

        return true;
    }

    LineOffsetCache* RemoteFunctionBody_SourceInfo::GetLineOffsetCache()
    {
        return RemoteUtf8SourceInfo(m_reader, this->GetFunctionBody()->m_utf8SourceInfo).ToTargetPtr()->m_lineOffsetCache;
    }

    Js::FunctionBody::StatementMapList* RemoteFunctionBody_SourceInfo::GetStatementMaps()
    {
        return (Js::FunctionBody::StatementMapList*)m_functionBody->GetAuxPtrs(FunctionProxy::AuxPointerType::StatementMaps);
    }

    SourceContextInfo* RemoteFunctionBody_SourceInfo::GetSourceContextInfo()
    {
        RemoteSRCINFO srcInfo(m_reader, this->GetHostSrcInfo());
        return srcInfo->sourceContextInfo;
    }

    const SRCINFO* RemoteFunctionBody_SourceInfo::GetHostSrcInfo()
    {
        RemoteUtf8SourceInfo utf8SourceInfo(m_reader, this->GetFunctionBody()->m_utf8SourceInfo);
        return utf8SourceInfo->m_srcInfo;
    }

    // Returns the start line for the script buffer (code buffer for the entire script tag) of this current function.
    // We subtract the lnMinHost because it is the number of lines we have added to augment scriplets passed through
    // ParseProcedureText to have a function name.
    ULONG RemoteFunctionBody_SourceInfo::GetHostStartLine()
    {
        RemoteSRCINFO srcInfo(m_reader, this->GetHostSrcInfo());
        return srcInfo->dlnHost - srcInfo->lnMinHost;
    }

    LPCUTF8 RemoteFunctionBody_SourceInfo::GetStartOfDocument()
    {
        RemoteUtf8SourceInfo funcUtf8SourceInfo(m_reader, this->GetFunctionBody()->m_utf8SourceInfo);
        return funcUtf8SourceInfo.GetDebugModeSource();
    }

    size_t RemoteFunctionBody_SourceInfo::StartOffset()
    {
        return GetFunctionBody()->m_cbStartOffset;
    }


    size_t RemoteFunctionBody_SourceInfo::LengthInBytes()
    {
        return GetFunctionBody()->m_cbLength;
    }

    // Determine if the end of this SourceInfo lies after the given source buffer offset.
    bool RemoteFunctionBody_SourceInfo::EndsAfter(size_t sourceBufferOffset)
    {
        return sourceBufferOffset < this->StartOffset() + this->LengthInBytes();
    }

    //
    // Same algorithm as in FunctionBody to retrieve a function body's source name (url).
    //
    LPCWSTR RemoteFunctionBody_SourceInfo::GetSourceName(const RemoteSourceContextInfo* remoteSourceContextInfo)
    {
        Assert(remoteSourceContextInfo);

        if (remoteSourceContextInfo->IsDynamic())
        {
            // TODO: consider evaluating this during function body creation and saving to sourceContextInfo->Url.
            // This will ensure we do not need to duplicate this logic with the runtime.
            // None of the logic seems to change over the lifetime of a function and it makes sense
            // to be set once at the time of FunctionBody initialization.
            if (this->GetFunctionBody()->m_isEval)
            {
                return Constants::EvalCode;
            }
            else if (this->GetFunctionBody()->m_isDynamicFunction)
            {
                return Constants::FunctionCode;
            }
            else
            {
                return Constants::UnknownScriptCode;
            }
        }
        else
        {
            return remoteSourceContextInfo->ToTargetPtr()->url;
        }
    }

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    void RemoteConfiguration::EnsureInitialize(IVirtualReader* reader, const Configuration* addr)
    {
        if(!m_isInitialized)
        {
            ULONG bytesRead;
            HRESULT hr = reader->ReadVirtual(addr, &m_data, _countof(m_data), &bytesRead);
            CheckHR(hr, DiagErrorCode::READ_VIRTUAL);
            if(bytesRead != _countof(m_data))
            {
                DiagException::Throw(E_UNEXPECTED, DiagErrorCode::READ_VIRTUAL_MISMATCH);
            }
            m_isInitialized = true;
        }
    }

    //static
    RemoteConfiguration* RemoteConfiguration::GetInstance()
    {
        Assert(s_instance.m_isInitialized);
        return &s_instance;
    }

    void RemoteConfiguration::EnsureInstance(IVirtualReader* reader, const Configuration* addr)
    {
        s_instance.EnsureInitialize(reader, addr);
    }

    bool RemoteConfiguration::PhaseOff1(Phase phase)
    {
        return ((Configuration*)this->m_data)->flags.Off.phaseList[(int)phase].valid;
    }

    bool RemoteConfiguration::PhaseOn1(Phase phase)
    {
        return ((Configuration*)this->m_data)->flags.On.phaseList[(int)phase].valid;
    }
#endif

    void RemoteFunctionBody::GetFunctionName(_Out_writes_z_(nameBufferElementCount) LPWSTR nameBuffer, ULONG nameBufferElementCount) const
    {
        nameBuffer[0] = 0;
        LPCWSTR displayNamePtr = (*this)->m_displayName;

        LPCWSTR displayName = nameBuffer;
        LPCWSTR name = displayName; // Default to displayName
        HRESULT hr = m_reader->ReadString(displayNamePtr, nameBuffer, nameBufferElementCount);
        CheckHR(hr, DiagErrorCode::READ_STRING);

        this->GetFunctionBodyInfo([this, &name, displayName](
            RemoteFunctionBody_SourceInfo& sourceInfo, RemoteSourceContextInfo& sourceContextInfo, bool isDynamicScript)
        {
            // Get function name, same logic as FunctionBody::GetExternalDisplayName
            bool isGlobalFunc = static_cast<const Js::FunctionBody*>(*this)->m_isGlobalFunc;
            name = this->GetExternalDisplayName(displayName, isDynamicScript, isGlobalFunc);
        });

        if (name != displayName)
        {
            wcscpy_s(nameBuffer, nameBufferElementCount, name);
        }
    }

    void RemoteFunctionBody::GetUri(_Out_writes_z_(urlBufferElementCount) LPWSTR urlBuffer, ULONG urlBufferElementCount) const
    {
        urlBuffer[0] = 0;
        this->GetFunctionBodyInfo([&](RemoteFunctionBody_SourceInfo& sourceInfo, RemoteSourceContextInfo& sourceContextInfo, bool isDynamicScript)
        {
            // Get source file/URI name
            LPCWSTR sourceName = sourceInfo.GetSourceName(&sourceContextInfo);
            if (isDynamicScript) // local constant string
            {
                wcscpy_s(urlBuffer, urlBufferElementCount, sourceName);
            }
            else if (sourceName != nullptr) // remote url
            {
                HRESULT hr = m_reader->ReadString(sourceName, urlBuffer, urlBufferElementCount);
                CheckHR(hr, DiagErrorCode::READ_STRING);
            }
        });
    }

    // Template method used to get both uri and function name.
    template <typename Fn>
    void RemoteFunctionBody::GetFunctionBodyInfo(Fn fn) const
    {
        RemoteFunctionBody_SourceInfo sourceInfo(m_reader, this);
        RemoteUtf8SourceInfo utf8SourceInfo(m_reader, (*this)->GetUtf8SourceInfo());
        const SRCINFO* hostSrcInfoAddr = utf8SourceInfo->m_srcInfo;

        if (hostSrcInfoAddr)
        {
            RemoteSRCINFO hostSrcInfo(m_reader, hostSrcInfoAddr);
            if (hostSrcInfo->sourceContextInfo)
            {
                RemoteSourceContextInfo sourceContextInfo(m_reader, hostSrcInfo->sourceContextInfo);
                bool isDynamicScript = sourceContextInfo->IsDynamic();
                fn(sourceInfo, sourceContextInfo, isDynamicScript);
            }
        }
    }

    const char16* RemoteFunctionBody::GetExternalDisplayName(
        const char16* displayName, BOOL isDynamicScript, BOOL isGlobalFunc) const
    {
        GetFunctionBodyNameData funcBody(*this, displayName, isDynamicScript, isGlobalFunc);
        return FunctionBody::GetExternalDisplayName(&funcBody);
    }

    Utf8SourceInfo* RemoteFunctionBody::GetUtf8SourceInfo() const
    {
        return ReadField<Utf8SourceInfo*>(offsetof(FunctionBody, m_utf8SourceInfo));
    }

    //
    // Get and cache {byteCodeOffset -> row/column} info. Source row/column lookup is very expensive.
    //
    void RemoteFunctionBody::GetRowColumn(int byteCodeOffset, ULONG* pRow, ULONG* pColumn)
    {
        RowColumn rowColumn;
        if (!m_rowColumnMap.Lookup(byteCodeOffset, rowColumn))
        {
            RemoteUtf8SourceInfo utf8SourceInfo(this->GetReader(), this->ToTargetPtr()->GetUtf8SourceInfo());
            if (utf8SourceInfo->GetIsLibraryCode())
            {
                rowColumn.row = 0;
                rowColumn.column = 0;
            }
            else
            {
                RemoteFunctionBody_SourceInfo sourceInfo(this->GetReader(), this);
                sourceInfo.GetLineCharOffset(byteCodeOffset, &rowColumn.row, &rowColumn.column);
            }

            m_rowColumnMap[byteCodeOffset] = rowColumn;
        }

        *pRow = rowColumn.row;
        *pColumn = rowColumn.column;
    }

    bool RemoteFunctionBody::GetNonTempSlotOffset(RegSlot slotId, __out int32 * slotOffset) const
    {
        if (this->IsNonTempLocalVar(slotId))
        {
            *slotOffset = (slotId - this->GetFirstNonTempLocalIndex()) * FunctionBody::DIAGLOCALSLOTSIZE;
            return true;
        }
        return false;
    }

    bool RemoteFunctionBody::IsNonTempLocalVar(uint32 varIndex) const
    {
        return GetFirstNonTempLocalIndex() <= varIndex && varIndex < GetEndNonTempLocalIndex();
    }

    uint32 RemoteFunctionBody::GetFirstNonTempLocalIndex() const
    {
        return this->GetCounter(FunctionBody::CounterFields::ConstantCount);
    }

    uint32 RemoteFunctionBody::GetEndNonTempLocalIndex() const
    {
        uint32 firstTempReg = this->GetCounter(FunctionBody::CounterFields::FirstTmpRegister);
        return firstTempReg != Constants::NoRegister ? firstTempReg : this->GetLocalsCount();
    }

    RegSlot RemoteFunctionBody::GetLocalsCount() const
    {
        return this->GetCounter(FunctionBody::CounterFields::ConstantCount)
            + this->GetCounter(FunctionBody::CounterFields::VarCount);
    }

    RootObjectBase* RemoteFunctionBody::GetRootObject() const
    {
        Js::Var* constTableAddr = this->ToTargetPtr()->m_constTable;
        Assert(constTableAddr);

        Js::Var* rootObjectAddr = constTableAddr + Js::FunctionBody::RootObjectRegSlot - FunctionBody::FirstRegSlot;
        Js::RootObjectBase* rootObject = VirtualReader::ReadVirtual<Js::RootObjectBase*>(m_reader, rootObjectAddr);

        return rootObject;
    }

    RegSlot RemoteFunctionBody::GetFrameDisplayRegister()
    {
        RemoteFunctionBody_SourceInfo sourceInfo(m_reader, this);
        return sourceInfo->frameDisplayRegister;
    }

    Js::Var RemoteGlobalObject::ToThis()
    {
        Js::Var ret = ToTargetPtr()->secureDirectHostObject;
        if (ret)
        {
            return ret;
        }

        // Not expecting compat mode (and can't inspect HostDispatch)
        Assert(ToTargetPtr()->hostObject == nullptr);

        return GetRemoteAddr(); // this
    }
} // namespace JsDiag.
