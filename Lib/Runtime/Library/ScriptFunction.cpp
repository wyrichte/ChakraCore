//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    const wchar_t ScriptFunction::diagDefaultCtor[]         = JS_DEFAULT_CTOR_DISPLAY_STRING;
    const wchar_t ScriptFunction::diagDefaultExtendsCtor[]  = JS_DEFAULT_EXTENDS_CTOR_DISPLAY_STRING;

    ScriptFunctionBase::ScriptFunctionBase(DynamicType * type) :
        JavascriptFunction(type)
    {}

    ScriptFunctionBase::ScriptFunctionBase(DynamicType * type, FunctionInfo * functionInfo) :
        JavascriptFunction(type, functionInfo)
    {}

    bool ScriptFunctionBase::Is(Var func)
    {
        return ScriptFunction::Is(func) || JavascriptGeneratorFunction::Is(func);
    }

    ScriptFunctionBase * ScriptFunctionBase::FromVar(Var func)
    {
        Assert(ScriptFunctionBase::Is(func));
        return reinterpret_cast<ScriptFunctionBase *>(func);
    }

    ScriptFunction::ScriptFunction(DynamicType * type) :
        ScriptFunctionBase(type), environment((FrameDisplay*)&NullFrameDisplay),
        cachedScopeObj(null), hasInlineCaches(false), hasSuperReference(false),
        isDefaultConstructor(false), isActiveScript(false)
    {}

    ScriptFunction::ScriptFunction(FunctionProxy * proxy, ScriptFunctionType* deferredPrototypeType)
        : ScriptFunctionBase(deferredPrototypeType, proxy),
        environment((FrameDisplay*)&NullFrameDisplay), cachedScopeObj(null),
        hasInlineCaches(false), hasSuperReference(false), isDefaultConstructor(false), isActiveScript(false)
    {
        Assert(proxy->GetFunctionProxy() == proxy);
        Assert(proxy->EnsureDeferredPrototypeType() == deferredPrototypeType)
        DebugOnly(VerifyEntryPoint());

#if ENABLE_NATIVE_CODEGEN
#ifdef BGJIT_STATS
        if (!proxy->IsDeferred())
        {
            FunctionBody* body = proxy->GetFunctionBody();
            if(!body->GetNativeEntryPointUsed() &&
                body->GetDefaultFunctionEntryPointInfo()->IsCodeGenDone())
            {
                MemoryBarrier();

                type->GetScriptContext()->jitCodeUsed += body->GetByteCodeCount();
                type->GetScriptContext()->funcJitCodeUsed++;

                body->SetNativeEntryPointUsed(true);
            }
        }
#endif
#endif
    }

    ScriptFunction * ScriptFunction::OP_NewScFunc(FrameDisplay *environment, FunctionProxy** proxyRef)
    {
        AssertMsg(proxyRef!= null, "BYTE-CODE VERIFY: Must specify a valid function to create");
        FunctionProxy* functionProxy = (*proxyRef);
        AssertMsg(functionProxy!= null, "BYTE-CODE VERIFY: Must specify a valid function to create");

        ScriptContext* scriptContext = functionProxy->GetScriptContext();

        bool hasSuperReference = (functionProxy->GetAttributes() & Js::FunctionInfo::Attributes::HasSuperReference) ? true : false;
        bool isDefaultConstructor = (functionProxy->GetAttributes() & Js::FunctionInfo::Attributes::IsDefaultConstructor) ? true : false;

        if (functionProxy->IsFunctionBody() && functionProxy->GetFunctionBody()->GetInlineCachesOnFunctionObject())
        {
            Js::FunctionBody * functionBody = functionProxy->GetFunctionBody();
            ScriptFunctionWithInlineCache* pfuncScriptWithInlineCache = scriptContext->GetLibrary()->CreateScriptFunctionWithInlineCache(functionProxy);
            pfuncScriptWithInlineCache->SetEnvironment(environment);
            JSETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_FUNCTION(pfuncScriptWithInlineCache, EtwTrace::GetFunctionId(functionProxy)));

            Assert(functionBody->GetInlineCacheCount() + functionBody->GetIsInstInlineCacheCount());

            if (functionBody->GetIsFirstFunctionObject())
            {
                // point the inline caches of the first function object to those on the function body.
                pfuncScriptWithInlineCache->SetInlineCachesFromFunctionBody();
                functionBody->SetIsNotFirstFunctionObject();
            }
            else
            {
                //allocate inline cache for this function object
                pfuncScriptWithInlineCache->CreateInlineCache();
            }

            pfuncScriptWithInlineCache->SetHasSuperReference(hasSuperReference);
            pfuncScriptWithInlineCache->SetIsDefaultConstructor(isDefaultConstructor);

            if (PHASE_TRACE1(Js::ScriptFunctionWithInlineCachePhase))
            {
                wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];
                
                Output::Print(L"Function object with inline cache: function number: (%s)\tfunction name: %s\n",
                    functionBody->GetDebugNumberSet(debugStringBuffer), functionBody->GetDisplayName());
                Output::Flush();
            }
            return pfuncScriptWithInlineCache;
        }

        else
        {
            ScriptFunction* pfuncScript = scriptContext->GetLibrary()->CreateScriptFunction(functionProxy);
            pfuncScript->SetEnvironment(environment);

            pfuncScript->SetHasSuperReference(hasSuperReference);
            pfuncScript->SetIsDefaultConstructor(isDefaultConstructor);

            JSETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_FUNCTION(pfuncScript, EtwTrace::GetFunctionId(functionProxy)));

            return pfuncScript;
        }
    }

    void ScriptFunction::SetEnvironment(FrameDisplay * environment)
    {
        //Assert(ThreadContext::IsOnStack(this) || !ThreadContext::IsOnStack(environment));
        this->environment = environment; 
    }

    void ScriptFunction::InvalidateCachedScopeChain()
    {
        // Note: Currently this helper assumes that we're in an eval-class case
        // where all the contents of the closure environment are dynamic objects.
        // Invalidating scopes that are raw slot arrays, etc., will have to be done
        // directly in byte code.

        // A function nested within this one has escaped.
        // Invalidate our own cached scope object, and walk the closure environment
        // doing this same.
        if (this->cachedScopeObj)
        {
            this->cachedScopeObj->InvalidateCachedScope();
        }
        FrameDisplay *pDisplay = this->environment;
        uint length = (uint)pDisplay->GetLength();
        for (uint i = 0; i < length; i++)
        {
            Var scope = pDisplay->GetItem(i);
            RecyclableObject *scopeObj = RecyclableObject::FromVar(scope);
            scopeObj->InvalidateCachedScope();
        }
    }

    bool ScriptFunction::Is(Var func)
    {
        return JavascriptFunction::Is(func) && JavascriptFunction::FromVar(func)->GetFunctionInfo()->HasBody();
    }

    ScriptFunction * ScriptFunction::FromVar(Var func)
    {
        Assert(ScriptFunction::Is(func));
        return reinterpret_cast<ScriptFunction *>(func);
    }

    ProxyEntryPointInfo * ScriptFunction::GetEntryPointInfo() const
    {
        return this->GetScriptFunctionType()->GetEntryPointInfo();
    }

    ScriptFunctionType * ScriptFunction::GetScriptFunctionType() const
    {
        return (ScriptFunctionType *)GetDynamicType();
    }

    ScriptFunctionType * ScriptFunction::DuplicateType()
    {
        ScriptFunctionType* type = RecyclerNew(this->GetScriptContext()->GetRecycler(),
            ScriptFunctionType, this->GetScriptFunctionType());

        this->GetFunctionProxy()->RegisterFunctionObjectType(type);

        return type;
    }

    void ScriptFunction::CopyEnvironmentTo(ScriptFunction *newFunction, ScriptContext *scriptContext)
    {
        VERIFY_COPY_ON_WRITE_ENABLED();

        if (environment)
        {
            uint16 length = environment->GetLength();
            if (length == 0)
            {
                // If the function has a null display environment just copy the reference.
                newFunction->environment = environment;
            }
            else
            {
                // Before we begin cloning the content we need to record the mapping from this object to the new
                // object to handle cycles.
                scriptContext->RecordCopyOnWrite(this, newFunction);

                // WARNING - this assumes that all scopes use an object (not a slot array) as the activation scope.
                // This is currently forced in the language service.
                // TODO: Adorn the function body with metadata describing the layout of the frame display allowing
                // this code to handle slot arrays as scopes.

                // The frame display is cloned immediately, instead of copy-on-write, because the the frame display is
                // indexed directly (via StSlot). There is no opportunity to trap the store so the copy needs to happen
                // now.
                FrameDisplay *clonedFrameDisplay = RecyclerNewPlus(scriptContext->GetRecycler(), length * sizeof(void*), FrameDisplay, length);
                for (size_t i = 0; i < length; i++)
                {
                    // If the above scope object assumption is violated a crash will most like appear below this call, probably in a call to GetTypeId().
                    clonedFrameDisplay->SetItem(i, scriptContext->CopyOnWrite(environment->GetItem(i)));
                }
                newFunction->environment = clonedFrameDisplay;
            }

        }
    }

    ScriptFunction *ScriptFunction::GetOriginalCopy()
    {
        return null;
    }
    typedef CopyOnWriteObject<ScriptFunction, JavascriptFunctionSpecialProperties> CopyOnWriteScriptFunctionBase;

    class CopyOnWriteScriptFunction : public CopyOnWriteScriptFunctionBase
    {
    private:
        ScriptFunction *original;
        bool isDelayFunctionInfo;

        // Notify that langauge service that deferred parsing is complete and the
        // deferred parsed function is now executing.
        static void NotifyAuthoringExecuting(JavascriptFunction* function)
        {
            if (BinaryFeatureControl::LanguageService())
            {
                auto scriptContext = function->GetScriptContext();
                if (scriptContext->authoringData && scriptContext->authoringData->Callbacks())
                    scriptContext->authoringData->Callbacks()->Executing();
            }
        }

    public:
        CopyOnWriteScriptFunction(DynamicType *type, ScriptFunction *original, ScriptContext *scriptContext) : CopyOnWriteScriptFunctionBase(type, original, scriptContext), original(original), isDelayFunctionInfo(false) { }

        static Var CopyOnWriteDeferredParsingThunk(RecyclableObject* function, CallInfo callInfo, ...);

        virtual ScriptFunction * GetOriginalCopy() override
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET();
            return original;
        }

        virtual bool GetIsDelayFunctionInfo() const override
        { 
            return isDelayFunctionInfo;
        }

        virtual void SetIsDelayFunctionInfo(bool set) override
        {
            isDelayFunctionInfo = set;
        }

        static Js::JavascriptMethod CopyOnWriteDeferredParse(ScriptFunction** functionRef)
        {
            VERIFY_COPY_ON_WRITE_ENABLED_RET();

            (*functionRef)->EnsureCopyFunction();

            return CopyOnWriteDeferredParseCore(functionRef);
        }

        static Js::JavascriptMethod CopyOnWriteDeferredParseCore(ScriptFunction** functionRef)
        {
            ScriptFunction * function = *functionRef;
            Assert(function->GetIsDelayFunctionInfo() == false);

            auto original = function->GetOriginalCopy();
            if (original)
            {
                // The function might already have byte code from a previous execution.
                Js::ParseableFunctionInfo* parseableFunctionInfo = function->GetFunctionInfo()->GetParseableFunctionInfo();

                Assert(parseableFunctionInfo);
                if (parseableFunctionInfo->IsDeferredParseFunction() || !parseableFunctionInfo->GetFunctionBody()->GetByteCode())
                {
                    // If this is not the original method, ensure the original is undeferred and then copy it.
                    Js::ParseableFunctionInfo* originalFunctionInfo = original->GetFunctionInfo()->GetParseableFunctionInfo();
                    Assert(originalFunctionInfo);

                    if (originalFunctionInfo->IsDeferredParseFunction() || !originalFunctionInfo->GetFunctionBody()->GetByteCode())
                    {
                        CopyOnWriteDeferredParseCore(&original);
                    }

                    Js::Utf8SourceInfo* utf8SourceInfo = parseableFunctionInfo->GetUtf8SourceInfo();
                    Js::FunctionBody* parsedFunctionBody = FunctionBody::NewFromRecycler(parseableFunctionInfo->GetScriptContext(),
                        parseableFunctionInfo->GetDisplayName(), parseableFunctionInfo->GetDisplayNameLength(), parseableFunctionInfo->GetNestedCount(), utf8SourceInfo,
                        parseableFunctionInfo->GetFunctionNumber(), utf8SourceInfo->GetSrcInfo()->sourceContextInfo->sourceContextId, parseableFunctionInfo->GetLocalFunctionId(),
                        parseableFunctionInfo->GetBoundPropertyRecords(),
                        (Js::FunctionInfo::Attributes)(parseableFunctionInfo->GetAttributes() & ~(Js::FunctionInfo::Attributes::DeferredDeserialize | Js::FunctionInfo::Attributes::DeferredParse))
#ifdef PERF_COUNTERS
                        , false /* is function from deferred deserialized proxy */
#endif
                        );
                    parseableFunctionInfo->UpdateFunctionBodyImpl(parsedFunctionBody);

                    Js::FunctionBody* originalBody = originalFunctionInfo->GetFunctionBody();
                    Assert(!originalBody->IsDeferredDeserializeFunction());
                    originalBody->CopyUndeferredInto(function->GetScriptContext(), parsedFunctionBody, parseableFunctionInfo->GetSourceIndex());
                    function->UpdateUndeferredBody(parsedFunctionBody);
                }
                else
                {
                    auto funcBody = parseableFunctionInfo->GetFunctionBody();
                    Assert(funcBody);
                    Assert(!funcBody->IsDeferredParseFunction());
                    function->UpdateUndeferredBody(funcBody);
                }

                FunctionBody * body = function->GetFunctionBody();
                auto entryPoint = body->GetDirectEntryPoint(body->GetDefaultEntryPointInfo());
                Assert(entryPoint != CopyOnWriteDeferredParsingThunk);
                NotifyAuthoringExecuting(function);
                return entryPoint;
            }
            else
            {
                // This is the original method, undefer it normally.
                auto entryPoint = Js::JavascriptFunction::DeferredParse(functionRef);
                return entryPoint;
            }
        }
    };

