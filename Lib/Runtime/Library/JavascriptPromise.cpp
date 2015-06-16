//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    JavascriptPromise::JavascriptPromise(DynamicType * type)
        : DynamicObject(type)
    {
        Assert(type->GetTypeId() == TypeIds_Promise);

        this->status = PromiseStatusCode_Undefined;
        this->constructor = this->GetScriptContext()->GetLibrary()->GetPromiseConstructor();
        this->result = nullptr;
        this->resolveReactions = nullptr;
        this->rejectReactions = nullptr;
    }

    // Promise() and new Promise() as defined by ES6.0 (draft 22) Sections 25.4.3.1 and 25.4.3.2
    Var JavascriptPromise::NewInstance(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
        ARGUMENTS(args, callInfo);
        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");

        ScriptContext* scriptContext = function->GetScriptContext();
        JavascriptLibrary* library = scriptContext->GetLibrary();

        CHAKRATEL_LANGSTATS_INC_BUILTINCOUNT(PromiseCount);

        // SkipDefaultNewObject function flag should have revent the default object
        // being created, except when call true a host dispatch
        Assert(!(callInfo.Flags & CallFlags_New) || args[0] == nullptr
            || JavascriptOperators::GetTypeId(args[0]) == TypeIds_HostDispatch);

        AUTO_TAG_NATIVE_LIBRARY_ENTRY(scriptContext, L"Promise");

        JavascriptPromise* promise = nullptr;

        if (callInfo.Flags & CallFlags_New)
        {
            promise = library->CreatePromise();
        }
        else
        {
            if (args.Info.Count < 1 || !JavascriptPromise::Is(args[0]))
            {
                JavascriptError::ThrowTypeErrorVar(scriptContext, JSERR_NeedObjectOfType, L"Promise", L"Promise");
            }

            promise = JavascriptPromise::FromVar(args[0]);
        }

        if (args.Info.Count < 2 || !JavascriptConversion::IsCallable(args[1]))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_FunctionArgument_NeedFunction, L"Promise");
        }

        RecyclableObject* executor = RecyclableObject::FromVar(args[1]);

        if (promise->status != PromiseStatusCode_Undefined)
        {
            JavascriptError::ThrowTypeErrorVar(scriptContext, JSERR_ObjectIsAlreadyInitialized, L"Promise", L"Promise");
        }

        JavascriptPromiseResolveOrRejectFunction* resolve;
        JavascriptPromiseResolveOrRejectFunction* reject;

        InitializePromise(promise, &resolve, &reject, scriptContext);

        try
        {
            executor->GetEntryPoint()(executor, CallInfo(CallFlags_Value, 3),
                library->GetUndefined(),
                resolve,
                reject);
        }
        catch (JavascriptExceptionObject* e)
        {
            // Call reject function on abrupt completion of the executor
            reject->GetEntryPoint()(reject, CallInfo(CallFlags_Value, 2),
                library->GetUndefined(),
                e->GetThrownObject(scriptContext));
        }

        return promise;
    }

    void JavascriptPromise::InitializePromise(JavascriptPromise* promise, JavascriptPromiseResolveOrRejectFunction** resolve, JavascriptPromiseResolveOrRejectFunction** reject, ScriptContext* scriptContext)
    {
        Assert(promise->status == PromiseStatusCode_Undefined);
        Assert(resolve);
        Assert(reject);

        Recycler* recycler = scriptContext->GetRecycler();
        JavascriptLibrary* library = scriptContext->GetLibrary();

        promise->status = PromiseStatusCode_Unresolved;

        promise->resolveReactions = RecyclerNew(recycler, JavascriptPromiseReactionList, recycler);
        promise->rejectReactions = RecyclerNew(recycler, JavascriptPromiseReactionList, recycler);

        *resolve = library->CreatePromiseResolveOrRejectFunction(EntryResolveOrRejectFunction, promise, false);
        *reject = library->CreatePromiseResolveOrRejectFunction(EntryResolveOrRejectFunction, promise, true);
    }

    bool JavascriptPromise::Is(Var aValue)
    {
        return Js::JavascriptOperators::GetTypeId(aValue) == TypeIds_Promise;
    }

    JavascriptPromise* JavascriptPromise::FromVar(Js::Var aValue)
    {
        AssertMsg(Is(aValue), "Ensure var is actually a 'JavascriptPromise'");

        return static_cast<JavascriptPromise *>(RecyclableObject::FromVar(aValue));
    }

    BOOL JavascriptPromise::GetDiagValueString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        stringBuilder->AppendCppLiteral(L"[...]");

        return TRUE;
    }

    BOOL JavascriptPromise::GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        stringBuilder->AppendCppLiteral(L"Promise");
        return TRUE;
    }

    RecyclableObject* JavascriptPromise::GetConstructor()
    {
        return this->constructor;
    }

    JavascriptPromiseReactionList* JavascriptPromise::GetResolveReactions()
    {
        return this->resolveReactions;
    }

    JavascriptPromiseReactionList* JavascriptPromise::GetRejectReactions()
    {
        return this->rejectReactions;
    }

    // Promise.all as described in ES6.0 (Release Candidate 3) Section 25.4.4.1
    Var JavascriptPromise::EntryAll(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
        ARGUMENTS(args, callInfo);
        Assert(!(callInfo.Flags & CallFlags_New));

        ScriptContext* scriptContext = function->GetScriptContext();

        AUTO_TAG_NATIVE_LIBRARY_ENTRY(scriptContext, L"Promise.all");

        JavascriptLibrary* library = scriptContext->GetLibrary();
        Var undefinedVar = library->GetUndefined();

        Var constructor = args[0];
        Var iterable;

        if (args.Info.Count > 1)
        {
            iterable = args[1];
        }
        else
        {
            iterable = undefinedVar;
        }

        if (!RecyclableObject::Is(constructor))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedObject, L"Promise.all");
        }

        JavascriptPromiseCapability* promiseCapability = NewPromiseCapability(constructor, scriptContext);
        uint32 index = 0;
        JavascriptArray* values;

        // We can't use a simple counter for the remaining element count since each Promise.all Resolve Element Function needs to know how many 
        // elements are remaining when it runs and needs to update that counter for all other functions created by this call to Promise.all. 
        // We can't just use a static variable, either, since this element count is only used for the Promise.all Resolve Element Functions created
        // by this call to Promise.all.
        JavascriptPromiseAllResolveElementFunctionRemainingElementsWrapper* remainingElementsWrapper = RecyclerNewStructZ(scriptContext->GetRecycler(), JavascriptPromiseAllResolveElementFunctionRemainingElementsWrapper);
        remainingElementsWrapper->remainingElements = 1;
        
        try
        {
            RecyclableObject* iterator = JavascriptOperators::GetIterator(iterable, scriptContext);

            Var next;
            RecyclableObject* constructorObject = nullptr;
            values = library->CreateArray(0);

            while (JavascriptOperators::IteratorStepAndValue(iterator, scriptContext, &next))
            {
                // We are going to use Invoke here which would call ToObject on constructor on each step of the loop.
                // To avoid extra calls to ToObject, we can just cache the result of the first ToObject call.
                // However, we need to do it after we've done the first IteratorStep and IteratorValue calls to make sure 
                // we throw from the ToObject call when the iterator is in the right state.
                if (constructorObject == nullptr && !JavascriptConversion::ToObject(constructor, scriptContext, &constructorObject))
                {
                    JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedObject, L"Promise.all");
                }

                Var resolveVar = JavascriptOperators::GetProperty(constructorObject, Js::PropertyIds::resolve, scriptContext);

                if (!JavascriptConversion::IsCallable(resolveVar))
                {
                    JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedFunction);
                }

                RecyclableObject* resolveFunc = RecyclableObject::FromVar(resolveVar);

                Var nextPromise = resolveFunc->GetEntryPoint()(resolveFunc, Js::CallInfo(CallFlags_Value, 2),
                    constructorObject,
                    next);

                JavascriptPromiseAllResolveElementFunction* resolveElement = library->CreatePromiseAllResolveElementFunction(EntryAllResolveElementFunction, index, values, promiseCapability, remainingElementsWrapper);

                remainingElementsWrapper->remainingElements++;

                RecyclableObject* nextPromiseObject;

                if (!JavascriptConversion::ToObject(nextPromise, scriptContext, &nextPromiseObject))
                {
                    JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedObject);
                }

                Var thenVar = JavascriptOperators::GetProperty(nextPromiseObject, Js::PropertyIds::then, scriptContext);

                if (!JavascriptConversion::IsCallable(thenVar))
                {
                    JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedFunction);
                }

                RecyclableObject* thenFunc = RecyclableObject::FromVar(thenVar);

                thenFunc->GetEntryPoint()(thenFunc, Js::CallInfo(CallFlags_Value, 3),
                    nextPromiseObject,
                    resolveElement,
                    promiseCapability->GetReject());

                index++;
            }
        }
        catch (JavascriptExceptionObject* e)
        {
            RecyclableObject* reject = promiseCapability->GetReject();

            reject->GetEntryPoint()(reject, CallInfo(CallFlags_Value, 2),
                undefinedVar,
                e->GetThrownObject(scriptContext));

            // We need to explicitly return here to make sure we don't resolve in case index == 0 here.
            // That would happen if GetIterator or IteratorValue throws an exception in the first iteration.
            return promiseCapability->GetPromise();
        }

        remainingElementsWrapper->remainingElements--;

        // We want this call to happen outside the try statement because if it throws, we aren't supposed to reject the promise.
        if (remainingElementsWrapper->remainingElements == 0)
        {
            RecyclableObject* resolve = promiseCapability->GetResolve();

            resolve->GetEntryPoint()(resolve, CallInfo(CallFlags_Value, 2),
                undefinedVar,
                values);
        }

        return promiseCapability->GetPromise();
    }

    // Promise.prototype.catch as defined in ES6.0 (draft 29) Section 25.4.5.1
    Var JavascriptPromise::EntryCatch(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
        ARGUMENTS(args, callInfo);
        Assert(!(callInfo.Flags & CallFlags_New));

        ScriptContext* scriptContext = function->GetScriptContext();

        AUTO_TAG_NATIVE_LIBRARY_ENTRY(scriptContext, L"Promise.prototype.catch");

        RecyclableObject* promise;

        if (!JavascriptConversion::ToObject(args[0], scriptContext, &promise))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedObject, L"Promise.prototype.catch");
        }

        Var funcVar = JavascriptOperators::GetProperty(promise, Js::PropertyIds::then, scriptContext);

        if (!JavascriptConversion::IsCallable(funcVar))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_FunctionArgument_NeedFunction, L"Promise.prototype.catch");
        }

        Var onRejected;
        RecyclableObject* undefinedVar = scriptContext->GetLibrary()->GetUndefined();

        if (args.Info.Count > 1)
        {
            onRejected = args[1];
        }
        else
        {
            onRejected = undefinedVar;
        }

        RecyclableObject* func = RecyclableObject::FromVar(funcVar);

        return func->GetEntryPoint()(func, Js::CallInfo(CallFlags_Value, 3),
            promise,
            undefinedVar,
            onRejected);
    }

    // Promise.race as described in ES6.0 (draft 29) Section 25.4.4.3
    Var JavascriptPromise::EntryRace(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
        ARGUMENTS(args, callInfo);
        Assert(!(callInfo.Flags & CallFlags_New));

        ScriptContext* scriptContext = function->GetScriptContext();

        AUTO_TAG_NATIVE_LIBRARY_ENTRY(scriptContext, L"Promise.race");

        Var undefinedVar = scriptContext->GetLibrary()->GetUndefined();
        Var constructor = args[0];
        Var iterable;

        if (args.Info.Count > 1)
        {
            iterable = args[1];
        }
        else
        {
            iterable = undefinedVar;
        }

        if (!RecyclableObject::Is(constructor))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedObject, L"Promise.race");
        }

        JavascriptPromiseCapability* promiseCapability = NewPromiseCapability(constructor, scriptContext);

        try
        {
            RecyclableObject* iterator = JavascriptOperators::GetIterator(iterable, scriptContext);

            Var next;
            RecyclableObject* constructorObject = nullptr;

            while (JavascriptOperators::IteratorStepAndValue(iterator, scriptContext, &next))
            {
                // We are going to use Invoke here which would call ToObject on constructor on each step of the loop.
                // To avoid extra calls to ToObject, we can just cache the result of the first ToObject call.
                // However, we need to do it after we've done the first IteratorStep and IteratorValue calls to make sure 
                // we throw from the ToObject call when the iterator is in the right state.
                if (constructorObject == nullptr && !JavascriptConversion::ToObject(constructor, scriptContext, &constructorObject))
                {
                    JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedObject, L"Promise.race");
                }

                Var resolveVar = JavascriptOperators::GetProperty(constructorObject, Js::PropertyIds::resolve, scriptContext);

                if (!JavascriptConversion::IsCallable(resolveVar))
                {
                    JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedFunction);
                }

                RecyclableObject* resolveFunc = RecyclableObject::FromVar(resolveVar);

                Var nextPromise = resolveFunc->GetEntryPoint()(resolveFunc, Js::CallInfo(CallFlags_Value, 2),
                    constructorObject,
                    next);

                RecyclableObject* nextPromiseObject;

                if (!JavascriptConversion::ToObject(nextPromise, scriptContext, &nextPromiseObject))
                {
                    JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedObject);
                }

                Var thenVar = JavascriptOperators::GetProperty(nextPromiseObject, Js::PropertyIds::then, scriptContext);

                if (!JavascriptConversion::IsCallable(thenVar))
                {
                    JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedFunction);
                }

                RecyclableObject* thenFunc = RecyclableObject::FromVar(thenVar);

                thenFunc->GetEntryPoint()(thenFunc, Js::CallInfo(CallFlags_Value, 3),
                    nextPromiseObject,
                    promiseCapability->GetResolve(),
                    promiseCapability->GetReject());
            }
        }
        catch (JavascriptExceptionObject* e)
        {
            RecyclableObject* reject = promiseCapability->GetReject();

            reject->GetEntryPoint()(reject, CallInfo(CallFlags_Value, 2),
                undefinedVar,
                e->GetThrownObject(scriptContext));
        }

        return promiseCapability->GetPromise();
    }

    // Promise.reject as described in ES6.0 (draft 29) Section 25.4.4.4
    Var JavascriptPromise::EntryReject(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
        ARGUMENTS(args, callInfo);
        Assert(!(callInfo.Flags & CallFlags_New));

        ScriptContext* scriptContext = function->GetScriptContext();
        Var undefinedVar = scriptContext->GetLibrary()->GetUndefined();
        Var r;
        Var constructor = args[0];

        if (args.Info.Count > 1)
        {
            r = args[1];
        }
        else
        {
            r = undefinedVar;
        }

        if (!RecyclableObject::Is(constructor))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedObject, L"Promise.reject");
        }

        JavascriptPromiseCapability* promiseCapability = NewPromiseCapability(constructor, scriptContext);
        RecyclableObject* reject = promiseCapability->GetReject();

        reject->GetEntryPoint()(reject, CallInfo(CallFlags_Value, 2),
            undefinedVar,
            r);

        return promiseCapability->GetPromise();
    }

    // Promise.resolve as described in ES6.0 (draft 29) Section 25.4.4.5
    Var JavascriptPromise::EntryResolve(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
        ARGUMENTS(args, callInfo);
        Assert(!(callInfo.Flags & CallFlags_New));

        ScriptContext* scriptContext = function->GetScriptContext();

        AUTO_TAG_NATIVE_LIBRARY_ENTRY(scriptContext, L"Promise.resolve");

        Var undefinedVar = scriptContext->GetLibrary()->GetUndefined();
        Var x;
        Var constructor = args[0];

        if (args.Info.Count > 1)
        {
            x = args[1];
        }
        else
        {
            x = undefinedVar;
        }

        if (IsPromise(x) && JavascriptConversion::SameValue(constructor, JavascriptPromise::FromVar(x)->GetConstructor(), scriptContext))
        {
            return x;
        }

        if (!RecyclableObject::Is(constructor))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedObject, L"Promise.resolve");
        }

        JavascriptPromiseCapability* promiseCapability = NewPromiseCapability(constructor, scriptContext);
        RecyclableObject* resolve = promiseCapability->GetResolve();

        resolve->GetEntryPoint()(resolve, CallInfo(CallFlags_Value, 2),
            undefinedVar,
            x);

        return promiseCapability->GetPromise();
    }

    // Promise.prototype.then as described in ES6.0 (draft 29) Section 25.4.5.3
    Var JavascriptPromise::EntryThen(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
        ARGUMENTS(args, callInfo);
        Assert(!(callInfo.Flags & CallFlags_New));

        ScriptContext* scriptContext = function->GetScriptContext();

        AUTO_TAG_NATIVE_LIBRARY_ENTRY(scriptContext, L"Promise.prototype.then");

        JavascriptPromise* promise;

        if (args.Info.Count < 1 || !JavascriptPromise::IsPromise(args[0]))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedPromise, L"Promise.prototype.then");
        }

        promise = JavascriptPromise::FromVar(args[0]);

        JavascriptLibrary* library = scriptContext->GetLibrary();
        Var constructor = JavascriptOperators::GetProperty(promise, Js::PropertyIds::constructor, scriptContext);
        JavascriptPromiseCapability* promiseCapability = NewPromiseCapability(constructor, scriptContext);
        RecyclableObject* rejectionHandler;
        RecyclableObject* fulfillmentHandler;

        if (args.Info.Count > 1 && JavascriptConversion::IsCallable(args[1]))
        {
            fulfillmentHandler = RecyclableObject::FromVar(args[1]);
        }
        else
        {
            fulfillmentHandler = library->GetIdentityFunction();
        }

        if (args.Info.Count > 2 && JavascriptConversion::IsCallable(args[2]))
        {
            rejectionHandler = RecyclableObject::FromVar(args[2]);
        }
        else
        {
            rejectionHandler = library->GetThrowerFunction();
        }

        JavascriptPromiseReaction* resolveReaction = JavascriptPromiseReaction::New(promiseCapability, fulfillmentHandler, scriptContext);
        JavascriptPromiseReaction* rejectReaction = JavascriptPromiseReaction::New(promiseCapability, rejectionHandler, scriptContext);

        switch (promise->status)
        {
        case PromiseStatusCode_Unresolved:
            promise->resolveReactions->Add(resolveReaction);
            promise->rejectReactions->Add(rejectReaction);
            break;
        case PromiseStatusCode_HasResolution:
            EnqueuePromiseReactionTask(resolveReaction, promise->result, scriptContext);
            break;
        case PromiseStatusCode_HasRejection:
            EnqueuePromiseReactionTask(rejectReaction, promise->result, scriptContext);
            break;
        default:
            AssertMsg(false, "Promise status is in an invalid state");
            break;
        }

        return promiseCapability->GetPromise();
    }

    // Promise Reject and Resolve Functions as described in ES6.0 (draft 29) Section 25.4.1.4.1 and 25.4.1.4.2
    Var JavascriptPromise::EntryResolveOrRejectFunction(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
        ARGUMENTS(args, callInfo);
        Assert(!(callInfo.Flags & CallFlags_New));

        ScriptContext* scriptContext = function->GetScriptContext();
        JavascriptLibrary* library = scriptContext->GetLibrary();
        Var undefinedVar = library->GetUndefined();
        Var resolution;

        if (args.Info.Count > 1)
        {
            resolution = args[1];
        }
        else
        {
            resolution = undefinedVar;
        }

        JavascriptPromiseResolveOrRejectFunction* resolveOrRejectFunction = JavascriptPromiseResolveOrRejectFunction::FromVar(function);

        if (resolveOrRejectFunction->IsAlreadyResolved())
        {
            return undefinedVar;
        }

        resolveOrRejectFunction->SetAlreadyResolved(true);

        bool rejecting = resolveOrRejectFunction->IsRejectFunction();

        JavascriptPromise* promise = resolveOrRejectFunction->GetPromise();

        // We only need to check SameValue and check for thenable resolution in the Resolve function case (not Reject)
        if (!rejecting)
        {
            if (JavascriptConversion::SameValue(resolution, promise, scriptContext))
            {
                JavascriptError* selfResolutionError = scriptContext->GetLibrary()->CreateTypeError();
                JavascriptError::SetErrorMessage(selfResolutionError, JSERR_PromiseSelfResolution, L"", scriptContext);

                resolution = selfResolutionError;
                rejecting = true;
            }
            else if (RecyclableObject::Is(resolution))
            {
                try
                {
                    RecyclableObject* thenable = RecyclableObject::FromVar(resolution);
                    Var then = JavascriptOperators::GetProperty(thenable, Js::PropertyIds::then, scriptContext);

                    if (JavascriptConversion::IsCallable(then))
                    {
                        JavascriptPromiseResolveThenableTaskFunction* resolveThenableTaskFunction = library->CreatePromiseResolveThenableTaskFunction(EntryResolveThenableTaskFunction, promise, thenable, RecyclableObject::FromVar(then));

                        library->EnqueueTask(resolveThenableTaskFunction);

                        return undefinedVar;
                    }
                }
                catch (JavascriptExceptionObject* e)
                {
                    resolution = e->GetThrownObject(scriptContext);
                    rejecting = true;
                }
            }
        }

        JavascriptPromiseReactionList* reactions;
        PromiseStatus newStatus;

        // Need to check rejecting state again as it might have changed due to failures
        if (rejecting)
        {
            reactions = promise->GetRejectReactions();
            newStatus = PromiseStatusCode_HasRejection;
        }
        else
        {
            reactions = promise->GetResolveReactions();
            newStatus = PromiseStatusCode_HasResolution;
        }

        promise->result = resolution;
        promise->resolveReactions = nullptr;
        promise->rejectReactions = nullptr;
        promise->status = newStatus;

        return TriggerPromiseReactions(reactions, resolution, scriptContext);
    }

    // Promise Capabilities Executor Function as described in ES6.0 (draft 29) Section 25.4.1.6.2
    Var JavascriptPromise::EntryCapabilitiesExecutorFunction(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
        ARGUMENTS(args, callInfo);
        Assert(!(callInfo.Flags & CallFlags_New));

        ScriptContext* scriptContext = function->GetScriptContext();
        Var undefinedVar = scriptContext->GetLibrary()->GetUndefined();
        Var resolve = undefinedVar;
        Var reject = undefinedVar;

        if (args.Info.Count > 1)
        {
            resolve = args[1];

            if (args.Info.Count > 2)
            {
                reject = args[2];
            }
        }

        JavascriptPromiseCapabilitiesExecutorFunction* capabilitiesExecutorFunction = JavascriptPromiseCapabilitiesExecutorFunction::FromVar(function);
        JavascriptPromiseCapability* promiseCapability = capabilitiesExecutorFunction->GetCapability();

        if (promiseCapability->GetResolve() != nullptr || promiseCapability->GetReject() != nullptr)
        {
            JavascriptError::ThrowTypeErrorVar(scriptContext, JSERR_UnexpectedMetadataFailure, L"Promise");
        }

        promiseCapability->SetResolve(RecyclableObject::FromVar(resolve));
        promiseCapability->SetReject(RecyclableObject::FromVar(reject));

        return undefinedVar;
    }

    // Promise Reaction Task Function as described in ES6.0 (draft 22) Section 25.4.2.1
    Var JavascriptPromise::EntryReactionTaskFunction(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
        ARGUMENTS(args, callInfo);
        Assert(!(callInfo.Flags & CallFlags_New));

        ScriptContext* scriptContext = function->GetScriptContext();
        Var undefinedVar = scriptContext->GetLibrary()->GetUndefined();

        JavascriptPromiseReactionTaskFunction* reactionTaskFunction = JavascriptPromiseReactionTaskFunction::FromVar(function);
        JavascriptPromiseReaction* reaction = reactionTaskFunction->GetReaction();
        Var argument = reactionTaskFunction->GetArgument();
        JavascriptPromiseCapability* promiseCapability = reaction->GetCapabilities();
        RecyclableObject* handler = reaction->GetHandler();
        Var handlerResult;

        try
        {
            handlerResult = handler->GetEntryPoint()(handler, Js::CallInfo(Js::CallFlags::CallFlags_Value, 2),
                undefinedVar,
                argument);
        }
        catch (JavascriptExceptionObject* e)
        {
            RecyclableObject* reject = promiseCapability->GetReject();

            Assert(reject);

            return reject->GetEntryPoint()(reject, Js::CallInfo(Js::CallFlags::CallFlags_Value, 2),
                undefinedVar,
                e->GetThrownObject(scriptContext));
        }

        RecyclableObject* resolve = promiseCapability->GetResolve();

        Assert(resolve);

        return resolve->GetEntryPoint()(resolve, Js::CallInfo(Js::CallFlags::CallFlags_Value, 2),
            undefinedVar,
            handlerResult);
    }

    // Promise Resolve Thenable Job as described in ES6.0 (draft 29) Section 25.4.2.2
    Var JavascriptPromise::EntryResolveThenableTaskFunction(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
        ARGUMENTS(args, callInfo);
        Assert(!(callInfo.Flags & CallFlags_New));

        ScriptContext* scriptContext = function->GetScriptContext();
        JavascriptLibrary* library = scriptContext->GetLibrary();

        JavascriptPromiseResolveThenableTaskFunction* resolveThenableTaskFunction = JavascriptPromiseResolveThenableTaskFunction::FromVar(function);
        JavascriptPromise* promise = resolveThenableTaskFunction->GetPromise();
        RecyclableObject* thenable = resolveThenableTaskFunction->GetThenable();
        RecyclableObject* thenFunction = resolveThenableTaskFunction->GetThenFunction();

        JavascriptPromiseResolveOrRejectFunction* resolve = library->CreatePromiseResolveOrRejectFunction(EntryResolveOrRejectFunction, promise, false);
        JavascriptPromiseResolveOrRejectFunction* reject = library->CreatePromiseResolveOrRejectFunction(EntryResolveOrRejectFunction, promise, true);

        try
        {
            return thenFunction->GetEntryPoint()(thenFunction, Js::CallInfo(Js::CallFlags::CallFlags_Value, 3),
                thenable,
                resolve,
                reject);
        }
        catch (JavascriptExceptionObject* e)
        {
            return reject->GetEntryPoint()(reject, Js::CallInfo(Js::CallFlags::CallFlags_Value, 2),
                library->GetUndefined(),
                e->GetThrownObject(scriptContext));
        }
    }

    // Promise Identity Function as described in ES6.0 (draft 22) Section 25.4.5.3.1
    Var JavascriptPromise::EntryIdentityFunction(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
        ARGUMENTS(args, callInfo);
        Assert(!(callInfo.Flags & CallFlags_New));

        Assert(args.Info.Count > 1);

        return args[1];
    }

    // Promise Thrower Function as described in ES6.0 (draft 22) Section 25.4.5.3.3
    Var JavascriptPromise::EntryThrowerFunction(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
        ARGUMENTS(args, callInfo);
        Assert(!(callInfo.Flags & CallFlags_New));

        ScriptContext* scriptContext = function->GetScriptContext();

        Assert(args.Info.Count > 1);

        JavascriptExceptionOperators::Throw(args[1], scriptContext);
    }

    // Promise.all Resolve Element Function as described in ES6.0 (Release Candidate 3) Section 25.4.4.1.2
    Var JavascriptPromise::EntryAllResolveElementFunction(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
        ARGUMENTS(args, callInfo);
        Assert(!(callInfo.Flags & CallFlags_New));

        ScriptContext* scriptContext = function->GetScriptContext();
        Var undefinedVar = scriptContext->GetLibrary()->GetUndefined();
        Var x;

        if (args.Info.Count > 1)
        {
            x = args[1];
        }
        else
        {
            x = undefinedVar;
        }

        JavascriptPromiseAllResolveElementFunction* allResolveElementFunction = JavascriptPromiseAllResolveElementFunction::FromVar(function);

        if (allResolveElementFunction->IsAlreadyCalled())
        {
            return undefinedVar;
        }

        allResolveElementFunction->SetAlreadyCalled(true);

        uint32 index = allResolveElementFunction->GetIndex();
        JavascriptArray* values = allResolveElementFunction->GetValues();
        JavascriptPromiseCapability* promiseCapability = allResolveElementFunction->GetCapabilities();

        try
        {
            values->DirectSetItemAt(index, x);
        }
        catch (JavascriptExceptionObject* e)
        {
            RecyclableObject* reject = promiseCapability->GetReject();

            reject->GetEntryPoint()(reject, CallInfo(CallFlags_Value, 2),
                undefinedVar,
                e->GetThrownObject(scriptContext));

            return promiseCapability->GetPromise();
        }

        if (allResolveElementFunction->DecrementRemainingElements() == 0)
        {
            RecyclableObject* resolve = promiseCapability->GetResolve();

            return resolve->GetEntryPoint()(resolve, CallInfo(CallFlags_Value, 2),
                undefinedVar,
                values);
        }

        return undefinedVar;
    }

    // IsPromise as described in ES6.0 (draft 22) Section 25.4.1.6
    bool JavascriptPromise::IsPromise(Var promise)
    {
        return JavascriptPromise::Is(promise) && JavascriptPromise::FromVar(promise)->status != PromiseStatusCode_Undefined;
    }

    // NewPromiseCapability as described in ES6.0 (draft 29) Section 25.4.1.6
    JavascriptPromiseCapability* JavascriptPromise::NewPromiseCapability(Var constructor, ScriptContext* scriptContext)
    {
        if (!JavascriptFunction::IsConstructor(constructor))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedFunction);
        }

        RecyclableObject* constructorFunc = RecyclableObject::FromVar(constructor);
        Var promise = scriptContext->GetLibrary()->CreatePromise();

        return CreatePromiseCapabilityRecord(promise, constructorFunc, scriptContext);
    }

    // CreatePromiseCapabilityRecord as described in ES6.0 (draft 29) Section 25.4.1.6.1
    JavascriptPromiseCapability* JavascriptPromise::CreatePromiseCapabilityRecord(Var promise, RecyclableObject* constructor, ScriptContext* scriptContext)
    {
        JavascriptLibrary* library = scriptContext->GetLibrary();
        JavascriptPromiseCapability* promiseCapability = JavascriptPromiseCapability::New(promise, nullptr, nullptr, scriptContext);

        JavascriptPromiseCapabilitiesExecutorFunction* executor = library->CreatePromiseCapabilitiesExecutorFunction(EntryCapabilitiesExecutorFunction, promiseCapability);

        Var constructorResult = constructor->GetEntryPoint()(constructor, Js::CallInfo(Js::CallFlags::CallFlags_Value, 2),
            promise,
            executor);

        if (promiseCapability->GetResolve() == nullptr || !JavascriptConversion::IsCallable(promiseCapability->GetResolve()) || 
            promiseCapability->GetReject() == nullptr || !JavascriptConversion::IsCallable(promiseCapability->GetReject()))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedFunction, L"Promise");
        }

        if (JavascriptOperators::IsObject(constructorResult) && !JavascriptConversion::SameValue(promise, constructorResult, scriptContext))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_UnexpectedMetadataFailure, L"Promise");
        }

        return promiseCapability;
    }

    // UpdatePromiseFromPotentialThenable as defined in ES6.0 (draft 22) Section 25.4.1.8
    bool JavascriptPromise::UpdatePromiseFromPotentialThenable(Var resolution, JavascriptPromiseCapability* promiseCapability, ScriptContext* scriptContext)
    {
        if (!JavascriptOperators::IsObjectType(JavascriptOperators::GetTypeId(resolution)))
        {
            return false;
        }

        RecyclableObject* obj = RecyclableObject::FromVar(resolution);
        Var then;

        try
        {
            then = JavascriptOperators::GetProperty(obj, Js::PropertyIds::then, scriptContext);
        }
        catch (JavascriptExceptionObject* e)
        {
            RecyclableObject* reject = promiseCapability->GetReject();
            JavascriptLibrary* library = scriptContext->GetLibrary();

            reject->GetEntryPoint()(reject, CallInfo(CallFlags_Value, 2),
                library->GetUndefined(),
                e->GetThrownObject(scriptContext));

            return true;
        }

        if (!JavascriptConversion::IsCallable(then))
        {
            return false;
        }

        RecyclableObject* thenFunc = RecyclableObject::FromVar(then);

        try
        {
            thenFunc->GetEntryPoint()(thenFunc, Js::CallInfo(Js::CallFlags::CallFlags_Value, 3),
                resolution,
                promiseCapability->GetResolve(),
                promiseCapability->GetReject());
        }
        catch (JavascriptExceptionObject* e)
        {
            RecyclableObject* reject = promiseCapability->GetReject();
            JavascriptLibrary* library = scriptContext->GetLibrary();

            reject->GetEntryPoint()(reject, CallInfo(CallFlags_Value, 2),
                library->GetUndefined(),
                e->GetThrownObject(scriptContext));
        }

        return true;
    }

    // TriggerPromiseReactions as defined in ES6.0 (draft 22) Section 25.4.1.7
    Var JavascriptPromise::TriggerPromiseReactions(JavascriptPromiseReactionList* reactions, Var resolution, ScriptContext* scriptContext)
    {
        JavascriptLibrary* library = scriptContext->GetLibrary();

        if (reactions != nullptr)
        {
            for (int i = 0; i < reactions->Count(); i++)
            {
                JavascriptPromiseReaction* reaction = reactions->Item(i);

                EnqueuePromiseReactionTask(reaction, resolution, scriptContext);
            }
        }

        return library->GetUndefined();
    }

    void JavascriptPromise::EnqueuePromiseReactionTask(JavascriptPromiseReaction* reaction, Var resolution, ScriptContext* scriptContext)
    {
        JavascriptLibrary* library = scriptContext->GetLibrary();
        JavascriptPromiseReactionTaskFunction* reactionTaskFunction = library->CreatePromiseReactionTaskFunction(EntryReactionTaskFunction, reaction, resolution);

        library->EnqueueTask(reactionTaskFunction);
    }

    JavascriptPromiseCapability* JavascriptPromise::MakeCopyOnWriteObjectJavascriptPromiseCapability(JavascriptPromiseCapability* src, ScriptContext* scriptContext)
    {
        Assert(src);

        Var promiseCopy = scriptContext->CopyOnWrite(src->GetPromise());
        RecyclableObject* resolveCopy = (RecyclableObject*)scriptContext->CopyOnWrite(src->GetResolve());
        RecyclableObject* rejectCopy = (RecyclableObject*)scriptContext->CopyOnWrite(src->GetReject());
        
        return JavascriptPromiseCapability::New(promiseCopy, resolveCopy, rejectCopy, scriptContext);
    }

    JavascriptPromiseReactionList* JavascriptPromise::MakeCopyOnWriteObjectJavascriptPromiseReactionList(JavascriptPromiseReactionList* src, ScriptContext* scriptContext)
    {
        Recycler* recycler = scriptContext->GetRecycler();

        JavascriptPromiseReactionList* result = RecyclerNew(recycler, JavascriptPromiseReactionList, recycler);

        if (src)
        {
            for (int i = 0; i < src->Count(); i++)
            {
                JavascriptPromiseReaction* reaction = src->Item(i);

                JavascriptPromiseCapability* capability = reaction->GetCapabilities();
                JavascriptPromiseCapability* capabilityCopy = MakeCopyOnWriteObjectJavascriptPromiseCapability(capability, scriptContext);

                RecyclableObject* handlerCopy = (RecyclableObject*)scriptContext->CopyOnWrite(reaction->GetHandler());
                JavascriptPromiseReaction* reactionCopy = JavascriptPromiseReaction::New(capabilityCopy, handlerCopy, scriptContext);

                result->Add(reactionCopy);
            }
        }

        return result;
    }

    JavascriptPromise* JavascriptPromise::MakeCopyOnWriteObject(ScriptContext* scriptContext)
    {
        VERIFY_COPY_ON_WRITE_ENABLED_RET();

        Recycler* recycler = scriptContext->GetRecycler();
        DynamicType* type = scriptContext->GetLibrary()->GetPromiseType();

        typedef CopyOnWriteObject<JavascriptPromise> JavascriptPromiseCopy;
        JavascriptPromiseCopy* result = RecyclerNew(recycler, JavascriptPromiseCopy, type, this, scriptContext);

        if (this->constructor)
        {
            result->constructor = (Js::RecyclableObject*)scriptContext->CopyOnWrite(this->constructor);
        }
        if (this->result)
        {
            result->result = scriptContext->CopyOnWrite(this->result);
        }

        result->status = this->status;

        result->resolveReactions = MakeCopyOnWriteObjectJavascriptPromiseReactionList(this->resolveReactions, scriptContext);
        result->rejectReactions = MakeCopyOnWriteObjectJavascriptPromiseReactionList(this->rejectReactions, scriptContext);

        return result;
    }

    JavascriptPromiseResolveOrRejectFunction::JavascriptPromiseResolveOrRejectFunction(DynamicType* type)
        : RuntimeFunction(type, &Js::JavascriptPromise::EntryInfo::ResolveOrRejectFunction), promise(nullptr), isReject(false), isAlreadyResolved(false)
    { }

    JavascriptPromiseResolveOrRejectFunction::JavascriptPromiseResolveOrRejectFunction(DynamicType* type, FunctionInfo* functionInfo, JavascriptPromise* promise, bool isReject)
        : RuntimeFunction(type, functionInfo), promise(promise), isReject(isReject), isAlreadyResolved(false)
    { }

    bool JavascriptPromiseResolveOrRejectFunction::Is(Var var)
    {
        if (JavascriptFunction::Is(var))
        {
            JavascriptFunction* obj = JavascriptFunction::FromVar(var);

            return VirtualTableInfo<JavascriptPromiseResolveOrRejectFunction>::HasVirtualTable(obj)
                || VirtualTableInfo<CrossSiteObject<JavascriptPromiseResolveOrRejectFunction>>::HasVirtualTable(obj)
                || VirtualTableInfo<CopyOnWriteObject<JavascriptPromiseResolveOrRejectFunction, Js::JavascriptFunctionSpecialProperties>>::HasVirtualTable(obj);
        }

        return false;
    }

    JavascriptPromiseResolveOrRejectFunction* JavascriptPromiseResolveOrRejectFunction::FromVar(Var var)
    {
        Assert(JavascriptPromiseResolveOrRejectFunction::Is(var));

        return static_cast<JavascriptPromiseResolveOrRejectFunction*>(var);
    }

    JavascriptPromiseResolveOrRejectFunction* JavascriptPromiseResolveOrRejectFunction::MakeCopyOnWriteObject(ScriptContext* scriptContext)
    {
        VERIFY_COPY_ON_WRITE_ENABLED_RET();
        
        Recycler* recycler = scriptContext->GetRecycler();
        DynamicType* type = scriptContext->GetLibrary()->CreateFunctionType(this->GetEntryPoint());

        typedef CopyOnWriteObject<JavascriptPromiseResolveOrRejectFunction, JavascriptFunctionSpecialProperties> JavascriptPromiseResolveOrRejectFunctionCopy;
        JavascriptPromiseResolveOrRejectFunctionCopy* result = RecyclerNew(recycler, JavascriptPromiseResolveOrRejectFunctionCopy, type, this, scriptContext);

        result->functionInfo = this->functionInfo;

        if (this->promise)
        {
            result->promise = (Js::JavascriptPromise*)scriptContext->CopyOnWrite(this->promise);
        }

        result->isAlreadyResolved = this->isAlreadyResolved;
        result->isReject = this->isReject;

        return result;
    }

    JavascriptPromise* JavascriptPromiseResolveOrRejectFunction::GetPromise()
    {
        return this->promise;
    }

    bool JavascriptPromiseResolveOrRejectFunction::IsRejectFunction()
    {
        return this->isReject;
    }

    bool JavascriptPromiseResolveOrRejectFunction::IsAlreadyResolved()
    {
        return this->isAlreadyResolved;
    }

    void JavascriptPromiseResolveOrRejectFunction::SetAlreadyResolved(bool is)
    {
        this->isAlreadyResolved = is;
    }

    JavascriptPromiseCapabilitiesExecutorFunction::JavascriptPromiseCapabilitiesExecutorFunction(DynamicType* type, FunctionInfo* functionInfo, JavascriptPromiseCapability* capability)
        : RuntimeFunction(type, functionInfo), capability(capability)
    { }

    bool JavascriptPromiseCapabilitiesExecutorFunction::Is(Var var)
    {
        if (JavascriptFunction::Is(var))
        {
            JavascriptFunction* obj = JavascriptFunction::FromVar(var);

            return VirtualTableInfo<JavascriptPromiseCapabilitiesExecutorFunction>::HasVirtualTable(obj)
                || VirtualTableInfo<CrossSiteObject<JavascriptPromiseCapabilitiesExecutorFunction>>::HasVirtualTable(obj);
        }

        return false;
    }

    JavascriptPromiseCapabilitiesExecutorFunction* JavascriptPromiseCapabilitiesExecutorFunction::FromVar(Var var)
    {
        Assert(JavascriptPromiseCapabilitiesExecutorFunction::Is(var));

        return static_cast<JavascriptPromiseCapabilitiesExecutorFunction*>(var);
    }

    JavascriptPromiseCapability* JavascriptPromiseCapabilitiesExecutorFunction::GetCapability()
    {
        return this->capability;
    }

    JavascriptPromiseCapability* JavascriptPromiseCapability::New(Var promise, RecyclableObject* resolve, RecyclableObject* reject, ScriptContext* scriptContext)
    {
        return RecyclerNew(scriptContext->GetRecycler(), JavascriptPromiseCapability, promise, resolve, reject);
    }

    RecyclableObject* JavascriptPromiseCapability::GetResolve()
    {
        return this->resolve;
    }

    RecyclableObject* JavascriptPromiseCapability::GetReject()
    {
        return this->reject;
    }

    Var JavascriptPromiseCapability::GetPromise()
    {
        return this->promise;
    }

    void JavascriptPromiseCapability::SetResolve(RecyclableObject* resolve)
    {
        this->resolve = resolve;
    }

    void JavascriptPromiseCapability::SetReject(RecyclableObject* reject)
    {
        this->reject = reject;
    }

    JavascriptPromiseReaction* JavascriptPromiseReaction::New(JavascriptPromiseCapability* capabilities, RecyclableObject* handler, ScriptContext* scriptContext)
    {
        return RecyclerNew(scriptContext->GetRecycler(), JavascriptPromiseReaction, capabilities, handler);
    }

    JavascriptPromiseCapability* JavascriptPromiseReaction::GetCapabilities()
    {
        return this->capabilities;
    }

    RecyclableObject* JavascriptPromiseReaction::GetHandler()
    {
        return this->handler;
    }

    JavascriptPromiseReaction* JavascriptPromiseReactionTaskFunction::GetReaction()
    {
        return this->reaction;
    }

    Var JavascriptPromiseReactionTaskFunction::GetArgument()
    {
        return this->argument;
    }

    JavascriptPromise* JavascriptPromiseResolveThenableTaskFunction::GetPromise()
    {
        return this->promise;
    }

    RecyclableObject* JavascriptPromiseResolveThenableTaskFunction::GetThenable()
    {
        return this->thenable;
    }

    RecyclableObject* JavascriptPromiseResolveThenableTaskFunction::GetThenFunction()
    {
        return this->thenFunction;
    }

    JavascriptPromiseAllResolveElementFunction::JavascriptPromiseAllResolveElementFunction(DynamicType* type)
        : RuntimeFunction(type, &Js::JavascriptPromise::EntryInfo::AllResolveElementFunction), index(0), values(nullptr), capabilities(nullptr), remainingElementsWrapper(nullptr), alreadyCalled(false)
    { }

    JavascriptPromiseAllResolveElementFunction::JavascriptPromiseAllResolveElementFunction(DynamicType* type, FunctionInfo* functionInfo, uint32 index, JavascriptArray* values, JavascriptPromiseCapability* capabilities, JavascriptPromiseAllResolveElementFunctionRemainingElementsWrapper* remainingElementsWrapper)
        : RuntimeFunction(type, functionInfo), index(index), values(values), capabilities(capabilities), remainingElementsWrapper(remainingElementsWrapper), alreadyCalled(false)
    { }

    bool JavascriptPromiseAllResolveElementFunction::Is(Var var)
    {
        if (JavascriptFunction::Is(var))
        {
            JavascriptFunction* obj = JavascriptFunction::FromVar(var);

            return VirtualTableInfo<JavascriptPromiseAllResolveElementFunction>::HasVirtualTable(obj)
                || VirtualTableInfo<CrossSiteObject<JavascriptPromiseAllResolveElementFunction>>::HasVirtualTable(obj)
                || VirtualTableInfo<CopyOnWriteObject<JavascriptPromiseAllResolveElementFunction, JavascriptFunctionSpecialProperties>>::HasVirtualTable(obj);
        }

        return false;
    }

    JavascriptPromiseAllResolveElementFunction* JavascriptPromiseAllResolveElementFunction::FromVar(Var var)
    {
        Assert(JavascriptPromiseAllResolveElementFunction::Is(var));

        return static_cast<JavascriptPromiseAllResolveElementFunction*>(var);
    }

    JavascriptPromiseAllResolveElementFunction* JavascriptPromiseAllResolveElementFunction::MakeCopyOnWriteObject(ScriptContext* scriptContext)
    {
        VERIFY_COPY_ON_WRITE_ENABLED_RET();

        Recycler* recycler = scriptContext->GetRecycler();
        DynamicType* type = scriptContext->GetLibrary()->CreateFunctionType(this->GetEntryPoint());

        typedef CopyOnWriteObject<JavascriptPromiseAllResolveElementFunction, JavascriptFunctionSpecialProperties> JavascriptPromiseAllResolveElementFunctionCopy;
        JavascriptPromiseAllResolveElementFunctionCopy* result = RecyclerNew(recycler, JavascriptPromiseAllResolveElementFunctionCopy, type, this, scriptContext);

        result->functionInfo = this->functionInfo;

        result->index = this->index;
        result->alreadyCalled = this->alreadyCalled;

        if (this->remainingElementsWrapper)
        {
            result->remainingElementsWrapper = RecyclerNewStructZ(recycler, JavascriptPromiseAllResolveElementFunctionRemainingElementsWrapper);
            result->remainingElementsWrapper->remainingElements = this->remainingElementsWrapper->remainingElements;
        }

        if (this->values)
        {
            result->values = (JavascriptArray*)scriptContext->CopyOnWrite(this->values);
        }

        if (this->capabilities)
        {
            result->capabilities = JavascriptPromise::MakeCopyOnWriteObjectJavascriptPromiseCapability(this->capabilities, scriptContext);
        }

        return result;
    }

    JavascriptPromiseCapability* JavascriptPromiseAllResolveElementFunction::GetCapabilities()
    {
        return this->capabilities;
    }

    uint32 JavascriptPromiseAllResolveElementFunction::GetIndex()
    {
        return this->index;
    }

    uint32 JavascriptPromiseAllResolveElementFunction::GetRemainingElements()
    {
        return this->remainingElementsWrapper->remainingElements;
    }

    JavascriptArray* JavascriptPromiseAllResolveElementFunction::GetValues()
    {
        return this->values;
    }

    uint32 JavascriptPromiseAllResolveElementFunction::DecrementRemainingElements()
    {
        return --(this->remainingElementsWrapper->remainingElements);
    }

    bool JavascriptPromiseAllResolveElementFunction::IsAlreadyCalled() const
    {
        return this->alreadyCalled;
    }

    void JavascriptPromiseAllResolveElementFunction::SetAlreadyCalled(const bool is)
    {
        this->alreadyCalled = is;
    }

    Var JavascriptPromise::EntryGetterSymbolSpecies(RecyclableObject* function, CallInfo callInfo, ...)
    {
        ARGUMENTS(args, callInfo);

        Assert(args.Info.Count > 0);

        return args[0];
    }
} // namespace Js
