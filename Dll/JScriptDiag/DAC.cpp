//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#include "stdafx.h"

namespace Js
{
    const wchar_t Constants::AnonymousFunction[] = L"Anonymous function";
    const wchar_t Constants::Anonymous[] = L"anonymous";
    const wchar_t Constants::FunctionCode[] = L"Function code";
    const wchar_t Constants::GlobalFunction[] = L"glo";
    const wchar_t Constants::GlobalCode[] = L"Global code";
    const wchar_t Constants::EvalCode[] = L"eval code";
    const wchar_t Constants::UnknownScriptCode[] = L"Unknown script code";
}
// --- Dummy definitions - to satisfy the linker ---
__declspec(noinline) void DebugHeap_OOM_fatal_error()
{
    Assert(false);
};
NoCheckHeapAllocator NoCheckHeapAllocator::Instance;
HANDLE NoCheckHeapAllocator::processHeap;
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

        return NULL;
    }

    template <class T>
    RecyclableObject* RemoteRecyclableObjectBase<T>::GetPrototype()
    {
        RemoteType type(m_reader, ToTargetPtr()->GetType());
        return type->GetPrototype();
    }

    template <class T>
    JavascriptLibrary* RemoteRecyclableObjectBase<T>::GetLibrary()
    {
        RemoteType type(m_reader, ToTargetPtr()->GetType());
        return type->GetLibrary();
    }

    template struct RemoteRecyclableObjectBase<RecyclableObject>;
    template struct RemoteRecyclableObjectBase<DynamicObject>;
    template struct RemoteRecyclableObjectBase<JavascriptVariantDate>;

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

    DebuggingFlags* RemoteThreadContext::GetDebuggingFlags()
    {
        return this->GetFieldAddr<DebuggingFlags>(offsetof(ThreadContext, debuggingFlags));
    }

    Js::JavascriptExceptionObject* RemoteThreadContext::GetUnhandledExceptionObject() const
    {
        // This method is in sync with ThreadContext::GetUnhandledExceptionObject() on the runtime side.
        RemoteRecyclableData recyclableData = RemoteRecyclableData(m_reader, this->ToTargetPtr()->recyclableData);
        return recyclableData->unhandledExceptionObject;
    }

    FunctionBody* RemoteFunctionInfo::GetFunction()
    {
        return (FunctionBody *)this->ToTargetPtr()->functionBodyImpl;
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
        return this->ToTargetPtr()->nativeAddress != NULL;
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

    FunctionBody* RemoteJavascriptFunction::GetFunction()
    {
        FunctionInfo* functionInfoAddr = this->ToTargetPtr()->functionInfo;
        Assert(functionInfoAddr);

        RemoteFunctionInfo functionInfo(m_reader, functionInfoAddr);
        return functionInfo.GetFunction();
    }

    bool RemoteJavascriptFunction::IsScriptFunction() const
    {
        FunctionInfo* functionInfoAddr = this->ToTargetPtr()->functionInfo;
        Assert(functionInfoAddr);

        RemoteFunctionInfo functionInfo(m_reader, functionInfoAddr);
        return !!functionInfo->HasBody();
    }

    bool RemoteJavascriptFunction::IsBoundFunction(const InspectionContext* inspectionContext) const
    {
        if (inspectionContext->IsVTable(ReadVTable(), Diag_BoundFunction, Diag_BoundFunction))
        {
            return true;
        }
        return false;
    }

    RecyclableObject* RemoteJavascriptFunction::FindCaller(
        const InspectionContext* inspectionContext,
        JsDiag::RemoteThreadContext* remoteThreadContext,
        JsDiag::RemoteScriptContext* remoteRequestContext,
        JavascriptFunction* nullObject,
        bool& foundThis)
    {
        Assert(inspectionContext);
        Assert(nullObject);
        Assert(remoteThreadContext);
        Assert(remoteRequestContext);

        JavascriptFunction* funcCaller = nullObject;

        // This method is in sync with JavascriptFunction::FindCaller() on the runtime side.
        auto reader = GetReader();
        DebugClient* debugClient = inspectionContext->GetDebugClient();

        RemoteStackWalker walker = RemoteStackWalker(
            debugClient,
            remoteThreadContext->ToTargetPtr()->currentThreadId,
            nullptr,
            false /*walkInternalFrame*/);

        foundThis = false;
        if (walker.WalkToTarget(this->GetRemoteAddr()))
        {
            foundThis = true;

            RemoteScriptConfiguration scriptConfiguration = RemoteScriptConfiguration(m_reader, remoteRequestContext->GetConfig());
            while (walker.WalkToNextJavascriptFrame())
            {
                funcCaller = walker.GetCurrentFunction();

                if (walker.IsCallerGlobalFunction())
                {                  
                    funcCaller = nullObject;
                }              

                break;
            }

            // Need this to be a RemoteRecyclableObject since it need not be a function
            RemoteRecyclableObject remoteFunctionCaller = RemoteRecyclableObject(m_reader, funcCaller);
            if (remoteFunctionCaller.GetScriptContext() != remoteRequestContext->GetRemoteAddr()
             && inspectionContext->GetTypeId(funcCaller) == Js::TypeId::TypeIds_Null)
            {
                funcCaller = nullObject;
            }
        }

        return funcCaller;
    }

    bool RemoteJavascriptFunction::GetCaller(const InspectionContext* context, Js::Var* value, CString& error)
    {
        Assert(context);
        Assert(value);

        // This method is in sync with JavascriptFunction::GetCallerProperty() on the runtime side.
        auto reader = GetReader();
        if (IsStrictMode())
        {
            error = CString(context->GetDebugClient()->GetErrorString(DIAGERR_FunctionCallNotSupported));
            return false;
        }

        RemoteJavascriptLibrary library = RemoteJavascriptLibrary(reader, GetLibrary());
        RecyclableObject* nullObject = library.GetNull();

        RemoteScriptContext scriptContext = RemoteScriptContext(reader, GetScriptContext());
        RemoteThreadContext threadContext = RemoteThreadContext(reader, scriptContext->threadContext);

        bool foundThis;
        RecyclableObject* caller = FindCaller(context, &threadContext, &scriptContext, (JavascriptFunction*)nullObject, foundThis);

        RemoteRecyclableObject remoteCaller = RemoteRecyclableObject(reader, caller);

        RemoteScriptContext remoteCallerScriptContext(reader, remoteCaller.GetScriptContext());
        if (foundThis && caller == nullObject && threadContext->HasUnhandledException())
        {
            Js::JavascriptExceptionObject* unhandledExceptionObject = threadContext.GetUnhandledExceptionObject();
            if (unhandledExceptionObject)
            {
                RemoteJavascriptExceptionObject unhandledExceptionObject = RemoteJavascriptExceptionObject(reader, threadContext.GetUnhandledExceptionObject());
                Js::JavascriptFunction* exceptionFunction = unhandledExceptionObject->GetFunction();
                if (exceptionFunction)
                {
                    RemoteJavascriptFunction remoteExceptionFunction = RemoteJavascriptFunction(reader, exceptionFunction);
                    if (scriptContext.GetRemoteAddr() == remoteExceptionFunction.GetScriptContext())
                    {
                        error = CString(context->GetDebugClient()->GetErrorString(DIAGERR_FunctionCallNotSupported));
                        return false;
                    }
                }
            }
        }
        else if (foundThis && scriptContext.GetRemoteAddr() != remoteCallerScriptContext.GetRemoteAddr())
        {
            // TODO: How do we do cross domain checking here?
        }

        if (caller != nullObject)
        {
            RemoteJavascriptFunction returnValueFunction = RemoteJavascriptFunction(reader, (Js::JavascriptFunction*)caller);
            if (returnValueFunction.IsStrictMode())
            {
                if (!threadContext->IsDisableImplicitException())
                {
                    error = CString(context->GetDebugClient()->GetErrorString(DIAGERR_FunctionCallNotSupported));
                    return false;
                }
            }
        }

        *value = caller;
        return true;
    }

    bool RemoteJavascriptFunction::GetArguments(const InspectionContext* context, Js::Var* value, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty, CString& error)
    {
        Assert(context);
        Assert(value);

        // This method is in sync with JavascriptFunction::GetArgumentsProperty() on the runtime side.
        auto reader = GetReader();
        if (IsStrictMode())
        {
            error = CString(context->GetDebugClient()->GetErrorString(DIAGERR_FunctionCallNotSupported));
            return false;
        }

        RemoteScriptContext scriptContext = RemoteScriptContext(reader, GetScriptContext());
        RemoteThreadContext threadContext = RemoteThreadContext(reader, scriptContext.GetThreadContext());
        RemoteJavascriptLibrary library = RemoteJavascriptLibrary(reader, GetLibrary());
        *value = library.GetNull();
        *ppDebugProperty = nullptr;

        DebugClient* debugClient = context->GetDebugClient();

        RemoteStackWalker walker = RemoteStackWalker(
            debugClient,
            threadContext.GetCurrentThreadId(),
            nullptr,
            false /*walkInternalFrame*/);

        if (walker.WalkToTarget(this->GetRemoteAddr()))
        {
            if (!walker.IsCallerGlobalFunction())
            {
                CComPtr<RemoteStackFrame> currentFrame;
                walker.GetCurrentJavascriptFrame(&currentFrame);
                currentFrame->SetInspectionContext(const_cast<InspectionContext*>(context));
                LocalsWalker::GetArgumentsObjectProperty(currentFrame, /*isStrictMode*/false, /*opt argumentsObject*/value, ppDebugProperty);
            }
        }

        return true;
    }

    uint16 RemoteBoundFunction::GetLength(InspectionContext* context, PROPERTY_INFO* propInfo) {
        Assert(context);
        auto reader = context->GetReader();

        RecyclableObject* targetFunction = this->ToTargetPtr()->targetFunction;
        Assert(targetFunction);
		
        RemoteJavascriptFunction remoteTargetFunction = RemoteJavascriptFunction(reader, static_cast<const JavascriptFunction*>(targetFunction));

        if (remoteTargetFunction.IsBoundFunction(context))
        {
            RemoteBoundFunction remoteBoundFunction = RemoteBoundFunction(reader, static_cast<const BoundFunction*>(targetFunction));
            return remoteBoundFunction.GetLength(context, propInfo);
        } 
        else if (remoteTargetFunction.IsScriptFunction())
        {
            return remoteTargetFunction.GetLength();
        }
        else
        {
            // It's a runtime function
            if (!context->GetProperty((Js::Var)targetFunction, Js::PropertyIds::length, propInfo)) 
            {
                DiagException::Throw(E_UNEXPECTED, DiagErrorCode::RUNTIME_GETPROPERTY);
            }
													
            return TARGETS_RUNTIME_FUNCTION;
        }
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

    FunctionBody* RemoteScriptFunction::GetFunction()
    {
        FunctionInfo* functionInfoAddr = this->ToTargetPtr()->functionInfo;
        Assert(functionInfoAddr);

        RemoteFunctionInfo functionInfo(m_reader, functionInfoAddr);
        return functionInfo.GetFunction();
    }

    bool RemoteArgumentsObject::AdvanceWalkerToArgsFrame(const InspectionContext* inspectionContext, RemoteStackWalker* walker)
    {
        Assert(inspectionContext);
        Assert(walker);

        // This method is in sync with JavascriptStackWalker::WalkToArgumentsFrame() on the runtime side.
        while (walker->WalkToNextJavascriptFrame())
        {
            Js::Var currentArgs = walker->GetPermanentArguments(inspectionContext);
            if (currentArgs == this->GetRemoteAddr())
            {
                return true;
            }
        }

        return false;
    }

    Js::Var RemoteArgumentsObject::GetCaller(
        const InspectionContext* inspectionContext,
        RemoteScriptContext* scriptContext,
        RemoteStackWalker* walker,
        JavascriptFunction* nullObject,
        bool skipGlobal)
    {
        Assert(inspectionContext);
        Assert(scriptContext);
        Assert(walker);
        Assert(nullObject);

        // This method is in sync with ArgumentsObject::GetCaller() on the runtime side.
        Js::JavascriptFunction* currentFunction = nullptr;
        RemoteScriptConfiguration scriptConfiguration = RemoteScriptConfiguration(m_reader, scriptContext->GetConfig());
        while (walker->WalkToNextJavascriptFrame())
        {
            currentFunction = walker->GetCurrentFunction();
            RemoteJavascriptFunction remoteCurrentFunction(GetReader(), currentFunction);
            if (walker->IsCallerGlobalFunction())
            {
                // Caller is global/eval. If we're in IE9 mode, and the caller is eval,
                // keep looking. Otherwise, caller is null.
                if (skipGlobal || walker->IsEvalCaller())
                {
                    continue;
                }

                currentFunction = nullptr;
            }          
            break;
        }

        if (currentFunction == nullptr
         || inspectionContext->GetTypeId(currentFunction) == Js::TypeId::TypeIds_Null)
        {
            return nullObject;
        }

        Js::CallInfo callInfo = walker->GetCurrentCallInfo();
        uint32 paramCount = callInfo.Count;
        Js::CallFlags flags = callInfo.Flags;

        if (paramCount == 0 || (flags & Js::CallFlags::CallFlags_Eval))
        {
            // The caller is the "global function" or eval, so we return "null".
            return nullObject;
        }

        Js::Var args = walker->GetPermanentArguments(inspectionContext);
        if (args == nullptr)
        {
            // TODO: How to load heap arguments?
            args = nullObject;
        }

        return args;
    }

    Js::Var RemoteArgumentsObject::GetCaller(
        const InspectionContext* inspectionContext,
        JsDiag::RemoteThreadContext* threadContext,
        JsDiag::RemoteScriptContext* scriptContext)
    {
        Assert(inspectionContext);
        Assert(threadContext);
        Assert(scriptContext);

        RemoteType type(m_reader, ToTargetPtr()->GetType());
        RemoteJavascriptLibrary library = RemoteJavascriptLibrary(m_reader, type->GetLibrary());
        JavascriptFunction* nullObject = (JavascriptFunction*)library.GetNull();

        // This method is in sync with ArgumentsObject::GetCaller() on the runtime side.
        auto reader = GetReader();
        DebugClient* debugClient = inspectionContext->GetDebugClient();

        RemoteStackWalker walker = RemoteStackWalker(
            debugClient,
            threadContext->ToTargetPtr()->currentThreadId,
            nullptr,
            false /*walkInternalFrame*/);

        if (!this->AdvanceWalkerToArgsFrame(inspectionContext, &walker))
        {
            return nullObject;
        }

        return this->GetCaller(
            inspectionContext,
            scriptContext,
            &walker,
            nullObject,
            false /*skipGlobal*/);
    }

    //static
    InlinedFrameLayout* RemoteInlinedFrameLayout::FromPhysicalFrame(
        IVirtualReader* reader, InternalStackFrame* physicalFrame, void* entry, Js::ScriptFunction* parent, FunctionEntryPointInfo* entryPoint)
    {
        Assert(physicalFrame);
        Assert(parent);

        InlinedFrameLayout* first = NULL;
        if (!physicalFrame->IsInStackCheckCode(entry))
        {
            void *frameBase = physicalFrame->FrameBase;
            first = (InlinedFrameLayout*)(((uint8*)frameBase) - RemoteScriptFunction(reader, parent).GetFrameHeight(entryPoint));
            Assert(first);
            first = RemoteInlinedFrameLayout(reader, first)->callInfo.Count ? first : NULL;
        }

        return first;
    }

    InlinedFrameLayout* RemoteInlinedFrameLayout::Next()
    {
        InlinedFrameLayout* nextAddr = (InlinedFrameLayout*)(
            reinterpret_cast<BYTE*>(const_cast<InlinedFrameLayout*>(this->m_remoteAddr)) +  // start
            sizeof(InlinedFrameLayout) +                                                    // now we point to the start of argv
            this->ToTargetPtr()->callInfo.Count * sizeof(Js::Var));
        return RemoteInlinedFrameLayout(m_reader, nextAddr)->callInfo.Count ? nextAddr : NULL;
    }

    uint32 RemoteScriptFunction::GetFrameHeight(FunctionEntryPointInfo* entryPointInfoAddr)
    {
        FunctionBody* functionBodyAddr = this->GetFunction();
        Assert(functionBodyAddr != NULL);

        RemoteFunctionBody functionBody(m_reader, functionBodyAddr);
        return functionBody.GetFrameHeight(entryPointInfoAddr);
    }

    FrameDisplay* RemoteScriptFunction::GetEnvironment()
    {
        return ReadField<FrameDisplay*>(offsetof(ScriptFunction, environment));
    }

    bool_result RemoteJavascriptFunction::TryReadDisplayName(_Out_ CString* name)
    {
        FunctionBody* pFuncBody = GetFunction();
        if (pFuncBody)
        {
            RemoteFunctionBody body(m_reader, pFuncBody);
            return body.TryReadDisplayName(name);
        }

        return false;
    }

    bool_result RemoteJavascriptFunction::TryReadDisplayName(IVirtualReader* reader, Js::Var var, _Out_ CString* name)
    {
        RemoteJavascriptFunction func(reader, static_cast<JavascriptFunction*>(var));
        return func.TryReadDisplayName(name);
    }

    Js::Var RemoteJavascriptFunction::GetSourceString(const InspectionContext* context) const
    {
        // NOTE: This is not equivalent to runtime GetSourceString. Only called when function body isn't available.

        // JavascriptFunction and bound function don't have source strings
        if (context->IsVTable(ReadVTable(), Diag_FirstNoSourceJavascriptFunction, Diag_LastNoSourceJavascriptFunction))
        {
            return nullptr;
        }

        RemoteRuntimeFunction runtimeFunc(GetReader(), (RuntimeFunction*)GetRemoteAddr());
        return runtimeFunc->functionNameId;
    }

    CString RemoteJavascriptFunction::GetDisplayNameString(InspectionContext* context)
    {
        Assert(!IsScriptFunction());
        CString name;
        Js::Var sourceString = GetSourceString(context);
        Assert(sourceString != nullptr); // We should be having the source string.
        if (sourceString != nullptr)
        {
            Js::TypeId typeId = context->GetTypeId(sourceString);
            if (typeId == Js::TypeIds_Integer)
            {
                auto reader = GetReader();
                RemoteScriptContext scriptContext = RemoteScriptContext(reader, GetScriptContext());
                Js::PropertyId nameId = TaggedInt::ToInt32(sourceString);
                name = context->ReadPropertyName(scriptContext.GetPropertyName(nameId));
            }
            else if (typeId == Js::TypeIds_String)
            {
                name = context->ReadString((JavascriptString*)sourceString);
            }
            else
            {
                Assert(false);
            }
        }

        return name;
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
            return reinterpret_cast<FrameDisplay*>(GetReg(frameDisplayRegister));
        }
        else
        {
            RemoteScriptFunction func(m_reader, ToTargetPtr()->GetJavascriptFunction());
            return func.GetEnvironment();
        }
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

    CustomHeap::Heap* RemoteEmitBufferManager::GetAllocationHeap()
    {
        return this->GetFieldAddr<CustomHeap::Heap>(offsetof(EmitBufferManager<CriticalSection>, allocationHeap));
    }

    HeapPageAllocator<VirtualAllocWrapper>* RemoteHeap::GetHeapPageAllocator()
    {
        return this->GetFieldAddr<HeapPageAllocator<VirtualAllocWrapper>>(offsetof(CustomHeap::Heap, pageAllocator));
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
        return this->GetFieldAddr<HeapPageAllocator<PreReservedVirtualAllocWrapper>>(offsetof(CustomHeap::Heap, preReservedHeapPageAllocator));
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

    EmitBufferManager<CriticalSection>* RemoteCodeGenAllocators::GetEmitBufferManager()
    {
        return this->GetFieldAddr<EmitBufferManager<CriticalSection>>(offsetof(CodeGenAllocators, emitBufferManager));
    }

    bool RemotePreReservedVirtualAllocWrapper::IsPreReservedRegionPresent()
    {
        return this->GetPreReservedStartAddress() != nullptr;
    }

    LPVOID RemotePreReservedVirtualAllocWrapper::GetPreReservedStartAddress()
    {
        return this->ReadField<LPVOID>(offsetof(TargetType, preReservedStartAddress)); 
    }

    LPVOID  RemotePreReservedVirtualAllocWrapper::GetPreReservedEndAddress()
    {
        return (char*) GetPreReservedStartAddress() + (PreReservedVirtualAllocWrapper::PreReservedAllocationSegmentCount * AutoSystemInfo::Data.GetAllocationGranularityPageCount() * AutoSystemInfo::PageSize);
    }

    bool RemotePreReservedVirtualAllocWrapper::IsInRange(void * address)
    {
        if (this->GetRemoteAddr() == nullptr || this->GetPreReservedStartAddress() == nullptr)
        {
            return false;
        }
#if DBG
        //Check if the region is in MEM_COMMIT state.
        MEMORY_BASIC_INFORMATION memBasicInfo;
        size_t bytes = VirtualQuery(address, &memBasicInfo, sizeof(memBasicInfo));
        if (bytes == 0 || memBasicInfo.State != MEM_COMMIT)
        {
            AssertMsg(false, "Memory not commited? Checking for uncommitted address region?");
        }
#endif
        return IsPreReservedRegionPresent() && address >= GetPreReservedStartAddress() && address < GetPreReservedEndAddress();
    }

    bool RemoteCodeGenAllocators::IsInRange(void* address)
    {
        RemoteEmitBufferManager emitBufferManager(m_reader, this->GetEmitBufferManager());
        RemoteHeap heap(m_reader, emitBufferManager.GetAllocationHeap());
        RemoteHeapPageAllocator heapPageAllocator(m_reader, heap.GetHeapPageAllocator());
        RemotePreReservedHeapPageAllocator preReservedHeapPageAllocator(m_reader, (HeapPageAllocator<PreReservedVirtualAllocWrapper>*)heap.GetPreReservedHeapPageAllocator());
        return preReservedHeapPageAllocator.IsInRange(address) || heapPageAllocator.IsAddressFromAllocator(address);
    }

    // Check current script context and all contexts from its thread context.
    bool RemoteScriptContext::IsNativeAddress(void* address)
    {
        RemoteThreadContext threadContext(m_reader, this->ToTargetPtr()->threadContext);

        PreReservedVirtualAllocWrapper * preReservedVirtualAllocator = threadContext.GetPreReservedVirtualAllocator();

        RemotePreReservedVirtualAllocWrapper preReservedVirtualAllocWrapper(m_reader, preReservedVirtualAllocator);

        if (preReservedVirtualAllocWrapper.IsPreReservedRegionPresent())
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

        // ScriptContext::IsNativeAddress():
        //     return IsNativeFunctionAddr(this, codeAddr) || this->threadContext->IsNativeAddress(codeAddr);
        // 1) return scriptContext->GetNativeCodeGenerator()->IsNativeFunctionAddr(address);
        //     NativeCodeGenerator::IsNativeFunctionAddr(void * address):
        //         this->backgroundAllocators && this->backgroundAllocators->emitBufferManager.IsInRange(address) ||
        //         this->foregroundAllocators && this->foregroundAllocators->emitBufferManager.IsInRange(address);
        // 2) ThreadContext -- scan all scriptContexts in the list and ask them IsNativeCodeAddress.

        // Get nativeCodeGen of ScriptContext (that being NULL is a valid case).
        if (this->IsNativeAddressCheckMeOnly(address))
        {
            return true;
        }

        // Now see in the thread context.
        return this->IsNativeAddressCheckThreadContext(address);
    }

    // Check only current script context.
    bool RemoteScriptContext::IsNativeAddressCheckMeOnly(void* address)
    {
        NativeCodeGenerator* nativeCodeGenAddr = this->ToTargetPtr()->nativeCodeGen;
        if (nativeCodeGenAddr)
        {
            RemoteNativeCodeGenerator nativeCodeGen(m_reader, nativeCodeGenAddr);
            if (this->IsNativeAddress(nativeCodeGen->backgroundAllocators, address) ||
                this->IsNativeAddress(nativeCodeGen->foregroundAllocators, address))
            {
                return true;
            }
        }
        return false;
    }

    bool RemoteScriptContext::IsNativeAddressCheckThreadContext(void* address)
    {
        RemoteThreadContext threadContext(m_reader, this->ToTargetPtr()->threadContext);
        ScriptContext* scriptContextAddr = threadContext.GetScriptContextList();
        while (scriptContextAddr)
        {
            RemoteScriptContext scriptContext(m_reader, scriptContextAddr);
            if (scriptContextAddr != m_remoteAddr)    // Prevent re-entrance.
            {
                if (scriptContext.IsNativeAddressCheckMeOnly(address))
                {
                    return true;
                }
            }
            scriptContextAddr = scriptContext->next;
        }

        return false;
    }

    
    bool RemoteScriptContext::IsNativeAddress(CodeGenAllocators* codeGenAllocatorsAddr, void* address)
    {
        if (codeGenAllocatorsAddr)
        {
            RemoteCodeGenAllocators codeGenAllocators(m_reader, codeGenAllocatorsAddr);
            if (codeGenAllocators.IsInRange(address))
            {
                return true;
            }
        }
        return false;
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

    FunctionBody::SourceInfo* RemoteFunctionBody::GetSourceInfo() const
    {
        return this->GetFieldAddr<FunctionBody::SourceInfo>(offsetof(FunctionBody, m_sourceInfo));
    }

    FunctionEntryPointInfo* RemoteFunctionBody::GetEntryPointFromNativeAddress(DWORD_PTR codeAddress)
    {
        FunctionEntryPointInfo* entryPoint = NULL;

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

        LoopEntryPointInfo* entryPoint = NULL;
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

    bool RemoteDynamicObject::HasObjectArray(InspectionContext* context)
    {
        return ((ToTargetPtr()->objectArray != null) && !ToTargetPtr()->UsesObjectArrayOrFlagsAsFlags() && 
            !((RemoteDynamicTypeHandler(context, GetTypeHandler(), ToTargetPtr())).IsObjectHeaderInlinedTypeHandler()));
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
        Assert(this->ToTargetPtr()->loopHeaderArray != NULL);
        Assert(index < this->ToTargetPtr()->loopCount);
        
        return this->ToTargetPtr()->loopHeaderArray + index;
    }

    BOOL RemoteFunctionBody::GetMatchingStatementMapFromNativeOffset(
        StatementData* statementMap, DWORD_PTR codeAddress, uint loopNum, uint32 inlineeOffset, FunctionBody *inlinee /* = NULL */)
    {
        AssertMsg(loopNum == LoopHeader::NoLoop || inlineeOffset == 0 && inlinee == NULL, 
            "Interpreter Jit loop body frame can't have inlinees.");
        return inlineeOffset != 0 ? 
            this->GetMatchingStatementMapFromNativeOffset(statementMap, codeAddress, inlineeOffset, inlinee) :
            this->GetMatchingStatementMapFromNativeAddress(statementMap, codeAddress, loopNum, inlinee);
    }

    BOOL RemoteFunctionBody::GetMatchingStatementMapFromNativeAddress(
        StatementData* statementMap, DWORD_PTR codeAddress, uint loopNum, FunctionBody *inlinee /* = NULL */)
    {
        SmallSpanSequence* spanSequence = NULL;
        DWORD_PTR nativeBaseAddress = NULL;

        FunctionEntryPointInfo* entryPointAddr = this->GetEntryPointFromNativeAddress(codeAddress);
        if (entryPointAddr != NULL)
        {
            RemoteFunctionEntryPointInfo entryPoint(m_reader, entryPointAddr);
            spanSequence = entryPoint.GetNativeThrowSpanSequence();
            nativeBaseAddress = entryPoint.GetNativeAddress();
        }
        else
        {
            LoopEntryPointInfo* entryPointAddr = this->GetLoopEntryPointFromNativeAddress(codeAddress, loopNum);
            if (entryPointAddr != NULL)
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
        StatementData* statementMap, DWORD_PTR codeAddress, uint32 offset, FunctionBody *inlinee /* = NULL */)
    {
        FunctionEntryPointInfo* entryPointAddr = this->GetEntryPointFromNativeAddress(codeAddress);

        SmallSpanSequence * spanSequence = NULL;
        if (entryPointAddr != NULL)
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

    bool RemoteFunctionBody::InstallProbe(int offset, RemoteAllocator* allocator)
    {        
        RemoteByteBlock remoteByteCodeBlock(m_reader, this->ToTargetPtr()->byteCodeBlock);
        if (offset < 0 || ((uint)offset + 1) >= remoteByteCodeBlock->m_contentSize)
        {
            Assert(false);
            DiagException::Throw(E_UNEXPECTED, DiagErrorCode::PROBE_OFFSET_OUTOFBOUND);
        }

        RemoteFunctionBody_SourceInfo remoteSourceInfo(this->m_reader, this);
        ByteBlock* probeBackingBlock = remoteSourceInfo.GetProbeBackingBlock();
        HRESULT hr = S_OK;
        BYTE* byteCodeBuffer = remoteByteCodeBlock->m_content;
        if(!probeBackingBlock)
        {
            int bufferSize = remoteByteCodeBlock->m_contentSize;
            BYTE* remoteProbeByteCodeBuffer = (BYTE*)allocator->Allocate(bufferSize);
            AutoArrayPtr<BYTE> byteCodeCopy = VirtualReader::ReadBuffer(this->m_reader, byteCodeBuffer, bufferSize);
            hr = m_reader->WriteMemory(remoteProbeByteCodeBuffer, byteCodeCopy, bufferSize);
            CheckHR(hr, DiagErrorCode::WRITE_VIRTUAL);
            probeBackingBlock = allocator->Allocate<ByteBlock>();

            RemoteByteBlock remoteProbeBackingBlock(m_reader, probeBackingBlock);
            remoteProbeBackingBlock->m_contentSize = remoteByteCodeBlock->m_contentSize;
            remoteProbeBackingBlock->m_content = remoteProbeByteCodeBuffer;
            remoteProbeBackingBlock.Flush();
            remoteSourceInfo.SetProbeBackingBlock(probeBackingBlock);
        }
        uint8 opcode = (uint8)OpCode::Break;
        uint8 currentOpCode = VirtualReader::ReadVirtual<uint8>(m_reader, byteCodeBuffer + offset);

        // If a breakpoint is set on the debugger keyword, we still want to refrence count it.
        remoteSourceInfo.IncrementProbeCount();
        if (opcode == currentOpCode)
        {
            return false;
        }
        hr = m_reader->WriteMemory(byteCodeBuffer + offset, &opcode, sizeof(uint8));
        CheckHR(hr, DiagErrorCode::WRITE_VIRTUAL);

        return true;
    }

    void RemoteFunctionBody::UninstallProbe(int offset)
    {        
        RemoteByteBlock remoteByteCodeBlock(m_reader, this->ToTargetPtr()->byteCodeBlock);
        if (offset < 0 || ((uint)offset + 1) >= remoteByteCodeBlock->m_contentSize)
        {
            Assert(false);
            DiagException::Throw(E_UNEXPECTED, DiagErrorCode::PROBE_OFFSET_OUTOFBOUND);
        }

        RemoteFunctionBody_SourceInfo remoteSourceInfo(this->m_reader, this);
        ByteBlock* probeBackingBlock = remoteSourceInfo.GetProbeBackingBlock();
        Assert(probeBackingBlock);
        RemoteByteBlock remoteProbeBackingBlock(m_reader, probeBackingBlock);

        // Although some of the original opcode are two bytes, we only ever replace one of them
        // so just restore the one byte
        uint8 originalOpCodeByte = VirtualReader::ReadVirtual<uint8>(m_reader, remoteProbeBackingBlock->m_content + offset);

        BYTE* byteCodeBuffer = remoteByteCodeBlock->m_content;
     
        HRESULT hr = m_reader->WriteMemory(byteCodeBuffer + offset, &originalOpCodeByte, sizeof(uint8));
        CheckHR(hr, DiagErrorCode::WRITE_VIRTUAL);
           

        remoteSourceInfo.DecrementProbeCount();
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
                    && (firstStatementLocation->function == NULL || firstStatementLocation->statement.begin < pSourceSpan->begin))
                {
                    firstStatementLocation->function = this->GetRemoteAddr();
                    firstStatementLocation->statement = *pSourceSpan;
                    firstStatementLocation->bytecodeSpan = statementMap->byteCodeSpan;
                }
                else if (pSourceSpan->begin >= characterOffset
                    && (secondStatementLocation->function == NULL || secondStatementLocation->statement.begin > pSourceSpan->begin))
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
                Assert(this->ToTargetPtr()->pStatementMaps);

                RemoteList<Js::FunctionBody::StatementMap*> statementMaps(m_reader, this->ToTargetPtr()->pStatementMaps);
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

    uint32 RemoteGrowingUint32HeapArray::ItemInBuffer(int index)
    {
        if (index < 0 || index >= this->ToTargetPtr()->count)
        {
            return 0;
        }

        // Read index's item into RemoteData and return the value read.
        RemoteData<uint32> item(m_reader, this->ToTargetPtr()->buffer + index);
        return *static_cast<uint32*>(item);
    }

        
    int RemoteSmallSpanSequence::Count()
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
        if (index >= statementBuffer->count)
        {
            return FALSE;
        }

        if (iter.accumulatedIndex <= 0 || iter.accumulatedIndex > index)
        {
            this->ResetIterator(iter);
        }

        while (iter.accumulatedIndex <= index)
        {
            Assert(iter.accumulatedIndex < statementBuffer->count);

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
        while (iter.accumulatedIndex < this->Count())
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
                || iter.accumulatedIndex <= 0 || iter.accumulatedIndex >= this->Count())
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
        Assert(index < this->Count());
        RemoteGrowingUint32HeapArray statementBuffer(m_reader, this->ToTargetPtr()->pStatementBuffer);
        SmallSpan span(statementBuffer.ItemInBuffer(index));  // Note: SmallSpan is fine to use as a data class.
        int countOfMissed = 0;

        if ((short)span.sourceBegin == SHRT_MAX)
        {
            // Look in ActualOffset store
            Assert(this->ToTargetPtr()->pActualOffsetList);

            RemoteGrowingUint32HeapArray actualOffsetList(m_reader, this->ToTargetPtr()->pActualOffsetList);
            Assert(actualOffsetList->count > 0);
            Assert(actualOffsetList->count > iter.indexOfActualOffset);

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
            Assert(actualOffsetList->count > 0);
            Assert(actualOffsetList->count > iter.indexOfActualOffset + countOfMissed);

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
                RemoteList<FunctionBody::StatementMap*> maps(m_reader, mapsAddr);
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

        Assert(this->ToTargetPtr()->pSpanSequence == NULL);

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


    bool RemoteFunctionBody_SourceInfo::HasLineBreak(charcount_t start, charcount_t end)
    {
        if (start > end) 
        {
            return false;
        }
        charcount_t cchLength = end - start;
        if (start < this->GetFunctionBody()->m_cchStartOffset || cchLength > this->GetFunctionBody()->m_cchLength) 
        {
            return false;
        }

        size_t lengthInBytes = this->LengthInBytes();
        RemoteUtf8SourceInfo funcUtf8SourceInfo(m_reader, this->GetFunctionBody()->m_utf8SourceInfo);
        LPCUTF8 src = funcUtf8SourceInfo.GetDebugModeSource() + this->GetFunctionBody()->m_cbStartOffset;
        size_t offset; 

        // TODO: PERF: Consider copying only part of this.
        RemoteBuffer<utf8char_t> source(m_reader, const_cast<LPUTF8>(src), lengthInBytes, lengthInBytes);
        if(lengthInBytes == this->GetFunctionBody()->m_cchLength)
        {
            offset = start - this->GetFunctionBody()->m_cchStartOffset;
        }
        else
        {
            offset = utf8::CharacterIndexToByteIndex(source.Ptr, lengthInBytes, start - this->GetFunctionBody()->m_cchStartOffset, utf8::doAllowThreeByteSurrogates);
        }
        src = source.Ptr + offset;
        utf8::DecodeOptions options = utf8::doAllowThreeByteSurrogates;

        // Note that "end" can belong to another function, so we have to make sure we don't go over length of current func.
        // that we read into the buffer. In the inproc case there is no such issue, as we we have one utf8 source buffer for the whole file
        // and don't read parts of it into another buffer.
        LPCUTF8 last = min(src + cchLength, source.Ptr + lengthInBytes);
        while (src < last)
        {
            switch (utf8::Decode(src, last, options))
            {
            case '\r':
            case '\n':
            case 0x2028:
            case 0x2029:
                return true;
            }
        }

        return false;
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
            wchar_t ch = utf8::Decode(allSource.Ptr, allSource.Ptr + availableBytes, options);

            switch (ch)
            {
            case L'\r':
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
            case L'\n':
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

    FunctionBody::StatementMapList* RemoteFunctionBody_SourceInfo::GetStatementMaps()
    {        
        return this->GetFunctionBody()->pStatementMaps;
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

    ByteBlock* RemoteFunctionBody_SourceInfo::GetProbeBackingBlock()
    {        
        return this->ToTargetPtr()->m_probeBackingBlock;
    }

    void RemoteFunctionBody_SourceInfo::SetProbeBackingBlock(ByteBlock* block)
    {
        this->WriteField(offsetof(FunctionBody::SourceInfo, m_probeBackingBlock), block);
    }

    void RemoteFunctionBody_SourceInfo::IncrementProbeCount()
    {
        int32 currentCount = this->ToTargetPtr()->m_probeCount;
        this->WriteField(offsetof(FunctionBody::SourceInfo, m_probeCount), currentCount + 1);
    }

    void RemoteFunctionBody_SourceInfo::DecrementProbeCount()
    {
        int32 currentCount = this->ToTargetPtr()->m_probeCount;
        AssertMsg(currentCount > 0, "ProbeCount is already 0 or less, and we are about to decrement!");
        this->WriteField(offsetof(FunctionBody::SourceInfo, m_probeCount), currentCount - 1);
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
    // Indicate to the target process that a hybrid debugger is attached.
    void RemoteConfiguration::SetHybridDebugging(IVirtualReader* reader, const Configuration* addr)
    {
        HRESULT hr;

        // Write the GUID -- important for launch scenario as Configuration::Global (static object) ctor hasn't run yet.
        hr = reader->WriteMemory(((BYTE*)addr) + offsetof(Configuration, hybridDebuggingGuid), &HybridDebuggingGuid, sizeof(HybridDebuggingGuid));
        CheckHR(hr, DiagErrorCode::WRITE_VIRTUAL);

        // Write to isHybridDebugging directly -- important for attach, Configuration::Global (static object) ctor has already run.
        bool trueConst = true;
        hr = reader->WriteMemory(((BYTE*)addr) + offsetof(Configuration, isHybridDebugging), &trueConst, sizeof(trueConst));
        CheckHR(hr, DiagErrorCode::WRITE_VIRTUAL);
    }

    bool RemoteFunctionBody::Is(InspectionContext* context, void* ptr)
    { 
        if(ptr) 
        { 
            return context->MatchVTable(ptr, Diag_FunctionBody);
        }
        return false;
    }

    bool_result RemoteFunctionBody::TryReadDisplayName(_Out_ CString* name) const
    {
        Assert(name);
        LPCWSTR displayName = (*this)->m_displayName;
        if (displayName)
        {
            *name = InspectionContext::ReadString(m_reader, displayName, DiagConstants::MaxFunctionNameLength);
            return true;
        }

        return false;
    }

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
            else if (sourceName != null) // remote url
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

    const wchar_t* RemoteFunctionBody::GetExternalDisplayName(
        const wchar_t* displayName, BOOL isDynamicScript, BOOL isGlobalFunc) const
    {
        GetFunctionBodyNameData funcBody(*this, displayName, isDynamicScript, isGlobalFunc);
        return FunctionBody::GetExternalDisplayName(&funcBody);
    }

    UINT64 RemoteFunctionBody::GetDocumentId() const
    {
        RemoteFunctionBody_SourceInfo sourceInfo(m_reader, this);

        const Utf8SourceInfo* utf8SourceInfoPtr = (*this)->GetUtf8SourceInfo();
        if(!utf8SourceInfoPtr)
        {
            DiagException::Throw(E_UNEXPECTED, DiagErrorCode::RUNTIME_NULL_UTF8SOURCEINFO);
        }
        
        RemoteUtf8SourceInfo remoteUtf8SourceInfo(m_reader, utf8SourceInfoPtr);
        UINT64 documentId = (UINT64)remoteUtf8SourceInfo.GetDocumentId();
        return documentId;
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
            *slotOffset = (slotId - this->GetFirstNonTempLocalIndex()) * DIAGLOCALSLOTSIZE;
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
        return this->ToTargetPtr()->m_constCount;
    }

    uint32 RemoteFunctionBody::GetEndNonTempLocalIndex() const
    {
        uint32 firstTempReg = this->ToTargetPtr()->m_firstTmpReg;
        return firstTempReg != Constants::NoRegister ? firstTempReg : this->GetLocalsCount();
    }

    RegSlot RemoteFunctionBody::GetLocalsCount() const
    {
        return this->ToTargetPtr()->m_constCount + this->ToTargetPtr()->m_varCount;
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

    void RemotePropertyIdOnRegSlotsContainer::FetchItemAt(uint index, RemoteFunctionBody* pFuncBody, _Out_ Js::PropertyId* pPropId, _Out_ RegSlot* pRegSlot) const
    {
        *pPropId = m_propertyIdsForRegSlots.Item(index);
        *pRegSlot = pFuncBody->ToTargetPtr()->MapRegSlot(index);
    }

    void* RemoteFrameDisplay::GetItem(uint index) const
    {
        return m_scopes.Item(index);
    }

    bool RemoteTypePath::TryLookup(Js::PropertyId propId, int typePathLength, PropertyIndex* index)
    {
        return ToTargetPtr()->map.TryGetValue(propId, index, *this)
            && *index < typePathLength;
    }

    RemotePropertyRecord RemoteTypePath::operator[](const int index) const
    {
        RemoteArray<const PropertyRecord *> assignments(m_reader, GetRemoteAddr()->GetPropertyAssignments());
        return RemotePropertyRecord(m_reader, assignments[index]);
    }

    bool RemoteJavascriptBoolean::GetValue(IVirtualReader* reader, Js::Var var)
    {
        RemoteJavascriptBoolean obj(reader, reinterpret_cast<const TargetType*>(var));
        return obj->GetValue() ? true : false;
    }

    CString RemoteJavascriptSymbol::GetValue(IVirtualReader* reader, Js::Var var)
    {
        RemoteJavascriptSymbol obj(reader, reinterpret_cast<const TargetType*>(var));
        const PropertyRecord* propertyRecord = obj->GetValue();

        CString name = L"Symbol(";
        name += InspectionContext::ReadPropertyName(reader, propertyRecord);
        name += L")";

        return name;
    }

    double RemoteJavascriptNumber::GetValue(IVirtualReader* reader, Js::Var var)
    {
#if FLOATVAR
        return JavascriptNumber::GetValue(var);
#else
        RemoteJavascriptNumber number(reader, reinterpret_cast<TargetType*>(var));
        return number->GetValue();
#endif
    }

    bool RemoteJavascriptBooleanObject::GetValue()
    {
        JavascriptBoolean* value = ToTargetPtr()->value;
        return value != NULL ? RemoteJavascriptBoolean::GetValue(m_reader, value) : false;
    }

    bool RemoteJavascriptBooleanObject::GetValue(IVirtualReader* reader, Js::Var var)
    {
        RemoteJavascriptBooleanObject obj(reader, reinterpret_cast<JavascriptBooleanObject*>(var));
        return obj.GetValue();
    }

    CString RemoteJavascriptSymbolObject::GetValue()
    {
        JavascriptSymbol* value = ToTargetPtr()->value;

        Assert(value != nullptr);

        return RemoteJavascriptSymbol::GetValue(m_reader, value);
    }

    CString RemoteJavascriptSymbolObject::GetValue(IVirtualReader* reader, Js::Var var)
    {
        RemoteJavascriptSymbolObject obj(reader, reinterpret_cast<JavascriptSymbolObject*>(var));
        return obj.GetValue();
    }

    double RemoteJavascriptNumberObject::GetValue()
    {
        Js::Var value = ToTargetPtr()->value;
        return TaggedInt::Is(value) ?
            TaggedInt::ToDouble(value) : RemoteJavascriptNumber::GetValue(m_reader, value);
    }

    double RemoteJavascriptNumberObject::GetValue(IVirtualReader* reader, Js::Var var)
    {
        RemoteJavascriptNumberObject obj(reader, reinterpret_cast<JavascriptNumberObject*>(var));
        return obj.GetValue();
    }

    CString RemoteJavascriptRegExp::GetSource()
    {
        PCWSTR source;
        charcount_t sourceLength;
        GetSource(&source, &sourceLength);

        return InspectionContext::ReadStringLen(GetReader(), source, sourceLength);
    }

    void RemoteJavascriptRegExp::GetSource(PCWSTR* pSource, charcount_t* pLength)
    {
        // In sync with JavascriptRegExp::GetPropertyBuiltIns() on the in-proc side.
        auto reader = GetReader();

        RemoteData<UnifiedRegex::RegexPattern> pattern(reader, ToTargetPtr()->pattern);
        RemoteData<UnifiedRegex::Program> program(reader, pattern->rep.unified.program);
        *pSource = program->source;
        *pLength = program->sourceLen;    
    }

    bool RemoteRegexPattern::IsFlagSet(UnifiedRegex::RegexFlags flag) const
    {
        return (this->cachedFlags & flag) != 0;
    }

    bool RemoteRegexPattern::IsGlobal() const
    {
        // In sync with RegexPattern::IsGlobal() on the in-proc side.
        return IsFlagSet(UnifiedRegex::GlobalRegexFlag);
    }

    bool RemoteRegexPattern::IsMultiline() const
    {
        // In sync with RegexPattern::IsMultiline() on the in-proc side.
        return IsFlagSet(UnifiedRegex::MultilineRegexFlag);
    }

    bool RemoteRegexPattern::IsIgnoreCase() const
    {
        // In sync with RegexPattern::IsIgnoreCase() on the in-proc side.
        return IsFlagSet(UnifiedRegex::IgnoreCaseRegexFlag);
    }

    bool RemoteRegexPattern::IsUnicode() const
    {
        // In sync with RegexPattern::IsUnicode() on the in-proc side.
        return IsFlagSet(UnifiedRegex::UnicodeRegexFlag);
    }

    bool RemoteRegexPattern::IsSticky() const
    {
        // In sync with RegexPattern::IsSticky() on the in-proc side.
        return IsFlagSet(UnifiedRegex::StickyRegexFlag);
    }

    CString RemoteJavascriptRegExp::GetOptions(bool IsES6UnicodeExtensionsEnabled /* = false */, bool isEs6RegExpFlagsEnable /* = false */) const
    {
        CString options;

        // In sync with JavascriptRegExp::GetPropertyBuiltIns() on the in-proc side.
        auto reader = GetReader();
        RemoteRegexPattern pattern(reader, ToTargetPtr()->pattern);

        // The ordering of options display varies between compatibility modes.  See
        // JavascriptRegExp::GetPropertyBuiltIns() for the matching details.
        if (pattern.IsGlobal())
        {
            options.AppendChar(L'g');
        }

        if (pattern.IsIgnoreCase())
        {
            options.AppendChar(L'i');
        }

        if (pattern.IsMultiline())
        {
            options.AppendChar(L'm');
        }
        if (IsES6UnicodeExtensionsEnabled && pattern.IsUnicode())
        {
            options.AppendChar(L'u');
        }
        if (isEs6RegExpFlagsEnable && pattern.IsSticky())
        {
            options.AppendChar(L'y');
        }

        return options;
    }

    const PropertyRecord* RemoteExternalObject::GetClassName()
    {
        RemoteExternalType type(m_reader, ToTargetPtr()->GetExternalType());
        Js::PropertyId propertyId = type.GetNameId();

        return RemoteScriptContext(m_reader, GetScriptContext()).GetPropertyName(propertyId);
    }
    
    bool RemoteExternalObject::IsProjectionObjectInstance(DebugClient* debugClient) const
    {
        const void* vtable = this->ReadVTable();
        return vtable == debugClient->GetVTable(Diag_ProjectionObjectInstance) || vtable == debugClient->GetVTable(Diag_EventHandlingProjectionObjectInstance);
    }

    Js::Var RemoteGlobalObject::ToThis()
    {
        Js::Var ret = ToTargetPtr()->secureDirectHostObject;
        if (ret)
        {
            return ret;
        }

        // Not expecting compat mode (and can't inspect HostDispatch)
        Assert(ToTargetPtr()->hostObject == NULL);

        return GetRemoteAddr(); // this
    }

    // Determines if the DebuggerScope contains a property with the passed in ID and
    // location in the internal property list.
    bool RemoteDebuggerScope::ContainsProperty(RemoteStackFrame* frame, Js::PropertyId propertyId, Js::RegSlot location, DebuggerScopeProperty* outProperty /* nullptr*/)
    {
        Assert(frame);
        InspectionContext* context = frame->GetInspectionContext();
        IVirtualReader* reader = context->GetReader();
        DebuggerScope* debuggerScope = ToTargetPtr();
        if (debuggerScope->scopeProperties == nullptr)
        {
            return false;
        }

        RemoteDebuggerScopePropertyList scopeProperties = RemoteDebuggerScopePropertyList(reader, debuggerScope->scopeProperties);
        bool foundPropertyMatch = scopeProperties.MapUntil(
            [&](uint i, DebuggerScopeProperty& debuggerScopeProperty)
        {
            if (debuggerScopeProperty.propId == propertyId && debuggerScopeProperty.location == location)
            {
                if (outProperty != nullptr)
                {
                    *outProperty = debuggerScopeProperty;
                }

                return true;
            }

            return false;
        });

        return foundPropertyMatch;

    }

    bool RemoteDebuggerScope::ContainsValidProperty(RemoteStackFrame* frame, Js::PropertyId propertyId, Js::RegSlot location, int offset, bool* isInDeadZone)
    {
        Assert(isInDeadZone);
        if (propertyId == Js::PropertyIds::_lexicalThisSlotSymbol)
        {
            return false;
        }

        DebuggerScopeProperty scopeProperty;
        if (ContainsProperty(frame, propertyId, location, &scopeProperty))
        {
            *isInDeadZone = scopeProperty.IsInDeadZone(offset);

            // Validates the current scope is included.
            return ToTargetPtr()->range.Includes(offset);
        }

        return false;
    }

    void RemoteDebuggingFlags::SetForceInterpreter(bool value)
    {
        size_t offset = offsetof(DebuggingFlags, m_forceInterpreter);
        this->WriteField<bool>(offset, value);
    }

} // namespace JsDiag.
