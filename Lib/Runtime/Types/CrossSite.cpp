//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "stdafx.h"

namespace Js
{

    BOOL CrossSite::NeedMarshalVar(Var instance, ScriptContext * requestContext)
    {
        if (TaggedNumber::Is(instance) || BinaryFeatureControl::LanguageService())
        {
            return FALSE;
        }
        RecyclableObject * object = RecyclableObject::FromVar(instance);
        if (object->GetScriptContext() == requestContext)
        {
            return FALSE;
        }
        if (DynamicType::Is(object->GetTypeId()))
        {
            return !DynamicObject::FromVar(object)->IsCrossSiteObject() && !object->IsExternal();
        }
        return TRUE;
    }

    void CrossSite::MarshalDynamicObject(ScriptContext * scriptContext, DynamicObject * object)
    {
        Assert(!object->IsExternal() && !object->IsCrossSiteObject());
        object->MarshalToScriptContext(scriptContext);
        if (object->GetTypeId() == TypeIds_Function)
        {
            AssertMsg(object != object->GetScriptContext()->GetLibrary()->GetDefaultAccessorFunction(), "default accessor marshalled");
            JavascriptFunction * function = JavascriptFunction::FromVar(object);
            FunctionInfo* functionInfo = Js::JavascriptFunction::FromVar(object)->GetFunctionInfo();
            const bool useSlotAccessCrossSiteThunk =
                    ((functionInfo->GetAttributes() & Js::FunctionInfo::Attributes::NeedCrossSiteSecurityCheck) != 0);
            if (function->GetDynamicType()->GetIsShared())
            {
                function->GetLibrary()->SetCrossSiteForSharedFunctionType(function, useSlotAccessCrossSiteThunk);
            }
            else
            {
                JavascriptMethod crossSiteThunk = useSlotAccessCrossSiteThunk ?
                    function->GetScriptContext()->GetHostScriptContext()->GetSimpleSlotAccessCrossSiteThunk() :
                    function->GetScriptContext()->CurrentCrossSiteThunk;    // The Crosssite thunk of the function will be different due to Profiler and debugger
                // we have a table of functonType for all functions of the same slot table. We should
                // avoid changing the original type in the table.
                if (useSlotAccessCrossSiteThunk)
                {
                    function->ChangeType();
                }
                function->SetEntryPoint(crossSiteThunk);
            }
        }
    }

    void CrossSite::MarshalPrototypeChain(ScriptContext* scriptContext, DynamicObject * object)
    {
        RecyclableObject * prototype = object->GetPrototype();
        while (prototype->GetTypeId() != TypeIds_Null && prototype->GetTypeId() != TypeIds_HostDispatch)
        {
            // We should not see any static type or host dispatch here
            DynamicObject * prototypeObject = DynamicObject::FromVar(prototype);
            if (prototypeObject->IsCrossSiteObject())
            {
                break;
            }
            if (scriptContext != prototypeObject->GetScriptContext() && !prototypeObject->IsExternal())
            {
                MarshalDynamicObject(scriptContext, prototypeObject);
            }
            prototype = prototypeObject->GetPrototype();
        }
    }

    void CrossSite::MarshalDynamicObjectAndPrototype(ScriptContext* scriptContext, DynamicObject * object)
    {
        MarshalDynamicObject(scriptContext, object);
        MarshalPrototypeChain(scriptContext, object);
    }

    Var CrossSite::MarshalFrameDisplay(ScriptContext* scriptContext, FrameDisplay *display)
    {
        uint16 length = display->GetLength();
        FrameDisplay *newDisplay =
            RecyclerNewPlus(scriptContext->GetRecycler(), length * sizeof(Var), FrameDisplay, length);
        for (uint16 i = 0; i < length; i++)
        {
            Var value = display->GetItem(i);
            if (WithScopeObject::Is(value))
            {
                // Here we are marshalling the wrappedObject and then ReWrapping th object in the new context.
                value = JavascriptOperators::ToWithObject(CrossSite::MarshalVar(scriptContext, WithScopeObject::FromVar(value)->GetWrappedObject()), scriptContext);
            }
            else
            {
                value = CrossSite::MarshalVar(scriptContext, value);
            }
            newDisplay->SetItem(i, value);
        }

        return (Var)newDisplay;
    }

    /*
        Enumerators are always created in current script context, and if the underline object is a cross
        site object, we'll marshal the enumerator by changing the vtbl of the base enumerator, such that
        we will marshal the return index before it's returned to caller.
        Notice that enumerator marshalling is somewhat different from the object marshalling. We have only
        one instance of object in cross site usage, but we can create multiple enumerators from different
        script context for the same cross site object.
    */
    Var CrossSite::MarshalEnumerator(ScriptContext* scriptContext, Var value)
    {
        TypeId typeId = JavascriptOperators::GetTypeId(value);
        if (typeId != TypeIds_Enumerator)
        {
            AssertMsg(FALSE, "invalid enumerator");
            return value;
        }
        JavascriptEnumerator* enumerator = JavascriptEnumerator::FromVar(value);
        enumerator->MarshalToScriptContext(scriptContext);
        return enumerator;
    }

