// Copyright (C) Microsoft. All rights reserved.

#include "StdAfx.h"

namespace Js
{

#ifdef FIELD_ACCESS_STATS
    void FieldAccessStats::Add(FieldAccessStats* other)
    {
        Assert(other != null);
        this->totalInlineCacheCount += other->totalInlineCacheCount;
        this->noInfoInlineCacheCount += other->noInfoInlineCacheCount;
        this->monoInlineCacheCount += other->monoInlineCacheCount;
        this->emptyMonoInlineCacheCount += other->emptyMonoInlineCacheCount;
        this->polyInlineCacheCount += other->polyInlineCacheCount;
        this->nullPolyInlineCacheCount += other->nullPolyInlineCacheCount;
        this->emptyPolyInlineCacheCount += other->emptyPolyInlineCacheCount;
        this->ignoredPolyInlineCacheCount += other->ignoredPolyInlineCacheCount;
        this->highUtilPolyInlineCacheCount += other->highUtilPolyInlineCacheCount;
        this->lowUtilPolyInlineCacheCount += other->lowUtilPolyInlineCacheCount;
        this->equivPolyInlineCacheCount += other->equivPolyInlineCacheCount;
        this->nonEquivPolyInlineCacheCount += other->nonEquivPolyInlineCacheCount;
        this->disabledPolyInlineCacheCount += other->disabledPolyInlineCacheCount;
        this->clonedMonoInlineCacheCount += other->clonedMonoInlineCacheCount;
        this->clonedPolyInlineCacheCount += other->clonedPolyInlineCacheCount;
    }
#endif

    // FunctionProxy methods
    FunctionProxy::FunctionProxy(JavascriptMethod entryPoint, Attributes attributes, int nestedCount, int derivedSize, LocalFunctionId functionId, ScriptContext* scriptContext, Utf8SourceInfo* utf8SourceInfo, uint functionNumber):
        FunctionInfo(entryPoint, attributes, functionId, (FunctionBody*) this),
        m_nestedCount(nestedCount),
        m_isTopLevel(false),
        m_isPublicLibraryCode(false),
        m_derivedSize(derivedSize),
        m_scriptContext(scriptContext),
        m_utf8SourceInfo(utf8SourceInfo),
        m_referenceInParentFunction(null),
        m_functionNumber(functionNumber),
        m_defaultEntryPointInfo(null),
        m_functionObjectTypeList(null)
    {
        PERF_COUNTER_INC(Code, TotalFunction);
    }

    uint FunctionProxy::GetSourceContextId() const
    {
        return m_utf8SourceInfo->GetSrcInfo()->sourceContextInfo->sourceContextId;
    }

    wchar_t* FunctionProxy::GetDebugNumberSet(wchar(&bufferToWriteTo)[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE]) const
    {
        // (#%u.%u), #%u --> (source file Id . function Id) , function Number
        int len = swprintf_s(bufferToWriteTo, MAX_FUNCTION_BODY_DEBUG_STRING_SIZE, L" (#%d.%d), #%d",
            this->GetSourceContextId(), this->GetLocalFunctionId(), this->GetFunctionNumber());
        Assert(len > 8);
        return bufferToWriteTo;
    }

    bool
    FunctionProxy::IsFunctionBody() const
    {
        return !IsDeferredDeserializeFunction() && GetParseableFunctionInfo()->IsFunctionParsed();
    }

    void ParseableFunctionInfo::BuildDeferredStubs(ParseNode *pnodeFnc)
    {
        Assert(pnodeFnc->nop == knopFncDecl);

        Recycler *recycler = GetScriptContext()->GetRecycler();
        this->deferredStubs = ParseableFunctionInfo::BuildDeferredStubTree(pnodeFnc, recycler);
    }

    DeferredFunctionStub *ParseableFunctionInfo::BuildDeferredStubTree(ParseNode *pnodeFnc, Recycler *recycler)
    {
        uint nestedCount = pnodeFnc->sxFnc.nestedCount;
        if (nestedCount == 0)
        {
            return nullptr;
        }

        if (pnodeFnc->sxFnc.deferredStub)
        {
            return pnodeFnc->sxFnc.deferredStub;
        }

        DeferredFunctionStub *deferredStubs = RecyclerNewArray(recycler, DeferredFunctionStub, nestedCount);
        uint i = 0;

        ParseNode *pnodeBlock = pnodeFnc->sxFnc.pnodeBodyScope;
        Assert(pnodeBlock != nullptr
               && pnodeBlock->nop == knopBlock
               && (pnodeBlock->sxBlock.blockType == PnodeBlockType::Function 
                   || pnodeBlock->sxBlock.blockType == PnodeBlockType::Parameter));

        for (ParseNode *pnodeChild = pnodeBlock->sxBlock.pnodeScopes; pnodeChild != nullptr;)
        {

            if (pnodeChild->nop != knopFncDecl)
            {
                // We only expect to find a function body block in a parameter scope block.
                Assert(pnodeChild->nop == knopBlock
                       && (pnodeBlock->sxBlock.blockType == PnodeBlockType::Parameter
                          || pnodeChild->sxBlock.blockType == PnodeBlockType::Function));
                pnodeChild = pnodeChild->sxBlock.pnodeNext;
                continue;
            }
            Assert(i < nestedCount);

            if (pnodeChild->sxFnc.IsGeneratedDefault())
            {
                ++i;
                pnodeChild = pnodeChild->sxFnc.pnodeNext;
                continue;
            }

            __analysis_assume(i < nestedCount);

            deferredStubs[i].fncFlags = pnodeChild->sxFnc.fncFlags;
            deferredStubs[i].nestedCount = pnodeChild->sxFnc.nestedCount;
            deferredStubs[i].restorePoint = *pnodeChild->sxFnc.pRestorePoint;
            deferredStubs[i].deferredStubs = ParseableFunctionInfo::BuildDeferredStubTree(pnodeChild, recycler);
#if DEBUG
            deferredStubs[i].ichMin = pnodeChild->ichMin;
#endif
            ++i;
            pnodeChild = pnodeChild->sxFnc.pnodeNext;
        }

        return deferredStubs;
    }

    FunctionProxyArray ParseableFunctionInfo::GetNestedFuncArray()
    {
        // The array is allocated as extra bytes past the end of the struct.
        Assert(this->m_nestedCount > 0);

        return (FunctionProxyArray )((char*)this + m_derivedSize);
    }

    void ParseableFunctionInfo::SetNestedFunc(FunctionProxy* nestedFunc, uint index, ulong flags)
    {
        AssertMsg(index < this->m_nestedCount, "Trying to write past the nested func array");

        FunctionProxyArray nested = this->GetNestedFuncArray();
        nested[index] = nestedFunc;

        if (nestedFunc)
        {
            nestedFunc->SetReferenceInParentFunction(GetNestedFuncReference(index));

            if (!this->GetSourceContextInfo()->IsDynamic() && nestedFunc->IsDeferredParseFunction() && nestedFunc->GetParseableFunctionInfo()->GetIsDeclaration() && this->GetIsTopLevel() && !(flags & fscrEvalCode))
            {
                this->m_utf8SourceInfo->TrackDeferredFunction(nestedFunc->GetLocalFunctionId(), nestedFunc->GetParseableFunctionInfo());
            }
        }

    }

    FunctionProxy* ParseableFunctionInfo::GetNestedFunc(uint index)
    {
        return *(GetNestedFuncReference(index));
    }

    FunctionProxyPtrPtr ParseableFunctionInfo::GetNestedFuncReference(uint index)
    {
        AssertMsg(index < this->m_nestedCount, "Trying to write past the nested func array");

        FunctionProxyArray nested = this->GetNestedFuncArray();
        return &nested[index];
    }

    ParseableFunctionInfo* ParseableFunctionInfo::GetNestedFunctionForExecution(uint index)
    {
        FunctionProxy* currentNestedFunction = this->GetNestedFunc(index);
        if (currentNestedFunction && currentNestedFunction->IsDeferredDeserializeFunction())
        {
            currentNestedFunction = currentNestedFunction->EnsureDeserialized();
            this->SetNestedFunc(currentNestedFunction, index, 0u);
        }

        return currentNestedFunction->GetParseableFunctionInfo();
    }

    void
    FunctionProxy::UpdateFunctionBodyImpl(FunctionBody * body)
    {
        Assert(functionBodyImpl == ((FunctionProxy*) this));
        Assert(!this->IsFunctionBody() || body == this);
        this->functionBodyImpl = body;
        this->attributes = (Attributes)(this->attributes & ~(DeferredParse | DeferredDeserialize));
        this->UpdateReferenceInParentFunction(body);
    }

    void ParseableFunctionInfo::ClearNestedFunctionParentFunctionReference()
    {
        if (this->m_nestedCount > 0)
        {
            // If the function is x-domain all the nested functions should also be marked as x-domain
            FunctionProxyArray nested = this->GetNestedFuncArray();
            for (uint i = 0; i < this->m_nestedCount; ++i)
            {
                if (nested[i])
                {
                    nested[i]->SetReferenceInParentFunction(null);
                }
            }
        }
    }

    //
    // This method gets a function body for the purposes of execution
    // It has an if within it to avoid making it a virtual- it's called from the interpreter
    // It will cause the function info to get deserialized if it hasn't been deserialized
    // already
    //
    ParseableFunctionInfo * FunctionProxy::EnsureDeserialized()
    {
        FunctionProxy * executionFunctionBody = this->functionBodyImpl;

        if (executionFunctionBody == this && IsDeferredDeserializeFunction())
        {
            // No need to deserialize function body if scriptContext closed because we can't execute it.
            // Bigger problem is the script engine might have released bytecode file mapping and we can't deserialize.
            Assert(!m_scriptContext->IsClosed());

            executionFunctionBody = ((DeferDeserializeFunctionInfo*) this)->Deserialize();
            this->functionBodyImpl = executionFunctionBody;
            Assert(executionFunctionBody->HasBody());
            Assert(executionFunctionBody != this);
        }

        return (ParseableFunctionInfo *)executionFunctionBody;
    }

    ScriptFunctionType * FunctionProxy::GetDeferredPrototypeType() const
    {
        return deferredPrototypeType;
    }

    ScriptFunctionType * FunctionProxy::EnsureDeferredPrototypeType()
    {
        Assert(this->GetFunctionProxy() == this);
        return (deferredPrototypeType != null)? deferredPrototypeType : AllocDeferredPrototypeType();
    }

    ScriptFunctionType * FunctionProxy::AllocDeferredPrototypeType()
    {
        Assert(deferredPrototypeType == null);
        ScriptFunctionType * type = ScriptFunctionType::New(this, true);
        deferredPrototypeType = type;
        return type;
    }

    JavascriptMethod FunctionProxy::GetDirectEntryPoint(ProxyEntryPointInfo* entryPoint) const
    {
        Assert((JavascriptMethod)entryPoint->address != NULL);
        return (JavascriptMethod)entryPoint->address;
    }

    // Function object type list methods
    template <typename Fn>
    void FunctionProxy::MapFunctionObjectTypes(Fn func)
    {
        // Can't call this during sweep since the weak references are resolved
        // using FastGet. If sweep needs to be supported, switch to using Get

        if (m_functionObjectTypeList)
        {
            m_functionObjectTypeList->Map([&] (int, FunctionTypeWeakRef* typeWeakRef)
            {
                if (typeWeakRef)
                {
                    DynamicType* type = typeWeakRef->Get();
                    if (type)
                    {
                        func(type);
                    }
                }
            });
        }

        if (this->deferredPrototypeType)
        {
            func(this->deferredPrototypeType);
        }
    }

    FunctionProxy::FunctionTypeWeakRefList* FunctionProxy::EnsureFunctionObjectTypeList()
    {
        if (m_functionObjectTypeList == null)
        {
            Recycler* recycler = this->GetScriptContext()->GetRecycler();
            m_functionObjectTypeList = RecyclerNew(recycler, FunctionTypeWeakRefList, recycler);
        }

        return m_functionObjectTypeList;
    }

    void FunctionProxy::RegisterFunctionObjectType(DynamicType* functionType)
    {
        FunctionTypeWeakRefList* typeList = EnsureFunctionObjectTypeList();

        Assert(functionType != deferredPrototypeType);
        Recycler * recycler = this->GetScriptContext()->GetRecycler();
        FunctionTypeWeakRef* weakRef = recycler->CreateWeakReferenceHandle(functionType);
        typeList->SetAtFirstFreeSpot(weakRef);
        OUTPUT_TRACE(Js::ExpirableCollectPhase, L"Registered type 0x%p on function body %p, count = %d\n", functionType, this, typeList->Count());
    }

    void DeferDeserializeFunctionInfo::SetDisplayName(const wchar_t* displayName)
    {
        SetDisplayName(displayName, wcslen(displayName));
    }

    void DeferDeserializeFunctionInfo::SetDisplayName(const wchar_t* pszDisplayName, uint displayNameLength, SetDisplayNameFlags flags /* default to None */)
    {
        this->m_displayNameLength = displayNameLength;
        FunctionProxy::SetDisplayName(pszDisplayName, &this->m_displayName, displayNameLength, m_scriptContext, flags);
    }

    LPCWSTR DeferDeserializeFunctionInfo::GetSourceInfo(int& lineNumber, int& columnNumber) const
    {
        // Read all the necessary information from the serialized byte code
        int lineNumberField, columnNumberField;
        bool m_isEval, m_isDynamicFunction;
        ByteCodeSerializer::ReadSourceInfo(this, lineNumberField, columnNumberField, m_isEval, m_isDynamicFunction);

        // Decode them
        lineNumber = ComputeAbsoluteLineNumber(lineNumberField);
        columnNumber = ComputeAbsoluteColumnNumber(lineNumberField, columnNumberField);
        return Js::ParseableFunctionInfo::GetSourceName<SourceContextInfo*>(this->GetSourceContextInfo(), m_isEval, m_isDynamicFunction);
    }

    void DeferDeserializeFunctionInfo::Finalize(bool isShutdown)
    {
        __super::Finalize(isShutdown);
        PERF_COUNTER_DEC(Code, DeferDeserializeFunctionProxy);
    }

    FunctionBody* DeferDeserializeFunctionInfo::Deserialize()
    {
        if (functionBodyImpl == (FunctionBody*) this)
        {
            FunctionBody * body = ByteCodeSerializer::DeserializeFunction(this->m_scriptContext, this);
            this->Copy(body);
            this->UpdateFunctionBodyImpl(body);
        }

        return GetFunctionBody();
    }

    //
    // hrParse can be one of the following from deferred re-parse (check CompileScriptException::ProcessError):
    //      E_OUTOFMEMORY
    //      E_UNEXPECTED
    //      SCRIPT_E_RECORDED,
    //          with ei.scode: ERRnoMemory, VBSERR_OutOfStack, E_OUTOFMEMORY, E_FAIL
    //          Any other ei.scode shouldn't appear in deferred re-parse.
    //
    // Map errors like OOM/OOS, return it and clean hrParse. Any other error remaining in hrParse is an internal error.
    //
    HRESULT ParseableFunctionInfo::MapDeferredReparseError(HRESULT& hrParse, const CompileScriptException& se)
    {
        HRESULT hrMapped = NO_ERROR;

        switch (hrParse)
        {
        case E_OUTOFMEMORY:
            hrMapped = E_OUTOFMEMORY;
            break;

        case SCRIPT_E_RECORDED:
            switch (se.ei.scode)
            {
            case ERRnoMemory:
            case E_OUTOFMEMORY:
            case VBSERR_OutOfMemory:
                hrMapped = E_OUTOFMEMORY;
                break;

            case VBSERR_OutOfStack:
                hrMapped = VBSERR_OutOfStack;
                break;
            }
        }

        if (FAILED(hrMapped))
        {
            // If we have mapped error, clear hrParse. We'll throw error from hrMapped.
            hrParse = NO_ERROR;
        }

        return hrMapped;
    }

    FunctionBody* ParseableFunctionInfo::Parse(ScriptFunction ** functionRef, bool isByteCodeDeserialization)
    {
        if ((functionBodyImpl != (FunctionBody*) this) || !IsDeferredParseFunction())
        {
            // If not deferredparsed, the functionBodyImpl and this will be the same, just return the current functionBody.
            Assert(GetFunctionBody()->IsFunctionParsed());
            return GetFunctionBody();
        }

        BOOL fParsed = FALSE;
        FunctionBody* returnFunctionBody = nullptr;
        ENTER_PINNED_SCOPE(Js::PropertyRecordList, propertyRecordList);
        Recycler* recycler = this->m_scriptContext->GetRecycler();
        propertyRecordList = RecyclerNew(recycler, Js::PropertyRecordList, recycler);

        bool isDebugReparse = !BinaryFeatureControl::LanguageService() && m_scriptContext->IsInDebugOrSourceRundownMode();
        bool isAsmJsReparse = false;

        FunctionBody* funcBody = NULL;

        // If m_hasBeenParsed = true, one of the following things happened things happened:
        // - We had multiple function objects which were all defer-parsed, but with the same function body and one of them
        //   got the body to be parsed before another was called
        // - We are in debug mode and had our thunks switched to DeferParseThunk
        // - This is an already parsed asm.js module, which has been invalidated at link time and must be reparsed as a non-asm.js function
        if (!this->m_hasBeenParsed)
        {
            funcBody = FunctionBody::NewFromRecycler(
                this->m_scriptContext,
                this->m_displayName,
                this->m_displayNameLength,
                this->m_nestedCount,
                this->m_utf8SourceInfo,
                this->m_functionNumber,
                m_utf8SourceInfo->GetSrcInfo()->sourceContextInfo->sourceContextId, /* script id */
                this->functionId, /* function id */
                propertyRecordList,
                (Attributes)(this->GetAttributes() & ~(Attributes::DeferredDeserialize | Attributes::DeferredParse))
#ifdef PERF_COUNTERS
                , false /* is function from deferred deserialized proxy */
#endif
                );

            this->Copy(funcBody);
            PERF_COUNTER_DEC(Code, DeferedFunction);

            if (!this->GetSourceContextInfo()->IsDynamic())
            {
                PHASE_PRINT_TESTTRACE1(Js::DeferParsePhase, L"TestTrace: Deferred function parsed - ID: %d; Display Name: %s; Length: %d; Nested Function Count: %d; Utf8SourceInfo: %d; Source Length: %d; Is Top Level: %s; Source Url: %s\n", m_functionNumber, m_displayName, this->m_cchLength, this->GetNestedCount(), this->m_utf8SourceInfo->GetSourceInfoId(), this->m_utf8SourceInfo->GetCchLength(), this->GetIsTopLevel() ? L"True" : L"False", this->GetSourceContextInfo()->url);
            }
            else
            {
                PHASE_PRINT_TESTTRACE1(Js::DeferParsePhase, L"TestTrace: Deferred function parsed - ID: %d; Display Name: %s; Length: %d; Nested Function Count: %d; Utf8SourceInfo: %d; Source Length: %d\n; Is Top Level: %s;", m_functionNumber, m_displayName, this->m_cchLength, this->GetNestedCount(),  this->m_utf8SourceInfo->GetSourceInfoId(), this->m_utf8SourceInfo->GetCchLength(), this->GetIsTopLevel() ? L"True" : L"False");
            }

            if (CONFIG_FLAG(DeferTopLevelTillFirstCall) && !this->GetIsTopLevel() && !this->GetSourceContextInfo()->IsDynamic())
            {
                this->m_utf8SourceInfo->UndeferGlobalFunctions([this](JsUtil::SimpleDictionaryEntry<Js::LocalFunctionId, Js::ParseableFunctionInfo*> func)
                {
                    Js::ParseableFunctionInfo *nextFunc = func.Value();
                    JavascriptExceptionObject* pExceptionObject = nullptr;

                    if (nextFunc != nullptr && this != nextFunc)
                    {
                        try
                        {
                            nextFunc->Parse();
                        }
                        catch (OutOfMemoryException) {}
                        catch (StackOverflowException) {}
                        catch (JavascriptExceptionObject* exceptionObject)
                        {
                            pExceptionObject = exceptionObject;
                        }

                        // Do not do anything with an OOM or SOE exception, returning true is fine, it will then be undeferred (or attempted to again when called)
                        if(pExceptionObject)
                        {
                            if(pExceptionObject != ThreadContext::GetContextForCurrentThread()->GetPendingOOMErrorObject() &&
                                pExceptionObject != ThreadContext::GetContextForCurrentThread()->GetPendingSOErrorObject())
                            {
                                throw pExceptionObject;
                            }
                        }
                    }

                    return true;
                });
            }
        }
        else
        {
            isAsmJsReparse = m_isAsmjsMode && !isDebugReparse;
            funcBody = this->GetFunctionBody();

            if (isDebugReparse || isAsmJsReparse)
            {
    #if ENABLE_DEBUG_CONFIG_OPTIONS
                wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];
    #endif
    #if DBG
                Assert(
                    funcBody->IsReparsed()
                    || m_scriptContext->GetThreadContext()->Diagnostics->languageServiceEnabled
                    || m_scriptContext->IsInDebugOrSourceRundownMode()
                    || m_isAsmjsMode);
    #endif
                OUTPUT_TRACE(Js::DebuggerPhase, L"Full nested reparse of function: %s (%s)\n", funcBody->GetDisplayName(), funcBody->GetDebugNumberSet(debugStringBuffer));

                if (funcBody->GetByteCode())
                {
                    // The current function needs to be cleaned up before getting generated in the debug mode.
                    funcBody->CleanupToReparse();
                }

            }
        }

        // Note that we may be trying to re-gen an already-completed function. (This can happen, for instance,
        // in the case of named function expressions inside "with" statements in compat mode.)
        // In such a case, there's no work to do.
        if (funcBody->GetByteCode() == NULL)
        {
            Assert(!funcBody->HasExecutionDynamicProfileInfo());

            // In debug mode, the eval code will be asked to recompile again.
            AssertMsg(isDebugReparse || !(funcBody->GetGrfscr() & (fscrImplicitThis | fscrImplicitParents)),
                        "Deferred parsing of event handler body?");

            // In debug or asm.js mode, the scriptlet will be asked to recompile again.
            AssertMsg(isAsmJsReparse || isDebugReparse || funcBody->GetGrfscr() & fscrGlobalCode || CONFIG_FLAG(DeferNested), "Deferred parsing of non-global procedure?");

            HRESULT hr = NO_ERROR;
            HRESULT hrParser = NO_ERROR;
            HRESULT hrParseCodeGen = NO_ERROR;

            BEGIN_LEAVE_SCRIPT_INTERNAL(m_scriptContext)
            {
                bool isCesu8 = m_scriptContext->GetSource(funcBody->GetSourceIndex())->IsCesu8();

                size_t offset = this->StartOffset();
                charcount_t charOffset = this->StartInDocument();
                size_t length = this->LengthInBytes();

                LPCUTF8 pszStart = this->GetStartOfDocument();

                ulong grfscr = funcBody->GetGrfscr() | fscrDeferredFnc;

                // For the global function we want to re-use the glo functionbody which is already created in the non-debug mode
                if (!funcBody->GetIsGlobalFunc())
                {
                    grfscr &= ~fscrGlobalCode;
                }

                if (!funcBody->GetIsDeclaration() && !funcBody->GetIsGlobalFunc()) // No refresh may reparse global function (e.g. eval code)
                {
                    // Notify the parser that the top-level function was defined in an expression,
                    // (not a function declaration statement).
                    grfscr |= fscrDeferredFncExpression;
                }
                if (!CONFIG_FLAG(DeferNested) || isDebugReparse || isAsmJsReparse)
                {
                    grfscr &= ~fscrDeferFncParse; // Disable deferred parsing if not DeferNested, or doing a debug/asm.js re-parse
                }

                // TODO: is ETW tracing possible/necessary here?

                BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT
                {
                    CompileScriptException se;
                    Parser ps(m_scriptContext, funcBody->GetIsStrictMode() ? TRUE : FALSE);
                    ParseNodePtr parseTree;

                    uint nextFunctionId = funcBody->GetLocalFunctionId();
                    hrParser = ps.ParseSourceWithOffset(&parseTree, pszStart, offset, length, charOffset, isCesu8, grfscr, &se,
                        &nextFunctionId, funcBody->GetRelativeLineNumber(), funcBody->GetSourceContextInfo(),
                        funcBody, (isDebugReparse || isAsmJsReparse), isAsmJsReparse);
                    Assert(FAILED(hrParser) || nextFunctionId == funcBody->deferredParseNextFunctionId || isDebugReparse || isAsmJsReparse || isByteCodeDeserialization);

                    if (FAILED(hrParser))
                    {
                        hrParseCodeGen = MapDeferredReparseError(hrParser, se); // Map certain errors like OOM/OOS
                        AssertMsg(FAILED(hrParseCodeGen) && SUCCEEDED(hrParser), "Syntax errors should never be detected on deferred re-parse");
                    }
                    else
                    {
                        TRACE_BYTECODE(L"\nDeferred parse %s\n", funcBody->GetDisplayName());
                        Js::AutoDynamicCodeReference dynamicFunctionReference(m_scriptContext);

                        bool forceNoNative = (isDebugReparse || isAsmJsReparse) ? this->GetScriptContext()->IsInterpreted() : false;
                        hrParseCodeGen = GenerateByteCode(parseTree, grfscr, m_scriptContext,
                            funcBody->GetParseableFunctionInfoRef(), funcBody->GetSourceIndex(),
                            forceNoNative, &ps, &se, funcBody->GetScopeInfo(), functionRef);

                        if (SUCCEEDED(hrParseCodeGen))
                        {
                            fParsed = TRUE;
                        }
                        else
                        {
                            Assert(hrParseCodeGen == SCRIPT_E_RECORDED);
                            hrParseCodeGen = se.ei.scode;
                        }
                    }
                }
                END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);
            }
            END_LEAVE_SCRIPT_INTERNAL(m_scriptContext);

            if (hr == E_OUTOFMEMORY)
            {
                JavascriptError::ThrowOutOfMemoryError(m_scriptContext);
            }
            else if(hr == VBSERR_OutOfStack)
            {
                JavascriptError::ThrowStackOverflowError(m_scriptContext);
            }
            else if(hr == E_ABORT)
            {
                throw Js::ScriptAbortException();
            }
                else if(FAILED(hr))
            {
                throw Js::InternalErrorException();
            }

            Assert(hr == NO_ERROR);

