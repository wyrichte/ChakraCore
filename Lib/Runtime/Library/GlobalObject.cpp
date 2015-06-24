//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    GlobalObject * GlobalObject::New(ScriptContext * scriptContext)
    {
        SimpleDictionaryTypeHandler* globalTypeHandler = SimpleDictionaryTypeHandler::New(
            scriptContext->GetRecycler(), InitialCapacity, InlineSlotCapacity, sizeof(Js::GlobalObject));

        DynamicType* globalType = DynamicType::New(
            scriptContext, TypeIds_GlobalObject, null, null, globalTypeHandler);

        GlobalObject* globalObject = RecyclerNewPlus(scriptContext->GetRecycler(),
            sizeof(Var) * InlineSlotCapacity, GlobalObject, globalType, scriptContext);

        globalTypeHandler->SetSingletonInstanceIfNeeded(scriptContext->GetRecycler()->CreateWeakReferenceHandle<DynamicObject>(globalObject));

        return globalObject;
    }

    GlobalObject::GlobalObject(DynamicType * type, ScriptContext* scriptContext) :
        RootObjectBase(type, scriptContext),
        directHostObject(NULL),
        secureDirectHostObject(NULL),
        EvalHelper(&GlobalObject::DefaultEvalHelper),
        reservedProperties(null)
    {
    }

    void GlobalObject::Initialize(ScriptContext * scriptContext)
    {
        Assert(type->javascriptLibrary == NULL);
        JavascriptLibrary* localLibrary = RecyclerNewFinalized(scriptContext->GetRecycler(), JavascriptLibrary, this);
        scriptContext->SetLibrary(localLibrary);
        type->javascriptLibrary = localLibrary;
        localLibrary->Initialize(scriptContext, this);
        library = localLibrary;
    }

    HRESULT GlobalObject::SetDirectHostObject(RecyclableObject* hostObject, RecyclableObject* secureDirectHostObject)
    {
        HRESULT hr = S_OK;

        BEGIN_TRANSLATE_OOM_TO_HRESULT_NESTED
        {
            // In fastDOM scenario, we should use the host object to lookup the prototype.
            this->SetPrototype(library->GetNull());
        }
        END_TRANSLATE_OOM_TO_HRESULT(hr)

        this->directHostObject = hostObject;
        this->secureDirectHostObject = secureDirectHostObject;
        return hr;
    }

    RecyclableObject* GlobalObject::GetDirectHostObject()
    {
        return this->directHostObject;
    }

    RecyclableObject* GlobalObject::GetSecureDirectHostObject()
    {
        return this->secureDirectHostObject;
    }

    // Converts the global object into the object that should be used as the 'this' parameter.
    // In a non-hosted environment, the global object (this) is returned.
    // In a hosted environment, the host object is returned.
    Var GlobalObject::ToThis()
    {
        Var ret;

        // In fast DOM, we need to give user the secure version of host object
        if (secureDirectHostObject)
        {
            ret = secureDirectHostObject;
        }
        else if (hostObject)
        {
            // If the global object has the host object, use that as "this"
            ret = hostObject->GetHostDispatchVar();
            Assert(ret);
        }
        else
        {
            // Otherwise just use the global object
            ret = this;
        }

        return ret;
    }

    BOOL GlobalObject::ReserveGlobalProperty(PropertyId propertyId)
    {
        if (DynamicObject::HasProperty(propertyId))
        {
            return false;
        }
        if (reservedProperties == null)
        {
            Recycler* recycler = this->GetScriptContext()->GetRecycler();
            reservedProperties = RecyclerNew(recycler, ReservedPropertiesHashSet, recycler, 3);
        }
        reservedProperties->AddNew(propertyId);
        return true;
    }

    BOOL GlobalObject::IsReservedGlobalProperty(PropertyId propertyId)
    {
        return reservedProperties != null && reservedProperties->Contains(propertyId);
    }

