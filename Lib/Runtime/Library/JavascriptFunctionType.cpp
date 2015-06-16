//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    /*static*/
    PropertyId JavascriptFunction::specialPropertyIds[] = 
    {
        PropertyIds::caller,
        PropertyIds::arguments
    };

    BOOL JavascriptFunction::HasProperty(PropertyId propertyId)
    {
        switch(propertyId)
        {
            case PropertyIds::caller:
            case PropertyIds::arguments:
                return true;
            case PropertyIds::length:
                if (this->IsScriptFunction())
                {
                    return true;
                }
                break;
        }
        return DynamicObject::HasProperty(propertyId);
    }

    BOOL JavascriptFunction::GetAccessors(PropertyId propertyId, Var *getter, Var *setter, ScriptContext * requestContext)
    {
        Assert(!this->IsBoundFunction());
        Assert(propertyId != Constants::NoProperty);
        Assert(getter);
        Assert(setter);
        Assert(requestContext);

        switch(propertyId)
        {
            case PropertyIds::caller:
                if (IsStrictMode())
                {
                    *setter = *getter = requestContext->GetLibrary()->GetThrowTypeErrorCallerAccessorFunction();
                    return true;
                }
                break;

            case PropertyIds::arguments:
                if (IsStrictMode())
                {
                    *setter = *getter = requestContext->GetLibrary()->GetThrowTypeErrorArgumentsAccessorFunction();
                    return true;
                }
                break;
        }

        return __super::GetAccessors(propertyId, getter, setter, requestContext);
    }

    DescriptorFlags JavascriptFunction::GetSetter(PropertyId propertyId, Var *setterValue, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        DescriptorFlags flags;
        if (GetSetterBuiltIns(propertyId, setterValue, info, requestContext, &flags))
        {
            return flags;
        }

        return __super::GetSetter(propertyId, setterValue, info, requestContext);
    }

    DescriptorFlags JavascriptFunction::GetSetter(JavascriptString* propertyNameString, Var *setterValue, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        DescriptorFlags flags;
        PropertyRecord const* propertyRecord;
        this->GetScriptContext()->FindPropertyRecord(propertyNameString, &propertyRecord);

        if (propertyRecord != null && GetSetterBuiltIns(propertyRecord->GetPropertyId(), setterValue, info, requestContext, &flags))
        {
            return flags;
        }

        return __super::GetSetter(propertyNameString, setterValue, info, requestContext);
    }

    bool JavascriptFunction::GetSetterBuiltIns(PropertyId propertyId, Var *setterValue, PropertyValueInfo* info, ScriptContext* requestContext, DescriptorFlags* descriptorFlags)
    {
        Assert(propertyId != Constants::NoProperty);
        Assert(setterValue);
        Assert(requestContext);

        switch(propertyId)
        {
            case PropertyIds::caller:
                PropertyValueInfo::SetNoCache(info, this);
                if(IsStrictMode())
                {
                    *setterValue = requestContext->GetLibrary()->GetThrowTypeErrorCallerAccessorFunction();
                    *descriptorFlags = Accessor;
                }
                else
                {
                    *descriptorFlags = Data;
                }
                return true;

            case PropertyIds::arguments:
                PropertyValueInfo::SetNoCache(info, this);
                if(IsStrictMode())
                {
                    *setterValue = requestContext->GetLibrary()->GetThrowTypeErrorArgumentsAccessorFunction();                    
                    *descriptorFlags = Accessor;
                }
                else
                {
                    *descriptorFlags = Data;
                }
                return true;
        }

        return false;
    }

    BOOL JavascriptFunction::IsConfigurable(PropertyId propertyId)
    {
        if (DynamicObject::GetPropertyIndex(propertyId) == Constants::NoSlot)
        {
            switch (propertyId)
            {
            case PropertyIds::caller:
            case PropertyIds::arguments:
                return false;
            case PropertyIds::length:
                if (this->IsScriptFunction() || this->IsBoundFunction())
                {
                    return true;
                }
                break;
            }
        }
        return DynamicObject::IsConfigurable(propertyId);
    }

    BOOL JavascriptFunction::IsEnumerable(PropertyId propertyId)
    {
        if (DynamicObject::GetPropertyIndex(propertyId) == Constants::NoSlot)
        {
            switch (propertyId)
            {
            case PropertyIds::caller:
            case PropertyIds::arguments:
                return false;
            case PropertyIds::length:
                if (this->IsScriptFunction())
                {
                    return false;
                }
                break;
            }
        }
        return DynamicObject::IsEnumerable(propertyId);
    }

    BOOL JavascriptFunction::IsWritable(PropertyId propertyId)
    {
        if (DynamicObject::GetPropertyIndex(propertyId) == Constants::NoSlot)
        {
            switch (propertyId)
            {
            case PropertyIds::caller:
            case PropertyIds::arguments:
                return false;
            case PropertyIds::length:
                if (this->IsScriptFunction())
                {
                    return false;
                }
                break;
            }
        }
        return DynamicObject::IsWritable(propertyId);
    }

    BOOL JavascriptFunction::GetSpecialPropertyName(uint32 index, Var *propertyName, ScriptContext * requestContext)
    {
        uint length = GetSpecialPropertyCount();
        if (index < length)
        {
            Assert(DynamicObject::GetPropertyIndex(specialPropertyIds[index]) == Constants::NoSlot);
            *propertyName = requestContext->GetPropertyString(specialPropertyIds[index]);
            return true;
        }

        if (index == length)
        {            
            if (this->IsScriptFunction() || this->IsBoundFunction())
            {
                if (DynamicObject::GetPropertyIndex(PropertyIds::length) == Constants::NoSlot)
                {
                    //Only for user defined functions length is a special property.
                    *propertyName = requestContext->GetPropertyString(PropertyIds::length);
                    return true;
                }
            }
        }
        return false;
    }

    // Returns the number of special non-enumerable properties this type has.
    uint JavascriptFunction::GetSpecialPropertyCount() const
    {
        return _countof(specialPropertyIds);
    }

    // Returns the list of special non-enumerable properties for the type.
    PropertyId* JavascriptFunction::GetSpecialPropertyIds() const
    {
        return specialPropertyIds;
    }

    BOOL JavascriptFunction::GetPropertyReference(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        return JavascriptFunction::GetProperty(originalInstance, propertyId, value, info, requestContext);
    }

    JavascriptFunction* JavascriptFunction::FindCaller(BOOL* foundThis, JavascriptFunction* nullValue, ScriptContext* requestContext)
    {
        ScriptContext* scriptContext = this->GetScriptContext();

        JavascriptFunction* funcCaller = nullValue;
        JavascriptStackWalker walker(scriptContext);

        if (walker.WalkToTarget(this))
        {
            *foundThis = TRUE;
            while (walker.GetCaller(&funcCaller))
            {
                if (walker.IsCallerGlobalFunction())
                {
                    // Caller is global/eval. If it's eval, keep looking.
                    // Otherwise, return null.
                    if (walker.IsEvalCaller())
                    {
                        continue;
                    }
                    funcCaller = nullValue;
                }                
                break;
            }

            if (funcCaller->GetScriptContext() != requestContext && funcCaller->GetTypeId() == TypeIds_Null)
            {
                //There are cases where Stackwalker might return null value from different scriptContext
                //Caller of this function expects nullValue from the requestContext.
                funcCaller = nullValue;
            }
        }

        return StackScriptFunction::EnsureBoxed(BOX_PARAM(funcCaller, null, L"caller"));
    }

    BOOL JavascriptFunction::GetCallerProperty(Var originalInstance, Var* value, ScriptContext* requestContext)
    {
        ScriptContext* scriptContext = this->GetScriptContext();
        if (this->IsStrictMode())
        {
            if (scriptContext->GetThreadContext()->RecordImplicitException())
            {
                JavascriptFunction* accessor = requestContext->GetLibrary()->GetThrowTypeErrorCallerAccessorFunction();
                *value = accessor->GetEntryPoint()(accessor, 1, originalInstance);
            }
            return true;
        }

        // Use a stack walker to find this function's frame. If we find it, find its caller.
        BOOL foundThis = FALSE;
        JavascriptFunction* nullValue = (JavascriptFunction*)requestContext->GetLibrary()->GetNull();
        JavascriptFunction* funcCaller = FindCaller(&foundThis, nullValue, requestContext);

        // WOOB #1142373. We are trying to get the caller in window.onerror = function(){alert(arguments.callee.caller);} case
        // window.onerror is called outside of JavascriptFunction::CallFunction loop, so the caller information is not available
        // in the stack to be found by the stack walker. We are doing minimal fix here by retrieving the caller information stored
        // in the exception object, as we already walk the stack at throw time.
        // The down side is that we can only find the top level caller at thrown time, and won't be able to find caller.caller etc.
        // We'll try to fetch the caller only if we can find the function on the stack, but we can't find the caller, and we are in
        // window.onerror scenario.
        *value = funcCaller;
        if (foundThis && funcCaller == nullValue && scriptContext->GetThreadContext()->HasUnhandledException())
        {
            Js::JavascriptExceptionObject* unhandledExceptionObject = scriptContext->GetThreadContext()->GetUnhandledExceptionObject();
            if (unhandledExceptionObject)
            {
                JavascriptFunction* exceptionFunction = unhandledExceptionObject->GetFunction();
                // this is for getcaller in window.onError. in IE8 mode we never return cross site caller; the behavior is different in
                // different browser, and neither FF & chrome can get the caller here.
                // we should be fine just return NULL.
                if (exceptionFunction && scriptContext == exceptionFunction->GetScriptContext())
                {
                    *value = exceptionFunction;
                }
            }
        }
        else if (foundThis && scriptContext != funcCaller->GetScriptContext())
        {
            HRESULT hr = scriptContext->GetHostScriptContext()->CheckCrossDomainScriptContext(funcCaller->GetScriptContext());
            if (S_OK != hr)
            {
                *value = scriptContext->GetLibrary()->GetNull();
            }
        }

        if (Js::JavascriptFunction::Is(*value) && Js::JavascriptFunction::FromVar(*value)->IsStrictMode())
        {
            if (scriptContext->GetThreadContext()->RecordImplicitException())
            {
                // ES5.15.3.5.4 [[Get]] (P) -- access to the 'caller' property of strict mode function results in TypeError.
                // Note that for caller coming from remote context (see the check right above) we can't call IsStrictMode()
                // unless CheckCrossDomainScriptContext succeeds. If it fails we don't know whether caller is strict mode
                // function or not and throw if it's not, so just return Null.
                JavascriptError::ThrowTypeError(scriptContext, JSERR_AccessCaller);
            }
        }

        return true;
    }

    BOOL JavascriptFunction::GetArgumentsProperty(Var originalInstance, Var* value, ScriptContext* requestContext)
    {
        ScriptContext* scriptContext = this->GetScriptContext();
        if (this->IsStrictMode())
        {
            if (scriptContext->GetThreadContext()->RecordImplicitException())
            {
                JavascriptFunction* accessor = requestContext->GetLibrary()->GetThrowTypeErrorArgumentsAccessorFunction();
                *value = accessor->GetEntryPoint()(accessor, 1, originalInstance);
            }
            return true;
        }

        if (!this->IsScriptFunction())
        {
            // builtin function do not have an argument object - return null.
            *value = scriptContext->GetLibrary()->GetNull();
            return true;
        }

        // Use a stack walker to find this function's frame. If we find it, compute its arguments.
        // Note that we are currently unable to guarantee that the binding between formal arguments
        // and foo.arguments[n] will be maintained after this object is returned.

        JavascriptStackWalker walker(scriptContext);

        if (walker.WalkToTarget(this))
        {
            if (walker.IsCallerGlobalFunction())
            {
                *value = requestContext->GetLibrary()->GetNull();
            }
            else
            {
                Var args = walker.GetPermanentArguments();

                if (args == NULL)
                {
                    CallInfo const *callInfo = walker.GetCallInfo();
                    args = JavascriptOperators::LoadHeapArguments(
                        this, callInfo->Count - 1,
                        walker.GetJavascriptArgs(),
                        scriptContext->GetLibrary()->GetNull(),
                        scriptContext->GetLibrary()->GetNull(),
                        scriptContext,
                        /* formalsAreLetDecls */ false);

                    walker.SetPermanentArguments(args);
                }

                *value = args;
            }
        }
        else
        {
            *value = scriptContext->GetLibrary()->GetNull();
        }
        return true;
    }

    BOOL JavascriptFunction::GetProperty(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        BOOL result = DynamicObject::GetProperty(originalInstance, propertyId, value, info, requestContext);

        if (result)
        {
            if (propertyId == PropertyIds::prototype)
            {
                PropertyValueInfo::DisableStoreFieldCache(info);
            }
        }
        else
        {
            GetPropertyBuiltIns(originalInstance, propertyId, value, requestContext, &result);
        }
        
        return result;
    }

    BOOL JavascriptFunction::GetProperty(Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        BOOL result;
        PropertyRecord const* propertyRecord;
        this->GetScriptContext()->FindPropertyRecord(propertyNameString, &propertyRecord);

        result = DynamicObject::GetProperty(originalInstance, propertyNameString, value, info, requestContext);
        if (result)
        {
            if (propertyRecord != null && propertyRecord->GetPropertyId() == PropertyIds::prototype)
            {
                PropertyValueInfo::DisableStoreFieldCache(info);
            }

            return result;
        }

        if (propertyRecord != null)
        {
            GetPropertyBuiltIns(originalInstance, propertyRecord->GetPropertyId(), value, requestContext, &result);
        }

        return result;
    }

    bool JavascriptFunction::GetPropertyBuiltIns(Var originalInstance, PropertyId propertyId, Var* value, ScriptContext* requestContext, BOOL* result)
    {
        if (propertyId == PropertyIds::caller)
        {
            *result = GetCallerProperty(originalInstance, value, requestContext);
            return true;
        }

        if (propertyId == PropertyIds::arguments)
        {
            *result = GetArgumentsProperty(originalInstance, value, requestContext);
            return true;
        }

        if (propertyId == PropertyIds::length)
        {
            FunctionProxy *proxy = this->GetFunctionProxy();
            if (proxy)
            {
                *value = TaggedInt::ToVarUnchecked(proxy->EnsureDeserialized()->GetReportedInParamsCount() - 1);
                *result = true;
                return true;
            }
        }

        return false;
    }

    BOOL JavascriptFunction::SetProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        bool isReadOnly = false;
        switch(propertyId)
        {
            case PropertyIds::caller:
                if(IsStrictMode())
                {
                    const auto thrower = GetLibrary()->GetThrowTypeErrorCallerAccessorFunction();
                    thrower->GetEntryPoint()(thrower, 1, this);
                    return false;
                }
                isReadOnly = true;
                break;

            case PropertyIds::arguments:
                if(IsStrictMode())
                {
                    const auto thrower = GetLibrary()->GetThrowTypeErrorArgumentsAccessorFunction();
                    thrower->GetEntryPoint()(thrower, 1, this);
                    return false;
                }
                isReadOnly = true;
                break;

            case PropertyIds::length:
                if (this->IsScriptFunction())
                {
                    isReadOnly = true;
                }
                break;

        }

        if(isReadOnly)
        {
            JavascriptError::ThrowCantAssignIfStrictMode(flags, this->GetScriptContext());
            return false;
        }

        BOOL result =  DynamicObject::SetProperty(propertyId, value, flags, info);

        if (propertyId == PropertyIds::prototype)
        {
            PropertyValueInfo::SetNoCache(info, this);
            InvalidateConstructorCacheOnPrototypeChange();
            this->GetScriptContext()->GetThreadContext()->InvalidateIsInstInlineCachesForFunction(this);
        }

        return result;
    }

    BOOL JavascriptFunction::SetPropertyWithAttributes(PropertyId propertyId, Var value, PropertyAttributes attributes, PropertyValueInfo* info, PropertyOperationFlags flags, SideEffects possibleSideEffects)
    {
        BOOL result = __super::SetPropertyWithAttributes(propertyId, value, attributes, info, flags, possibleSideEffects);

        if (propertyId == PropertyIds::prototype)
        {
            PropertyValueInfo::SetNoCache(info, this);
            InvalidateConstructorCacheOnPrototypeChange();
            this->GetScriptContext()->GetThreadContext()->InvalidateIsInstInlineCachesForFunction(this);
        }

        return result;
    }

    BOOL JavascriptFunction::SetProperty(JavascriptString* propertyNameString, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        PropertyRecord const * propertyRecord;
        this->GetScriptContext()->FindPropertyRecord(propertyNameString, &propertyRecord);

        if (propertyRecord != null)
        {
            return JavascriptFunction::SetProperty(propertyRecord->GetPropertyId(), value, flags, info);
        }
        else
        {
            return DynamicObject::SetProperty(propertyNameString, value, flags, info);
        }
    }

    BOOL JavascriptFunction::DeleteProperty(PropertyId propertyId, PropertyOperationFlags flags)
    {
        switch(propertyId)
        {
            case PropertyIds::caller:
            case PropertyIds::arguments:
                JavascriptError::ThrowCantDeleteIfStrictMode(flags, this->GetScriptContext(), this->GetScriptContext()->GetPropertyName(propertyId)->GetBuffer());
                return false;
            case PropertyIds::length:
                if( this->IsScriptFunction())
                {
                    JavascriptError::ThrowCantDeleteIfStrictMode(flags, this->GetScriptContext(), this->GetScriptContext()->GetPropertyName(propertyId)->GetBuffer());
                    return false;
                }
                break;
        }

        BOOL result = DynamicObject::DeleteProperty(propertyId, flags);

#if TRUE
        // It looks like in certain legacy modes (-version:1) we actually permit the prototype property to be deleted.
        // If that happens, we better invalidate relevant caches.
        if (result && propertyId == PropertyIds::prototype)
        {
            InvalidateConstructorCacheOnPrototypeChange();
            this->GetScriptContext()->GetThreadContext()->InvalidateIsInstInlineCachesForFunction(this);
        }
#else
        AssertMsg(propertyId != PropertyIds::prototype || !result, "It shouldn't be possible to delete a prototype property of a function.");
#endif

        return result;
    }

    void JavascriptFunction::InvalidateConstructorCacheOnPrototypeChange()
    {
        Assert(this->constructorCache != null);

#if DBG_DUMP
        if (PHASE_TRACE1(Js::ConstructorCachePhase))
        {
            // This is under DBG_DUMP so we can allow a check
            ParseableFunctionInfo* body = this->GetFunctionProxy() != null ? this->GetFunctionProxy()->EnsureDeserialized() : null;
            const wchar_t* ctorName = body != null ? body->GetDisplayName() : L"<unknown>";
            
            wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];
            
            Output::Print(L"CtorCache: before invalidating cache (0x%p) for ctor %s (%s): ", this->constructorCache, ctorName, 
                body ? body->GetDebugNumberSet(debugStringBuffer) : L"(null)");
            this->constructorCache->Dump();
            Output::Print(L"\n");
            Output::Flush();
        }