            if (!SUCCEEDED(hrParser))
            {
                JavascriptError::ThrowError(m_scriptContext, VBSERR_InternalError);
            }
            else if (!SUCCEEDED(hrParseCodeGen))
            {
                /*
                    * VBSERR_OutOfStack is of type kjstError but we throw a (more specific) StackOverflowError when a hard stack
                    * overflow occurs. To keep the behavior consistent I'm special casing it here.
                    */
                if (hrParseCodeGen == VBSERR_OutOfStack)
                {
                    JavascriptError::ThrowStackOverflowError(m_scriptContext);
                }
                JavascriptError::MapAndThrowError(m_scriptContext, hrParseCodeGen);
            }
        }
        else
        {
            fParsed = FALSE;
        }


        if (BinaryFeatureControl::LanguageService())
        {
            // Notify that langauge service that deferred parsing is complete and the
            // deferred parsed function is now executing.
            if (m_scriptContext->authoringData && m_scriptContext->authoringData->Callbacks())
                m_scriptContext->authoringData->Callbacks()->Executing();
        }

        if (fParsed == TRUE)
        {
            this->UpdateFunctionBodyImpl(funcBody);
            this->m_hasBeenParsed = true;
        }
        
        returnFunctionBody = GetFunctionBody();

        LEAVE_PINNED_SCOPE();

        return returnFunctionBody;
    }

    FunctionBody* ParseableFunctionInfo::ParseAsmJs(Parser * ps, __out CompileScriptException * se, __out ParseNodePtr * parseTree)
    {
        Assert(IsDeferredParseFunction());
        Assert(m_isAsmjsMode);

        FunctionBody* returnFunctionBody = nullptr;
        ENTER_PINNED_SCOPE(Js::PropertyRecordList, propertyRecordList);
        Recycler* recycler = this->m_scriptContext->GetRecycler();
        propertyRecordList = RecyclerNew(recycler, Js::PropertyRecordList, recycler);

        FunctionBody* funcBody = NULL;

        funcBody = FunctionBody::NewFromRecycler(
            this->m_scriptContext,
            this->m_displayName,
            this->m_displayNameLength,
            this->m_nestedCount,
            this->m_utf8SourceInfo,
            this->m_functionNumber,
            m_utf8SourceInfo->GetSrcInfo()->sourceContextInfo->sourceContextId, /* script id */
            this->functionId, /* function id */
            propertyRecordList,
            (Attributes)(this->GetAttributes() & ~(Attributes::DeferredDeserialize | Attributes::DeferredParse))
#ifdef PERF_COUNTERS
            , false /* is function from deferred deserialized proxy */
#endif            
            );

        this->Copy(funcBody);
        PERF_COUNTER_DEC(Code, DeferedFunction);

        if (!this->GetSourceContextInfo()->IsDynamic())
        {
            PHASE_PRINT_TESTTRACE1(Js::DeferParsePhase, L"TestTrace: Deferred function parsed - ID: %d; Display Name: %s; Length: %d; Nested Function Count: %d; Utf8SourceInfo: %d; Source Length: %d; Is Top Level: %s; Source Url: %s\n", m_functionNumber, m_displayName, this->m_cchLength, this->GetNestedCount(), this->m_utf8SourceInfo->GetSourceInfoId(), this->m_utf8SourceInfo->GetCchLength(), this->GetIsTopLevel() ? L"True" : L"False", this->GetSourceContextInfo()->url);
        }
        else
        {
            PHASE_PRINT_TESTTRACE1(Js::DeferParsePhase, L"TestTrace: Deferred function parsed - ID: %d; Display Name: %s; Length: %d; Nested Function Count: %d; Utf8SourceInfo: %d; Source Length: %d\n; Is Top Level: %s;", m_functionNumber, m_displayName, this->m_cchLength, this->GetNestedCount(), this->m_utf8SourceInfo->GetSourceInfoId(), this->m_utf8SourceInfo->GetCchLength(), this->GetIsTopLevel() ? L"True" : L"False");
        }

        Assert(!funcBody->HasExecutionDynamicProfileInfo());

        HRESULT hrParser = NO_ERROR;
        HRESULT hrParseCodeGen = NO_ERROR;

        bool isCesu8 = m_scriptContext->GetSource(funcBody->GetSourceIndex())->IsCesu8();

        size_t offset = this->StartOffset();
        charcount_t charOffset = this->StartInDocument();
        size_t length = this->LengthInBytes();

        LPCUTF8 pszStart = this->GetStartOfDocument();

        ulong grfscr = funcBody->GetGrfscr() | fscrDeferredFnc | fscrDeferredFncExpression;

        uint nextFunctionId = funcBody->GetLocalFunctionId();

        // if parser throws, it will be caught by function trying to bytecode gen the asm.js module, so don't need to catch/rethrow here
        hrParser = ps->ParseSourceWithOffset(parseTree, pszStart, offset, length, charOffset, isCesu8, grfscr, se,
                    &nextFunctionId, funcBody->GetRelativeLineNumber(), funcBody->GetSourceContextInfo(),
                    funcBody, false, false);

        Assert(FAILED(hrParser) || funcBody->deferredParseNextFunctionId == nextFunctionId);
        if (FAILED(hrParser))
        {
            hrParseCodeGen = MapDeferredReparseError(hrParser, *se); // Map certain errors like OOM/OOS
            AssertMsg(FAILED(hrParseCodeGen) && SUCCEEDED(hrParser), "Syntax errors should never be detected on deferred re-parse");
        }

        if (!SUCCEEDED(hrParser))
        {
            JavascriptError::ThrowError(m_scriptContext, VBSERR_InternalError);
        }
        else if (!SUCCEEDED(hrParseCodeGen))
        {
            // special casing VBSERR_OutOfStack as per Parse method above
            if (hrParseCodeGen == VBSERR_OutOfStack)
            {
                JavascriptError::ThrowStackOverflowError(m_scriptContext);
            }
            JavascriptError::MapAndThrowError(m_scriptContext, hrParseCodeGen);
        }

        UpdateFunctionBodyImpl(funcBody);
        m_hasBeenParsed = true;

        returnFunctionBody = GetFunctionBody();

        LEAVE_PINNED_SCOPE();

        return returnFunctionBody;
    }

    void ParseableFunctionInfo::Finalize(bool isShutdown)
    {
        __super::Finalize(isShutdown);

        if (!this->m_hasBeenParsed)
        {
            if (!this->GetSourceContextInfo()->IsDynamic()  && !this->GetIsTopLevel())
            {
                this->m_utf8SourceInfo->StopTrackingDeferredFunction(this->GetLocalFunctionId());
            }
            PERF_COUNTER_DEC(Code, DeferedFunction);
        }
    }

    bool ParseableFunctionInfo::IsFakeGlobalFunc(ulong flags) const
    {
        return GetIsGlobalFunc() && !(flags & fscrGlobalCode);
    }

    bool ParseableFunctionInfo::GetExternalDisplaySourceName(BSTR* sourceName)
    {
        Assert(sourceName);

        if (IsDynamicScript() && GetUtf8SourceInfo()->HasDocumentText())
        {
            IDebugDocumentText *documentText = static_cast<IDebugDocumentText *>(GetUtf8SourceInfo()->GetDocumentText());
            if (documentText->GetName(DOCUMENTNAMETYPE_URL, sourceName) == S_OK)
            {
                return true;
            }
        }

        *sourceName = ::SysAllocString(GetSourceName());
        return *sourceName != nullptr;
    }

    const wchar_t* FunctionProxy::WrapWithBrackets(const wchar_t* name, charcount_t sz, ScriptContext* scriptContext)
    {
        wchar_t * wrappedName = RecyclerNewArrayLeaf(scriptContext->GetRecycler(), wchar_t, sz + 3); //[]\0
        wrappedName[0] = L'[';
        wchar_t *next = wrappedName;
        js_wmemcpy_s(++next, sz, name, sz);
        wrappedName[sz + 1] = L']';
        wrappedName[sz + 2] = L'\0';
        return wrappedName;

    }

    const wchar_t* FunctionProxy::GetShortDisplayName(size_t* shortNameLength)
    {
        const wchar_t* name = this->GetDisplayName();
        size_t nameLength = this->GetDisplayNameLength();

        if (name == nullptr)
        {
            *shortNameLength = 0;
            return Constants::Empty;
        }

        if (IsConstantFunctionName(name))
        {
            *shortNameLength = nameLength;
            return name;
        }

        // TODO (update) shortening for string literal names will not work for "*[" cases
        // need a place to store short names and return them here before we do the shortening
        // (upate 2) forgive the uglyness of this function the next change will store the short name offsets
        // giving us the start and end of the buffer simplifying this function significantly.

        const wchar_t * shortName = wcsrchr(name, L'.'); // use of wcsrchr bites us when we have nulls in object names like so var o = { "\0a" : { foo : function() {} }}
        const wchar_t * shorterName = wcsrchr(name, L'[');
       

        if (shortName != nullptr)
        {
            // skip after '.'
            shortName++;
        }

        //if the period is after the "]" or shorterName is a nullptr
        const wchar_t *endingBracket = nullptr;
        if (shorterName == nullptr || (shortName > (endingBracket = wcsrchr(name, L']')) 
            && endingBracket != nullptr))
        {
            if (shortName)
            {
                *shortNameLength = nameLength - (shortName - name);
                return shortName;
            }
            *shortNameLength = nameLength;
            return name;
        }

        if (name == shorterName && endingBracket == nullptr)
        {
            *shortNameLength = nameLength;
            return name;
        }

        // this will fix any [[*] excluding '[' case. Still need a fix for [*[]
        while (name != shorterName && *(shorterName-1) == L'[')
        {
            
            shorterName--;
        }

        // skip after '['
        shorterName++;
        if (*shorterName == ']' && shorterName == name + nameLength)
        {
            *shortNameLength = 0;
            return Constants::Empty;
        }

        size_t deltaNameLength = nameLength - (shorterName - name);
        *shortNameLength = deltaNameLength - 1;
        if (shorterName[deltaNameLength - 1] == ']')
        {
            wchar_t * finalshorterName = RecyclerNewArrayLeaf(this->GetScriptContext()->GetRecycler(), wchar_t, deltaNameLength); // size of number in brackets + ] which will be used by the null terminator
            js_wmemcpy_s(finalshorterName, deltaNameLength, shorterName, deltaNameLength - 1); // we don't want the last character in shorterName
            finalshorterName[deltaNameLength - 1] = L'\0';
            
            return finalshorterName;
        }

        return shorterName; 
    }

    /*static*/
    bool FunctionProxy::IsConstantFunctionName(const wchar_t* srcName)
    {
        if (srcName == Js::Constants::GlobalFunction ||
            srcName == Js::Constants::AnonymousFunction ||
            srcName == Js::Constants::GlobalCode ||
            srcName == Js::Constants::Anonymous ||
            srcName == Js::Constants::UnknownScriptCode ||
            srcName == Js::Constants::FunctionCode)
        {
            return true;
        }
        return false;
    }

    /*static */
    /*Return value: Whether the target value is a recycler pointer or not*/
    bool FunctionProxy::SetDisplayName(const wchar_t* srcName, const wchar_t** destName, uint displayNameLength,  ScriptContext * scriptContext, SetDisplayNameFlags flags /* default to None */)
    {
        Assert(destName);
        Assert(scriptContext);

        if (srcName == nullptr)
        {
            *destName = (L"");
            return false;
        }
        else if (IsConstantFunctionName(srcName) || (flags & SetDisplayNameFlagsDontCopy) != 0)
        {
            *destName = srcName;
            return (flags & SetDisplayNameFlagsRecyclerAllocated) != 0; // Return true if array is recycler allocated
        }
        else
        {
            uint  numCharacters =  displayNameLength + 1;
            Assert((flags & SetDisplayNameFlagsDontCopy) == 0);

            *destName = RecyclerNewArrayLeaf(scriptContext->GetRecycler(), wchar_t, numCharacters);
            js_wmemcpy_s((wchar_t *)*destName, numCharacters, srcName, numCharacters);
            ((wchar_t *)(*destName))[numCharacters - 1] = L'\0';

            return true;
        }
    }

    void FunctionProxy::SetDisplayName(const wchar_t* srcName, WriteBarrierPtr<const wchar_t>* destName, uint displayNameLength, ScriptContext * scriptContext, SetDisplayNameFlags flags /* default to None */)
    {
        const wchar_t* dest = NULL;
        bool targetIsRecyclerMemory = SetDisplayName(srcName, &dest, displayNameLength, scriptContext, flags);

        if (targetIsRecyclerMemory)
        {
            *destName = dest;
        }
        else
        {
            destName->NoWriteBarrierSet(dest);
        }
    }
    void ParseableFunctionInfo::SetDisplayName(const wchar_t* pszDisplayName)
    {
        SetDisplayName(pszDisplayName, wcslen(pszDisplayName));
    }
    void ParseableFunctionInfo::SetDisplayName(const wchar_t* pszDisplayName, uint displayNameLength, SetDisplayNameFlags flags /* default to None */)
    {
        this->m_displayNameLength = displayNameLength;
        FunctionProxy::SetDisplayName(pszDisplayName, &this->m_displayName, displayNameLength, m_scriptContext, flags);
    }

    // SourceInfo methods
    FunctionBody::StatementMapList * FunctionBody::GetStatementMaps() const
    {
        return this->pStatementMaps;
    }

    /* static */ FunctionBody::StatementMap * FunctionBody::GetNextNonSubexpressionStatementMap(StatementMapList *statementMapList, int & startingAtIndex)
    {
        AssertMsg(statementMapList != null, "Must have valid statementMapList to execute");

        FunctionBody::StatementMap *map = statementMapList->Item(startingAtIndex);
        while (map->isSubexpression && startingAtIndex < statementMapList->Count() - 1)
        {
            map = statementMapList->Item(++startingAtIndex);
        }
        if (map->isSubexpression)   // Didn't find any non inner maps
        {
            return nullptr;
        }
        return map;
    }

    /* static */ FunctionBody::StatementMap * FunctionBody::GetPrevNonSubexpressionStatementMap(StatementMapList *statementMapList, int & startingAtIndex)
    {
        AssertMsg(statementMapList != null, "Must have valid statementMapList to execute");

        FunctionBody::StatementMap *map = statementMapList->Item(startingAtIndex);
        while (startingAtIndex && map->isSubexpression)
        {
            map = statementMapList->Item(--startingAtIndex);
        }
        if (map->isSubexpression)   // Didn't find any non inner maps
        {
            return nullptr;
        }
        return map;
    }
    void ParseableFunctionInfo::CloneSourceInfo(ScriptContext* scriptContext, const ParseableFunctionInfo& other, ScriptContext* othersScriptContext, uint sourceIndex)
    {
        if (!m_utf8SourceHasBeenSet)
        {
            this->m_utf8SourceInfo = scriptContext->GetSource(sourceIndex);
            this->m_sourceIndex = sourceIndex;
            this->m_cchStartOffset = other.m_cchStartOffset;
            this->m_cchLength = other.m_cchLength;
            this->m_lineNumber = other.m_lineNumber;
            this->m_columnNumber = other.m_columnNumber;
            this->m_isEval = other.m_isEval;
            this->m_isDynamicFunction = other.m_isDynamicFunction;
            this->m_cbStartOffset =  other.StartOffset();
            this->m_cbLength = other.LengthInBytes();
            this->m_utf8SourceHasBeenSet = true;

            if (this->IsFunctionBody())
            {
                this->GetFunctionBody()->FinishSourceInfo();
            }
        }
#if DBG
        else
        {
            AssertMsg(this->m_cchStartOffset == other.m_cchStartOffset, "Mismatched source character start offset");
            AssertMsg(this->StartOffset() == other.StartOffset(), "Mismatched source start offset");
            AssertMsg(this->m_cchLength == other.m_cchLength, "Mismatched source character length");
            AssertMsg(this->LengthInBytes() == other.LengthInBytes(), "Mismatch source byte length");
            AssertMsg(this->GetUtf8SourceInfo()->GetSourceHolder() == scriptContext->GetSource(this->m_sourceIndex)->GetSourceHolder(),
                      "Mismatched source holder pointer");
            AssertMsg(this->m_isEval == other.m_isEval, "Mismatched source type");
            AssertMsg(this->m_isDynamicFunction == other.m_isDynamicFunction, "Mismatch source type");
       }
#endif
    }

    void ParseableFunctionInfo::SetSourceInfo(uint sourceIndex, ParseNodePtr node, bool isEval, bool isDynamicFunction)
    {
        if (!m_utf8SourceHasBeenSet)
        {
            this->m_sourceIndex = sourceIndex;
            this->m_cchStartOffset = node->ichMin;
            this->m_cchLength = node->LengthInCodepoints();
            this->m_lineNumber = node->sxFnc.lineNumber;
            this->m_columnNumber = node->sxFnc.columnNumber;
            this->m_isEval = isEval;
            this->m_isDynamicFunction = isDynamicFunction;
            this->m_cbStartOffset =  node->sxFnc.cbMin;
            this->m_cbLength = node->sxFnc.LengthInBytes();

            Assert(this->m_utf8SourceInfo != nullptr);
            this->m_utf8SourceHasBeenSet = true;

            if (this->IsFunctionBody())
            {
                this->GetFunctionBody()->FinishSourceInfo();
            }
        }
#if DBG
        else
        {
            AssertMsg(this->m_sourceIndex == sourceIndex, "Mismatched source index");
            if (!this->GetIsGlobalFunc())
            {
                // In the global function case with a @cc_on, we modify some of these values so it might
                // not match on reparse (see ParseableFunctionInfo::Parse()).
                AssertMsg(this->StartOffset() == node->sxFnc.cbMin, "Mismatched source start offset");
                AssertMsg(this->m_cchStartOffset == node->ichMin, "Mismatched source character start offset");
                AssertMsg(this->m_cchLength == node->LengthInCodepoints(), "Mismatched source length");
                AssertMsg(this->LengthInBytes() == node->sxFnc.LengthInBytes(), "Mismatched source encoded byte length");
            }

            AssertMsg(this->m_isEval == isEval, "Mismatched source type");
            AssertMsg(this->m_isDynamicFunction == isDynamicFunction, "Mismatch source type");
       }
#endif

#if DBG_DUMP
        if (PHASE_TRACE1(Js::FunctionSourceInfoParsePhase))
        {
            if (this->HasBody())
            {
                FunctionProxy* proxy = this->GetFunctionProxy();
                if (proxy->IsFunctionBody())
                {
                    FunctionBody* functionBody = this->GetFunctionBody();
                    Assert( functionBody != nullptr );

                    functionBody->PrintStatementSourceLineFromStartOffset(functionBody->StartInDocument());
                    Output::Flush();
                }
            }
        }
#endif
    }

    bool FunctionBody::Is(void* ptr)
    {
        if(!ptr)
        {
            return false;
        }
        return VirtualTableInfo<FunctionBody>::HasVirtualTable(ptr);
    }

    bool FunctionBody::HasLineBreak() const
    {
        return this->HasLineBreak(this->StartOffset(), this->m_cchStartOffset + this->m_cchLength);
    }

    bool FunctionBody::HasLineBreak(charcount_t start, charcount_t end) const
    {
        if (start > end) return false;
        charcount_t cchLength = end - start;
        if (start < this->m_cchStartOffset || cchLength > this->m_cchLength) return false;
        LPCUTF8 src = this->GetSource(L"FunctionBody::HasLineBreak");
        LPCUTF8 last = src + this->LengthInBytes();
        size_t offset = this->LengthInBytes() == this->m_cchLength ?
            start - this->m_cchStartOffset :
            utf8::CharacterIndexToByteIndex(src, this->LengthInBytes(), start - this->m_cchStartOffset, utf8::doAllowThreeByteSurrogates);
        src = src + offset;

        utf8::DecodeOptions options = utf8::doAllowThreeByteSurrogates;

        for (charcount_t cch = cchLength; cch > 0; --cch)
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

    FunctionBody::StatementMap* FunctionBody::GetMatchingStatementMapFromByteCode(int byteCodeOffset, bool ignoreSubexpressions /* = false */)
    {
        StatementMapList * pStatementMaps = this->GetStatementMaps();
        if (pStatementMaps)
        {
            Assert(m_sourceInfo.pSpanSequence == NULL);
            for (int index = 0; index < pStatementMaps->Count(); index++)
            {
                FunctionBody::StatementMap* pStatementMap = pStatementMaps->Item(index);

                if (!(ignoreSubexpressions && pStatementMap->isSubexpression) &&  pStatementMap->byteCodeSpan.Includes(byteCodeOffset))
                {
                    return pStatementMap;
                }
            }
        }
        return NULL;
    }

    // Returns the StatementMap for the offset.
    // 1. Current statementMap if bytecodeoffset falls within bytecode's span
    // 2. Previous if the bytecodeoffset is in between previous's end to current's begin

    FunctionBody::StatementMap* FunctionBody::GetEnclosingStatementMapFromByteCode(int byteCodeOffset, bool ignoreSubexpressions /* = false */)
    {
        int index = GetEnclosingStatementIndexFromByteCode(byteCodeOffset, ignoreSubexpressions);
        if (index != -1)
        {
            return this->GetStatementMaps()->Item(index);
        }
        return nullptr;
    }

    // Returns the index of StatementMap for
    // 1. Current statementMap if bytecodeoffset falls within bytecode's span
    // 2. Previous if the bytecodeoffset is in between previous's end to current's begin
    // 3. -1 of the failures.

    int FunctionBody::GetEnclosingStatementIndexFromByteCode(int byteCodeOffset, bool ignoreSubexpressions /* = false */)
    {
        StatementMapList * pStatementMaps = this->GetStatementMaps();
        if (pStatementMaps == nullptr)
        {
            // eg. internal library.
            return -1;
        }

        Assert(m_sourceInfo.pSpanSequence == NULL);

        for (int index = 0; index < pStatementMaps->Count(); index++)
        {
            FunctionBody::StatementMap* pStatementMap = pStatementMaps->Item(index);

            if (!(ignoreSubexpressions && pStatementMap->isSubexpression) && pStatementMap->byteCodeSpan.Includes(byteCodeOffset))
            {
                return index;
            }
            else if (!pStatementMap->isSubexpression && byteCodeOffset < pStatementMap->byteCodeSpan.begin) // We always ignore sub expressions when checking if we went too far
            {
                return index > 0 ? index - 1 : 0;
            }
        }

        return pStatementMaps->Count() - 1;
    }

    // In some cases in legacy mode, due to the state scriptContext->windowIdList, parser might not detect an eval call in the first parse but do so in the reparse
    // This fixes up the state at the start of reparse - Bug 272122
    void FunctionBody::SaveState(ParseNodePtr pnode)
    {
        Assert(!this->IsReparsed());
        this->SetChildCallsEval(!!pnode->sxFnc.ChildCallsEval());
        this->SetCallsEval(!!pnode->sxFnc.CallsEval());
        this->SetHasReferenceableBuiltInArguments(!!pnode->sxFnc.HasReferenceableBuiltInArguments());
    }

    void FunctionBody::RestoreState(ParseNodePtr pnode)
    {
        Assert(this->IsReparsed());
#if ENABLE_DEBUG_CONFIG_OPTIONS
        wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];
#endif
        if(!!pnode->sxFnc.ChildCallsEval() != this->GetChildCallsEval())
        {
            OUTPUT_VERBOSE_TRACE(Js::DebuggerPhase, L"Child calls eval is differerent on debug reparse: %s(%s)\n", this->GetExternalDisplayName(), this->GetDebugNumberSet(debugStringBuffer));
        }
        if(!!pnode->sxFnc.CallsEval() != this->GetCallsEval())
        {
            OUTPUT_VERBOSE_TRACE(Js::DebuggerPhase, L"Calls eval is differerent on debug reparse: %s(%s)\n", this->GetExternalDisplayName(), this->GetDebugNumberSet(debugStringBuffer));
        }
        if(!!pnode->sxFnc.HasReferenceableBuiltInArguments() != this->HasReferenceableBuiltInArguments())
        {
            OUTPUT_VERBOSE_TRACE(Js::DebuggerPhase, L"Referencable Built in args is differerent on debug reparse: %s(%s)\n", this->GetExternalDisplayName(), this->GetDebugNumberSet(debugStringBuffer));
        }

        pnode->sxFnc.SetChildCallsEval(this->GetChildCallsEval());
        pnode->sxFnc.SetCallsEval(this->GetCallsEval());
        pnode->sxFnc.SetHasReferenceableBuiltInArguments(this->HasReferenceableBuiltInArguments());
    }

    // Retrieves statement map for given byte code offset.
    // Parameters:
    // - sourceOffset: byte code offset to get map for.
    // - mapIndex: if not NULL, receives the index of founf map.
    FunctionBody::StatementMap* FunctionBody::GetMatchingStatementMapFromSource(int sourceOffset, int* pMapIndex /* = NULL */)
    {
        StatementMapList * pStatementMaps = this->GetStatementMaps();
        if (pStatementMaps && pStatementMaps->Count() > 0)
        {
            Assert(m_sourceInfo.pSpanSequence == NULL);
            for (int index = pStatementMaps->Count() - 1; index >= 0; index--)
            {
                FunctionBody::StatementMap* pStatementMap = pStatementMaps->Item(index);

                if (!pStatementMap->isSubexpression && pStatementMap->sourceSpan.Includes(sourceOffset))
                {
                    if (pMapIndex)
                    {
                        *pMapIndex = index;
                    }
                    return pStatementMap;
                }
            }
        }

        if (pMapIndex)
        {
            *pMapIndex = 0;
        }
        return NULL;
    }

    //
    // The function determine the the line and column for a bytecode offset within the current script buffer.
    //
    bool FunctionBody::GetLineCharOffset(int byteCodeOffset, ULONG* _line, LONG* _charOffset, bool canAllocateLineCache /*= true*/)
    {
        Assert(!this->GetUtf8SourceInfo()->GetIsLibraryCode());

        int startCharOfStatement = this->m_cchStartOffset; // Default to the start of this function

        if (m_sourceInfo.pSpanSequence)
        {
            SmallSpanSequenceIter iter;
            m_sourceInfo.pSpanSequence->Reset(iter);

            StatementData data;

            if (m_sourceInfo.pSpanSequence->GetMatchingStatementFromBytecode(byteCodeOffset, iter, data)
                && EndsAfter(data.sourceBegin))
            {
                startCharOfStatement = data.sourceBegin;
            }
        }
        else
        {
            Js::FunctionBody::StatementMap* map = this->GetEnclosingStatementMapFromByteCode(byteCodeOffset, false);
            if (map && EndsAfter(map->sourceSpan.begin))
            {
                startCharOfStatement = map->sourceSpan.begin;
            }
        }

        return this->GetLineCharOffsetFromStartChar(startCharOfStatement, _line, _charOffset, canAllocateLineCache);
    }

    bool FunctionBody::GetLineCharOffsetFromStartChar(int startCharOfStatement, ULONG* _line, LONG* _charOffset, bool canAllocateLineCache /*= true*/)
    {
        Assert(!this->GetUtf8SourceInfo()->GetIsLibraryCode());

        // The following adjusts for where the script is within the document
        ULONG line = this->GetHostStartLine();
        charcount_t column = 0;
        ULONG lineCharOffset = 0;
        charcount_t lineByteOffset = 0;

        if (startCharOfStatement > 0)
        {
            bool doSlowLookup = !canAllocateLineCache;
            if (canAllocateLineCache)
            {
                HRESULT hr = m_utf8SourceInfo->EnsureLineOffsetCacheNoThrow();
                if (FAILED(hr))
                {
                    if (hr != E_OUTOFMEMORY)
                    {
                        Assert(hr == E_ABORT); // The only other possible error we know about is ScriptAbort from QueryContinue.
                        return false;
                    }

                    // Clear the cache so it is not used.
                    this->m_utf8SourceInfo->DeleteLineOffsetCache();

                    //We can try and do the slow lookup below
                    doSlowLookup = true;
                }
            }

            charcount_t cacheLine = 0;
            this->m_utf8SourceInfo->GetLineInfoForCharPosition(startCharOfStatement, &cacheLine, &column, &lineByteOffset, doSlowLookup);

            // Update the tracking variables to jump to the line position (only need to jump if not on the first line).
            if (cacheLine > 0)
            {
                line += cacheLine;
                lineCharOffset = startCharOfStatement - column;
            }
        }

        if (this->GetSourceContextInfo()->IsDynamic() && this->m_isDynamicFunction)
        {
            line -= JavascriptFunction::numberLinesPrependedToAnonymousFunction;
        }

        if(_line)
        {
            *_line = line;
        }

        if(_charOffset)
        {
            *_charOffset = column;

            // If we are at the beginning of the host code, adjust the offset based on the host provided offset
            if (this->GetHostSrcInfo()->dlnHost == line)
            {
                *_charOffset += (LONG)this->GetHostStartColumn();
            }
        }

        return true;
    }

    bool FunctionBody::GetStatementIndexAndLengthAt(int byteCodeOffset, UINT32* statementIndex, UINT32* statementLength)
    {
        Assert(statementIndex != nullptr);
        Assert(statementLength != nullptr);

        Assert(m_scriptContext->IsInDebugMode());

        StatementMap * statement = GetEnclosingStatementMapFromByteCode(byteCodeOffset, false);
        Assert(statement != nullptr);

        // Bailout if we are unable to find a statement.
        // We shouldn't be missing these when a debugger is attached but we don't want to AV on retail builds.
        if (statement == nullptr)
        {
            return false;
        }

        Assert(m_utf8SourceInfo);
        const SRCINFO * srcInfo = m_utf8SourceInfo->GetSrcInfo();

        // Offset from the beginning of the document minus any host-supplied source characters.
        // Host supplied characters are inserted (for example) around onload:
        //      onload="foo('somestring', 0)" -> function onload(event).{.foo('somestring', 0).}
        ULONG offsetFromDocumentBegin = srcInfo ? srcInfo->ulCharOffset - srcInfo->ichMinHost : 0;

        *statementIndex = statement->sourceSpan.Begin() + offsetFromDocumentBegin;
        *statementLength = statement->sourceSpan.End() - statement->sourceSpan.Begin();
        return true;
    }

    void FunctionBody::RecordFrameDisplayRegister(RegSlot slot)
    {
        AssertMsg(slot != 0, "The assumption that the Frame Display Register cannot be at the 0 slot is wrong.");
        SetFrameDisplayRegister(slot);
    }

    void FunctionBody::RecordObjectRegister(RegSlot slot)
    {
        AssertMsg(slot != 0, "The assumption that the Object Register cannot be at the 0 slot is wrong.");
        SetObjectRegister(slot);
    }

    Js::RootObjectBase * FunctionBody::GetRootObject() const
    {
        // Safe to be used by the JIT thread
        Assert(this->m_constTable != null);
        return (Js::RootObjectBase *)this->m_constTable[Js::FunctionBody::RootObjectRegSlot - FunctionBody::FirstRegSlot];
    }

    Js::RootObjectBase * FunctionBody::LoadRootObject() const
    {
        if (this->GetModuleID() == kmodGlobal)
        {
            return JavascriptOperators::OP_LdRoot(this->GetScriptContext());
        }
        return JavascriptOperators::GetModuleRoot(this->GetModuleID(), this->GetScriptContext());
    }

    FunctionEntryPointInfo * FunctionBody::GetEntryPointFromNativeAddress(DWORD_PTR codeAddress)
    {
        FunctionEntryPointInfo * entryPoint = NULL;
        this->MapEntryPoints([&entryPoint, &codeAddress](int index, FunctionEntryPointInfo * currentEntryPoint)
        {
            // We need to do a second check for IsNativeCode because the entry point could be in the process of
            // being recorded on the background thread
            if (currentEntryPoint->IsInNativeAddressRange(codeAddress))
            {
                entryPoint = currentEntryPoint;
            }
        });

        return entryPoint;
    }

    LoopEntryPointInfo * FunctionBody::GetLoopEntryPointInfoFromNativeAddress(DWORD_PTR codeAddress, uint loopNum) const
    {
        LoopEntryPointInfo * entryPoint = NULL;

        LoopHeader * loopHeader = this->GetLoopHeader(loopNum);
        Assert(loopHeader);

        loopHeader->MapEntryPoints([&](int index, LoopEntryPointInfo * currentEntryPoint)
        {
            if (currentEntryPoint->IsCodeGenDone() &&
                codeAddress >= currentEntryPoint->GetNativeAddress() &&
                codeAddress < currentEntryPoint->GetNativeAddress() + currentEntryPoint->GetCodeSize())
            {
                entryPoint = currentEntryPoint;
            }
        });

        return entryPoint;
    }

    int FunctionBody::GetStatementIndexFromNativeOffset(SmallSpanSequence *pThrowSpanSequence, uint32 nativeOffset)
    {
        int statementIndex = -1;
        if (pThrowSpanSequence)
        {
            SmallSpanSequenceIter iter;
            StatementData tmpData;
            if (pThrowSpanSequence->GetMatchingStatementFromBytecode(nativeOffset, iter, tmpData))
            {
                statementIndex = tmpData.sourceBegin; // sourceBegin represents statementIndex here
            }
            else
            {
                // If nativeOffset falls on the last span, GetMatchingStatement would miss it because SmallSpanSequence
                // does not know about the last span end. Since we checked that codeAddress is within our range,
                // we can safely consider it matches the last span.
                statementIndex = iter.accumulatedSourceBegin;
            }
        }

        return statementIndex;
    }

    int FunctionBody::GetStatementIndexFromNativeAddress(SmallSpanSequence *pThrowSpanSequence, DWORD_PTR codeAddress, DWORD_PTR nativeBaseAddress)
    {
        uint32 nativeOffset = (uint32)(codeAddress - nativeBaseAddress);

        return GetStatementIndexFromNativeOffset(pThrowSpanSequence, nativeOffset);
    }

    BOOL FunctionBody::GetMatchingStatementMap(StatementData &data, int statementIndex, FunctionBody *inlinee)
    {
        SourceInfo *si = &this->m_sourceInfo;
        if (inlinee)
        {
            si = &inlinee->m_sourceInfo;
            Assert(si);
        }

        if (statementIndex >= 0)
        {
            SmallSpanSequence *pSpanSequence = si->pSpanSequence;
            if (pSpanSequence)
            {
                SmallSpanSequenceIter iter;
                pSpanSequence->Reset(iter);

                if (pSpanSequence->Item(statementIndex, iter, data))
                {
                    return TRUE;
                }
            }
            else
            {
                StatementMapList* pStatementMaps = GetStatementMaps();
                Assert(pStatementMaps);
                if (statementIndex >= pStatementMaps->Count())
                {
                    return FALSE;
                }

                data.sourceBegin = pStatementMaps->Item(statementIndex)->sourceSpan.begin;
                data.bytecodeBegin = pStatementMaps->Item(statementIndex)->byteCodeSpan.begin;
                return TRUE;
            }
        }

        return FALSE;
    }

    void FunctionBody::FindClosestStatements(long characterOffset, StatementLocation *firstStatementLocation, StatementLocation *secondStatementLocation)
    {
        auto statementMaps = this->GetStatementMaps();
        if (statementMaps)
        {
            for(int i = 0; i < statementMaps->Count(); i++)
            {
                regex::Interval* pSourceSpan = &(statementMaps->Item(i)->sourceSpan);
                if (FunctionBody::IsDummyGlobalRetStatement(pSourceSpan))
                {
                    // Workaround for handling global return, which is a empty range.
                    // Ideal fix should be in bytecode generator, which should not emit the bytecode for global return.
                    // Once the fix done, this 'if' statement should be changed to an Assert
                    continue;
                }

                if (pSourceSpan->begin < characterOffset
                    && (firstStatementLocation->function == NULL || firstStatementLocation->statement.begin < pSourceSpan->begin))
                {
                    firstStatementLocation->function = this;
                    firstStatementLocation->statement = *pSourceSpan;
                    firstStatementLocation->bytecodeSpan = statementMaps->Item(i)->byteCodeSpan;
                }
                else if (pSourceSpan->begin >= characterOffset
                    && (secondStatementLocation->function == NULL || secondStatementLocation->statement.begin > pSourceSpan->begin))
                {
                    secondStatementLocation->function = this;
                    secondStatementLocation->statement = *pSourceSpan;
                    secondStatementLocation->bytecodeSpan = statementMaps->Item(i)->byteCodeSpan;
                }
            }
        }
    }

    BOOL FunctionBody::GetMatchingStatementMapFromNativeAddress(DWORD_PTR codeAddress, StatementData &data, uint loopNum, FunctionBody *inlinee /* = null */)
    {
        SmallSpanSequence * spanSequence = NULL;
        FunctionEntryPointInfo * entryPoint = GetEntryPointFromNativeAddress(codeAddress);
        DWORD_PTR nativeBaseAddress = NULL;

        if (entryPoint != NULL)
        {
            spanSequence = entryPoint->GetNativeThrowSpanSequence();
            nativeBaseAddress = entryPoint->GetNativeAddress();
        }
        else
        {
            LoopEntryPointInfo * entryPoint = GetLoopEntryPointInfoFromNativeAddress(codeAddress, loopNum);
            if (entryPoint != NULL)
            {
                spanSequence = entryPoint->GetNativeThrowSpanSequence();
                nativeBaseAddress = entryPoint->GetNativeAddress();
            }
        }

        int statementIndex = GetStatementIndexFromNativeAddress(spanSequence, codeAddress, nativeBaseAddress);

        return GetMatchingStatementMap(data, statementIndex, inlinee);
    }

    BOOL FunctionBody::GetMatchingStatementMapFromNativeOffset(DWORD_PTR codeAddress, uint32 offset, StatementData &data, FunctionBody *inlinee /* = null */)
    {
        SmallSpanSequence * spanSequence = NULL;
        FunctionEntryPointInfo * entryPoint = GetEntryPointFromNativeAddress(codeAddress);

        if (entryPoint != NULL)
        {
            spanSequence = entryPoint->GetNativeThrowSpanSequence();
        }

        int statementIndex = GetStatementIndexFromNativeOffset(spanSequence, offset);

        return GetMatchingStatementMap(data, statementIndex, inlinee);
    }

    void FunctionBody::LoadDynamicProfileInfo()
    {
        SourceDynamicProfileManager * sourceDynamicProfileManager = GetSourceContextInfo()->sourceDynamicProfileManager;
        if (sourceDynamicProfileManager != null)
        {
            this->dynamicProfileInfo = sourceDynamicProfileManager->GetDynamicProfileInfo(this);
#if DBG_DUMP
            if(this->dynamicProfileInfo)
            {
                if (Configuration::Global.flags.Dump.IsEnabled(DynamicProfilePhase, this->GetSourceContextId(), this->GetLocalFunctionId()))
                {
                    Output::Print(L"Loaded:");
                    this->dynamicProfileInfo->Dump(this);
                }
            }
#endif
        }

#ifdef DYNAMIC_PROFILE_MUTATOR
        DynamicProfileMutator::Mutate(this);
#endif
    }

    bool FunctionBody::NeedEnsureDynamicProfileInfo() const
    {
        // Only need to ensure dynamic profile if we don't already have link up the dynamic profile info
        // and dynamic profile collection is enabled
        return
            !this->m_isFromNativeCodeModule &&
            !this->m_isAsmJsFunction &&
            !this->HasExecutionDynamicProfileInfo() &&
            DynamicProfileInfo::IsEnabled(this);
    }

    DynamicProfileInfo * FunctionBody::EnsureDynamicProfileInfo()
    {
        if (this->NeedEnsureDynamicProfileInfo())
        {
            m_scriptContext->AddDynamicProfileInfo(this, &this->dynamicProfileInfo);
            Assert(!this->HasExecutionDynamicProfileInfo());
            this->hasExecutionDynamicProfileInfo = true;
        }

        return this->dynamicProfileInfo;
    }

    DynamicProfileInfo* FunctionBody::AllocateDynamicProfile()
    {
        return DynamicProfileInfo::New(m_scriptContext->GetRecycler(), this);
    }

    BOOL FunctionBody::IsNativeOriginalEntryPoint() const
    {
#ifdef ENABLE_NATIVE_CODEGEN
        return IsNativeFunctionAddr(this->GetScriptContext(), this->originalEntryPoint);
#else
        return false;
#endif
    }

    bool FunctionBody::IsSimpleJitOriginalEntryPoint() const
    {
        const FunctionEntryPointInfo *const simpleJitEntryPointInfo = GetSimpleJitEntryPointInfo();
        return
            simpleJitEntryPointInfo &&
            reinterpret_cast<Js::JavascriptMethod>(simpleJitEntryPointInfo->GetNativeAddress()) == originalEntryPoint;
    }

    void FunctionProxy::Finalize(bool isShutdown)
    {
        __super::Finalize(isShutdown);

        this->CleanupFunctionProxyCounters();
    }

#if DBG
    bool FunctionBody::HasValidSourceInfo()
    {
        SourceContextInfo* sourceContextInfo;

        if (m_scriptContext->GetSourceContextInfoMap())
        {
            if(m_scriptContext->GetSourceContextInfoMap()->TryGetValue(this->GetHostSourceContext(), &sourceContextInfo) &&
                sourceContextInfo == this->GetSourceContextInfo())
            {
                return true;
            }
        }
        Assert(this->IsDynamicScript());

        if(m_scriptContext->GetDynamicSourceContextInfoMap())
        {
            if(m_scriptContext->GetDynamicSourceContextInfoMap()->TryGetValue(this->GetSourceContextInfo()->hash, &sourceContextInfo) &&
                sourceContextInfo == this->GetSourceContextInfo())
            {
                return true;
            }
        }

        // The SourceContextInfo will not be added to the dynamicSourceContextInfoMap, if they are host provided dynamic code. But they are valid source context info
        if (this->GetSourceContextInfo()->isHostDynamicDocument)
        {
            return true;
        }
        return m_scriptContext->IsNoContextSourceContextInfo(this->GetSourceContextInfo());
    }

    // originalEntryPoint: DefaultDeferredParsingThunk, DefaultDeferredDeserializeThunk, DefaultEntryThunk, dynamic interpreter thunk or native entry point
    // directEntryPoint:
    //      if (!profiled) - DefaultDeferredParsingThunk, DefaultDeferredDeserializeThunk, DefaultEntryThunk, CheckCodeGenThunk,
    //                       dynamic interpreter thunk, native entry point
    //      if (profiling) - ProfileDeferredParsingThunk, ProfileDeferredDeserializeThunk, ProfileEntryThunk, CheckCodeGenThunk
    bool FunctionProxy::HasValidNonProfileEntryPoint() const
    {
        JavascriptMethod directEntryPoint = (JavascriptMethod)this->GetDefaultEntryPointInfo()->address;
        JavascriptMethod originalEntryPoint = this->originalEntryPoint;

        // Check the direct entry point to see if it is code gen thunk
        // if it is not, the back ground code gen thread have updated both original entry point and direct entry point
        // and they should still match, same as cases other then code gen
        return IsIntermediateCodeGenThunk(directEntryPoint) || originalEntryPoint == directEntryPoint
            || (directEntryPoint == DynamicProfileInfo::EnsureDynamicProfileInfoThunk &&
            this->IsFunctionBody() && this->GetFunctionBody()->IsNativeOriginalEntryPoint()
#ifdef ASMJS_PLAT
            || (GetFunctionBody()->GetIsAsmJsFunction() && directEntryPoint == AsmJsDefaultEntryThunk)
            || (IsAsmJsCodeGenThunk(directEntryPoint))
#endif
            );
    }
    bool FunctionProxy::HasValidProfileEntryPoint() const
    {
        JavascriptMethod directEntryPoint = (JavascriptMethod)this->GetDefaultEntryPointInfo()->address;
        if (this->originalEntryPoint == DefaultDeferredParsingThunk)
        {
            return directEntryPoint == ProfileDeferredParsingThunk;
        }
        if (this->originalEntryPoint == DefaultDeferredDeserializeThunk)
        {
            return directEntryPoint == ProfileDeferredDeserializeThunk;
        }
        if (!this->IsFunctionBody())
        {
            return false;
        }
        FunctionBody * functionBody = this->GetFunctionBody();
        if (functionBody->IsInterpreterThunk() || functionBody->IsSimpleJitOriginalEntryPoint())
        {
            return directEntryPoint == ProfileEntryThunk || IsIntermediateCodeGenThunk(directEntryPoint);
        }

        // In the profiler mode, the EnsureDynamicProfileInfoThunk is valid as we would be assigning to appropriate thunk when that thunk called.
        return functionBody->IsNativeOriginalEntryPoint() &&
            (directEntryPoint == DynamicProfileInfo::EnsureDynamicProfileInfoThunk || directEntryPoint == ProfileEntryThunk);
    }

    bool FunctionProxy::HasValidEntryPoint() const
    {
        if (!m_scriptContext->HadProfiled() &&
            !(m_scriptContext->IsInDebugMode() && m_scriptContext->IsExceptionWrapperForBuiltInsEnabled()))
        {
            return this->HasValidNonProfileEntryPoint();
        }
        if (m_scriptContext->IsProfiling())
        {
            return this->HasValidProfileEntryPoint();
        }
        return this->HasValidNonProfileEntryPoint() || this->HasValidProfileEntryPoint();
    }