#ifdef IR_VIEWER
    Var GlobalObject::EntryParseIR(RecyclableObject *function, CallInfo callInfo, ...)
    {
        //
        // retrieve arguments
        //

        RUNTIME_ARGUMENTS(args, callInfo);
        Js::Var codeVar = args[1];  // args[0] is (this)

        Js::JavascriptString *codeStringVar = NULL;
        const wchar_t *source = NULL;
        size_t sourceLength = 0;

        if (Js::JavascriptString::Is(codeVar))
        {
            codeStringVar = (Js::JavascriptString *)codeVar;
            source = codeStringVar->GetString();
            sourceLength = codeStringVar->GetLength();
        }
        else
        {
            AssertMsg(false, "The input to parseIR was not a string.");
        }

        //
        // collect arguments for eval
        //

        /* @see NativeCodeGenerator::CodeGen */
        ScriptContext *scriptContext = function->GetScriptContext();

        // used arbitrary defaults for these, but it seems to work
        ModuleID moduleID = 0;
        ulong grfscr = 0;
        LPCOLESTR pszTitle = L"";
        BOOL registerDocument = false;

        BOOL strictMode = true;

        return IRDumpEvalHelper(scriptContext, source, sourceLength, moduleID, grfscr,
            pszTitle, registerDocument, FALSE, strictMode);
    }

    // TODO remove when refactor
    Js::PropertyId GlobalObject::CreateProperty(Js::ScriptContext *scriptContext, const wchar_t *propertyName)
    {
        Js::PropertyRecord const *propertyRecord;
        scriptContext->GetOrAddPropertyRecord(propertyName, (int) wcslen(propertyName), &propertyRecord);
        Js::PropertyId propertyId = propertyRecord->GetPropertyId();

        return propertyId;
    }

    // TODO remove when refactor
    void GlobalObject::SetProperty(Js::DynamicObject *obj, const wchar_t *propertyName, Js::Var value)
    {
        const size_t len = wcslen(propertyName);
        if (!(len > 0))
        {
            return;
        }

        Js::PropertyId id = CreateProperty(obj->GetScriptContext(), propertyName);
        SetProperty(obj, id, value);
    }

    // TODO remove when refactor
    void GlobalObject::SetProperty(Js::DynamicObject *obj, Js::PropertyId id, Js::Var value)
    {
        if (value == NULL)
        {
            return;
        }

        Js::JavascriptOperators::SetProperty(obj, obj, id, value, obj->GetScriptContext());
    }

    Var GlobalObject::FunctionInfoObjectBuilder(ScriptContext *scriptContext, const wchar_t *file,
        const wchar_t *function, ULONG lineNum, ULONG colNum,
        uint functionId, Js::Utf8SourceInfo *utf8SrcInfo, Js::Var source)
    {
        Js::DynamicObject *fnInfoObj = scriptContext->GetLibrary()->CreateObject();

        // create javascript objects for properties
        Js::Var filenameString = Js::JavascriptString::NewCopyBuffer(file, wcslen(file), scriptContext);
        Js::Var funcnameString = Js::JavascriptString::NewCopyBuffer(function, wcslen(function), scriptContext);
        Js::Var lineNumber = Js::JavascriptNumber::ToVar((int64) lineNum, scriptContext);
        Js::Var colNumber = Js::JavascriptNumber::ToVar((int64) colNum, scriptContext);
        Js::Var functionIdNumberVar = Js::JavascriptNumber::ToVar(functionId, scriptContext);
        Js::Var utf8SourceInfoVar = Js::JavascriptNumber::ToVar((long) utf8SrcInfo, scriptContext);

        // assign properties to function info object
        SetProperty(fnInfoObj, L"filename", filenameString);
        SetProperty(fnInfoObj, L"function", funcnameString);
        SetProperty(fnInfoObj, L"line", lineNumber);
        SetProperty(fnInfoObj, L"col", colNumber);
        SetProperty(fnInfoObj, L"funcId", functionIdNumberVar);
        SetProperty(fnInfoObj, L"utf8SrcInfoPtr", utf8SourceInfoVar);
        SetProperty(fnInfoObj, L"source", source);

        return fnInfoObj;
    }

    /**
     * Return a list of functions visible in the current ScriptContext.
     */
    Var GlobalObject::EntryFunctionList(RecyclableObject *function, CallInfo callInfo, ...)
    {
        // Note: the typedefs below help make the following code more readable

        // See: Js::ScriptContext::SourceList (declaration not visible from this file)
        typedef JsUtil::List<RecyclerWeakReference<Utf8SourceInfo>*, Recycler, false, Js::FreeListedRemovePolicy> SourceList;
        typedef RecyclerWeakReference<Js::Utf8SourceInfo> Utf8SourceInfoRef;

        ScriptContext *originalScriptContext = function->GetScriptContext();
        ThreadContext *threadContext = originalScriptContext->GetThreadContext();
        ScriptContext *scriptContext;

        int fnCount = 0;
        for (scriptContext = threadContext->GetScriptContextList();
            scriptContext;
            scriptContext = scriptContext->next)
        {
            if (scriptContext->IsClosed()) continue;
            // if (scriptContext == originalScriptContext) continue;  // uncomment to ignore the originalScriptContext

            //
            // get count of functions in all files in script context
            //
            SourceList *sourceList = scriptContext->GetSourceList();
            sourceList->Map([&fnCount](uint i, Utf8SourceInfoRef *sourceInfoWeakRef)
            {
                Js::Utf8SourceInfo *sourceInfo = sourceInfoWeakRef->Get();
                if (sourceInfo == null || sourceInfo->GetIsLibraryCode()) // library code has no source, skip
                {
                    return;
                }

                fnCount += sourceInfo->GetFunctionBodyCount();
            });
        }

#ifdef ENABLE_IR_VIEWER_DBG_DUMP
        if (Js::Configuration::Global.flags.Verbose)
        {
            Output::Print(L">> There are %d total functions\n", fnCount);
            Output::Flush();
        }
#endif

        //
        // create a javascript array to hold info for all of the functions
        //
        Js::JavascriptArray *functionList = originalScriptContext->GetLibrary()->CreateArray(fnCount);

        int count = 0;
        for (scriptContext = threadContext->GetScriptContextList();
            scriptContext;
            scriptContext = scriptContext->next)
        {
            if (scriptContext->IsClosed()) continue;
            // if (scriptContext == originalScriptContext) continue;  // uncomment to ignore the originalScriptContext

            SourceList *sourceList = scriptContext->GetSourceList();
            sourceList->Map([&fnCount, &count, functionList, scriptContext](uint i, Utf8SourceInfoRef *sourceInfoWeakRef)
            {
                Js::Utf8SourceInfo *sourceInfo = sourceInfoWeakRef->Get();
                if (sourceInfo == null || sourceInfo->GetIsLibraryCode()) // library code has no source, skip
                {
                    return;
                }

                const SRCINFO *srcInfo = sourceInfo->GetSrcInfo();
                SourceContextInfo *srcContextInfo = srcInfo->sourceContextInfo;

                //
                // get URL of source file
                //
                wchar_t filenameBuffer[128];  // hold dynamically built filename
                wchar_t const *srcFileUrl = NULL;
                if (!srcContextInfo->IsDynamic())
                {
                    Assert(srcContextInfo->url != NULL);
                    srcFileUrl = srcContextInfo->url;
                }
                else
                {
                    StringCchPrintf(filenameBuffer, 128, L"[dynamic(hash:%d)]", srcContextInfo->hash);
                    srcFileUrl = filenameBuffer;
                }

                sourceInfo->MapFunction([scriptContext, &count, functionList, srcFileUrl](Js::FunctionBody *functionBody)
                {
                    wchar_t const *functionName = functionBody->GetExternalDisplayName();

                    ULONG lineNum = functionBody->GetLineNumber();
                    ULONG colNum = functionBody->GetColumnNumber();

                    uint funcId = functionBody->GetLocalFunctionId();
                    Js::Utf8SourceInfo *utf8SrcInfo = functionBody->GetUtf8SourceInfo();
                    if (utf8SrcInfo == null)
                    {
                        return;
                    }

                    Assert(utf8SrcInfo->FindFunction(funcId) == functionBody);

                    BufferStringBuilder builder(functionBody->LengthInChars(), scriptContext);
                    utf8::DecodeOptions options = utf8SrcInfo->IsCesu8() ? utf8::doAllowThreeByteSurrogates : utf8::doDefault;
                    utf8::DecodeInto(builder.DangerousGetWritableBuffer(), functionBody->GetSource(), functionBody->LengthInChars(), options);
                    Var cachedSourceString = builder.ToString();

                    Js::Var obj = FunctionInfoObjectBuilder(scriptContext, srcFileUrl,
                        functionName, lineNum, colNum, funcId, utf8SrcInfo, cachedSourceString);
                    functionList->SetItem(count, obj, Js::PropertyOperationFlags::PropertyOperation_None);

                    ++count;
                });
            });
        }

#ifdef ENABLE_IR_VIEWER_DBG_DUMP
        if (Js::Configuration::Global.flags.Verbose)
        {
            Output::Print(L">> Returning from functionList()\n");
            Output::Flush();
        }
#endif

        return functionList;
    }

    /**
     * Return the parsed IR for the given function.
     */
    Var GlobalObject::EntryRejitFunction(RecyclableObject *function, CallInfo callInfo, ...)
    {
#ifdef ENABLE_IR_VIEWER_DBG_DUMP
        if (Js::Configuration::Global.flags.Verbose)
        {
            Output::Print(L">> Entering rejitFunction()\n");
            Output::Flush();
        }
#endif

        //
        // retrieve and coerce arguments
        //

        RUNTIME_ARGUMENTS(args, callInfo);  // args[0] is (callInfo)
        Js::Var jsVarUtf8SourceInfo = args[1];
        Js::Var jsVarFunctionId = args[2];

#ifdef ENABLE_IR_VIEWER_DBG_DUMP
        if (Js::Configuration::Global.flags.Verbose)
        {
            Output::Print(L"jsVarUtf8SourceInfo: %d (0x%08X)\n", jsVarUtf8SourceInfo, jsVarUtf8SourceInfo);
            Output::Print(L"jsVarFunctionId: %d (0x%08X)\n", jsVarFunctionId, jsVarFunctionId);
        }
#endif

        Js::JavascriptNumber *jsUtf8SourceInfoNumber = NULL;
        Js::JavascriptNumber *jsFunctionIdNumber = NULL;
        long utf8SourceInfoNumber = 0;  // null
        long functionIdNumber = -1;  // start with invalid function id

        // extract value of jsVarUtf8SourceInfo
        if (Js::TaggedInt::Is(jsVarUtf8SourceInfo))
        {
            utf8SourceInfoNumber = (long)TaggedInt::ToInt64(jsVarUtf8SourceInfo); // REVIEW: just truncate?
        }
        else if (Js::JavascriptNumber::Is(jsVarUtf8SourceInfo))
        {
            jsUtf8SourceInfoNumber = (Js::JavascriptNumber *)jsVarUtf8SourceInfo;
            utf8SourceInfoNumber = (long)JavascriptNumber::GetValue(jsUtf8SourceInfoNumber);    // REVIEW: just truncate?
        }
        else
        {
            // should not reach here
            AssertMsg(false, "Failed to extract value for jsVarUtf8SourceInfo as either TaggedInt or JavascriptNumber.\n");
        }

        // extract value of jsVarFunctionId
        if (Js::TaggedInt::Is(jsVarFunctionId))
        {
            functionIdNumber = (long)TaggedInt::ToInt64(jsVarFunctionId); // REVIEW: just truncate?
        }
        else if (Js::JavascriptNumber::Is(jsVarFunctionId))
        {
            jsFunctionIdNumber = (Js::JavascriptNumber *)jsVarFunctionId;
            functionIdNumber = (long)JavascriptNumber::GetValue(jsFunctionIdNumber); // REVIEW: just truncate?
        }
        else
        {
            // should not reach here
            AssertMsg(false, "Failed to extract value for jsVarFunctionId as either TaggedInt or JavascriptNumber.\n");
        }

#ifdef ENABLE_IR_VIEWER_DBG_DUMP
        if (Js::Configuration::Global.flags.Verbose)
        {
            Output::Print(L"utf8SourceInfoNumber (value): %d (0x%08X)\n", utf8SourceInfoNumber, utf8SourceInfoNumber);
            Output::Print(L"jsVarFunctionId (value): %d (0x%08X)\n", functionIdNumber, functionIdNumber);
            Output::Print(L">> Executing rejitFunction(%d, %d)\n", utf8SourceInfoNumber, functionIdNumber);
            Output::Flush();
        }
#endif

        //
        // recover functionBody
        //

        Js::Utf8SourceInfo *sourceInfo = (Js::Utf8SourceInfo *)(utf8SourceInfoNumber);
        if (sourceInfo == NULL)
        {
            return NULL;
        }

        Js::FunctionBody *functionBody = sourceInfo->FindFunction((Js::LocalFunctionId)functionIdNumber);

        //
        // rejit the function body
        //

        Js::ScriptContext *scriptContext = function->GetScriptContext();
        NativeCodeGenerator *nativeCodeGenerator = scriptContext->GetNativeCodeGenerator();

        if (!functionBody || !functionBody->GetByteCode())
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }

        Js::Var retVal = RejitIRViewerFunction(nativeCodeGenerator, functionBody, scriptContext);

        //
        // return the parsed IR object
        //

