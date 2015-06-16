//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "BackEndAPI.h"
extern "C" PVOID _ReturnAddress(VOID);
#pragma intrinsic(_ReturnAddress)
#ifdef F_JSETW
#include <IERESP_mshtml.h>
#endif

#ifdef _M_IX86
#ifdef _CONTROL_FLOW_GUARD
extern "C" PVOID __guard_check_icall_fptr;
#endif
#endif

extern "C" void __cdecl _alloca_probe_16();
namespace Js
{
    DEFINE_RECYCLER_TRACKER_PERF_COUNTER(JavascriptFunction);
    JavascriptFunction::JavascriptFunction(DynamicType * type)
        : DynamicObject(type), functionInfo(null), constructorCache(&ConstructorCache::DefaultInstance)
    {
        Assert(this->constructorCache != null);
    }


    JavascriptFunction::JavascriptFunction(DynamicType * type, FunctionInfo * functionInfo)
        : DynamicObject(type), functionInfo(functionInfo), constructorCache(&ConstructorCache::DefaultInstance)

    {
        Assert(this->constructorCache != null);
        this->GetTypeHandler()->ClearHasOnlyWritableDataProperties(); // length is non-writable
        if (GetTypeHandler()->GetFlags() & DynamicTypeHandler::IsPrototypeFlag)
        {
            // No need to invalidate store field caches for non-writable properties here.  Since this type is just being created, it cannot represent
            // an object that is already a prototype.  If it becomes a prototype and then we attempt to add a property to an object dervied from this
            // object, then we will check if this property is writable, and only if it is will we do the fast path for add property.
            // GetScriptContext()->InvalidateStoreFieldCaches(PropertyIds::length);
            GetLibrary()->NoPrototypeChainsAreEnsuredToHaveOnlyWritableDataProperties();
        }
    }

    JavascriptFunction::JavascriptFunction(DynamicType * type, FunctionInfo * functionInfo, ConstructorCache* cache)
        : DynamicObject(type), functionInfo(functionInfo), constructorCache(cache)

    {
        Assert(this->constructorCache != null);
        this->GetTypeHandler()->ClearHasOnlyWritableDataProperties(); // length is non-writable
        if (GetTypeHandler()->GetFlags() & DynamicTypeHandler::IsPrototypeFlag)
        {
            // No need to invalidate store field caches for non-writable properties here.  Since this type is just being created, it cannot represent
            // an object that is already a prototype.  If it becomes a prototype and then we attempt to add a property to an object dervied from this
            // object, then we will check if this property is writable, and only if it is will we do the fast path for add property.
            // GetScriptContext()->InvalidateStoreFieldCaches(PropertyIds::length);
            GetLibrary()->NoPrototypeChainsAreEnsuredToHaveOnlyWritableDataProperties();
        }
    }

    static wchar_t const funcName[] = L"function anonymous";
    static wchar_t const genFuncName[] = L"function* anonymous";
    static wchar_t const bracket[] = L" {\012";

    Var JavascriptFunction::NewInstanceHelper(ScriptContext *scriptContext, RecyclableObject* function, CallInfo callInfo, Js::ArgumentReader& args, bool isGenerator /* = false */)
    {
        JavascriptLibrary* library = function->GetLibrary();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");

        // SkipDefaultNewObject function flag should have revent the default object
        // being created, except when call true a host dispatch
        Assert(!(callInfo.Flags & CallFlags_New) || args[0] == null
            || JavascriptOperators::GetTypeId(args[0]) == TypeIds_HostDispatch);

        // Legacy engine uses comma with a space, while ES5 spec specifies comma to be used.
        JavascriptString* separator = library->GetCommaDisplayString();

        // Gather all the formals into a string like (fml1, fml2, fml3)
        JavascriptString *fmls = library->CreateStringFromCppLiteral(L"(");
        for (uint i = 1; i < args.Info.Count - 1; ++i)
        {
            if (i != 1)
            {
                fmls = JavascriptString::Concat(fmls, separator);
            }
            fmls = JavascriptString::Concat(fmls, JavascriptConversion::ToString(args.Values[i], scriptContext));
        }
        fmls = JavascriptString::Concat(fmls, library->CreateStringFromCppLiteral(L")"));

        // Function body, last argument to Function(...)
        JavascriptString *fnBody = NULL;
        if (args.Info.Count > 1)
        {
            fnBody = JavascriptConversion::ToString(args.Values[args.Info.Count - 1], scriptContext);
        }

        // Create a string representing the anonymous function
        Assert(CountNewlines(funcName) + CountNewlines(bracket) == numberLinesPrependedToAnonymousFunction); // Be sure to add exactly one line to anonymous function

        JavascriptString *bs = isGenerator ?
            library->CreateStringFromCppLiteral(genFuncName) :
            library->CreateStringFromCppLiteral(funcName);
        bs = JavascriptString::Concat(bs, fmls);
        bs = JavascriptString::Concat(bs, library->CreateStringFromCppLiteral(bracket));
        if (fnBody != NULL)
        {
            bs = JavascriptString::Concat(bs, fnBody);
        }

        bs = JavascriptString::Concat(bs, library->CreateStringFromCppLiteral(L"\012}"));

        // Bug 1105479. Get the module id from the caller
        ModuleID moduleID = kmodGlobal;
       
        // In IE9 mode, it should always be evaluated in the global context
        //

        // TODO: Enable strictMode for new Function
        BOOL strictMode = FALSE;

        JavascriptFunction *pfuncScript;
        ParseableFunctionInfo *pfuncBodyCache = NULL;
        wchar_t const * sourceString = bs->GetSz();
        charcount_t sourceLen = bs->GetLength();
        EvalMapString key(sourceString, sourceLen, moduleID, strictMode, /* isLibraryCode = */ false);
        if (!scriptContext->IsInNewFunctionMap(key, &pfuncBodyCache))
        {
            // ES3 and ES5 specs require validation of the formal list and the function body            
            
            // Validate fmls here....
            scriptContext->GetGlobalObject()->ValidateSyntax(scriptContext, fmls->GetSz(), fmls->GetLength(), isGenerator, &Parser::ValidateFormals);
            if (fnBody != NULL)
            {
                // Validate function body
                scriptContext->GetGlobalObject()->ValidateSyntax(scriptContext, fnBody->GetSz(), fnBody->GetLength(), isGenerator, &Parser::ValidateSourceElementList);
            }

            pfuncScript = scriptContext->GetGlobalObject()->EvalHelper(scriptContext, sourceString, sourceLen, moduleID, fscrNil, Constants::FunctionCode, TRUE, TRUE, strictMode);

            // Indicate that this is a top-level function. We don't pass the fscrGlobalCode flag to the eval helper,
            // or it will return the global function that wraps the declared function body, as though it were an eval.
            // But we want, for instance, to be able to verify that we did the right amount of deferred parsing.
            ParseableFunctionInfo *functionInfo = pfuncScript->GetParseableFunctionInfo();
            Assert(functionInfo);
            functionInfo->SetGrfscr(functionInfo->GetGrfscr() | fscrGlobalCode);

            scriptContext->AddToNewFunctionMap(key, functionInfo);
        }
        else
        {
            // Get the latest proxy
            FunctionProxy * proxy = pfuncBodyCache->GetFunctionProxy();
            if (proxy->IsGenerator())
            {
                pfuncScript = scriptContext->GetLibrary()->CreateGeneratorVirtualScriptFunction(proxy);
            }
            else
            {
                pfuncScript = scriptContext->GetLibrary()->CreateScriptFunction(proxy);
            }
        }

        if (BinaryFeatureControl::LanguageService() && scriptContext->authoringData && scriptContext->authoringData->Callbacks())
            scriptContext->authoringData->Callbacks()->Executing();

        JSETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_FUNCTION(pfuncScript, EtwTrace::GetFunctionId(pfuncScript->GetFunctionProxy())));

        if (isGenerator)
        {
            Assert(pfuncScript->GetFunctionInfo()->IsGenerator());
            auto pfuncVirt = static_cast<GeneratorVirtualScriptFunction*>(pfuncScript);
            auto pfuncGen = scriptContext->GetLibrary()->CreateGeneratorFunction(JavascriptGeneratorFunction::EntryGeneratorFunctionImplementation, pfuncVirt);
            pfuncVirt->SetRealGeneratorFunction(pfuncGen);
            pfuncScript = pfuncGen;
        }