    // static
    __inline Var CrossSite::MarshalVar(ScriptContext* scriptContext, Var value, bool fRequestWrapper)
    {
        // value might be null from disable implicit call
        // no marshalling in the case of language service mode.
        if (BinaryFeatureControl::LanguageService() || value == nullptr || Js::TaggedNumber::Is(value))
        {
            return value;
        }
        Js::RecyclableObject* object =  RecyclableObject::FromVar(value);
        if (fRequestWrapper || scriptContext != object->GetScriptContext())
        {
            return MarshalVarInner(scriptContext, object, fRequestWrapper);
        }
        return value;
    }

    bool CrossSite::DoRequestWrapper(Js::RecyclableObject* object, bool fRequestWrapper)
    {
        return fRequestWrapper && JavascriptFunction::Is(object) && JavascriptFunction::FromVar(object)->IsExternalFunction();
    }

    Var CrossSite::MarshalVarInner(ScriptContext* scriptContext, __in Js::RecyclableObject* object, bool fRequestWrapper)
    {
        if (scriptContext == object->GetScriptContext())
        {
            if (DoRequestWrapper(object, fRequestWrapper))
            {
                // If we get here then we need to either wrap in the caller's type system or we need to return undefined.
                // VBScript will pass in the scriptContext (requestContext) from the JavascriptDispatch and this will be the
                // same as the object's script context and so we have to safely pretend this value doesn't exist.
                return scriptContext->GetLibrary()->GetUndefined();
            }
            return object;
        }

        // In heapenum, we are traversing through the object graph to dump out the content of recyclable objects. The content
        // of the objects are duplicated to the heapenum result, and we are not storing/changing the object graph during heap enum.
        // We don't actually need to do cross site thunk here.
        if (scriptContext->GetRecycler()->IsHeapEnumInProgress())
        {
            return object;
        }

        Assert(scriptContext->GetThreadContext()->GetIsThreadBound());
        JavascriptLibrary::CheckAndConvertCopyOnAccessNativeIntArray<Var>(object);
        TypeId typeId = object->GetTypeId();
        AssertMsg(typeId != TypeIds_Enumerator, "enumerator shouldn't be marshalled here");
        
        // At the moment the mental model for WithScopeObject Marshaling is this:
        // Are we trying to marshal a WithScopeObject in the Frame Display? - then 1) unwrap in MarshalFrameDisplay, 
        // 2) marshal the wrapped object,  3) Create a new WithScopeObject in the current scriptContext and rewrap.
        // We can avoid copying the WithScopeObject b\c it has no  properties and never should. 
        // Thus creating a new WithScopeObject per context in MarshalFrameDisplay should be kosher.
        // If it is not a FrameDisplay then we should not marshal. We can wrap cross context objects with a 
        // withscopeObject in a different context. When we unwrap for property lookups and the wrapped object 
        // is cross context, then we marshal the wrapped object into the current scriptContext, thus avoiding 
        // the need to copy the WithScopeObject itself. Thus We don't have to handle marshaling the WithScopeObject
        // in non-FrameDisplay cases.
        AssertMsg(typeId != TypeIds_WithScopeObject, "WithScopeObject shouldn't be marshalled here");
        
        if (StaticType::Is(typeId))
        {
            return object->CloneToScriptContext(scriptContext);
        }

        if (typeId == TypeIds_ModuleRoot)
        {
            ModuleRoot *moduleRoot = static_cast<ModuleRoot*>(object);
            RecyclableObject* hostObject = moduleRoot->GetHostObject();

            // When marshaling module root, all we need is the host object.
            // So, if the module root which is being marshaled has host object, marshal it.
            if (hostObject)
            {
                Var hostDispatch = hostObject->GetHostDispatchVar();
                return CrossSite::MarshalVar(scriptContext, hostDispatch);
            }
        }

        if (typeId == TypeIds_Function)
        {
            if (object == object->GetScriptContext()->GetLibrary()->GetDefaultAccessorFunction() )
            {
                return scriptContext->GetLibrary()->GetDefaultAccessorFunction();
            }

            if (DoRequestWrapper(object, fRequestWrapper))
            {
                // Marshal as a cross-site thunk if necessary before re-wrapping in an external function thunk.
                MarshalVarInner(scriptContext, object, false);
                return scriptContext->GetLibrary()->CreateWrappedExternalFunction(static_cast<JavascriptExternalFunction*>(object));
            }
        }

        // We have a object marshaled, we need to keep track of the related script context
        // so optimization overrides can be updated as a group
        scriptContext->optimizationOverrides.Merge(&object->GetScriptContext()->optimizationOverrides);

        DynamicObject * dynamicObject = DynamicObject::FromVar(object);
        if (!dynamicObject->IsExternal())
        {
            if (!dynamicObject->IsCrossSiteObject())
            {
                MarshalDynamicObjectAndPrototype(scriptContext, dynamicObject);
            }
        }
        else
        {
            MarshalPrototypeChain(scriptContext, dynamicObject);
            if (Js::JavascriptConversion::IsCallable(dynamicObject))
            {
                dynamicObject->MarshalToScriptContext(scriptContext);
            }
        }

        return dynamicObject;
    }