#ifdef ENABLE_IR_VIEWER_DBG_DUMP
        if (Js::Configuration::Global.flags.Verbose)
        {
            Output::Print(L"rejitFunction - retVal: 0x%08X\n", retVal);
            Output::Flush();
        }
#endif

        return retVal;
    }

#endif /* IR_VIEWER */
    Var GlobalObject::EntryEvalHelper(ScriptContext* scriptContext, RecyclableObject* function, CallInfo callInfo, Js::Arguments& args)
    {
        FrameDisplay* environment = (FrameDisplay*)&NullFrameDisplay;
        ModuleID moduleID = kmodGlobal;
        BOOL strictMode = FALSE;
        // TODO: Handle call from global scope, strict mode
        BOOL isIndirect = FALSE;

        if (args.Info.Flags & CallFlags_CallEval)
        {
            // This was recognized as an eval call at compile time. The last one or two args are internal to us.
            // Argcount will be one of the following when called from global code
            //  - eval("...")     : argcount 3 : this, evalString, frameDisplay
            //  - eval.call("..."): argcount 2 : this(which is string) , frameDisplay
            if (args.Info.Count >= 2)
            {
                environment = (FrameDisplay*)(args[args.Info.Count - 1]);

                // Check for a module root passed from the caller. If it's there, use its module ID to compile the eval.
                // when called inside a module root, module root would be added before the frame display in above scenarios

                // ModuleRoot is optional
                //  - eval("...")     : argcount 3/4 : this, evalString , [module root], frameDisplay
                //  - eval.call("..."): argcount 2/3 : this(which is string) , [module root], frameDisplay

                strictMode = environment->GetStrictMode();

                if (args.Info.Count >= 3 && JavascriptOperators::GetTypeId(args[args.Info.Count - 2]) == TypeIds_ModuleRoot)
                {
                    moduleID = ((Js::ModuleRoot*)(RecyclableObject::FromVar(args[args.Info.Count - 2])))->GetModuleID();
                    args.Info.Count--;
                }
                args.Info.Count--;
            }
        }
        else
        {
            // This must be an indirect "eval" call that we didn't detect at compile time.
            // Pass null as the environment, which will force all lookups in the eval code
            // to use the root for the current module.
            // Also pass "null" for "this", which will force the callee to use the current module root.
            isIndirect = !PHASE_OFF1(Js::FastIndirectEvalPhase);
        }

        return GlobalObject::VEval(function->GetLibrary(), environment, moduleID, !!strictMode, !!isIndirect, args, /* isLibraryCode = */ false, /* registerDocument */ true);
    }

    Var GlobalObject::EntryEvalRestrictedMode(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        RUNTIME_ARGUMENTS(args, callInfo);
        Assert(!(callInfo.Flags & CallFlags_New));

        JavascriptLibrary* library = function->GetLibrary();
        ScriptContext* scriptContext = library->GetScriptContext();

        scriptContext->CheckEvalRestriction();
        
        return EntryEvalHelper(scriptContext, function, callInfo, args);
    }

    // This function is used to decipher eval function parameters and we dont want the stack arguments optimization by C++ compiler so turning off the optimization
    Var GlobalObject::EntryEval(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        RUNTIME_ARGUMENTS(args, callInfo);
        Assert(!(callInfo.Flags & CallFlags_New));

        JavascriptLibrary* library = function->GetLibrary();
        ScriptContext* scriptContext = library->GetScriptContext();

        return EntryEvalHelper(scriptContext, function, callInfo, args);
    }

    Var GlobalObject::VEval(JavascriptLibrary* library, FrameDisplay* environment, ModuleID moduleID, bool strictMode, bool isIndirect, 
        Arguments& args, bool isLibraryCode, bool registerDocument)
    {
        Assert(library);
        ScriptContext* scriptContext = library->GetScriptContext();

        unsigned argCount = args.Info.Count;

        AssertMsg(argCount > 0, "Should always have implicit 'this'");

        bool doRegisterDocument = registerDocument & !isLibraryCode;
        Var varThis = library->GetUndefined();

        if (argCount < 2)
        {
            return library->GetUndefined();
        }

        Var evalArg = args[1];
        if (!JavascriptString::Is(evalArg))
        {
            // "If x is not a string value, return x."
            return evalArg;
        }

        // It might happen that no script parsed on this context (scriptContext) till now,
        // so this Eval acts as the first source compile for scriptContext, tranisition to debugMode as needed
        scriptContext->TransitionToDebugModeIfFirstSource(/* utf8SourceInfo = */ nullptr);

        JavascriptString *argString = JavascriptString::FromVar(evalArg);
        ScriptFunction *pfuncScript;
        wchar_t const * sourceString = argString->GetSz();
        charcount_t sourceLen = argString->GetLength();
        FastEvalMapString key(sourceString, sourceLen, moduleID, strictMode, isLibraryCode);
        if (!scriptContext->IsInEvalMap(key, isIndirect, &pfuncScript))
        {
            if (BinaryFeatureControl::LanguageService() && scriptContext->authoringData && scriptContext->authoringData->Callbacks())
                scriptContext->authoringData->Callbacks()->PreparingEval(sourceLen);

            ulong grfscr = fscrReturnExpression | fscrEval | fscrEvalCode | fscrGlobalCode;
            if (isLibraryCode)
            {
                grfscr |= fscrIsLibraryCode;
            }
            pfuncScript = library->GetGlobalObject()->EvalHelper(scriptContext, argString->GetSz(), argString->GetLength(), moduleID, 
                grfscr, Constants::EvalCode, doRegisterDocument, isIndirect, strictMode);
            Assert(!pfuncScript->GetFunctionInfo()->IsGenerator());

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
            if (scriptContext->IsInDebugMode())
            {
                if (!(pfuncScript->GetFunctionBody()->GetUtf8SourceInfo()->GetIsLibraryCode() || pfuncScript->GetFunctionBody()->IsByteCodeDebugMode()))
                {
                    // Identifing if any function escaped for not being in debug mode. (This can be removed as a part of TFS : 935011)
                    Throw::FatalInternalError();
                }
            }
#endif
            scriptContext->AddToEvalMap(key, isIndirect, pfuncScript);
        }

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        else
        {
            if (scriptContext->IsInDebugMode())
            {
                if (!(pfuncScript->GetFunctionBody()->GetUtf8SourceInfo()->GetIsLibraryCode() || pfuncScript->GetFunctionBody()->IsByteCodeDebugMode()))
                {
                    // Identifing if any function escaped for not being in debug mode. (This can be removed as a part of TFS : 935011)
                    Throw::FatalInternalError();
                }
            }
        }
#endif

        //We shouldn't be serializing eval functions; unless with -ForceSerialized flag
        if (CONFIG_FLAG(ForceSerialized)) {
            pfuncScript->GetFunctionProxy()->EnsureDeserialized();
        }
        if (pfuncScript->GetFunctionBody()->GetHasThis())
        {
            // The eval expression refers to "this"
            if (args.Info.Flags & CallFlags_CallEval)
            {
                // If we are non-hidden call to eval then look for the "this" object in the frame display if the caller is a lambda else get "this" from the caller's frame.
                JavascriptFunction* pfuncCaller;
                JavascriptStackWalker::GetCaller(&pfuncCaller, scriptContext);
                if (pfuncCaller->GetFunctionInfo() != nullptr && pfuncCaller->GetFunctionInfo()->IsLambda())
                {
                    Var defaultInstance = (moduleID == kmodGlobal) ? JavascriptOperators::OP_LdRoot(scriptContext)->ToThis() : (Var)JavascriptOperators::GetModuleRoot(moduleID, scriptContext);
                    varThis = JavascriptOperators::OP_GetThisScoped(environment, defaultInstance, scriptContext);
                    UpdateThisForEval(varThis, moduleID, scriptContext, strictMode);
                }
                else
                {
                    JavascriptStackWalker::GetThis(&varThis, moduleID, scriptContext);
                    UpdateThisForEval(varThis, moduleID, scriptContext, strictMode);
                }
            }
            else
            {
                // The expression, which refers to "this", is evaluated by an indirect eval.
                // Set "this" to the current module root.
                varThis = JavascriptOperators::OP_GetThis(scriptContext->GetLibrary()->GetUndefined(), moduleID, scriptContext);
            }
        }

        if (pfuncScript->HasSuperReference())
        {
            // Indirect evals cannot have a super reference.
            if (!(args.Info.Flags & CallFlags_CallEval))
            {
                JavascriptError::ThrowSyntaxError(scriptContext, ERRSuperInIndirectEval, L"super");
            }
        }

        if (BinaryFeatureControl::LanguageService() && scriptContext->authoringData && scriptContext->authoringData->Callbacks())
            scriptContext->authoringData->Callbacks()->Executing();

        return library->GetGlobalObject()->ExecuteEvalParsedFunction(pfuncScript, environment, varThis);
    }

    void GlobalObject::UpdateThisForEval(Var &varThis, ModuleID moduleID, ScriptContext *scriptContext, BOOL strictMode)
    {
        if (strictMode)
        {
            varThis = JavascriptOperators::OP_StrictGetThis(varThis, scriptContext);
        }
        else
        {
            varThis = JavascriptOperators::OP_GetThisNoFastPath(varThis, moduleID, scriptContext);
        }
    }


    Var GlobalObject::ExecuteEvalParsedFunction(ScriptFunction *pfuncScript, FrameDisplay* environment, Var &varThis)
    {
        Assert(pfuncScript != NULL);

        pfuncScript->SetEnvironment(environment);
        //This function is supposed to be deserialized
        Assert(pfuncScript->GetFunctionBody());
        if (pfuncScript->GetFunctionBody()->GetFuncEscapes())
        {
            // Executing the eval causes the scope chain to escape.
            pfuncScript->InvalidateCachedScopeChain();
        }
        Var varResult = pfuncScript->GetEntryPoint()(pfuncScript, CallInfo(CallFlags_Eval, 1), varThis);
        pfuncScript->SetEnvironment(null);
        return varResult;
    }

    ScriptFunction* GlobalObject::ProfileModeEvalHelper(ScriptContext* scriptContext, const wchar_t *source, int sourceLength, ModuleID moduleID, ulong grfscr, LPCOLESTR pszTitle, BOOL registerDocument, BOOL isIndirect, BOOL strictMode)
    {
#if ENABLE_DEBUG_CONFIG_OPTIONS
        wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];