        return pfuncScript;
    }

    Var JavascriptFunction::NewInstanceRestrictedMode(RecyclableObject* function, CallInfo callInfo, ...)
    {
        ScriptContext* scriptContext = function->GetScriptContext();

        scriptContext->CheckEvalRestriction();

        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);

        return NewInstanceHelper(scriptContext, function, callInfo, args);
    }

    Var JavascriptFunction::NewInstance(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        return NewInstanceHelper(scriptContext, function, callInfo, args);
    }

    //
    // Dummy EntryPoint for Function.prototype
    //
    Var JavascriptFunction::PrototypeEntryPoint(RecyclableObject* function, CallInfo callInfo, ...)
    {
        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();
        JavascriptLibrary* library = function->GetLibrary();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");

        if (callInfo.Flags & CallFlags_New)
        {
            JavascriptError::ThrowTypeError(scriptContext, VBSERR_ActionNotSupported);
        }

        return library->GetUndefined();
    }

    BOOL JavascriptFunction::IsThrowTypeErrorFunction(JavascriptFunction* function, ScriptContext* scriptContext)
    {
        Assert(function);
        Assert(scriptContext);

        return
            function == scriptContext->GetLibrary()->GetThrowTypeErrorAccessorFunction() ||
            function == scriptContext->GetLibrary()->GetThrowTypeErrorCalleeAccessorFunction() ||
            function == scriptContext->GetLibrary()->GetThrowTypeErrorCallerAccessorFunction() ||
            function == scriptContext->GetLibrary()->GetThrowTypeErrorArgumentsAccessorFunction();
    }

    enum : unsigned { STACK_ARGS_ALLOCA_THRESHOLD = 8 }; // Number of stack args we allow before using _alloca

    // ES5 15.3.4.3
    //When the apply method is called on an object func with arguments thisArg and argArray the following steps are taken:
    //    1.    If IsCallable(func) is false, then throw a TypeError exception.
    //    2.    If argArray is null or undefined, then
    //          a.      Return the result of calling the [[Call]] internal method of func, providing thisArg as the this value and an empty list of arguments.
    //    3.    If Type(argArray) is not Object, then throw a TypeError exception.
    //    4.    Let len be the result of calling the [[Get]] internal method of argArray with argument "length".
    //
    //    Steps 5 and 7 deleted from July 19 Errata of ES5 spec
    //
    //    5.    If len is null or undefined, then throw a TypeError exception.
    //    6.    Len n be ToUint32(len).
    //    7.    If n is not equal to ToNumber(len), then throw a TypeError exception.
    //    8.    Let argList  be an empty List.
    //    9.    Let index be 0.
    //    10.   Repeat while index < n
    //          a.      Let indexName be ToString(index).
    //          b.      Let nextArg be the result of calling the [[Get]] internal method of argArray with indexName as the argument.
    //          c.      Append nextArg as the last element of argList.
    //          d.      Set index to index + 1.
    //    11.   Return the result of calling the [[Call]] internal method of func, providing thisArg as the this value and argList as the list of arguments.
    //    The length property of the apply method is 2.

    // TODO:  this implementation follows V5.8. there are several differences between V5.8, ES3 and ES5 to be addressed here:
    // 1. in V5.8 and ES3, if no 'this' argument is passed, the global object is used for 'this'.
    //      Passing global object could pose a security risk. It allows a quick access to the global object for code that is intended to be denied access to the global object.
    //      ES5 removes the automatically provided 'this'. Make sure to apply ES5 spec to both Function.prototype.apply and Function.prototype.call()
    // 2. V5.8 and ES3 allows only arrays and Arguments object as 'argArray'.  ES5 allows any array like object, i.e. any object that has a length.
    //      Question: what about external objects that have a 'length' property?
    // 3. ES3 has no restriction on argArray.length. V5.8 check the length to be a non negative value and limited by 'LONG_MAX/sizeof(VARIANT)'. ES5 has clause 7,
    //    which restricts the length to be a 32 unsigned value.
    Var JavascriptFunction::EntryApply(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        // Ideally, we want to maintain CallFlags_Eval behavior and pass along the extra FrameDisplay parameter
        // but that we would be a bigger change than what we want to do in this ship cycle. See WIN8: 915315.
        // If eval is executed using apply it will not get the frame display and always execute in global scope.
        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        Assert(!(callInfo.Flags & CallFlags_New));

        ///
        /// Check Argument[0] has internal [[Call]] property
        /// If not, throw TypeError
        ///
        if (args.Info.Count == 0 || !JavascriptConversion::IsCallable(args[0]))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedFunction, L"Function.prototype.apply");
        }

        Var thisVar = NULL;
        Var argArray = NULL;
        RecyclableObject* pFunc = RecyclableObject::FromVar(args[0]);

        if (args.Info.Count == 1)
        {
            thisVar = scriptContext->GetLibrary()->GetUndefined();
        }
        else if (args.Info.Count == 2)
        {
            thisVar = args.Values[1];
        }
        else if (args.Info.Count > 2)
        {
            thisVar = args.Values[1];
            argArray = args.Values[2];
        }

        return CalloutHelper<false>(pFunc, thisVar, argArray, scriptContext);
    }

    template <bool isConstruct>
    Var JavascriptFunction::CalloutHelper(RecyclableObject* pFunc, Var thisVar, Var argArray, ScriptContext* scriptContext)
    {
        CallFlags callFlag;
        if (isConstruct)
        {
            callFlag = CallFlags_New;
        }
        else
        {
            callFlag = CallFlags_Value;
        }
        Arguments outArgs(CallInfo(callFlag, 0), nullptr);

        Var stackArgs[STACK_ARGS_ALLOCA_THRESHOLD];

        if (nullptr == argArray)
        {
            outArgs.Info.Count = 1;
            outArgs.Values = &thisVar;
        }
        else
        {
            bool isArray = JavascriptArray::Is(argArray);
            TypeId typeId = JavascriptOperators::GetTypeId(argArray);
            bool isNullOrUndefined = (typeId == TypeIds_Null || typeId == TypeIds_Undefined);

            if (!isNullOrUndefined && !JavascriptOperators::IsObject(argArray)) // ES5: throw if Type(argArray) is not Object
            {
                JavascriptError::ThrowTypeError(scriptContext, JSERR_FunctionArgument_NeedObject, L"Function.prototype.apply");
            }

            // argArray is array or object, check length prop for object case
            // TODO v5.8 (ES3) mode: add  check for 'arguments' object when 'arguments' obj identity is available
            //
            // TODO v5.8 mode:  enforce the argArray to be an internal JScript object or a an object array. V5.8 doesn't allow external object to be passed as arg arrays.
            // HostDisptach objects that wrap Jscript objects or JScript arrays should be looked into because V5.8 doesn't use wrapping.
            // HostDisptach objects that wrap true external objects should be rejected.

            int64 len;
            JavascriptArray* arr = NULL;
            RecyclableObject* dynamicObject = RecyclableObject::FromVar(argArray);

            if (isNullOrUndefined)
            {
                len = 0;
            }
            else if (isArray)
            {
                JavascriptLibrary::CheckAndConvertCopyOnAccessNativeIntArray<Var>(argArray);
                arr = JavascriptArray::FromVar(argArray);
                len = arr->GetLength();
            }
            else
            {
                Var lenProp = JavascriptOperators::OP_GetLength(dynamicObject, scriptContext);
                len = JavascriptConversion::ToLength(lenProp, scriptContext);
            }

            if (len >= CallInfo::kMaxCountArgs)
            {
                JavascriptError::ThrowRangeError(scriptContext, JSERR_ArgListTooLarge);                
            }

            outArgs.Info.Count = (uint)len + 1;
            if (len == 0)
            {
                outArgs.Values = &thisVar;
            }
            else
            {
                if (outArgs.Info.Count > STACK_ARGS_ALLOCA_THRESHOLD)
                {
                    PROBE_STACK(scriptContext, outArgs.Info.Count * sizeof(Var)+Js::Constants::MinStackDefault); // args + function call
                    outArgs.Values = (Var*)_alloca(outArgs.Info.Count * sizeof(Var));
                }
                else
                {
                    outArgs.Values = stackArgs;
                }
                outArgs.Values[0] = thisVar;


                Var undefined = pFunc->GetLibrary()->GetUndefined();
                if (isArray && arr->GetScriptContext() == scriptContext)
                {
                    arr->ForEachItemInRange<false>(0, (uint)len, undefined, scriptContext,
                        [&outArgs](uint index, Var element)
                    {
                        outArgs.Values[index + 1] = element;
                    });
                }
                else
                {
                    for (uint i = 0; i < len; i++)
                    {
                        Var element;
                        if (!JavascriptOperators::GetItem(dynamicObject, i, &element, scriptContext))
                        {
                            element = undefined;
                        }
                        outArgs.Values[i + 1] = element;
                    }
                }
            }
        }

        if (isConstruct)
        {
            return JavascriptFunction::CallAsConstructor(pFunc, outArgs, scriptContext);
        }
        else
        {
            return JavascriptFunction::CallFunction<true>(pFunc, pFunc->GetEntryPoint(), outArgs);
        }
    }

    Var JavascriptFunction::ApplyHelper(RecyclableObject* function, Var thisArg, Var argArray, ScriptContext* scriptContext)
    {
        return CalloutHelper<false>(function, thisArg, argArray, scriptContext);
    }

    Var JavascriptFunction::ConstructHelper(RecyclableObject* function, Var thisArg, Var argArray, ScriptContext* scriptContext)
    {
        return CalloutHelper<true>(function, thisArg, argArray, scriptContext);
    }


    Var JavascriptFunction::EntryBind(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        CHAKRATEL_LANGSTATS_INC_BUILTINCOUNT(FunctionBindCount);

        Assert(!(callInfo.Flags & CallFlags_New));

        ///
        /// Check Argument[0] has internal [[Call]] property
        /// If not, throw TypeError
        ///
        if (args.Info.Count == 0 || !JavascriptConversion::IsCallable(args[0]))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedFunction, L"Function.prototype.bind");
        }

        BoundFunction* boundFunc = BoundFunction::New(scriptContext, args);

        return boundFunc;
    }

    Var JavascriptFunction::EntryToMethod(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        Assert(!(callInfo.Flags & CallFlags_New));

        if (args.Info.Count <= 1 || !RecyclableObject::Is(args[1]) || !JavascriptOperators::IsObject(RecyclableObject::FromVar(args[1])))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_FunctionArgument_NeedObject, L"Function.prototype.toMethod");
        }

        Var newHome = RecyclableObject::FromVar(args[1]);
        JavascriptFunction* newMethod = nullptr;

        if (!JavascriptFunction::Is(args[0]))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedFunction, L"Function.prototype.toMethod");
        }
        
        if (!JavascriptFunction::FromVar(args[0])->CloneMethod(&newMethod, newHome))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedCloneMethod, L"Function.prototype.toMethod");
        }

        return newMethod;
    }

    // ES5 15.3.4.4
    // Function.prototype.call (thisArg [ , arg1 [ , arg2, ... ] ] )
    //    When the call method is called on an object func with argument thisArg and optional arguments arg1, arg2 etc, the following steps are taken:
    //    1.    If IsCallable(func) is false, then throw a TypeError exception.
    //    2.    Let argList be an empty List.
    //    3.    If this method was called with more than one argument then in left to right order starting with arg1 append each argument as the last element of argList
    //    4.    Return the result of calling the [[Call]] internal method of func, providing thisArg as the this value and argList as the list of arguments.
    //    The length property of the call method is 1.

    Var JavascriptFunction::EntryCall(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        RUNTIME_ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        Assert(!(callInfo.Flags & CallFlags_New));

        ///
        /// Check Argument[0] has internal [[Call]] property
        /// If not, throw TypeError
        ///
        if (args.Info.Count == 0 || !JavascriptConversion::IsCallable(args[0]))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedFunction, L"Function.prototype.call");
        }

        RecyclableObject *pFunc = RecyclableObject::FromVar(args[0]);
        if (args.Info.Count == 1)
        {
            args.Values[0] = scriptContext->GetLibrary()->GetUndefined();
        }
        else
        {
            ///
            /// Remove function object from the arguments and pass the rest
            ///
            for (uint i = 0; i < args.Info.Count - 1; ++i)
            {
                args.Values[i] = args.Values[i + 1];
            }
            args.Info.Count = args.Info.Count - 1;
        }

        ///
        /// Call the [[Call]] method on the function object
        ///
        return JavascriptFunction::CallFunction<true>(pFunc, pFunc->GetEntryPoint(), args);
    }

    Var JavascriptFunction::CallRootFunctionInScript(JavascriptFunction* func, Arguments args)
    {
        ScriptContext* scriptContext = func->GetScriptContext();
        if (scriptContext->GetThreadContext()->HasPreviousHostScriptContext())
        {
            ScriptContext* requestContext = scriptContext->GetThreadContext()->GetPreviousHostScriptContext()->GetScriptContext();
            func = JavascriptFunction::FromVar(CrossSite::MarshalVar(requestContext, func));
        }
        return JavascriptFunction::CallFunction<true>(func, func->GetEntryPoint(), args);
    }
    Var JavascriptFunction::CallRootFunction(Arguments args, ScriptContext * scriptContext)
    {

#ifdef _M_X64
        Var ret = null;
        __try
        {
            ret = CallRootFunctionInternal(args, scriptContext);
        }        
        __except (ResumeForOutOfBoundsAsmJSArrayRefs(GetExceptionCode(), GetExceptionInformation()))
        {
            // should never reach here
            Assert(false);
        }
        //ret should never be null here
        Assert(ret);
        return ret;
#else
        return CallRootFunctionInternal(args, scriptContext);
#endif
    }
    Var JavascriptFunction::CallRootFunctionInternal(Arguments args, ScriptContext * scriptContext)
    {
#if DBG
        if (IsInAssert != 0)
        {
            // Just don't execute anything if we are in an assert
            // throw the exception directly to avoid additional assert in Js::Throw::InternalError
            throw Js::InternalErrorException();
        }
#endif
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        Js::Var varThis;
        if (PHASE_FORCE1(Js::EvalCompilePhase) && args.Info.Count == 0)
        {
            varThis = JavascriptOperators::OP_GetThis(scriptContext->GetLibrary()->GetUndefined(), kmodGlobal, scriptContext);
            args.Info.Flags = (Js::CallFlags)(args.Info.Flags | CallFlags_Eval);
            args.Info.Count = 1;
            args.Values = &varThis;
        }
#endif

        // work around overzealous C4701 warning
        Var varResult = null;
        ThreadContext *threadContext;
        threadContext = scriptContext->GetThreadContext();

        JavascriptExceptionObject* pExceptionObject = NULL;
        bool hasCaller = scriptContext->GetHostScriptContext() ? !!scriptContext->GetHostScriptContext()->HasCaller() : false;
        Assert(scriptContext == GetScriptContext());
        BEGIN_JS_RUNTIME_CALLROOT_EX(scriptContext, hasCaller)
        {
            scriptContext->VerifyAlive(true);
            try
            {
                varResult =
                    args.Info.Flags & CallFlags_New ?
                    CallAsConstructor(this, args, scriptContext) :
                    CallFunction<true>(this, this->GetEntryPoint(), args);

                // Win8 976001: Temporary work around for post-Dev11 compiler bug 470304.
                if (threadContext == NULL)
                {
                    throw (JavascriptExceptionObject*)NULL;
                }
            }
            catch (JavascriptExceptionObject* exceptionObject)
            {
                pExceptionObject = exceptionObject;
            }

            if (pExceptionObject)
            {
                pExceptionObject = pExceptionObject->CloneIfStaticExceptionObject(scriptContext);
                throw pExceptionObject;
            }
        }
        END_JS_RUNTIME_CALL(scriptContext);

        Assert(varResult != null);
        return varResult;
    }