#endif

        this->constructorCache->InvalidateOnPrototypeChange();

#if DBG_DUMP
        if (PHASE_TRACE1(Js::ConstructorCachePhase))
        {
            // This is under DBG_DUMP so we can allow a check
            ParseableFunctionInfo* body = this->GetFunctionProxy() != null ? this->GetFunctionProxy()->EnsureDeserialized() : null;
            const wchar_t* ctorName = body != null ? body->GetDisplayName() : L"<unknown>";
            wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];
           
            Output::Print(L"CtorCache: after invalidating cache (0x%p) for ctor %s (%s): ", this->constructorCache, ctorName,
                body ? body->GetDebugNumberSet(debugStringBuffer) : L"(null)");
            this->constructorCache->Dump();
            Output::Print(L"\n");
            Output::Flush();
        }
#endif

    }

    BOOL JavascriptFunction::GetDiagValueString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        JavascriptString * pString = NULL;

        Var sourceString = this->GetSourceString();

        // Taken mostly from JavascriptFunction::EntryToString, but this function will not change the state of pFunc->sourceString

        if (sourceString == null)
        {
            FunctionProxy* proxy = this->GetFunctionProxy();
            if(proxy)
            {
                ParseableFunctionInfo * func = proxy->EnsureDeserialized();
                Utf8SourceInfo* sourceInfo = func->GetUtf8SourceInfo();
                if(sourceInfo->GetIsLibraryCode())
                {
                    size_t displayNameLength = 0;
                    pString = JavascriptFunction::GetLibraryCodeDisplayString(this->GetScriptContext(), func->GetShortDisplayName(&displayNameLength));
                }
                else
                {
                    charcount_t count = min(DIAG_MAX_FUNCTION_STRING, func->LengthInChars());
                    utf8::DecodeInto(stringBuilder->AllocBufferSpace(count), func->GetSource(L"JavascriptFunction::GetDiagValueString"), count, utf8::doAllowThreeByteSurrogates);
                    stringBuilder->IncreaseCount(count);
                    return TRUE;
                }
            }
            else
            {
                pString = GetLibrary()->GetFunctionDisplayString();
            }
        }
        else
        {
            if (TaggedInt::Is(sourceString))
            {
                pString = GetNativeFunctionDisplayString(this->GetScriptContext(), this->GetScriptContext()->GetPropertyString(TaggedInt::ToInt32(sourceString)));
            }
            else
            {
                Assert(JavascriptString::Is(sourceString));
                pString = JavascriptString::FromVar(sourceString);
            }
        }

        Assert(pString);
        stringBuilder->Append(pString->GetString(), pString->GetLength());

        return TRUE;
    }

    BOOL JavascriptFunction::GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        stringBuilder->AppendCppLiteral(L"Object, (Function)");
        return TRUE;
    }

    JavascriptString* JavascriptFunction::GetDisplayNameImpl() const
    {
        Assert(this->GetFunctionProxy() != nullptr); // The caller should guarantee a proxy exists
        ParseableFunctionInfo * func = this->GetFunctionProxy()->EnsureDeserialized();
        size_t length = 0;
        const wchar_t* name = func->GetShortDisplayName(&length);
        
        return DisplayNameHelper(name, length);
    }

    JavascriptString* JavascriptFunction::DisplayNameHelper(const wchar_t* name, charcount_t length) const
    {
        ScriptContext* scriptContext = this->GetScriptContext();
        Assert(this->GetFunctionProxy() != nullptr); // The caller should guarantee a proxy exists
        ParseableFunctionInfo * func = this->GetFunctionProxy()->EnsureDeserialized();
        if (wcscmp(func->GetDisplayName(), Js::Constants::AnonymousFunction) == 0)
        {
            return LiteralString::CreateEmptyString(scriptContext->GetLibrary()->GetStringTypeStatic());
        }
        else if (wcscmp(func->GetDisplayName(), Js::Constants::FunctionCode) == 0)
        {
            return LiteralString::NewCopyBuffer(Js::Constants::Anonymous, Js::Constants::AnonymousLength, scriptContext);
        }
        else if (func->GetIsAccessor())
        {
            const wchar_t* accessorName = func->GetDisplayName();
            if (accessorName[0] == L'g')
            {
                return LiteralString::Concat(LiteralString::NewCopySz(L"get ", scriptContext), LiteralString::NewCopyBuffer(name, length, scriptContext));
            }
            AssertMsg(accessorName[0] == L's', "should be a set");
            return LiteralString::Concat(LiteralString::NewCopySz(L"set ", scriptContext), LiteralString::NewCopyBuffer(name, length, scriptContext));
        }
        return LiteralString::NewCopyBuffer(name, length, scriptContext);
    }

    JavascriptString* JavascriptFunction::GetDisplayName(bool isFunctionName) const
    {
        ScriptContext* scriptContext = this->GetScriptContext();
        Var sourceString = this->GetSourceString();
        FunctionProxy* proxy = this->GetFunctionProxy();
        

        if (isFunctionName && (proxy || BoundFunction::Is(const_cast<JavascriptFunction*>(this)) ||
            JavascriptGeneratorFunction::Is(const_cast<JavascriptFunction*>(this))))
        {
            return GetDisplayNameImpl();
        }

        if (proxy)
        {
            ParseableFunctionInfo * func = proxy->EnsureDeserialized();
            return LiteralString::NewCopySz(func->GetDisplayName(), scriptContext);
        }
        if (sourceString)
        {
            if (TaggedInt::Is(sourceString))
            {
                return scriptContext->GetPropertyString(TaggedInt::ToInt32(sourceString));
            }
            Assert(JavascriptString::Is(sourceString));
            return JavascriptString::FromVar(sourceString);
        }

        if (isFunctionName)
        {
            return LiteralString::CreateEmptyString(scriptContext->GetLibrary()->GetStringTypeStatic()); // empty string for function prototype hits here
        }
        return scriptContext->GetLibrary()->GetFunctionDisplayString(); //TODO
    }

    Var JavascriptFunction::GetTypeOfString(ScriptContext * requestContext)
    {
        return requestContext->GetLibrary()->GetFunctionTypeDisplayString();
    }

    // Check if this function is native/script library code
    bool JavascriptFunction::IsLibraryCode() const
    {
        return !this->IsScriptFunction() || this->GetFunctionBody()->GetUtf8SourceInfo()->GetIsLibraryCode();
    }

    BOOL JavascriptFunction::HasInstance(Var instance, ScriptContext* scriptContext, IsInstInlineCache* inlineCache)
    {
        Var funcPrototype;

        if (this->GetTypeHandler()->GetHasKnownSlot0())
        {
            Assert(this->GetDynamicType()->GetTypeHandler()->GetPropertyId(scriptContext, (PropertyIndex)0) == PropertyIds::prototype);
            funcPrototype = this->GetSlot(0);
        }
        else
        {
            funcPrototype = JavascriptOperators::GetProperty(this, PropertyIds::prototype, scriptContext, NULL);
        }
        funcPrototype = CrossSite::MarshalVar(scriptContext, funcPrototype);
        return JavascriptFunction::HasInstance(funcPrototype, instance, scriptContext, inlineCache, this);
    }

    BOOL JavascriptFunction::HasInstance(Var funcPrototype, Var instance, ScriptContext * scriptContext, IsInstInlineCache* inlineCache, JavascriptFunction *function)
    {
        BOOL result = FALSE;
        JavascriptBoolean * javascriptResult;
       
        //
        // if "instance" is not a JavascriptObject, return false
        //
        if (!JavascriptOperators::IsObject(instance))
        {               
            // Only update the cache for primitive cache if it is empty already for the JIT fast path
            if (inlineCache && inlineCache->function == null 
                && scriptContext == function->GetScriptContext())// only register when function has same scriptContext
            {
                inlineCache->Cache(RecyclableObject::Is(instance)?
                    RecyclableObject::FromVar(instance)->GetType() : null, 
                    function, scriptContext->GetLibrary()->GetFalse(), scriptContext);
            }
            return result;
        }

        // If we have an instance of inline cache, let's try to use it to speed up the operation.
        // We would like to catch all cases when we already know (by having checked previously) 
        // that an object on the left of instance of has been created by a function on the right,
        // as well as when we already know the object on the left has not been created by a function on the right.
        // In practice, we can do so only if the function matches the function in the cache, and the object's type matches the
        // type in the cache.  Notably, this typically means that if some of the objects evolved after construction,
        // while others did not, we will miss the cache for one of the two (sets of objects).
        // An important subtelty here arises when a function is called from different script contexts.
        // Suppose we called function foo from script context A, and we pass it an object o created in the same script context.
        // When function foo checks if object o is an instance of itself (function foo) for the first time (from context A) we will
        // populate the cache with function foo and object o's type (which is permanently bound to the script context A,
        // in which object o was created). If we later later invoked function foo from script context B and perform the same instance-of check,
        // the function will still match the function in the cache (because objects' identities do not change during cross-context marshalling). 
        // However, object o's type (even if it is of the same "shape" as before) will be different, because the object types are permanently
        // bound and unique to the script context from which they were created.  Hence, the cache may miss, even if the function matches.
        if (inlineCache != NULL)
        {
            Assert(function != NULL);
            if (inlineCache->TryGetResult(instance, function, &javascriptResult))
            {
                return javascriptResult == scriptContext->GetLibrary()->GetTrue();
            }
        }

        // If we are here, then me must have missed the cache.  This may be because:
        // a) the cache has never been populated in the first place,
        // b) the cache has been populated, but for an object of a different type (even if the object was created by the same constructor function),
        // c) the cache has been populated, but for a different function,
        // d) the cache has been populated, even for the same object type and function, but has since been invalidated, because the function's
        //    prototype property has been changed (see JavascriptFunction::SetProperty and ThreadContext::InvalidateIsInstInlineCachesForFunction).
        // TODO (jedmiad): Create unit tests for each of the above conditions.
        // Curiously, we may even miss the cache if we ask again about the very same object the very same function the cache was populated with.
        // This subtelty arises when a function is called from two (or more) different script contexts.
        // Suppose we called function foo from script context A, and passed it an object o created in the same script context.
        // When function foo checks if object o is an instance of itself (function foo) for the first time (from context A) we will
        // populate the cache with function foo and object o's type (which is permanently bound to the script context A,
        // in which object o was created). If we later later invoked function foo from script context B and perform the same instance of check,
        // the function will still match the function in the cache (because objects' identities do not change during cross-context marshalling). 
        // However, object o's type (even if it is of the same "shape" as before, and even if o is the very same object) will be different, 
        // because the object types are permanently bound and unique to the script context from which they were created.

        Var prototype = JavascriptOperators::GetPrototype(RecyclableObject::FromVar(instance));

        if (!JavascriptOperators::IsObject(funcPrototype))
        {            
            JavascriptError::ThrowTypeError(scriptContext, JSERR_InvalidPrototype);
        }

        // Since we missed the cache, we must now walk the prototype chain of the object to check if the given function's prototype is somewhere in
        // that chain.  If it is, we return true.  Otherwise, i.e. we hit the end of the chain before finding the function's prototype, we return false.
        while (JavascriptOperators::GetTypeId(prototype) != TypeIds_Null)
        {
            if (prototype == funcPrototype)
            {
                result = TRUE;
                break;
            }

            prototype = JavascriptOperators::GetPrototype(RecyclableObject::FromVar(prototype));
        }

        // Now that we know the answer, let's cache it for next time if we have a cache.
        if (inlineCache != NULL)
        {
            Assert(function != NULL);
            JavascriptBoolean * boolResult = result ? scriptContext->GetLibrary()->GetTrue() : 
                scriptContext->GetLibrary()->GetFalse(); 
            Type * instanceType = RecyclableObject::FromVar(instance)->GetType();

            if (!instanceType->HasSpecialPrototype()
                && scriptContext == function->GetScriptContext()) // only register when function has same scriptContext, otherwise when scriptContext close 
                                                                  // and the isinst inline cache chain will be broken by clearing the arenaAllocator
            {
                inlineCache->Cache(instanceType, function, boolResult, scriptContext);
            }
        }

        return result;
    }
}