#endif
        ScriptFunction *pEvalFunction = DefaultEvalHelper(scriptContext, source, sourceLength, moduleID, grfscr, pszTitle, registerDocument, isIndirect, strictMode);
        Assert(pEvalFunction);
        Js::FunctionProxy *proxy = pEvalFunction->GetFunctionProxy();
        Assert(proxy);
        
        OUTPUT_TRACE(Js::ScriptProfilerPhase, L"GlobalObject::ProfileModeEvalHelper FunctionNumber : %s, Entrypoint : 0x%08X IsFunctionDefer : %d\n",
                                    proxy->GetDebugNumberSet(debugStringBuffer), pEvalFunction->GetEntryPoint(), proxy->IsDeferred());

        if (proxy->IsDeferred())
        {
            // This could happen if the top level function is marked as deferred, we need to parse this to generate the script compile information (RegisterScript depends on that)
            Js::JavascriptFunction::DeferredParse(&pEvalFunction);
        }

        scriptContext->RegisterScript(proxy);

        return pEvalFunction;
    }

    void GlobalObject::ValidateSyntax(ScriptContext* scriptContext, const wchar_t *source, int sourceLength, bool isGenerator, void (Parser::*validateSyntax)())
    {
        Assert(sourceLength >= 0);

        HRESULT hr = S_OK;
        HRESULT hrParser = E_FAIL;
        CompileScriptException se;

        BEGIN_LEAVE_SCRIPT_INTERNAL(scriptContext);
        BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT
        {
            ArenaAllocator tempAlloc(L"ValidateSyntaxArena", scriptContext->GetThreadContext()->GetPageAllocator(), Throw::OutOfMemory);

            size_t cchSource = sourceLength;
            size_t cbUtf8Buffer = (cchSource + 1) * 3;
            LPUTF8 utf8Source = AnewArray(&tempAlloc, utf8char_t, cbUtf8Buffer);
            Assert(cchSource < MAXLONG);
            size_t cbSource = utf8::EncodeIntoAndNullTerminate(utf8Source, source, static_cast< charcount_t >(cchSource));
            utf8Source = reinterpret_cast< LPUTF8 >( tempAlloc.Realloc(utf8Source, cbUtf8Buffer, cbSource + 1) );

            Parser parser(scriptContext);
            hrParser = parser.ValidateSyntax(utf8Source, cbSource, isGenerator, &se, validateSyntax);
        }
        END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);
        END_LEAVE_SCRIPT_INTERNAL(scriptContext);

        if (FAILED(hr))
        {
            if (hr == E_OUTOFMEMORY)
            {
                JavascriptError::ThrowOutOfMemoryError(scriptContext);
            }
            if (hr == VBSERR_OutOfStack)
            {
                JavascriptError::ThrowStackOverflowError(scriptContext);
            }
        }
        if (!SUCCEEDED(hrParser))
        {
            hrParser = SCRIPT_E_RECORDED;
            EXCEPINFO ei;
            se.GetError(&hrParser, &ei);

            ErrorTypeEnum errorType;
            switch (ei.scode)
            {
    #define RT_ERROR_MSG(name, errnum, str1, str2, jst, errorNumSource) \
            case name: \
                errorType = jst; \
                break;
    #define RT_PUBLICERROR_MSG(name, errnum, str1, str2, jst, errorNumSource) RT_ERROR_MSG(name, errnum, str1, str2, jst, errorNumSource)
    #include "rterrors.h"
    #undef RT_PUBLICERROR_MSG
    #undef RT_ERROR_MSG
            default:
                errorType = kjstSyntaxError;
            }
            JavascriptError::MapAndThrowError(scriptContext, ei.scode, errorType, &ei);
        }
    }

    ScriptFunction* GlobalObject::DefaultEvalHelper(ScriptContext* scriptContext, const wchar_t *source, int sourceLength, ModuleID moduleID, ulong grfscr, LPCOLESTR pszTitle, BOOL registerDocument, BOOL isIndirect, BOOL strictMode)
    {
        Assert(sourceLength >= 0);

        if (scriptContext->GetThreadContext()->EvalDisabled())
        {
            throw Js::EvalDisabledException();
        }

#ifdef PROFILE_EXEC
        scriptContext->ProfileBegin(Js::EvalCompilePhase);
#endif
        void * frameAddr = null;
        GET_CURRENT_FRAME_ID(frameAddr);

        HRESULT hr = S_OK;
        HRESULT hrParser = S_OK;
        HRESULT hrCodeGen = S_OK;
        CompileScriptException se;
        Js::ParseableFunctionInfo * funcBody = NULL;

        BEGIN_LEAVE_SCRIPT_INTERNAL(scriptContext);
        BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT
        {
            size_t cchSource = sourceLength;
            size_t cbUtf8Buffer = (cchSource + 1) * 3;

            ArenaAllocator tempArena(L"EvalHelperArena", scriptContext->GetThreadContext()->GetPageAllocator(), Js::Throw::OutOfMemory);
            LPUTF8 utf8Source = AnewArray(&tempArena, utf8char_t, cbUtf8Buffer);

            Assert(cchSource < MAXLONG);
            size_t cbSource = utf8::EncodeIntoAndNullTerminate(utf8Source, source, static_cast< charcount_t >(cchSource));
            Assert(cbSource + 1 <= cbUtf8Buffer);

            SRCINFO const * pSrcInfo = scriptContext->GetModuleSrcInfo(moduleID);
            // Source Info objects are kept alive by the function bodies that are referencing it
            // The function body is created in GenerateByteCode but the source info isn't passed in, only the index
            // So we need to pin it here (TODO: Change GenerateByteCode to take in the sourceInfo itself)
            ENTER_PINNED_SCOPE(Utf8SourceInfo, sourceInfo);
            sourceInfo = Utf8SourceInfo::New(scriptContext, utf8Source, cchSource, cbSource, pSrcInfo);
            if ((grfscr & fscrIsLibraryCode) != 0)
            {
                sourceInfo->SetIsLibraryCode();
            }

            Parser parser(scriptContext, strictMode);
            bool forceNoNative = false;

            ParseNodePtr parseTree;

            SourceContextInfo * sourceContextInfo = pSrcInfo->sourceContextInfo;
            ULONG deferParseThreshold = Parser::GetDeferralThreshold(sourceContextInfo->sourceDynamicProfileManager);
            if ((ULONG)sourceLength > deferParseThreshold && !PHASE_OFF1(Phase::DeferParsePhase))
            {
                // Defer function bodies declared inside large dynamic blocks.
                grfscr |= fscrDeferFncParse;
            }

            grfscr = grfscr | fscrDynamicCode;

            hrParser = parser.ParseCesu8Source(&parseTree, utf8Source, cbSource, grfscr, &se, &sourceContextInfo->nextLocalFunctionId,
                sourceContextInfo);
            sourceInfo->SetParseFlags(grfscr);

            if (SUCCEEDED(hrParser) && parseTree)
            {
                // This keeps function bodies generated by the byte code alive till we return
                Js::AutoDynamicCodeReference dynamicFunctionReference(scriptContext);

                Assert(cchSource < MAXLONG);
                uint sourceIndex = scriptContext->SaveSourceNoCopy(sourceInfo, cchSource, true);                

                // Tell byte code gen not to attempt to interact with the caller's context if this is indirect eval.
                // TODO: Handle strict mode.
                if (isIndirect && 
                    !strictMode && 
                    !parseTree->sxFnc.GetStrictMode())
                {
                    grfscr &= ~fscrEval;
                }
                hrCodeGen = GenerateByteCode(parseTree, grfscr, scriptContext, &funcBody, sourceIndex, forceNoNative, &parser, &se);
                sourceInfo->SetByteCodeGenerationFlags(grfscr);
            }

            LEAVE_PINNED_SCOPE();
        }
        END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);
        END_LEAVE_SCRIPT_INTERNAL(scriptContext);


