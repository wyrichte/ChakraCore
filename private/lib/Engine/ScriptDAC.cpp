//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "EnginePch.h"
#include "CodeGenAllocators.h"
#include "CodeGenWorkItemType.h"
#include "QueuedFullJitWorkItem.h"
#include "NativeCodeGenerator.h"

#define DebugSiteOut(fmt, ...)              \
    {                                       \
        WCHAR buf[256];                     \
        swprintf_s(buf, fmt, __VA_ARGS__);  \
        debugSite->Out(buf);                \
    }                                       \

namespace Js
{
    typedef RemoteData<ThreadContext, DynamicDataBuffer> RemoteThreadContext;
    typedef RemoteData<InProcCodeGenAllocators, DynamicDataBuffer> RemoteInProcCodeGenAllocators;

    ScriptDAC::ScriptDAC()
    {
    }

    //
    // Load script symbols into a script debug site.
    //
    STDMETHODIMP ScriptDAC::LoadScriptSymbols(IScriptDebugSite* debugSite)
    {
        IfNullReturnError(debugSite, E_INVALIDARG);
        HRESULT hr = S_OK;

        LPCVOID threadContextListPtr;
        IfFailGo(debugSite->FindSymbol(_u("ThreadContext::globalListFirst"), &threadContextListPtr));

        const ThreadContext* threadContextPtr;
        IfFailGo(ReadPointer(debugSite, threadContextListPtr, &threadContextPtr));

        while (threadContextPtr)
        {
            RemoteThreadContext threadContext;
            IfFailGo(threadContext.Read(debugSite, threadContextPtr));

            const ScriptContext* scriptContextPtr = threadContext->GetScriptContextList();
            while (scriptContextPtr)
            {
                RemoteScriptContext scriptContext;
                IfFailGo(scriptContext.Read(debugSite, scriptContextPtr));

                // Load symbols for functions in the script context
                IfFailGo(scriptContext.LoadScriptSymbols(debugSite));

                scriptContextPtr = scriptContext->next;
            }

            threadContextPtr = threadContext->Next();
        }

Error:
        return hr;
    }

    //
    // Read requested bufferSize bytes. Fail if read less bytes than requested.
    //
    HRESULT ScriptDAC::Read(IScriptDebugSite* debugSite, LPCVOID addr, void* buffer, ULONG bufferSize)
    {
        ULONG bytesRead;
        HRESULT hr = debugSite->ReadVirtual(addr, buffer, bufferSize, &bytesRead);
        if (SUCCEEDED(hr) && bytesRead != bufferSize)
        {
            hr = E_FAIL;
        }
        return hr;
    }

    //
    // Load symbols for functions in a script context.
    //
    HRESULT RemoteScriptContext::LoadScriptSymbols(IScriptDebugSite* debugSite)
    {
        HRESULT hr = S_OK;

        // Read remote interpreter and jit emit buffer allocations and add synthetic modules
        IfFailGo(ReadEmitBufferAllocations(debugSite));

        MapFunctions(debugSite, [&debugSite, this] (Js::LocalFunctionId functionId, FunctionBody* funcBody)
        {
            RemoteFunctionBody functionBody;
            if (SUCCEEDED(functionBody.Read(debugSite, funcBody)))
            {
                // Ignore failures, try next one if failed.
                functionBody.LoadSymbols(debugSite, *this);
            }
        });

Error:
        return hr;
    }