#if DBG
    /*static*/
    void JavascriptFunction::CheckValidDebugThunk(ScriptContext* scriptContext, RecyclableObject *function)
    {
        Assert(scriptContext != nullptr);
        Assert(function != nullptr);

        if (scriptContext->IsInDebugMode()
            && !scriptContext->IsInterpreted() && !CONFIG_FLAG(ForceDiagnosticsMode)    // Does not work nicely if we change the default settings.
            && function->GetEntryPoint() != scriptContext->CurrentThunk
            && function->GetEntryPoint() != scriptContext->CurrentCrossSiteThunk
            && JavascriptFunction::Is(function))
        {

            JavascriptFunction *jsFunction = JavascriptFunction::FromVar(function);
            if (!jsFunction->IsBoundFunction()
                && !jsFunction->GetFunctionInfo()->IsDeferred()
                && (jsFunction->GetFunctionInfo()->GetAttributes() & FunctionInfo::DoNotProfile) != FunctionInfo::DoNotProfile
                && jsFunction->GetFunctionInfo() != &JavascriptExternalFunction::EntryInfo::WrappedFunctionThunk)
            {
                Js::FunctionProxy *proxy = jsFunction->GetFunctionProxy();
                if (proxy)          // Need to look at this one again, why this is null.
                {
                    AssertMsg(proxy->HasValidEntryPoint(), "Function does not have valid entrypoint");
                }
            }
        }
    }