#ifdef PROFILE_EXEC
        scriptContext->ProfileEnd(Js::EvalCompilePhase);
#endif
        if (hr == E_OUTOFMEMORY)
        {
            JavascriptError::ThrowOutOfMemoryError(scriptContext);
        }
        else if(hr == VBSERR_OutOfStack)
        {
            JavascriptError::ThrowStackOverflowError(scriptContext);
        }
        else if(hr == E_ABORT)
        {
            throw Js::ScriptAbortException();
        }
        else if(FAILED(hr))
        {
            throw Js::InternalErrorException();
        }

        if (!SUCCEEDED(hrParser))
        {
            JavascriptError::ThrowParserError(scriptContext, hrParser, &se);
        }
        else if (!SUCCEEDED(hrCodeGen))
        {
            Assert(hrCodeGen == SCRIPT_E_RECORDED);
            hrCodeGen = se.ei.scode;
            /*
             * VBSERR_OutOfStack is of type kjstError but we throw a (more specific) StackOverflowError when a hard stack
             * overflow occurs. To keep the behavior consistent I'm special casing it here.
             */
            se.Free();
            if (hrCodeGen == VBSERR_OutOfStack)
            {
                JavascriptError::ThrowStackOverflowError(scriptContext);
            }
            JavascriptError::MapAndThrowError(scriptContext, hrCodeGen);
        }
        else
        {
            Assert(funcBody != null);
            funcBody->SetDisplayName(pszTitle);

            // Set the functionbody information to dynamic content PROFILER_SCRIPT_TYPE_DYNAMIC
            funcBody->SetIsTopLevel(true);

            // If not library code then let's find the parent, we may need to register this source if any exception happens later
            if ((grfscr & fscrIsLibraryCode) == 0)
            {
                // For parented eval get the caller's utf8SourceInfo
                JavascriptFunction* pfuncCaller;
                if (JavascriptStackWalker::GetCaller(&pfuncCaller, scriptContext) && pfuncCaller && pfuncCaller->IsScriptFunction())
                {
                    FunctionBody* parentFuncBody = pfuncCaller->GetFunctionBody();
                    Utf8SourceInfo* parentUtf8SourceInfo = parentFuncBody->GetUtf8SourceInfo();
                    Utf8SourceInfo* utf8SourceInfo = funcBody->GetFunctionProxy()->GetUtf8SourceInfo();
                    utf8SourceInfo->SetCallerUtf8SourceInfo(parentUtf8SourceInfo);
                }
            }

            if (registerDocument)
            {
                scriptContext->DbgRegisterFunction(funcBody, pszTitle);
                funcBody = funcBody->GetParseableFunctionInfo(); // DbgRegisterFunction may parse and update function body
            }

            ScriptFunction* pfuncScript = funcBody->IsGenerator() ?
                scriptContext->GetLibrary()->CreateGeneratorVirtualScriptFunction(funcBody) :
                scriptContext->GetLibrary()->CreateScriptFunction(funcBody);

            return pfuncScript;
        }
    }

#ifdef IR_VIEWER
    Var GlobalObject::IRDumpEvalHelper(ScriptContext* scriptContext, const wchar_t *source,
        int sourceLength, ModuleID moduleID, ulong grfscr, LPCOLESTR pszTitle,
        BOOL registerDocument, BOOL isIndirect, BOOL strictMode)
    {
        // TODO (t-doilij) clean up this function, specifically used for ir dump (don't execute bytecode; potentially dangerous)

        Assert(sourceLength >= 0);

        if (scriptContext->GetThreadContext()->EvalDisabled())
        {
            throw Js::EvalDisabledException();
        }

#ifdef PROFILE_EXEC
        scriptContext->ProfileBegin(Js::EvalCompilePhase);
#endif
        void * frameAddr = null;
        GET_CURRENT_FRAME_ID(frameAddr);

        HRESULT hr = S_OK;
        HRESULT hrParser = S_OK;
        HRESULT hrCodeGen = S_OK;
        CompileScriptException se;
        Js::ParseableFunctionInfo * funcBody = NULL;

        BEGIN_LEAVE_SCRIPT_INTERNAL(scriptContext);
        BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT
        {
            size_t cchSource = sourceLength;
            size_t cbUtf8Buffer = (cchSource + 1) * 3;

            ArenaAllocator tempArena(L"EvalHelperArena", scriptContext->GetThreadContext()->GetPageAllocator(), Js::Throw::OutOfMemory);
            LPUTF8 utf8Source = AnewArray(&tempArena, utf8char_t, cbUtf8Buffer);

            Assert(cchSource < MAXLONG);
            size_t cbSource = utf8::EncodeIntoAndNullTerminate(utf8Source, source, static_cast< charcount_t >(cchSource));
            Assert(cbSource + 1 <= cbUtf8Buffer);

            SRCINFO const * pSrcInfo = scriptContext->GetModuleSrcInfo(moduleID);
            // Source Info objects are kept alive by the function bodies that are referencing it
            // The function body is created in GenerateByteCode but the source info isn't passed in, only the index
            // So we need to pin it here (TODO: Change GenerateByteCode to take in the sourceInfo itself)
            ENTER_PINNED_SCOPE(Utf8SourceInfo, sourceInfo);
            sourceInfo = Utf8SourceInfo::New(scriptContext, utf8Source, cchSource, cbSource, pSrcInfo);
            if ((grfscr & fscrIsLibraryCode) != 0)
            {
                sourceInfo->SetIsLibraryCode();
            }

            Parser parser(scriptContext, strictMode);
            bool forceNoNative = false;

            ParseNodePtr parseTree;

            SourceContextInfo * sourceContextInfo = pSrcInfo->sourceContextInfo;

            grfscr = grfscr | fscrDynamicCode;

            hrParser = parser.ParseCesu8Source(&parseTree, utf8Source, cbSource, grfscr, &se, &sourceContextInfo->nextLocalFunctionId,
                sourceContextInfo);
            sourceInfo->SetParseFlags(grfscr);

            if (SUCCEEDED(hrParser) && parseTree)
            {
                // This keeps function bodies generated by the byte code alive till we return
                Js::AutoDynamicCodeReference dynamicFunctionReference(scriptContext);

                Assert(cchSource < MAXLONG);
                uint sourceIndex = scriptContext->SaveSourceNoCopy(sourceInfo, cchSource, true);

                grfscr = grfscr | fscrIrDumpEnable;

                hrCodeGen = GenerateByteCode(parseTree, grfscr, scriptContext, &funcBody, sourceIndex, forceNoNative, &parser, &se);
                sourceInfo->SetByteCodeGenerationFlags(grfscr);
            }

            LEAVE_PINNED_SCOPE();
        }
        END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);
        END_LEAVE_SCRIPT_INTERNAL(scriptContext);