    bool CrossSite::IsThunk(JavascriptMethod thunk)
    {
        return (thunk == CrossSite::ProfileThunk || thunk == CrossSite::DefaultThunk);
    }

    Var CrossSite::ProfileThunk(RecyclableObject* callable, CallInfo callInfo, ...)
    {
        JavascriptFunction* function = JavascriptFunction::FromVar(callable);
        Assert(function->GetTypeId() == TypeIds_Function);
        Assert(function->GetEntryPoint() == CrossSite::ProfileThunk);
        RUNTIME_ARGUMENTS(args, callInfo);
        ScriptContext * scriptContext = function->GetScriptContext();
        // It is not safe to access the function body if the script context is not alive.
        scriptContext->VerifyAliveWithHostContext(!function->IsExternal(),
            scriptContext->GetThreadContext()->GetPreviousHostScriptContext());

        JavascriptMethod entryPoint;
        FunctionInfo *funcInfo = function->GetFunctionInfo();

        if (funcInfo->HasBody())
        {
#if ENABLE_DEBUG_CONFIG_OPTIONS
            wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];
#endif
            entryPoint = (JavascriptMethod)ScriptFunction::FromVar(function)->GetEntryPointInfo()->address;
            if (funcInfo->IsDeferred() && scriptContext->IsProfiling())
            {
                // if the current entrypoint is deferred parse we need to update it appropriately for the profiler mode.
                entryPoint = Js::ScriptContext::GetProfileModeThunk(entryPoint);
            }

            OUTPUT_TRACE(Js::ScriptProfilerPhase, L"CrossSite::ProfileThunk FunctionNumber : %s, Entrypoint : 0x%08X\n", funcInfo->GetFunctionProxy()->GetDebugNumberSet(debugStringBuffer), entryPoint);
        }
        else
        {
            entryPoint = ProfileEntryThunk;
        }