#endif

    Var JavascriptFunction::CallAsConstructor(Var v, Arguments args, ScriptContext* scriptContext, const Js::AuxArray<uint32> *spreadIndices)
    {
        Assert(v);
        Assert(args.Info.Flags & CallFlags_New);
        Assert(scriptContext);

        if (JavascriptProxy::Is(v))
        {
            JavascriptProxy* proxy = JavascriptProxy::FromVar(v);
            return proxy->ConstructorTrap(args, scriptContext, spreadIndices);
        }

        // Create the empty object if necessary:
        // - Built-in constructor functions will return a new object of a specific type, so a new empty object does not need to
        //   be created
        // - For user-defined constructor functions, an empty object is created with the function's prototype
        Var resultObject = JavascriptOperators::NewScObjectNoCtor(v, scriptContext);

        // JavascriptOperators::NewScObject should have thrown if 'v' is not a constructor
        RecyclableObject* functionObj = RecyclableObject::FromVar(v);

#if DBG
        if (scriptContext->IsInDebugMode())
        {
            CheckValidDebugThunk(scriptContext, functionObj);
        }
#endif

        // Call the constructor function:
        // - Pass in the new empty object as the 'this' parameter. This can be null if an empty object was not created.
        args.Values[0] = resultObject;
        Var functionResult;
        if (spreadIndices != nullptr)
        {
            functionResult = CallSpreadFunction(functionObj, functionObj->GetEntryPoint(), args, spreadIndices);
        }
        else
        {
            functionResult = CallFunction<true>(functionObj, functionObj->GetEntryPoint(), args);
        }

        return
            FinishConstructor(
                functionResult,
                resultObject,
                JavascriptFunction::Is(functionObj) && functionObj->GetScriptContext() == scriptContext ?
                JavascriptFunction::FromVar(functionObj) :
                null);
    }

    Var JavascriptFunction::FinishConstructor(
        const Var constructorReturnValue,
        Var newObject,
        JavascriptFunction *const function)
    {
        Assert(constructorReturnValue);

        // TODO (jedmiad): Consider using constructorCache->ctorHasNoExplicitReturnValue to speed up this interpreter code path.
        if (JavascriptOperators::IsObject(constructorReturnValue))
        {
            newObject = constructorReturnValue;
        }

        if (function && function->GetConstructorCache()->NeedsUpdateAfterCtor())
        {
            JavascriptOperators::UpdateNewScObjectCache(function, newObject, function->GetScriptContext());
        }

        return newObject;
    }

    BOOL JavascriptFunction::IsConstructor(Var obj)
    {
        if (!JavascriptFunction::Is(obj))
        {
            return false;
        }

        return JavascriptFunction::FromVar(obj)->IsConstructor();
    }

    Var JavascriptFunction::EntrySpreadCall(const Js::AuxArray<uint32> *spreadIndices, RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        RUNTIME_ARGUMENTS(args, callInfo);

        return JavascriptFunction::CallSpreadFunction(function, function->GetEntryPoint(), args, spreadIndices);
    }

    uint32 JavascriptFunction::GetSpreadSize(const Arguments args, const Js::AuxArray<uint32> *spreadIndices, ScriptContext *scriptContext)
    {
        // Work out the expanded number of arguments.
        uint32 totalLength = args.Info.Count - spreadIndices->count;
        ::Math::RecordOverflowPolicy overflow;
        for (unsigned i = 0; i < spreadIndices->count; ++i)
        {
            uint32 elementLength = JavascriptArray::GetSpreadArgLen(args[spreadIndices->elements[i]], scriptContext);
            totalLength = UInt32Math::Add(totalLength, elementLength, overflow);
        }

        if (totalLength >= CallInfo::kMaxCountArgs || overflow.HasOverflowed())
        {
            JavascriptError::ThrowRangeError(scriptContext, JSERR_ArgListTooLarge);
        }

        return totalLength;
    }

    void JavascriptFunction::SpreadArgs(const Arguments args, Arguments& destArgs, const Js::AuxArray<uint32> *spreadIndices, ScriptContext *scriptContext)
    {
        Assert(args.Values != nullptr);
        Assert(destArgs.Values != nullptr);

        CallInfo callInfo = args.Info;
        size_t destArgsByteSize = destArgs.Info.Count * sizeof(Var);

        destArgs.Values[0] = args[0];

        // Iterate over the arguments, spreading inline. We skip 'this'.
        Var undefined = scriptContext->GetLibrary()->GetUndefined();
        
        for (unsigned i = 1, argsIndex = 1, spreadArgIndex = 0; i < callInfo.Count; ++i)
        {
            size_t spreadIndex = spreadIndices->elements[spreadArgIndex]; // Next index to be spread.
            if (i < spreadIndex)
            {
                // Copy everything until the next spread index.
                js_memcpy_s(destArgs.Values + argsIndex,
                            destArgsByteSize - (argsIndex * sizeof(Var)),
                            args.Values + i,
                            (spreadIndex - i) * sizeof(Var));
                argsIndex += spreadIndex - i;
                i = spreadIndex - 1;
                continue;
            }
            else if (i > spreadIndex)
            {
                // Copy everything after the last spread index.
                js_memcpy_s(destArgs.Values + argsIndex,
                            destArgsByteSize - (argsIndex * sizeof(Var)),
                            args.Values + i,
                            (args.Info.Count - i) * sizeof(Var));
                break;
            }
            else
            {
                // Expand the spread element.
                Var instance = args[spreadIndex];

                if (SpreadArgument::Is(instance))
                {
                    SpreadArgument* spreadedArgs = SpreadArgument::FromVar(instance);
                    uint size = spreadedArgs->GetArgumentSpreadCount();
                    const Var * spreadBuffer = spreadedArgs->GetArgumentSpread();
                    js_memcpy_s(destArgs.Values + argsIndex,
                        size * sizeof(Var),
                        spreadBuffer,
                        size * sizeof(Var));
                    argsIndex += size;
                }
                else
                {
                    // We first try to interpret the spread parameter as an array.
                    JavascriptArray *arr = nullptr;
                    if (JavascriptArray::Is(instance))
                    {
                        arr = JavascriptArray::FromVar(instance);
                    }

                    if (arr != nullptr && !arr->IsCrossSiteObject())
                    {
                        // TODO (tcare): Optimize by creating a JavascriptArray routine which allows
                        // memcpy-like semantics in optimal situations (no gaps, etc.)
                        for (uint32 j = 0; j < arr->GetLength(); j++)
                        {
                            Var element;
                            if (!arr->DirectGetItemAtFull(j, &element))
                            {
                                element = undefined;
                            }
#pragma warning(push)
#pragma warning(suppress: 26014) // SAL gets confused here when we use the stack allocated memory and thinks we might overflow.
                            destArgs.Values[argsIndex++] = element;
                        }
                    }
                    else
                    {
                        // Try to use the spread argument as an object with a length property.
                        RecyclableObject *propertyObject;
                        if (!JavascriptOperators::GetPropertyObject(instance, scriptContext, &propertyObject))
                        {
                            JavascriptError::ThrowTypeError(scriptContext, JSERR_InvalidSpreadArgument);
                        }

                        uint32 len = JavascriptArray::GetSpreadArgLen(instance, scriptContext);

                        for (uint j = 0; j < len; j++)
                        {
                            Var element;
                            if (!JavascriptOperators::GetItem(instance, propertyObject, j, &element, scriptContext))
                            {
                                element = undefined;
                            }
#pragma warning(suppress: 26015) // SAL gets confused here when we use the stack allocated memory and thinks we might overflow.
                            destArgs.Values[argsIndex++] = element;
#pragma warning(pop)
                        }
                    }
                }
                
            if (spreadArgIndex < spreadIndices->count - 1)
            {
                spreadArgIndex++;
            }
            }
        }
    }

    Var JavascriptFunction::CallSpreadFunction(RecyclableObject* function, JavascriptMethod entryPoint, Arguments args, const Js::AuxArray<uint32> *spreadIndices)
    {
        CallInfo callInfo = args.Info;
        ScriptContext* scriptContext = function->GetScriptContext();

        // Work out the expanded number of arguments.
        uint32 actualLength = GetSpreadSize(args, spreadIndices, scriptContext);

        // Allocate (if needed) space for the expanded arguments.
        Arguments outArgs(CallInfo(args.Info.Flags, 0), nullptr);
        outArgs.Info.Count = actualLength;
        Var stackArgs[STACK_ARGS_ALLOCA_THRESHOLD];
        size_t outArgsSize = 0;
        if (outArgs.Info.Count > STACK_ARGS_ALLOCA_THRESHOLD)
        {
            PROBE_STACK(scriptContext, outArgs.Info.Count * sizeof(Var) + Js::Constants::MinStackDefault); // args + function call
            outArgsSize = outArgs.Info.Count * sizeof(Var);
            outArgs.Values = (Var*)_alloca(outArgsSize);
        }
        else
        {
            outArgs.Values = stackArgs;
            outArgsSize = STACK_ARGS_ALLOCA_THRESHOLD * sizeof(Var);
            ZeroMemory(outArgs.Values, outArgsSize); // We may not use all of the elements
        }

        SpreadArgs(args, outArgs, spreadIndices, scriptContext);
        
        return JavascriptFunction::CallFunction<true>(function, entryPoint, outArgs);
    }

    Var JavascriptFunction::CallFunction(Arguments args)
    {
        return JavascriptFunction::CallFunction<true>(this, this->GetEntryPoint(), args);
    }

    template Var JavascriptFunction::CallFunction<true>(RecyclableObject* function, JavascriptMethod entryPoint, Arguments args);
    template Var JavascriptFunction::CallFunction<false>(RecyclableObject* function, JavascriptMethod entryPoint, Arguments args);

