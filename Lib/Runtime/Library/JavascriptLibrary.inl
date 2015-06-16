//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    inline DynamicType * JavascriptLibrary::GetObjectLiteralType(uint16 requestedInlineSlotCapacity)
    {
        if (requestedInlineSlotCapacity <= MaxPreInitializedObjectTypeInlineSlotCount)
        {
            return objectTypes[DynamicTypeHandler::RoundUpInlineSlotCapacity(requestedInlineSlotCapacity) / InlineSlotCountIncrement];
        }
        else
        {
            return objectTypes[PreInitializedObjectTypeCount - 1];
        }
    }

    inline DynamicType * JavascriptLibrary::GetObjectHeaderInlinedLiteralType(uint16 requestedInlineSlotCapacity)
    {
        Assert(requestedInlineSlotCapacity <= MaxPreInitializedObjectHeaderInlinedTypeInlineSlotCount);

        return
            objectHeaderInlinedTypes[
                (
                    DynamicTypeHandler::RoundUpObjectHeaderInlinedInlineSlotCapacity(requestedInlineSlotCapacity) -
                    DynamicTypeHandler::GetObjectHeaderInlinableSlotCapacity()
                ) / InlineSlotCountIncrement];
    }

    inline HeapArgumentsObject* JavascriptLibrary::CreateHeapArguments(Var frameObj, uint32 formalCount)
    {
        AssertMsg(heapArgumentsType, "Where's heapArgumentsType?");

        Recycler *recycler = this->GetRecycler();
        
        if (!this->arrayPrototypeValuesFunction) //InitializeArrayPrototype can be delay loaded, which could prevent us from access to array.prototype.values
        {
            this->arrayPrototypeValuesFunction = DefaultCreateFunction(&JavascriptArray::EntryInfo::Values, 0, nullptr, nullptr, PropertyIds::values);
        }
        return RecyclerNew(recycler, HeapArgumentsObject, recycler, (ActivationObject*)frameObj, formalCount, heapArgumentsType);
    }

    inline JavascriptArray* JavascriptLibrary::CreateArray()
    {
        AssertMsg(arrayType, "Where's arrayType?");
        return JavascriptArray::New<Var, JavascriptArray>(this->GetRecycler(), arrayType);
    }

    inline JavascriptArray* JavascriptLibrary::CreateArray(uint32 length)
    {
        AssertMsg(arrayType, "Where's arrayType?");
        JavascriptArray* arr = JavascriptArray::New<Var, JavascriptArray, 0>(length, arrayType, this->GetRecycler());
        JSETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_ARRAY(arr));

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        arr->CheckForceES5Array();
#endif
        return arr;
    }

    inline JavascriptArray *JavascriptLibrary::CreateArrayOnStack(void *const stackAllocationPointer)
    {
        return JavascriptArray::New<JavascriptArray, 0>(stackAllocationPointer, 0, arrayType);
    }

    inline JavascriptNativeIntArray* JavascriptLibrary::CreateNativeIntArray()
    {
        AssertMsg(nativeIntArrayType, "Where's nativeIntArrayType?");
        return JavascriptArray::New<int32, JavascriptNativeIntArray>(this->GetRecycler(), nativeIntArrayType);
    }

    inline JavascriptNativeIntArray* JavascriptLibrary::CreateNativeIntArray(uint32 length)
    {
        AssertMsg(nativeIntArrayType, "Where's nativeIntArrayType?");
        JavascriptNativeIntArray* arr = JavascriptArray::New<int32, JavascriptNativeIntArray, 0>(length, nativeIntArrayType, this->GetRecycler());
        JSETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_ARRAY(arr));

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        arr->CheckForceES5Array();
#endif
        return arr;
    }

    inline JavascriptNativeFloatArray* JavascriptLibrary::CreateNativeFloatArray()
    {
        AssertMsg(nativeFloatArrayType, "Where's nativeFloatArrayType?");
        return JavascriptArray::New<double, JavascriptNativeFloatArray>(this->GetRecycler(), nativeFloatArrayType);
    }

    inline JavascriptNativeFloatArray* JavascriptLibrary::CreateNativeFloatArray(uint32 length)
    {
        AssertMsg(nativeFloatArrayType, "Where's nativeIntArrayType?");
        JavascriptNativeFloatArray* arr = JavascriptArray::New<double, JavascriptNativeFloatArray, 0>(length, nativeFloatArrayType, this->GetRecycler());
        JSETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_ARRAY(arr));

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        arr->CheckForceES5Array();
#endif
        return arr;
    }

    inline JavascriptArray* JavascriptLibrary::CreateArrayLiteral(uint32 length)
    {
        AssertMsg(arrayType, "Where's arrayType?");
        JavascriptArray* arr = JavascriptArray::NewLiteral<Var, JavascriptArray, 0>(length, arrayType, this->GetRecycler());
        JSETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_ARRAY(arr));

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        arr->CheckForceES5Array();
#endif
        return arr;
    }

    inline JavascriptNativeIntArray* JavascriptLibrary::CreateNativeIntArrayLiteral(uint32 length)
    {
        AssertMsg(nativeIntArrayType, "Where's arrayType?");
        JavascriptNativeIntArray* arr = JavascriptArray::NewLiteral<int32, JavascriptNativeIntArray, 0>(length, nativeIntArrayType, this->GetRecycler());
        JSETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_ARRAY(arr));

        return arr;
    }

    inline JavascriptNativeIntArray* JavascriptLibrary::CreateCopyOnAccessNativeIntArrayLiteral(ArrayCallSiteInfo *arrayInfo, FunctionBody *functionBody, const Js::AuxArray<int32> *ints)
    {
        AssertMsg(copyOnAccessNativeIntArrayType, "Where's arrayType?");
        JavascriptNativeIntArray* arr = JavascriptArray::NewCopyOnAccessLiteral<int32, JavascriptCopyOnAccessNativeIntArray, 0>(copyOnAccessNativeIntArrayType, arrayInfo, functionBody, ints, this->GetRecycler());
        JSETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_ARRAY(arr));

        return arr;
    }

    inline JavascriptNativeFloatArray* JavascriptLibrary::CreateNativeFloatArrayLiteral(uint32 length)
    {
        AssertMsg(nativeFloatArrayType, "Where's arrayType?");
        JavascriptNativeFloatArray* arr = JavascriptArray::NewLiteral<double, JavascriptNativeFloatArray, 0>(length, nativeFloatArrayType, this->GetRecycler());
        JSETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_ARRAY(arr));

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        arr->CheckForceES5Array();
#endif
        return arr;
    }

    inline JavascriptArray* JavascriptLibrary::CreateArray(uint32 length, uint32 size)
    {
        AssertMsg(arrayType, "Where's arrayType?");
        JavascriptArray* arr = RecyclerNew(this->GetRecycler(), JavascriptArray, length, size, arrayType);
        JSETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_ARRAY(arr));

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        arr->CheckForceES5Array();
#endif
        return arr;
    }

    inline ArrayBuffer* JavascriptLibrary::CreateArrayBuffer(uint32 length)
    {
        ArrayBuffer* arr = JavascriptArrayBuffer::Create(length, arrayBufferType);
        return arr;
    }

    inline ArrayBuffer* JavascriptLibrary::CreateArrayBuffer(byte* buffer, uint32 length)
    {
        ArrayBuffer* arr = JavascriptArrayBuffer::Create(buffer, length, arrayBufferType);
        return arr;
    }

    inline ArrayBuffer* JavascriptLibrary::CreateProjectionArraybuffer(uint32 length)
    {
        ArrayBuffer* arr = ProjectionArrayBuffer::Create(length, arrayBufferType);
        JSETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_OBJECT(arr));
        return arr;
    }

    inline ArrayBuffer* JavascriptLibrary::CreateProjectionArraybuffer(byte* buffer, uint32 length)
    {
        ArrayBuffer* arr = ProjectionArrayBuffer::Create(buffer, length, arrayBufferType);
        JSETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_OBJECT(arr));
        return arr;
    }

    inline DataView* JavascriptLibrary::CreateDataView(ArrayBuffer* arrayBuffer, uint32 offset, uint32 length)
    {
        DataView* dataView = RecyclerNew(this->GetRecycler(), DataView, arrayBuffer, offset, length, dataViewType);

        // Only add these members in pre-ES6 modes. After ES6, these live on Dataview.prototype as getters.
        if (!scriptContext->GetConfig()->IsKhronosInteropEnabled())
        {
            AddMember(dataView, PropertyIds::buffer, arrayBuffer, PropertyNone);
            AddMember(dataView, PropertyIds::byteOffset, JavascriptNumber::ToVar(offset, scriptContext), PropertyNone);
            AddMember(dataView, PropertyIds::byteLength, JavascriptNumber::ToVar(length, scriptContext), PropertyNone);
        }

        return dataView;
    }

    inline JavascriptPixelArray* JavascriptLibrary::CreatePixelArray(uint32 length)
    {
        AssertMsg(pixelArrayType, "Where's pixelArrayType?");
        JavascriptPixelArray* newArray = RecyclerNewFinalized(this->GetRecycler(), JavascriptPixelArray, length, pixelArrayType);
        return newArray;
    }

    inline JavascriptBoolean* JavascriptLibrary::CreateBoolean(BOOL value)
    {
        AssertMsg(booleanTrue, "Where's booleanTrue?");
        AssertMsg(booleanFalse, "Where's booleanFalse?");
        return value ? booleanTrue : booleanFalse;
    }

    inline JavascriptDate* JavascriptLibrary::CreateDate()
    {
        AssertMsg(dateType, "Where's dateType?");
        return RecyclerNew(this->GetRecycler(), JavascriptDate, 0, dateType);
    }

    inline JavascriptDate* JavascriptLibrary::CreateDate(double value)
    {
        AssertMsg(dateType, "Where's dateType?");
        return RecyclerNew(this->GetRecycler(), JavascriptDate, value, dateType);
    }

    inline JavascriptDate* JavascriptLibrary::CreateDate(SYSTEMTIME* pst)
    {
        AssertMsg(dateType, "Where's dateType?");
        double value = DateImplementation::TimeFromSt(pst);
        return CreateDate(value);
    }

    inline JavascriptMap* JavascriptLibrary::CreateMap()
    {
        AssertMsg(mapType, "Where's mapType?");
        return RecyclerNew(this->GetRecycler(), JavascriptMap, mapType);
    }

    inline JavascriptSet* JavascriptLibrary::CreateSet()
    {
        AssertMsg(setType, "Where's setType?");
        return RecyclerNew(this->GetRecycler(), JavascriptSet, setType);
    }

    inline JavascriptWeakMap* JavascriptLibrary::CreateWeakMap()
    {
        AssertMsg(weakMapType, "Where's weakMapType?");
        return RecyclerNewFinalized(this->GetRecycler(), JavascriptWeakMap, weakMapType);
    }

    inline JavascriptWeakSet* JavascriptLibrary::CreateWeakSet()
    {
        AssertMsg(weakSetType, "Where's weakSetType?");
        return RecyclerNewFinalized(this->GetRecycler(), JavascriptWeakSet, weakSetType);
    }

    inline JavascriptPromise* JavascriptLibrary::CreatePromise()
    {
        AssertMsg(promiseType, "Where's promiseType?");
        return RecyclerNew(this->GetRecycler(), JavascriptPromise, promiseType);
    }

    inline JavascriptGenerator* JavascriptLibrary::CreateGenerator(Arguments& args, ScriptFunction* scriptFunction, RecyclableObject* prototype)
    {
        Assert(scriptContext->GetConfig()->IsES6GeneratorsEnabled());
        DynamicType* generatorType = CreateGeneratorType(prototype);
        return RecyclerNew(this->GetRecycler(), JavascriptGenerator, generatorType, args, scriptFunction);
    }

    inline JavascriptError* JavascriptLibrary::CreateError()
    {
        AssertMsg(errorType, "Where's errorType?");
        JavascriptError *pError = RecyclerNew(this->GetRecycler(), JavascriptError, errorType);
        JavascriptError::SetErrorType(pError, kjstError);
        return pError;
    }

    inline JavascriptSymbol* JavascriptLibrary::CreateSymbol(JavascriptString* description)
    {
        return this->CreateSymbol(description->GetString(), (int)description->GetLength());
    }

    inline JavascriptSymbol* JavascriptLibrary::CreateSymbol(const wchar_t* description, int descriptionLength)
    {
        ENTER_PINNED_SCOPE(const Js::PropertyRecord, propertyRecord);

        propertyRecord = this->scriptContext->GetThreadContext()->UncheckedAddPropertyId(description, descriptionLength, /*bind*/false, /*isSymbol*/true);

        LEAVE_PINNED_SCOPE();

        return this->CreateSymbol(propertyRecord);
    }

    inline JavascriptSymbol* JavascriptLibrary::CreateSymbol(const PropertyRecord* propertyRecord)
    {
        AssertMsg(symbolTypeStatic, "Where's symbolTypeStatic?");
        return RecyclerNew(this->GetRecycler(), JavascriptSymbol, propertyRecord, symbolTypeStatic);
    }

    inline JavascriptError* JavascriptLibrary::CreateExternalError(ErrorTypeEnum errorTypeEnum)
    {
        DynamicType* baseErrorType = NULL;
        switch (errorTypeEnum)
        {
        case kjstError:
        default:
            baseErrorType = errorType;
            break;
        case kjstRangeError:
            baseErrorType = rangeErrorType;
            break;
        case kjstReferenceError:
            baseErrorType = referenceErrorType;
            break;
        case kjstSyntaxError:
            baseErrorType = syntaxErrorType;
            break;
        case kjstTypeError:
            baseErrorType = typeErrorType;
            break;
        case kjstURIError:
            baseErrorType = uriErrorType;
            break;
        }

        JavascriptError *pError = RecyclerNew(recycler, JavascriptError, baseErrorType, TRUE);
        JavascriptError::SetErrorType(pError, errorTypeEnum);
        return pError;
    }

    inline JavascriptError* JavascriptLibrary::CreateEvalError()
    {
        AssertMsg(evalErrorType, "Where's evalErrorType?");
        JavascriptError *pError = RecyclerNew(this->GetRecycler(), JavascriptError, evalErrorType);
        JavascriptError::SetErrorType(pError, kjstEvalError);
        return pError;
    }

    inline JavascriptError* JavascriptLibrary::CreateRangeError()
    {
        AssertMsg(rangeErrorType, "Where's rangeErrorType?");
        JavascriptError *pError = RecyclerNew(this->GetRecycler(), JavascriptError, rangeErrorType);
        JavascriptError::SetErrorType(pError, kjstRangeError);
        return pError;
    }

    inline JavascriptError* JavascriptLibrary::CreateReferenceError()
    {
        AssertMsg(referenceErrorType, "Where's referenceErrorType?");
        JavascriptError *pError = RecyclerNew(this->GetRecycler(), JavascriptError, referenceErrorType);
        JavascriptError::SetErrorType(pError, kjstReferenceError);
        return pError;
    }

    inline JavascriptError* JavascriptLibrary::CreateSyntaxError()
    {
        AssertMsg(syntaxErrorType, "Where's syntaxErrorType?");
        JavascriptError *pError = RecyclerNew(this->GetRecycler(), JavascriptError, syntaxErrorType);
        JavascriptError::SetErrorType(pError, kjstSyntaxError);
        return pError;
    }

    inline JavascriptError* JavascriptLibrary::CreateTypeError()
    {
        AssertMsg(typeErrorType, "Where's typeErrorType?");
        JavascriptError *pError = RecyclerNew(this->GetRecycler(), JavascriptError, typeErrorType);
        JavascriptError::SetErrorType(pError, kjstTypeError);
        return pError;
    }

    inline JavascriptError* JavascriptLibrary::CreateURIError()
    {
        AssertMsg(uriErrorType, "Where's uriErrorType?");
        JavascriptError *pError = RecyclerNew(this->GetRecycler(), JavascriptError, uriErrorType);
        JavascriptError::SetErrorType(pError, kjstURIError);
        return pError;
    }

    inline JavascriptError* JavascriptLibrary::CreateStackOverflowError()
    {
#if DBG
        // If we are doing a heap enum, we need to be able to allocate the error object.
        Recycler::AutoAllowAllocationDuringHeapEnum autoAllowAllocationDuringHeapEnum(this->GetRecycler());
#endif
        
        JavascriptError* stackOverflowError = scriptContext->GetLibrary()->CreateError();
        JavascriptError::SetErrorMessage(stackOverflowError, VBSERR_OutOfStack, NULL, scriptContext);
        return stackOverflowError;
    }

    inline JavascriptError* JavascriptLibrary::CreateOutOfMemoryError()
    {
        JavascriptError* outOfMemoryError = scriptContext->GetLibrary()->CreateError();
        JavascriptError::SetErrorMessage(outOfMemoryError, VBSERR_OutOfMemory, NULL, scriptContext);
        return outOfMemoryError;
    }

    // Should only be called when WinRT is enabled
    inline JavascriptError* JavascriptLibrary::CreateWinRTError()
    {
        // If WinRT isn't enabled, create an error of type kjstError instead.
        if (!scriptContext->GetConfig()->IsWinRTEnabled())
        {
            return scriptContext->GetLibrary()->CreateError();
        }
        AssertMsg(winrtErrorType, "Where's winrtErrorType?");
        JavascriptError *pError = RecyclerNew(this->GetRecycler(), JavascriptError, winrtErrorType);
        JavascriptError::SetErrorType(pError, kjstWinRTError);
        return pError;
    }

    inline JavascriptFunction* JavascriptLibrary::CreateNonProfiledFunction(FunctionInfo * functionInfo)
    {
        Assert(functionInfo->GetAttributes() & FunctionInfo::DoNotProfile);
        return EnsureReadyIfHybridDebugging(RecyclerNew(this->GetRecycler(), RuntimeFunction,
            CreateDeferredPrototypeFunctionType(functionInfo->GetOriginalEntryPoint()),
            functionInfo));
    }

    inline ScriptFunction* JavascriptLibrary::CreateScriptFunction(FunctionProxy * proxy)
    {
        ScriptFunctionType* deferredPrototypeType = proxy->EnsureDeferredPrototypeType();
        return EnsureReadyIfHybridDebugging(RecyclerNewEnumClass(this->GetRecycler(), EnumFunctionClass, ScriptFunction, proxy, deferredPrototypeType));
    }

    inline ScriptFunctionWithInlineCache* JavascriptLibrary::CreateScriptFunctionWithInlineCache(FunctionProxy * proxy)
    {
        ScriptFunctionType* deferredPrototypeType = proxy->EnsureDeferredPrototypeType();
        return EnsureReadyIfHybridDebugging(RecyclerNewEnumClass(this->GetRecycler(), EnumFunctionClass, ScriptFunctionWithInlineCache, proxy, deferredPrototypeType));
    }

    inline GeneratorVirtualScriptFunction* JavascriptLibrary::CreateGeneratorVirtualScriptFunction(FunctionProxy * proxy)
    {
        ScriptFunctionType* deferredPrototypeType = proxy->EnsureDeferredPrototypeType();
        return EnsureReadyIfHybridDebugging(RecyclerNewEnumClass(this->GetRecycler(), EnumFunctionClass, GeneratorVirtualScriptFunction, proxy, deferredPrototypeType));
    }

    inline JavascriptTypedObjectSlotAccessorFunction* JavascriptLibrary::CreateTypedObjectSlotGetterFunction(unsigned int slotIndex, FunctionInfo* functionInfo, int typeId, PropertyId nameId)
    {
        // GC should zero out the whole library; we shouldn't need to explicitly zero out
        if (typedObjectSlotGetterFunctionTypes[slotIndex] == nullptr)
        {
            typedObjectSlotGetterFunctionTypes[slotIndex] = CreateFunctionWithLengthType(functionInfo);
            scriptContext->EnsureDOMFastPathIRHelperMap()->Add(functionInfo, ::DOMFastPathInfo::GetGetterIRHelper(slotIndex));
        }
        return EnsureReadyIfHybridDebugging(RecyclerNewEnumClass(this->GetRecycler(), EnumFunctionClass, JavascriptTypedObjectSlotAccessorFunction, typedObjectSlotGetterFunctionTypes[slotIndex], functionInfo, typeId, nameId));
    }

    inline JavascriptTypedObjectSlotAccessorFunction* JavascriptLibrary::CreateTypedObjectSlotSetterFunction(unsigned int slotIndex, FunctionInfo* functionInfo, int typeId, PropertyId nameId)
    {
        // GC should zero out the whole library; we shouldn't need to explicitly zero out
        if (typedObjectSlotSetterFunctionTypes[slotIndex] == nullptr)
        {
            typedObjectSlotSetterFunctionTypes[slotIndex] = CreateFunctionWithLengthType(functionInfo);

        }
        return EnsureReadyIfHybridDebugging(RecyclerNewEnumClass(this->GetRecycler(), EnumFunctionClass, JavascriptTypedObjectSlotAccessorFunction, typedObjectSlotSetterFunctionTypes[slotIndex], functionInfo, typeId, nameId));
    }

    inline DynamicType * JavascriptLibrary::CreateGeneratorType(RecyclableObject* prototype)
    {
        return DynamicType::New(scriptContext, TypeIds_Generator, prototype, nullptr, NullTypeHandler<false>::GetDefaultInstance());
    }

    template <class MethodType>
    inline JavascriptExternalFunction* JavascriptLibrary::CreateIdMappedExternalFunction(MethodType entryPoint, DynamicType *pPrototypeType)
    {
        return EnsureReadyIfHybridDebugging(RecyclerNewEnumClass(this->GetRecycler(), EnumFunctionClass, JavascriptExternalFunction, entryPoint, pPrototypeType));
    }

    inline JavascriptWinRTFunction* JavascriptLibrary::CreateIdMappedWinRTFunction(DynamicType * type, WinRTFunctionInfo * functionInfo, Var signature)
    {
        return EnsureReadyIfHybridDebugging(RecyclerNewEnumClass(this->GetRecycler(), EnumFunctionClass, JavascriptWinRTFunction, type, functionInfo, signature));
    }

    inline JavascriptWinRTFunction* JavascriptLibrary::CreateIdMappedWinRTConstructorFunction(DynamicType * type, WinRTFunctionInfo * functionInfo, Var signature)
    {
        return EnsureReadyIfHybridDebugging(RecyclerNewEnumClass(this->GetRecycler(), EnumFunctionClass, JavascriptWinRTConstructorFunction, type, functionInfo, signature));
    }

    inline JavascriptGeneratorFunction* JavascriptLibrary::CreateGeneratorFunction(JavascriptMethod entryPoint, GeneratorVirtualScriptFunction* scriptFunction)
    {
        Assert(scriptContext->GetConfig()->IsES6GeneratorsEnabled());

        DynamicType* type = CreateDeferredPrototypeGeneratorFunctionType(entryPoint);

        return EnsureReadyIfHybridDebugging(RecyclerNewEnumClass(this->GetRecycler(), EnumFunctionClass, JavascriptGeneratorFunction, type, scriptFunction));
    }

    inline JavascriptExternalFunction* JavascriptLibrary::CreateStdCallExternalFunction(StdCallJavascriptMethod entryPoint, PropertyId nameId, void *callbackState)
    {       
        Assert(nameId == 0 || scriptContext->IsTrackedPropertyId(nameId));
        return CreateStdCallExternalFunction(entryPoint, TaggedInt::ToVarUnchecked(nameId), callbackState);
    }

    inline JavascriptExternalFunction* JavascriptLibrary::CreateStdCallExternalFunction(StdCallJavascriptMethod entryPoint, Var nameId, void *callbackState)
    {
        JavascriptExternalFunction* function = EnsureReadyIfHybridDebugging(this->CreateIdMappedExternalFunction(entryPoint, stdCallFunctionWithDeferredPrototypeType));
        function->SetFunctionNameId(nameId);
        function->SetCallbackState(callbackState);

        return function;
    }

    inline JavascriptWinRTFunction* JavascriptLibrary::CreateWinRTFunction(JavascriptMethod entryPoint, PropertyId nameId, Var signature, bool fConstructor)
    {
        auto functionInfo = RecyclerNew(this->GetRecycler(), WinRTFunctionInfo, entryPoint);
        JavascriptWinRTFunction *function = nullptr;
        DynamicType * type = CreateDeferredPrototypeFunctionType(this->inDispatchProfileMode ? ProfileEntryThunk : entryPoint);
        if (fConstructor)
        {
            function = this->CreateIdMappedWinRTConstructorFunction(type, functionInfo, signature);
        }
        else
        {
            function = this->CreateIdMappedWinRTFunction(type, functionInfo, signature);
        }
        function = EnsureReadyIfHybridDebugging(function);

        function->SetFunctionNameId(TaggedInt::ToVarUnchecked(nameId));
        return function;
    }

    inline JavascriptPromiseCapabilitiesExecutorFunction* JavascriptLibrary::CreatePromiseCapabilitiesExecutorFunction(JavascriptMethod entryPoint, JavascriptPromiseCapability* capability)
    {
        Assert(scriptContext->GetConfig()->IsES6PromiseEnabled());

        FunctionInfo* functionInfo = RecyclerNew(this->GetRecycler(), FunctionInfo, entryPoint);
        DynamicType* type = CreateDeferredPrototypeFunctionType(this->inDispatchProfileMode ? ProfileEntryThunk : entryPoint);

        return EnsureReadyIfHybridDebugging(RecyclerNewEnumClass(this->GetRecycler(), EnumFunctionClass, JavascriptPromiseCapabilitiesExecutorFunction, type, functionInfo, capability));
    }

    inline JavascriptPromiseResolveOrRejectFunction* JavascriptLibrary::CreatePromiseResolveOrRejectFunction(JavascriptMethod entryPoint, JavascriptPromise* promise, bool isReject)
    {
        Assert(scriptContext->GetConfig()->IsES6PromiseEnabled());

        FunctionInfo* functionInfo = &Js::JavascriptPromise::EntryInfo::ResolveOrRejectFunction;
        DynamicType* type = CreateDeferredPrototypeFunctionType(this->inDispatchProfileMode ? ProfileEntryThunk : entryPoint);

        return EnsureReadyIfHybridDebugging(RecyclerNewEnumClass(this->GetRecycler(), EnumFunctionClass, JavascriptPromiseResolveOrRejectFunction, type, functionInfo, promise, isReject));
    }

    inline JavascriptPromiseReactionTaskFunction* JavascriptLibrary::CreatePromiseReactionTaskFunction(JavascriptMethod entryPoint, JavascriptPromiseReaction* reaction, Var argument)
    {
        Assert(scriptContext->GetConfig()->IsES6PromiseEnabled());

        FunctionInfo* functionInfo = RecyclerNew(this->GetRecycler(), FunctionInfo, entryPoint);
        DynamicType* type = CreateDeferredPrototypeFunctionType(this->inDispatchProfileMode ? ProfileEntryThunk : entryPoint);

        return EnsureReadyIfHybridDebugging(RecyclerNewEnumClass(this->GetRecycler(), EnumFunctionClass, JavascriptPromiseReactionTaskFunction, type, functionInfo, reaction, argument));
    }

    inline JavascriptPromiseResolveThenableTaskFunction* JavascriptLibrary::CreatePromiseResolveThenableTaskFunction(JavascriptMethod entryPoint, JavascriptPromise* promise, RecyclableObject* thenable, RecyclableObject* thenFunction)
    {
        Assert(scriptContext->GetConfig()->IsES6PromiseEnabled());

        FunctionInfo* functionInfo = RecyclerNew(this->GetRecycler(), FunctionInfo, entryPoint);
        DynamicType* type = CreateDeferredPrototypeFunctionType(this->inDispatchProfileMode ? ProfileEntryThunk : entryPoint);

        return EnsureReadyIfHybridDebugging(RecyclerNewEnumClass(this->GetRecycler(), EnumFunctionClass, JavascriptPromiseResolveThenableTaskFunction, type, functionInfo, promise, thenable, thenFunction));
    }

    inline JavascriptPromiseAllResolveElementFunction* JavascriptLibrary::CreatePromiseAllResolveElementFunction(JavascriptMethod entryPoint, uint32 index, JavascriptArray* values, JavascriptPromiseCapability* capabilities, JavascriptPromiseAllResolveElementFunctionRemainingElementsWrapper* remainingElements)
    {
        Assert(scriptContext->GetConfig()->IsES6PromiseEnabled());

        FunctionInfo* functionInfo = &Js::JavascriptPromise::EntryInfo::AllResolveElementFunction;
        DynamicType* type = CreateDeferredPrototypeFunctionType(this->inDispatchProfileMode ? ProfileEntryThunk : entryPoint);

        return EnsureReadyIfHybridDebugging(RecyclerNewEnumClass(this->GetRecycler(), EnumFunctionClass, JavascriptPromiseAllResolveElementFunction, type, functionInfo, index, values, capabilities, remainingElements));
    }

    inline JavascriptExternalFunction* JavascriptLibrary::CreateWrappedExternalFunction(JavascriptExternalFunction* wrappedFunction)
    {
        // The wrapped function will have profiling, so the wrapper function does not need it.
        JavascriptExternalFunction* function = EnsureReadyIfHybridDebugging(RecyclerNew(this->GetRecycler(), JavascriptExternalFunction, wrappedFunction, wrappedFunctionWithDeferredPrototypeType));
        function->SetFunctionNameId(wrappedFunction->GetSourceString());

        return function;
    }