    //
    // Read remote interpreter and jit emit buffer allocations of the script context and add synthetic modules for them.
    //
    HRESULT RemoteScriptContext::ReadEmitBufferAllocations(IScriptDebugSite* debugSite)
    {
        HRESULT hr = S_OK;

        // Read interpret emit buffer allocations
        RemoteData<InterpreterThunkEmitter> interpreterThunkEmitter;
#ifdef ASMJS_PLAT
        RemoteData<InterpreterThunkEmitter> asmJsInterpreterThunkEmitter;
#endif
        IfFailGo(interpreterThunkEmitter.Read(debugSite, (*this)->interpreterThunkEmitter));

        if (interpreterThunkEmitter->GetEmitBufferManager()->allocations != nullptr)
        {
            IfFailGo(AddSyntheticModules(debugSite, interpreterThunkEmitter->GetEmitBufferManager()->allocations));
        }
        else
        {
            RemoteList<ThunkBlock, ArenaAllocator> remoteList((SListNode<ThunkBlock, ArenaAllocator> *)interpreterThunkEmitter->GetThunkBlocksList());

            IfFailGo(remoteList.Map(debugSite, [&](ThunkBlock * data) 
            {
                HRESULT hr = S_OK;
                void * address = data->GetStart();
                IfFailGo(debugSite->AddSyntheticModule(address, InterpreterThunkEmitter::BlockSize));
            Error:
                return hr;
            }));
        }
#ifdef ASMJS_PLAT
        IfFailGo(asmJsInterpreterThunkEmitter.Read(debugSite, (*this)->asmJsInterpreterThunkEmitter));
        IfFailGo(AddSyntheticModules(debugSite, asmJsInterpreterThunkEmitter->GetEmitBufferManager()->allocations));
#endif

#if ENABLE_NATIVE_CODEGEN
        // Read jit emit buffer allocations
        const NativeCodeGenerator* nativeCodeGenPtr = (*this)->GetNativeCodeGenerator();
        if (nativeCodeGenPtr)
        {
            RemoteData<NativeCodeGenerator> nativeCodeGen;
            IfFailGo(nativeCodeGen.Read(debugSite, nativeCodeGenPtr));

            /*
                For OOPJIT -> allocations will be nullptr and hence we add the address range by walking through the cache.
                For InProcJIT -> either one of the allocators will not be null and will add the address range - we don't have to walk through the cache in this case.
            */
            bool modulesAdded = false;

            if (nativeCodeGen->foregroundAllocators)
            {
                RemoteInProcCodeGenAllocators foregroundAllocators;
                IfFailGo(foregroundAllocators.Read(debugSite, nativeCodeGen->foregroundAllocators));
                if (foregroundAllocators->emitBufferManager.allocations != nullptr)
                {
                    IfFailGo(AddSyntheticModules(debugSite, foregroundAllocators->emitBufferManager.allocations));
                    modulesAdded = true;
                }
            }

            if (nativeCodeGen->backgroundAllocators)
            {
                RemoteInProcCodeGenAllocators backgroundAllocators;
                IfFailGo(backgroundAllocators.Read(debugSite, nativeCodeGen->backgroundAllocators));
                if (backgroundAllocators->emitBufferManager.allocations != nullptr)
                {
                    IfFailGo(AddSyntheticModules(debugSite, backgroundAllocators->emitBufferManager.allocations));
                    modulesAdded = true;
                }
            }

            if (!modulesAdded)
            {
                RemoteThreadContext threadContext;
                IfFailGo(threadContext.Read(debugSite, (*this)->GetThreadContext()));
                uintptr_t preReservedRegionStartAddr = threadContext->GetPreReservedRegionAddr();

                if (preReservedRegionStartAddr != 0)
                {
                    uintptr_t preReservedRegionEndAddr = (uintptr_t)PreReservedVirtualAllocWrapper::GetPreReservedEndAddress((void*)preReservedRegionStartAddr);
                    IfFailGo(debugSite->AddSyntheticModule((void*)preReservedRegionStartAddr, (ULONG)(preReservedRegionEndAddr - preReservedRegionStartAddr)));
                }

                RemoteData<JITPageAddrToFuncRangeCache> jitPageAddrToFuncRangeCache;
                IfFailGo(jitPageAddrToFuncRangeCache.Read(debugSite, (*this)->GetJitFuncRangeCache()));
                if (jitPageAddrToFuncRangeCache.GetRemoteAddress() != nullptr)
                {
                    if (jitPageAddrToFuncRangeCache->GetJITPageAddrToFuncRangeMap() != nullptr)
                    {
                        RemoteDictionary<JITPageAddrToFuncRangeCache::JITPageAddrToFuncRangeMap> jitPageAddrToFuncRangeMap;
                        IfFailGo(jitPageAddrToFuncRangeMap.Read(debugSite, jitPageAddrToFuncRangeCache->GetJITPageAddrToFuncRangeMap()));

                        if (jitPageAddrToFuncRangeMap.GetRemoteAddress() != nullptr)
                        {
                            jitPageAddrToFuncRangeMap.Map(debugSite, [&](void * pageAddr, JITPageAddrToFuncRangeCache::RangeMap * rangeMap)
                            {
                                RemoteDictionary<JITPageAddrToFuncRangeCache::RangeMap> remoteRangeMap;
                                IfFailGo(remoteRangeMap.Read(debugSite, rangeMap));
                                Assert(remoteRangeMap != nullptr);

                                remoteRangeMap.Map(debugSite, [&](void * address, uint bytes)
                                {
                                    IfFailGo(debugSite->AddSyntheticModule(address, bytes));
                                Error:
                                    return;
                                });
                            Error:
                                return;
                            });
                        }
                    }

                    if (jitPageAddrToFuncRangeCache->GetLargeJITFuncAddrToSizeMap() != nullptr)
                    {
                        RemoteDictionary<JITPageAddrToFuncRangeCache::LargeJITFuncAddrToSizeMap> jitLargePageAddrMap;
                        IfFailGo(jitLargePageAddrMap.Read(debugSite, jitPageAddrToFuncRangeCache->GetLargeJITFuncAddrToSizeMap()));
                        jitLargePageAddrMap.Map(debugSite, [&](void * pageAddr, uint bytes)
                        {
                            IfFailGo(debugSite->AddSyntheticModule(pageAddr, bytes));
                        Error:
                            return;
                        });
                    }
                }
            }
        }
#endif

Error:
        return hr;
    }