#ifdef _M_IX86
    template <bool doStackProbe>
    Var JavascriptFunction::CallFunction(RecyclableObject* function, JavascriptMethod entryPoint, Arguments args)
    {
        Js::Var varResult;

#if DBG && defined(ENABLE_NATIVE_CODEGEN)
        CheckIsExecutable(function, entryPoint);
#endif
        // compute size of stack to reserve
        CallInfo callInfo = args.Info;
        uint argsSize = callInfo.Count * sizeof(Var);

        ScriptContext * scriptContext = function->GetScriptContext();

        if (doStackProbe)
        {
            PROBE_STACK_CALL(scriptContext, function, argsSize);
        }

        void *data;
        void *savedEsp;
        __asm
        {
            // Save ESP
            mov savedEsp, esp
            mov eax, argsSize
            // Make sure we don't go beyond guard page
            cmp eax, 0x1000
            jge alloca_probe
            sub esp, eax
            jmp dbl_align
alloca_probe:
            // Use alloca to allocate more then a page size
            // Alloca assumes eax, contains size, and adjust ESP while
            // probing each page.
            call _alloca_probe_16
dbl_align:
            // 8-byte align frame to improve floating point perf of our JIT'd code.
            and esp, -8

            mov data, esp
        }

        {

            Var* dest = (Var*)data;
            Var* src = args.Values;
            for(unsigned int i =0; i < callInfo.Count; i++)
            {
                dest[i] = src[i];
            }
        }

        // call variable argument function provided in entryPoint
        __asm
        {
#ifdef _CONTROL_FLOW_GUARD
            // verify that the call target is valid
            mov  ecx, entryPoint
            call [__guard_check_icall_fptr]
            ; no need to restore ecx ('call entryPoint' is a __cdecl call)
#endif

            push callInfo
            push function
            call entryPoint

            // Restore ESP
            mov esp, savedEsp

            // save the return value from realsum.
            mov varResult, eax;
        }

        return varResult;
    }

#elif _M_X64
    template <bool doStackProbe>
    Var JavascriptFunction::CallFunction(RecyclableObject *function, JavascriptMethod entryPoint, Arguments args)
    {
        // compute size of stack to reserve and make sure we have enough stack.
        CallInfo callInfo = args.Info;
        uint argsSize = callInfo.Count * sizeof(Var);
        if (doStackProbe == true)
        {
            PROBE_STACK_CALL(function->GetScriptContext(), function, argsSize);
        }
#if DBG && defined(ENABLE_NATIVE_CODEGEN)
        CheckIsExecutable(function, entryPoint);
#endif
#ifdef _CONTROL_FLOW_GUARD
        _guard_check_icall((uintptr_t) entryPoint); /* check function pointer integrity */
#endif
        return amd64_CallFunction(function, entryPoint, args.Info, args.Info.Count, &args.Values[0]);
    }
#elif defined(_M_ARM)
    extern "C"
    {
        extern Var arm_CallFunction(JavascriptFunction* function, CallInfo info, Var* values, JavascriptMethod entryPoint);
    }

    template <bool doStackProbe>
    Var JavascriptFunction::CallFunction(RecyclableObject* function, JavascriptMethod entryPoint, Arguments args)
    {
        // compute size of stack to reserve and make sure we have enough stack.
        CallInfo callInfo = args.Info;
        uint argsSize = callInfo.Count * sizeof(Var);
        if (doStackProbe)
        {
            PROBE_STACK_CALL(function->GetScriptContext(), function, argsSize);
        }

#if DBG && defined(ENABLE_NATIVE_CODEGEN)
        CheckIsExecutable(function, entryPoint);
#endif
        Js::Var varResult;

        //The ARM can pass 4 arguments via registers so handle the cases for 0 or 1 values without resorting to asm code
        //(so that the asm code can assume 0 or more values will go on the stack: putting -1 values on the stack is unhealthy).
        unsigned count = args.Info.Count;
        if (count == 0)
        {
            varResult = entryPoint((JavascriptFunction*)function, args.Info);
        }
        else if (count == 1)
        {
            varResult = entryPoint((JavascriptFunction*)function, args.Info, args.Values[0]);
        }
        else
        {
            varResult = arm_CallFunction((JavascriptFunction*)function, args.Info, args.Values, entryPoint);
        }

        return varResult;
    }
#elif defined(_M_ARM64)
    extern "C"
    {
        extern Var arm64_CallFunction(JavascriptFunction* function, CallInfo info, Var* values, JavascriptMethod entryPoint);
    }

    template <bool doStackProbe>
    Var JavascriptFunction::CallFunction(RecyclableObject* function, JavascriptMethod entryPoint, Arguments args)
    {
        // compute size of stack to reserve and make sure we have enough stack.
        CallInfo callInfo = args.Info;
        uint argsSize = callInfo.Count * sizeof(Var);
        if (doStackProbe)
        {
            PROBE_STACK_CALL(function->GetScriptContext(), function, argsSize);
        }

#if DBG && defined(ENABLE_NATIVE_CODEGEN)
        CheckIsExecutable(function, entryPoint);
#endif
        Js::Var varResult;

        varResult = arm64_CallFunction((JavascriptFunction*)function, args.Info, args.Values, entryPoint);

        return varResult;
    }
#else
    Var JavascriptFunction::CallFunction(RecyclableObject *function, JavascriptMethod entryPoint, Arguments args)
    {
#if DBG && defined(ENABLE_NATIVE_CODEGEN)
        CheckIsExecutable(function, entryPoint);
#endif
#if 1
        Js::Throw::NotImplemented();
        return null;
#else
        Var varResult;
        switch (info.Count)
        {
        case 0:
            {
                varResult=entryPoint((JavascriptFunction*)function, args.Info);
                break;
            }
        case 1: {
            varResult=entryPoint(
                (JavascriptFunction*)function,
                args.Info,
                args.Values[0]);
            break;
                }
        case 2: {
            varResult=entryPoint(
                (JavascriptFunction*)function,
                args.Info,
                args.Values[0],
                args.Values[1]);
            break;
                }
        case 3: {
            varResult=entryPoint(
                (JavascriptFunction*)function,
                args.Info,
                args.Values[0],
                args.Values[1],
                args.Values[2]);
            break;
                }
        case 4: {
            varResult=entryPoint(
                (JavascriptFunction*)function,
                args.Info,
                args.Values[0],
                args.Values[1],
                args.Values[2],
                args.Values[3]);
            break;
                }
        case 5: {
            varResult=entryPoint(
                (JavascriptFunction*)function,
                args.Info,
                args.Values[0],
                args.Values[1],
                args.Values[2],
                args.Values[3],
                args.Values[4]);
            break;
                }
        case 6: {
            varResult=entryPoint(
                (JavascriptFunction*)function,
                args.Info,
                args.Values[0],
                args.Values[1],
                args.Values[2],
                args.Values[3],
                args.Values[4],
                args.Values[5]);
            break;
                }
        case 7: {
            varResult=entryPoint(
                (JavascriptFunction*)function,
                args.Info,
                args.Values[0],
                args.Values[1],
                args.Values[2],
                args.Values[3],
                args.Values[4],
                args.Values[5],
                args.Values[6]);
            break;
                }
        case 8: {
            varResult=entryPoint(
                (JavascriptFunction*)function,
                args.Info,
                args.Values[0],
                args.Values[1],
                args.Values[2],
                args.Values[3],
                args.Values[4],
                args.Values[5],
                args.Values[6],
                args.Values[7]);
            break;
                }
        case 9: {
            varResult=entryPoint(
                (JavascriptFunction*)function,
                args.Info,
                args.Values[0],
                args.Values[1],
                args.Values[2],
                args.Values[3],
                args.Values[4],
                args.Values[5],
                args.Values[6],
                args.Values[7],
                args.Values[8]);
            break;
                }
        default:
            ScriptContext* scriptContext = function->type->GetScriptContext();
            varResult = scriptContext->GetLibrary()->GetUndefined();
            AssertMsg(false, "CallFunction call with unsupported number of arguments");
            break;
        }

#endif
        return varResult;
    }
#endif

    Var JavascriptFunction::EntryToString(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        Assert(!(callInfo.Flags & CallFlags_New));

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        if (args.Info.Count == 0 || !JavascriptFunction::Is(args[0]))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedFunction, L"Function.prototype.toString");
        }
        JavascriptFunction *pFunc = JavascriptFunction::FromVar(args[0]);

        // pFunc can be from a different script context if Function.prototype.toString is invoked via .call/.apply.
        // Marshal the resulting string to the current script context (that of the toString)
        return CrossSite::MarshalVar(scriptContext, pFunc->EnsureSourceString());
    }

    JavascriptString* JavascriptFunction::GetNativeFunctionDisplayString(ScriptContext *scriptContext, JavascriptString *name)
    {
        return GetNativeFunctionDisplayStringCommon<JavascriptString>(scriptContext, name);
    }

    JavascriptString* JavascriptFunction::GetLibraryCodeDisplayString(ScriptContext *scriptContext, PCWSTR displayName)
    {
        return GetLibraryCodeDisplayStringCommon<JavascriptString, JavascriptString*>(scriptContext, displayName);
    }