#if _M_IX86
    _declspec(naked)
        Var CopyOnWriteScriptFunction::CopyOnWriteDeferredParsingThunk(RecyclableObject* function, CallInfo callInfo, ...)
    {
        __asm
        {
            push ebp
                mov ebp, esp
                lea eax, [esp+8]
                push eax
                call CopyOnWriteScriptFunction::CopyOnWriteDeferredParse
                pop ebp
                jmp eax
        }
    }
#else
    Var CopyOnWriteScriptFunction::CopyOnWriteDeferredParsingThunk(RecyclableObject* function, CallInfo callInfo, ...)
    {
        Js::Throw::NotImplemented();
        return null;
    }
#endif

    ScriptFunction* ScriptFunction::MakeCopyOnWriteObject(ScriptContext* scriptContext)
    {
        VERIFY_COPY_ON_WRITE_ENABLED_RET();

        ParseableFunctionInfo* newFunctionInfo = this->functionInfo->GetParseableFunctionInfo();
        Assert(newFunctionInfo != null);
        ProxyEntryPointInfo* entryPointInfo = newFunctionInfo->GetDefaultEntryPointInfo();

        bool isDelayed = false;

        if (newFunctionInfo->GetScriptContext() != scriptContext)
        {
            // Delay copying the function info, if the current function is the deferred parse function.
            if (entryPointInfo->address != DefaultDeferredParsingThunk && newFunctionInfo->IsFunctionBody())
            {
                newFunctionInfo = scriptContext->CopyFunction(newFunctionInfo);
                entryPointInfo = newFunctionInfo->GetDefaultEntryPointInfo();
            }
            else
            {
                isDelayed = true;
            }
        }

        Assert(!newFunctionInfo->IsFunctionBody() ||
            (entryPointInfo->address != DefaultDeferredParsingThunk &&
            entryPointInfo->address != ProfileDeferredParsingThunk));
        Recycler * recycler = scriptContext->GetRecycler();
        JavascriptLibrary * library = scriptContext->GetLibrary();
        ScriptFunctionType * type = RecyclerNew(recycler, ScriptFunctionType,
            scriptContext, library->GetFunctionPrototype(),
            (JavascriptMethod)entryPointInfo->address, entryPointInfo,
            NullTypeHandler<false>::GetDefaultInstance(), false, false);
        CopyOnWriteScriptFunction *result = RecyclerNew(recycler,
            CopyOnWriteScriptFunction, type, this, scriptContext);

        CopyEnvironmentTo(result, scriptContext);

        result->SetIsDelayFunctionInfo(isDelayed);
        result->functionInfo = newFunctionInfo;
#if _M_IX86
        if (result->GetEntryPoint() == JavascriptFunction::DeferredParsingThunk)
            result->SetEntryPoint(CopyOnWriteScriptFunction::CopyOnWriteDeferredParsingThunk);
#endif
        return result;
    }

    void ScriptFunction::EnsureCopyFunction()
    {
        VERIFY_COPY_ON_WRITE_ENABLED();

        Js::ScriptFunction * originalFunction = this->GetOriginalCopy();
        if (originalFunction && this->GetIsDelayFunctionInfo() && this->GetScriptContext() != originalFunction->GetScriptContext())
        {
            // Recursively go to last one.
            originalFunction->EnsureCopyFunction();

            Assert(originalFunction->GetParseableFunctionInfo() != nullptr);
            this->SetFunctionInfo(this->GetScriptContext()->CopyFunction(originalFunction->GetParseableFunctionInfo()));
            this->SetIsDelayFunctionInfo(false);
        }
    }

    uint32 ScriptFunction::GetFrameHeight(FunctionEntryPointInfo* entryPointInfo) const
    {
        Assert(this->GetFunctionBody() != NULL);

        return this->GetFunctionBody()->GetFrameHeight(entryPointInfo);
    }

    bool ScriptFunction::HasFunctionBody()
    {
        //for asmjs we want to first check if the functionobject has a function body. Check that the function is not deferred
        return  !this->GetFunctionInfo()->IsDeferredParseFunction() && !this->GetFunctionInfo()->IsDeferredDeserializeFunction() && GetParseableFunctionInfo()->IsFunctionParsed();
    }

    void ScriptFunction::ChangeEntryPoint(ProxyEntryPointInfo* entryPointInfo, JavascriptMethod entryPoint)
    {
        Assert(entryPoint != NULL);
        Assert(this->GetTypeId() == TypeIds_Function);
        Assert(!IsCrossSiteObject() || entryPoint != (Js::JavascriptMethod)checkCodeGenThunk);

        Assert((entryPointInfo != null && this->GetFunctionProxy() != NULL));
        if (this->GetEntryPoint() == entryPoint && this->GetScriptFunctionType()->GetEntryPointInfo() == entryPointInfo)
        {
            return;
        }
        bool isAsmJS = false;
        if (HasFunctionBody())
        {
            isAsmJS = this->GetFunctionBody()->GetIsAsmjsMode();
        }
        //ASMJS:- for asmjs we dont need to update the entry point here as it updates the types entry point
        if (!isAsmJS)
        {
            // We can't go from cross-site to non-cross-site. Update only in the non-cross site case
            if (!CrossSite::IsThunk(this->GetEntryPoint()))
            {
                this->SetEntryPoint(entryPoint);
            }
        }
        //instead update the address in the function entrypoint info
        else
        {
            entryPointInfo->address = entryPoint;
        }
        if (!isAsmJS)
        {
            ProxyEntryPointInfo* oldEntryPointInfo = this->GetScriptFunctionType()->GetEntryPointInfo();
            if (oldEntryPointInfo
                && oldEntryPointInfo != entryPointInfo
                && oldEntryPointInfo->SupportsExpiration())
            {
                // The old entry point could be executing so we need root it to make sure
                // it isn't prematurely collected. The rooting is done by queueing it up on the threadContext
                ThreadContext* threadContext = ThreadContext::GetContextForCurrentThread();

                threadContext->QueueFreeOldEntryPointInfoIfInScript((FunctionEntryPointInfo*)oldEntryPointInfo);
            }
        }

        this->GetScriptFunctionType()->SetEntryPointInfo(entryPointInfo);
    }

    FunctionProxy * ScriptFunction::GetFunctionProxy() const
    {
        Assert(this->functionInfo->HasBody());
        return reinterpret_cast<FunctionProxy *>(this->functionInfo);
    }
    JavascriptMethod ScriptFunction::UpdateUndeferredBody(FunctionBody* newFunctionInfo)
    {
        // Update deferred parsed/serialized function to the real function body
        Assert(this->functionInfo->HasBody());
        if (this->functionInfo != newFunctionInfo)
        {
            Assert(this->functionInfo->GetFunctionBody() == newFunctionInfo);
            Assert(!newFunctionInfo->IsDeferred());
            DynamicType * type = this->GetDynamicType();

            // If the type is shared, it must be the shared one in the old function proxy

            DebugOnly(FunctionProxy * oldProxy = this->GetFunctionProxy());
            this->functionInfo = newFunctionInfo;

            if (type->GetIsShared())
            {
                // if it is shared, it must still be the deferred prototype from the old proxy
                Assert(type == oldProxy->GetDeferredPrototypeType());

                // the type is still shared, we can't modify it, just migrate to the shared one in the function body
                this->ReplaceType(newFunctionInfo->EnsureDeferredPrototypeType());
            }
        }

        // The type has change from the default, it is not share, just use that one.
        JavascriptMethod directEntryPoint = newFunctionInfo->GetDirectEntryPoint(newFunctionInfo->GetDefaultEntryPointInfo());
        Assert(directEntryPoint != DefaultDeferredParsingThunk && directEntryPoint != ProfileDeferredParsingThunk);

        Js::FunctionEntryPointInfo* defaultEntryPointInfo = newFunctionInfo->GetDefaultFunctionEntryPointInfo();
        JavascriptMethod thunkEntryPoint = this->UpdateThunkEntryPoint(defaultEntryPointInfo,
                directEntryPoint);

        this->GetScriptFunctionType()->SetEntryPointInfo(defaultEntryPointInfo);

        return thunkEntryPoint;

    }

    JavascriptMethod ScriptFunction::UpdateThunkEntryPoint(FunctionEntryPointInfo* entryPointInfo, JavascriptMethod entryPoint)
    {
        this->ChangeEntryPoint(entryPointInfo, entryPoint);

        if (!CrossSite::IsThunk(this->GetEntryPoint()))
        {
            return entryPoint;
        }

        // We already pass thru the cross site thunk, which would have called the profile thunk already if necessary
        // So just call the original entry point if our direct entry is the profile entry thunk
        // Otherwise, call the directEntryPoint which may have additional processing to do (e.g. ensure dynamic profile)
        Assert(this->IsCrossSiteObject());
        if (entryPoint != ProfileEntryThunk)
        {
            return entryPoint;
        }
        //Based on the comment below, this shouldn't be a a defer deserialization function as it would have a deferred thunk
        FunctionBody * functionBody = this->GetFunctionBody();
        // The original entry point should be an interpreter thunk or the native entry point;
        Assert(functionBody->IsInterpreterThunk() || functionBody->IsNativeOriginalEntryPoint());
        return functionBody->GetOriginalEntryPoint();
    }

    Var ScriptFunction::GetSourceString() const
    {
        return this->GetFunctionProxy()->EnsureDeserialized()->GetCachedSourceString();
    }

    Var ScriptFunction::FormatToString(JavascriptString* inputString)
    {
        FunctionProxy* proxy = this->GetFunctionProxy();
        ParseableFunctionInfo * pFuncBody = proxy->EnsureDeserialized();
        ScriptContext * scriptContext = this->GetScriptContext();
        JavascriptLibrary *javascriptLibrary = scriptContext->GetLibrary();
        bool isClassMethod = this->GetHomeObj() != nullptr;
        JavascriptString* prefixString = nullptr;
        uint prefixStringLength = 0;
        const wchar_t* name = L"";
        size_t nameLength = 0;
        Var returnStr = nullptr;
        ENTER_PINNED_SCOPE(JavascriptString, computedName);

        if (!isClassMethod)
        {
            prefixString = javascriptLibrary->GetFunctionPrefixString();
            if (pFuncBody->IsGenerator())
            {
                prefixString = javascriptLibrary->GetGeneratorFunctionPrefixString();
            }
            prefixStringLength = prefixString->GetLength();
            
            if (pFuncBody->GetIsAccessor())
            {
                name = pFuncBody->GetShortDisplayName(&nameLength);
                
            }
            else if (pFuncBody->GetIsDeclaration() || pFuncBody->GetIsNamedFunctionExpression())
            {
                name = pFuncBody->GetDisplayName();
                nameLength = pFuncBody->GetDisplayNameLength();
                if (wcscmp(name, Js::Constants::FunctionCode) == 0)
                {
                    name = Js::Constants::Anonymous;
                    nameLength = Js::Constants::AnonymousLength;
                }
               
            }
        }
        else
        {
            computedName = this->GetComputedName();
            if (computedName != nullptr)
            {
                name = computedName->GetString();
                nameLength = computedName->GetLength();
            }
            else if (IsClassConstructor())
            {
                name = L"constructor";
                nameLength = _countof(L"constructor") -1; //subtract off \0
            }
            else
            {
                name = pFuncBody->GetShortDisplayName(&nameLength); //strip off prototype.
            }
        }
        
        const wchar_t * inputStr = inputString->GetString();
        const wchar_t * paramStr = wcschr(inputStr, L'(');
        Assert(paramStr != nullptr);

        //Length is a uint32 so max length of functionBody can't be more than that even if we are using 64bit pointers
        uint functionBodyLength = inputString->GetLength() - ((uint)(paramStr - inputStr)); 
        
        uint totalLength = prefixStringLength + functionBodyLength + nameLength;
        wchar_t * funcBodyStr = RecyclerNewArrayLeaf(this->GetScriptContext()->GetRecycler(), wchar_t, totalLength);
        wchar_t * funcBodyStrStart = funcBodyStr;
        if (prefixString != nullptr)
        {
            js_wmemcpy_s(funcBodyStr, prefixStringLength, prefixString->GetString(), prefixStringLength);
            funcBodyStrStart += prefixStringLength;
        }
        
        js_wmemcpy_s(funcBodyStrStart, nameLength, name, nameLength);
        funcBodyStrStart = funcBodyStrStart + nameLength;
        js_wmemcpy_s(funcBodyStrStart, functionBodyLength, paramStr, functionBodyLength);
        
        returnStr = LiteralString::NewCopyBuffer(funcBodyStr, totalLength, scriptContext);

        LEAVE_PINNED_SCOPE();

        return returnStr;
    }

    Var ScriptFunction::EnsureSourceString()
    {
        // The function may be defer serialize, need to be deserialized
        FunctionProxy* proxy = this->GetFunctionProxy();
        ParseableFunctionInfo * pFuncBody = proxy->EnsureDeserialized();
        Var cachedSourceString = pFuncBody->GetCachedSourceString();
        if (cachedSourceString != null)
        {
            return cachedSourceString;
        }

        ScriptContext * scriptContext = this->GetScriptContext();

        if (isDefaultConstructor)
        {
            PCWSTR fakeCode = hasSuperReference ? diagDefaultExtendsCtor : diagDefaultCtor;
            size_t fakeStrLen = hasSuperReference ? _countof(diagDefaultExtendsCtor) : _countof(diagDefaultCtor);
            Var fakeString = JavascriptString::NewCopyBuffer(fakeCode, fakeStrLen - 1, scriptContext);

            pFuncBody->SetCachedSourceString(fakeString);
            return fakeString;
        }

        //Library code should behave the same way as RuntimeFunctions
        Utf8SourceInfo* source = pFuncBody->GetUtf8SourceInfo();
        if (source != null && source->GetIsLibraryCode())
        {
            //Don't display if it is annonymous function
            size_t displayNameLength = 0;
            PCWSTR displayName = pFuncBody->GetShortDisplayName(&displayNameLength);
            cachedSourceString = JavascriptFunction::GetLibraryCodeDisplayString(scriptContext, displayName);
        }
        else if (!pFuncBody->GetUtf8SourceInfo()->GetIsXDomain())
        {
            // Decode UTF8 into Unicode
            // Consider: Should we have a JavascriptUtf8Substring class which defers decoding
            // until it's needed?

            BufferStringBuilder builder(pFuncBody->LengthInChars(), scriptContext);
            // TODO: What about surrogate pairs?
            utf8::DecodeOptions options = pFuncBody->GetUtf8SourceInfo()->IsCesu8() ? utf8::doAllowThreeByteSurrogates : utf8::doDefault;
            utf8::DecodeInto(builder.DangerousGetWritableBuffer(), pFuncBody->GetSource(L"ScriptFunction::EnsureSourceString"), pFuncBody->LengthInChars(), options);
            if (pFuncBody->IsLambda() || isActiveScript || scriptContext->GetConfig()->IsWinRTEnabled())
            {
                cachedSourceString = builder.ToString();
            }
            else
            {
                cachedSourceString = FormatToString(builder.ToString());
            }
        }
        else
        {
            cachedSourceString = scriptContext->GetLibrary()->GetXDomainFunctionDisplayString();
        }
        Assert(cachedSourceString != null);
        pFuncBody->SetCachedSourceString(cachedSourceString);
        return cachedSourceString;
    }

    BOOL ScriptFunction::GetDiagValueString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        if (!isDefaultConstructor)
        {
            return JavascriptFunction::GetDiagValueString(stringBuilder, requestContext);
        }

        if (hasSuperReference)
            stringBuilder->AppendCppLiteral(diagDefaultExtendsCtor);
        else
            stringBuilder->AppendCppLiteral(diagDefaultCtor);

        return TRUE;
    }

    bool ScriptFunction::CloneMethod(JavascriptFunction** pnewMethod, const Var newHome)
    {
        ScriptContext* scriptContext = this->GetScriptContext();
        FunctionProxy* proxy = JavascriptOperators::GetDeferredDeserializedFunctionProxy(this);

        ScriptFunction* cloneScriptFunc = scriptContext->GetLibrary()->CreateScriptFunction(proxy);
        cloneScriptFunc->SetEnvironment(this->GetEnvironment());
        cloneScriptFunc->SetHasSuperReference(this->HasSuperReference());
        JSETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_FUNCTION(cloneScriptFunc, EtwTrace::GetFunctionId(proxy)));
        cloneScriptFunc->SetHomeObj(newHome);

        *pnewMethod = cloneScriptFunc;
        return true;
    }


    ScriptFunctionWithInlineCache::ScriptFunctionWithInlineCache(FunctionProxy * proxy, ScriptFunctionType* deferredPrototypeType) :
        ScriptFunction(proxy, deferredPrototypeType)
    {}

    ScriptFunctionWithInlineCache::ScriptFunctionWithInlineCache(DynamicType * type) :
        ScriptFunction(type)
    {}

    bool ScriptFunctionWithInlineCache::Is(Var func)
    {
        return ScriptFunction::Is(func) && ScriptFunction::FromVar(func)->GetHasInlineCaches();
    }

    ScriptFunctionWithInlineCache* ScriptFunctionWithInlineCache::FromVar(Var func)
    {
        Assert(ScriptFunctionWithInlineCache::Is(func));
        return reinterpret_cast<ScriptFunctionWithInlineCache *>(func);
    }

    InlineCache * ScriptFunctionWithInlineCache::GetInlineCache(uint index)
    {
        Assert(this->m_inlineCaches != null);
        Assert(index < this->GetInlineCacheCount());
#if DBG
        Assert(this->m_inlineCacheTypes[index] == InlineCacheTypeNone ||
            this->m_inlineCacheTypes[index] == InlineCacheTypeInlineCache);
        this->m_inlineCacheTypes[index] = InlineCacheTypeInlineCache;
#endif
        return reinterpret_cast<InlineCache *>(this->m_inlineCaches[index]);
    }

    void ScriptFunctionWithInlineCache::SetInlineCachesFromFunctionBody()
    {
        SetHasInlineCaches(true);
        Js::FunctionBody* functionBody = this->GetFunctionBody();
        this->m_inlineCaches = functionBody->GetInlineCaches();
#if DBG
        this->m_inlineCacheTypes = functionBody->GetInlineCacheTypes();
#endif
        this->rootObjectLoadInlineCacheStart = functionBody->GetRootObjectLoadInlineCacheStart();
        this->rootObjectLoadMethodInlineCacheStart = functionBody->GetRootObjectLoadMethodInlineCacheStart();
        this->rootObjectStoreInlineCacheStart = functionBody->GetRootObjectStoreInlineCacheStart();
        this->inlineCacheCount = functionBody->GetInlineCacheCount();
        this->isInstInlineCacheCount = functionBody->GetIsInstInlineCacheCount();
    }

    void ScriptFunctionWithInlineCache::CreateInlineCache()
    {
        Js::FunctionBody *functionBody = this->GetFunctionBody();
        this->rootObjectLoadInlineCacheStart = functionBody->GetRootObjectLoadInlineCacheStart();
        this->rootObjectStoreInlineCacheStart = functionBody->GetRootObjectStoreInlineCacheStart();
        this->inlineCacheCount = functionBody->GetInlineCacheCount();
        this->isInstInlineCacheCount = functionBody->GetIsInstInlineCacheCount();

        SetHasInlineCaches(true);
        AllocateInlineCache();
    }

    void ScriptFunctionWithInlineCache::AllocateInlineCache()
    {
        Assert(this->m_inlineCaches == null);
        uint isInstInlineCacheStart = this->GetInlineCacheCount();
        uint totalCacheCount = isInstInlineCacheStart + isInstInlineCacheCount;
        Js::FunctionBody* functionBody = this->GetFunctionBody();

        if (totalCacheCount != 0)
        {
            // Root object inline cache are not leaf
            Js::ScriptContext* scriptContext = this->GetFunctionBody()->GetScriptContext();
            void ** inlineCaches = RecyclerNewArrayZ(scriptContext->GetRecycler() ,
                void*, totalCacheCount);
#if DBG
            this->m_inlineCacheTypes = RecyclerNewArrayLeafZ(scriptContext->GetRecycler(),
                byte, totalCacheCount);
#endif
            uint i = 0;
            uint plainInlineCacheEnd = rootObjectLoadInlineCacheStart;
            __analysis_assume(plainInlineCacheEnd < totalCacheCount);
            for (;i < plainInlineCacheEnd; i++)
            {
                inlineCaches[i] = AllocatorNewZ(InlineCacheAllocator,
                    scriptContext->GetInlineCacheAllocator(), InlineCache);
            }
            Js::RootObjectBase * rootObject = functionBody->GetRootObject();
            ThreadContext * threadContext = scriptContext->GetThreadContext();
            uint rootObjectLoadInlineCacheEnd = rootObjectLoadMethodInlineCacheStart;
            __analysis_assume(rootObjectLoadInlineCacheEnd < totalCacheCount);
            for (; i < rootObjectLoadInlineCacheEnd; i++)
            {
                inlineCaches[i] = rootObject->GetInlineCache(
                    threadContext->GetPropertyName(functionBody->GetPropertyIdFromCacheId(i)), false, false);
            }
            uint rootObjectLoadMethodInlineCacheEnd = rootObjectStoreInlineCacheStart;
            __analysis_assume(rootObjectLoadMethodInlineCacheEnd < totalCacheCount);
            for (; i < rootObjectLoadMethodInlineCacheEnd; i++)
            {
                inlineCaches[i] = rootObject->GetInlineCache(
                    threadContext->GetPropertyName(functionBody->GetPropertyIdFromCacheId(i)), true, false);
            }
            uint rootObjectStoreInlineCacheEnd = isInstInlineCacheStart;
            for (; i < rootObjectStoreInlineCacheEnd; i++)
            {
                inlineCaches[i] = rootObject->GetInlineCache(
                    threadContext->GetPropertyName(functionBody->GetPropertyIdFromCacheId(i)), false, true);
            }
            for (;i < totalCacheCount; i++)
            {
                inlineCaches[i] = AllocatorNewStructZ(IsInstInlineCacheAllocator,
                    functionBody->GetScriptContext()->GetIsInstInlineCacheAllocator(), IsInstInlineCache);
            }
#if DBG
            this->m_inlineCacheTypes = RecyclerNewArrayLeafZ(functionBody->GetScriptContext()->GetRecycler(),
                byte, totalCacheCount);
#endif
            this->m_inlineCaches = inlineCaches;
        }
    }

    bool ScriptFunction::GetSymbolName(const wchar_t** symbolName, charcount_t* length) const
    {
        if (nullptr != this->computedNameVar && JavascriptSymbol::Is(this->computedNameVar))
        {
            const PropertyRecord* symbolRecord = JavascriptSymbol::FromVar(this->computedNameVar)->GetValue();
            *symbolName = symbolRecord->GetBuffer();
            *length = symbolRecord->GetLength();
            return true;
        }
        *symbolName = nullptr;
        *length = 0;
        return false;
    }

    JavascriptString* ScriptFunction::GetDisplayNameImpl() const
    {
        Assert(this->GetFunctionProxy() != nullptr); // The caller should guarantee a proxy exists
        ParseableFunctionInfo * func = this->GetFunctionProxy()->EnsureDeserialized();
        const wchar_t* name = nullptr;
        size_t length = 0;
        JavascriptString* returnStr = nullptr;
        ENTER_PINNED_SCOPE(JavascriptString, computedName);

        if (computedNameVar != nullptr)
        {
            const wchar_t* symbolName = nullptr;
            charcount_t symbolNameLength = 0;
            if (this->GetSymbolName(&symbolName, &symbolNameLength))
            {
                if (symbolNameLength == 0)
                {
                    name = symbolName;
                }
                else
                {
                    name = FunctionProxy::WrapWithBrackets(symbolName, symbolNameLength, this->GetScriptContext());
                    length = symbolNameLength + 2; //adding 2 to length for  brackets
                }
            }
            else
            {
                computedName = this->GetComputedName();
                if (!func->GetIsAccessor())
                {
                    return computedName;
                }
                name = computedName->GetString();
                length = computedName->GetLength();
            }
        }
        else
        { 
            name = Constants::Empty;
            if (func->GetIsNamedFunctionExpression()) // GetIsNamedFunctionExpression -> ex. var a = function foo() {} where name is foo
            {
                name = func->GetShortDisplayName(&length);
            }
            else if (func->GetIsNameIdentifierRef()) // GetIsNameIdentifierRef        -> confirms a name is not attached like o.x = function() {}
            {
                if (this->GetScriptContext()->GetConfig()->IsES6FunctionNameFullEnabled())
                {
                    name = func->GetShortDisplayName(&length);
                }
                else if (func->GetIsDeclaration() || // GetIsDeclaration -> ex. function foo () {}
                         func->GetIsAccessor()    || // GetIsAccessor    -> ex. var a = { get f() {}} new enough sytax that we do not have to disable by default
                         func->IsLambda()         || // IsLambda         -> ex. var y = { o : () => {}}
                         GetHomeObj())               // GetHomeObj       -> ex. var o = class {}, confirms this is a constructor or method on a class
                {
                    name = func->GetShortDisplayName(&length);
                }
            }
        }
        returnStr = DisplayNameHelper(name, length);

        LEAVE_PINNED_SCOPE();

        return returnStr;
    }

    JavascriptString* ScriptFunction::GetComputedName() const
    {
        JavascriptString* computedName = nullptr;
        ScriptContext* scriptContext = this->GetScriptContext();
        if (computedNameVar != nullptr)
        {
            if (TaggedInt::Is(computedNameVar))
            {
                computedName = TaggedInt::ToString(computedNameVar, scriptContext);
            }
            else
            {
                computedName = JavascriptConversion::ToString(computedNameVar, scriptContext);
            }
            return computedName;
        }
        return nullptr;
    }

    void ScriptFunctionWithInlineCache::ClearInlineCacheOnFunctionObject()
    {
        if (NULL != this->m_inlineCaches)
        {
            this->m_inlineCaches = NULL;
            this->inlineCacheCount = 0;
            this->rootObjectLoadInlineCacheStart = 0;
            this->rootObjectLoadMethodInlineCacheStart = 0;
            this->rootObjectStoreInlineCacheStart = 0;
            this->isInstInlineCacheCount = 0;
        }
        SetHasInlineCaches(false);
    }
}