#ifdef PROFILE_EXEC
        scriptContext->ProfileEnd(Js::EvalCompilePhase);
#endif
        if (hr == E_OUTOFMEMORY)
        {
            JavascriptError::ThrowOutOfMemoryError(scriptContext);
        }
        else if(hr == VBSERR_OutOfStack)
        {
            JavascriptError::ThrowStackOverflowError(scriptContext);
        }
        else if(hr == E_ABORT)
        {
            throw Js::ScriptAbortException();
        }
        else if(FAILED(hr))
        {
            throw Js::InternalErrorException();
        }

        if (!SUCCEEDED(hrParser))
        {
            hrParser = SCRIPT_E_RECORDED;
            EXCEPINFO ei;
            se.GetError(&hrParser, &ei);

            ErrorTypeEnum errorType;
            switch (ei.scode)
            {
    #define RT_ERROR_MSG(name, errnum, str1, str2, jst, errorNumSource) \
            case name: \
                errorType = jst; \
                break;
    #define RT_PUBLICERROR_MSG(name, errnum, str1, str2, jst, errorNumSource) RT_ERROR_MSG(name, errnum, str1, str2, jst, errorNumSource)
    #include "rterrors.h"
    #undef RT_PUBLICERROR_MSG
    #undef RT_ERROR_MSG
            default:
                errorType = kjstSyntaxError;
            }
            JavascriptError::MapAndThrowError(scriptContext, ei.scode, errorType, &ei);
        }
        else if (!SUCCEEDED(hrCodeGen))
        {
            Assert(hrCodeGen == SCRIPT_E_RECORDED);
            hrCodeGen = se.ei.scode;
            /*
             * VBSERR_OutOfStack is of type kjstError but we throw a (more specific) StackOverflowError when a hard stack
             * overflow occurs. To keep the behavior consistent I'm special casing it here.
             */
            se.Free();
            if (hrCodeGen == VBSERR_OutOfStack)
            {
                JavascriptError::ThrowStackOverflowError(scriptContext);
            }
            JavascriptError::MapAndThrowError(scriptContext, hrCodeGen);
        }
        else
        {
            Assert(funcBody != null);
            funcBody->SetDisplayName(pszTitle);

            // Set the functionbody information to dynamic content PROFILER_SCRIPT_TYPE_DYNAMIC
            funcBody->SetIsTopLevel(true);

            if (registerDocument)
            {
                scriptContext->DbgRegisterFunction(funcBody, pszTitle);
                funcBody = funcBody->GetParseableFunctionInfo(); // DbgRegisterFunction may parse and update function body
            }

            NativeCodeGenerator *nativeCodeGenerator = scriptContext->GetNativeCodeGenerator();
            Js::FunctionBody *functionBody = funcBody->GetFunctionBody(); // funcBody is ParseableFunctionInfo

            if (!functionBody || !functionBody->GetByteCode())
            {
                return scriptContext->GetLibrary()->GetUndefined();
            }

            Js::Var retVal = RejitIRViewerFunction(nativeCodeGenerator, functionBody, scriptContext);
            return retVal;
        }
    }