#ifdef _M_IX86
    // This code is enabled by the -checkAlignment switch.
    // It verifies that all of our JS frames are 8 byte aligned.
    // Our alignments is based on aligning the return address of the function.
    // Note that this test can fail when Javascript functions are called directly
    // from helper functions.  This could be fixed by making these calls through
    // CallFunction(), or by having the helper 8 byte align the frame itself before
    // the call.  A lot of these though are not dealing with floats, so the cost
    // of doing the 8 byte alignment would outweigh the benefit...
    __declspec (naked)
    void JavascriptFunction::CheckAlignment()
    {
        _asm
        {
            test esp, 0x4
            je   LABEL1
            ret
LABEL1:
            call Throw::InternalError
        }
    }
#else
    void JavascriptFunction::CheckAlignment()
    {
        // Note: in order to enable this on ARM, uncomment/fix code in LowerMD.cpp (LowerEntryInstr).
    }
#endif

#ifdef ENABLE_NATIVE_CODEGEN
    BOOL JavascriptFunction::IsNativeAddress(ScriptContext * scriptContext, void * codeAddr)
    {
        return scriptContext->IsNativeAddress(codeAddr);
    }
#endif

    // TODO: Move this to ScriptFunction
    Js::JavascriptMethod JavascriptFunction::DeferredParse(ScriptFunction** functionRef)
    {
        BOOL fParsed;
        return Js::ScriptFunction::DeferredParseCore(functionRef, fParsed);
    }

    // TODO: Move this to ScriptFunction
    Js::JavascriptMethod JavascriptFunction::DeferredParseCore(ScriptFunction** functionRef, BOOL &fParsed)
    {
        // Do the actual deferred parsing and byte code generation, passing the new entry point to the caller.

        ParseableFunctionInfo* functionInfo = (*functionRef)->GetParseableFunctionInfo();
        FunctionBody* funcBody = null;

        Assert(functionInfo);

        if (functionInfo->IsDeferredParseFunction())
        {
            funcBody = functionInfo->Parse(functionRef);
            fParsed = funcBody->IsFunctionParsed() ? TRUE : FALSE;

            // This is the first call to the function, ensure dynamic profile info
            funcBody->EnsureDynamicProfileInfo();
        }
        else
        {
            funcBody = functionInfo->GetFunctionBody();
            Assert(funcBody != null);
            Assert(!funcBody->IsDeferredParseFunction());
        }

        DebugOnly(JavascriptMethod directEntryPoint = funcBody->GetDirectEntryPoint(funcBody->GetDefaultEntryPointInfo()));
        Assert(directEntryPoint != DefaultDeferredParsingThunk && directEntryPoint != ProfileDeferredParsingThunk);
        return (*functionRef)->UpdateUndeferredBody(funcBody);
    }

    void JavascriptFunction::ReparseAsmJsModule(ScriptFunction** functionRef)
    {
        ParseableFunctionInfo* functionInfo = (*functionRef)->GetParseableFunctionInfo();

        Assert(functionInfo);
        Assert(functionInfo->HasBody());
        functionInfo->GetFunctionBody()->AddDeferParseAttribute();
        functionInfo->GetFunctionBody()->ResetEntryPoint();
        functionInfo->GetFunctionBody()->ResetInParams();

        FunctionBody * funcBody = functionInfo->Parse(functionRef);

        // This is the first call to the function, ensure dynamic profile info
        funcBody->EnsureDynamicProfileInfo();
        
        (*functionRef)->UpdateUndeferredBody(funcBody);
    }

    JavascriptFunction* JavascriptFunction::MakeCopyOnWriteObject(ScriptContext* scriptContext)
    {
        VERIFY_COPY_ON_WRITE_ENABLED_RET();

        typedef CopyOnWriteObject<JavascriptFunction, JavascriptFunctionSpecialProperties> CopyOnWriteJavascriptFunction;
        CopyOnWriteJavascriptFunction *result = RecyclerNew(scriptContext->GetRecycler(), CopyOnWriteJavascriptFunction,
            scriptContext->GetLibrary()->CreateFunctionType(this->GetEntryPoint(), RecyclableObject::FromVar(scriptContext->CopyOnWrite(this->GetPrototype()))), this, scriptContext);
        result->functionInfo = this->functionInfo;
        return result;
    }

    // Thunk for handling calls to functions that have not had byte code generated for them.

#if _M_IX86
    __declspec(naked)
    Var JavascriptFunction::DeferredParsingThunk(RecyclableObject* function, CallInfo callInfo, ...)
    {
        __asm
        {
            push ebp
            mov ebp, esp
            lea eax, [esp+8]                // load the address of the funciton os that if we need to box, we can patch it up
            push eax
            call JavascriptFunction::DeferredParse
            pop ebp
            jmp eax
        }
    }
#elif defined(_M_X64) || defined(_M_ARM32_OR_ARM64)
    //Do nothing: the implementation of JavascriptFunction::DeferredParsingThunk is declared (appropriately decorated) in
    // Library\amd64\javascriptfunctiona.asm
    // Library\arm\arm_DeferredParsingThunk.asm
    // Library\arm64\arm64_DeferredParsingThunk.asm
#else
    Var JavascriptFunction::DeferredParsingThunk(RecyclableObject* function, CallInfo callInfo, ...)
    {
        Js::Throw::NotImplemented();
        return null;
    }
#endif

    ConstructorCache* JavascriptFunction::EnsureValidConstructorCache()
    {
        Assert(this->constructorCache != null);
        this->constructorCache = ConstructorCache::EnsureValidInstance(this->constructorCache, this->GetScriptContext());
        return this->constructorCache;
    }

    void JavascriptFunction::ResetConstructorCacheToDefault()
    {
        Assert(this->constructorCache != null);

        if (!this->constructorCache->IsDefault())
        {
            this->constructorCache = &ConstructorCache::DefaultInstance;
        }
    }

    // Thunk for handling calls to functions that have not had byte code generated for them.

#if _M_IX86
    __declspec(naked)
    Var JavascriptFunction::DeferredDeserializeThunk(RecyclableObject* function, CallInfo callInfo, ...)
    {
        __asm
        {
            push ebp
            mov ebp, esp
            push [esp+8]
            call JavascriptFunction::DeferredDeserialize
            pop ebp
            jmp eax
        }
    }
#elif defined(_M_X64) || defined(_M_ARM32_OR_ARM64)
    //Do nothing: the implementation of JavascriptFunction::DeferredParsingThunk is declared (appropriately decorated) in
    // Library\amd64\javascriptfunctiona.asm
    // Library\arm\arm_DeferredParsingThunk.asm
    // Library\arm64\arm64_DeferredParsingThunk.asm
#else
    Var JavascriptFunction::DeferredDeserializeThunk(RecyclableObject* function, CallInfo callInfo, ...)
    {
        Js::Throw::NotImplemented();
        return null;
    }
#endif

    Js::JavascriptMethod JavascriptFunction::DeferredDeserialize(ScriptFunction* function)
    {
        FunctionInfo* funcInfo = function->GetFunctionInfo();
        Assert(funcInfo);
        FunctionBody* funcBody = null;

        // If we haven't already deserialized this function, do so now
        // FunctionProxies could have gotten deserialized during the interpreter when
        // we tried to record the callsite info for the function which meant that it was a
        // target of a call. Or we could have deserialized the function info in another JavascriptFunctionInstance
        // In any case, fix up the function info if it's already been deserialized so that
        // we don't hold on to the proxy for too long, and rethunk it so that it directly
        // calls the default entry point the next time around
        if (funcInfo->IsDeferredDeserializeFunction())
        {
            DeferDeserializeFunctionInfo* deferDeserializeFunction = (DeferDeserializeFunctionInfo*) funcInfo;

            // This is the first call to the function, ensure dynamic profile info
            // Deserialize is a no-op if the function has already been deserialized
            funcBody = deferDeserializeFunction->Deserialize();
            funcBody->EnsureDynamicProfileInfo();
        }
        else
        {
            funcBody = funcInfo->GetFunctionBody();
            Assert(funcBody != null);
            Assert(!funcBody->IsDeferredDeserializeFunction());
        }

        return function->UpdateUndeferredBody(funcBody);
    }
    void JavascriptFunction::SetEntryPoint(JavascriptMethod method)
    {
        this->GetDynamicType()->SetEntryPoint(method);
    }

    Var JavascriptFunction::EnsureSourceString()
    {
        return this->GetLibrary()->GetFunctionDisplayString();
    }