#endif
    void ParseableFunctionInfo::SetDeferredParsingEntryPoint()
    {
        Assert(m_scriptContext->DeferredParsingThunk == ProfileDeferredParsingThunk
            || m_scriptContext->DeferredParsingThunk == DefaultDeferredParsingThunk);

        this->SetEntryPoint(this->GetDefaultEntryPointInfo(), m_scriptContext->DeferredParsingThunk);
        originalEntryPoint = DefaultDeferredParsingThunk;
    }

    void ParseableFunctionInfo::SetInitialDefaultEntryPoint()
    {
        Assert(m_scriptContext->CurrentThunk == ProfileEntryThunk || m_scriptContext->CurrentThunk == DefaultEntryThunk);
        Assert(originalEntryPoint == DefaultDeferredParsingThunk || originalEntryPoint == ProfileDeferredParsingThunk ||
               originalEntryPoint == DefaultDeferredDeserializeThunk || originalEntryPoint == ProfileDeferredDeserializeThunk ||
               originalEntryPoint == DefaultEntryThunk || originalEntryPoint == ProfileEntryThunk);
        Assert(this->m_defaultEntryPointInfo != null);

        // CONSIDER: we can optimize this to generate the dynamic interpreter thunk up front
        // If we know that we are in the defer parsing thunk already
        this->SetEntryPoint(this->GetDefaultEntryPointInfo(), m_scriptContext->CurrentThunk);
        this->originalEntryPoint = DefaultEntryThunk;
    }

    void FunctionBody::SetCheckCodeGenEntryPoint(FunctionEntryPointInfo* entryPointInfo, JavascriptMethod entryPoint)
    {
        Assert(IsIntermediateCodeGenThunk(entryPoint));
        Assert(
            this->GetEntryPoint(entryPointInfo) == m_scriptContext->CurrentThunk ||
            (entryPointInfo == this->m_defaultEntryPointInfo && this->IsInterpreterThunk()) ||
            (
                GetSimpleJitEntryPointInfo() &&
                GetEntryPoint(entryPointInfo) == reinterpret_cast<void *>(GetSimpleJitEntryPointInfo()->GetNativeAddress())
            ));
        this->SetEntryPoint(entryPointInfo, entryPoint);
    }

    void FunctionBody::GenerateDynamicInterpreterThunk()
    {
        if (this->m_dynamicInterpreterThunk == null)
        {
            // NOTE: Etw rundown thread may be reading this->dynamicInterpreterThunk concurrently. We don't need to synchronize
            // access as it is ok for etw rundown to get either null or updated new value.

            if (m_isAsmJsFunction)
            {
                this->originalEntryPoint = this->m_scriptContext->GetNextDynamicAsmJsInterpreterThunk(&this->m_dynamicInterpreterThunk);
            }
            else
            {
                this->originalEntryPoint = this->m_scriptContext->GetNextDynamicInterpreterThunk(&this->m_dynamicInterpreterThunk);
            }
            EtwTrace::LogMethodInterpreterThunkLoadEvent(this);
        }
        else
        {
            this->originalEntryPoint = (JavascriptMethod)InterpreterThunkEmitter::ConvertToEntryPoint(this->m_dynamicInterpreterThunk);
        }
    }

    JavascriptMethod FunctionBody::EnsureDynamicInterpreterThunk(FunctionEntryPointInfo* entryPointInfo)
    {
        // This may be first call to the function, make sure we have dynamic profile info
        //
        // We need to ensure dynamic profile info even if we didn't generate a dynamic interpreter thunk
        // This happens when we go thru CheckCodeGen thunk, to DelayDynamicInterpreterThunk, to here
        // but the back ground code gen thread updated the entry point with the native entry point.

        this->EnsureDynamicProfileInfo();

        Assert(HasValidEntryPoint());
        if (InterpreterStackFrame::IsDelayDynamicInterpreterThunk(this->GetEntryPoint(entryPointInfo)))
        {
            // We are not doing code gen on this function, just change the entry point directly
            Assert(InterpreterStackFrame::IsDelayDynamicInterpreterThunk(originalEntryPoint));
            GenerateDynamicInterpreterThunk();
            this->SetEntryPoint(entryPointInfo, originalEntryPoint);
        }
        else if (this->GetEntryPoint(entryPointInfo) == ProfileEntryThunk)
        {
            // We are not doing code gen on this function, just change the entry point directly
            // Don't replace the profile entry thunk
            Assert(InterpreterStackFrame::IsDelayDynamicInterpreterThunk(originalEntryPoint));
            GenerateDynamicInterpreterThunk();
        }
        else if (InterpreterStackFrame::IsDelayDynamicInterpreterThunk(originalEntryPoint))
        {
            JsUtil::JobProcessor * jobProcessor = this->GetScriptContext()->GetThreadContext()->GetJobProcessor();
            if (jobProcessor->ProcessesInBackground())
            {
                JsUtil::BackgroundJobProcessor * backgroundJobProcessor = static_cast<JsUtil::BackgroundJobProcessor *>(jobProcessor);
                AutoCriticalSection autocs(backgroundJobProcessor->GetCriticalSection());
                // Check again under lock
                if (InterpreterStackFrame::IsDelayDynamicInterpreterThunk(originalEntryPoint))
                {
                    // If the original entry point is DelayDynamicInterpreterThunk then there must be a version of this
                    // function being codegen'd.
                    Assert(IsIntermediateCodeGenThunk((JavascriptMethod)this->GetEntryPoint(this->GetDefaultEntryPointInfo())) || IsAsmJsCodeGenThunk((JavascriptMethod)this->GetEntryPoint(this->GetDefaultEntryPointInfo())));
                    GenerateDynamicInterpreterThunk();
                }
            }
            else
            {
                // If the original entry point is DelayDynamicInterpreterThunk then there must be a version of this
                // function being codegen'd.
                Assert(IsIntermediateCodeGenThunk((JavascriptMethod)this->GetEntryPoint(this->GetDefaultEntryPointInfo())) || IsAsmJsCodeGenThunk((JavascriptMethod)this->GetEntryPoint(this->GetDefaultEntryPointInfo())));
                GenerateDynamicInterpreterThunk();
            }
        }
        return this->originalEntryPoint;
    }

    void FunctionBody::SetNativeEntryPoint(FunctionEntryPointInfo* entryPointInfo, JavascriptMethod originalEntryPoint, Var directEntryPoint)
    {
        if(entryPointInfo->nativeEntryPointProcessed)
        {
            return;
        }
        bool isAsmJs = this->GetIsAsmjsMode();
        Assert(IsIntermediateCodeGenThunk((JavascriptMethod)entryPointInfo->address) || CONFIG_FLAG(Prejit) || this->m_isFromNativeCodeModule || isAsmJs);
        entryPointInfo->EnsureIsReadyToCall();

        // keep originalEntryPoint updated with the latest known good native entry point
        if (entryPointInfo == this->GetDefaultEntryPointInfo())
        {
            this->originalEntryPoint = originalEntryPoint;
        }

        if (entryPointInfo->entryPointIndex == 0 && this->NeedEnsureDynamicProfileInfo())
        {
            entryPointInfo->address = DynamicProfileInfo::EnsureDynamicProfileInfoThunk;
        }
        else
        {
            entryPointInfo->address = directEntryPoint;
        }
        if (isAsmJs)
        {
            // release the old entrypointinfo if available
            FunctionEntryPointInfo* oldEntryPointInfo = entryPointInfo->GetOldFunctionEntryPointInfo();
            if (oldEntryPointInfo)
            {
                this->GetScriptContext()->GetThreadContext()->QueueFreeOldEntryPointInfoIfInScript(oldEntryPointInfo);
                oldEntryPointInfo = nullptr;
            }
        }
        this->CaptureDynamicProfileState(entryPointInfo);

        if(entryPointInfo->GetJitMode() == ExecutionMode::SimpleJit)
        {
            Assert(GetExecutionMode() == ExecutionMode::SimpleJit);
            SetSimpleJitEntryPointInfo(entryPointInfo);
            ResetSimpleJitCallCount();
        }
        else
        {
            Assert(entryPointInfo->GetJitMode() == ExecutionMode::FullJit);
            Assert(isAsmJs || GetExecutionMode() == ExecutionMode::FullJit);
            entryPointInfo->callsCount =
                static_cast<uint8>(
                    min(
                        static_cast<uint>(static_cast<uint8>(CONFIG_FLAG(MinBailOutsBeforeRejit))) *
                            (Js::FunctionEntryPointInfo::GetDecrCallCountPerBailout() - 1),
                        0xffu));
        }
        TraceExecutionMode();

        if(entryPointInfo->GetJitMode() == ExecutionMode::SimpleJit)
        {
            Assert(GetExecutionMode() == ExecutionMode::SimpleJit);
            SetSimpleJitEntryPointInfo(entryPointInfo);
            ResetSimpleJitCallCount();
        }
        else
        {
            Assert(entryPointInfo->GetJitMode() == ExecutionMode::FullJit);
            Assert(GetExecutionMode() == ExecutionMode::FullJit);
            entryPointInfo->callsCount =
                static_cast<uint8>(
                    min(
                        static_cast<uint>(static_cast<uint8>(CONFIG_FLAG(MinBailOutsBeforeRejit))) *
                            (Js::FunctionEntryPointInfo::GetDecrCallCountPerBailout() - 1),
                        0xffu));
        }

        EtwTrace::LogMethodNativeLoadEvent(this, entryPointInfo);

#ifdef _M_ARM
        // For ARM we need to make sure that pipeline is synchronized with memory/cache for newly jitted code.
        _InstructionSynchronizationBarrier();
#endif

        entryPointInfo->nativeEntryPointProcessed = true;
    }

    void FunctionBody::DefaultSetNativeEntryPoint(FunctionEntryPointInfo* entryPointInfo, FunctionBody * functionBody, JavascriptMethod entryPoint)
    {
        Assert(functionBody->m_scriptContext->CurrentThunk == DefaultEntryThunk);
        functionBody->SetNativeEntryPoint(entryPointInfo, entryPoint, entryPoint);
    }


    void FunctionBody::ProfileSetNativeEntryPoint(FunctionEntryPointInfo* entryPointInfo, FunctionBody * functionBody, JavascriptMethod entryPoint)
    {
        Assert(functionBody->m_scriptContext->CurrentThunk == ProfileEntryThunk);
        functionBody->SetNativeEntryPoint(entryPointInfo, entryPoint, ProfileEntryThunk);
    }

    Js::JavascriptMethod FunctionBody::GetLoopBodyEntryPoint(Js::LoopHeader * loopHeader, int entryPointIndex)
    {
#if DBG
        this->GetLoopNumber(loopHeader);
#endif
        return (Js::JavascriptMethod)(loopHeader->GetEntryPointInfo(entryPointIndex)->address);
    }

    void FunctionBody::SetLoopBodyEntryPoint(Js::LoopHeader * loopHeader, EntryPointInfo* entryPointInfo, Js::JavascriptMethod entryPoint)
    {
#if DBG_DUMP
        uint loopNum = this->GetLoopNumber(loopHeader);
        if (PHASE_TRACE1(Js::JITLoopBodyPhase))
        {
            DumpFunctionId(true);
            Output::Print(L": %-20s LoopBody EntryPt  Loop: %2d Address : %x\n", GetDisplayName(), loopNum, entryPoint);
            Output::Flush();
        }
#endif
        Assert(((LoopEntryPointInfo*) entryPointInfo)->loopHeader == loopHeader);
        Assert(entryPointInfo->address == null);
        entryPointInfo->address = (void*)entryPoint;
        // reset the counter to 1 less than the threshold for TJLoopBody
        if (loopHeader->GetCurrentEntryPointInfo()->GetIsAsmJSFunction())
        {
            loopHeader->interpretCount = entryPointInfo->GetFunctionBody()->GetLoopInterpretCount(loopHeader) - 1;
        }
        EtwTrace::LogLoopBodyLoadEvent(this, loopHeader, ((LoopEntryPointInfo*) entryPointInfo));
    }

    void FunctionBody::MarkScript(ByteBlock *byteCodeBlock, ByteBlock* auxBlock, ByteBlock* auxContextBlock,
        uint byteCodeCount, uint byteCodeInLoopCount, uint byteCodeWithoutLDACount)
    {
        CheckNotExecuting();
        CheckEmpty();

#ifdef PERF_COUNTERS
        DWORD byteCodeSize = byteCodeBlock->GetLength()
            + (auxBlock? auxBlock->GetLength() : 0)
            + (auxContextBlock? auxContextBlock->GetLength() : 0);
        PERF_COUNTER_ADD(Code, DynamicByteCodeSize, byteCodeSize);
#endif

        m_byteCodeCount = byteCodeCount;
        m_byteCodeInLoopCount = byteCodeInLoopCount;
        m_byteCodeWithoutLDACount = byteCodeWithoutLDACount;

        InitializeExecutionModeAndLimits();

        this->auxBlock = auxBlock;
        this->auxContextBlock = auxContextBlock;

        // Memory barrier is needed here to make sure the background code gen thread's inliner
        // Get all the assignment before it sees that the function has been parse
        MemoryBarrier();

        this->byteCodeBlock = byteCodeBlock;
        PERF_COUNTER_ADD(Code, TotalByteCodeSize, byteCodeSize);

        // If this is a defer parse function body, we would not have registered it
        // on the function bodies list so we should register it now
        if (!this->m_isFuncRegistered)
        {
            this->m_utf8SourceInfo->SetFunctionBody(this);
        }
    }

    uint
    FunctionBody::GetLoopNumber(LoopHeader const * loopHeader) const
    {
        Assert(loopHeader >=  this->loopHeaderArray);
        uint loopNum = (uint)(loopHeader - this->loopHeaderArray);
        Assert(loopNum < GetLoopCount());
        return loopNum;
    }

    bool FunctionBody::InstallProbe(int offset)
    {
        if (offset < 0 || ((uint)offset + 1) >= byteCodeBlock->GetLength())
        {
            return false;
        }

        byte* pbyteCodeBlockBuffer = this->byteCodeBlock->GetBuffer();

        if(!GetProbeBackingBlock())
        {
            // The probe backing block is set on a different thread than the main thread
            // The recycler doesn't like allocations from a different thread, so we allocate
            // the backing byte code block in the arena
            ArenaAllocator *pArena = m_scriptContext->AllocatorForDiagnostics();
            AssertMem(pArena);
            ByteBlock* probeBackingBlock = ByteBlock::NewFromArena(pArena, pbyteCodeBlockBuffer, byteCodeBlock->GetLength());
            SetProbeBackingBlock(probeBackingBlock);
        }

        // Make sure Break opcode only need one byte
        Assert(OpCodeUtil::IsSmallEncodedOpcode(OpCode::Break));
        Assert(!OpCodeAttr::HasMultiSizeLayout(OpCode::Break));
        *(byte *)(pbyteCodeBlockBuffer + offset) = (byte)OpCode::Break;

        ++m_sourceInfo.m_probeCount;

        return true;
    }

    bool FunctionBody::UninstallProbe(int offset)
    {
        if (offset < 0 || ((uint)offset + 1) >= byteCodeBlock->GetLength())
        {
            return false;
        }
        byte* pbyteCodeBlockBuffer = byteCodeBlock->GetBuffer();

        Js::OpCode originalOpCode = ByteCodeReader::PeekByteOp(GetProbeBackingBlock()->GetBuffer() + offset);
        *(pbyteCodeBlockBuffer + offset) = (byte)originalOpCode;

        --m_sourceInfo.m_probeCount;
        AssertMsg(m_sourceInfo.m_probeCount >= 0, "Probe (Break Point) count became negative!");

        return true;
    }

    bool FunctionBody::ProbeAtOffset(int offset, OpCode* pOriginalOpcode)
    {
        if (!GetProbeBackingBlock())
        {
            return false;
        }

        if (offset < 0 || ((uint)offset + 1) >= this->byteCodeBlock->GetLength())
        {
            // Something is very wrong at this point.
            AssertMsg(false, "ProbeAtOffset called with out of bounds offset");
            return false;
        }

        Js::OpCode runningOpCode = ByteCodeReader::PeekByteOp(this->byteCodeBlock->GetBuffer() + offset);
        Js::OpCode originalOpcode = ByteCodeReader::PeekByteOp(GetProbeBackingBlock()->GetBuffer() + offset);

        if ( runningOpCode != originalOpcode)
        {
            *pOriginalOpcode = originalOpcode;
            return true;
        }
        else
        {
            // e.g. inline break or a step hit and is checking for a bp
            return false;
        }
    }

    void FunctionBody::CloneByteCodeInto(ScriptContext * scriptContext, FunctionBody *newFunctionBody, uint sourceIndex)
    {
        ((ParseableFunctionInfo*) this)->CopyFunctionInfoInto(scriptContext, newFunctionBody, sourceIndex);

        newFunctionBody->m_constCount = this->m_constCount;
        newFunctionBody->m_varCount = this->m_varCount;
        newFunctionBody->m_outParamMaxDepth = this->m_outParamMaxDepth;

        newFunctionBody->m_firstTmpReg = this->m_firstTmpReg;
        newFunctionBody->stackClosureRegister = this->stackClosureRegister;
        newFunctionBody->loopCount = this->loopCount;
        newFunctionBody->profiledDivOrRemCount = this->profiledDivOrRemCount;
        newFunctionBody->profiledSwitchCount = this->profiledSwitchCount;
        newFunctionBody->profiledCallSiteCount = this->profiledCallSiteCount;
        newFunctionBody->profiledArrayCallSiteCount = this->profiledArrayCallSiteCount;
        newFunctionBody->profiledReturnTypeCount = this->profiledReturnTypeCount;
        newFunctionBody->profiledLdElemCount = this->profiledLdElemCount;
        newFunctionBody->profiledStElemCount = this->profiledStElemCount;
        newFunctionBody->profiledSlotCount = this->profiledSlotCount;
        newFunctionBody->flags = this->flags;
        newFunctionBody->m_isFuncRegistered = this->m_isFuncRegistered;
        newFunctionBody->m_isFuncRegisteredToDiag = this->m_isFuncRegisteredToDiag;
        newFunctionBody->m_hasBailoutInstrInJittedCode = this->m_hasBailoutInstrInJittedCode;
        newFunctionBody->m_depth = this->m_depth;
        newFunctionBody->inlineDepth = 0;
        newFunctionBody->recentlyBailedOutOfJittedLoopBody = false;
        newFunctionBody->m_pendingLoopHeaderRelease = this->m_pendingLoopHeaderRelease;
        newFunctionBody->m_envDepth = this->m_envDepth;

        if (this->m_constTable != NULL)
        {
            this->CloneConstantTable(newFunctionBody);
        }

        newFunctionBody->cacheIdToPropertyIdMap = this->cacheIdToPropertyIdMap;
        newFunctionBody->referencedPropertyIdMap = this->referencedPropertyIdMap;
        newFunctionBody->propertyIdsForScopeSlotArray = this->propertyIdsForScopeSlotArray;
        newFunctionBody->propertyIdOnRegSlotsContainer = this->propertyIdOnRegSlotsContainer;
        newFunctionBody->scopeSlotArraySize = this->scopeSlotArraySize;

        if (this->byteCodeBlock == NULL)
        {
            newFunctionBody->SetDeferredParsingEntryPoint();
        }
        else
        {
            // TODO: Most byte code block are allocated in the recycler, so we don't really need to clone them
            // But byte code serializer still create byte code block in the arena.  We should untangle that sometime
            if (BinaryFeatureControl::LanguageService())
                // The language service ensures that the lifetime of a function
                // clone will live longer than the clone.
                newFunctionBody->byteCodeBlock = this->byteCodeBlock;
            else
                newFunctionBody->byteCodeBlock = this->byteCodeBlock->Clone(this->m_scriptContext->GetRecycler());

            newFunctionBody->isByteCodeDebugMode = this->isByteCodeDebugMode;
            newFunctionBody->m_byteCodeCount = this->m_byteCodeCount;
            newFunctionBody->m_byteCodeWithoutLDACount = this->m_byteCodeWithoutLDACount;
            newFunctionBody->m_byteCodeInLoopCount = this->m_byteCodeInLoopCount;

#ifdef PERF_COUNTERS
            DWORD byteCodeSize = this->byteCodeBlock->GetLength();
#endif
            if (this->auxBlock)
            {
                if (BinaryFeatureControl::LanguageService())
                    // The language service ensures that the lifetime of a function
                    // clone will live longer than the clone.
                    newFunctionBody->auxBlock = this->auxBlock;
                else
                    newFunctionBody->auxBlock = this->auxBlock->Clone(this->m_scriptContext->GetRecycler());

#ifdef PERF_COUNTERS
                byteCodeSize += this->auxBlock->GetLength();
#endif
            }

            if (this->auxContextBlock)
            {
                newFunctionBody->auxContextBlock = this->auxContextBlock->Clone(scriptContext->GetRecycler(), scriptContext);
#ifdef PERF_COUNTERS
                byteCodeSize += this->auxContextBlock->GetLength();
#endif
            }

            if (this->GetProbeBackingBlock())
            {
                newFunctionBody->SetProbeBackingBlock(this->GetProbeBackingBlock()->Clone(scriptContext->GetRecycler()));
                newFunctionBody->m_sourceInfo.m_probeCount = m_sourceInfo.m_probeCount;
            }

#ifdef PERF_COUNTERS
            PERF_COUNTER_ADD(Code, DynamicByteCodeSize, byteCodeSize);
            PERF_COUNTER_ADD(Code, TotalByteCodeSize, byteCodeSize);
#endif
            newFunctionBody->SetFrameDisplayRegister(this->GetFrameDisplayRegister());
            newFunctionBody->SetObjectRegister(this->GetObjectRegister());

            StatementMapList * pStatementMaps = this->GetStatementMaps();
            if (pStatementMaps != null)
            {
                if (BinaryFeatureControl::LanguageService())
                {
                    // We don't need to copy this as the original function live longer than the cloned one.
                    newFunctionBody->pStatementMaps = pStatementMaps;
                }
                else
                {
                    Recycler* recycler = newFunctionBody->GetScriptContext()->GetRecycler();
                    StatementMapList * newStatementMaps = RecyclerNew(recycler, StatementMapList, recycler);
                    newFunctionBody->pStatementMaps = newStatementMaps;
                    pStatementMaps->Map([recycler, newStatementMaps](int index, FunctionBody::StatementMap* oldStatementMap)
                    {
                        FunctionBody::StatementMap* newStatementMap = StatementMap::New(recycler);
                        *newStatementMap = *oldStatementMap;
                        newStatementMaps->Add(newStatementMap);
                    });
                }
            }

            if (this->m_sourceInfo.pSpanSequence != null)
            {
                // Span sequence is heap allocated
                newFunctionBody->m_sourceInfo.pSpanSequence = this->m_sourceInfo.pSpanSequence->Clone();
            }

            Assert(newFunctionBody->GetDirectEntryPoint(newFunctionBody->GetDefaultEntryPointInfo()) == scriptContext->CurrentThunk);
            Assert(newFunctionBody->IsInterpreterThunk());
        }

        // Create a new inline cache
        newFunctionBody->inlineCacheCount = this->inlineCacheCount;
        newFunctionBody->rootObjectLoadInlineCacheStart = this->rootObjectLoadInlineCacheStart;
        newFunctionBody->rootObjectStoreInlineCacheStart = this->rootObjectStoreInlineCacheStart;
        newFunctionBody->isInstInlineCacheCount = this->isInstInlineCacheCount;
        newFunctionBody->referencedPropertyIdCount = this->referencedPropertyIdCount;
        newFunctionBody->AllocateInlineCache();

        newFunctionBody->objLiteralCount = this->objLiteralCount;
        newFunctionBody->AllocateObjectLiteralTypeArray();

        newFunctionBody->simpleJitEntryPointInfo = null;
        newFunctionBody->loopInterpreterLimit = loopInterpreterLimit;
        newFunctionBody->ReinitializeExecutionModeAndLimits();

        // Clone literal regexes
        newFunctionBody->literalRegexCount = this->literalRegexCount;
        newFunctionBody->AllocateLiteralRegexArray();
        for(uint i = 0; i < this->literalRegexCount; ++i)
        {
            const auto literalRegex = this->literalRegexes[i];
            if(!literalRegex)
            {
                Assert(!newFunctionBody->GetLiteralRegex(i));
                continue;
            }

            if (BinaryFeatureControl::LanguageService())
            {
                newFunctionBody->SetLiteralRegex(i, scriptContext->CopyPattern(literalRegex));
            }
            else
            {
                const auto source = literalRegex->GetSource();
                newFunctionBody->SetLiteralRegex(
                    i,
                    RegexHelper::CompileDynamic(
                        scriptContext,
                        source.GetBuffer(),
                        source.GetLength(),
                        literalRegex->GetFlags(),
                        true));
            }
        }

        if (this->DoJITLoopBody())
        {
            newFunctionBody->AllocateLoopHeaders();

            for (uint i = 0; i < this->GetLoopCount(); i++)
            {
                newFunctionBody->GetLoopHeader(i)->startOffset = GetLoopHeader(i)->startOffset;
                newFunctionBody->GetLoopHeader(i)->endOffset = GetLoopHeader(i)->endOffset;
            }
        }

        newFunctionBody->serializationIndex = this->serializationIndex;
        newFunctionBody->m_isFromNativeCodeModule = this->m_isFromNativeCodeModule;
    }

    FunctionBody *
    FunctionBody::Clone(ScriptContext * scriptContext, uint sourceIndex)
    {
#ifdef ENABLE_PREJIT
        bool isNested = sourceIndex != Constants::InvalidSourceIndex;
#endif
        Utf8SourceInfo* sourceInfo = NULL;
        if(sourceIndex == Constants::InvalidSourceIndex)
        {
            // If we're copying a source info across script contexts, we need
            // to create a copy of the Utf8SourceInfo (just the structure, not the source code itself)
            // because a Utf8SourceInfo must reference only function bodies created within that script
            // context
            Utf8SourceInfo* oldSourceInfo = GetUtf8SourceInfo();
            SRCINFO* srcInfo = GetHostSrcInfo()->Clone(scriptContext);
            sourceInfo = scriptContext->CloneSourceCrossContext(oldSourceInfo, srcInfo);
            sourceIndex = scriptContext->SaveSourceNoCopy(sourceInfo, oldSourceInfo->GetCchLength(), oldSourceInfo->GetIsCesu8());
        }
        else
        {
            sourceInfo = scriptContext->GetSource(sourceIndex);
        }

        FunctionBody * newFunctionBody = FunctionBody::NewFromRecycler(scriptContext, this->GetDisplayName(), this->GetDisplayNameLength(),
                this->m_nestedCount, sourceInfo, this->m_functionNumber, this->m_uScriptId, 
                this->GetLocalFunctionId(), this->m_boundPropertyRecords,
                this->GetAttributes()
#ifdef PERF_COUNTERS
                , false
#endif
                );


        if (BinaryFeatureControl::LanguageService())
            scriptContext->RecordFunctionClone(this, newFunctionBody);

        if (this->m_scopeInfo != NULL)
        {
            newFunctionBody->SetScopeInfo(m_scopeInfo->CloneFor(newFunctionBody));
        }

        newFunctionBody->CloneSourceInfo(scriptContext, (*this), this->m_scriptContext, sourceIndex);
        CloneByteCodeInto(scriptContext, newFunctionBody, sourceIndex);

#if DBG
        newFunctionBody->m_iProfileSession = this->m_iProfileSession;
        newFunctionBody->deferredParseNextFunctionId = this->deferredParseNextFunctionId;
#endif

        if (this->HasDynamicProfileInfo())
        {
            newFunctionBody->EnsureDynamicProfileInfo();
        }

        newFunctionBody->byteCodeCache = this->byteCodeCache;

#ifdef ENABLE_NATIVE_CODEGEN
        if (newFunctionBody->GetByteCode() && (IsIntermediateCodeGenThunk(this->GetOriginalEntryPoint())
            || IsNativeOriginalEntryPoint()))
        {
#ifdef ENABLE_PREJIT
            if (Js::Configuration::Global.flags.Prejit)
            {
                if (!isNested)
                {
                    GenerateAllFunctions(scriptContext->GetNativeCodeGenerator(), newFunctionBody);
                }
            }
            else
#endif
            {
                GenerateFunction(scriptContext->GetNativeCodeGenerator(), newFunctionBody);
            }
        }
#endif
        return newFunctionBody;
    }

    void FunctionBody::SetStackNestedFuncParent(FunctionBody * parentFunctionBody)
    {
        Assert(this->stackNestedFuncParent == null);
        Assert(CanDoStackNestedFunc());
        Assert(parentFunctionBody->DoStackNestedFunc());
        this->stackNestedFuncParent = this->GetScriptContext()->GetRecycler()->CreateWeakReferenceHandle(parentFunctionBody);
    }

    FunctionBody * FunctionBody::GetStackNestedFuncParent()
    {
        Assert(this->stackNestedFuncParent);
        return this->stackNestedFuncParent->Get();
    }

    FunctionBody * FunctionBody::GetAndClearStackNestedFuncParent()
    {
        if (this->stackNestedFuncParent)
        {
            FunctionBody * parentFunctionBody = GetStackNestedFuncParent();
            ClearStackNestedFuncParent();
            return parentFunctionBody;
        }
        return null;
    }

    void FunctionBody::ClearStackNestedFuncParent()
    {
        this->stackNestedFuncParent = null;
    }

    ParseableFunctionInfo* ParseableFunctionInfo::CopyFunctionInfoInto(ScriptContext *scriptContext, Js::ParseableFunctionInfo* newFunctionInfo, uint sourceIndex)
    {
        newFunctionInfo->m_inParamCount = this->m_inParamCount;
        newFunctionInfo->m_grfscr = this->m_grfscr;

        newFunctionInfo->m_isDeclaration = this->m_isDeclaration;
        newFunctionInfo->m_hasImplicitArgIns = this->m_hasImplicitArgIns;
        newFunctionInfo->m_isAccessor = this->m_isAccessor;
        newFunctionInfo->m_isGlobalFunc = this->m_isGlobalFunc;
        newFunctionInfo->m_dontInline = this->m_dontInline;
        newFunctionInfo->m_isTopLevel = this->m_isTopLevel;
        newFunctionInfo->m_isPublicLibraryCode = this->m_isPublicLibraryCode;

        newFunctionInfo->scopeSlotArraySize = this->scopeSlotArraySize;

        for (uint index = 0; index < this->m_nestedCount; index++)
        {
            FunctionProxy* proxy = this->GetNestedFunc(index);
            if (proxy)
            {
                // Deserialize the proxy here if we have to
                // TODO: may be we don't have to?
                ParseableFunctionInfo* body = proxy->EnsureDeserialized();
                FunctionProxy* newBody;
                if (BinaryFeatureControl::LanguageService())
                {
                    newBody = scriptContext->CopyFunction(body);
                }
                else
                {
                    if (body->IsDeferredParseFunction())
                    {
                        newBody = body->Clone(scriptContext, sourceIndex);
                    }
                    else
                    {
                        newBody = body->GetFunctionBody()->Clone(scriptContext, sourceIndex);
                    }
                }
                // 0u is an empty value for the bit-mask 'flags', when initially parsing this is used to track defer-parse functions.
                newFunctionInfo->SetNestedFunc(newBody, index, 0u);
            }
            else
            {
                // 0u is an empty value for the bit-mask 'flags', when initially parsing this is used to track defer-parse functions.
                newFunctionInfo->SetNestedFunc(NULL, index, 0u);
            }
        }

        return newFunctionInfo;
    }

    ParseableFunctionInfo* ParseableFunctionInfo::Clone(ScriptContext *scriptContext, uint sourceIndex)
    {
        Utf8SourceInfo* sourceInfo = NULL;
        if(sourceIndex == Constants::InvalidSourceIndex)
        {
            // If we're copying a source info across script contexts, we need
            // to create a copy of the Utf8SourceInfo (just the structure, not the source code itself)
            // because a Utf8SourceInfo must reference only function bodies created within that script
            // context
            Utf8SourceInfo* oldSourceInfo = GetUtf8SourceInfo();
            SRCINFO* srcInfo = GetHostSrcInfo()->Clone(scriptContext);
            sourceInfo = scriptContext->CloneSourceCrossContext(oldSourceInfo, srcInfo);
            sourceIndex = scriptContext->SaveSourceNoCopy(sourceInfo, oldSourceInfo->GetCchLength(), oldSourceInfo->GetIsCesu8());
        }
        else
        {
            sourceInfo = scriptContext->GetSource(sourceIndex);
        }

        ParseableFunctionInfo* newFunctionInfo = ParseableFunctionInfo::New(scriptContext, this->m_nestedCount, this->GetLocalFunctionId(), sourceInfo, this->GetDisplayName(), this->GetDisplayNameLength(), this->m_boundPropertyRecords, this->GetAttributes());

        if (BinaryFeatureControl::LanguageService())
            scriptContext->RecordFunctionClone(this, newFunctionInfo);

        if (this->m_scopeInfo != NULL)
        {
            newFunctionInfo->SetScopeInfo(m_scopeInfo->CloneFor(newFunctionInfo));
        }

        newFunctionInfo->CloneSourceInfo(scriptContext, (*this), this->m_scriptContext, sourceIndex);
        CopyFunctionInfoInto(scriptContext, newFunctionInfo, sourceIndex);

#if DBG
        newFunctionInfo->deferredParseNextFunctionId = this->deferredParseNextFunctionId;
#endif

        return newFunctionInfo;
    }

    void FunctionBody::CopyUndeferredInto(ScriptContext *scriptContext, FunctionBody *newFunctionBody, uint sourceIndex)
    {
        VERIFY_COPY_ON_WRITE_ENABLED();

        Assert(scriptContext);
        Assert(newFunctionBody);

        newFunctionBody->SetInitialDefaultEntryPoint();

        newFunctionBody->CloneSourceInfo(scriptContext, (*this), this->m_scriptContext, sourceIndex);
        CloneByteCodeInto(scriptContext, newFunctionBody, this->GetSourceIndex());

        // Since the original function (this) was defer-parsed when the new function body was created,
        // the new function body would not have been registered with the utf8SourceInfo. However,
        // when CopyUndeferredInto was called, the original function body got parsed and did get
        // registered with the utf8SourceInfo. We need to register the new one too now.
        Utf8SourceInfo* newFunctionSourceInfo = newFunctionBody->GetUtf8SourceInfo();
        LocalFunctionId newFunctionId = newFunctionBody->GetLocalFunctionId();

        Assert(newFunctionId == this->GetLocalFunctionId());
        Assert(newFunctionSourceInfo != this->m_utf8SourceInfo);
    }

    void FunctionBody::CreateCacheIdToPropertyIdMap(uint rootObjectLoadInlineCacheStart, uint rootObjectLoadMethodInlineCacheStart,
        uint rootObjectStoreInlineCacheStart,
        uint totalFieldAccessInlineCacheCount, uint isInstInlineCacheCount)
    {
        Assert(this->rootObjectLoadInlineCacheStart == 0);
        Assert(this->rootObjectLoadMethodInlineCacheStart == 0);
        Assert(this->rootObjectStoreInlineCacheStart == 0);
        Assert(this->inlineCacheCount == 0);
        Assert(this->isInstInlineCacheCount == 0);

        this->rootObjectLoadInlineCacheStart = rootObjectLoadInlineCacheStart;
        this->rootObjectLoadMethodInlineCacheStart = rootObjectLoadMethodInlineCacheStart;
        this->rootObjectStoreInlineCacheStart = rootObjectStoreInlineCacheStart;
        this->inlineCacheCount = totalFieldAccessInlineCacheCount;
        this->isInstInlineCacheCount = isInstInlineCacheCount;

        this->CreateCacheIdToPropertyIdMap();
    }

    void FunctionBody::CreateCacheIdToPropertyIdMap()
    {
        Assert(this->cacheIdToPropertyIdMap == null);
        Assert(this->inlineCaches == null);
        uint count = this->GetInlineCacheCount() ;
        if (count!= 0)
        {
            this->cacheIdToPropertyIdMap =
                RecyclerNewArrayLeaf(this->m_scriptContext->GetRecycler(), PropertyId, count);
#if DBG
            for (uint i = 0; i < count; i++)
            {
                this->cacheIdToPropertyIdMap[i] = Js::Constants::NoProperty;
            }
#endif
        }

    }

#if DBG
    void FunctionBody::VerifyCacheIdToPropertyIdMap()
    {
        uint count = this->GetInlineCacheCount();
        for (uint i = 0; i < count; i++)
        {
            Assert(this->cacheIdToPropertyIdMap[i] != Js::Constants::NoProperty);
        }
    }
#endif

    void FunctionBody::SetPropertyIdForCacheId(uint cacheId, PropertyId propertyId)
    {
        Assert(this->cacheIdToPropertyIdMap != null);
        Assert(cacheId < this->GetInlineCacheCount());
        Assert(this->cacheIdToPropertyIdMap[cacheId] == Js::Constants::NoProperty);

        this->cacheIdToPropertyIdMap[cacheId] = propertyId;
    }

    void FunctionBody::CreateReferencedPropertyIdMap(uint referencedPropertyIdCount)
    {
        this->referencedPropertyIdCount = referencedPropertyIdCount;
        this->CreateReferencedPropertyIdMap();
    }

    void FunctionBody::CreateReferencedPropertyIdMap()
    {
        Assert(this->referencedPropertyIdMap == null);
        uint count = this->GetReferencedPropertyIdCount();
        if (count!= 0)
        {
            this->referencedPropertyIdMap =
                RecyclerNewArrayLeaf(this->m_scriptContext->GetRecycler(), PropertyId, count);
#if DBG
            for (uint i = 0; i < count; i++)
            {
                this->referencedPropertyIdMap[i] = Js::Constants::NoProperty;
            }
#endif
        }
    }

#if DBG
    void FunctionBody::VerifyReferencedPropertyIdMap()
    {
        uint count = this->GetReferencedPropertyIdCount();
        for (uint i = 0; i < count; i++)
        {
            Assert(this->referencedPropertyIdMap[i] != Js::Constants::NoProperty);
        }
    }
#endif

    PropertyId FunctionBody::GetReferencedPropertyId(uint index)
    {
        if (index < (uint)TotalNumberOfBuiltInProperties)
        {
            return index;
        }
        uint mapIndex = index - TotalNumberOfBuiltInProperties;
        return GetReferencedPropertyIdWithMapIndex(mapIndex);
    }

    PropertyId FunctionBody::GetReferencedPropertyIdWithMapIndex(uint mapIndex)
    {
        Assert(this->referencedPropertyIdMap);
        Assert(mapIndex < this->GetReferencedPropertyIdCount());
        return this->referencedPropertyIdMap[mapIndex];
    }

    void FunctionBody::SetReferencedPropertyIdWithMapIndex(uint mapIndex, PropertyId propertyId)
    {
        Assert(propertyId >= TotalNumberOfBuiltInProperties);
        Assert(mapIndex < this->GetReferencedPropertyIdCount());
        Assert(this->referencedPropertyIdMap != null);
        Assert(this->referencedPropertyIdMap[mapIndex] == Js::Constants::NoProperty);
        this->referencedPropertyIdMap[mapIndex] = propertyId;
    }

    void FunctionBody::CreateConstantTable()
    {
        Assert(this->m_constTable == null);
        Assert(m_constCount > FirstRegSlot);

        this->m_constTable = RecyclerNewArrayZ(this->m_scriptContext->GetRecycler(), Var, m_constCount);

        // Initialize with the root object, which will always be recorded here.
        Js::RootObjectBase * rootObject = this->LoadRootObject();
        if (rootObject)
        {
            this->RecordConstant(RootObjectRegSlot, rootObject);
        }
        else
        {
            // This shouldn't happen
            Assert(false);
            this->RecordConstant(RootObjectRegSlot, this->m_scriptContext->GetLibrary()->GetUndefined());
        }

    }

    void FunctionBody::RecordConstant(RegSlot location, Var var)
    {
        Assert(location < m_constCount);
        Assert(this->m_constTable);
        Assert(var != NULL);
        Assert(this->m_constTable[location - FunctionBody::FirstRegSlot] == NULL);
        this->m_constTable[location - FunctionBody::FirstRegSlot] = var;
    }

    void FunctionBody::RecordNullObject(RegSlot location)
    {
        ScriptContext *scriptContext = this->GetScriptContext();
        Var nullObject = JavascriptOperators::OP_LdNull(scriptContext);
        this->RecordConstant(location, nullObject);
    }

    void FunctionBody::RecordUndefinedObject(RegSlot location)
    {
        ScriptContext *scriptContext = this->GetScriptContext();
        Var undefObject = JavascriptOperators::OP_LdUndef(scriptContext);
        this->RecordConstant(location, undefObject);
    }

    void FunctionBody::RecordTrueObject(RegSlot location)
    {
        ScriptContext *scriptContext = this->GetScriptContext();
        Var trueObject = JavascriptBoolean::OP_LdTrue(scriptContext);
        this->RecordConstant(location, trueObject);
    }

    void FunctionBody::RecordFalseObject(RegSlot location)
    {
        ScriptContext *scriptContext = this->GetScriptContext();
        Var falseObject = JavascriptBoolean::OP_LdFalse(scriptContext);
        this->RecordConstant(location, falseObject);
    }

    void FunctionBody::RecordIntConstant(RegSlot location, unsigned int val)
    {
        ScriptContext *scriptContext = this->GetScriptContext();
        Var intConst = JavascriptNumber::ToVar((int32)val, scriptContext);
        this->RecordConstant(location, intConst);
    }

    void FunctionBody::RecordStrConstant(RegSlot location, LPCOLESTR psz, ulong cch)
    {
        ScriptContext *scriptContext = this->GetScriptContext();
        PropertyRecord const * propertyRecord;
        scriptContext->FindPropertyRecord(psz, cch, &propertyRecord);
        Var str;
        if (propertyRecord == null)
        {
            str = JavascriptString::NewCopyBuffer(psz, cch, scriptContext);
        }
        else
        {
            // If a particular string constant already has a propertyId, just create a property string for it
            // as it might be likely that it is used for a property lookup
            str = scriptContext->GetPropertyString(propertyRecord->GetPropertyId());
        }
        this->RecordConstant(location, str);
    }

    void FunctionBody::RecordFloatConstant(RegSlot location, double d)
    {
        ScriptContext *scriptContext = this->GetScriptContext();
        Var floatConst = JavascriptNumber::ToVarIntCheck(d, scriptContext);

        this->RecordConstant(location, floatConst);
    }

    void FunctionBody::RecordNullDisplayConstant(RegSlot location)
    {
        this->RecordConstant(location, (Js::Var)&Js::NullFrameDisplay);
    }

    void FunctionBody::RecordStrictNullDisplayConstant(RegSlot location)
    {
        this->RecordConstant(location, (Js::Var)&Js::StrictNullFrameDisplay);
    }

    void FunctionBody::InitConstantSlots(Var *dstSlots)
    {
        // Initialize the given slots from the constant table.

        Assert(m_constCount > FunctionBody::FirstRegSlot);

        js_memcpy_s(dstSlots, (m_constCount - FunctionBody::FirstRegSlot) * sizeof(Var), this->m_constTable, (m_constCount - FunctionBody::FirstRegSlot) * sizeof(Var));
    }


    Var FunctionBody::GetConstantVar(RegSlot location)
    {
        Assert(this->m_constTable);
        Assert(location < m_constCount);
        Assert(location != 0);

        return this->m_constTable[location - FunctionBody::FirstRegSlot];
    }


    void FunctionBody::CloneConstantTable(FunctionBody *newFunc)
    {
        // Creating the constant table initializes the root object.
        newFunc->CreateConstantTable();

        // Start walking the slots after the root object.
        for (RegSlot reg = FunctionBody::RootObjectRegSlot + 1; reg < m_constCount; reg++)
        {
            Var oldVar = this->GetConstantVar(reg);
            Assert(oldVar != null);
            if (TaggedInt::Is(oldVar))
            {
                newFunc->RecordIntConstant(reg, TaggedInt::ToInt32(oldVar));
            }
            else if (oldVar == &Js::NullFrameDisplay)
            {
                newFunc->RecordNullDisplayConstant(reg);
            }
            else if (oldVar == &Js::StrictNullFrameDisplay)
            {
                newFunc->RecordStrictNullDisplayConstant(reg);
            }
            else
            {
                switch (JavascriptOperators::GetTypeId(oldVar))
                {
                case Js::TypeIds_Undefined:
                    newFunc->RecordUndefinedObject(reg);
                    break;

                case Js::TypeIds_Null:
                    newFunc->RecordNullObject(reg);
                    break;

                case Js::TypeIds_Number:
                    newFunc->RecordFloatConstant(reg, JavascriptNumber::GetValue(oldVar));
                    break;

                case Js::TypeIds_String:
                {
                    JavascriptString *str = JavascriptString::FromVar(oldVar);
                    newFunc->RecordStrConstant(reg, str->GetSz(), str->GetLength());
                    break;
                }
                case Js::TypeIds_ES5Array:
                    newFunc->RecordConstant(reg, oldVar);
                    break;
                case Js::TypeIds_Boolean:
                    if (Js::JavascriptBoolean::FromVar(oldVar)->GetValue())
                    {
                        newFunc->RecordTrueObject(reg);
                    }
                    else
                    {
                        newFunc->RecordFalseObject(reg);
                    }
                    break;

                default:
                    AssertMsg(UNREACHED, "Unexpected object type in CloneConstantTable");
                    break;
                }
            }
        }
    }