#endif /* IR_VIEWER */

    Var GlobalObject::EntryParseInt(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        Assert(!(callInfo.Flags & CallFlags_New));

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");

        JavascriptString *str;
        int radix = 0;


        if(args.Info.Count < 2)
        {
            // We're really converting "undefined" here, but "undefined" produces
            // NaN, so just return that directly.
            return JavascriptNumber::ToVarNoCheck(JavascriptNumber::NaN, scriptContext);
        }


        // Short cut for numbers and radix 10
        if (((args.Info.Count == 2) ||
            (TaggedInt::Is(args[2]) && TaggedInt::ToInt32(args[2]) == 10) ||
            (JavascriptNumber::Is(args[2]) && JavascriptNumber::GetValue(args[2]) == 10)))
        {
            if (TaggedInt::Is(args[1]))
            {
                return args[1];
            }
            if (JavascriptNumber::Is(args[1]))
            {
                double value = JavascriptNumber::GetValue(args[1]);

                // make sure we are in the ranges that don't have exponential notation.
                double absValue = ::abs(value);
                if (absValue < 1.0e21 && absValue >= 1e-5)
                {
                    double result;
                    ::modf(value, &result);
                    return JavascriptNumber::ToVarIntCheck(result, scriptContext);
                }
            }
        }

        // convert input to a string
        if (JavascriptString::Is(args[1]))
        {
            str = JavascriptString::FromVar(args[1]);
        }
        else
        {
            str = JavascriptConversion::ToString(args[1], scriptContext);
        }

        if(args.Info.Count > 2)
        {
            // retrieve the radix
            // TODO : verify for ES5 : radix is 10 when undefined or 0, except when it starts with 0x when it is 16.
            radix = JavascriptConversion::ToInt32(args[2], scriptContext);
            if(radix < 0 || radix == 1 || radix > 36)
            {
                //illegal, return NaN
                return JavascriptNumber::ToVarNoCheck(JavascriptNumber::NaN, scriptContext);
            }
        }

        Var result = str->ToInteger(radix);
        return result;
    }

    Var GlobalObject::EntryParseFloat(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        Assert(!(callInfo.Flags & CallFlags_New));

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");

        JavascriptString *str;

        if(args.Info.Count < 2)
        {
            // We're really converting "undefined" here, but "undefined" produces
            // NaN, so just return that directly.
            return JavascriptNumber::ToVarNoCheck(JavascriptNumber::NaN, scriptContext);
        }

        // Short cut for numbers
        if (TaggedInt::Is(args[1]))
        {
            return args[1];
        }
        if(JavascriptNumber::Is_NoTaggedIntCheck(args[1]))
        {
            // If the value is negative zero, return positive zero. Since the parameter type is string, -0 would first be
            // converted to the string "0", then parsed to result in positive zero.
            return
                JavascriptNumber::IsNegZero(JavascriptNumber::GetValue(args[1])) ?
                    TaggedInt::ToVarUnchecked(0) :
                    args[1];
        }

        // convert input to a string
        if (JavascriptString::Is(args[1]))
        {
            str = JavascriptString::FromVar(args[1]);
        }
        else
        {
            str = JavascriptConversion::ToString(args[1], scriptContext);
        }

        // skip preceding whitespace
        const wchar_t *pch = scriptContext->GetCharClassifier()->SkipWhiteSpace(str->GetSz());

        // perform the string -> float conversion
        double result = StrToDbl(pch, &pch, scriptContext);

        return JavascriptNumber::ToVarNoCheck(result, scriptContext);
    }

    Var GlobalObject::EntryIsNaN(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        Assert(!(callInfo.Flags & CallFlags_New));

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");

        if(args.Info.Count < 2)
        {
            return scriptContext->GetLibrary()->GetTrue();
        }
        return JavascriptBoolean::ToVar(
            JavascriptNumber::IsNan(JavascriptConversion::ToNumber(args[1],scriptContext)),
            scriptContext);
    }

    Var GlobalObject::EntryIsFinite(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        Assert(!(callInfo.Flags & CallFlags_New));

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        if(args.Info.Count < 2)
        {
            return scriptContext->GetLibrary()->GetFalse();
        }
        return JavascriptBoolean::ToVar(
            NumberUtilities::IsFinite(JavascriptConversion::ToNumber(args[1],scriptContext)),
            scriptContext);
    }

    Var GlobalObject::EntryDecodeURI(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        Assert(!(callInfo.Flags & CallFlags_New));

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        return UriHelper::DecodeCoreURI(scriptContext, args, UriHelper::URIReserved);
    }

    Var GlobalObject::EntryDecodeURIComponent(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        Assert(!(callInfo.Flags & CallFlags_New));

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        return UriHelper::DecodeCoreURI(scriptContext, args, UriHelper::URINone);
    }

    Var GlobalObject::EntryEncodeURI(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        Assert(!(callInfo.Flags & CallFlags_New));

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        return UriHelper::EncodeCoreURI(scriptContext, args, UriHelper::URIReserved | UriHelper::URIUnescaped);
    }

    Var GlobalObject::EntryEncodeURIComponent(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        Assert(!(callInfo.Flags & CallFlags_New));

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");

        return UriHelper::EncodeCoreURI(scriptContext, args, UriHelper::URIUnescaped);
    }

    //
    // Part of Appendix B of ES5 spec
    //

    // This is a bit field for the hex values: 00-29, 2C, 3A-3F, 5B-5E, 60, 7B-FF
    // These are the values escape encodes using the default mask (or mask >= 4)
    static const BYTE _grfbitEscape[] =
    {
        0xFF, 0xFF, // 00 - 0F
        0xFF, 0xFF, // 10 - 1F
        0xFF, 0x13, // 20 - 2F
        0x00, 0xFC, // 30 - 3F
        0x00, 0x00, // 40 - 4F
        0x00, 0x78, // 50 - 5F
        0x01, 0x00, // 60 - 6F
        0x00, 0xF8, // 70 - 7F
        0xFF, 0xFF, // 80 - 8F
        0xFF, 0xFF, // 90 - 9F
        0xFF, 0xFF, // A0 - AF
        0xFF, 0xFF, // B0 - BF
        0xFF, 0xFF, // C0 - CF
        0xFF, 0xFF, // D0 - DF
        0xFF, 0xFF, // E0 - EF
        0xFF, 0xFF, // F0 - FF
    };
    Var GlobalObject::EntryEscape(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        Assert(!(callInfo.Flags & CallFlags_New));

        if (args.Info.Count <= 1)
        {
            return scriptContext->GetLibrary()->GetUndefinedDisplayString();
        }

        JavascriptString *src = JavascriptConversion::ToString(args[1], scriptContext);

        CompoundString *const bs = CompoundString::NewWithCharCapacity(src->GetLength(), scriptContext->GetLibrary());
        wchar_t chw;
        wchar_t * pchSrc;
        wchar_t * pchLim;

        const char _rgchHex[] = "0123456789ABCDEF";

        pchSrc = const_cast<wchar_t *>(src->GetString());
        pchLim = pchSrc + src->GetLength();
        while (pchSrc < pchLim)
        {
            chw = *pchSrc++;

            if (0 != (chw & 0xFF00))
            {
                bs->AppendChars(L"%u");
                bs->AppendChars(static_cast<wchar_t>(_rgchHex[(chw >> 12) & 0x0F]));
                bs->AppendChars(static_cast<wchar_t>(_rgchHex[(chw >> 8) & 0x0F]));
                bs->AppendChars(static_cast<wchar_t>(_rgchHex[(chw >> 4) & 0x0F]));
                bs->AppendChars(static_cast<wchar_t>(_rgchHex[chw & 0x0F]));
            }
            else if (_grfbitEscape[chw >> 3] & (1 << (chw & 7)))
            {
                // Escape the byte.
                bs->AppendChars(L'%');
                bs->AppendChars(static_cast<wchar_t>(_rgchHex[chw >> 4]));
                bs->AppendChars(static_cast<wchar_t>(_rgchHex[chw & 0x0F]));
            }
            else
            {
                bs->AppendChars(chw);
            }
        }

        return bs;
    }

    //
    // Part of Appendix B of ES5 spec
    //
    Var GlobalObject::EntryUnEscape(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        Assert(!(callInfo.Flags & CallFlags_New));

        if (args.Info.Count <= 1)
        {
            return scriptContext->GetLibrary()->GetUndefinedDisplayString();
        }

        wchar_t chw;
        UOLECHAR chT;
        wchar_t * pchSrc;
        wchar_t * pchLim;
        wchar_t * pchMin;

        JavascriptString *src = JavascriptConversion::ToString(args[1], scriptContext);

        CompoundString *const bs = CompoundString::NewWithCharCapacity(src->GetLength(), scriptContext->GetLibrary());

        pchSrc = const_cast<wchar_t *>(src->GetString());
        pchLim = pchSrc + src->GetLength();
        while (pchSrc < pchLim)
        {
            if ('%' != (chT = *pchSrc++))
            {
                bs->AppendChars(chT);
                continue;
            }

            pchMin = pchSrc;
            chT = *pchSrc++;
            if ('u' != chT)
            {
                chw = 0;
                goto LTwoHexDigits;
            }

            // 4 hex digits.
            if ((chT = *pchSrc++ - '0') <= 9)
                chw = chT * 0x1000;
            else if ((chT -= 'A' - '0') <= 5 || (chT -= 'a' - 'A') <= 5)
                chw = (chT + 10) * 0x1000;
            else
                goto LHexError;
            if ((chT = *pchSrc++ - '0') <= 9)
                chw += chT * 0x0100;
            else if ((chT -= 'A' - '0') <= 5 || (chT -= 'a' - 'A') <= 5)
                chw += (chT + 10) * 0x0100;
            else
                goto LHexError;
            chT = *pchSrc++;
LTwoHexDigits:
            if ((chT -= '0') <= 9)
                chw += chT * 0x0010;
            else if ((chT -= 'A' - '0') <= 5 || (chT -= 'a' - 'A') <= 5)
                chw += (chT + 10) * 0x0010;
            else
                goto LHexError;
            if ((chT = *pchSrc++ - '0') <= 9)
                chw += chT;
            else if ((chT -= 'A' - '0') <= 5 || (chT -= 'a' - 'A') <= 5)
                chw += chT + 10;
            else
            {
LHexError:
                pchSrc = pchMin;
                chw = '%';
            }

            bs->AppendChars(chw);
        }

        return bs;
    }

    //TODO review these extensions per spec . and version them
    Var GlobalObject::EntryScriptEngine(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        Assert(!(callInfo.Flags & CallFlags_New));

        ScriptContext* scriptContext = function->GetScriptContext();
        return scriptContext->GetLibrary()->CreateStringFromCppLiteral(L"JScript");
    }
    Var GlobalObject::EntryScriptEngineMajorVersion(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        Assert(!(callInfo.Flags & CallFlags_New));

        ScriptContext* scriptContext = function->GetScriptContext();
        return Js::JavascriptNumber::ToVar(SCRIPT_ENGINE_MAJOR_VERSION, scriptContext);
    }
    Var GlobalObject::EntryScriptEngineMinorVersion(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        Assert(!(callInfo.Flags & CallFlags_New));

        ScriptContext* scriptContext = function->GetScriptContext();
        return Js::JavascriptNumber::ToVar(SCRIPT_ENGINE_MINOR_VERSION, scriptContext);
    }
    Var GlobalObject::EntryScriptEngineBuildVersion(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        Assert(!(callInfo.Flags & CallFlags_New));

        ScriptContext* scriptContext = function->GetScriptContext();
        return Js::JavascriptNumber::ToVar(SCRIPT_ENGINE_BUILDNUMBER, scriptContext);
    }

#if DBG
    void DebugClearStack()
    {
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
    }