#if DBG_DUMP
    void JavascriptFunction::Dump()
    {
        //Note: This code has been commented since August 2010; if it becomes uncommented it can be decided at that time if the function body has to be deserialzied at this point.
        FunctionBody* pFuncBody = this->GetFunctionBody();
        if(pFuncBody)
        {
            //FunctionBody::SourceInfo* pSourceInfo = pFuncBody;
            //charcount_t cchLength = pSourceInfo->LengthInChars();
            //LPOLESTR pszBuffer = reinterpret_cast<LPOLESTR>(malloc((cchLength + 1) * sizeof(WCHAR)));
            //utf8::DecodeIntoAndNulTerminate(pszBuffer, pSourceInfo->GetSource(), cchLength);
            //Output::Print(L"%s ", pszBuffer);
            //free(pszBuffer);
        }
    }

#endif
    /*
    *****************************************************************************************************************
                                Conditions checked by instruction decoder (In sequential order)
    ******************************************************************************************************************
    1)	Exception Code is AV i.e STATUS_ACCESS_VIOLATION
    2)  Check if Rip is Native address
    3)	Get the function object from RBP (a fixed offset from RBP) and check for the following
        a.	Not Null
        b.  Ensure that the function object is heap allocated
        c.  Ensure that the entrypointInfo is heap allocated
        d.  Ensure that the functionbody is heap allocated
        e.	Is a function
        f.	Is AsmJs Function object for asmjs
    4)	Check if Rip is Native address
    5)	Check if Array BufferLength > 0x10000 (64K), power of 2 if length is less than 2^24 or multiple of 2^24  and multiple of 0x1000(4K) for asmjs
    6)	Check If the instruction is valid
        a.	Is one of the move instructions , i.e. mov, movsx, movzx, movsxd, movss or movsd               
        b.	Get the array buffer register and its value for asmjs
        c.	Get the dst register(in case of load)
        d.	Calculate the number of bytes read in order to get the length of the instruction , ensure that the length should never be greater than 15 bytes
    7)	Check that the Array buffer value is same as the one we passed in EntryPointInfo in asmjs
    8)	Set the dst reg if the instr type is load
    9)	Add the bytes read to Rip and set it as new Rip
    10)	Return EXCEPTION_CONTINUE_EXECUTION

    */