    //
    // Add synthetic modules for emit buffer allocations (for holding synthetic symbols).
    //
    HRESULT RemoteScriptContext::AddSyntheticModules(IScriptDebugSite* debugSite, const EmitBufferAllocation<VirtualAllocWrapper, PreReservedVirtualAllocWrapper>* allocation)
    {
        return MapLinkedList(debugSite, allocation,
            [&](const RemoteData<EmitBufferAllocation<VirtualAllocWrapper, PreReservedVirtualAllocWrapper>>& emitBufferAllocation, const EmitBufferAllocation<VirtualAllocWrapper, PreReservedVirtualAllocWrapper>** addr) -> HRESULT
        {
            HRESULT hr = S_OK;

            RemoteData<CustomHeap::Allocation> allocation;
            IfFailGo(allocation.Read(debugSite, emitBufferAllocation->allocation));

            LPCVOID bufferStart = allocation->address;
            ULONG bytesUsed = emitBufferAllocation->bytesUsed;
            IfFailGo(debugSite->AddSyntheticModule(bufferStart, bytesUsed));

            // Advance addr to next
            *addr = emitBufferAllocation->nextAllocation;
Error:
            return hr;
        });
    }

    //
    // Load function body and jit loop body symbols.
    //
    HRESULT RemoteFunctionBody::LoadSymbols(
        IScriptDebugSite* debugSite, const RemoteScriptContext& scriptContext) const
    {
        HRESULT hr = S_OK;

        WCHAR name[MAX_FUNCTION_NAME], url[MAX_URL];
        ULONG line, column;

        IfFailGo(GetFunctionBodyInfo(debugSite, name, _countof(name), url, _countof(url), &line, &column));

        // Load interpreted symbol
        if ((*this)->HasInterpreterThunkGenerated())
        {
            IfFailGo(AddSymbol(debugSite,
                (*this)->GetDynamicInterpreterEntryPoint(),
                (*this)->GetDynamicInterpreterThunkSize(),
                name, url, line, column));
        }


        // Load jit symbols
        // Need to static_cast here because the entry points are a write barriered field and Map is unable to deduce that we want it to be cast to
        IfFailGo(Map(debugSite, static_cast<Js::FunctionEntryPointList*>((*this)->entryPoints), [&](int index, const RecyclerWeakReference<FunctionEntryPointInfo>* pEntryPointWeakRef)
        {
            RemoteWeakReference<FunctionEntryPointInfo> remoteWeakRef;

            if (SUCCEEDED(remoteWeakRef.Read(debugSite, pEntryPointWeakRef)) && remoteWeakRef.Get())
            {
                RemoteData<FunctionEntryPointInfo> entryPoint;
                if (SUCCEEDED(entryPoint.Read(debugSite, remoteWeakRef.Get()))
                    && entryPoint->IsCodeGenDone())
                {
                    AddSymbol(debugSite,
                        (void *)entryPoint->GetNativeAddress(),
                        static_cast<ULONG>(entryPoint->GetCodeSize()),
                        name, url, line, column);
                }
            }
        }));

        // Load jit loop body symbols
        Js::LoopHeader* loopHeaderArray = static_cast<Js::LoopHeader*>(this->GetAuxPtrs(debugSite, FunctionProxy::AuxPointerType::LoopHeaderArray));
        uint loopCount = this->GetCounter(debugSite, FunctionBody::CounterFields::LoopCount);
        IfFailGo(Map(debugSite, loopHeaderArray, loopCount, [&](int loopNumber, const LoopHeader& header)
        {
            header.MapEntryPoints(debugSite, [&](int, const LoopEntryPointInfo* pEntryPointInfo)
            {
                RemoteData<LoopEntryPointInfo> entryPoint;
                if (SUCCEEDED(entryPoint.Read(debugSite, pEntryPointInfo))
                    && entryPoint->IsCodeGenDone())
                {
                    WCHAR loopBodyName[MAX_SYMBOL_NAME];
                    swprintf_s(loopBodyName, _u("%s Loop%d (%s:%d,%d)"), name, loopNumber, url, line, column);
                    AddSymbol(debugSite,
                        (void *)entryPoint->GetNativeAddress(),
                        static_cast<ULONG>(entryPoint->GetCodeSize()),
                        loopBodyName);
                }
            });
        }));
Error:
        return hr;
    }