#if DBG_DUMP
    void FunctionBody::Dump()
    {
        Js::ByteCodeDumper::Dump(this);
    }

    void FunctionBody::DumpScopes()
    {
        if(this->GetScopeObjectChain())
        {
            wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];

            Output::Print(L"%s (%s) :\n", this->GetDisplayName(), this->GetDebugNumberSet(debugStringBuffer));
            this->GetScopeObjectChain()->pScopeChain->Map( [=] (uint index, DebuggerScope* scope )
            {
                scope->Dump();
            });
        }
    }

    void EntryPointInfo::DumpNativeOffsetMaps()
    {
        // Native Offsets
        if (this->nativeOffsetMaps.Count() > 0)
        {
            Output::Print(L"Native Map: baseAddr: 0x%0Ix, size: 0x%0Ix\nstatementId, offset range, address range\n",
                          this->GetNativeAddress(),
                          this->GetCodeSize());


            int count = this->nativeOffsetMaps.Count();
            for(int i = 0; i < count; i++)
            {
                const NativeOffsetMap* map = &this->nativeOffsetMaps.Item(i);

                Output::Print(L"S%4d, (%5d, %5d)  (0x%012Ix, 0x%012Ix)\n", map->statementIndex,
                                                      map->nativeOffsetSpan.begin,
                                                      map->nativeOffsetSpan.end,
                                                      map->nativeOffsetSpan.begin + this->GetNativeAddress(),
                                                      map->nativeOffsetSpan.end + this->GetNativeAddress());
            }
        }
    }

    void FunctionBody::DumpStatementMaps()
    {
        // Source Map to ByteCode
        StatementMapList * pStatementMaps = this->GetStatementMaps();
        if (pStatementMaps)
        {
            Output::Print(L"Statement Map:\nstatementId, SourceSpan, ByteCodeSpan\n");
            int count = pStatementMaps->Count();
            for(int i = 0; i < count; i++)
            {
                StatementMap* map = pStatementMaps->Item(i);

                Output::Print(L"S%4d, (C%5d, C%5d)  (B%5d, B%5d) Inner=%d\n", i,
                                                      map->sourceSpan.begin,
                                                      map->sourceSpan.end,
                                                      map->byteCodeSpan.begin,
                                                      map->byteCodeSpan.end,
                                                      map->isSubexpression);
            }
        }
    }

    void EntryPointInfo::DumpNativeThrowSpanSequence()
    {
        // Native Throw Map
        if (this->nativeThrowSpanSequence)
        {
            Output::Print(L"Native Throw Map: baseAddr: 0x%0Ix, size: 0x%Ix\nstatementId, offset range, address range\n",
                          this->GetNativeAddress(),
                          this->GetCodeSize());

            int count = this->nativeThrowSpanSequence->Count();
            SmallSpanSequenceIter iter;
            for (int i = 0; i < count; i++)
            {
                StatementData data;
                if (this->nativeThrowSpanSequence->Item(i, iter, data))
                {
                    Output::Print(L"S%4d, (%5d -----)  (0x%012Ix --------)\n", data.sourceBegin, // statementIndex
                        data.bytecodeBegin, // nativeOffset
                        data.bytecodeBegin + this->GetNativeAddress());
                }
            }
        }
    }

    void FunctionBody::PrintStatementSourceLine(uint statementIndex)
    {
        const size_t startOffset = GetStatementStartOffset(statementIndex);

        // startOffset should only be 0 if statementIndex is 0, otherwise it is EOF and we should skip printing anything
        if (startOffset != 0 || statementIndex == 0)
        {
            PrintStatementSourceLineFromStartOffset(startOffset);
        }
    }

    void FunctionBody::PrintStatementSourceLineFromStartOffset(uint cchStartOffset)
    {        
        ULONG line;
        LONG col;

        LPCUTF8 source = GetStartOfDocument(L"FunctionBody::PrintStatementSourceLineFromStartOffset");
        Utf8SourceInfo* sourceInfo = this->GetUtf8SourceInfo();
        Assert(sourceInfo != nullptr);
        LPCUTF8 sourceInfoSrc = sourceInfo->GetSource(L"FunctionBody::PrintStatementSourceLineFromStartOffset");
        if(!sourceInfoSrc)
        {
            Assert(sourceInfo->GetIsLibraryCode());
            return;
        }
        if( source != sourceInfoSrc )
        {
            Output::Print(L"\nDETECTED MISMATCH:\n");
            Output::Print(L"GetUtf8SourceInfo()->GetSource(): 0x%08X: %.*s ...\n", sourceInfo, 16, sourceInfo);
            Output::Print(L"GetStartOfDocument():             0x%08X: %.*s ...\n", source, 16, source);

            AssertMsg(false, "Non-matching start of document");
        }

        GetLineCharOffsetFromStartChar(cchStartOffset, &line, &col, false /*canAllocateLineCache*/);

        WORD color = 0;
        if (Js::Configuration::Global.flags.DumpLineNoInColor)
        {
            color = Output::SetConsoleForeground(12);
        }
        Output::Print(L"\n\n  Line %3d: ", line + 1);
        // Need to match up cchStartOffset to appropriate cbStartOffset given function's cbStartOffset and cchStartOffset
        size_t i = utf8::CharacterIndexToByteIndex(source, sourceInfo->GetCbLength(), cchStartOffset, this->m_cbStartOffset, this->m_cchStartOffset);

        size_t lastOffset = StartOffset() + LengthInBytes();
        for (;i < lastOffset && source[i] != '\n' && source[i] != '\r'; i++)
        {
            Output::Print(L"%C", source[i]);
        }
        Output::Print(L"\n");
        Output::Print(L"  Col %4d:%s^\n", col + 1, ((col+1)<10000) ? L" " : L"");

        if (color != 0)
        {
            Output::SetConsoleForeground(color);
        }
    }
#endif // DBG_DUMP

    /**
     * Get the source code offset for the given <statementIndex>.
     */
    size_t FunctionBody::GetStatementStartOffset(const uint statementIndex)
    {
        size_t startOffset = 0;

        if (statementIndex != Js::Constants::NoStatementIndex)
        {
            const Js::FunctionBody::SourceInfo * sourceInfo = &(this->m_sourceInfo);
            if (sourceInfo->pSpanSequence != null)
            {
                Js::SmallSpanSequenceIter iter;
                sourceInfo->pSpanSequence->Reset(iter);
                Js::StatementData data;
                sourceInfo->pSpanSequence->Item(statementIndex, iter, data);
                startOffset = data.sourceBegin;
            }
            else
            {
                int index = statementIndex;
                Js::FunctionBody::StatementMap * statementMap = GetNextNonSubexpressionStatementMap(GetStatementMaps(), index);
                startOffset = statementMap->sourceSpan.Begin();
            }
        }

        return startOffset;
    }


#ifdef IR_VIEWER
/* BEGIN potentially reusable code */

/*
    This code could be reused for locating source code in a debugger or to
    retrieve the text of source statements.

    Currently this code is used to retrieve the text of a source code statement
    in the IR_VIEWER feature.
*/

    /**
     * Given a statement's starting offset in the source code, calculate the beginning and end of a statement,
     * as well as the line and column number where the statement appears.
     *
     * @param startOffset (input) The offset into the source code where this statement begins.
     * @param sourceBegin (output) The beginning of the statement in the source string.
     * @param sourceEnd (output) The end of the statement in the source string.
     * @param line (output) The line number where the statement appeared in the source.
     * @param col (output) The column number where the statement appeared in the source.
     */
    void FunctionBody::GetSourceLineFromStartOffset(const uint startOffset, LPCUTF8 *sourceBegin, LPCUTF8 *sourceEnd,
                                                    ULONG * line, LONG * col)
    {
        //
        // get source info
        //

        LPCUTF8 source = GetStartOfDocument(L"IR Viewer FunctionBody::GetSourceLineFromStartOffset");
        Utf8SourceInfo* sourceInfo = this->GetUtf8SourceInfo();
        Assert(sourceInfo != nullptr);
        LPCUTF8 sourceInfoSrc = sourceInfo->GetSource(L"IR Viewer FunctionBody::GetSourceLineFromStartOffset");
        if (!sourceInfoSrc)
        {
            Assert(sourceInfo->GetIsLibraryCode());
            return;
        }
        if (source != sourceInfoSrc)
        {
            Output::Print(L"\nDETECTED MISMATCH:\n");
            Output::Print(L"GetUtf8SourceInfo()->GetSource(): 0x%08X: %.*s ...\n", sourceInfo, 16, sourceInfo);
            Output::Print(L"GetStartOfDocument():             0x%08X: %.*s ...\n", source, 16, source);

            AssertMsg(false, "Non-matching start of document");
        }

        //
        // calculate source line info
        //

        size_t cbStartOffset = utf8::CharacterIndexToByteIndex(source, sourceInfo->GetCbLength(), (const charcount_t)startOffset, (size_t)this->m_cbStartOffset, (charcount_t)this->m_cchStartOffset);;
        GetLineCharOffsetFromStartChar(startOffset, line, col);

        size_t lastOffset = StartOffset() + LengthInBytes();
        size_t i = 0;
        for (i = cbStartOffset; i < lastOffset && source[i] != '\n' && source[i] != '\r'; i++)
        {
            // do nothing; scan until end of statement
        }
        size_t cbEndOffset = i;

        //
        // return
        //

        *sourceBegin = &source[cbStartOffset];
        *sourceEnd = &source[cbEndOffset];
    }

    /**
     * Given a statement index and output parameters, calculate the beginning and end of a statement,
     * as well as the line and column number where the statement appears.
     *
     * @param statementIndex (input) The statement's index (as used by the StatementBoundary pragma).
     * @param sourceBegin (output) The beginning of the statement in the source string.
     * @param sourceEnd (output) The end of the statement in the source string.
     * @param line (output) The line number where the statement appeared in the source.
     * @param col (output) The column number where the statement appeared in the source.
     */
    void FunctionBody::GetStatementSourceInfo(const uint statementIndex, LPCUTF8 *sourceBegin, LPCUTF8 *sourceEnd,
        ULONG * line, LONG * col)
    {
        const size_t startOffset = GetStatementStartOffset(statementIndex);

        // startOffset should only be 0 if statementIndex is 0, otherwise it is EOF and we should return empty string
        if (startOffset != 0 || statementIndex == 0)
        {
            GetSourceLineFromStartOffset(startOffset, sourceBegin, sourceEnd, line, col);
        }
        else
        {
            *sourceBegin = NULL;
            *sourceEnd = NULL;
            *line = 0;
            *col = 0;
            return;
        }
    }

/* END potentially reusable code */
#endif /* IR_VIEWER */

#ifdef IR_VIEWER
    Js::DynamicObject * FunctionBody::GetIRDumpBaseObject()
    {
        if (!this->m_irDumpBaseObject)
        {
            this->m_irDumpBaseObject = this->m_scriptContext->GetLibrary()->CreateObject();
        }
        return this->m_irDumpBaseObject;
    }
#endif /* IR_VIEWER */

#ifdef VTUNE_PROFILING
    int EntryPointInfo::GetNativeOffsetMapCount() const
    {
        return this->nativeOffsetMaps.Count();
    }

    uint EntryPointInfo::PopulateLineInfo(void* pInfo, FunctionBody* body)
    {
        LineNumberInfo* pLineInfo = (LineNumberInfo*)pInfo;
        ULONG functionLineNumber = body->GetLineNumber();
        pLineInfo[0].Offset = 0;
        pLineInfo[0].LineNumber = functionLineNumber;

        int lineNumber = 0;
        int j = 1; // start with 1 since offset 0 has already been populated with function line number
        int count = this->nativeOffsetMaps.Count();
        for(int i = 0; i < count; i++)
        {
            const NativeOffsetMap* map = &this->nativeOffsetMaps.Item(i);
            uint32 statementIndex = map->statementIndex;
            if (statementIndex == 0)
            {
                // statementIndex is 0, first line in the function, populate with function line number
                pLineInfo[j].Offset = map->nativeOffsetSpan.begin;
                pLineInfo[j].LineNumber = functionLineNumber;
                j++;
            }

            lineNumber = body->GetSourceLineNumber(statementIndex);
            if (lineNumber != 0)
            {
                pLineInfo[j].Offset = map->nativeOffsetSpan.end;
                pLineInfo[j].LineNumber = lineNumber;
                j++;
            }
        }

        return j;
    }

    ULONG FunctionBody::GetSourceLineNumber(uint statementIndex)
    {
        ULONG line = 0;
        if (statementIndex != Js::Constants::NoStatementIndex)
        {
            size_t startOffset = GetStartOffset(statementIndex);

            if (startOffset != 0 || statementIndex == 0)
            {
                GetLineCharOffsetFromStartChar(startOffset, &line, nullptr, false /*canAllocateLineCache*/);
                line = line + 1;
            }
        }

        return line;
    }

    size_t FunctionBody::GetStartOffset(uint statementIndex) const
    {
        size_t startOffset = 0;

        const Js::FunctionBody::SourceInfo * sourceInfo = &this->m_sourceInfo;
        if (sourceInfo->pSpanSequence != null)
        {
            Js::SmallSpanSequenceIter iter;
            sourceInfo->pSpanSequence->Reset(iter);
            Js::StatementData data;
            sourceInfo->pSpanSequence->Item(statementIndex, iter, data);
            startOffset = data.sourceBegin;
        }
        else
        {
            int index = statementIndex;
            Js::FunctionBody::StatementMap * statementMap = GetNextNonSubexpressionStatementMap(GetStatementMaps(), index);
            startOffset = statementMap->sourceSpan.Begin();
        }

        return startOffset;
    }
#endif

    void FunctionBody::SetIsNonUserCode(bool set)
    {
        // Mark current function as a non user code, so that it will participate to distinguish exception thrown kind
        SetFlags(set, Flags_NonUserCode);

        // Propagate setting for all functions in this scope (nested).
        for (uint uIndex = 0; uIndex < this->m_nestedCount; uIndex++)
        {
            Js::FunctionBody * pBody = this->GetNestedFunc(uIndex)->GetFunctionBody();
            if (pBody != NULL)
            {
                pBody->SetIsNonUserCode(set);
            }
        }
    }

    void FunctionBody::InsertSymbolToRegSlotList(JsUtil::CharacterBuffer<WCHAR> const& propName, RegSlot reg, RegSlot totalRegsCount)
    {
        if (totalRegsCount > 0)
        {
            PropertyId propertyId = GetOrAddPropertyIdTracked(propName);
            InsertSymbolToRegSlotList(reg, propertyId, totalRegsCount);
        }
    }

    void FunctionBody::InsertSymbolToRegSlotList(RegSlot reg, PropertyId propertyId, RegSlot totalRegsCount)
    {
        if (totalRegsCount > 0)
        {
            if (this->propertyIdOnRegSlotsContainer == NULL)
            {
                this->propertyIdOnRegSlotsContainer = PropertyIdOnRegSlotsContainer::New(m_scriptContext->GetRecycler());
            }

            if (this->propertyIdOnRegSlotsContainer->propertyIdsForRegSlots == NULL)
            {
                this->propertyIdOnRegSlotsContainer->CreateRegSlotsArray(m_scriptContext->GetRecycler(), totalRegsCount);
            }

            Assert(this->propertyIdOnRegSlotsContainer != NULL);
            this->propertyIdOnRegSlotsContainer->Insert(reg, propertyId);
        }
    }

    void FunctionBody::SetPropertyIdsOfFormals(PropertyIdArray * formalArgs)
    {
        Assert(formalArgs);
        if (this->propertyIdOnRegSlotsContainer == NULL)
        {
            this->propertyIdOnRegSlotsContainer = PropertyIdOnRegSlotsContainer::New(m_scriptContext->GetRecycler());
        }
        this->propertyIdOnRegSlotsContainer->SetFormalArgs(formalArgs);
    }

    HRESULT FunctionBody::RegisterFunction(BOOL fChangeMode, BOOL fOnlyCurrent)
    {
        if (!this->IsFunctionParsed())
        {
            return S_OK;
        }

        HRESULT hr = this->ReportFunctionCompiled();
        if (FAILED(hr))
        {
            return hr;
        }

        if (fChangeMode)
        {
            this->SetEntryToProfileMode();
        }

        if (!fOnlyCurrent)
        {
            for (uint uIndex = 0; uIndex < this->m_nestedCount; uIndex++)
            {
                Js::ParseableFunctionInfo * pBody = this->GetNestedFunctionForExecution(uIndex);
                if (pBody == NULL || !pBody->IsFunctionParsed())
                {
                    continue;
                }

                hr = pBody->GetFunctionBody()->RegisterFunction(fChangeMode);
                if (FAILED(hr))
                {
                    break;
                }
            }
        }
        return hr;
    }

    HRESULT FunctionBody::ReportScriptCompiled()
    {
        AssertMsg(m_scriptContext != NULL, "Script Context is null when reporting function information");

        PROFILER_SCRIPT_TYPE type = IsDynamicScript() ? PROFILER_SCRIPT_TYPE_DYNAMIC : PROFILER_SCRIPT_TYPE_USER;

        IDebugDocumentContext *pDebugDocumentContext = NULL;
        this->m_scriptContext->GetDocumentContext(this->m_scriptContext, this, &pDebugDocumentContext);

        HRESULT hr = m_scriptContext->OnScriptCompiled((PROFILER_TOKEN) this->GetUtf8SourceInfo()->GetSourceInfoId(), type, pDebugDocumentContext);

        RELEASEPTR(pDebugDocumentContext);

        return hr;
    }

    HRESULT FunctionBody::ReportFunctionCompiled()
    {
        // Some assumptions by Logger interface.
        // to send NULL as a name in case the name is anonymous and hint is anonymous code.

        const wchar_t *pwszName = GetExternalDisplayName();

        IDebugDocumentContext *pDebugDocumentContext = NULL;
        this->m_scriptContext->GetDocumentContext(this->m_scriptContext, this, &pDebugDocumentContext);

        SetHasFunctionCompiledSent(true);

        HRESULT hr = m_scriptContext->OnFunctionCompiled(m_functionNumber, (PROFILER_TOKEN) this->GetUtf8SourceInfo()->GetSourceInfoId(), pwszName, NULL, pDebugDocumentContext);
        RELEASEPTR(pDebugDocumentContext);

#if DBG
        if (m_iProfileSession >= m_scriptContext->GetProfileSession())
        {
            OUTPUT_TRACE_DEBUGONLY(Js::ScriptProfilerPhase, L"FunctionBody::ReportFunctionCompiled, Duplicate compile event (%d < %d) for FunctionNumber : %d\n",
                m_iProfileSession, m_scriptContext->GetProfileSession(), m_functionNumber);
        }

        AssertMsg(m_iProfileSession < m_scriptContext->GetProfileSession(), "Duplicate compile event sent");
        m_iProfileSession = m_scriptContext->GetProfileSession();
#endif

        return hr;
    }

    void FunctionBody::SetEntryToProfileMode()
    {
#ifdef ENABLE_NATIVE_CODEGEN
        AssertMsg(this->m_scriptContext->CurrentThunk == ProfileEntryThunk, "ScriptContext not in profile mode");
#if DBG
        AssertMsg(m_iProfileSession == m_scriptContext->GetProfileSession(), "Changing mode to profile for function that didnt send compile event");
#endif
        // This is always done when bg thread is paused hence we dont need any kind of thread - synchronisation at this point.

        // Change entry points to Profile Thunk
        //  If the entrypoint is CodeGenOnDemand or CodeGen - then we dont change the entry points
        ProxyEntryPointInfo* defaultEntryPointInfo = this->GetDefaultEntryPointInfo();

        if (!IsIntermediateCodeGenThunk((JavascriptMethod) defaultEntryPointInfo->address)
            && defaultEntryPointInfo->address != DynamicProfileInfo::EnsureDynamicProfileInfoThunk)
        {
            if (this->originalEntryPoint == DefaultDeferredParsingThunk)
            {
                defaultEntryPointInfo->address = ProfileDeferredParsingThunk;
            }
            else if (this->originalEntryPoint == DefaultDeferredDeserializeThunk)
            {
                defaultEntryPointInfo->address = ProfileDeferredDeserializeThunk;
            }
            else
            {
                defaultEntryPointInfo->address = ProfileEntryThunk;
            }
        }

        // Update old entry points on the deferred prototype type so that they match current defaultEntryPointInfo.
        // to make sure that new JavascriptFunction instances use profile thunk.
        if (this->deferredPrototypeType)
        {
            this->deferredPrototypeType->SetEntryPoint((JavascriptMethod)this->GetDefaultEntryPointInfo()->address);
            this->deferredPrototypeType->SetEntryPointInfo(this->GetDefaultEntryPointInfo());
        }

#if DBG
        if (!this->HasValidEntryPoint())
        {
            OUTPUT_TRACE_DEBUGONLY(Js::ScriptProfilerPhase, L"FunctionBody::SetEntryToProfileMode, Assert due to HasValidEntryPoint(), directEntrypoint : 0x%0IX, originalentrypoint : 0x%0IX\n",
                (JavascriptMethod)this->GetDefaultEntryPointInfo()->address, this->originalEntryPoint);

            AssertMsg(false, "Not a valid EntryPoint");
        }
#endif

#endif //ENABLE_NATIVE_CODEGEN
    }

#if DBG
    void FunctionBody::MustBeInDebugMode()
    {
        Assert(m_scriptContext->IsInDebugMode());
        Assert(IsByteCodeDebugMode());
        Assert(m_sourceInfo.pSpanSequence == nullptr);
        Assert(pStatementMaps != nullptr);
    }
#endif

    void FunctionBody::CleanupToReparse()
    {
        //
        // The current function is already compiled. In order to prep this function to ready for debug mode, most of the previous information need to be thrown away.

        // Clean up the nested functions
        for (uint i = 0; i < m_nestedCount; i++)
        {
            FunctionProxy* proxy = GetNestedFunc(i);
            if (proxy && proxy->IsFunctionBody())
            {
                proxy->GetFunctionBody()->CleanupToReparse();
            }
        }

        CleanupRecyclerData(/* isShutdown */ false, true /* capture entry point cleanup stack trace */);

        this->entryPoints->ClearAndZero();

        // Store the originalEntryPoint to restore it back immediately.
        JavascriptMethod originalEntryPoint = this->originalEntryPoint;
        this->CreateNewDefaultEntryPoint();
        this->originalEntryPoint = originalEntryPoint;
        if (this->m_defaultEntryPointInfo)
        {
            this->GetDefaultFunctionEntryPointInfo()->entryPointIndex = 0;
        }

        this->auxBlock = NULL;
        this->auxContextBlock = NULL;
        this->byteCodeBlock = NULL;
        this->loopHeaderArray = NULL;
        this->m_constTable = NULL;
        this->m_scopeInfo = NULL;
        this->m_codeGenRuntimeData = NULL;
        this->m_codeGenGetSetRuntimeData = NULL;
        this->cacheIdToPropertyIdMap = NULL;
        this->referencedPropertyIdMap = null;
        this->literalRegexes = NULL;
        this->propertyIdsForScopeSlotArray = NULL;
        this->propertyIdOnRegSlotsContainer = NULL;
        this->pStatementMaps = NULL;

        this->profiledLdElemCount = 0;
        this->profiledStElemCount = 0;
        this->profiledCallSiteCount = 0;
        this->profiledArrayCallSiteCount = 0;
        this->profiledDivOrRemCount = 0;
        this->profiledSwitchCount = 0;
        this->profiledReturnTypeCount = 0;
        this->profiledSlotCount = 0;
        this->loopCount = 0;

        this->m_envDepth = (uint16)-1;

        this->m_byteCodeCount = 0;
        this->m_byteCodeWithoutLDACount = 0;
        this->m_byteCodeInLoopCount = 0;

        this->functionBailOutRecord = NULL;

        this->dynamicProfileInfo = NULL;
        this->hasExecutionDynamicProfileInfo = false;

        this->m_firstTmpReg = Constants::NoRegister;
        this->m_varCount = 0;
        this->m_constCount = 0;
        this->stackClosureRegister = Constants::NoRegister;

        this->ResetObjectLiteralTypes();

        this->inlineCacheCount = 0;
        this->rootObjectLoadInlineCacheStart = 0;
        this->rootObjectLoadMethodInlineCacheStart = 0;
        this->rootObjectStoreInlineCacheStart = 0;
        this->isInstInlineCacheCount = 0;
        this->m_inlineCachesOnFunctionObject = false;
        this->referencedPropertyIdCount = 0;
        this->polymorphicCallSiteInfoHead = null;

        this->interpretedCount = 0;

        this->m_hasDoneAllNonLocalReferenced = false;

        this->debuggerScopeIndex = 0;
        this->m_utf8SourceInfo->DeleteLineOffsetCache();

        // Reset to default.
        this->flags = Flags_HasNoExplicitReturnValue;

        ResetInParams();

        this->m_isAsmjsMode = false;
        this->m_isAsmJsFunction = false;
        this->m_isAsmJsScheduledForFullJIT = false;
        this->m_asmJsTotalLoopCount = NULL;

        recentlyBailedOutOfJittedLoopBody = false;

        loopInterpreterLimit = CONFIG_FLAG(LoopInterpretCount);
        ReinitializeExecutionModeAndLimits();

        Assert(this->m_sourceInfo.m_probeCount == 0);
        this->m_sourceInfo.m_probeBackingBlock = nullptr;

#if DBG
        // This could be non-zero if the function threw exception before. Reset it.
        this->m_DEBUG_executionCount = 0;
#endif
        if (this->m_sourceInfo.pSpanSequence != NULL)
        {
            HeapDelete(this->m_sourceInfo.pSpanSequence);
            this->m_sourceInfo.pSpanSequence = NULL;
        }

        if (this->m_sourceInfo.m_auxStatementData != NULL)
        {
            // This must be consistent with how we allocate the data for this and inner structures.
            // We are using recycler, thus it's enough just to set to NULL.
            Assert(m_scriptContext->GetRecycler()->IsValidObject(m_sourceInfo.m_auxStatementData));
            m_sourceInfo.m_auxStatementData = NULL;
        }
    }

    void FunctionBody::SetEntryToDeferParseForDebugger()
    {
        ProxyEntryPointInfo* defaultEntryPointInfo = this->GetDefaultEntryPointInfo();
        if (defaultEntryPointInfo->address != DefaultDeferredParsingThunk && defaultEntryPointInfo->address != ProfileDeferredParsingThunk)
        {
            // Just change the thunk, the cleanup will be done once the function gets called.
            if (this->m_scriptContext->CurrentThunk == ProfileEntryThunk)
            {
                defaultEntryPointInfo->address = ProfileDeferredParsingThunk;
            }
            else
            {
                defaultEntryPointInfo->address = DefaultDeferredParsingThunk;
            }

            this->originalEntryPoint = DefaultDeferredParsingThunk;

            // Abandon the shared type so a new function will get a new one
            // REVIEW: reuse it and set the entry point?
            // TODO: RTM: make this consistent with the case for deferredPrototypeType in ResetEntryPointForDebugger below.
            this->deferredPrototypeType = null;
            this->attributes = (FunctionInfo::Attributes) (this->attributes | FunctionInfo::Attributes::DeferredParse);
        }

        // Set other state back to before parse as well
        this->SetStackNestedFunc(false);
        this->stackNestedFuncParent = null;
        this->SetReparsed(true);
#if DBG
        wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];
        OUTPUT_VERBOSE_TRACE(Js::DebuggerPhase, L"Regenerate Due To Debug Mode: function %s (%s) from script context %p\n",
            this->GetDisplayName(), this->GetDebugNumberSet(debugStringBuffer), m_scriptContext);