#ifdef _M_X64
    AsmJSInstructionDecoder::InstructionData AsmJSInstructionDecoder::CheckValidInstr(BYTE* &pc, PEXCEPTION_POINTERS exceptionInfo, FunctionBody* funcBody) // get the reg operand and isLoad and 
    {
        InstructionData instrData;
        uint prefixValue = 0;
        AsmJSInstructionDecoder::RexByteValue rexByteValue;
        bool isFloat = false;
        uint  immBytes = 0;
        uint dispBytes = 0;
        bool isImmediate = false;
        bool isSIB = false;
        // Read first  byte - check for prefix 
        BYTE* beginPc = pc;
        if (((*pc) == 0x0F2) || ((*pc) == 0x0F3))
        {
            //MOVSD or MOVSS                   
            prefixValue = *pc;
            isFloat = true;
            pc++;
        }
        else if (*pc == 0x66)
        {
            prefixValue = *pc;
            pc++;
        }

        // Check for Rex Byte - After prefix we should have a rexbyte if there is one 
        if (*pc >= 0x40 && *pc <= 0x4F)
        {            
            rexByteValue.rexValue = *pc;
            uint rexByte = *pc - 0x40;
            if (rexByte & 0x8)
            {
                rexByteValue.isW = true;
            }
            if (rexByte & 0x4)
            {
                rexByteValue.isR = true;
            }
            if (rexByte & 0x2)
            {
                rexByteValue.isX = true;
            }
            if (rexByte & 0x1)
            {
                rexByteValue.isB = true;
            }
            pc++;
        }

        // read opcode 
        //6a.	Is one of the move instructions , i.e. mov, movsx, movzx, movsxd, movss or movsd
        switch (*pc)
        {
        //MOV - Store
        case 0x89:
        case 0x88:
        {
                     pc++;
                     instrData.isLoad = false;
                     break;
        }
        //MOV - Load
        case 0x8A:
        case 0x8B:
        {
                     pc++;
                     instrData.isLoad = true;
                     break;
        }
        case 0x0F:
        {
                     // more than one byte opcode and hence we will read pc multiple times
                     pc++;
                     //MOVSX  , MOVSXD  
                     if (*pc == 0xBE || *pc == 0xBF)
                     {                                             
                         instrData.isLoad = true;
                     }
                     //MOVZX
                     else if (*pc == 0xB6 || *pc == 0xB7) 
                     {                         
                         instrData.isLoad = true;                         
                     }
                     //MOVSS - Load
                     else if (*pc == 0x10 && prefixValue == 0xF3)
                     {                         
                         Assert(isFloat);
                         instrData.isLoad = true;
                         instrData.isFloat32 = true;
                     }
                     //MOVSS - Store
                     else if (*pc == 0x11 && prefixValue == 0xF3)
                     {                         
                         Assert(isFloat);
                         instrData.isLoad = false;
                         instrData.isFloat32 = true;
                     }
                     //MOVSD - Load
                     else if (*pc == 0x10 && prefixValue == 0xF2)
                     {                         
                         Assert(isFloat);
                         instrData.isLoad = true;
                         instrData.isFloat64 = true;
                     }
                     //MOVSD - Store
                     else if (*pc == 0x11 && prefixValue == 0xF2)
                     {                         
                         Assert(isFloat);
                         instrData.isLoad = false;
                         instrData.isFloat64 = true;
                     }
                     else
                     {
                         instrData.isInvalidInstr = true;
                     }
                     pc++;
                     break;
        }
            //Support Mov Immediates
            // MOV 
        case 0xC6:
        case 0xC7:
        {
            instrData.isLoad = false;
            instrData.isFloat64 = false; 
            isImmediate = true;
            if (*pc == 0xC6)
            {
                immBytes = 1;
            }
            else if (rexByteValue.rexValue) // if REX is set then we have a 32 bit immediate value
            {
                immBytes = 4;
            }
            else
            {
                immBytes = 2;
            }
            pc++;
            break;
        }

        default:
            instrData.isInvalidInstr = true;
            break;
        }
        // if the opcode is not a move return
        if (instrData.isInvalidInstr)
        {
            return instrData;
        }

        //Read ModR/M
        // Read the Src Reg and also check for SIB 
        // Add the isR bit to SrcReg and get the actual SRCReg 
        // Get the number of bytes for displacement

        //get mod bits
        BYTE modVal = *pc & 0xC0; // first two bits(7th and 6th bits)
        modVal >>= 6;

        //get the R/M bits
        BYTE rmVal = (*pc) & 0x07; // last 3 bits ( 0,1 and 2nd bits)
        
        //get the reg value
        BYTE dstReg = (*pc) & 0x38; // mask reg bits (3rd 4th and 5th bits)
        dstReg >>= 3;

        Assert(dstReg <= 0x07);
        Assert(modVal <= 0x03);
        Assert(rmVal <= 0x07);

        switch (modVal)
        {
        case 0x00:
            dispBytes = 0;
            break;
        case 0x01:
            dispBytes = 1;
            break;
        case 0x02:
            dispBytes = 4;
            break;
        default:
            instrData.isInvalidInstr = true;
            break;
        }

        if (instrData.isInvalidInstr)
        {
            return instrData;
        }

        // Get the R/M value and see if SIB is present , else get the buffer reg
        if (rmVal == 0x04)
        {
            isSIB = true;
        }
        else
        {
            instrData.bufferReg = rmVal;
        }
        //Get the RegByes from ModRM 

        instrData.dstReg = dstReg;

        //increment the modrm byte 
        pc++;
        //Check if we have SIB and in that case bufferReg should not be set
        if (isSIB)
        {
            Assert(!instrData.bufferReg);
            // Get the Base and Index Reg from SIB and ensure that Scale is zero 
            // We dont care about the Index reg 
            // Add the isB value from Rex and get the actual Base Reg             
            //Get the base register        

            //6f. Get the array buffer register and its value
            instrData.bufferReg = (*pc % 8);
            pc++;
        }
        // check for the Rex.B value and append it to the base register
        if (rexByteValue.isB)
        {
            instrData.bufferReg |= 1 << 3;
        }
        // check for the Rex.R value and append it to the dst register
        if (rexByteValue.isR)
        {
            instrData.dstReg |= 1 << 3;
        }

        // Get the buffer address - this is always 64 bit GPR        
        switch (instrData.bufferReg)
        {
        case 0x0:
            instrData.bufferValue = exceptionInfo->ContextRecord->Rax;
            break;
        case 0x1:
            instrData.bufferValue = exceptionInfo->ContextRecord->Rcx;
            break;
        case 0x2:
            instrData.bufferValue = exceptionInfo->ContextRecord->Rdx;
            break;
        case 0x3:
            instrData.bufferValue = exceptionInfo->ContextRecord->Rbx;
            break;
        case 0x4:
            instrData.bufferValue = exceptionInfo->ContextRecord->Rsp;
            break;
        case 0x5:
            instrData.isInvalidInstr = true;
            Assert(false);// should never reach here as the check for disp is done above
            return instrData;
        case 0x6:
            instrData.bufferValue = exceptionInfo->ContextRecord->Rsi;
            break;
        case 0x7:
            instrData.bufferValue = exceptionInfo->ContextRecord->Rdi;
            break;
        case 0x8:
            instrData.bufferValue = exceptionInfo->ContextRecord->R8;
            break;
        case 0x9:
            instrData.bufferValue = exceptionInfo->ContextRecord->R9;
            break;
        case 0xA:
            instrData.bufferValue = exceptionInfo->ContextRecord->R10;
            break;
        case 0xB:
            instrData.bufferValue = exceptionInfo->ContextRecord->R11;
            break;
        case 0xC:
            instrData.bufferValue = exceptionInfo->ContextRecord->R12;
            break;
        case 0xD:
            instrData.bufferValue = exceptionInfo->ContextRecord->R13;
            break;
        case 0xE:
            instrData.bufferValue = exceptionInfo->ContextRecord->R14;
            break;
        case 0xF:
            instrData.bufferValue = exceptionInfo->ContextRecord->R15;
            break;
        default:
            instrData.isInvalidInstr = true;
            Assert(false);// should never reach here as validation is done before itself
            return instrData;
        }
        // add the pc for displacement , we dont need the displacement Byte value
        if (dispBytes > 0)
        {
            pc = pc + dispBytes;
        }
        instrData.instrSizeInByte = (uint)(pc - beginPc);
        if (isImmediate)
        {
            Assert(immBytes > 0);
            instrData.instrSizeInByte += immBytes;
        }
        //6h.	Calculate the number of bytes read in order to get the length of the instruction , ensure that the length should never be greater than 15 bytes
        if (instrData.instrSizeInByte > 15)
        {
            // no instr size can be greater than 15
            instrData.isInvalidInstr = true;
        }
        return instrData;
    }
    int JavascriptFunction::ResumeForOutOfBoundsAsmJSArrayRefs(int exceptionCode, PEXCEPTION_POINTERS exceptionInfo)
    {
        // 1) Exception Code is AV i.e STATUS_ACCESS_VIOLATION
        if (exceptionCode == STATUS_ACCESS_VIOLATION)
        {
            ThreadContext* threadContext = ThreadContext::GetContextForCurrentThread();

            //2)  Check if Rip is Native address

            BOOL isNativeAddress = threadContext->IsNativeAddress((Var)exceptionInfo->ContextRecord->Rip);
            if (isNativeAddress)
            {

                BYTE* pc = (BYTE*)exceptionInfo->ExceptionRecord->ExceptionAddress;
                Var* addressOfFuncObj = (Var*)(exceptionInfo->ContextRecord->Rbp + 2 * sizeof(Var));
                Js::ScriptFunction* func = (ScriptFunction::Is(*addressOfFuncObj))?(Js::ScriptFunction*)(*addressOfFuncObj):nullptr;
                RecyclerHeapObjectInfo heapObject;
                Recycler* recycler = threadContext->GetRecycler();

                //3b.   Ensure that the function object is heap allocated
                //3c.   Ensure that the entrypointInfo is heap allocated
                //3d.   Ensure that the functionbody is heap allocated
                if (func)
                {
                    bool isFuncObjHeapAllocated = recycler->FindHeapObject(func, FindHeapObjectFlags_NoFlags, heapObject); // recheck if this needs to be removed
                    bool isEntryPointHeapAllocated = recycler->FindHeapObject(func->GetEntryPointInfo(), FindHeapObjectFlags_NoFlags, heapObject);
                    bool isFunctionBodyHeapAllocated = recycler->FindHeapObject(func->GetFunctionBody(), FindHeapObjectFlags_NoFlags, heapObject);

                    //3a. Not Null
                    //3c. Is a function
                    //3d. Is AsmJs Function object
                    if (isFuncObjHeapAllocated && isEntryPointHeapAllocated && isFunctionBodyHeapAllocated)
                    {
                        bool isAsmJs = func->GetFunctionBody()->GetIsAsmJsFunction();
                        Js::FunctionBody* funcBody = func->GetFunctionBody();
                        AsmJSInstructionDecoder::InstructionData instrData;
                        //4.	Check if Rip is Native address
                        BOOL isJitAddress = IsNativeAddress(funcBody->GetScriptContext(), (Var)exceptionInfo->ContextRecord->Rip);
                        BYTE* buffer = nullptr;
                        Js::FunctionEntryPointInfo* funcEntryPointInfo = nullptr;
                        ArrayBuffer* arrayBuffer = nullptr;
                        if (isAsmJs)
                        {
                            funcEntryPointInfo = (Js::FunctionEntryPointInfo*)funcBody->GetDefaultEntryPointInfo();
                            arrayBuffer = *(ArrayBuffer**)(funcEntryPointInfo->GetModuleAddress() + AsmJsModuleMemory::MemoryTableBeginOffset);
                        }
                        bool isValidAsmLength = (isAsmJs) ? false : true;
                        if (isAsmJs && arrayBuffer)
                        {
                            buffer = arrayBuffer->GetBuffer();
                            //4.	Check if Rip is Native address
                            Assert(buffer);

                            uint bufferLength = arrayBuffer->GetByteLength();
                            Assert(funcEntryPointInfo->IsCodeGenDone() && !(funcEntryPointInfo->GetIsTJMode()));

                            //5.    Check if Array BufferLength > 0x10000 (64K) , power of 2 and multiple of 0x1000(4K) 
                            isValidAsmLength = arrayBuffer->IsValidAsmJsBufferLength(bufferLength);
                        }

                        //5.    Check if Array BufferLength > 0x10000 (64K) , power of 2 and multiple of 0x1000(4K) 
                        if (isJitAddress && isValidAsmLength)
                        {
                            // 6.	Check If the instruction is valid
                            AsmJSInstructionDecoder::InstructionData instrData = AsmJSInstructionDecoder::CheckValidInstr(pc, exceptionInfo, funcBody);

                            // is it a valid instr
                            if (!instrData.isInvalidInstr && (!isAsmJs || (instrData.bufferValue == (uint64)buffer)))
                            {
                                //8.    Set the dst reg if the instr type is load
                                if (instrData.isLoad)
                                {
                                    Var exceptionInfoReg = exceptionInfo->ContextRecord;
                                    Var* exceptionInfoIntReg = (Var*)((uint64)exceptionInfoReg + offsetof(CONTEXT, CONTEXT::Rax)); // offset in the contextRecord for RAX , the assert below checks for any change in the exceptionInfo struct
                                    Var* exceptionInfoFloatReg = (Var*)((uint64)exceptionInfoReg + offsetof(CONTEXT, CONTEXT::Xmm0));// offset in the contextRecord for XMM0 , the assert below checks for any change in the exceptionInfo struct                                    
                                    Assert((DWORD64)*exceptionInfoIntReg == exceptionInfo->ContextRecord->Rax);
                                    Assert((uint64)*exceptionInfoFloatReg == exceptionInfo->ContextRecord->Xmm0.Low);

                                    if (instrData.isLoad)
                                    {
                                        double nanVal = JavascriptNumber::NaN;
                                        if (instrData.isFloat64)
                                        {
                                            double* destRegLocation = (double*)((uint64)exceptionInfoFloatReg + 16 * (instrData.dstReg));
                                            *destRegLocation = nanVal;
                                        }
                                        else if (instrData.isFloat32)
                                        {
                                            float* destRegLocation = (float*)((uint64)exceptionInfoFloatReg + 16 * (instrData.dstReg));
                                            *destRegLocation = (float)nanVal;
                                        }
                                        else
                                        {
                                            uint64* destRegLocation = (uint64*)((uint64)exceptionInfoIntReg + 8 * (instrData.dstReg));
                                            *destRegLocation = 0;
                                        }
                                    }

                                }
                                //modify the RIP finally
                                //9.	Add the bytes read to Rip and set it as new Rip
                                exceptionInfo->ContextRecord->Rip = exceptionInfo->ContextRecord->Rip + instrData.instrSizeInByte;
                                //10.	Return EXCEPTION_CONTINUE_EXECUTION
                                return EXCEPTION_CONTINUE_EXECUTION;
                            }
                        }
                    }
                }
            }
        }
        return EXCEPTION_CONTINUE_SEARCH;
    }
#endif
#if DBG
    void JavascriptFunction::VerifyEntryPoint()
    {
        JavascriptMethod callEntryPoint = this->GetType()->GetEntryPoint();
        if (this->IsCrossSiteObject())
        {
            Assert(CrossSite::IsThunk(callEntryPoint));
        }
        else if (ScriptFunction::Is(this))
        {
        }
        else
        {
            JavascriptMethod originalEntryPoint = this->GetFunctionInfo()->GetOriginalEntryPoint();
            Assert(callEntryPoint == originalEntryPoint || callEntryPoint == ProfileEntryThunk
                || callEntryPoint == DOMFastPathInfo::CrossSiteSimpleSlotAccessorThunk);
        }
    }
#endif
}