    //
    // Get a function name for tagging a funciton entry point.
    //
    HRESULT RemoteFunctionBody::GetFunctionBodyInfo(
        IScriptDebugSite* debugSite,
        _Out_writes_z_(nameBufferSize) LPWSTR nameBuffer, ULONG nameBufferSize,
        _Out_writes_z_(urlBufferSize) LPWSTR urlBuffer, ULONG urlBufferSize,
        ULONG* line, ULONG* column) const
    {
        HRESULT hr = S_OK;
        nameBuffer[0] = 0;
        urlBuffer[0] = 0;
        *line = 0;
        *column = 0;

        RemoteData<Js::Utf8SourceInfo> utf8SourceInfo;

        LPCWSTR displayNamePtr = (*this)->m_displayName;

        LPCWSTR displayName = nameBuffer;
        LPCWSTR name = displayName; // Default to displayName
        IfFailGo(debugSite->ReadString(displayNamePtr, nameBuffer, nameBufferSize));

        Js::Utf8SourceInfo* sourceInfoPtr = (*this)->m_utf8SourceInfo;

        IfFailGo(utf8SourceInfo.Read(debugSite, sourceInfoPtr));
        const SRCINFO* srcInfoPtr = utf8SourceInfo->GetSrcInfo();

        if (srcInfoPtr)
        {
            RemoteData<SRCINFO> srcInfo;
            IfFailGo(srcInfo.Read(debugSite, srcInfoPtr));

            if (srcInfo->sourceContextInfo)
            {
                RemoteData<SourceContextInfo> sourceContextInfo;
                IfFailGo(sourceContextInfo.Read(debugSite, srcInfo->sourceContextInfo));

                // Get function name, same logic as FunctionBody::GetExternalDisplayName
                bool isDynamicScript = sourceContextInfo->IsDynamic();
                bool isGlobalFunction = (*this)->m_isGlobalFunc;

                name = GetExternalDisplayName(displayName, isDynamicScript, isGlobalFunction);

                // Get source name
                LPCWSTR sourceName = (*this)->GetSourceName(sourceContextInfo);
                if (isDynamicScript) // local constant string
                {
                    wcscpy_s(urlBuffer, urlBufferSize, sourceName);
                }
                else if (sourceName != nullptr) // remote url
                {
                    debugSite->ReadString(sourceName, urlBuffer, urlBufferSize);
                }

                *line = srcInfo->dlnHost + (*this)->m_lineNumber + 1;
                *column = ((*this)->m_lineNumber == 0 ? srcInfo->ulColumnHost : 0)
                    + (*this)->m_columnNumber + 1;
            }
        }

        if (name != displayName)
        {
            wcscpy_s(nameBuffer, nameBufferSize, name);
        }
Error:
        return hr;
    }

    void* RemoteFunctionBody::GetAuxPtrs(IScriptDebugSite* debugSite, FunctionProxy::AuxPointerType pointerType) const
    {
        void* auxPtrsRaw = static_cast<void*>((*this)->auxPtrs);
        uint8* auxPtrs = static_cast<uint8*>(auxPtrsRaw);
        uint8 pointerTypeValue = static_cast<uint8>(pointerType);
        ulong bytesRead;
        HRESULT hr;
        void* ret = nullptr;
        if (auxPtrs != nullptr)
        {
            uint8 count = 0;
            IFFAILGO(debugSite->ReadVirtual(auxPtrs, &count, sizeof(count), &bytesRead));
            if (count == FunctionProxy::AuxPtrsT::AuxPtrs16::MaxCount)
            {
                for (uint8 i = 0; i < count; i++)
                {
                    uint8 type = 0;
                    IFFAILGO(debugSite->ReadVirtual(auxPtrs + offsetof(FunctionProxy::AuxPtrsT::AuxPtrs16, type) + i, &type, sizeof(type), &bytesRead));
                    if (type == pointerTypeValue)
                    {
                        IFFAILGO(debugSite->ReadVirtual(auxPtrs + offsetof(FunctionProxy::AuxPtrsT::AuxPtrs16, ptr) + sizeof(void*) * i, &ret, sizeof(ret), &bytesRead));
                        return ret;
                    }
                }
            }
            else if (count == FunctionProxy::AuxPtrsT::AuxPtrs32::MaxCount)
            {
                for (uint8 i = 0; i < count; i++)
                {
                    uint8 type = 0;
                    IFFAILGO(debugSite->ReadVirtual(auxPtrs + offsetof(FunctionProxy::AuxPtrsT::AuxPtrs32, type) + i, &type, sizeof(type), &bytesRead));
                    if (type == pointerTypeValue)
                    {
                        IFFAILGO(debugSite->ReadVirtual(auxPtrs + offsetof(FunctionProxy::AuxPtrsT::AuxPtrs32, ptr) + sizeof(void*) * i, &ret, sizeof(ret), &bytesRead));
                        return ret;
                    }
                }
            }
            else if (count > FunctionProxy::AuxPtrsT::AuxPtrs32::MaxCount)
            {
                uint8 offset = 0;
                IFFAILGO(debugSite->ReadVirtual(auxPtrs + offsetof(FunctionProxy::AuxPtrsT, offsets) + pointerTypeValue, &offset, sizeof(offset), &bytesRead));
                if (offset != (uint8)FunctionProxy::AuxPointerType::Invalid)
                {
                    IFFAILGO(debugSite->ReadVirtual(auxPtrs + offsetof(FunctionProxy::AuxPtrsT, ptrs) + sizeof(void*) * offset, &ret, sizeof(ret), &bytesRead));
                    return ret;
                }
            }
        }
    LReturn:
        return ret;
    }