#endif

    Var GlobalObject::EntryCollectGarbage(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

#if DBG_DUMP
        if (Js::Configuration::Global.flags.TraceWin8Allocations)
        {
            Output::Print(L"MemoryTrace: GlobalObject::EntryCollectGarbage Invoke\n");
            Output::Flush();
        }
#endif

#if DBG
        // Clear 1K of stack to avoid false positive in debug build.
        // Because we don't debug build don't stack pack
        DebugClearStack();
#endif
        Assert(!(callInfo.Flags & CallFlags_New));

        ScriptContext* scriptContext = function->GetScriptContext();
        Recycler* recycler = scriptContext->GetRecycler();
        if (recycler)
        {
#if DBG && defined(RECYCLER_DUMP_OBJECT_GRAPH)
            ARGUMENTS(args, callInfo);
            if (args.Info.Count > 1)
            {
                double value = JavascriptConversion::ToNumber(args[1], function->GetScriptContext());
                if (value == 1.0)
                {
                    recycler->dumpObjectOnceOnCollect = true;
                }
            }
#endif
            recycler->CollectNow<CollectNowDecommitNowExplicit>();
        }

#if DBG_DUMP
#if ENABLE_PROJECTION
        scriptContext->GetThreadContext()->DumpProjectionContextMemoryStats(L"Stats after GlobalObject::EntryCollectGarbage call");
#endif

        if (Js::Configuration::Global.flags.TraceWin8Allocations)
        {
            Output::Print(L"MemoryTrace: GlobalObject::EntryCollectGarbage Exit\n");
            Output::Flush();
        }
#endif

        return scriptContext->GetLibrary()->GetUndefined();
    }

    //Pattern match is unique to RuntimeObject. Only leading and trailing * are implemented
    //Example: *pat*tern* actually matches all the strings having pat*tern as substring.
    BOOL GlobalObject::MatchPatternHelper(JavascriptString *propertyName, JavascriptString *pattern, ScriptContext *scriptContext)
    {
        if (null == propertyName || null == pattern)
        {
            return FALSE;
        }

        charcount_t     patternLength = pattern->GetLength();
        charcount_t     nameLength = propertyName->GetLength();
        BOOL    bStart;
        BOOL    bEnd;

        if (0 == patternLength)
        {
            return TRUE; // empty pattern matches all
        }

        bStart = (L'*' == pattern->GetItem(0));// Is there a start star?
        bEnd = (L'*' == pattern->GetItem(patternLength - 1));// Is there an end star?

        //deal with the stars
        if (bStart || bEnd)
        {
            JavascriptString *subPattern;
            int idxStart = bStart? 1: 0;
            int idxEnd   = bEnd? (patternLength - 1): patternLength;
            if (idxEnd <= idxStart)
            {
                return TRUE; //scenario double star **
            }

            //get a pattern which doesn't contain leading and trailing stars
            subPattern = JavascriptString::FromVar(JavascriptString::SubstringCore(pattern, idxStart, idxEnd - idxStart, scriptContext));

            uint index = JavascriptString::strstr(propertyName, subPattern, false);

            if (index == (uint)-1)
            {
                return FALSE;
            }
            if (FALSE == bStart)
            {
                return index == 0; //begin at the same place;
            }
            else if (FALSE == bEnd)
            {
                return (index + patternLength - 1) == (nameLength);
            }
            return TRUE;
        }
        else
        {
            //full match
            if (0 == JavascriptString::strcmp(propertyName, pattern))
            {
                return TRUE;
            }
        }

        return FALSE;
    }    

    GlobalObject* GlobalObject::MakeCopyOnWriteObject(ScriptContext* scriptContext)
    {
        VERIFY_COPY_ON_WRITE_ENABLED_RET();

        // If the target script context already has a global object, return that one.
        GlobalObject * destGlobalObject = scriptContext->GetGlobalObject();
        if (destGlobalObject)
        {
            AssertMsg(destGlobalObject->GetScriptContext() == scriptContext &&
                destGlobalObject->GetLibrary() == scriptContext->GetLibrary(), "existing global object is invalid");
            return destGlobalObject;
        }

        Recycler *recycler = scriptContext->GetRecycler();
        destGlobalObject = GlobalObject::New(scriptContext);
        DynamicType * globalType = destGlobalObject->GetDynamicType();
        recycler->RootAddRef(destGlobalObject);

        JavascriptLibrary* destLibrary = RecyclerNewFinalized(recycler, JavascriptLibrary, destGlobalObject);
        scriptContext->SetLibrary(destLibrary);
        globalType->javascriptLibrary = destLibrary;
        destGlobalObject->library = destLibrary;
        destLibrary->globalObject = destGlobalObject;
        destLibrary->scriptContext = scriptContext;
        destLibrary->recycler = scriptContext->GetRecycler();

        scriptContext->RecordCopyOnWrite(this, destGlobalObject);

        // Before we can call scriptContext->CopyOnWrite() the globalObject of the script context
        // must be pointing to this object or the copy-on-write objects will have the wrong
        // global object and prototypes.
        scriptContext->SetGlobalObject(destGlobalObject);

        destLibrary->CopyOnWriteFrom(scriptContext, destGlobalObject, this->GetLibrary());

        // Copy the remaining properties. We don't create a copy-on-write object since
        // the global object is going to be modified immediately and would need to be
        // detached immediately anyway.
        Var enumeratorVar;
        if (this->GetEnumerator(true, &enumeratorVar, scriptContext, false, false))
        {
            JavascriptEnumerator* enumerator = (JavascriptEnumerator*)enumeratorVar;
            while (enumerator->MoveNext())
            {
                Var index = enumerator->GetCurrentIndex();

                PropertyId id;
                if (enumerator->GetCurrentPropertyId(&id))
                {
                    Var getter;
                    Var setter;

                    // The global object is special in that all the built-ins have already
                    // been partially populated by library->CopyOnWriteFrom() and copying
                    // again is redundant and might result in incorrect behavior so we avoid
                    // copying values that are already set on global.
                    if (destGlobalObject->HasProperty(id))
                        continue;

                    // If the value is a getter or setter ensure we copy it correctly
                    if (this->GetAccessors(id, &getter, &setter, this->GetScriptContext()))
                    {
                        // This is a getter and/or setter property.
                        Js::PropertyDescriptor propertyDescriptor;
                        if (!this->IsEnumerable(id))
                            propertyDescriptor.SetEnumerable(false);
                        if (!this->IsConfigurable(id))
                            propertyDescriptor.SetConfigurable(false);
                        // Ignore Writable because specifying turns the descriptor into a data descriptor
                        // which ignores the setter and getter.
                        if (getter)
                            propertyDescriptor.SetGetter(scriptContext->CopyOnWrite(getter));
                        if (setter)
                            propertyDescriptor.SetSetter(scriptContext->CopyOnWrite(setter));
                        JavascriptOperators::DefineOwnPropertyDescriptor(destGlobalObject, id, propertyDescriptor, false, scriptContext);
                        continue;
                    }

                    // If the property has non-default attributes, ensure the copy has them as well.
                    PropertyAttributes attributes =
                        (this->IsEnumerable(id) ? PropertyEnumerable : PropertyNone) |
                        (this->IsWritable(id) ? PropertyWritable : PropertyNone) |
                        (this->IsConfigurable(id) ? PropertyConfigurable : PropertyNone);
                    if (attributes != PropertyDynamicTypeDefaults)
                    {
                        destGlobalObject->SetPropertyWithAttributes(id, scriptContext->CopyOnWrite(enumerator->GetCurrentValue()), attributes, null);
                        continue;
                    }
                }

                // Otherwise just copy the value directly.
                Var value = scriptContext->CopyOnWrite(enumerator->GetCurrentValue());
                JavascriptOperators::OP_SetElementI(destGlobalObject, index, value, scriptContext);
            }
        }

        this->MapLetConstGlobals([destGlobalObject](const PropertyRecord* propertyRecord, Var value, bool isConst)
        {
            if (isConst)
            {
                JavascriptOperators::OP_InitConstProperty(destGlobalObject, propertyRecord->GetPropertyId(), value);
            }
            else
            {
                JavascriptOperators::OP_InitLetProperty(destGlobalObject, propertyRecord->GetPropertyId(), value);
            }
        });

        return destGlobalObject;
    }

}