#if !FLOATVAR
    __inline JavascriptNumber * JavascriptLibrary::CreateNumber(double value, RecyclerJavascriptNumberAllocator * numberAllocator)
    {
        AssertMsg(numberTypeStatic, "Where's numberTypeStatic?");
        return AllocatorNew(RecyclerJavascriptNumberAllocator, numberAllocator, JavascriptNumber, value, numberTypeStatic);
    }

    inline JavascriptNumber* JavascriptLibrary::CreateCodeGenNumber(CodeGenNumberAllocator * alloc, double value)
    {
        AssertMsg(numberTypeStatic, "Where's numberTypeStatic?");
        return new (alloc->Alloc()) JavascriptNumber(value, numberTypeStatic);        
    }
#endif

    inline DynamicObject* JavascriptLibrary::CreateGeneratorConstructorPrototypeObject()
    {
        AssertMsg(generatorConstructorPrototypeObjectType, "Where's generatorConstructorPrototypeObjectType?");
        DynamicObject * prototype = DynamicObject::New(this->GetRecycler(), generatorConstructorPrototypeObjectType);
        // Generator functions' prototype objects are not created with a .constructor property
        return prototype;
    }

    inline DynamicObject* JavascriptLibrary::CreateConstructorPrototypeObject(JavascriptFunction * constructor)
    {
        AssertMsg(constructorPrototypeObjectType, "Where's constructorPrototypeObjectType?");
        DynamicObject * prototype = DynamicObject::New(this->GetRecycler(), constructorPrototypeObjectType);
        AddMember(prototype, PropertyIds::constructor, constructor);
        return prototype;
    }

    inline DynamicObject* JavascriptLibrary::CreateObject(
        const bool allowObjectHeaderInlining,
        const PropertyIndex requestedInlineSlotCapacity)
    {
        Assert(GetObjectType());
        Assert(GetObjectHeaderInlinedType());

        const bool useObjectHeaderInlining =
            allowObjectHeaderInlining && FunctionBody::DoObjectHeaderInliningForObjectLiteral(requestedInlineSlotCapacity);
        DynamicType *const type =
            useObjectHeaderInlining
                ? GetObjectHeaderInlinedLiteralType(requestedInlineSlotCapacity)
                : GetObjectLiteralType(requestedInlineSlotCapacity);
        return DynamicObject::New(GetRecycler(), type);
    }

    inline DynamicObject* JavascriptLibrary::CreateObject(DynamicTypeHandler * typeHandler)
    {
        return DynamicObject::New(this->GetRecycler(),
            Js::DynamicType::New(scriptContext, Js::TypeIds_Object, this->GetObjectPrototype(),
            RecyclableObject::DefaultEntryPoint, typeHandler, false, false));
    }

    inline DynamicType* JavascriptLibrary::CreateObjectType(RecyclableObject* prototype, Js::TypeId typeId, uint16 requestedInlineSlotCapacity)
    {
        const bool useObjectHeaderInlining = FunctionBody::DoObjectHeaderInliningForConstructor(requestedInlineSlotCapacity);
        const uint16 offsetOfInlineSlots =
            useObjectHeaderInlining
                ? DynamicTypeHandler::GetOffsetOfObjectHeaderInlineSlots()
                : sizeof(DynamicObject);

        DynamicType* dynamicType = null;
        const bool useCache = prototype->GetScriptContext() == this->scriptContext;
        if (useCache &&
            prototype->GetInternalProperty(prototype, Js::InternalPropertyIds::TypeOfPrototypObject, (Js::Var*) &dynamicType, NULL, this->scriptContext))
        {
            //If the prototype is externalObject, then ExternalObject::Reinitialize can set all the properties to undefined in navigation scenario.
            //Check to make sure dynamicType which is stored as a Js::Var is not undefined. 
            //See Blue 419324
            if (dynamicType != null && (Js::Var)dynamicType != this->GetUndefined())
            {
                DynamicTypeHandler *const dynamicTypeHandler = dynamicType->GetTypeHandler();
                if(dynamicTypeHandler->IsObjectHeaderInlinedTypeHandler() == useObjectHeaderInlining &&
                    (
                        dynamicTypeHandler->GetInlineSlotCapacity() ==
                        (
                            useObjectHeaderInlining
                                ? DynamicTypeHandler::RoundUpObjectHeaderInlinedInlineSlotCapacity(requestedInlineSlotCapacity)
                                : DynamicTypeHandler::RoundUpInlineSlotCapacity(requestedInlineSlotCapacity)
                        )
                    ))
                {
                    Assert(dynamicType->GetIsShared());
                    return dynamicType;
                }
            }
        }

        SimplePathTypeHandler* typeHandler = SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, requestedInlineSlotCapacity, offsetOfInlineSlots, true, true);
        dynamicType = DynamicType::New(scriptContext, typeId, prototype, RecyclableObject::DefaultEntryPoint, typeHandler, true, true);

        if(useCache)
        {
            prototype->SetInternalProperty(Js::InternalPropertyIds::TypeOfPrototypObject, (Var) dynamicType, PropertyOperationFlags::PropertyOperation_Force, NULL);
        }

        return dynamicType;
    }

    inline DynamicType* JavascriptLibrary::CreateObjectTypeNoCache(RecyclableObject* prototype, Js::TypeId typeId)
    {
        return DynamicType::New(scriptContext, typeId, prototype, RecyclableObject::DefaultEntryPoint, 
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
    }

    inline DynamicType* JavascriptLibrary::CreateObjectType(RecyclableObject* prototype, uint16 requestedInlineSlotCapacity)
    {
        // We can't reuse the type in objectType even if hte prototype is the object protoypte, because those has inline slot capacity fixed
        return CreateObjectType(prototype, TypeIds_Object, requestedInlineSlotCapacity);
    }

    inline DynamicObject* JavascriptLibrary::CreateObject(RecyclableObject* prototype, uint16 requestedInlineSlotCapacity)
    {
        Assert(JavascriptOperators::IsObject(prototype));

        DynamicType* dynamicType = CreateObjectType(prototype, requestedInlineSlotCapacity);
        return DynamicObject::New(this->GetRecycler(), dynamicType);
    }

    inline PropertyStringCacheMap* JavascriptLibrary::EnsurePropertyStringMap()
    {
        if (this->propertyStringMap == null)
        {
            this->propertyStringMap = RecyclerNew(this->recycler, PropertyStringCacheMap, this->GetRecycler());
            this->scriptContext->RegisterWeakReferenceDictionary((JsUtil::IWeakReferenceDictionary*) this->propertyStringMap);
        }
        return this->propertyStringMap;
    }

    inline DynamicObject* JavascriptLibrary::CreateActivationObject()
    {
        AssertMsg(activationObjectType, "Where's activationObjectType?");
        return RecyclerNew(this->GetRecycler(), ActivationObject, activationObjectType);
    }

    inline DynamicObject* JavascriptLibrary::CreatePseudoActivationObject()
    {
        AssertMsg(activationObjectType, "Where's activationObjectType?");
        return RecyclerNew(this->GetRecycler(), PseudoActivationObject, activationObjectType);
    }

    inline DynamicObject* JavascriptLibrary::CreateBlockActivationObject()
    {
        AssertMsg(activationObjectType, "Where's activationObjectType?");
        return RecyclerNew(this->GetRecycler(), BlockActivationObject, activationObjectType);
    }

    inline JavascriptString* JavascriptLibrary::GetEmptyString() const
    {
        AssertMsg(emptyString, "Where's emptyString?");
#ifdef PROFILE_STRINGS
        StringProfiler::RecordEmptyStringRequest(scriptContext);
#endif
        return emptyString;
    }    

    // Create a string literal from a C++ string (const wchar_t array with compile-time determined size)
    // Note: The template arg is the string length in characters, including the NUL terminator.
    template< size_t N > JavascriptString* JavascriptLibrary::CreateStringFromCppLiteral(const wchar_t (&value)[N]) const
    {
        CompileAssert(N>2); // Other values are handled by the specializations below
        return LiteralString::New(GetStringTypeStatic(), value, N - 1 /*don't include terminating NUL*/, this->GetRecycler());        
    }

    // Specialization for the empty string
    template<> JavascriptString* JavascriptLibrary::CreateStringFromCppLiteral(const wchar_t (&value)[1]) const
    {
        return GetEmptyString();
    }

    // Specialization for single-char strings
    template<> JavascriptString* JavascriptLibrary::CreateStringFromCppLiteral(const wchar_t (&value)[2]) const
    {
        return charStringCache.GetStringForChar(value[0]);
    }

    inline PropertyString* JavascriptLibrary::CreatePropertyString(const Js::PropertyRecord* propertyRecord)
    {
        AssertMsg(stringTypeStatic, "Where's stringTypeStatic?");
        return PropertyString::New(stringTypeStatic, propertyRecord, this->GetRecycler());
    }

    inline PropertyString* JavascriptLibrary::CreatePropertyString(const Js::PropertyRecord* propertyRecord, ArenaAllocator *arena)
    {
        AssertMsg(stringTypeStatic, "Where's stringTypeStatic?");
        return PropertyString::New(stringTypeStatic, propertyRecord, arena);
    }

    inline JavascriptVariantDate* JavascriptLibrary::CreateVariantDate(const double value)
    {
        AssertMsg(variantDateType, "Where's variantDateType?");
        return RecyclerNewLeafZ(this->GetRecycler(), JavascriptVariantDate, value, variantDateType);
    }

    inline JavascriptBooleanObject* JavascriptLibrary::CreateBooleanObject()
    {
        AssertMsg(booleanTypeDynamic, "Where's booleanTypeDynamic?");
        return RecyclerNew(this->GetRecycler(), JavascriptBooleanObject, nullptr, booleanTypeDynamic);
    }

    inline JavascriptBooleanObject* JavascriptLibrary::CreateBooleanObject(BOOL value)
    {
        AssertMsg(booleanTypeDynamic, "Where's booleanTypeDynamic?");
        return RecyclerNew(this->GetRecycler(), JavascriptBooleanObject, CreateBoolean(value), booleanTypeDynamic);
    }

    inline JavascriptSymbolObject* JavascriptLibrary::CreateSymbolObject(JavascriptSymbol* value)
    {
        AssertMsg(symbolTypeDynamic, "Where's symbolTypeDynamic?");
        return RecyclerNew(this->GetRecycler(), JavascriptSymbolObject, value, symbolTypeDynamic);
    }

    inline JavascriptNumberObject* JavascriptLibrary::CreateNumberObject(Var number)
    {
        AssertMsg(numberTypeDynamic, "Where's numberTypeDynamic?");
        return RecyclerNew(this->GetRecycler(), JavascriptNumberObject, number, numberTypeDynamic);
    }

    inline JavascriptNumberObject* JavascriptLibrary::CreateNumberObjectWithCheck(double value)
    {
        return CreateNumberObject(JavascriptNumber::ToVarWithCheck(value, scriptContext));
    }

    inline JavascriptStringObject* JavascriptLibrary::CreateStringObject(JavascriptString* value)
    {
        AssertMsg(stringTypeDynamic, "Where's stringTypeDynamic?");
        return RecyclerNew(this->GetRecycler(), JavascriptStringObject, value, stringTypeDynamic);
    }

    inline JavascriptStringObject* JavascriptLibrary::CreateStringObject(const wchar_t* value, charcount_t length)
    {
        AssertMsg(stringTypeDynamic, "Where's stringTypeDynamic?");
        return RecyclerNew(this->GetRecycler(), JavascriptStringObject, 
            Js::JavascriptString::NewWithBuffer(value, length, scriptContext), stringTypeDynamic);
    }

    inline JavascriptRegExp* JavascriptLibrary::CreateRegExp(UnifiedRegex::RegexPattern* pattern)
    {
        AssertMsg(regexType, "Where's regexType?");
        return RecyclerNew(this->GetRecycler(), JavascriptRegExp, pattern, regexType);
    }

    inline JavascriptArrayIterator* JavascriptLibrary::CreateArrayIterator(Var iterable, JavascriptArrayIteratorKind kind)
    {
        AssertMsg(arrayIteratorType, "Where's arrayIteratorType");
        return RecyclerNew(this->GetRecycler(), JavascriptArrayIterator, arrayIteratorType, iterable, kind);
    }

    inline JavascriptMapIterator* JavascriptLibrary::CreateMapIterator(JavascriptMap* map, JavascriptMapIteratorKind kind)
    {
        AssertMsg(mapIteratorType, "Where's mapIteratorType");
        return RecyclerNew(this->GetRecycler(), JavascriptMapIterator, mapIteratorType, map, kind);
    }

    inline JavascriptSetIterator* JavascriptLibrary::CreateSetIterator(JavascriptSet* set, JavascriptSetIteratorKind kind)
    {
        AssertMsg(setIteratorType, "Where's setIteratorType");
        return RecyclerNew(this->GetRecycler(), JavascriptSetIterator, setIteratorType, set, kind);
    }

    inline JavascriptStringIterator* JavascriptLibrary::CreateStringIterator(JavascriptString* string)
    {
        AssertMsg(stringIteratorType, "Where's stringIteratorType");
        return RecyclerNew(this->GetRecycler(), JavascriptStringIterator, stringIteratorType, string);
    }

    inline DynamicObject* JavascriptLibrary::CreateIteratorResultObject(Var value, Var done)
    {
        DynamicObject* iteratorResult = DynamicObject::New(this->GetRecycler(), iteratorResultType);

        iteratorResult->SetSlot(SetSlotArguments(Js::PropertyIds::value, 0, value));
        iteratorResult->SetSlot(SetSlotArguments(Js::PropertyIds::done, 1, done));

        return iteratorResult;
    }

    inline DynamicObject* JavascriptLibrary::CreateIteratorResultObjectValueFalse(Var value)
    {
        return CreateIteratorResultObject(value, GetFalse());
    }

    inline DynamicObject* JavascriptLibrary::CreateIteratorResultObjectUndefinedTrue()
    {
        return CreateIteratorResultObject(GetUndefined(), GetTrue());
    }

    inline RecyclableObject* JavascriptLibrary::CreateThrowErrorObject(JavascriptError* error)
    {
        return ThrowErrorObject::New(this->throwErrorObjectType, error, this->GetRecycler());
    }

    inline void JavascriptLibrary::SetForInEnumeratorCache(ForInObjectEnumerator* enumerator)
    {
        Assert(enumerator);
        this->cachedForInEnumerator = enumerator->GetWeakReference(this->recycler);
        }

    inline ForInObjectEnumerator* JavascriptLibrary::GetAndClearForInEnumeratorCache()
    {
        auto cachedEnumerator = this->cachedForInEnumerator;
        if (cachedEnumerator)
        {
            ForInObjectEnumerator * enumerator = cachedEnumerator->Get();
            this->cachedForInEnumerator = null;
            return enumerator;
        }
            return null;
    }

    template <size_t N>
    inline JavascriptFunction * JavascriptLibrary::AddFunctionToLibraryObjectWithPropertyName(DynamicObject* object, const wchar_t(&propertyName)[N], FunctionInfo * functionInfo, int length)
    {
        // The PID need to be tracked because it is assigned to the runtime function's nameId
        return AddFunctionToLibraryObject(object, scriptContext->GetOrAddPropertyIdTracked(propertyName), functionInfo, length);
    }

    inline bool JavascriptLibrary::IsCopyOnAccessArrayCallSite(JavascriptLibrary *lib, ArrayCallSiteInfo *arrayInfo, uint32 length)
    {
        return
            lib->cacheForCopyOnAccessArraySegments
            && lib->cacheForCopyOnAccessArraySegments->IsNotOverHardLimit()
            && (
                PHASE_FORCE1(CopyOnAccessArrayPhase)  // -force:copyonaccessarray is only restricted by hard limit of the segment cache
                || (
                    !arrayInfo->isNotCopyOnAccessArray        // from profile
                    && !PHASE_OFF1(CopyOnAccessArrayPhase)
                    && lib->cacheForCopyOnAccessArraySegments->IsNotFull()  // cache size soft limit thru -copyonaccessarraysegmentcachesize:<number>
                    && length <= (uint32) CONFIG_FLAG(MaxCopyOnAccessArrayLength)  // -maxcopyonaccessarraylength:<number>
                    && length >= (uint32) CONFIG_FLAG(MinCopyOnAccessArrayLength)  // -mincopyonaccessarraylength:<number>
                )
            );
    }

    inline bool JavascriptLibrary::IsCachedCopyOnAccessArrayCallSite(const JavascriptLibrary *lib, ArrayCallSiteInfo *arrayInfo)
    {
        return lib->cacheForCopyOnAccessArraySegments
            && lib->cacheForCopyOnAccessArraySegments->IsValidIndex(arrayInfo->copyOnAccessArrayCacheIndex);
    }

    template <>
    inline void JavascriptLibrary::CheckAndConvertCopyOnAccessNativeIntArray(const Var instance)
    {
        if (instance && JavascriptCopyOnAccessNativeIntArray::Is(instance))
        {
            JavascriptCopyOnAccessNativeIntArray::FromVar(instance)->ConvertCopyOnAccessSegment();
        }
    }

    template <typename T>
    inline void JavascriptLibrary::CheckAndConvertCopyOnAccessNativeIntArray(const T instance)
    {
        // dummy template function
        // Js::SCAEngine::Clone() to convert JavascriptCopyOnAccessNativeIntArray only if src is a 'Var'
    }

#ifdef ENABLE_NATIVE_CODEGEN
    inline JavascriptFunction ** JavascriptLibrary::GetBuiltinFunctions()
    {
        AssertMsg(this->builtinFunctions, "builtinFunctions table must've been initialized as part of library initialization!");
        return this->builtinFunctions;
    }

    inline INT_PTR* JavascriptLibrary::GetVTableAddresses()
    {
        AssertMsg(this->vtableAddresses, "vtableAddresses table must've been initialized as part of library initialization!");
        return this->vtableAddresses;
    }

    // TODO: get rid of switch and use table-driven approach. See reverse function: InliningDecider::GetBuiltInInlineCandidateOpCode
    //static 
    inline BuiltinFunction JavascriptLibrary::GetBuiltInInlineCandidateId(OpCode opCode)
    {
        switch (opCode)
        {
            case OpCode::InlineMathAcos:
                return BuiltinFunction::Math_Acos;

            case OpCode::InlineMathAsin:
                return BuiltinFunction::Math_Asin;

            case OpCode::InlineMathAtan:
                return BuiltinFunction::Math_Atan;

            case OpCode::InlineMathAtan2:
                return BuiltinFunction::Math_Atan2;

            case OpCode::InlineMathCos:
                return BuiltinFunction::Math_Cos;

            case OpCode::InlineMathExp:
                return BuiltinFunction::Math_Exp;

            case OpCode::InlineMathLog:
                return BuiltinFunction::Math_Log;

            case OpCode::InlineMathPow:
                return BuiltinFunction::Math_Pow;

            case OpCode::InlineMathSin:
                return BuiltinFunction::Math_Sin;

            case OpCode::InlineMathSqrt:
                return BuiltinFunction::Math_Sqrt;

            case OpCode::InlineMathTan:
                return BuiltinFunction::Math_Tan;

            // The ones below will be enabled in IE11.
            // TODO: add string.prototype.concat.
            case OpCode::InlineMathAbs:
                return BuiltinFunction::Math_Abs;

            case OpCode::InlineMathClz32:
                return BuiltinFunction::Math_Clz32;

            case OpCode::InlineMathCeil:
                return BuiltinFunction::Math_Ceil;

            case OpCode::InlineMathFloor:
                return BuiltinFunction::Math_Floor;

            case OpCode::InlineMathMax:
                return BuiltinFunction::Math_Max;

            case OpCode::InlineMathMin:
                return BuiltinFunction::Math_Min;

            case OpCode::InlineMathImul:
                return BuiltinFunction::Math_Imul;

            case OpCode::InlineMathRandom:
                return BuiltinFunction::Math_Random;

            case OpCode::InlineMathRound:
                return BuiltinFunction::Math_Round;

            case OpCode::InlineMathFround:
                return BuiltinFunction::Math_Fround;

            case OpCode::InlineStringCharAt:
                return BuiltinFunction::String_CharAt;

            case OpCode::InlineStringCharCodeAt:
                return BuiltinFunction::String_CharCodeAt;

            case OpCode::InlineStringCodePointAt:
                return BuiltinFunction::String_CodePointAt;

            case OpCode::InlineArrayPop:
                return BuiltinFunction::Array_Pop;

            case OpCode::InlineArrayPush:
                return BuiltinFunction::Array_Push;

            case OpCode::InlineFunctionApply:
                return BuiltinFunction::Function_Apply;

            case OpCode::InlineFunctionCall:
                return BuiltinFunction::Function_Call;

            case OpCode::InlineRegExpExec:
                return BuiltinFunction::RegExp_Exec;

        }

        return BuiltinFunction::None;
    }

    // Parses given flags and arg kind (dst or src1, or src2) returns the type the arg must be type-specialized to.
    // static 
    inline BuiltInArgSpecizationType JavascriptLibrary::GetBuiltInArgType(BuiltInFlags flags, BuiltInArgShift argKind)
    {
        Assert(argKind == BuiltInArgShift::BIAS_Dst || BuiltInArgShift::BIAS_Src1 || BuiltInArgShift::BIAS_Src2);

        BuiltInArgSpecizationType type = static_cast<BuiltInArgSpecizationType>(
            (flags >> argKind) &                            // Shift-out everyting to the right of start of interesting area.
            ((1 << Js::BIAS_ArgSize) - 1));   // Mask-out everything to the left of interesting area.

        return type;
    }
#endif
}