    uint RemoteFunctionBody::GetCounter(IScriptDebugSite* debugSite, FunctionBody::CounterFields fieldEnum) const
    {

        // for registers, it's using UINT32_MAX to represent NoRegister
        if ((fieldEnum == FunctionBody::CounterFields::LocalClosureRegister && !(*this)->m_hasLocalClosureRegister)
            || (fieldEnum == FunctionBody::CounterFields::LocalFrameDisplayRegister && !(*this)->m_hasLocalFrameDisplayRegister)
            || (fieldEnum == FunctionBody::CounterFields::EnvRegister && !(*this)->m_hasEnvRegister)
            || (fieldEnum == FunctionBody::CounterFields::ThisRegisterForEventHandler && !(*this)->m_hasThisRegisterForEventHandler)
            || (fieldEnum == FunctionBody::CounterFields::FirstInnerScopeRegister && !(*this)->m_hasFirstInnerScopeRegister)
            || (fieldEnum == FunctionBody::CounterFields::FuncExprScopeRegister && !(*this)->m_hasFuncExprScopeRegister)
            || (fieldEnum == FunctionBody::CounterFields::FirstTmpRegister && !(*this)->m_hasFirstTmpRegister)
            )
        {
            return Constants::NoRegister;
        }

        uint8 fieldEnumVal = static_cast<uint8>(fieldEnum);
        const auto& remoteCounters = (*this)->counters;
        uint8 fieldSize = remoteCounters.GetFieldSize();
        ulong bytesRead;
        HRESULT hr;
        if (fieldSize == 1)
        {
            uint8 result;
            IFFAILGO(debugSite->ReadVirtual(&remoteCounters.GetFields()->u8Fields[fieldEnumVal], &result, sizeof(result), &bytesRead));
            return result;
        }
        else if (fieldSize == 2)
        {
            uint16 result;
            IFFAILGO(debugSite->ReadVirtual(&remoteCounters.GetFields()->u16Fields[fieldEnumVal], &result, sizeof(result), &bytesRead));
            return result;
        }
        else if (fieldSize == 4)
        {
            uint32 result;
            IFFAILGO(debugSite->ReadVirtual(&remoteCounters.GetFields()->u32Fields[fieldEnumVal], &result, sizeof(result), &bytesRead));
            return result;
        }
        else if (fieldSize == 0) // in case OOM while initializing
        {
            return 0;
        }
        else
        {
            Assert(false);
            return 0;
        }
    LReturn:
        Assert(false);
        return 0;
    }

    HRESULT RemoteFunctionBody::AddSymbol(
        IScriptDebugSite* debugSite, LPCVOID addr, ULONG size, LPCWSTR name,
        LPCWSTR url, ULONG line, ULONG column)
    {
        WCHAR fullName[MAX_SYMBOL_NAME];
        if (url && *url)
        {
            swprintf_s(fullName, _u("%s (%s:%d,%d)"), name, url, line, column);
            name = fullName;
        }

        // On ARM the least significant bit indicates thumb mode. Ignore it here.
#ifdef _M_ARM
        addr = (LPCVOID)((UINT_PTR)addr & (((UINT_PTR)-1) - 1));
#endif

        return debugSite->AddSyntheticSymbol(addr, size, name);
    }
}