#endif
    }

    //
    // For library code all references to jitted entry points need to be removed
    //
    void FunctionBody::ResetEntryPoint()
    {
        if (this->entryPoints)
        {
            this->MapEntryPoints([] (int index, FunctionEntryPointInfo* entryPoint)
            {
                if (NULL != entryPoint)
                {
                    // Finalize = Free up work item if it hasn't been released yet + clean up
                    // isShutdown is false because cleanup is called only in the !isShutdown case
                    entryPoint->Finalize(/*isShutdown*/ false);
                }
            });

            this->MapLoopHeaders([] (uint loopNumber, LoopHeader* header)
            {
                header->MapEntryPoints([] (int index, LoopEntryPointInfo* entryPoint)
                {
                    entryPoint->Cleanup(/*isShutdown*/ false, true /* capture cleanup stack */);
                });
            });
        }

        this->entryPoints->ClearAndZero();
        this->CreateNewDefaultEntryPoint();
        this->originalEntryPoint = DefaultEntryThunk;
        m_defaultEntryPointInfo->address = m_scriptContext->CurrentThunk;

        if (this->deferredPrototypeType)
        {
            // Update old entry points on the deferred prototype type,
            // as they may point to old native code gen regions which age gone now.
            this->deferredPrototypeType->SetEntryPoint((JavascriptMethod)this->GetDefaultEntryPointInfo()->address);
            this->deferredPrototypeType->SetEntryPointInfo(this->GetDefaultEntryPointInfo());
        }
        ReinitializeExecutionModeAndLimits();
    }

    void FunctionBody::AddDeferParseAttribute()
    {
        this->attributes = (FunctionInfo::Attributes) (this->attributes | DeferredParse);
    }

    void FunctionBody::RemoveDeferParseAttribute()
    {
        this->attributes = (FunctionInfo::Attributes) (this->attributes & (~DeferredParse));
    }

    Js::DebuggerScope * FunctionBody::GetDiagCatchScopeObjectAt(int byteCodeOffset)
    {
        if (GetScopeObjectChain())
        {
            for (int i = 0; i < GetScopeObjectChain()->pScopeChain->Count(); i++)
            {
                Js::DebuggerScope *debuggerScope = GetScopeObjectChain()->pScopeChain->Item(i);
                Assert(debuggerScope);

                if (debuggerScope->IsCatchScope() && debuggerScope->IsOffsetInScope(byteCodeOffset))
                {
                    return debuggerScope;
                }
            }
        }
        return NULL;
    }


    ushort SmallSpanSequence::GetDiff(int current, int prev)
    {
        int diff = current - prev;

        if ((diff) < SHRT_MIN  || (diff) >= SHRT_MAX)
        {
            diff = SHRT_MAX;

            if (!this->pActualOffsetList)
            {
                this->pActualOffsetList = JsUtil::GrowingUint32HeapArray::Create(4);
            }

            this->pActualOffsetList->Add(current);
        }

        return (ushort)diff;
    }

    // Get Values of the beginnig of the statement at particular index.
    BOOL SmallSpanSequence::GetRangeAt(int index, SmallSpanSequenceIter &iter, int * pCountOfMissed, StatementData & data)
    {
        Assert(index < pStatementBuffer->Count());

        SmallSpan span(pStatementBuffer->ItemInBuffer(index));

        int countOfMissed = 0;

        if ((short)span.sourceBegin == SHRT_MAX)
        {
            // Look in ActualOffset store
            Assert(this->pActualOffsetList);
            Assert(this->pActualOffsetList->Count() > 0);
            Assert(this->pActualOffsetList->Count() > iter.indexOfActualOffset);

            data.sourceBegin = this->pActualOffsetList->ItemInBuffer(iter.indexOfActualOffset);
            countOfMissed++;
        }
        else
        {
            data.sourceBegin = iter.accumulatedSourceBegin + (short)span.sourceBegin;
        }

        if (span.bytecodeBegin == SHRT_MAX)
        {
            // Look in ActualOffset store
            Assert(this->pActualOffsetList);
            Assert(this->pActualOffsetList->Count() > 0);
            Assert(this->pActualOffsetList->Count() > iter.indexOfActualOffset + countOfMissed);

            data.bytecodeBegin = this->pActualOffsetList->ItemInBuffer(iter.indexOfActualOffset + countOfMissed);
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

    void SmallSpanSequence::Reset(SmallSpanSequenceIter &iter)
    {
        iter.accumulatedIndex = 0;
        iter.accumulatedSourceBegin = baseValue;
        iter.accumulatedBytecodeBegin = 0;
        iter.indexOfActualOffset = 0;
    }

    BOOL SmallSpanSequence::GetMatchingStatementFromBytecode(int bytecode, SmallSpanSequenceIter &iter, StatementData & data)
    {
        if (Count() > 0 && bytecode >= 0)
        {
            // Support only in forward direction
            if (bytecode < iter.accumulatedBytecodeBegin
                || iter.accumulatedIndex <= 0 || iter.accumulatedIndex >= Count())
            {
                // re-initialize the accumulaters
                Reset(iter);
            }

            while (iter.accumulatedIndex < Count())
            {
                int countOfMissed = 0;
                if (!GetRangeAt(iter.accumulatedIndex, iter, &countOfMissed, data))
                {
                    Assert(FALSE);
                    break;
                }

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

                // Look for the next
                iter.accumulatedSourceBegin = data.sourceBegin;
                iter.accumulatedBytecodeBegin = data.bytecodeBegin;
                iter.accumulatedIndex++;

                if (countOfMissed)
                {
                    iter.indexOfActualOffset += countOfMissed;
                }
            }

            if (iter.accumulatedIndex != -1)
            {
                // Give the last one.
                Assert(data.bytecodeBegin < bytecode);
                return TRUE;
            }
        }

        // Failed to give the correct one, init to default
        iter.accumulatedIndex = -1;
        return FALSE;
    }

    BOOL SmallSpanSequence::Item(int index, SmallSpanSequenceIter &iter, StatementData & data)
    {
        if (!pStatementBuffer || index < 0 || index >= pStatementBuffer->Count())
        {
            return FALSE;
        }

        if (iter.accumulatedIndex <= 0 || iter.accumulatedIndex > index)
        {
            Reset(iter);
        }

        while (iter.accumulatedIndex <= index)
        {
            Assert(iter.accumulatedIndex < pStatementBuffer->Count());

            int countOfMissed = 0;
            if (!GetRangeAt(iter.accumulatedIndex, iter, &countOfMissed, data))
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

    BOOL SmallSpanSequence::Seek(int index, StatementData & data)
    {
        // This method will not alter any state of the variables, so this will just do plain search
        // from the beginning to look for that index.

        SmallSpanSequenceIter iter;
        Reset(iter);

        return Item(index, iter, data);
    }

    SmallSpanSequence * SmallSpanSequence::Clone()
    {
        SmallSpanSequence *pNewSequence = HeapNew(SmallSpanSequence);
        pNewSequence->baseValue = baseValue;

        if (pStatementBuffer)
        {
            pNewSequence->pStatementBuffer = pStatementBuffer->Clone();
        }

        if (pActualOffsetList)
        {
            pNewSequence->pActualOffsetList = pActualOffsetList->Clone();
        }

        return pNewSequence;
    }

    PropertyIdOnRegSlotsContainer * PropertyIdOnRegSlotsContainer::New(Recycler * recycler)
    {
        return RecyclerNew(recycler, PropertyIdOnRegSlotsContainer);
    }

    PropertyIdOnRegSlotsContainer::PropertyIdOnRegSlotsContainer()
        :  propertyIdsForRegSlots(NULL), length(0), propertyIdsForFormalArgs(NULL)
    {
    }

    void PropertyIdOnRegSlotsContainer::CreateRegSlotsArray(Recycler * recycler, uint _length)
    {
        Assert(propertyIdsForRegSlots == NULL);
        propertyIdsForRegSlots = RecyclerNewArrayLeafZ(recycler, PropertyId, _length);
        length = _length;
    }

    void PropertyIdOnRegSlotsContainer::SetFormalArgs(PropertyIdArray * formalArgs)
    {
        propertyIdsForFormalArgs = formalArgs;
    }

    //
    // Helper methods for PropertyIdOnRegSlotsContainer

    void PropertyIdOnRegSlotsContainer::Insert(RegSlot reg, PropertyId propId)
    {
        //
        // Reg is being used as an index;

        Assert(propertyIdsForRegSlots);
        Assert(reg < length);

        //
        // the current reg is unaccounted for const reg count. while fetching calculate the actual regslot value.

        Assert(propertyIdsForRegSlots[reg] == NULL || propertyIdsForRegSlots[reg] == propId);
        propertyIdsForRegSlots[reg] = propId;
    }

    void PropertyIdOnRegSlotsContainer::FetchItemAt(uint index, FunctionBody *pFuncBody, __out PropertyId *pPropId, __out RegSlot *pRegSlot)
    {
        Assert(index < length);
        Assert(pPropId);
        Assert(pRegSlot);
        Assert(pFuncBody);

        *pPropId = propertyIdsForRegSlots[index];
        *pRegSlot = pFuncBody->MapRegSlot(index);
    }

    bool PropertyIdOnRegSlotsContainer::IsRegSlotFormal(RegSlot reg)
    {
        if (propertyIdsForFormalArgs != NULL && reg < length)
        {
            PropertyId propId = propertyIdsForRegSlots[reg];
            for (uint32 i = 0; i < propertyIdsForFormalArgs->count; i++)
            {
                if (propertyIdsForFormalArgs->elements[i] == propId)
                {
                    return true;
                }
            }
        }

        return false;
    }

    ScopeType FrameDisplay::GetScopeType(void* scope)
    {
        if(Js::ActivationObject::Is(scope))
        {
            return ScopeType_ActivationObject;
        }
        if(Js::ScopeSlots::Is(scope))
        {
            return ScopeType_SlotArray;
        }
        return ScopeType_WithScope;
    }

    // DebuggerScope

    // Get the sibling for the current debugger scope.
    DebuggerScope * DebuggerScope::GetSiblingScope(RegSlot location, FunctionBody *functionBody)
    {
        // This is expected to be called only when the current scope is either slot or activation object.
        Assert(scopeType == Js::DiagExtraScopesType::DiagBlockScopeInSlot || scopeType == Js::DiagExtraScopesType::DiagBlockScopeInObject);

        if (siblingScope == nullptr)
        {
            // If the sibling isn't there, attempt to retrieve it if we're reparsing or create it anew if this is the first parse.
            siblingScope = functionBody->RecordStartScopeObject(Js::DiagExtraScopesType::DiagBlockScopeDirect, GetStart(), location);
        }

        return siblingScope;
    }

    // Adds a new property to be tracked in the debugger scope.
    // location     - The slot array index or register slot location of where the property is stored.
    // propertyId   - The property ID of the property.
    // flags        - Flags that help describe the property.
    void DebuggerScope::AddProperty(RegSlot location, Js::PropertyId propertyId, DebuggerScopePropertyFlags flags)
    {
        DebuggerScopeProperty scopeProperty;

        scopeProperty.location = location;
        scopeProperty.propId = propertyId;

        // This offset is uninitialized until the property is initialized (with a ld opcode, for example).
        scopeProperty.byteCodeInitializationOffset = Constants::InvalidByteCodeOffset;
        scopeProperty.flags = flags;

        // Delay allocate the property list so we don't take up memory if there are no properties in this scope.
        // Scopes are created during non-debug mode as well so we want to keep them as small as possible.
        this->EnsurePropertyListIsAllocated();

        // The property doesn't exist yet, so add it.
        this->scopeProperties->Add(scopeProperty);
    }

    bool DebuggerScope::GetPropertyIndex(Js::PropertyId propertyId, int& index)
    {
        if (!this->HasProperties())
        {
            index = -1;
            return false;
        }

        bool found = this->scopeProperties->MapUntil( [&](int i, const DebuggerScopeProperty& scopeProperty) {
            if(scopeProperty.propId == propertyId)
            {
                index = scopeProperty.location;
                return true;
            }
            return false;
        });

        if(!found)
        {
            return false;
        }
        return true;
    }
#if DBG
    void DebuggerScope::Dump()
    {
        int indent = (GetScopeDepth() - 1) * 4;

        Output::Print(indent, L"Begin scope: Address: %p Type: %s Location: %d Sibling: %p Range: [%d, %d]\n ", this, GetDebuggerScopeTypeString(scopeType), scopeLocation, this->siblingScope, range.begin, range.end);
        if (this->HasProperties())
        {
            this->scopeProperties->Map( [=] (int i, Js::DebuggerScopeProperty& scopeProperty) {
                Output::Print(indent, L"%s(%d) Location: %d Const: %s Initialized: %d\n", ThreadContext::GetContextForCurrentThread()->GetPropertyName(scopeProperty.propId)->GetBuffer(),
                    scopeProperty.propId, scopeProperty.location, scopeProperty.IsConst() ? L"true": L"false", scopeProperty.byteCodeInitializationOffset);
            });
        }

        Output::Print(L"\n");
    }

    // Returns the debugger scope type in string format.
    PCWSTR DebuggerScope::GetDebuggerScopeTypeString(DiagExtraScopesType scopeType)
    {
        switch (scopeType)
        {
        case DiagExtraScopesType::DiagBlockScopeDirect:
            return L"DiagBlockScopeDirect";
        case DiagExtraScopesType::DiagBlockScopeInObject:
            return L"DiagBlockScopeInObject";
        case DiagExtraScopesType::DiagBlockScopeInSlot:
            return L"DiagBlockScopeInSlot";
        case DiagExtraScopesType::DiagBlockScopeRangeEnd:
            return L"DiagBlockScopeRangeEnd";
        case DiagExtraScopesType::DiagCatchScopeDirect:
            return L"DiagCatchScopeDirect";
        case DiagExtraScopesType::DiagCatchScopeInObject:
            return L"DiagCatchScopeInObject";
        case DiagExtraScopesType::DiagCatchScopeInSlot:
            return L"DiagCatchScopeInSlot";
        case DiagExtraScopesType::DiagUnknownScope:
            return L"DiagUnknownScope";
        case DiagExtraScopesType::DiagWithScope:
            return L"DiagWithScope";
        default:
            AssertMsg(false, "Missing a debug scope type.");
            return L"";
        }
    }
#endif
    // Updates the current offset of where the property is first initialized.  This is used to
    // detect whether or not a property is in a dead zone when broken in the debugger.
    // location                 - The slot array index or register slot location of where the property is stored.
    // propertyId               - The property ID of the property.
    // byteCodeOffset           - The offset to set the initialization point at.
    // isFunctionDeclaration    - Whether or not the property is a function declaration or not.  Used for verification.
    // <returns>        - True if the property was found and updated for the current scope, else false.
    bool DebuggerScope::UpdatePropertyInitializationOffset(
        RegSlot location,
        Js::PropertyId propertyId,
        int byteCodeOffset,
        bool isFunctionDeclaration /*= false*/)
    {
        if (UpdatePropertyInitializationOffsetInternal(location, propertyId, byteCodeOffset, isFunctionDeclaration))
        {
            return true;
        }
        if (siblingScope != nullptr && siblingScope->UpdatePropertyInitializationOffsetInternal(location, propertyId, byteCodeOffset, isFunctionDeclaration))
        {
            return true;
        }
        return false;
    }

    bool DebuggerScope::UpdatePropertyInitializationOffsetInternal(
        RegSlot location,
        Js::PropertyId propertyId,
        int byteCodeOffset,
        bool isFunctionDeclaration /*= false*/)
    {
        if (scopeProperties == nullptr)
        {
            return false;
        }

        for (int i = 0; i < scopeProperties->Count(); ++i)
        {
            DebuggerScopeProperty propertyItem = scopeProperties->Item(i);
            if (propertyItem.propId == propertyId && propertyItem.location == location)
            {
                if (propertyItem.byteCodeInitializationOffset == Constants::InvalidByteCodeOffset)
                {
                    propertyItem.byteCodeInitializationOffset = byteCodeOffset;
                    scopeProperties->SetExistingItem(i, propertyItem);
                }
#if DBG
                else
                {
                    // If the bytecode initialization offset is not Constants::InvalidByteCodeOffset,
                    // it means we have two or more functions declared in the same scope with the same name
                    // and one has already been marked.  We track each location with a property entry
                    // on the debugging side (when calling DebuggerScope::AddProperty()) as opposed to scanning
                    // and checking if the property already exists each time we add in order to avoid duplicates.
                    AssertMsg(isFunctionDeclaration, "Only function declarations can be defined more than once in the same scope with the same name.");
                    AssertMsg(propertyItem.byteCodeInitializationOffset == byteCodeOffset, "The bytecode offset for all function declarations should be identical for this scope.");
                }
#endif // DBG

                return true;
            }
        }

        return false;
    }

    // Updates the debugger scopes fields due to a regeneration of bytecode (happens during debugger attach or detach, for
    // example).
    void DebuggerScope::UpdateDueToByteCodeRegeneration(DiagExtraScopesType scopeType, int start, RegSlot scopeLocation)
    {
#if DBG
        if (this->scopeType != Js::DiagUnknownScope)
        {
            // If the scope is unknown, it was deserialized without a scope type.  Otherwise, it should not have changed.
            // The scope type can change on a re-parse in certain scenarios related to eval detection in legacy mode -> Winblue: 272122
            AssertMsg(this->scopeType == scopeType, "The debugger scope type should not have changed when generating bytecode again.");
        }
#endif // DBG

        this->scopeType = scopeType;
        this->SetBegin(start);
        if(this->scopeProperties)
        {
            this->scopeProperties->Clear();
        }

        // Reset the scope location as it may have changed during bytecode generation from the last run.
        this->SetScopeLocation(scopeLocation);

        if (siblingScope)
        {
            // If we had a sibling scope during initial parsing, clear it now so that it will be reset
            // when it is retrieved during this bytecode generation pass, in GetSiblingScope().
            // GetSiblingScope() will ensure that the FunctionBody currentDebuggerScopeIndex value is
            // updated accordingly to account for future scopes coming after the sibling.
            // Calling of GetSiblingScope() will happen when register properties are added to this scope
            // via TrackRegisterPropertyForDebugger().
            siblingScope = null;
        }
    }

    void DebuggerScope::EnsurePropertyListIsAllocated()
    {
        if (this->scopeProperties == nullptr)
        {
            this->scopeProperties = RecyclerNew(this->recycler, DebuggerScopePropertyList, this->recycler);
        }
    }

    // Checks if the passed in ByteCodeGenerator offset is in this scope's being/end range.
    bool DebuggerScope::IsOffsetInScope(int offset) const
    {
        Assert(this->range.end != -1);
        return this->range.Includes(offset);
    }

    // Determines if the DebuggerScope contains a property with the passed in ID and
    // location in the internal property list.
    // propertyId       - The ID of the property to search for.
    // location         - The slot array index or register to search for.
    // outScopeProperty - Optional parameter that will return the property, if found.
    bool DebuggerScope::Contains(Js::PropertyId propertyId, RegSlot location) const
    {
        DebuggerScopeProperty tempProperty;
        return TryGetProperty(propertyId, location, &tempProperty);
    }

    // Gets whether or not the scope is a block scope (non-catch or with).
    bool DebuggerScope::IsBlockScope() const
    {
        AssertMsg(this->scopeType != Js::DiagBlockScopeRangeEnd, "Debugger scope type should never be set to range end - only reserved for marking the end of a scope (not persisted).");
        return this->scopeType == Js::DiagBlockScopeDirect
            || this->scopeType == Js::DiagBlockScopeInObject
            || this->scopeType == Js::DiagBlockScopeInSlot
            || this->scopeType == Js::DiagBlockScopeRangeEnd;
    }

    // Gets whether or not the scope is a catch block scope.
    bool DebuggerScope::IsCatchScope() const
    {
        return this->scopeType == Js::DiagCatchScopeDirect
            || this->scopeType == Js::DiagCatchScopeInObject
            || this->scopeType == Js::DiagCatchScopeInSlot;
    }

    // Gets whether or not the scope is a with block scope.
    bool DebuggerScope::IsWithScope() const
    {
        return this->scopeType == Js::DiagWithScope;
    }

    // Gets whether or not the scope is a slot array scope.
    bool DebuggerScope::IsSlotScope() const
    {
        return this->scopeType == Js::DiagBlockScopeInSlot
            || this->scopeType == Js::DiagCatchScopeInSlot;
    }

    // Gets whether or not the scope has any properties in it.
    bool DebuggerScope::HasProperties() const
    {
        return this->scopeProperties && this->scopeProperties->Count() > 0;
    }

    // Checks if this scope is an ancestor of the passed in scope.
    bool DebuggerScope::IsAncestorOf(const DebuggerScope* potentialChildScope)
    {
        if (potentialChildScope == nullptr)
        {
            // If the child scope is null, it represents the global scope which
            // cannot be a child of anything.
            return false;
        }

        const DebuggerScope* currentScope = potentialChildScope;
        while (currentScope)
        {
            if (currentScope->GetParentScope() == this)
            {
                return true;
            }

            currentScope = currentScope->GetParentScope();
        }

        return false;
    }

    // Checks if all properties of the scope are currently in a dead zone given the specified offset.
    bool DebuggerScope::AreAllPropertiesInDeadZone(int byteCodeOffset) const
    {
        if (!this->HasProperties())
        {
            return false;
        }

        return this->scopeProperties->All([&](Js::DebuggerScopeProperty& propertyItem)
            {
                return propertyItem.IsInDeadZone(byteCodeOffset);
            });
    }

    // Attempts to get the specified property.  Returns true if the property was copied to the structure; false otherwise.
    bool DebuggerScope::TryGetProperty(Js::PropertyId propertyId, RegSlot location, DebuggerScopeProperty* outScopeProperty) const
    {
        Assert(outScopeProperty);

        if (scopeProperties == NULL)
        {
            return false;
        }

        for (int i = 0; i < scopeProperties->Count(); ++i)
        {
            DebuggerScopeProperty propertyItem = scopeProperties->Item(i);
            if (propertyItem.propId == propertyId && propertyItem.location == location)
            {
                *outScopeProperty = propertyItem;
                return true;
            }
        }

        return false;
    }

    bool DebuggerScope::TryGetValidProperty(Js::PropertyId propertyId, RegSlot location, int offset, DebuggerScopeProperty* outScopeProperty, bool* isInDeadZone) const
    {
        if (TryGetProperty(propertyId, location, outScopeProperty))
        {
            if (IsOffsetInScope(offset))
            {
                if (isInDeadZone != nullptr)
                {
                    *isInDeadZone = outScopeProperty->IsInDeadZone(offset);
                }

                return true;
            }
        }

        return false;
    }

    void DebuggerScope::SetBegin(int begin)
    {
        range.begin = begin;
        if (siblingScope != nullptr)
        {
            siblingScope->SetBegin(begin);
        }
    }

    void DebuggerScope::SetEnd(int end)
    {
        range.end = end;
        if (siblingScope != nullptr)
        {
            siblingScope->SetEnd(end);
        }
    }

    // Finds the common ancestor scope between this scope and the passed in scope.
    // Returns nullptr if the scopes are part of different trees.
    DebuggerScope* DebuggerScope::FindCommonAncestor(DebuggerScope* debuggerScope)
    {
        Assert(debuggerScope);

        if (this == debuggerScope)
        {
            return debuggerScope;
        }

        if (this->IsAncestorOf(debuggerScope))
        {
            return this;
        }

        if (debuggerScope->IsAncestorOf(this))
        {
            return debuggerScope;
        }

        DebuggerScope* firstNode = this;
        DebuggerScope* secondNode = debuggerScope;

        int firstDepth = firstNode->GetScopeDepth();
        int secondDepth = secondNode->GetScopeDepth();

        // Calculate the depth difference in order to bring the deep node up to the sibling
        // level of the shorter node.
        int depthDifference = abs(firstDepth - secondDepth);

        DebuggerScope*& nodeToBringUp = firstDepth > secondDepth ? firstNode : secondNode;
        while (depthDifference > 0)
        {
            Assert(nodeToBringUp);
            nodeToBringUp = nodeToBringUp->GetParentScope();
            --depthDifference;
        }

        // Move up the tree and see where the nodes meet.
        while (firstNode && secondNode)
        {
            if (firstNode == secondNode)
            {
                return firstNode;
            }

            firstNode = firstNode->GetParentScope();
            secondNode = secondNode->GetParentScope();
        }

        // The nodes are not part of the same scope tree.
        return nullptr;
    }

    // Gets the depth of the scope in the parent link tree.
    int DebuggerScope::GetScopeDepth() const
    {
        int depth = 0;
        const DebuggerScope* currentDebuggerScope = this;
        while (currentDebuggerScope)
        {
            currentDebuggerScope = currentDebuggerScope->GetParentScope();
            ++depth;
        }

        return depth;
    }

    bool ScopeObjectChain::TryGetDebuggerScopePropertyInfo(PropertyId propertyId, RegSlot location, int offset, bool* isPropertyInDebuggerScope, bool *isConst, bool* isInDeadZone)
    {
        Assert(pScopeChain);
        Assert(isPropertyInDebuggerScope);
        Assert(isConst);

        *isPropertyInDebuggerScope = false;
        *isConst = false;

        // Search through each block scope until we find the current scope.  If the register was found
        // in any of the scopes going down until we reach the scope of the debug break, then it's in scope.
        // if found but not in the scope, the out param will be updated (since it is actually a let or const), so that caller can make a call accordingly.
        for (int i = 0; i < pScopeChain->Count(); i++)
        {
            Js::DebuggerScope *debuggerScope = pScopeChain->Item(i);
            DebuggerScopeProperty debuggerScopeProperty;
            if (debuggerScope->TryGetProperty(propertyId, location, &debuggerScopeProperty))
            {
                bool isOffsetInScope = debuggerScope->IsOffsetInScope(offset);

                // For the Object scope, all the properties will have the same location (-1) so they can match. Use further check below to determine the propertyInDebuggerScope
                *isPropertyInDebuggerScope = isOffsetInScope || !debuggerScope->IsBlockObjectScope();

                if (isOffsetInScope)
                {
                    if (isInDeadZone != nullptr)
                    {
                        *isInDeadZone = debuggerScopeProperty.IsInDeadZone(offset);
                    }

                    *isConst = debuggerScopeProperty.IsConst();
                    return true;
                }
            }
        }

        return false;
    }

    void FunctionBody::AllocateInlineCache()
    {
        Assert(this->inlineCaches == null);
        uint isInstInlineCacheStart = this->GetInlineCacheCount();
        uint totalCacheCount = isInstInlineCacheStart + isInstInlineCacheCount;

        if (totalCacheCount != 0)
        {
            // Root object inline cache are not leaf
            void ** inlineCaches = RecyclerNewArrayZ(this->m_scriptContext->GetRecycler(),
                void*, totalCacheCount);
#if DBG
            this->m_inlineCacheTypes = RecyclerNewArrayLeafZ(this->m_scriptContext->GetRecycler(),
                byte, totalCacheCount);
#endif
            uint i = 0;
            uint plainInlineCacheEnd = rootObjectLoadInlineCacheStart;
            __analysis_assume(plainInlineCacheEnd < totalCacheCount);
            for (;i < plainInlineCacheEnd; i++)
            {
                inlineCaches[i] = AllocatorNewZ(InlineCacheAllocator,
                    this->m_scriptContext->GetInlineCacheAllocator(), InlineCache);
            }
            Js::RootObjectBase * rootObject = this->GetRootObject();
            ThreadContext * threadContext = this->GetScriptContext()->GetThreadContext();
            uint rootObjectLoadInlineCacheEnd = rootObjectLoadMethodInlineCacheStart;
            __analysis_assume(rootObjectLoadInlineCacheEnd < totalCacheCount);
            for (; i < rootObjectLoadInlineCacheEnd; i++)
            {
                inlineCaches[i] = rootObject->GetInlineCache(
                    threadContext->GetPropertyName(this->GetPropertyIdFromCacheId(i)), false, false);
            }
            uint rootObjectLoadMethodInlineCacheEnd = rootObjectStoreInlineCacheStart;
            __analysis_assume(rootObjectLoadMethodInlineCacheEnd < totalCacheCount);
            for (; i < rootObjectLoadMethodInlineCacheEnd; i++)
            {
                inlineCaches[i] = rootObject->GetInlineCache(
                    threadContext->GetPropertyName(this->GetPropertyIdFromCacheId(i)), true, false);
            }
            uint rootObjectStoreInlineCacheEnd = isInstInlineCacheStart;
            for (; i < rootObjectStoreInlineCacheEnd; i++)
            {
                inlineCaches[i] = rootObject->GetInlineCache(
                    threadContext->GetPropertyName(this->GetPropertyIdFromCacheId(i)), false, true);
            }
            for (;i < totalCacheCount; i++)
            {
                inlineCaches[i] = AllocatorNewStructZ(IsInstInlineCacheAllocator,
                    this->m_scriptContext->GetIsInstInlineCacheAllocator(), IsInstInlineCache);
            }
#if DBG
            this->m_inlineCacheTypes = RecyclerNewArrayLeafZ(this->m_scriptContext->GetRecycler(),
                byte, totalCacheCount);
#endif
            this->inlineCaches = inlineCaches;
        }
    }

    InlineCache *FunctionBody::GetInlineCache(uint index)
    {
        Assert(this->inlineCaches != null);
        Assert(index < this->GetInlineCacheCount());
#if DBG
        Assert(this->m_inlineCacheTypes[index] == InlineCacheTypeNone ||
            this->m_inlineCacheTypes[index] == InlineCacheTypeInlineCache);
        this->m_inlineCacheTypes[index] = InlineCacheTypeInlineCache;
#endif
        return reinterpret_cast<InlineCache *>(this->inlineCaches[index]);
    }

    bool FunctionBody::CanFunctionObjectHaveInlineCaches()
    {
        if (this->DoStackNestedFunc() || this->IsGenerator())
        {
            return false;
        }

        uint totalCacheCount = this->GetInlineCacheCount() + this->GetIsInstInlineCacheCount();
        if (PHASE_FORCE(Js::ScriptFunctionWithInlineCachePhase, this) && totalCacheCount > 0)
        {
            return true;
        }

        //Only have inline caches on function object for possible inlining candidates.
        //Since we don't know the size of the top function, check against the maximum possible inline threshold
        // Negative inline byte code size threshold disable inline cache on function object.
        const int byteCodeSizeThreshold = CONFIG_FLAG(InlineThreshold) + CONFIG_FLAG(InlineThresholdAdjustCountInSmallFunction);
        if (byteCodeSizeThreshold < 0 || this->GetByteCodeWithoutLDACount() > (uint)byteCodeSizeThreshold)
        {
            return false;
        }
        // Negative FuncObjectInlineCacheThreshold disable inline cache on function object.
        if (CONFIG_FLAG(FuncObjectInlineCacheThreshold) < 0 || totalCacheCount > (uint)CONFIG_FLAG(FuncObjectInlineCacheThreshold) || totalCacheCount == 0)
        {
            return false;
        }

        return true;
    }

    void** FunctionBody::GetInlineCaches()
    {
        return this->inlineCaches;
    }

#if DBG
    byte* FunctionBody::GetInlineCacheTypes()
    {
        return this->m_inlineCacheTypes;
    }
#endif

    IsInstInlineCache *FunctionBody::GetIsInstInlineCache(uint index)
    {
        Assert(this->inlineCaches != NULL);
        Assert(index < GetIsInstInlineCacheCount());
        index += this->GetInlineCacheCount();
#if DBG
        Assert(this->m_inlineCacheTypes[index] == InlineCacheTypeNone ||
            this->m_inlineCacheTypes[index] == InlineCacheTypeIsInst);
        this->m_inlineCacheTypes[index] = InlineCacheTypeIsInst;
#endif
        return reinterpret_cast<IsInstInlineCache *>(this->inlineCaches[index]);
    }

    PolymorphicInlineCache * FunctionBody::GetPolymorphicInlineCache(uint index)
    {
        return this->polymorphicInlineCaches.GetInlineCache(this, index);
    }

    PolymorphicInlineCache * FunctionBody::CreateNewPolymorphicInlineCache(uint index, PropertyId propertyId, InlineCache * inlineCache)
    {
        Assert(GetPolymorphicInlineCache(index) == null);
        // Only create polymorphic inline caches for non-root inline cache indexes
        if (index < rootObjectLoadInlineCacheStart
#if DBG
            && !PHASE_OFF1(Js::PolymorphicInlineCachePhase)
#endif
            )
        {
            PolymorphicInlineCache * polymorphicInlineCache = CreatePolymorphicInlineCache(index, PolymorphicInlineCache::GetInitialSize());
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
            if (PHASE_VERBOSE_TRACE1(Js::PolymorphicInlineCachePhase))
            {
                this->DumpFullFunctionName();
                Output::Print(L": New PIC, index = %d, size = %d\n", index, PolymorphicInlineCache::GetInitialSize());
            }

#endif
#if PHASE_PRINT_INTRUSIVE_TESTTRACE1
            wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];
#endif
            PHASE_PRINT_INTRUSIVE_TESTTRACE1(
                Js::PolymorphicInlineCachePhase,
                L"TestTrace PIC:  New, Function %s (%s), 0x%x, index = %d, size = %d\n", this->GetDisplayName(), this->GetDebugNumberSet(debugStringBuffer), polymorphicInlineCache, index, PolymorphicInlineCache::GetInitialSize());
            inlineCache->CopyTo(propertyId, m_scriptContext, &(polymorphicInlineCache->GetInlineCaches()[polymorphicInlineCache->GetInlineCacheIndexForType(inlineCache->GetType())]));
            return polymorphicInlineCache;
        }
        return null;
    }

    PolymorphicInlineCache * FunctionBody::CreateBiggerPolymorphicInlineCache(uint index, PropertyId propertyId)
    {
        PolymorphicInlineCache * polymorphicInlineCache = GetPolymorphicInlineCache(index);
        Assert(polymorphicInlineCache && polymorphicInlineCache->CanAllocateBigger());
        uint16 polymorphicInlineCacheSize = polymorphicInlineCache->GetSize();
        uint16 newPolymorphicInlineCacheSize = PolymorphicInlineCache::GetNextSize(polymorphicInlineCacheSize);
        Assert(newPolymorphicInlineCacheSize > polymorphicInlineCacheSize);
        PolymorphicInlineCache * newPolymorphicInlineCache = CreatePolymorphicInlineCache(index, newPolymorphicInlineCacheSize);
        polymorphicInlineCache->CopyTo(propertyId, m_scriptContext, newPolymorphicInlineCache);
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        if (PHASE_VERBOSE_TRACE1(Js::PolymorphicInlineCachePhase))
        {
            this->DumpFullFunctionName();
            Output::Print(L": Bigger PIC, index = %d, oldSize = %d, newSize = %d\n", index, polymorphicInlineCacheSize, newPolymorphicInlineCacheSize);
        }
#endif
#if PHASE_PRINT_INTRUSIVE_TESTTRACE1
        wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];
#endif
        PHASE_PRINT_INTRUSIVE_TESTTRACE1(
            Js::PolymorphicInlineCachePhase,
            L"TestTrace PIC:  Bigger, Function %s (%s), 0x%x, index = %d, size = %d\n", this->GetDisplayName(), this->GetDebugNumberSet(debugStringBuffer), newPolymorphicInlineCache, index, newPolymorphicInlineCacheSize);
        return newPolymorphicInlineCache;
    }

    void FunctionBody::ResetInlineCaches()
    {
        isInstInlineCacheCount = inlineCacheCount = rootObjectLoadInlineCacheStart = rootObjectStoreInlineCacheStart = 0;
        this->inlineCaches = null;
        this->polymorphicInlineCaches.Reset();
    }

    PolymorphicInlineCache * FunctionBody::CreatePolymorphicInlineCache(uint index, uint16 size)
    {
        Recycler * recycler = this->m_scriptContext->GetRecycler();
        PolymorphicInlineCache * newPolymorphicInlineCache = PolymorphicInlineCache::New(size, this);
        this->polymorphicInlineCaches.SetInlineCache(recycler, this, index, newPolymorphicInlineCache);
        return newPolymorphicInlineCache;
    }

    uint FunctionBody::NewObjectLiteral()
    {
        Assert(objLiteralTypes == null);
        return objLiteralCount++;
    }

    DynamicType ** FunctionBody::GetObjectLiteralTypeRef(uint index)
    {
        Assert(index < objLiteralCount);
        Assert(objLiteralTypes != null);
        return objLiteralTypes + index;
    }

    void FunctionBody::AllocateObjectLiteralTypeArray()
    {
        Assert(objLiteralTypes == null);
        if (objLiteralCount == 0)
        {
            return;
        }

        objLiteralTypes = RecyclerNewArrayZ(this->GetScriptContext()->GetRecycler(), DynamicType *, objLiteralCount);
    }

    uint FunctionBody::NewLiteralRegex()
    {
        Assert(!this->literalRegexes);
        return literalRegexCount++;
    }

    uint FunctionBody::GetLiteralRegexCount() const
    {
        return literalRegexCount;
    }

    void FunctionBody::AllocateLiteralRegexArray()
    {
        Assert(!this->literalRegexes);

        if (literalRegexCount == 0)
        {
            return;
        }

        this->literalRegexes =
            RecyclerNewArrayZ(m_scriptContext->GetRecycler(), UnifiedRegex::RegexPattern *, literalRegexCount);
    }

    AsmJsFunctionInfo* FunctionBody::AllocateAsmJsFunctionInfo()
    {
        Assert( !this->asmJsFunctionInfo );
        this->asmJsFunctionInfo = RecyclerNew( m_scriptContext->GetRecycler(), AsmJsFunctionInfo );
        return this->asmJsFunctionInfo;
    }

    AsmJsModuleInfo* FunctionBody::AllocateAsmJsModuleInfo()
    {
        Assert( !this->asmJsModuleInfo );
        Recycler* rec = m_scriptContext->GetRecycler();
        this->asmJsModuleInfo = RecyclerNew( rec, AsmJsModuleInfo, rec );
        return this->asmJsModuleInfo;
    }

    UnifiedRegex::RegexPattern *FunctionBody::GetLiteralRegex(const uint index)
    {
        Assert(index < literalRegexCount);
        Assert(this->literalRegexes);

        return this->literalRegexes[index];
    }

    void FunctionBody::SetLiteralRegex(const uint index, UnifiedRegex::RegexPattern *const pattern)
    {
        Assert(index < literalRegexCount);
        Assert(this->literalRegexes);

        if (this->literalRegexes[index] && this->literalRegexes[index] == pattern)
        {
            return;
        }
        Assert(!this->literalRegexes[index]);

        this->literalRegexes[index] = pattern;
    }

    void FunctionBody::ResetObjectLiteralTypes()
    {
        this->objLiteralTypes = null;
        this->objLiteralCount = 0;
    }

    void FunctionBody::ResetLiteralRegexes()
    {
        literalRegexCount = 0;
        this->literalRegexes = null;
    }

    void FunctionBody::ResetProfileIds()
    {
        Assert(!HasDynamicProfileInfo()); // profile data relies on the profile ID counts; it should not have been created yet
        Assert(!this->m_codeGenRuntimeData); // relies on 'profiledCallSiteCount'

        profiledCallSiteCount = 0;
        profiledArrayCallSiteCount = 0;
        profiledReturnTypeCount = 0;
        profiledSlotCount = 0;
        profiledLdElemCount = 0;
        profiledStElemCount = 0;
    }

    void FunctionBody::ResetByteCodeGenState()
    {
        // Byte code generation failed for this function. Revert any intermediate state being tracked in the function body, in
        // case byte code generation is attempted again for this function body.

        ResetInlineCaches();
        ResetObjectLiteralTypes();
        ResetLiteralRegexes();
        ResetLoops();
        ResetProfileIds();

        m_firstTmpReg = Constants::NoRegister;
        stackClosureRegister = Constants::NoRegister;
        m_constCount = 0;
        this->m_constTable = null;
        this->byteCodeBlock = null;

        // There is other state that is set by the byte code generator but the state should be the same each time byte code
        // generation is done for the function, so they don't need to be reverted
    }

    void FunctionBody::ResetByteCodeGenVisitState()
    {
        // This function body is about to be visited by the byte code generator after defer-parsing it. Since the previous visit
        // pass may have failed, we need to restore state that is tracked on the function body by the visit pass.

        ResetLiteralRegexes();
    }

    const FunctionCodeGenRuntimeData *FunctionBody::GetInlineeCodeGenRuntimeData(const ProfileId profiledCallSiteId) const
    {
        Assert(profiledCallSiteId < profiledCallSiteCount);

        return this->m_codeGenRuntimeData ? this->m_codeGenRuntimeData[profiledCallSiteId] : null;
    }

    const FunctionCodeGenRuntimeData *FunctionBody::GetInlineeCodeGenRuntimeDataForTargetInlinee(const ProfileId profiledCallSiteId, Js::FunctionBody *inlineeFuncBody) const
    {
        Assert(profiledCallSiteId < profiledCallSiteCount);

        if (!this->m_codeGenRuntimeData)
        {
            return null;
        }
        const FunctionCodeGenRuntimeData *runtimeData = this->m_codeGenRuntimeData[profiledCallSiteId];
        while (runtimeData && runtimeData->GetFunctionBody() != inlineeFuncBody)
        {
            runtimeData = runtimeData->GetNext();
        }
        return runtimeData;
    }

    FunctionCodeGenRuntimeData *FunctionBody::EnsureInlineeCodeGenRuntimeData(
        Recycler *const recycler,
        __in_range(0, profiledCallSiteCount - 1) const ProfileId profiledCallSiteId,
        FunctionBody *const inlinee)
    {
        Assert(recycler);
        Assert(profiledCallSiteId < profiledCallSiteCount);
        Assert(inlinee);

        if(!this->m_codeGenRuntimeData)
        {
            const auto codeGenRuntimeData = RecyclerNewArrayZ(recycler, FunctionCodeGenRuntimeData *, profiledCallSiteCount);
            this->m_codeGenRuntimeData = codeGenRuntimeData;
        }

        const auto inlineeData = this->m_codeGenRuntimeData[profiledCallSiteId];

        if(!inlineeData)
        {
            return this->m_codeGenRuntimeData[profiledCallSiteId] = RecyclerNew(recycler, FunctionCodeGenRuntimeData, inlinee);
        }

        //Find the right code gen runtime data
        FunctionCodeGenRuntimeData *next = inlineeData;

        while(next && (next->GetFunctionBody() != inlinee))
        {
            next = next->GetNext();
        }

        if (next)
        {
            return next;
        }

        FunctionCodeGenRuntimeData *runtimeData = RecyclerNew(recycler, FunctionCodeGenRuntimeData, inlinee);
        runtimeData->SetupRuntimeDataChain(inlineeData);
        return this->m_codeGenRuntimeData[profiledCallSiteId] = runtimeData;
    }

    const FunctionCodeGenRuntimeData *FunctionBody::GetLdFldInlineeCodeGenRuntimeData(const uint inlineCacheIndex) const
    {
        Assert(inlineCacheIndex < inlineCacheCount);

        return this->m_codeGenGetSetRuntimeData ? this->m_codeGenGetSetRuntimeData[inlineCacheIndex] : null;
    }

    FunctionCodeGenRuntimeData *FunctionBody::EnsureLdFldInlineeCodeGenRuntimeData(
        Recycler *const recycler,
        __in_range(0, m_inlineCacheCount - 1) const uint inlineCacheIndex,
        FunctionBody *const inlinee)
    {
        Assert(recycler);
        Assert(inlineCacheIndex < this->GetInlineCacheCount());
        Assert(inlinee);

        if(!this->m_codeGenGetSetRuntimeData)
        {
            const auto codeGenRuntimeData = RecyclerNewArrayZ(recycler, FunctionCodeGenRuntimeData *, this->GetInlineCacheCount());
            this->m_codeGenGetSetRuntimeData = codeGenRuntimeData;
        }

        const auto inlineeData = this->m_codeGenGetSetRuntimeData[inlineCacheIndex];
        if(inlineeData)
        {
            return inlineeData;
        }

        return this->m_codeGenGetSetRuntimeData[inlineCacheIndex] = RecyclerNew(recycler, FunctionCodeGenRuntimeData, inlinee);
    }

    void FunctionBody::AllocateLoopHeaders()
    {
        Assert(this->loopHeaderArray == null);

        if (loopCount != 0)
        {
            this->loopHeaderArray = RecyclerNewArrayZ(this->m_scriptContext->GetRecycler(), LoopHeader, loopCount);
            for (uint i = 0; i < loopCount; i++)
            {
                this->loopHeaderArray[i].Init(this);
            }
        }
    }

    void FunctionBody::ReleaseLoopHeaders()
    {
        this->MapLoopHeaders([](uint loopNumber, LoopHeader * loopHeader)
        {
            loopHeader->ReleaseEntryPoints();
        });
    }

    void FunctionBody::ResetLoops()
    {
        loopCount = 0;
        this->loopHeaderArray = null;
    }

    void FunctionBody::RestoreOldDefaultEntryPoint(FunctionEntryPointInfo* oldEntryPointInfo,
        JavascriptMethod oldOriginalEntryPoint,
        FunctionEntryPointInfo* newEntryPointInfo)
    {
        Assert(newEntryPointInfo);

        this->SetDefaultFunctionEntryPointInfo(oldEntryPointInfo, oldOriginalEntryPoint);
        this->entryPoints->RemoveAt(newEntryPointInfo->entryPointIndex);
    }

    FunctionEntryPointInfo* FunctionBody::CreateNewDefaultEntryPoint()
    {
        Recycler *const recycler = this->m_scriptContext->GetRecycler();
        const JavascriptMethod currentThunk = m_scriptContext->CurrentThunk;

        FunctionEntryPointInfo *const entryPointInfo =
            RecyclerNewFinalized(
                recycler,
                FunctionEntryPointInfo,
                this,
                currentThunk,
                m_scriptContext->GetThreadContext(),
                (void*) m_scriptContext->GetNativeCodeGenerator());
        AddEntryPointToEntryPointList(entryPointInfo);

        {
            // Allocations in this region may trigger expiry and cause unexpected changes to state
            AUTO_NO_EXCEPTION_REGION;

            FunctionEntryPointInfo *const simpleJitEntryPointInfo = GetSimpleJitEntryPointInfo();
            Js::JavascriptMethod originalEntryPoint, directEntryPoint;
            if(simpleJitEntryPointInfo && GetExecutionMode() == ExecutionMode::FullJit)
            {
                directEntryPoint =
                    originalEntryPoint = reinterpret_cast<Js::JavascriptMethod>(simpleJitEntryPointInfo->GetNativeAddress());
            }
            else
            {
                // If the dynamic interpreter thunk hasn't been created yet, then the entry point can be set to
                // the default entry point. Otherwise, since the new default entry point is being created to
                // move back to the interpreter, the original entry point is going to be the dynamic interpreter thunk
                originalEntryPoint =
                    m_dynamicInterpreterThunk
                        ? static_cast<JavascriptMethod>(InterpreterThunkEmitter::ConvertToEntryPoint(m_dynamicInterpreterThunk))
                        : DefaultEntryThunk;

                directEntryPoint = currentThunk == DefaultEntryThunk ? originalEntryPoint : currentThunk;
            }

            entryPointInfo->address = directEntryPoint;
            SetDefaultFunctionEntryPointInfo(entryPointInfo, originalEntryPoint);
        }

        return entryPointInfo;
    }

    LoopHeader *FunctionBody::GetLoopHeader(uint index) const
    {
        Assert(this->loopHeaderArray != null);
        Assert(index < loopCount);
        return &this->loopHeaderArray[index];
    }

    FunctionEntryPointInfo *FunctionBody::GetSimpleJitEntryPointInfo() const
    {
        return simpleJitEntryPointInfo;
    }

    void FunctionBody::SetSimpleJitEntryPointInfo(FunctionEntryPointInfo *const entryPointInfo)
    {
        simpleJitEntryPointInfo = entryPointInfo;
    }

    void FunctionBody::VerifyExecutionMode(const ExecutionMode executionMode) const
    {
#if DBG
        Assert(initializedExecutionModeAndLimits);
        Assert(executionMode < ExecutionMode::Count);

        switch(executionMode)
        {
            case ExecutionMode::Interpreter:
                Assert(!DoInterpreterProfile());
                break;

            case ExecutionMode::AutoProfilingInterpreter:
                Assert(DoInterpreterProfile());
                Assert(DoInterpreterAutoProfile());
                break;

            case ExecutionMode::ProfilingInterpreter:
                Assert(DoInterpreterProfile());
                break;

            case ExecutionMode::SimpleJit:
                Assert(DoSimpleJit());
                break;

            case ExecutionMode::FullJit:
                Assert(DoFullJit());
                break;

            default:
                Assert(false);
                __assume(false);
        }
#endif
    }

    ExecutionMode FunctionBody::GetDefaultInterpreterExecutionMode() const
    {
        if(!DoInterpreterProfile())
        {
            VerifyExecutionMode(ExecutionMode::Interpreter);
            return ExecutionMode::Interpreter;
        }
        if(DoInterpreterAutoProfile())
        {
            VerifyExecutionMode(ExecutionMode::AutoProfilingInterpreter);
            return ExecutionMode::AutoProfilingInterpreter;
        }
        VerifyExecutionMode(ExecutionMode::ProfilingInterpreter);
        return ExecutionMode::ProfilingInterpreter;
    }

    ExecutionMode FunctionBody::GetExecutionMode() const
    {
        VerifyExecutionMode(executionMode);
        return executionMode;
    }

    ExecutionMode FunctionBody::GetInterpreterExecutionMode(const bool isPostBailout)
    {
        Assert(initializedExecutionModeAndLimits);

        if(isPostBailout && DoInterpreterProfile())
        {
            return ExecutionMode::ProfilingInterpreter;
        }

        switch(GetExecutionMode())
        {
            case ExecutionMode::Interpreter:
            case ExecutionMode::AutoProfilingInterpreter:
            case ExecutionMode::ProfilingInterpreter:
                return GetExecutionMode();

            case ExecutionMode::SimpleJit:
                if(IsNewSimpleJit())
                {
                    return GetDefaultInterpreterExecutionMode();
                }
                // fall through

            case ExecutionMode::FullJit:
            {
                const ExecutionMode executionMode =
                    DoInterpreterProfile() ? ExecutionMode::ProfilingInterpreter : ExecutionMode::Interpreter;
                VerifyExecutionMode(executionMode);
                return executionMode;
            }

            default:
                Assert(false);
                __assume(false);
        }
    }

    void FunctionBody::SetExecutionMode(const ExecutionMode executionMode)
    {
        VerifyExecutionMode(executionMode);
        this->executionMode = executionMode;
    }

    bool FunctionBody::IsInterpreterExecutionMode() const
    {
        return GetExecutionMode() <= ExecutionMode::ProfilingInterpreter;
    }

    bool FunctionBody::TryTransitionToNextExecutionMode()
    {
        Assert(initializedExecutionModeAndLimits);

        switch(GetExecutionMode())
        {
            case ExecutionMode::Interpreter:
                if(interpretedCount < interpreterLimit)
                {
                    VerifyExecutionMode(GetExecutionMode());
                    return false;
                }
                CommitExecutedIterations(interpreterLimit, interpreterLimit);
                goto TransitionToFullJit;

            TransitionToAutoProfilingInterpreter:
                if(autoProfilingInterpreter0Limit != 0 || autoProfilingInterpreter1Limit != 0)
                {
                    SetExecutionMode(ExecutionMode::AutoProfilingInterpreter);
                    interpretedCount = 0;
                    return true;
                }
                goto TransitionFromAutoProfilingInterpreter;

            case ExecutionMode::AutoProfilingInterpreter:
            {
                uint16 &autoProfilingInterpreterLimit =
                    autoProfilingInterpreter0Limit == 0 && profilingInterpreter0Limit == 0
                        ? autoProfilingInterpreter1Limit
                        : autoProfilingInterpreter0Limit;
                if(interpretedCount < autoProfilingInterpreterLimit)
                {
                    VerifyExecutionMode(GetExecutionMode());
                    return false;
                }
                CommitExecutedIterations(autoProfilingInterpreterLimit, autoProfilingInterpreterLimit);
                // fall through
            }

            TransitionFromAutoProfilingInterpreter:
                Assert(autoProfilingInterpreter0Limit == 0 || autoProfilingInterpreter1Limit == 0);
                if(profilingInterpreter0Limit == 0 && autoProfilingInterpreter1Limit == 0)
                {
                    goto TransitionToSimpleJit;
                }
                // fall through

            TransitionToProfilingInterpreter:
                if(profilingInterpreter0Limit != 0 || profilingInterpreter1Limit != 0)
                {
                    SetExecutionMode(ExecutionMode::ProfilingInterpreter);
                    interpretedCount = 0;
                    return true;
                }
                goto TransitionFromProfilingInterpreter;

            case ExecutionMode::ProfilingInterpreter:
            {
                uint16 &profilingInterpreterLimit =
                    profilingInterpreter0Limit == 0 && autoProfilingInterpreter1Limit == 0 && simpleJitLimit == 0
                        ? profilingInterpreter1Limit
                        : profilingInterpreter0Limit;
                if(interpretedCount < profilingInterpreterLimit)
                {
                    VerifyExecutionMode(GetExecutionMode());
                    return false;
                }
                CommitExecutedIterations(profilingInterpreterLimit, profilingInterpreterLimit);
                // fall through
            }

            TransitionFromProfilingInterpreter:
                Assert(profilingInterpreter0Limit == 0 || profilingInterpreter1Limit == 0);
                if(autoProfilingInterpreter1Limit == 0 && simpleJitLimit == 0 && profilingInterpreter1Limit == 0)
                {
                    goto TransitionToFullJit;
                }
                goto TransitionToAutoProfilingInterpreter;

            TransitionToSimpleJit:
                if(simpleJitLimit != 0)
                {
                    SetExecutionMode(ExecutionMode::SimpleJit);

                    // Zero the interpreted count here too, so that it can be determined how many interpreter iterations ran
                    // while waiting for simple JIT
                    interpretedCount = 0;
                    return true;
                }
                goto TransitionToProfilingInterpreter;

            case ExecutionMode::SimpleJit:
            {
                FunctionEntryPointInfo *const simpleJitEntryPointInfo = GetSimpleJitEntryPointInfo();
                if(!simpleJitEntryPointInfo || simpleJitEntryPointInfo->callsCount != 0)
                {
                    VerifyExecutionMode(GetExecutionMode());
                    return false;
                }
                CommitExecutedIterations(simpleJitLimit, simpleJitLimit);
                goto TransitionToProfilingInterpreter;
            }

            TransitionToFullJit:
                if(DoFullJit())
                {
                    SetExecutionMode(ExecutionMode::FullJit);
                    return true;
                }
                // fall through

            case ExecutionMode::FullJit:
                VerifyExecutionMode(GetExecutionMode());
                return false;

            default:
                Assert(false);
                __assume(false);
        }
    }

    void FunctionBody::TryTransitionToNextInterpreterExecutionMode()
    {
        Assert(IsInterpreterExecutionMode());

        TryTransitionToNextExecutionMode();
        SetExecutionMode(GetInterpreterExecutionMode(false));
    }

    void FunctionBody::SetIsSpeculativeJitCandidate()
    {
        // This function is a candidate for speculative JIT. Ensure that it is profiled immediately by transitioning out of the
        // auto-profiling interpreter mode.
        if(GetExecutionMode() != ExecutionMode::AutoProfilingInterpreter || GetProfiledIterations() != 0)
        {
            return;
        }

        TraceExecutionMode("IsSpeculativeJitCandidate (before)");

        if(autoProfilingInterpreter0Limit != 0)
        {
            (profilingInterpreter0Limit == 0 ? profilingInterpreter0Limit : autoProfilingInterpreter1Limit) +=
                autoProfilingInterpreter0Limit;
            autoProfilingInterpreter0Limit = 0;
        }
        else if(profilingInterpreter0Limit == 0)
        {
            profilingInterpreter0Limit += autoProfilingInterpreter1Limit;
            autoProfilingInterpreter1Limit = 0;
        }

        TraceExecutionMode("IsSpeculativeJitCandidate");
        TryTransitionToNextInterpreterExecutionMode();
    }

    bool FunctionBody::TryTransitionToJitExecutionMode()
    {
        const ExecutionMode previousExecutionMode = GetExecutionMode();

        TryTransitionToNextExecutionMode();
        switch(GetExecutionMode())
        {
            case ExecutionMode::SimpleJit:
                break;

            case ExecutionMode::FullJit:
                if(fullJitRequeueThreshold == 0)
                {
                    break;
                }
                --fullJitRequeueThreshold;
                return false;

            default:
                return false;
        }

        if(GetExecutionMode() != previousExecutionMode)
        {
            TraceExecutionMode();
        }
        return true;
    }

    void FunctionBody::TransitionToSimpleJitExecutionMode()
    {
        CommitExecutedIterations();

        interpreterLimit = 0;
        autoProfilingInterpreter0Limit = 0;
        profilingInterpreter0Limit = 0;
        autoProfilingInterpreter1Limit = 0;
        fullJitThreshold = simpleJitLimit + profilingInterpreter1Limit;

        VerifyExecutionModeLimits();
        SetExecutionMode(ExecutionMode::SimpleJit);
    }

    void FunctionBody::TransitionToFullJitExecutionMode()
    {
        CommitExecutedIterations();

        interpreterLimit = 0;
        autoProfilingInterpreter0Limit = 0;
        profilingInterpreter0Limit = 0;
        autoProfilingInterpreter1Limit = 0;
        simpleJitLimit = 0;
        profilingInterpreter1Limit = 0;
        fullJitThreshold = 0;

        VerifyExecutionModeLimits();
        SetExecutionMode(ExecutionMode::FullJit);
    }

    void FunctionBody::VerifyExecutionModeLimits()
    {
        Assert(initializedExecutionModeAndLimits);
        Assert(
            (
                interpreterLimit +
                autoProfilingInterpreter0Limit +
                profilingInterpreter0Limit +
                autoProfilingInterpreter1Limit +
                simpleJitLimit +
                profilingInterpreter1Limit
            ) == fullJitThreshold);
    }

    void FunctionBody::InitializeExecutionModeAndLimits()
    {
        DebugOnly(initializedExecutionModeAndLimits = true);

        const ConfigFlagsTable &configFlags = Configuration::Global.flags;

        interpreterLimit = 0;
        autoProfilingInterpreter0Limit = static_cast<uint16>(configFlags.AutoProfilingInterpreter0Limit);
        profilingInterpreter0Limit = static_cast<uint16>(configFlags.ProfilingInterpreter0Limit);
        autoProfilingInterpreter1Limit = static_cast<uint16>(configFlags.AutoProfilingInterpreter1Limit);
        simpleJitLimit = static_cast<uint16>(configFlags.SimpleJitLimit);
        profilingInterpreter1Limit = static_cast<uint16>(configFlags.ProfilingInterpreter1Limit);

        // Based on which execution modes are disabled, calculate the number of additional iterations that need to be covered by
        // the execution mode that will scale with the full JIT threshold
        uint16 scale = 0;
        const bool doInterpreterProfile = DoInterpreterProfile();
        if(!doInterpreterProfile)
        {
            scale +=
                autoProfilingInterpreter0Limit +
                profilingInterpreter0Limit +
                autoProfilingInterpreter1Limit +
                profilingInterpreter1Limit;
            autoProfilingInterpreter0Limit = 0;
            profilingInterpreter0Limit = 0;
            autoProfilingInterpreter1Limit = 0;
            profilingInterpreter1Limit = 0;
        }
        else if(!DoInterpreterAutoProfile())
        {
            scale += autoProfilingInterpreter0Limit + autoProfilingInterpreter1Limit;
            autoProfilingInterpreter0Limit = 0;
            autoProfilingInterpreter1Limit = 0;
            if(!IsNewSimpleJit())
            {
                simpleJitLimit += profilingInterpreter0Limit;
                profilingInterpreter0Limit = 0;
            }
        }
        if(!DoSimpleJit())
        {
            if(!IsNewSimpleJit() && doInterpreterProfile)
            {
                // The old simple JIT is off, but since it does profiling, it will be replaced with the profiling interpreter
                profilingInterpreter1Limit += simpleJitLimit;
            }
            else
            {
                scale += simpleJitLimit;
            }
            simpleJitLimit = 0;
        }
        if(!DoFullJit())
        {
            scale += profilingInterpreter1Limit;
            profilingInterpreter1Limit = 0;
        }

        uint16 fullJitThreshold =
            static_cast<uint16>(
                configFlags.AutoProfilingInterpreter0Limit +
                configFlags.ProfilingInterpreter0Limit +
                configFlags.AutoProfilingInterpreter1Limit +
                configFlags.SimpleJitLimit +
                configFlags.ProfilingInterpreter1Limit);
        if(!configFlags.EnforceExecutionModeLimits)
        {
            /*
            Scale the full JIT threshold based on some heuristics:
                - If the % of code in loops is > 50, scale by 1
                - Byte-code size of code outside loops
                    - If the size is < 50, scale by 1.2
                    - If the size is < 100, scale by 1.4
                    - If the size is >= 100, scale by 1.6
            */
            const uint loopPercentage = GetByteCodeInLoopCount() * 100 / max(1u, GetByteCodeCount());
            if(loopPercentage <= 50)
            {
                const uint straightLineSize = GetByteCodeCount() - GetByteCodeInLoopCount();
                double fullJitDelayMultiplier;
                if(straightLineSize < 50)
                {
                    fullJitDelayMultiplier = 1.2;
                }
                else if(straightLineSize < 100)
                {
                    fullJitDelayMultiplier = 1.4;
                }
                else
                {
                    fullJitDelayMultiplier = 1.6;
                }

                const uint16 newFullJitThreshold = static_cast<uint16>(fullJitThreshold * fullJitDelayMultiplier);
                scale += newFullJitThreshold - fullJitThreshold;
                fullJitThreshold = newFullJitThreshold;
            }
        }

        Assert(fullJitThreshold >= scale);
        this->fullJitThreshold = fullJitThreshold - scale;
        interpretedCount = 0;
        SetExecutionMode(GetDefaultInterpreterExecutionMode());
        SetFullJitThreshold(fullJitThreshold);
        TryTransitionToNextInterpreterExecutionMode();
    }

    void FunctionBody::ReinitializeExecutionModeAndLimits()
    {
        wasCalledFromLoop = false;
        fullJitRequeueThreshold = 0;
        committedProfiledIterations = 0;
        InitializeExecutionModeAndLimits();
    }

    void FunctionBody::SetFullJitThreshold(const uint16 newFullJitThreshold, const bool skipSimpleJit)
    {
        Assert(initializedExecutionModeAndLimits);
        Assert(GetExecutionMode() != ExecutionMode::FullJit);

        int scale = newFullJitThreshold - fullJitThreshold;
        if(scale == 0)
        {
            VerifyExecutionModeLimits();
            return;
        }
        fullJitThreshold = newFullJitThreshold;

        const auto ScaleLimit = [&](uint16 &limit) -> bool
        {
            Assert(scale != 0);
            const int limitScale = max(-static_cast<int>(limit), scale);
            const int newLimit = limit + limitScale;
            Assert(static_cast<int>(static_cast<uint16>(newLimit)) == newLimit);
            limit = static_cast<uint16>(newLimit);
            scale -= limitScale;
            Assert(limit == 0 || scale == 0);

            if(&limit == simpleJitLimit.AddressOf())
            {
                FunctionEntryPointInfo *const simpleJitEntryPointInfo = GetSimpleJitEntryPointInfo();
                if(GetDefaultFunctionEntryPointInfo() == simpleJitEntryPointInfo)
                {
                    Assert(GetExecutionMode() == ExecutionMode::SimpleJit);
                    const int newSimpleJitCallCount = max(0, simpleJitEntryPointInfo->callsCount + limitScale);
                    Assert(static_cast<int>(static_cast<uint16>(newSimpleJitCallCount)) == newSimpleJitCallCount);
                    SetSimpleJitCallCount(static_cast<uint16>(newSimpleJitCallCount));
                }
            }

            return scale == 0;
        };

        /*
        Determine which execution mode's limit scales with the full JIT threshold, in order of preference:
            - New simple JIT
            - Auto-profiling interpreter 1
            - Auto-profiling interpreter 0
            - Interpreter
            - Profiling interpreter 0 (when using old simple JIT)
            - Old simple JIT
            - Profiling interpreter 1
            - Profiling interpreter 0 (when using new simple JIT)
        */
        const bool doSimpleJit = DoSimpleJit();
        const bool doInterpreterProfile = DoInterpreterProfile();
        const bool fullyScaled =
            IsNewSimpleJit() && doSimpleJit && ScaleLimit(simpleJitLimit) ||
            (
                doInterpreterProfile
                    ?   DoInterpreterAutoProfile() &&
                        (ScaleLimit(autoProfilingInterpreter1Limit) || ScaleLimit(autoProfilingInterpreter0Limit))
                    :   ScaleLimit(interpreterLimit)
            ) ||
            (
                IsNewSimpleJit()
                    ?   doInterpreterProfile &&
                        (ScaleLimit(profilingInterpreter1Limit) || ScaleLimit(profilingInterpreter0Limit))
                    :   doInterpreterProfile && ScaleLimit(profilingInterpreter0Limit) ||
                        doSimpleJit && ScaleLimit(simpleJitLimit) ||
                        doInterpreterProfile && ScaleLimit(profilingInterpreter1Limit)
            );
        Assert(fullyScaled);
        Assert(scale == 0);

        if(GetExecutionMode() != ExecutionMode::SimpleJit)
        {
            Assert(IsInterpreterExecutionMode());
            if(simpleJitLimit != 0 &&
                (skipSimpleJit || simpleJitLimit < DEFAULT_CONFIG_MinSimpleJitIterations) &&
                !PHASE_FORCE(Phase::SimpleJitPhase, this))
            {
                // Simple JIT code has not yet been generated, and was either requested to be skipped, or the limit ws scaled
                // down too much. Skip simple JIT by moving any remaining iterations to an equivalent interpreter execution
                // mode.
                (IsNewSimpleJit() ? autoProfilingInterpreter1Limit : profilingInterpreter1Limit) += simpleJitLimit;
                simpleJitLimit = 0;
                TryTransitionToNextInterpreterExecutionMode();
            }
        }

        VerifyExecutionModeLimits();
    }

    void FunctionBody::CommitExecutedIterations()
    {
        Assert(initializedExecutionModeAndLimits);

        switch(GetExecutionMode())
        {
            case ExecutionMode::Interpreter:
                CommitExecutedIterations(interpreterLimit, interpretedCount);
                break;

            case ExecutionMode::AutoProfilingInterpreter:
                CommitExecutedIterations(
                    autoProfilingInterpreter0Limit == 0 && profilingInterpreter0Limit == 0
                        ? autoProfilingInterpreter1Limit
                        : autoProfilingInterpreter0Limit,
                    interpretedCount);
                break;

            case ExecutionMode::ProfilingInterpreter:
                CommitExecutedIterations(
                    GetSimpleJitEntryPointInfo()
                        ? profilingInterpreter1Limit
                        : profilingInterpreter0Limit,
                    interpretedCount);
                break;

            case ExecutionMode::SimpleJit:
                CommitExecutedIterations(simpleJitLimit, GetSimpleJitExecutedIterations());
                break;

            case ExecutionMode::FullJit:
                break;

            default:
                Assert(false);
                __assume(false);
        }
    }

    void FunctionBody::CommitExecutedIterations(uint16 &limit, const uint executedIterations)
    {
        Assert(initializedExecutionModeAndLimits);
        Assert(
            &limit == interpreterLimit.AddressOf() ||
            &limit == autoProfilingInterpreter0Limit.AddressOf() ||
            &limit == profilingInterpreter0Limit.AddressOf() ||
            &limit == autoProfilingInterpreter1Limit.AddressOf() ||
            &limit == simpleJitLimit.AddressOf() ||
            &limit == profilingInterpreter1Limit.AddressOf());

        const uint16 clampedExecutedIterations = executedIterations >= limit ? limit : static_cast<uint16>(executedIterations);
        Assert(fullJitThreshold >= clampedExecutedIterations);
        fullJitThreshold -= clampedExecutedIterations;
        limit -= clampedExecutedIterations;
        VerifyExecutionModeLimits();

        if(&limit == profilingInterpreter0Limit.AddressOf() ||
            !IsNewSimpleJit() && &limit == simpleJitLimit.AddressOf() ||
            &limit == profilingInterpreter1Limit.AddressOf())
        {
            const uint16 newCommittedProfiledIterations = committedProfiledIterations + clampedExecutedIterations;
            committedProfiledIterations =
                newCommittedProfiledIterations >= committedProfiledIterations ? newCommittedProfiledIterations : MAXUINT16;
        }
    }

    uint16 FunctionBody::GetSimpleJitExecutedIterations() const
    {
        Assert(initializedExecutionModeAndLimits);
        Assert(GetExecutionMode() == ExecutionMode::SimpleJit);

        FunctionEntryPointInfo *const simpleJitEntryPointInfo = GetSimpleJitEntryPointInfo();
        if(!simpleJitEntryPointInfo)
        {
            return 0;
        }

        // Simple JIT counts down and transitions on overflow
        const uint8 callCount = simpleJitEntryPointInfo->callsCount;
        Assert(simpleJitLimit == 0 ? callCount == 0 : simpleJitLimit > callCount);
        return callCount == 0 ? simpleJitLimit : simpleJitLimit - callCount - 1;
    }

    void FunctionBody::ResetSimpleJitLimitAndCallCount()
    {
        Assert(initializedExecutionModeAndLimits);
        Assert(GetExecutionMode() == ExecutionMode::SimpleJit);
        Assert(GetDefaultFunctionEntryPointInfo() == GetSimpleJitEntryPointInfo());

        const uint16 simpleJitNewLimit = static_cast<uint8>(Configuration::Global.flags.SimpleJitLimit);
        Assert(simpleJitNewLimit == Configuration::Global.flags.SimpleJitLimit);
        if(simpleJitLimit < simpleJitNewLimit)
        {
            fullJitThreshold += simpleJitNewLimit - simpleJitLimit;
            simpleJitLimit = simpleJitNewLimit;
        }

        interpretedCount = 0;
        ResetSimpleJitCallCount();
    }

    void FunctionBody::SetSimpleJitCallCount(const uint16 simpleJitLimit) const
    {
        Assert(GetExecutionMode() == ExecutionMode::SimpleJit);
        Assert(GetDefaultFunctionEntryPointInfo() == GetSimpleJitEntryPointInfo());

        // Simple JIT counts down and transitions on overflow
        const uint8 limit = static_cast<uint8>(min(0xffui16, simpleJitLimit));
        GetSimpleJitEntryPointInfo()->callsCount = limit == 0 ? 0 : limit - 1;
    }

    void FunctionBody::ResetSimpleJitCallCount()
    {
        SetSimpleJitCallCount(
            simpleJitLimit > interpretedCount
                ? simpleJitLimit - static_cast<uint16>(interpretedCount)
                : 0ui16);
    }

    uint16 FunctionBody::GetProfiledIterations() const
    {
        Assert(initializedExecutionModeAndLimits);

        uint16 profiledIterations = committedProfiledIterations;
        switch(GetExecutionMode())
        {
            case ExecutionMode::ProfilingInterpreter:
            {
                const uint16 clampedInterpretedCount =
                    interpretedCount <= MAXUINT16
                        ? static_cast<uint16>(interpretedCount)
                        : MAXUINT16;
                const uint16 newProfiledIterations = profiledIterations + clampedInterpretedCount;
                profiledIterations = newProfiledIterations >= profiledIterations ? newProfiledIterations : MAXUINT16;
                break;
            }

            case ExecutionMode::SimpleJit:
                if(!IsNewSimpleJit())
                {
                    const uint16 newProfiledIterations = profiledIterations + GetSimpleJitExecutedIterations();
                    profiledIterations = newProfiledIterations >= profiledIterations ? newProfiledIterations : MAXUINT16;
                }
                break;
        }
        return profiledIterations;
    }

    void FunctionBody::OnFullJitDequeued(const FunctionEntryPointInfo *const entryPointInfo)
    {
        Assert(initializedExecutionModeAndLimits);
        Assert(GetExecutionMode() == ExecutionMode::FullJit);
        Assert(entryPointInfo);

        if(entryPointInfo != GetDefaultFunctionEntryPointInfo())
        {
            return;
        }

        // Requeue the full JIT work item after this many iterations
        fullJitRequeueThreshold = static_cast<uint16>(DEFAULT_CONFIG_FullJitRequeueThreshold);
    }

    void FunctionBody::TraceExecutionMode(const char *const eventDescription) const
    {
        Assert(initializedExecutionModeAndLimits);

        if(PHASE_TRACE(Phase::ExecutionModePhase, this))
        {
            DoTraceExecutionMode(eventDescription);
        }
    }

    void FunctionBody::TraceInterpreterExecutionMode() const
    {
        Assert(initializedExecutionModeAndLimits);

        if(!PHASE_TRACE(Phase::ExecutionModePhase, this))
        {
            return;
        }

        switch(GetExecutionMode())
        {
            case ExecutionMode::Interpreter:
            case ExecutionMode::AutoProfilingInterpreter:
            case ExecutionMode::ProfilingInterpreter:
                DoTraceExecutionMode(nullptr);
                break;
        }
    }

    void FunctionBody::DoTraceExecutionMode(const char *const eventDescription) const
    {
        Assert(PHASE_TRACE(Phase::ExecutionModePhase, this));
        Assert(initializedExecutionModeAndLimits);

        wchar_t functionIdString[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];
        Output::Print(
            L"ExecutionMode - "
                L"function: %s (%s), "
                L"mode: %S, "
                L"size: %u, "
                L"limits: %hu.%hu.%hu.%hu.%hu = %hu",
            GetDisplayName(),
                GetDebugNumberSet(functionIdString),
            ExecutionModeName(executionMode),
            GetByteCodeCount(),
            interpreterLimit + autoProfilingInterpreter0Limit,
                profilingInterpreter0Limit,
                autoProfilingInterpreter1Limit,
                simpleJitLimit,
                profilingInterpreter1Limit,
                fullJitThreshold);

        if(eventDescription)
        {
            Output::Print(L", event: %S", eventDescription);
        }

        Output::Print(L"\n");
        Output::Flush();
    }

    bool FunctionBody::IsNewSimpleJit()
    {
        return CONFIG_FLAG(NewSimpleJit);
    }

    bool FunctionBody::DoSimpleJit() const
    {
        return
            !PHASE_OFF(Js::SimpleJitPhase, this) &&
            !GetScriptContext()->GetConfig()->IsNoNative() &&
            !GetScriptContext()->IsInDebugMode() &&
            DoInterpreterProfile() &&
            (!IsNewSimpleJit() || DoInterpreterAutoProfile()) &&
            !IsGenerator(); // Generator JIT requires bailout which SimpleJit cannot do since it skips GlobOpt
    }

    bool FunctionBody::DoSimpleJitDynamicProfile() const
    {
        Assert(DoSimpleJit());

        return !PHASE_OFF(Js::SimpleJitDynamicProfilePhase, this) && !IsNewSimpleJit();
    }

    bool FunctionBody::DoInterpreterProfile() const
    {
        //Switch off profiling is asmJsFunction
        if (this->GetIsAsmJsFunction())
        {
            return false;
        }
        else
        {
            return !PHASE_OFF(InterpreterProfilePhase, this) && DynamicProfileInfo::IsEnabled(this);
        }
    }

    bool FunctionBody::DoInterpreterAutoProfile() const
    {
        Assert(DoInterpreterProfile());

        return !PHASE_OFF(InterpreterAutoProfilePhase, this) && !GetScriptContext()->IsInDebugMode();
    }

    bool FunctionBody::WasCalledFromLoop() const
    {
        return wasCalledFromLoop;
    }

    void FunctionBody::SetWasCalledFromLoop()
    {
        if(wasCalledFromLoop)
        {
            return;
        }
        wasCalledFromLoop = true;

        if(Configuration::Global.flags.EnforceExecutionModeLimits)
        {
            if(PHASE_TRACE(Phase::ExecutionModePhase, this))
            {
                CommitExecutedIterations();
                TraceExecutionMode("WasCalledFromLoop (before)");
            }
        }
        else
        {
            // This function is likely going to be called frequently since it's called from a loop. Reduce the full JIT
            // threshold to realize the full JIT perf benefit sooner.
            CommitExecutedIterations();
            TraceExecutionMode("WasCalledFromLoop (before)");
            if(fullJitThreshold > 1)
            {
                SetFullJitThreshold(fullJitThreshold / 2, !IsNewSimpleJit());
            }
        }

        {
            // Reduce the loop interpreter limit too, for the same reasons as above
            const uint oldLoopInterpreterLimit = loopInterpreterLimit;
            const uint newLoopInterpreterLimit = GetReducedLoopInterpretCount();
            Assert(newLoopInterpreterLimit <= oldLoopInterpreterLimit);
            loopInterpreterLimit = newLoopInterpreterLimit;

            // Adjust loop headers' interpret counts to ensure that loops will still be profiled a number of times before
            // loop bodies are jitted
            const uint oldLoopProfileThreshold = GetLoopProfileThreshold(oldLoopInterpreterLimit);
            const uint newLoopProfileThreshold = GetLoopProfileThreshold(newLoopInterpreterLimit);
            MapLoopHeaders([=](const uint index, LoopHeader *const loopHeader)
            {
                const uint interpretedCount = loopHeader->interpretCount;
                if(interpretedCount <= newLoopProfileThreshold || interpretedCount >= oldLoopInterpreterLimit)
                {
                    // The loop hasn't been profiled yet and wouldn't have started profiling even with the new profile
                    // threshold, or it has already been profiled the necessary minimum number of times based on the old limit
                    return;
                }

                if(interpretedCount <= oldLoopProfileThreshold)
                {
                    // The loop hasn't been profiled yet, but would have started profiling with the new profile threshold. Start
                    // profiling on the next iteration.
                    loopHeader->interpretCount = newLoopProfileThreshold;
                    return;
                }

                // The loop has been profiled some already. Preserve the number of profiled iterations.
                loopHeader->interpretCount = newLoopProfileThreshold + (interpretedCount - oldLoopProfileThreshold);
            });
        }

        TraceExecutionMode("WasCalledFromLoop");
    }

    bool FunctionBody::RecentlyBailedOutOfJittedLoopBody() const
    {
        return recentlyBailedOutOfJittedLoopBody;
    }

    void FunctionBody::SetRecentlyBailedOutOfJittedLoopBody(const bool value)
    {
        recentlyBailedOutOfJittedLoopBody = value;
    }

    uint16 FunctionBody::GetMinProfileIterations()
    {
        return
            static_cast<uint>(
                IsNewSimpleJit()
                    ? DEFAULT_CONFIG_MinProfileIterations
                    : DEFAULT_CONFIG_MinProfileIterations_OldSimpleJit);
    }

    uint16 FunctionBody::GetMinFunctionProfileIterations()
    {
        return GetMinProfileIterations();
    }

    uint FunctionBody::GetMinLoopProfileIterations(const uint loopInterpreterLimit)
    {
        return min(static_cast<uint>(GetMinProfileIterations()), loopInterpreterLimit);
    }

    uint FunctionBody::GetLoopProfileThreshold(const uint loopInterpreterLimit) const
    {
        return
            DoInterpreterProfile()
                ? DoInterpreterAutoProfile()
                    ? loopInterpreterLimit - GetMinLoopProfileIterations(loopInterpreterLimit)
                    : 0
                : static_cast<uint>(-1);
    }

    uint FunctionBody::GetReducedLoopInterpretCount()
    {
        const uint loopInterpretCount = CONFIG_FLAG(LoopInterpretCount);
        if(CONFIG_ISENABLED(LoopInterpretCountFlag))
        {
            return loopInterpretCount;
        }
        return max(loopInterpretCount / 3, GetMinLoopProfileIterations(loopInterpretCount));
    }

    uint FunctionBody::GetLoopInterpretCount(LoopHeader* loopHeader) const
    {
        if(loopHeader->isNested)
        {
            Assert(loopInterpreterLimit >= GetReducedLoopInterpretCount());
            return GetReducedLoopInterpretCount();
        }
        return loopInterpreterLimit;
    }

    bool FunctionBody::DoObjectHeaderInlining()
    {
        return !PHASE_OFF1(ObjectHeaderInliningPhase);
    }

    bool FunctionBody::DoObjectHeaderInliningForConstructors()
    {
        return !PHASE_OFF1(ObjectHeaderInliningForConstructorsPhase) && DoObjectHeaderInlining();
    }

    bool FunctionBody::DoObjectHeaderInliningForConstructor(const uint32 inlineSlotCapacity)
    {
        return inlineSlotCapacity == 0 ? DoObjectHeaderInliningForEmptyObjects() : DoObjectHeaderInliningForConstructors();
    }

    bool FunctionBody::DoObjectHeaderInliningForObjectLiterals()
    {
        return !PHASE_OFF1(ObjectHeaderInliningForObjectLiteralsPhase) && DoObjectHeaderInlining();
    }

    bool FunctionBody::DoObjectHeaderInliningForObjectLiteral(const uint32 inlineSlotCapacity)
    {
        return
            inlineSlotCapacity == 0
                ?   DoObjectHeaderInliningForEmptyObjects()
                :   DoObjectHeaderInliningForObjectLiterals() &&
                    inlineSlotCapacity <= static_cast<uint32>(MaxPreInitializedObjectHeaderInlinedTypeInlineSlotCount);
    }

    bool FunctionBody::DoObjectHeaderInliningForObjectLiteral(
        const PropertyIdArray *const propIds,
        ScriptContext *const scriptContext)
    {
        Assert(propIds);
        Assert(scriptContext);

        return
            DoObjectHeaderInliningForObjectLiteral(propIds->count) &&
            PathTypeHandlerBase::UsePathTypeHandlerForObjectLiteral(propIds, scriptContext);
    }

    bool FunctionBody::DoObjectHeaderInliningForEmptyObjects()
    {
        #pragma prefast(suppress:6237, "OACR is incompetent - (<zero> && <expression>) is always zero. <expression> is never evaluated and might have side effects.")
        return PHASE_ON1(ObjectHeaderInliningForEmptyObjectsPhase) && DoObjectHeaderInlining();
    }

    void FunctionBody::Finalize(bool isShutdown)
    {
#if ENABLE_DEBUG_CONFIG_OPTIONS
        if (Js::Configuration::Global.flags.Instrument.IsEnabled(Js::LinearScanPhase, this->GetSourceContextId(), this->GetLocalFunctionId()))
        {
            this->DumpRegStats(this);
        }
#endif
        this->Cleanup(isShutdown);
        this->CleanupSourceInfo(isShutdown);
        this->ClearNestedFunctionParentFunctionReference();
        this->CleanupFunctionProxyCounters();
    }

    void FunctionBody::CleanupSourceInfo(bool isScriptContextClosing)
    {
        Assert(this->cleanedUp);

        if (!sourceInfoCleanedUp)
        {
            if (GetIsFuncRegistered() && !isScriptContextClosing)
            {
                // If our function is registered, then there must
                // be a utf8 source info pinned by it.
                Assert(this->m_utf8SourceInfo);

                this->m_utf8SourceInfo->RemoveFunctionBody(this);
            }

            if (this->m_sourceInfo.pSpanSequence != null)
            {
                HeapDelete(this->m_sourceInfo.pSpanSequence);
                this->m_sourceInfo.pSpanSequence = null;
            }

            sourceInfoCleanedUp = true;
        }
    }

    template<bool IsScriptContextShutdown>
    void FunctionBody::CleanUpInlineCaches()
    {
        uint unregisteredInlineCacheCount = 0;

        if (NULL != this->inlineCaches)
        {
            // Inline caches are in this order
            //      plain inline cache
            //      root object load inline cache
            //      root object store inline cache
            //      isInst inline cache
            // The inlineCacheCont include all but isInst inline cache

            uint i = 0;
            uint plainInlineCacheEnd = GetRootObjectLoadInlineCacheStart();
            for (; i < plainInlineCacheEnd; i++)
            {
                if (NULL != this->inlineCaches[i])
                {
                    InlineCache* inlineCache = (InlineCache*)this->inlineCaches[i];
                    if (IsScriptContextShutdown)
                    {
                        memset(inlineCache, 0, sizeof(InlineCache));
                    }
                    else
                    {
                        if (inlineCache->RemoveFromInvalidationList())
                        {
                            unregisteredInlineCacheCount++;
                        }
                        AllocatorDelete(InlineCacheAllocator, this->m_scriptContext->GetInlineCacheAllocator(), inlineCache);
                    }
                }
            }

            RootObjectBase * rootObjectBase = this->GetRootObject();
            uint rootObjectLoadInlineCacheEnd = GetRootObjectLoadMethodInlineCacheStart();
            for (; i < rootObjectLoadInlineCacheEnd; i++)
            {
                if (NULL != this->inlineCaches[i])
                {
                    InlineCache* inlineCache = (InlineCache*)this->inlineCaches[i];
                    if (IsScriptContextShutdown)
                    {
                        memset(inlineCache, 0, sizeof(InlineCache));
                    }
                    else
                    {
                        // A single root object inline caches for a given property is shared by all functions.  It is ref counted
                        // and doesn't get released to the allocator until there are no more outstanding references.  Thus we don't need
                        // to (and, in fact, cannot) remove it from the invalidation list here.  Instead, we'll do it in ReleaseInlineCache
                        // when there are no more outstanding references.
                        rootObjectBase->ReleaseInlineCache(this->GetPropertyIdFromCacheId(i), false, false, IsScriptContextShutdown);
                    }
                }
            }

            uint rootObjectLoadMethodInlineCacheEnd = GetRootObjectStoreInlineCacheStart();
            for (; i < rootObjectLoadMethodInlineCacheEnd; i++)
            {
                if (NULL != this->inlineCaches[i])
                {
                    InlineCache* inlineCache = (InlineCache*)this->inlineCaches[i];
                    if (IsScriptContextShutdown)
                    {
                        memset(inlineCache, 0, sizeof(InlineCache));
                    }
                    else
                    {
                        // A single root object inline caches for a given property is shared by all functions.  It is ref counted
                        // and doesn't get released to the allocator until there are no more outstanding references.  Thus we don't need
                        // to (and, in fact, cannot) remove it from the invalidation list here.  Instead, we'll do it in ReleaseInlineCache
                        // when there are no more outstanding references.
                        rootObjectBase->ReleaseInlineCache(this->GetPropertyIdFromCacheId(i), true, false, IsScriptContextShutdown);
                    }
                }
            }

            uint rootObjectStoreInlineCacheEnd = this->GetInlineCacheCount();
            for (; i < rootObjectStoreInlineCacheEnd; i++)
            {
                if (NULL != this->inlineCaches[i])
                {
                    InlineCache* inlineCache = (InlineCache*)this->inlineCaches[i];
                    if (IsScriptContextShutdown)
                    {
                        memset(inlineCache, 0, sizeof(InlineCache));
                    }
                    else
                    {
                        // A single root object inline caches for a given property is shared by all functions.  It is ref counted
                        // and doesn't get released to the allocator until there are no more outstanding references.  Thus we don't need
                        // to (and, in fact, cannot) remove it from the invalidation list here.  Instead, we'll do it in ReleaseInlineCache
                        // when there are no more outstanding references.
                        rootObjectBase->ReleaseInlineCache(this->GetPropertyIdFromCacheId(i), false, true, IsScriptContextShutdown);
                    }
                }
            }

            uint totalCacheCount = inlineCacheCount + GetIsInstInlineCacheCount();
            for (; i < totalCacheCount; i++)
            {
                if (NULL != this->inlineCaches[i])
                {
                    IsInstInlineCache* inlineCache = (IsInstInlineCache*)this->inlineCaches[i];
                    if (IsScriptContextShutdown)
                    {
                        memset(inlineCache, 0, sizeof(IsInstInlineCache));
                    }
                    else
                    {
                        AllocatorDelete(IsInstInlineCacheAllocator, this->m_scriptContext->GetIsInstInlineCacheAllocator(), inlineCache);
                    }
                }
            }

            this->inlineCaches = NULL;

        }

        if (NULL != this->m_codeGenRuntimeData)
        {
            for (ProfileId i = 0; i < this->profiledCallSiteCount; i++)
            {
                const FunctionCodeGenRuntimeData* runtimeData = this->m_codeGenRuntimeData[i];
                if (NULL != runtimeData)
                {
                    runtimeData->MapInlineCaches([&](InlineCache* inlineCache)
                    {
                        if (NULL != inlineCache)
                        {
                            if (IsScriptContextShutdown)
                            {
                                memset(inlineCache, 0, sizeof(InlineCache));
                            }
                            else
                            {
                                if (inlineCache->RemoveFromInvalidationList())
                                {
                                    unregisteredInlineCacheCount++;
                                }
                                AllocatorDelete(InlineCacheAllocator, this->m_scriptContext->GetInlineCacheAllocator(), inlineCache);
                            }
                        }
                    });
                }
            }
        }

        if (NULL != this->m_codeGenGetSetRuntimeData)
        {
            for (uint i = 0; i < this->GetInlineCacheCount(); i++)
            {
                const FunctionCodeGenRuntimeData* runtimeData = this->m_codeGenGetSetRuntimeData[i];
                if (NULL != runtimeData)
                {
                    runtimeData->MapInlineCaches([&](InlineCache* inlineCache)
                    {
                        if (NULL != inlineCache)
                        {
                            if (IsScriptContextShutdown)
                            {
                                memset(inlineCache, 0, sizeof(InlineCache));
                            }
                            else
                            {
                                if (inlineCache->RemoveFromInvalidationList())
                                {
                                    unregisteredInlineCacheCount++;
                                }
                                AllocatorDelete(InlineCacheAllocator, this->m_scriptContext->GetInlineCacheAllocator(), inlineCache);
                            }
                        }
                    });
                }
            }
        }

        if (!IsScriptContextShutdown)
        {
            ThreadContext* threadContext = this->m_scriptContext->GetThreadContext();
            if (unregisteredInlineCacheCount > 0)
            {
                threadContext->NotifyInlineCacheBatchUnregistered(unregisteredInlineCacheCount);
            }
        }

        while (polymorphicInlineCachesHead)
        {
            polymorphicInlineCachesHead->Finalize(IsScriptContextShutdown);
        }
        polymorphicInlineCaches.Reset();
    }

    void FunctionBody::CleanupRecyclerData(bool isShutdown, bool doEntryPointCleanupCaptureStack)
    {
        // If we're not shutting down (as in closing the script context), we need to remove our inline caches from
        // thread context's invalidation lists, and release memory back to the arena.  During script context shutdown,
        // we leave everything in place, because the inline cache arena will stay alive until script context is destroyed
        // (as in destructor has been called) and thus the invadation lists are safe to keep references to caches from this
        // script context.  We will, however, zero all inline caches so that we don't have to process them on subsequent
        // collections, which may still happen from other script contexts.

        if (isShutdown)
        {
            CleanUpInlineCaches<true>();
        }
        else
        {
            CleanUpInlineCaches<false>();
        }

        if (this->entryPoints)
        {
#if defined(ENABLE_DEBUG_CONFIG_OPTIONS) && !(DBG)
            // On fretest builds, capture the stack only if the FreTestDiagMode switch is on
            doEntryPointCleanupCaptureStack = doEntryPointCleanupCaptureStack && Js::Configuration::Global.flags.FreTestDiagMode;
#endif

            this->MapEntryPoints([=](int index, FunctionEntryPointInfo* entryPoint)
            {
                if (NULL != entryPoint)
                {
                    // Finalize = Free up work item if it hasn't been released yet + clean up
                    // isShutdown is false because cleanup is called only in the !isShutdown case
                    entryPoint->Finalize(isShutdown);

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
                    // Do this seperately since calling EntryPoint::Finalize doesn't capture the stack trace
                    // and in some calls to CleanupRecyclerData, we do want the stack trace captured.

                    if (doEntryPointCleanupCaptureStack)
                    {
                        entryPoint->CaptureCleanupStackTrace();
                    }
#endif
                }
            });

            this->MapLoopHeaders([=](uint loopNumber, LoopHeader* header)
            {
                bool shuttingDown = isShutdown;
                header->MapEntryPoints([=](int index, LoopEntryPointInfo* entryPoint)
                {
                    entryPoint->Cleanup(shuttingDown, doEntryPointCleanupCaptureStack);
                });
            });
        }

#ifdef PERF_COUNTERS
        this->CleanupPerfCounter();
#endif
    }

    //
    // Removes all references of the function body and causes clean up of entry points.
    // If the cleanup has already occurred before this would be a no-op.
    //
    void FunctionBody::Cleanup(bool isScriptContextClosing)
    {
        if (cleanedUp)
        {
            return;
        }

        CleanupRecyclerData(isScriptContextClosing, false /* capture entry point cleanup stack trace */);
        this->ResetObjectLiteralTypes();

        // Manually clear these values to break any circular references
        // that might prevent the script context from being disposed
        this->auxBlock = null;
        this->auxContextBlock = null;
        this->byteCodeBlock = null;
        this->entryPoints = null;
        this->loopHeaderArray = null;
        this->m_constTable = null;
        this->m_codeGenRuntimeData = null;
        this->m_codeGenGetSetRuntimeData = null;
        this->inlineCaches = null;
        this->polymorphicInlineCaches.Reset();
        this->polymorphicInlineCachesHead = null;
        this->cacheIdToPropertyIdMap = null;
        this->referencedPropertyIdMap = null;
        this->literalRegexes = null;
        this->propertyIdsForScopeSlotArray = null;
        this->propertyIdOnRegSlotsContainer = null;

        if (this->HasInterpreterThunkGenerated())
        {
            EtwTrace::LogMethodInterpreterThunkUnloadEvent(this);

            if (!isScriptContextClosing)
            {
                m_scriptContext->ReleaseDynamicInterpreterThunk((BYTE*)this->m_dynamicInterpreterThunk, /*addtoFreeList*/!isScriptContextClosing);
            }
        }

        this->polymorphicCallSiteInfoHead = null;
        this->cleanedUp = true;
    }


#ifdef PERF_COUNTERS
    void FunctionBody::CleanupPerfCounter()
    {
        // We might have byte code block yet if we defer parsed.
        DWORD byteCodeSize = (this->byteCodeBlock? this->byteCodeBlock->GetLength() : 0)
            + (this->auxBlock? this->auxBlock->GetLength() : 0)
            + (this->auxContextBlock? this->auxContextBlock->GetLength() : 0);
        PERF_COUNTER_SUB(Code, DynamicByteCodeSize, byteCodeSize);

        if (this->m_isDeserializedFunction)
        {
            PERF_COUNTER_DEC(Code, DeserializedFunctionBody);
        }

        PERF_COUNTER_SUB(Code, TotalByteCodeSize, byteCodeSize);
    }
#endif

    void FunctionBody::CaptureDynamicProfileState(FunctionEntryPointInfo* entryPointInfo)
    {
        // (See also the FunctionBody member written in CaptureDymamicProfileState.)
        this->savedPolymorphicCacheState = entryPointInfo->GetPendingPolymorphicCacheState();
        this->savedInlinerVersion = entryPointInfo->GetPendingInlinerVersion();
        this->savedImplicitCallsFlags = entryPointInfo->GetPendingImplicitCallFlags();
    }

    BYTE FunctionBody::GetSavedInlinerVersion() const
    {
        Assert(this->dynamicProfileInfo != NULL);
        return this->savedInlinerVersion;
    }

    uint32 FunctionBody::GetSavedPolymorphicCacheState() const
    {
        Assert(this->dynamicProfileInfo != NULL);
        return this->savedPolymorphicCacheState;
    }

    void FunctionBody::SetHasHotLoop()
    {
        if(hasHotLoop)
        {
            return;
        }
        hasHotLoop = true;

        if(Configuration::Global.flags.EnforceExecutionModeLimits)
        {
            return;
        }

        CommitExecutedIterations();
        TraceExecutionMode("HasHotLoop (before)");
        if(fullJitThreshold > 1)
        {
            SetFullJitThreshold(1, true);
        }
        TraceExecutionMode("HasHotLoop");
    }

    bool FunctionBody::IsInlineApplyDisabled()
    {
        return this->disableInlineApply;
    }

    void FunctionBody::SetDisableInlineApply(bool set)
    {
        this->disableInlineApply = set;
    }

    void FunctionBody::InitDisableInlineApply()
    {
        SetDisableInlineApply(this->functionId != Js::Constants::NoFunctionId && PHASE_OFF(Js::InlinePhase, this) || PHASE_OFF(Js::InlineApplyPhase, this));        
    }

    bool FunctionBody::CheckCalleeContextForInlining(FunctionProxy* calleeFunctionProxy)
    {
        if (this->GetScriptContext() == calleeFunctionProxy->GetScriptContext())
        {
            // TODO: For now, only function from the same source file can be encoded
            if (this->GetHostSourceContext() == calleeFunctionProxy->GetHostSourceContext() &&
                this->GetSecondaryHostSourceContext() == calleeFunctionProxy->GetSecondaryHostSourceContext())
            {
                return true;
            }
        }
        return false;
    }

    ImplicitCallFlags FunctionBody::GetSavedImplicitCallsFlags() const
    {
        Assert(this->dynamicProfileInfo != NULL);
        return this->savedImplicitCallsFlags;
    }

    bool FunctionBody::HasNonBuiltInCallee()
    {
        for (ProfileId i = 0; i < profiledCallSiteCount; i++)
        {
            Assert(HasDynamicProfileInfo());
            bool ctor;
            bool isPolymorphic;
            FunctionInfo *info = dynamicProfileInfo->GetCallSiteInfo(this, i, &ctor, &isPolymorphic);
            if (info == null || info->HasBody())
            {
                return true;
            }
        }
        return false;
    }

    void FunctionBody::CheckAndRegisterFuncToDiag(ScriptContext *scriptContext)
    {
        // We will register function if, this is not host managed and it was not registered before.
        if (!BinaryFeatureControl::LanguageService()
            && GetHostSourceContext() == Js::Constants::NoHostSourceContext
            && !m_isFuncRegisteredToDiag
            && !scriptContext->diagProbesContainer.IsContextRegistered(GetSecondaryHostSourceContext()))
        {
            FunctionBody *pFunc = scriptContext->diagProbesContainer.GetGlobalFunc(scriptContext, GetSecondaryHostSourceContext());
            if (pFunc)
            {
                // Existing behaviour here is to ignore the OOM and since this function
                // can throw now, we simply ignore the OOM here
                try
                {
                    // Register the function to the PDM as eval code (the debugger app will show file as 'eval code')
                    scriptContext->DbgRegisterFunction(pFunc, Constants::EvalCode);
                }
                catch (Js::OutOfMemoryException)
                {
                }

                scriptContext->diagProbesContainer.RegisterContextToDiag(GetSecondaryHostSourceContext(), scriptContext->AllocatorForDiagnostics());

                m_isFuncRegisteredToDiag = true;
            }
        }
        else
        {
            m_isFuncRegisteredToDiag = true;
        }

    }

    DebuggerScope* FunctionBody::RecordStartScopeObject(DiagExtraScopesType scopeType, int start, RegSlot scopeLocation, int* index)
    {
        Recycler* recycler = m_scriptContext->GetRecycler();

        if (!GetScopeObjectChain())
        {
            SetScopeObjectChain(RecyclerNew(recycler, ScopeObjectChain, recycler));
        }

        // Check if we need to create the scope object or if it already exists from a previous bytecode
        // generator pass.
        DebuggerScope* debuggerScope = nullptr;
        int currentDebuggerScopeIndex = this->GetNextDebuggerScopeIndex();
        if (!this->TryGetDebuggerScopeAt(currentDebuggerScopeIndex, debuggerScope))
        {
            // Create a new debugger scope.
            debuggerScope = AddScopeObject(scopeType, start, scopeLocation);
        }
        else
        {
            debuggerScope->UpdateDueToByteCodeRegeneration(scopeType, start, scopeLocation);
        }

        if(index)
        {
            *index = currentDebuggerScopeIndex;
        }

        return debuggerScope;
    }

    void FunctionBody::RecordEndScopeObject(DebuggerScope* currentScope, int end)
    {
        AssertMsg(currentScope, "No current debugger scope passed in.");
        currentScope->SetEnd(end);
    }

    DebuggerScope * FunctionBody::AddScopeObject(DiagExtraScopesType scopeType, int start, RegSlot scopeLocation)
    {
        Assert(GetScopeObjectChain());

        DebuggerScope *scopeObject = RecyclerNew(m_scriptContext->GetRecycler(), DebuggerScope, m_scriptContext->GetRecycler(), scopeType, scopeLocation, start);
        GetScopeObjectChain()->pScopeChain->Add(scopeObject);

        return scopeObject;
    }

    // Tries to retrieve the debugger scope at the specified index.  If the index is out of range, nullptr
    // is returned.
    bool FunctionBody::TryGetDebuggerScopeAt(int index, DebuggerScope*& debuggerScope)
    {
        AssertMsg(this->GetScopeObjectChain(), "TryGetDebuggerScopeAt should only be called with a valid scope chain in place.");
        Assert(index >= 0);

        const Js::ScopeObjectChain::ScopeObjectChainList* scopeChain = this->GetScopeObjectChain()->pScopeChain;
        if (index < scopeChain->Count())
        {
            debuggerScope = scopeChain->Item(index);
            return true;
        }

        return false;
    }

    DWORD FunctionBody::GetDynamicInterpreterThunkSize() const
    {
        return InterpreterThunkEmitter::ThunkSize;
    }

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    void
    FunctionBody::DumpFullFunctionName()
    {
        wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];

        Output::Print(L"Function %s (%s)", this->GetDisplayName(), this->GetDebugNumberSet(debugStringBuffer));
    }

    void FunctionBody::DumpFunctionId(bool pad)
    {
        uint sourceContextId = this->GetSourceContextInfo()->sourceContextId;
        if (sourceContextId == Js::Constants::NoSourceContext)
        {
            if (this->IsDynamicScript())
            {
                Output::Print(pad? L"Dy.%-3d" : L"Dyn#%d", this->GetLocalFunctionId());
            }
            else
            {
                // Function from LoadFile
                Output::Print(pad? L"%-5d" : L"#%d", this->GetLocalFunctionId());
            }
        }
        else
        {
            Output::Print(pad? L"%2d.%-3d" : L"#%d.%d", sourceContextId, this->GetLocalFunctionId());
        }
    }

#endif

    void FunctionBody::EnsureAuxStatementData()
    {
        if (m_sourceInfo.m_auxStatementData == NULL)
        {
            Recycler* recycler = m_scriptContext->GetRecycler();

            // Note: allocating must be consistent with clean up in CleanupToReparse.
            m_sourceInfo.m_auxStatementData = RecyclerNew(recycler, AuxStatementData);
        }
    }

    /*static*/
    void FunctionBody::GetShortNameFromUrl(__in LPCWSTR pchUrl, __RPC__in_ecount_full(cchBuffer) LPWSTR pchShortName, __in size_t cchBuffer)
    {
        // Note : We can use help from the wininet for cracking the url properly. but for now below logic will just do.

        LPWSTR pchFile = wcsrchr(pchUrl, L'/');
        if (pchFile == NULL)
        {
            pchFile = wcsrchr(pchUrl, L'\\');
        }

        LPCWSTR pchToCopy = pchUrl;

        if (pchFile != NULL)
        {
            pchToCopy = pchFile + 1;
        }

        wcscpy_s(pchShortName, cchBuffer, pchToCopy);
    }

    FunctionBody::StatementAdjustmentRecordList* FunctionBody::GetStatementAdjustmentRecords()
    {
        if (m_sourceInfo.m_auxStatementData)
        {
            return m_sourceInfo.m_auxStatementData->m_statementAdjustmentRecords;
        }
        return NULL;
    }

    FunctionBody::CrossFrameEntryExitRecordList* FunctionBody::GetCrossFrameEntryExitRecords()
    {
        if (m_sourceInfo.m_auxStatementData)
        {
            return m_sourceInfo.m_auxStatementData->m_crossFrameBlockEntryExisRecords;
        }
        return NULL;
    }

    void FunctionBody::RecordCrossFrameEntryExitRecord(uint byteCodeOffset, bool isEnterBlock)
    {
        this->EnsureAuxStatementData();

        Recycler* recycler = this->m_scriptContext->GetRecycler();
        if (this->GetCrossFrameEntryExitRecords() == NULL)
        {
            m_sourceInfo.m_auxStatementData->m_crossFrameBlockEntryExisRecords = RecyclerNew(recycler, CrossFrameEntryExitRecordList, recycler);
        }
        Assert(this->GetCrossFrameEntryExitRecords());

        CrossFrameEntryExitRecord record(byteCodeOffset, isEnterBlock);
        this->GetCrossFrameEntryExitRecords()->Add(record); // Will copy stack value and put the copy into the container.
    }

    FunctionBody::AuxStatementData::AuxStatementData() : m_statementAdjustmentRecords(NULL), m_crossFrameBlockEntryExisRecords(NULL)
    {
    }

    FunctionBody::StatementAdjustmentRecord::StatementAdjustmentRecord() :
        m_byteCodeOffset((uint)Constants::InvalidOffset), m_adjustmentType(SAT_None)
    {
    }

    FunctionBody::StatementAdjustmentRecord::StatementAdjustmentRecord(StatementAdjustmentType type, int byteCodeOffset) :
        m_adjustmentType(type), m_byteCodeOffset(byteCodeOffset)
    {
        Assert(SAT_None <= type && type <= SAT_All);
    }

    FunctionBody::StatementAdjustmentRecord::StatementAdjustmentRecord(const StatementAdjustmentRecord& other) :
        m_byteCodeOffset(other.m_byteCodeOffset), m_adjustmentType(other.m_adjustmentType)
    {
    }

    uint FunctionBody::StatementAdjustmentRecord::GetByteCodeOffset()
    {
        Assert(m_byteCodeOffset != Constants::InvalidOffset);
        return m_byteCodeOffset;
    }

    FunctionBody::StatementAdjustmentType FunctionBody::StatementAdjustmentRecord::GetAdjustmentType()
    {
        Assert(this->m_adjustmentType != SAT_None);
        return m_adjustmentType;
    }

    FunctionBody::CrossFrameEntryExitRecord::CrossFrameEntryExitRecord() :
        m_byteCodeOffset((uint)Constants::InvalidOffset), m_isEnterBlock(false)
    {
    }

    FunctionBody::CrossFrameEntryExitRecord::CrossFrameEntryExitRecord(uint byteCodeOffset, bool isEnterBlock) :
        m_byteCodeOffset(byteCodeOffset), m_isEnterBlock(isEnterBlock)
    {
    }

    FunctionBody::CrossFrameEntryExitRecord::CrossFrameEntryExitRecord(const CrossFrameEntryExitRecord& other) :
        m_byteCodeOffset(other.m_byteCodeOffset), m_isEnterBlock(other.m_isEnterBlock)
    {
    }

    uint FunctionBody::CrossFrameEntryExitRecord::GetByteCodeOffset() const
    {
        Assert(m_byteCodeOffset != Constants::InvalidOffset);
        return m_byteCodeOffset;
    }

    bool FunctionBody::CrossFrameEntryExitRecord::GetIsEnterBlock()
    {
        return m_isEnterBlock;
    }

    PolymorphicInlineCacheInfo * EntryPointPolymorphicInlineCacheInfo::GetInlineeInfo(FunctionBody * inlineeFunctionBody)
    {
        SListBase<PolymorphicInlineCacheInfo*>::Iterator iter(&inlineeInfo);
        while (iter.Next())
        {
            PolymorphicInlineCacheInfo * info = iter.Data();
            if (info->GetFunctionBody() == inlineeFunctionBody)
            {
                return info;
            }
        }

        return null;
    }

    PolymorphicInlineCacheInfo * EntryPointPolymorphicInlineCacheInfo::EnsureInlineeInfo(Recycler * recycler, FunctionBody * inlineeFunctionBody)
    {
        PolymorphicInlineCacheInfo * info = GetInlineeInfo(inlineeFunctionBody);
        if (!info)
        {
            info = RecyclerNew(recycler, PolymorphicInlineCacheInfo, inlineeFunctionBody);
            inlineeInfo.Prepend(recycler, info);
        }
        return info;
    }

    void EntryPointPolymorphicInlineCacheInfo::SetPolymorphicInlineCache(FunctionBody * functionBody, uint index, PolymorphicInlineCache * polymorphicInlineCache, bool isInlinee, byte polyCacheUtil)
    {
        if (!isInlinee)
        {
            SetPolymorphicInlineCache(&selfInfo, functionBody, index, polymorphicInlineCache, polyCacheUtil);
            Assert(functionBody == selfInfo.GetFunctionBody());
        }
        else
        {
            SetPolymorphicInlineCache(EnsureInlineeInfo(functionBody->GetScriptContext()->GetRecycler(), functionBody), functionBody, index, polymorphicInlineCache, polyCacheUtil);
            Assert(functionBody == GetInlineeInfo(functionBody)->GetFunctionBody());
        }
    }

    void EntryPointPolymorphicInlineCacheInfo::SetPolymorphicInlineCache(PolymorphicInlineCacheInfo * polymorphicInlineCacheInfo, FunctionBody * functionBody, uint index, PolymorphicInlineCache * polymorphicInlineCache, byte polyCacheUtil)
    {
        polymorphicInlineCacheInfo->GetPolymorphicInlineCaches()->SetInlineCache(functionBody->GetScriptContext()->GetRecycler(), functionBody, index, polymorphicInlineCache);
        polymorphicInlineCacheInfo->GetUtilArray()->SetUtil(functionBody, index, polyCacheUtil);
    }

    void PolymorphicCacheUtilizationArray::SetUtil(Js::FunctionBody* functionBody, uint index, byte util)
    {
        Assert(functionBody);
        Assert(index < functionBody->GetInlineCacheCount());

        EnsureUtilArray(functionBody->GetScriptContext()->GetRecycler(), functionBody);
        this->utilArray[index] = util;
    }

    byte PolymorphicCacheUtilizationArray::GetUtil(Js::FunctionBody* functionBody, uint index)
    {
        Assert(index < functionBody->GetInlineCacheCount());
        return this->utilArray[index];
    }

    void PolymorphicCacheUtilizationArray::EnsureUtilArray(Recycler * const recycler, Js::FunctionBody * functionBody)
    {
        Assert(recycler);
        Assert(functionBody);
        Assert(functionBody->GetInlineCacheCount() != 0);

        if(this->utilArray)
        {
            return;
        }

        this->utilArray = RecyclerNewArrayZ(recycler, byte, functionBody->GetInlineCacheCount());
    }

    void EntryPointInfo::AddWeakFuncRef(RecyclerWeakReference<FunctionBody> *weakFuncRef, Recycler *recycler)
    {
        Assert(this->state == CodeGenPending);

        this->weakFuncRefSet = this->EnsureWeakFuncRefSet(recycler);
        this->weakFuncRefSet->AddNew(weakFuncRef);
    }

    EntryPointInfo::WeakFuncRefSet *
    EntryPointInfo::EnsureWeakFuncRefSet(Recycler *recycler)
    {
        if (this->weakFuncRefSet == null)
        {
            this->weakFuncRefSet = RecyclerNew(recycler, WeakFuncRefSet, recycler);
        }

        return this->weakFuncRefSet;
    }

    void EntryPointInfo::EnsureIsReadyToCall()
    {
        ProcessJitTransferData();
    }

    void EntryPointInfo::ProcessJitTransferData()
    {
        Assert(!IsCleanedUp());
        if (GetJitTransferData() != null && GetJitTransferData()->GetIsReady())
        {
            class AutoCleanup
            {
                EntryPointInfo *entryPointInfo;
            public:
                AutoCleanup(EntryPointInfo *entryPointInfo) : entryPointInfo(entryPointInfo)
                {
                }

                void Done()
                {
                    entryPointInfo = null;
                }
                ~AutoCleanup()
                {
                    if (entryPointInfo)
                    {
                        entryPointInfo->OnNativeCodeInstallFailure();
                    }
                }
            } autoCleanup(this);

            ScriptContext* scriptContext = GetScriptContext();
            PinTypeRefs(scriptContext);
            InstallGuards(scriptContext);
            FreeJitTransferData();

            autoCleanup.Done();
        }
    }

    EntryPointInfo::JitTransferData* EntryPointInfo::EnsureJitTransferData(Recycler* recycler)
    {
        if (this->jitTransferData == null)
        {
            this->jitTransferData = RecyclerNew(recycler, EntryPointInfo::JitTransferData);
        }
        return this->jitTransferData;
    }

#ifdef FIELD_ACCESS_STATS
    FieldAccessStats* EntryPointInfo::EnsureFieldAccessStats(Recycler* recycler)
    {
        if (this->fieldAccessStats == null)
        {
            this->fieldAccessStats = RecyclerNew(recycler, FieldAccessStats);
        }
        return this->fieldAccessStats;
    }
#endif

    void EntryPointInfo::JitTransferData::AddJitTimeTypeRef(void* typeRef, Recycler* recycler)
    {
        Assert(typeRef != null);
        EnsureJitTimeTypeRefs(recycler);
        this->jitTimeTypeRefs->AddNew(typeRef);
    }

    void EntryPointInfo::JitTransferData::EnsureJitTimeTypeRefs(Recycler* recycler)
    {
        if (this->jitTimeTypeRefs == null)
        {
            this->jitTimeTypeRefs = RecyclerNew(recycler, TypeRefSet, recycler);
        }
    }

    void EntryPointInfo::PinTypeRefs(ScriptContext* scriptContext)
    {
        Assert(this->jitTransferData != null && this->jitTransferData->GetIsReady());

        Recycler* recycler = scriptContext->GetRecycler();
        if (this->jitTransferData->GetRuntimeTypeRefs() != null)
        {
            // Copy pinned types from a heap allocated array created on the background thread (and will be freed at the end of
            // NativeCodeGenerator::CheckCodeGenDone) to a recycler allocated array which will live as long as this EntryPointInfo.
            void** jitPinnedTypeRefs = this->jitTransferData->GetRuntimeTypeRefs();
            size_t jitPinnedTypeRefCount = this->jitTransferData->GetRuntimeTypeRefCount();
            this->runtimeTypeRefs = RecyclerNewArray(recycler, void*, jitPinnedTypeRefCount + 1);
            js_memcpy_s(this->runtimeTypeRefs, jitPinnedTypeRefCount * sizeof(void*), jitPinnedTypeRefs, jitPinnedTypeRefCount * sizeof(void*));
            this->runtimeTypeRefs[jitPinnedTypeRefCount] = null;
        }
    }

    void EntryPointInfo::InstallGuards(ScriptContext* scriptContext)
    {
        Assert(this->jitTransferData != null && this->jitTransferData->GetIsReady());
        Assert(this->equivalentTypeCacheCount == 0 && this->equivalentTypeCaches == null);
        Assert(this->propertyGuardCount == 0 && this->propertyGuardWeakRefs == null);

        class AutoCleanup
        {
            EntryPointInfo *entryPointInfo;
        public:
            AutoCleanup(EntryPointInfo *entryPointInfo) : entryPointInfo(entryPointInfo)
            {
            }

            void Done()
            {
                entryPointInfo = null;
            }
            ~AutoCleanup()
            {
                if (entryPointInfo)
                {
                    entryPointInfo->equivalentTypeCacheCount = 0;
                    entryPointInfo->equivalentTypeCaches = null;
                    entryPointInfo->propertyGuardCount = 0;
                    entryPointInfo->propertyGuardWeakRefs = null;
                    entryPointInfo->UnregisterEquivalentTypeCaches();
                }
            }
        } autoCleanup(this);

        for (int i = 0; i < this->jitTransferData->lazyBailoutPropertyCount; i++)
        {
            Assert(this->jitTransferData->lazyBailoutProperties != nullptr);

            Js::PropertyId propertyId = this->jitTransferData->lazyBailoutProperties[i];
            Js::PropertyGuard* sharedPropertyGuard;
            bool hasSharedPropertyGuard = TryGetSharedPropertyGuard(propertyId, sharedPropertyGuard);
            Assert(hasSharedPropertyGuard);
            bool isValid = hasSharedPropertyGuard ? sharedPropertyGuard->IsValid() : false;
            if (isValid)
            {
                scriptContext->GetThreadContext()->RegisterLazyBailout(propertyId, this);
            }
            else
            {
                OUTPUT_TRACE2(Js::LazyBailoutPhase, this->GetFunctionBody(), L"Lazy bailout - Invalidation due to property: %s \n", scriptContext->GetPropertyName(propertyId)->GetBuffer());
                this->Invalidate(true);
                return;
            }
        }

        if (this->jitTransferData->equivalentTypeGuardCount > 0)
        {
            Assert(this->jitTransferData->equivalentTypeGuards != null);

            Recycler* recycler = scriptContext->GetRecycler();

            int guardCount = this->jitTransferData->equivalentTypeGuardCount;
            JitEquivalentTypeGuard** guards = this->jitTransferData->equivalentTypeGuards;

            // Create an array of equivalent type caches on the entry point info to ensure they are kept
            // alive for the lifetime of the entry point.
            this->equivalentTypeCacheCount = guardCount;

            // No need to zero-initialize, since we will populate all data slots.
            // We used to let the recycler scan the types in the cache, but we no longer do. See
            // ThreadContext::ClearEquivalentTypeCaches for an explanation.
            this->equivalentTypeCaches = RecyclerNewArrayLeafZ(recycler, EquivalentTypeCache, guardCount);

            this->RegisterEquivalentTypeCaches();

            EquivalentTypeCache* cache = this->equivalentTypeCaches;

            for (JitEquivalentTypeGuard** guard = guards; guard < guards + guardCount; guard++)
            {
                EquivalentTypeCache* oldCache = (*guard)->GetCache();
                // Copy the contents of the heap-allocated cache to the recycler-allocated version to make sure the types are
                // kept alive. Allow the properties pointer to refer to the heap-allocated arrays. It will stay alive as long
                // as the entry point is alive, and property entries contain no pointers to other recycler allocated objects.
                (*cache) = (*oldCache);
                // Set the recycler-allocated cache on the (heap-allocated) guard.
                (*guard)->SetCache(cache);
                cache++;
            }
        }

        // The propertyGuardsByPropertyId structure is temporary and serves only to register the type guards for the correct
        // properties.  If we've done code gen for this EntryPointInfo, typePropertyGuardsByPropertyId will have been used and nulled out.
        if (this->jitTransferData->propertyGuardsByPropertyId != null)
        {
            this->propertyGuardCount = this->jitTransferData->propertyGuardCount;
            this->propertyGuardWeakRefs = RecyclerNewArrayZ(scriptContext->GetRecycler(), FakePropertyGuardWeakReference*, this->propertyGuardCount);

            ThreadContext* threadContext = scriptContext->GetThreadContext();

            Js::TypeGuardTransferEntry* entry = this->jitTransferData->propertyGuardsByPropertyId;
            while (entry->propertyId != Js::Constants::NoProperty)
            {
                Js::PropertyId propertyId = entry->propertyId;
                Js::PropertyGuard* sharedPropertyGuard;

                // We use the shared guard created during work item creation to ensure that the condition we assumed didn't change while
                // we were JIT-ing. If we don't have a shared property guard for this property then we must not need to protect it,
                // because it exists on the instance.  Unfortunately, this means that if we have a bug and fail to create a shared
                // guard for some property during work item creation, we won't find out about it here.
                bool isNeeded = TryGetSharedPropertyGuard(propertyId, sharedPropertyGuard);
                bool isValid = isNeeded ? sharedPropertyGuard->IsValid() : false;
                int entryGuardIndex = 0;
                while (entry->guards[entryGuardIndex] != null)
                {
                    if (isNeeded)
                    {
                        Js::JitIndexedPropertyGuard* guard = entry->guards[entryGuardIndex];
                        int guardIndex = guard->GetIndex();
                        Assert(guardIndex >= 0 && guardIndex < this->propertyGuardCount);
                        // We use the shared guard here to make sure the conditions we assumed didn't change while we were JIT-ing.
                        // If they did, we proactively invalidate the guard here, so that we bail out if we try to call this code.
                        if (isValid)
                        {
                            auto propertyGuardWeakRef = this->propertyGuardWeakRefs[guardIndex];
                            if (propertyGuardWeakRef == null)
                            {
                                propertyGuardWeakRef = Js::FakePropertyGuardWeakReference::New(scriptContext->GetRecycler(), guard);
                                this->propertyGuardWeakRefs[guardIndex] = propertyGuardWeakRef;
                            }
                            Assert(propertyGuardWeakRef->Get() == guard);
                            threadContext->RegisterUniquePropertyGuard(propertyId, propertyGuardWeakRef);
                        }
                        else
                        {
                            guard->Invalidate();
                        }
                    }
                    entryGuardIndex++;
                }
                entry = reinterpret_cast<Js::TypeGuardTransferEntry*>(&entry->guards[++entryGuardIndex]);
            }
        }

        // The ctorCacheGuardsByPropertyId structure is temporary and serves only to register the constructor cache guards for the correct
        // properties.  If we've done code gen for this EntryPointInfo, ctorCacheGuardsByPropertyId will have been used and nulled out.
        // Unlike type property guards, constructor cache guards use the live constructor caches associated with function objects. These are
        // recycler allocated and are kept alive by the constructorCaches field, where they were inserted during work item creation.
        if (this->jitTransferData->ctorCacheGuardsByPropertyId != null)
        {
            ThreadContext* threadContext = scriptContext->GetThreadContext();

            Js::CtorCacheGuardTransferEntry* entry = this->jitTransferData->ctorCacheGuardsByPropertyId;
            while (entry->propertyId != Js::Constants::NoProperty)
            {
                Js::PropertyId propertyId = entry->propertyId;
                Js::PropertyGuard* sharedPropertyGuard;

                // We use the shared guard created during work item creation to ensure that the condition we assumed didn't change while
                // we were JIT-ing. If we don't have a shared property guard for this property then we must not need to protect it,
                // because it exists on the instance.  Unfortunately, this means that if we have a bug and fail to create a shared
                // guard for some property during work item creation, we won't find out about it here.
                bool isNeeded = TryGetSharedPropertyGuard(propertyId, sharedPropertyGuard);
                bool isValid = isNeeded ? sharedPropertyGuard->IsValid() : false;
                int entryCacheIndex = 0;
                while (entry->caches[entryCacheIndex] != null)
                {
                    if (isNeeded)
                    {
                        Js::ConstructorCache* cache = entry->caches[entryCacheIndex];
                        // We use the shared cache here to make sure the conditions we assumed didn't change while we were JIT-ing.
                        // If they did, we proactively invalidate the cache here, so that we bail out if we try to call this code.
                        if (isValid)
                        {
                            threadContext->RegisterConstructorCache(propertyId, cache);
                        }
                        else
                        {
                            cache->InvalidateAsGuard();
                        }
                    }
                    entryCacheIndex++;
                }
                entry = reinterpret_cast<Js::CtorCacheGuardTransferEntry*>(&entry->caches[++entryCacheIndex]);
            }
        }

        if (PHASE_ON(Js::FailNativeCodeInstallPhase, this->GetFunctionBody()))
        {
            Js::Throw::OutOfMemory();
        }

        autoCleanup.Done();
    }

    PropertyGuard* EntryPointInfo::RegisterSharedPropertyGuard(Js::PropertyId propertyId, ScriptContext* scriptContext)
    {
        if (this->sharedPropertyGuards == null)
        {
            Recycler* recycler = scriptContext->GetRecycler();
            this->sharedPropertyGuards = RecyclerNew(recycler, SharedPropertyGuardDictionary, recycler);
        }

        PropertyGuard* guard;
        if (!this->sharedPropertyGuards->TryGetValue(propertyId, &guard))
        {
            ThreadContext* threadContext = scriptContext->GetThreadContext();
            guard = threadContext->RegisterSharedPropertyGuard(propertyId);
            this->sharedPropertyGuards->Add(propertyId, guard);
        }

        return guard;
    }

    bool EntryPointInfo::HasSharedPropertyGuard(Js::PropertyId propertyId)
    {
        return this->sharedPropertyGuards != null ? this->sharedPropertyGuards->ContainsKey(propertyId) : false;
    }

    bool EntryPointInfo::TryGetSharedPropertyGuard(Js::PropertyId propertyId, Js::PropertyGuard*& guard)
    {
        return this->sharedPropertyGuards != null ? this->sharedPropertyGuards->TryGetValue(propertyId, &guard) : false;
    }

    void EntryPointInfo::RecordTypeGuards(int typeGuardCount, TypeGuardTransferEntry* typeGuardTransferRecord, size_t typeGuardTransferPlusSize)
    {
        Assert(this->jitTransferData != null);

        this->jitTransferData->propertyGuardCount = typeGuardCount;
        this->jitTransferData->propertyGuardsByPropertyId = typeGuardTransferRecord;
        this->jitTransferData->propertyGuardsByPropertyIdPlusSize = typeGuardTransferPlusSize;
    }

    void EntryPointInfo::RecordCtorCacheGuards(CtorCacheGuardTransferEntry* ctorCacheTransferRecord, size_t ctorCacheTransferPlusSize)
    {
        Assert(this->jitTransferData != null);

        this->jitTransferData->ctorCacheGuardsByPropertyId = ctorCacheTransferRecord;
        this->jitTransferData->ctorCacheGuardsByPropertyIdPlusSize = ctorCacheTransferPlusSize;
    }

    void EntryPointInfo::FreePropertyGuards()
    {
        // While typePropertyGuardWeakRefs are allocated via NativeCodeData::Allocator and will be automatically freed to the heap,
        // we must zero out the fake weak references so that property guard invalidation doesn't access freed memory.
        if (this->propertyGuardWeakRefs != null)
        {
            for (int i = 0; i < this->propertyGuardCount; i++)
            {
                if (this->propertyGuardWeakRefs[i] != null)
                {
                    this->propertyGuardWeakRefs[i]->Zero();
                }
            }
            this->propertyGuardCount = 0;
            this->propertyGuardWeakRefs = null;
        }
    }

    void EntryPointInfo::RecordBailOutMap(JsUtil::List<LazyBailOutRecord, ArenaAllocator>* bailoutMap)
    {
        Assert(this->bailoutRecordMap == nullptr);       
        this->bailoutRecordMap = HeapNew(BailOutRecordMap, &HeapAllocator::Instance);
        this->bailoutRecordMap->Copy(bailoutMap);
    }

    void EntryPointInfo::DoLazyBailout(BYTE** addressOfInstructionPointer, Js::FunctionBody* functionBody, const PropertyRecord* propertyRecord)
    {
        BYTE* instructionPointer = *addressOfInstructionPointer;
        Assert(instructionPointer > (BYTE*)this->nativeAddress && instructionPointer < ((BYTE*)this->nativeAddress + this->codeSize));
        size_t offset = instructionPointer - (BYTE*)this->nativeAddress;
        LazyBailOutRecord record;
        int found = this->bailoutRecordMap->BinarySearch([=](const LazyBailOutRecord& record, int index)
        {
            // find the closest entry which is greater than the current offset.
            if (record.offset >= offset)
            {
                if (index == 0 || index > 0 && this->bailoutRecordMap->Item(index - 1).offset < offset)
                {
                    return 0;
                }
                else
                {
                    return 1;
                }
            }
            return -1;
        });
        if (found != -1)
        {
            LazyBailOutRecord& record = this->bailoutRecordMap->Item(found);
            *addressOfInstructionPointer = record.instructionPointer;
            record.SetBailOutKind();
            if (PHASE_TRACE1(Js::LazyBailoutPhase))
            {
                Output::Print(L"On stack lazy bailout. Property: %s Old IP: 0x%x New IP: 0x%x ", propertyRecord->GetBuffer(), instructionPointer, record.instructionPointer);
#if DBG
                record.Dump(functionBody);
#endif
                Output::Print(L"\n");
            }
        }
        else
        {
            AssertMsg(false, "Lazy Bailout addresss mapping missing");
        }
    }

    void FunctionEntryPointInfo::RecordInlineeFrameMap(JsUtil::List<NativeOffsetInlineeFramePair, ArenaAllocator>* tempInlineeFrameMap)
    {
        Assert(this->inlineeFrameMap == nullptr);
        if (tempInlineeFrameMap->Count() > 0)
        {
            this->inlineeFrameMap = HeapNew(InlineeFrameMap, &HeapAllocator::Instance);
            this->inlineeFrameMap->Copy(tempInlineeFrameMap);
        }
    }

    InlineeFrameRecord* FunctionEntryPointInfo::FindInlineeFrame(void* returnAddress)
    {
        if (this->inlineeFrameMap == nullptr)
        {
            return nullptr;
        }

        size_t offset = (size_t)((BYTE*)returnAddress - (BYTE*)this->GetNativeAddress());
        int index = this->inlineeFrameMap->BinarySearch([=](const NativeOffsetInlineeFramePair& pair, int index){
            if (pair.offset >= offset)
            {
                if (index == 0 || index > 0 && this->inlineeFrameMap->Item(index - 1).offset < offset)
                {
                    return 0;
                }
                else
                {
                    return 1;
                }
            }
            return -1;
        });

        if (index == -1)
        {
            return nullptr;
        }
        return this->inlineeFrameMap->Item(index).record;
    }

    void EntryPointInfo::FreeJitTransferData()
    {
        JitTransferData* jitTransferData = this->jitTransferData;
        this->jitTransferData = null;

        if (jitTransferData != null)
        {
            // This dictionary is recycler allocated so it doesn't need to be explicitly freed.
            jitTransferData->jitTimeTypeRefs = null;

            if (jitTransferData->lazyBailoutProperties != null)
            {
                HeapDeleteArray(jitTransferData->lazyBailoutPropertyCount, jitTransferData->lazyBailoutProperties);
                jitTransferData->lazyBailoutProperties = nullptr;
            }

            // All structures below are heap allocated and need to be freed explicitly.
            if (jitTransferData->runtimeTypeRefs != null)
            {
                HeapDeleteArray(jitTransferData->runtimeTypeRefCount, jitTransferData->runtimeTypeRefs);
                jitTransferData->runtimeTypeRefs = null;
            }
            jitTransferData->runtimeTypeRefCount = 0;

            if (jitTransferData->propertyGuardsByPropertyId != null)
            {
                HeapDeletePlus(jitTransferData->propertyGuardsByPropertyIdPlusSize, jitTransferData->propertyGuardsByPropertyId);
                jitTransferData->propertyGuardsByPropertyId = null;
            }
            jitTransferData->propertyGuardCount = 0;
            jitTransferData->propertyGuardsByPropertyIdPlusSize = 0;

            if (jitTransferData->ctorCacheGuardsByPropertyId != null)
            {
                HeapDeletePlus(jitTransferData->ctorCacheGuardsByPropertyIdPlusSize, jitTransferData->ctorCacheGuardsByPropertyId);
                jitTransferData->ctorCacheGuardsByPropertyId = null;
            }
            jitTransferData->ctorCacheGuardsByPropertyIdPlusSize = 0;

            if (jitTransferData->equivalentTypeGuards != null)
            {
                HeapDeleteArray(jitTransferData->equivalentTypeGuardCount, jitTransferData->equivalentTypeGuards);
                jitTransferData->equivalentTypeGuards = null;
            }
            jitTransferData->equivalentTypeGuardCount = 0;

            if (jitTransferData->data != null)
            {
                HeapDelete(jitTransferData->data);
                jitTransferData->data = null;
            }

            jitTransferData = null;
        }
    }

    void EntryPointInfo::RegisterEquivalentTypeCaches()
    {
        Assert(this->registeredEquivalentTypeCacheRef == nullptr);
        this->registeredEquivalentTypeCacheRef =
            GetScriptContext()->GetThreadContext()->RegisterEquivalentTypeCacheEntryPoint(this);
    }

    void EntryPointInfo::UnregisterEquivalentTypeCaches()
    {
        if (this->registeredEquivalentTypeCacheRef != nullptr)
        {
            ScriptContext *scriptContext = GetScriptContext();
            if (scriptContext != nullptr)
            {
                scriptContext->GetThreadContext()->UnregisterEquivalentTypeCacheEntryPoint(
                    this->registeredEquivalentTypeCacheRef);
            }
            this->registeredEquivalentTypeCacheRef = nullptr;
        }
    }

    bool EntryPointInfo::ClearEquivalentTypeCaches()
    {
        Assert(this->equivalentTypeCaches != nullptr);
        Assert(this->equivalentTypeCacheCount > 0);

        bool isAnyCacheLive = false;
        Recycler *recycler = GetScriptContext()->GetRecycler();
        for (EquivalentTypeCache *cache = this->equivalentTypeCaches;
             cache < this->equivalentTypeCaches + this->equivalentTypeCacheCount;
             cache++)
        {
            bool isCacheLive = cache->ClearUnusedTypes(recycler);
            if (isCacheLive)
            {
                isAnyCacheLive = true;
            }
        }

        if (!isAnyCacheLive)
        {
            // The caller must take care of unregistering this entry point. We may be in the middle of
            // walking the list of registered entry points.
            this->equivalentTypeCaches = nullptr;
            this->equivalentTypeCacheCount = 0;
            this->registeredEquivalentTypeCacheRef = nullptr;
        }

        return isAnyCacheLive;
    }

    bool EquivalentTypeCache::ClearUnusedTypes(Recycler *recycler)
    {
        bool isAnyTypeLive = false;

        Assert(this->guard);
        if (this->guard->IsValid())
        {
            Type *type = reinterpret_cast<Type*>(this->guard->GetValue());
            if (!recycler->IsObjectMarked(type))
            {
                this->guard->Invalidate();
            }
            else
            {
                isAnyTypeLive = true;
            }
        }

        // Question: if the guard is cleared above, is the cache effectively dead?

        for (int i = 0; i < EQUIVALENT_TYPE_CACHE_SIZE; i++)
        {
            Type *type = this->types[i];
            if (type != nullptr)
            {
                if (!recycler->IsObjectMarked(type))
                {
                    this->types[i] = nullptr;
                }
                else
                {
                    isAnyTypeLive = true;
                }
            }
        }

        return isAnyTypeLive;
    }

    void EntryPointInfo::RegisterConstructorCache(Js::ConstructorCache* constructorCache, Recycler* recycler)
    {
        Assert(constructorCache != null);

        if (!this->constructorCaches)
        {
            this->constructorCaches = RecyclerNew(recycler, ConstructorCacheList, recycler);
        }

        this->constructorCaches->Prepend(constructorCache);
    }

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    void EntryPointInfo::CaptureCleanupStackTrace()
    {
        // REVIEW: In the debugger case, we might call cleanup twice?
        if (this->cleanupStack != nullptr)
        {
            this->cleanupStack->Delete(&NoCheckHeapAllocator::Instance);
            this->cleanupStack = nullptr;
        }

        // REVIEW: NoCheckHeapAllocator doesn't throw, it kills the process if we fail to alloc
        // Is this ok? Too noisy for stress?chakra
        this->cleanupStack = StackBackTrace::Capture(&NoCheckHeapAllocator::Instance);
    }
#endif

    void EntryPointInfo::Finalize(bool isShutdown)
    {
        __super::Finalize(isShutdown);

        if (!isShutdown)
        {
            ReleasePendingWorkItem();
        }

#if ENABLE_DEBUG_CONFIG_OPTIONS
        this->SetCleanupReason(CleanupReason::CleanUpForFinalize);
#endif

        this->Cleanup(isShutdown, false);

#if DBG
        if (this->cleanupStack != nullptr)
        {
            this->cleanupStack->Delete(&NoCheckHeapAllocator::Instance);
            this->cleanupStack = nullptr;
        }
#endif
        
        this->library = null;
    }

    EntryPointPolymorphicInlineCacheInfo * EntryPointInfo::EnsurePolymorphicInlineCacheInfo(Recycler * recycler, FunctionBody * functionBody)
    {
        if (!polymorphicInlineCacheInfo)
        {
            polymorphicInlineCacheInfo = RecyclerNew(recycler, EntryPointPolymorphicInlineCacheInfo, functionBody);
        }
        return polymorphicInlineCacheInfo;
    }

    void EntryPointInfo::Cleanup(bool isShutdown, bool captureCleanupStack)
    {
        if (this->GetState() != CleanedUp)
        {
            this->OnCleanup(isShutdown);

            FreeJitTransferData();

            if (this->bailoutRecordMap != nullptr)
            {
                HeapDelete(this->bailoutRecordMap);
                bailoutRecordMap = nullptr;
            }

            if (this->sharedPropertyGuards != null)
            {
                sharedPropertyGuards->Clear();
                sharedPropertyGuards = null;
            }

            FreePropertyGuards();

            if (this->equivalentTypeCaches != null)
            {
                this->UnregisterEquivalentTypeCaches();
                this->equivalentTypeCacheCount = 0;
                this->equivalentTypeCaches = null;
            }

            if (this->constructorCaches != null)
            {
                this->constructorCaches->Clear();
            }

            // This is how we set the CleanedUp state
            this->workItem = NULL;
            this->nativeAddress = NULL;
            this->weakFuncRefSet = NULL;
            this->runtimeTypeRefs = NULL;
            this->codeSize = -1;
            this->library = NULL;

            DeleteNativeCodeData(this->data);
            this->data = null;
            this->numberChunks = null;
            this->state = CleanedUp;
#if ENABLE_DEBUG_CONFIG_OPTIONS
#if !DBG
            captureCleanupStack = captureCleanupStack && Js::Configuration::Global.flags.FreTestDiagMode;
#endif

            if (captureCleanupStack)
            {
                this->CaptureCleanupStackTrace();
            }
#endif

            // Needs to be in cleanup rather than finalize because Cleanup might get called before the finalizer
            if (NULL != this->nativeThrowSpanSequence)
            {
                HeapDelete(this->nativeThrowSpanSequence);
                this->nativeThrowSpanSequence = null;
            }

            this->polymorphicInlineCacheInfo = null;

#if DBG_DUMP | defined(VTUNE_PROFILING) 
            this->nativeOffsetMaps.Reset();            
#endif
        }
    }

    void EntryPointInfo::Reset(bool resetStateToNotScheduled)
    {
        Assert(this->GetState() != CleanedUp);
        this->nativeAddress = NULL;
        this->workItem = NULL;
        if (NULL != this->nativeThrowSpanSequence)
        {
            HeapDelete(this->nativeThrowSpanSequence);
            this->nativeThrowSpanSequence = null;
        }
        this->codeSize = 0;
        this->weakFuncRefSet = NULL;
        this->sharedPropertyGuards = null;
        FreePropertyGuards();
        FreeJitTransferData();
        if (this->data != nullptr)
        {
            DeleteNativeCodeData(this->data);
            this->data = nullptr;
        }
        //Set the state to NotScheduled only if the call to Reset is not because of JIT cap being reached
        if (resetStateToNotScheduled)
        {
            this->state = NotScheduled;
        }
    }

    void EntryPointInfo::ResetOnNativeCodeInstallFailure()
    {
        // Reset the entry point without attempting to create a new default and GenerateFunction on it.
        // Do this for LoopEntryPointInfo or if we throw during FunctionEntryPointInfo::Invalidate.
        this->Reset(true);
        Assert(this->address != nullptr);
        FreeNativeCodeGenAllocation(GetScriptContext(), this->address);
        this->address = nullptr;
    }

#ifdef PERF_COUNTERS
    void FunctionEntryPointInfo::OnRecorded()
    {
        PERF_COUNTER_ADD(Code, TotalNativeCodeSize, GetCodeSize());
        PERF_COUNTER_ADD(Code, FunctionNativeCodeSize, GetCodeSize());
        PERF_COUNTER_ADD(Code, DynamicNativeCodeSize, GetCodeSize());
    }
#endif

    FunctionEntryPointInfo::FunctionEntryPointInfo(FunctionProxy * functionProxy, void * address, ThreadContext* context, void* cookie) :
        EntryPointInfo(address, functionProxy->GetScriptContext()->GetLibrary(), cookie, context),
        frameHeight(0),
        localVarSlotsOffset(Js::Constants::InvalidOffset),
        localVarChangedOffset(Js::Constants::InvalidOffset),
        callsCount(0),
        jitMode(ExecutionMode::Interpreter),
        nativeEntryPointProcessed(false),
        functionProxy(functionProxy),
        nextEntryPoint(null),
        mIsTemplatizedJitMode(false),
        inlineeFrameMap(nullptr)
    { 
    }

    void FunctionEntryPointInfo::SetOldFunctionEntryPointInfo(FunctionEntryPointInfo* entrypointInfo)
    {
        Assert(this->GetIsAsmJSFunction());
        Assert(entrypointInfo);
        mOldFunctionEntryPointInfo = entrypointInfo;
    };

    FunctionEntryPointInfo* FunctionEntryPointInfo::GetOldFunctionEntryPointInfo()const
    {
        Assert(this->GetIsAsmJSFunction());
        return mOldFunctionEntryPointInfo;
    };
    void FunctionEntryPointInfo::SetIsTJMode(bool value)
    {
        Assert(this->GetIsAsmJSFunction());
        mIsTemplatizedJitMode = value;
    }

    bool FunctionEntryPointInfo::GetIsTJMode()const
    {
        return mIsTemplatizedJitMode;
    };
    //End AsmJS Support

    ExecutionMode FunctionEntryPointInfo::GetJitMode() const
    {
        return jitMode;
    }

    void FunctionEntryPointInfo::SetJitMode(const ExecutionMode jitMode)
    {
        Assert(jitMode == ExecutionMode::SimpleJit || jitMode == ExecutionMode::FullJit);

        this->jitMode = jitMode;
    }
    void FunctionEntryPointInfo::ReleasePendingWorkItem()
    {
        // Do this outside of Cleanup since cleanup can be called from the background thread
        // We remove any workitems corresponding to the function body being reclaimed
        // so that the background thread doesn't try to use them. ScriptContext != null => this
        // is a function entry point
        // In general this is not needed for loop bodies since loop bodies aren't in the low pri
        // queue, they should be jitted before the entry point is finalized
        if (!this->IsNotScheduled() && !this->IsCleanedUp())
        {
#if defined(_M_ARM32_OR_ARM64)
            // On arm machines, order of writes is not guaranteed while reading data from another processor
            // So we need to have a memory barrier here in order to make sure that the work item is consistent
            MemoryBarrier();
#endif
            InMemoryCodeGenWorkItem* workItem = this->GetWorkItem();
            if (workItem != null)
            {
                // Probably don't need to do anything else here
                // We'll set the work item anyway in Cleanup
                Assert(this->library != null);
                TryReleaseNonHiPriWorkItem(this->library->GetScriptContext(), workItem);
            }

        }
    }

    FunctionBody *FunctionEntryPointInfo::GetFunctionBody() const
    {
        return functionProxy->GetFunctionBody();
    }

    void FunctionEntryPointInfo::OnCleanup(bool isShutdown)
    {
        if (this->IsCodeGenDone())
        {
            Assert(this->functionProxy->HasBody());
            if (nullptr != this->inlineeFrameMap)
            {
                HeapDelete(this->inlineeFrameMap);
                this->inlineeFrameMap = nullptr;
            }

            if(nativeEntryPointProcessed)
            {
                EtwTrace::LogMethodNativeUnloadEvent(this->functionProxy->GetFunctionBody(), this);
            }

            FunctionBody* functionBody = this->functionProxy->GetFunctionBody();
            if (this->GetIsTJMode())
            {
                // release LoopHeaders here if the entrypointInfo is TJ 
                this->GetFunctionBody()->ReleaseLoopHeaders();
            }
            if(functionBody->GetSimpleJitEntryPointInfo() == this)
            {
                functionBody->SetSimpleJitEntryPointInfo(nullptr);
            }
            // If we're shutting down, the script context might be gone
            if (!isShutdown)
            {
                ScriptContext* scriptContext = this->functionProxy->GetScriptContext();

                // In the debugger case, we might call cleanup after the native code gen that
                // allocated this entry point has already shutdown. In that case, the validation
                // check below should fail and we should not try to free this entry point
                // since it's already been freed
                NativeCodeGenerator* current = scriptContext->GetNativeCodeGenerator();
                Assert(this->validationCookie != null);
                if (validationCookie == (void*) current)
                {
                    scriptContext->FreeFunctionEntryPoint((Js::JavascriptMethod)this->GetNativeAddress());
                }
            }

#ifdef PERF_COUNTERS
            PERF_COUNTER_SUB(Code, TotalNativeCodeSize, GetCodeSize());
            PERF_COUNTER_SUB(Code, FunctionNativeCodeSize, GetCodeSize());
            PERF_COUNTER_SUB(Code, DynamicNativeCodeSize, GetCodeSize());
#endif
        }

        this->functionProxy = null;
    }

    void FunctionEntryPointInfo::OnNativeCodeInstallFailure()
    {
        this->Invalidate(false);
#if ENABLE_DEBUG_CONFIG_OPTIONS
        this->SetCleanupReason(CleanupReason::NativeCodeInstallFailure);
#endif
        this->Cleanup(false, true /* capture cleanup stack */);
    }

    void FunctionEntryPointInfo::EnterExpirableCollectMode()
    {
        this->lastCallsCount = this->callsCount;
        //For code that is not jited yet we dont want to expire since there is nothing to free here
        if (this->IsCodeGenPending())
        {
            this->SetIsObjectUsed();
        }

    }


    void FunctionEntryPointInfo::Invalidate(bool prolongEntryPoint)
    {
        Assert(!this->functionProxy->IsDeferred());
        FunctionBody* functionBody = this->functionProxy->GetFunctionBody();
        Assert(this != functionBody->GetSimpleJitEntryPointInfo());

        // We may have got here following OOM in ProcessJitTransferData. Free any data we have
        // to reduce the chance of another OOM below.
        this->FreeJitTransferData();
        FunctionEntryPointInfo* entryPoint = functionBody->GetDefaultFunctionEntryPointInfo();
        if (entryPoint->IsCodeGenPending())
        {
            OUTPUT_TRACE(Js::LazyBailoutPhase, L"Skipping creating new entrypoint as one is already pending\n");
        }
        else
        {
            class AutoCleanup
            {
                EntryPointInfo *entryPointInfo;
            public:
                AutoCleanup(EntryPointInfo *entryPointInfo) : entryPointInfo(entryPointInfo)
                {
                }

                void Done()
                {
                    entryPointInfo = null;
                }
                ~AutoCleanup()
                {
                    if (entryPointInfo)
                    {
                        entryPointInfo->ResetOnNativeCodeInstallFailure();
                    }
                }
            } autoCleanup(this);

            entryPoint = functionBody->CreateNewDefaultEntryPoint();

            GenerateFunction(functionBody->GetScriptContext()->GetNativeCodeGenerator(), functionBody, /*function*/ nullptr);
            autoCleanup.Done();

        }
        this->functionProxy->MapFunctionObjectTypes([&](DynamicType* type)
        {
            Assert(type->GetTypeId() == TypeIds_Function);

            ScriptFunctionType* functionType = (ScriptFunctionType*)type;
            if (functionType->GetEntryPointInfo() == this)
            {
                functionType->SetEntryPointInfo(entryPoint);
                functionType->SetEntryPoint(this->functionProxy->GetDirectEntryPoint(entryPoint));
            }
        });
        if (!prolongEntryPoint)
        {
            ThreadContext* threadContext = this->functionProxy->GetScriptContext()->GetThreadContext();
            threadContext->QueueFreeOldEntryPointInfoIfInScript(this);
        }
    }

    void FunctionEntryPointInfo::Expire()
    {
        if (this->lastCallsCount != this->callsCount || !this->nativeEntryPointProcessed || this->IsCleanedUp())
        {
            return;
        }

        ThreadContext* threadContext = this->functionProxy->GetScriptContext()->GetThreadContext();

        Assert(!this->functionProxy->IsDeferred());
        FunctionBody* functionBody = this->functionProxy->GetFunctionBody();

        FunctionEntryPointInfo *simpleJitEntryPointInfo = functionBody->GetSimpleJitEntryPointInfo();
        const bool expiringSimpleJitEntryPointInfo = simpleJitEntryPointInfo == this;
        if(expiringSimpleJitEntryPointInfo)
        {
            if(functionBody->GetExecutionMode() != ExecutionMode::FullJit)
            {
                // Don't expire simple JIT code until the transition to full JIT
                return;
            }
            simpleJitEntryPointInfo = null;
            functionBody->SetSimpleJitEntryPointInfo(null);
        }

        try
        {
            AUTO_NESTED_HANDLED_EXCEPTION_TYPE(ExceptionType_OutOfMemory);

            FunctionEntryPointInfo* newEntryPoint = nullptr;
            FunctionEntryPointInfo *const defaultEntryPointInfo = functionBody->GetDefaultFunctionEntryPointInfo();
            if(this == defaultEntryPointInfo)
            {
                if(simpleJitEntryPointInfo)
                {
                    newEntryPoint = simpleJitEntryPointInfo;
                    functionBody->SetDefaultFunctionEntryPointInfo(
                        simpleJitEntryPointInfo,
                        reinterpret_cast<JavascriptMethod>(newEntryPoint->GetNativeAddress()));
                    functionBody->SetExecutionMode(ExecutionMode::SimpleJit);
                    functionBody->ResetSimpleJitLimitAndCallCount();
                }
#ifdef ASMJS_PLAT
                else if (functionBody->GetIsAsmJsFunction())
                {
                    // the new entrypoint will be set to interpreter
                    // REVIEW: Should we check here to not expire TJ code ?
                    newEntryPoint = functionBody->CreateNewDefaultEntryPoint();
                    newEntryPoint->SetIsAsmJSFunction(true);
                    newEntryPoint->address = AsmJsDefaultEntryThunk;
                    newEntryPoint->SetModuleAddress(GetModuleAddress());
                    functionBody->SetIsAsmJsFullJitScheduled(false);
                    functionBody->SetExecutionMode(functionBody->GetDefaultInterpreterExecutionMode());
                    this->functionProxy->SetOriginalEntryPoint(AsmJsDefaultEntryThunk);
                }
#endif
                else
                {
                    newEntryPoint = functionBody->CreateNewDefaultEntryPoint();
                    functionBody->SetExecutionMode(functionBody->GetDefaultInterpreterExecutionMode());
                }
                functionBody->TraceExecutionMode("JitCodeExpired");
            }
            else
            {
                newEntryPoint = defaultEntryPointInfo;
            }

            OUTPUT_TRACE(Js::ExpirableCollectPhase,  L"Expiring 0x%p\n", this);
            this->functionProxy->MapFunctionObjectTypes([&] (DynamicType* type)
            {
                Assert(type->GetTypeId() == TypeIds_Function);

                ScriptFunctionType* functionType = (ScriptFunctionType*) type;
                if (functionType->GetEntryPointInfo() == this)
                {
                    OUTPUT_TRACE(Js::ExpirableCollectPhase, L"Type 0x%p uses this entry point- switching to default entry point\n", this);
                    functionType->SetEntryPointInfo(newEntryPoint);
                    //we are allowed to replace the entry point on the type only if it's directly using the jitted code or a type is referencing this entry point but hasn't been called since the codegen thunk was installed on it
                    if (functionType->GetEntryPoint() == functionProxy->GetDirectEntryPoint(this) || IsIntermediateCodeGenThunk(functionType->GetEntryPoint()))
                    {
                        functionType->SetEntryPoint(this->functionProxy->GetDirectEntryPoint(newEntryPoint));
                    }
                }
            });

            if(expiringSimpleJitEntryPointInfo)
            {
                // We could have just created a new entry point info that is using the simple JIT code. An allocation may have
                // triggered shortly after, resulting in expiring the simple JIT entry point info. Update any entry point infos
                // that are using the simple JIT code, and update the original entry point as necessary as well.
                const JavascriptMethod newOriginalEntryPoint =
                    functionBody->GetDynamicInterpreterEntryPoint()
                        ?   static_cast<JavascriptMethod>(
                                InterpreterThunkEmitter::ConvertToEntryPoint(functionBody->GetDynamicInterpreterEntryPoint()))
                        :   DefaultEntryThunk;
                const JavascriptMethod currentThunk = functionBody->GetScriptContext()->CurrentThunk;
                const JavascriptMethod newDirectEntryPoint =
                    currentThunk == DefaultEntryThunk ? newOriginalEntryPoint : currentThunk;
                const JavascriptMethod simpleJitNativeAddress = reinterpret_cast<JavascriptMethod>(GetNativeAddress());
                functionBody->MapEntryPoints([&](const int entryPointIndex, FunctionEntryPointInfo *const entryPointInfo)
                {
                    if(entryPointInfo != this && entryPointInfo->address == simpleJitNativeAddress)
                    {
                        entryPointInfo->address = newDirectEntryPoint;
                    }
                });
                if(functionBody->GetOriginalEntryPoint_Unchecked() == simpleJitNativeAddress)
                {
                    functionBody->SetOriginalEntryPoint(newOriginalEntryPoint);
                    functionBody->VerifyOriginalEntryPoint();
                }
            }

            threadContext->QueueFreeOldEntryPointInfoIfInScript(this);
        }
        catch (Js::OutOfMemoryException)
        {
            // If we can't allocate a new entry point, skip expiring this object
            if(expiringSimpleJitEntryPointInfo)
            {
                simpleJitEntryPointInfo = this;
                functionBody->SetSimpleJitEntryPointInfo(this);
            }
        }
    }

#ifdef PERF_COUNTERS
    void LoopEntryPointInfo::OnRecorded()
    {
        PERF_COUNTER_ADD(Code, TotalNativeCodeSize, GetCodeSize());
        PERF_COUNTER_ADD(Code, LoopNativeCodeSize, GetCodeSize());
        PERF_COUNTER_ADD(Code, DynamicNativeCodeSize, GetCodeSize());
    }
#endif

    FunctionBody *LoopEntryPointInfo::GetFunctionBody() const
    {
        return loopHeader->functionBody;
    }

    //End AsmJs Support 

    void LoopEntryPointInfo::OnCleanup(bool isShutdown)
    {
        if (this->IsCodeGenDone() && !this->GetIsTJMode())
        {
            EtwTrace::LogLoopBodyUnloadEvent(this->loopHeader->functionBody, this->loopHeader, this);

            if (!isShutdown)
            {
                // In the debugger case, we might call cleanup after the native code gen that
                // allocated this entry point has already shutdown. In that case, the validation
                // check below should fail and we should not try to free this entry point
                // since it's already been freed
                ScriptContext* scriptContext = this->loopHeader->functionBody->GetScriptContext();
                NativeCodeGenerator* current = scriptContext->GetNativeCodeGenerator();
                Assert(this->validationCookie != null);
                if (validationCookie == (void*) current)
                {
                    scriptContext->FreeLoopBody((Js::JavascriptMethod)this->GetNativeAddress());
                }
            }

#ifdef PERF_COUNTERS
            PERF_COUNTER_SUB(Code, TotalNativeCodeSize, GetCodeSize());
            PERF_COUNTER_SUB(Code, LoopNativeCodeSize, GetCodeSize());
            PERF_COUNTER_SUB(Code, DynamicNativeCodeSize, GetCodeSize());
#endif
        }
    }

    void LoopEntryPointInfo::OnNativeCodeInstallFailure()
    {
        this->ResetOnNativeCodeInstallFailure();
    }

    void LoopHeader::Init( FunctionBody * functionBody )
    {
        this->functionBody = functionBody;
        Recycler* recycler = functionBody->GetScriptContext()->GetRecycler();

        // Sync entryPoints changes to etw rundown lock
        auto syncObj = functionBody->GetScriptContext()->GetThreadContext()->GetEtwRundownCriticalSection();
        this->entryPoints = RecyclerNew(recycler, LoopEntryPointList, recycler, syncObj);

        this->CreateEntryPoint();
    }

    int LoopHeader::CreateEntryPoint()
    {
        ScriptContext* scriptContext = this->functionBody->GetScriptContext();
        Recycler* recycler = scriptContext->GetRecycler();
        LoopEntryPointInfo* entryPoint = RecyclerNew(recycler, LoopEntryPointInfo, this, scriptContext->GetLibrary(), scriptContext->GetNativeCodeGenerator());
        return this->entryPoints->Add(entryPoint);
    }

    void LoopHeader::ReleaseEntryPoints()
    {
        for (int iEntryPoint = 0; iEntryPoint < this->entryPoints->Count(); iEntryPoint++)
        {
            LoopEntryPointInfo * entryPoint = this->entryPoints->Item(iEntryPoint);

            if (entryPoint != NULL && entryPoint->IsCodeGenDone())
            {
                // ReleaseEntryPoints is not called during recycler shutdown scenarios
                // We also don't capture the cleanup stack since we've not seen cleanup bugs affect
                // loop entry points so far. We can pass true here if this is no longer the case.
                entryPoint->Cleanup(false /* isShutdown */, false /* capture cleanup stack */);
                this->entryPoints->Item(iEntryPoint, NULL);
            }
        }
    }

#if ENABLE_DEBUG_CONFIG_OPTIONS
    void FunctionBody::DumpRegStats(FunctionBody *funcBody)
    {
        if (funcBody->callCountStats == 0)
        {
            return;
        }
        uint loads = funcBody->regAllocLoadCount;
        uint stores = funcBody->regAllocStoreCount;

        if (Js::Configuration::Global.flags.NormalizeStats)
        {
            loads /= this->callCountStats;
            stores /= this->callCountStats;
        }
        funcBody->DumpFullFunctionName();
        Output::SkipToColumn(55);
        Output::Print(L"Calls:%6d  Loads:%9d  Stores:%9d  Total refs:%9d\n", this->callCountStats,
            loads, stores, loads + stores);
    }
#endif

    Js::RegSlot FunctionBody::GetRestParamRegSlot()
    {
        Js::RegSlot dstRegSlot = GetConstantCount();
        if (GetHasImplicitArgIns())
        {
            dstRegSlot += GetInParamsCount() - 1;
        }
        return dstRegSlot;
    }
    uint FunctionBody::GetNumberOfRecursiveCallSites()
    {
        uint recursiveInlineSpan = 0;
        uint recursiveCallSiteInlineInfo = 0;
        if (this->HasDynamicProfileInfo())
        {
            recursiveCallSiteInlineInfo = this->dynamicProfileInfo->GetRecursiveInlineInfo();
        }

        while (recursiveCallSiteInlineInfo)
        {
            recursiveInlineSpan += (recursiveCallSiteInlineInfo & 1);
            recursiveCallSiteInlineInfo >>= 1;
        }
        return recursiveInlineSpan;
    }

    bool FunctionBody::CanInlineRecursively(uint depth, bool tryAggressive)
    {
        uint recursiveInlineSpan = this->GetNumberOfRecursiveCallSites();
    
        uint minRecursiveInlineDepth = (uint)CONFIG_FLAG(RecursiveInlineDepthMin);

        if (recursiveInlineSpan != this->GetProfiledCallSiteCount() || tryAggressive == false)
        {
            return depth < minRecursiveInlineDepth;
        }

        uint maxRecursiveInlineDepth = (uint)CONFIG_FLAG(RecursiveInlineDepthMax);
        uint maxRecursiveBytecodeBudget = (uint)CONFIG_FLAG(RecursiveInlineThreshold);
        uint numberOfAllowedFuncs = maxRecursiveBytecodeBudget / this->m_byteCodeWithoutLDACount;
        uint maxDepth;
        
        if (recursiveInlineSpan == 1)
        {
            maxDepth = numberOfAllowedFuncs;
        }
        else 
        {
            maxDepth = (uint)ceil(log((double)((double)numberOfAllowedFuncs) / log((double)recursiveInlineSpan)));
        }
        maxDepth = maxDepth < minRecursiveInlineDepth ? minRecursiveInlineDepth : maxDepth;
        maxDepth = maxDepth < maxRecursiveInlineDepth ? maxDepth : maxRecursiveInlineDepth;
        return depth < maxDepth;
    }
}