        return CommonThunk(function, entryPoint, args);
    }

    Var CrossSite::DefaultThunk(RecyclableObject* callable, CallInfo callInfo, ...)
    {
        JavascriptFunction* function = JavascriptFunction::FromVar(callable);
        Assert(function->GetTypeId() == TypeIds_Function);
        Assert(function->GetEntryPoint() == CrossSite::DefaultThunk);
        RUNTIME_ARGUMENTS(args, callInfo);

        // It is not safe to access the function body if the script context is not alive.
        function->GetScriptContext()->VerifyAliveWithHostContext(!function->IsExternal(),
            ThreadContext::GetContextForCurrentThread()->GetPreviousHostScriptContext());

        JavascriptMethod entryPoint;
        FunctionInfo *funcInfo = function->GetFunctionInfo();

        if (funcInfo->HasBody())
        {
            entryPoint = (JavascriptMethod)ScriptFunction::FromVar(function)->GetEntryPointInfo()->address;
        }
        else
        {
            entryPoint = funcInfo->GetOriginalEntryPoint();
        }
        return CommonThunk(function, entryPoint, args);
    }

    Var CrossSite::CommonThunk(RecyclableObject* recyclableObject, JavascriptMethod entryPoint, Arguments args)
    {
        DynamicObject* function = DynamicObject::FromVar(recyclableObject);
        ScriptContext* targetScriptContext = function->GetScriptContext();
        Assert(!targetScriptContext->IsClosed());
        Assert(function->IsExternal() || function->IsCrossSiteObject());
        Assert(targetScriptContext->GetThreadContext()->IsScriptActive());

        HostScriptContext* calleeHostScriptContext = targetScriptContext->GetHostScriptContext();
        HostScriptContext* callerHostScriptContext = targetScriptContext->GetThreadContext()->GetPreviousHostScriptContext();

        if (callerHostScriptContext == calleeHostScriptContext || (callerHostScriptContext == NULL && !calleeHostScriptContext->HasCaller()))
        {
            return JavascriptFunction::CallFunction<true>(function, entryPoint, args);
        }

#if DBG_DUMP || defined(PROFILE_EXEC) || defined(PROFILE_MEM)
        calleeHostScriptContext->EnsureParentInfo(callerHostScriptContext->GetScriptContext());
#endif

        uint i = 0;
        if (args.Values[0] == null)
        {
            i = 1;
            Assert(args.Info.Flags & CallFlags_New);
            Assert(JavascriptFunction::Is(function) && JavascriptFunction::FromVar(function)->GetFunctionInfo()->GetAttributes() & FunctionInfo::SkipDefaultNewObject);
        }
        uint count = args.Info.Count;
        if (args.Info.Flags & CallFlags_CallEval)
        {
            // The final eval arg is a frame display that needs to be marshaled specially.
            args.Values[count-1] = CrossSite::MarshalFrameDisplay(targetScriptContext, (FrameDisplay*)args.Values[count-1]);
            count--;
        }
        for (; i < count; i++)
        {
            args.Values[i] = CrossSite::MarshalVar(targetScriptContext, args.Values[i]);
        }

#ifdef ENABLE_NATIVE_CODEGEN
        CheckCodeGenFunction checkCodeGenFunction = GetCheckCodeGenFunction(entryPoint);
        if (checkCodeGenFunction != null)
        {
            ScriptFunction* callFunc = ScriptFunction::FromVar(function);
            entryPoint = checkCodeGenFunction(callFunc);
            Assert(CrossSite::IsThunk(function->GetEntryPoint()));
        }
#endif

        // We need to setup the caller chain when we go across script site boundary. Property access
        // is OK, and we need to let host know who the caller is when a call is from another script site.
        // CrossSiteObject is the natural palce but it is in the target site. We build up the site
        // chain through PushDispatchExCaller/PopDispatchExCaller, and we call SetCaller in the target site
        // to indicate who the caller is. We first need to get the site from previously pushed site
        // and set that as the caller for current call, and Push a new DispatchExCaller for future calls
        // off this site. GetDispatchExCaller and ReleaseDispatchExCaller is used to get the current caller.
        // currentDispatchExCaller is cached to avoid multiple allocations.
        IUnknown* sourceCaller = NULL, *previousSourceCaller = NULL;
        HRESULT hr = NOERROR;
        Var result = NULL;
        BOOL wasDispatchExCallerPushed = FALSE, wasCallerSet = FALSE;
        __try
        {
            hr = callerHostScriptContext->GetDispatchExCaller((void**)&sourceCaller);

            if (SUCCEEDED(hr))
            {
                hr = calleeHostScriptContext->SetCaller((IUnknown*)sourceCaller, (IUnknown**)&previousSourceCaller);
            }

            if (SUCCEEDED(hr))
            {
                wasCallerSet = TRUE;
                hr = calleeHostScriptContext->PushHostScriptContext();
            }
            if (FAILED(hr))
            {
                // CONSIDER: Should this be callerScriptContext if we failed?
                JavascriptError::MapAndThrowError(targetScriptContext, hr);
            }
            wasDispatchExCallerPushed = TRUE;

            result = JavascriptFunction::CallFunction<true>(function, entryPoint, args);
            ScriptContext* callerScriptContext = callerHostScriptContext->GetScriptContext();
            result = CrossSite::MarshalVar(callerScriptContext, result);
        }
        __finally
        {
            if (sourceCaller != NULL)
            {
                callerHostScriptContext->ReleaseDispatchExCaller(sourceCaller);
            }
            IUnknown* originalCaller = NULL;
            if (wasDispatchExCallerPushed)
            {
                calleeHostScriptContext->PopHostScriptContext();
            }
            if (wasCallerSet)
            {
                calleeHostScriptContext->SetCaller(previousSourceCaller, &originalCaller);
                RELEASEPTR(previousSourceCaller);
                RELEASEPTR(originalCaller);
            }
        }
        Assert(result != NULL);
        return result;
    }

    //  For prototype chain to install cross-site thunk.
    //  when we change prototype using __proto__, those prototype might not have crosssite thunk
    // installed eventhough the CEO is accessed from a different context. During changeprototype time
    // we don't really know where is the requestContext.
    //  Force installing cross-site thunk for all prototype change. it's relatively less frequent used
    // scenario.
    void CrossSite::ForceCrossSiteThunkOnPrototypeChain(RecyclableObject* object)
    {
        if (BinaryFeatureControl::LanguageService())
        {
            return;
        }
        if (TaggedNumber::Is(object))
        {
            return;
        }
        while (DynamicType::Is(object->GetTypeId()) && !JavascriptProxy::Is(object))
        {
            DynamicObject* dynamicObject = DynamicObject::FromVar(object);
            if (!dynamicObject->IsCrossSiteObject() && !dynamicObject->IsExternal())
            {
                // force to install cross-siste thunk on prototype objects.
                dynamicObject->MarshalToScriptContext(nullptr);
            }
            object = object->GetPrototype();
        }
        return;

    }
};
