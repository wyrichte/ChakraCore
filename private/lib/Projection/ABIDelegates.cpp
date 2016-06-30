//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#include "ProjectionPch.h"

// Description: Implementation for the ABI Projector delegates
namespace Projection
{
    Define_UnknownImpl_VTable(g_DelegateVtable,
        CUnknownImpl_VarArgT_VTableEntry(Delegate, Invoke));

    // -----------------------------------------------------------------------------------
    // Delegate class implementation
    // -----------------------------------------------------------------------------------
    // Info:        Construct
    // Parameters:  projectionContext - projection context
    Delegate::Delegate(ProjectionContext *projectionContext)
        : CUnknownImpl(projectionContext, g_DelegateVtable
#if DBG_DUMP
            , delegateWrapper
#endif
        ), callback(NULL), signature(NULL), eventInfo(nullptr), eventProjectionHandler(nullptr),
        prioritizedDelegate(nullptr)
    { 
        // Make sure DLL is not unloaded until this object is destroyed. When the DLL is unloaded
        // we will clean up the ThreadContext which will empty a lot of memory which this object 
        // might try to use causing potential memory corruption or access violations.
        DLLAddRef();
    }

    // Info:        Destruct
    Delegate::~Delegate()
    {
        if (eventInfo)
        {
            JS_ETW(EventWriteJSCRIPT_RECYCLER_FREE_WINRT_EVENTHANDLER_OBJECT(this));
        }
        else
        {
            JS_ETW(EventWriteJSCRIPT_RECYCLER_FREE_WINRT_DELEGATE_OBJECT(this));
        }

        DLLRelease();
    }


    void Delegate::ThrowFatalDisconnectedDelegateError(Js::ScriptContext *scriptContext)
    {
        Assert(eventInfo != nullptr);
        Assert(Js::Configuration::Global.flags.FailFastIfDisconnectedDelegate);
        ULONG_PTR ExceptionInformation[2];
        ExceptionInformation[0] = (ULONG_PTR)m_typeName;
        ExceptionInformation[1] = (ULONG_PTR)StringOfId(scriptContext, eventInfo->nameId); 
        RaiseException((DWORD)SCRIPT_E_DISCONNECTED_DELEGATE, EXCEPTION_NONCONTINUABLE, 2, (ULONG_PTR*)ExceptionInformation);
    }

    // Name: Create
    // Info: Creates an initialized Delegate object
    // Parameters:  
    //              projectionContext - projection Context object
    //              delegateTypeName - type string of the delegate
    //              signature - delegate signature info
    //              callback - JavaScript callback function to be called when the delegate is invoked
    //              evetnInfo - event this delegate eventhandler corresponds to if non null
    //              newDelegateObject - the newly created delegate object
    HRESULT Delegate::Create(
        __in ProjectionContext *projectionContext,
        __in LPCWSTR delegateTypeName,
        __in RtABIMETHODSIGNATURE signature,
        __in Js::JavascriptFunction* callback,
        __in RtEVENT eventInfo,
        __in bool isInAsyncInterface,
        __out Delegate** newDelegateObject)
    {
        Assert(projectionContext != NULL);
        Assert(signature != NULL);
        Assert(callback != NULL);
        Assert(newDelegateObject != NULL);

        Delegate* delegateObject = new Delegate(projectionContext);
        IfNullReturnError(delegateObject, E_OUTOFMEMORY);

        // Initialize the Delegate object
        RecyclerWeakReference<Js::JavascriptFunction> *weakRefFunc = projectionContext->GetScriptContext()->GetRecycler()->CreateWeakReferenceHandle<Js::JavascriptFunction>(callback);
        HRESULT hr = delegateObject->Initialize(delegateTypeName, signature, weakRefFunc, eventInfo);
        if (FAILED(hr))
        {
            delete delegateObject;
            return hr;
        }

        IDelegateWrapper* delegateWrapper = projectionContext->GetDelegateWrapper();

        if ((delegateWrapper != nullptr) && (eventInfo != nullptr))
        {
            // Ask for an event delegate wrapper if the delegate wrapper supports it.
            IEventDelegateWrapper *eventDelegateWrapper = nullptr;
            if (SUCCEEDED(delegateWrapper->QueryInterface(IID_PPV_ARGS(&eventDelegateWrapper))))
            {
                IUnknown* eventDelegate = nullptr;
                IUnknown* originalDelegate = delegateObject->__super::GetUnknown();
#if DBG_DUMP
                if (Js::Configuration::Global.flags.TraceProjection <= TraceLevel_Warning)
                {
                    Output::Print(_u("event delegate \n"));
                    Output::Flush();
                }
#endif
                hr = eventDelegateWrapper->RegisterEventItem(originalDelegate, delegateObject->signature->iid->instantiated, &eventDelegate);
                if (SUCCEEDED(hr))
                {
                    Assert(eventDelegate != nullptr);

                    // This will be the object gives out to external. 
                    // The last release on the wrapped delegate will release everything. 
                    if (eventDelegate != originalDelegate)
                    {
                        delegateObject->prioritizedDelegate = eventDelegate;
                    }

                    // RegisterEventItem either returned the original delegate or must now hold a reference to
                    // the original delegate.  If it is returned it comes back with a reference so release it.  
                    // Otherwise GetUnknown() now returns the prioritized delegate so the original reference for 
                    // Create will not be released, so release it here.  Releasing prioritized delegate will release
                    // the original delegate.
                    originalDelegate->Release();
                }

                eventDelegateWrapper->Release();
            }
        }
        else if ((delegateWrapper != nullptr) && isInAsyncInterface)
        {
            // We'll ask for a priority wrapper for both [in] delegate only
            IUnknown* prioritizedDelegate = nullptr;
            IUnknown* originalDelegate = delegateObject->__super::GetUnknown();

#if DBG
            {
                const bool verifyShouldWrap = isInAsyncInterface && 
                    (eventInfo == nullptr) &&
                    (wcscmp(signature->parameters->callPattern, _u("+Interface+Int32")) == 0) &&
                    (wcscmp(StringOfId(projectionContext->GetScriptContext(), signature->nameId), _u("invoke")) == 0);

                // Technically isInAsyncInterface should be enough to determine that we should wrap the delegate. However, for
                // additional assurance we have verifyShouldWrap which performs a few other checks.
                NT_ASSERTMSG("If by isInAsyncInterface we should wrap the delegate then the method signature should be the one we handle.",
                    isInAsyncInterface == verifyShouldWrap);
            }
#endif

#if DBG_DUMP
            if (Js::Configuration::Global.flags.TraceProjection <= TraceLevel_Warning)
            {
                Output::Print(_u("priority delegate \n"));
                Output::Flush();
            }
#endif

            hr = delegateWrapper->RegisterPriorityItem(originalDelegate, delegateObject->signature->iid->instantiated, &prioritizedDelegate);
            if (SUCCEEDED(hr))
            {
                Assert(prioritizedDelegate != nullptr);

                // This will be the object gives out to external. 
                // The last release on the wrapped delegate will release everything. 
                if (prioritizedDelegate != originalDelegate)
                {
                    delegateObject->prioritizedDelegate = prioritizedDelegate;
                }

                // RegisterPriorityItem always AddRef's the original delegate, even if they do not 
                // return a new prioritized delegate. Either way we need to Release that refcount 
                // to avoid leaking the object.
                originalDelegate->Release();
            }
        }

        if (SUCCEEDED(hr))
        {
            *newDelegateObject = delegateObject;
        }
        else
        {
            delegateObject->GetUnknown()->Release();
            delegateObject = nullptr;
        }

        return hr;
    }

    IUnknown* Delegate::GetUnknown()
    {
        if (prioritizedDelegate != nullptr)
        {
            return prioritizedDelegate;
        }
        return __super::GetUnknown();
    }

    // Name: Initialize
    // Info: Initializes a Delegate object
    // Parameters:  delegateTypeName - type name string for delegate
    //              signature - delegate signature
    //              callback - JavaScript callback function to be called when the delegate is invoked
    //              eventInfo - event information if representing event handler
    HRESULT Delegate::Initialize(
        __in LPCWSTR delegateTypeName,
        __in RtABIMETHODSIGNATURE signature,
        __in RecyclerWeakReference<Js::JavascriptFunction> *callback,
        __in RtEVENT eventInfo)
    {
        Assert(signature);
        Assert(callback);

        HRESULT hr = __super::Initialize(signature->iid->instantiated, delegateTypeName);
        IfFailedReturn(hr);

        AddRef();

        this->callback = callback;
        this->signature = signature;
        this->eventInfo = eventInfo;
        this->paramsCount = signature->GetParameters()->allParameters->Count();
        this->sizeOfCallStack = signature->GetParameters()->sizeOfCallstack;


        if (eventInfo)
        {
            JS_ETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_WINRT_EVENTHANDLER_OBJECT(this, this->GetFullTypeName(), callback, StringOfId(projectionContext->GetScriptContext(), eventInfo->nameId)));
        }
        else
        {
            JS_ETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_WINRT_DELEGATE_OBJECT(this, this->GetFullTypeName(), callback));
        }

        return hr;
    }

    Var Delegate::GetEvObjectFromJsParams(__in JsVarList &jsCallbackParams, __in Var evVar)
    {
        Js::ScriptContext * scriptContext = projectionContext->GetScriptContext();
        LPCWSTR eventName = StringOfId(scriptContext, eventInfo->nameId);

#ifdef ENABLE_JS_ETW
        if(EventEnabledJSCRIPT_PROJECTION_INVOKEEVENTEVPARAMPREP_START()) {
            LPCWSTR runtimeClassName = StringOfId(scriptContext, signature->runtimeClassNameId);
            EventWriteJSCRIPT_PROJECTION_INVOKEEVENTEVPARAMPREP_START(runtimeClassName, eventName);
        }
#endif

        Assert(eventInfo != nullptr);
        
        int jsParamsCount = jsCallbackParams.Count();
        Assert(jsParamsCount >= 1);

        Var *jsParams = jsCallbackParams.Values();

        Js::RecyclableObject *evObject = NULL;
        if (evVar == NULL || !Js::JavascriptConversion::ToObject(evVar, scriptContext, &evObject) || evObject->GetTypeId() == Js::TypeIds_Null)
        {
            // it isnt object create new empty object
            evObject = scriptContext->GetLibrary()->CreateObject();
        }
        
        // Create detail array and target
        Var targetVar = NULL;
        Js::JavascriptArray *detailArray;
        BOOL succeeded;
        if (jsParamsCount >= 2)
        {
            // This, sender, [arg1, arg2, ... ]
            detailArray = scriptContext->GetLibrary()->CreateArray(jsParamsCount - 2);
            for (int i = 2; i < jsParamsCount; i++)
            {
                detailArray->DirectSetItemAt(i - 2, jsParams[i]);
            }
            succeeded = detailArray->PreventExtensions();
            Js::VerifyCatastrophic(succeeded);

            targetVar = jsParams[1];
        }
        else
        {
            // This
            detailArray = scriptContext->GetLibrary()->CreateArray(0);
            succeeded = detailArray->PreventExtensions();
            Js::VerifyCatastrophic(succeeded);

            targetVar = scriptContext->GetLibrary()->GetNull();
        }

        // Set properties - target and detail
        Assert(evObject != NULL);
        Js::PropertyDescriptor descriptor;
        descriptor.SetWritable(false);
        descriptor.SetEnumerable(true);
        descriptor.SetConfigurable(false);

        ProjectionWriter *writer = projectionContext->GetProjectionWriter();

        evObject->SetPropertyWithAttributes(writer->targetId, targetVar, PropertyEnumerable, nullptr);
        evObject->SetPropertyWithAttributes(writer->detailId, detailArray, PropertyEnumerable, nullptr);
        evObject->SetPropertyWithAttributes(writer->typeId, Js::JavascriptString::NewCopySz(eventName, scriptContext), PropertyEnumerable, nullptr);

#ifdef ENABLE_JS_ETW
        if(EventEnabledJSCRIPT_PROJECTION_INVOKEEVENTEVPARAMPREP_STOP()) {
            LPCWSTR runtimeClassName = StringOfId(scriptContext, signature->runtimeClassNameId);
            EventWriteJSCRIPT_PROJECTION_INVOKEEVENTEVPARAMPREP_STOP(runtimeClassName, eventName);
        }
#endif

        succeeded = evObject->PreventExtensions();
        Js::VerifyCatastrophic(succeeded);

        return evObject;
    }

#pragma warning(push)
#pragma warning(disable: 4731)
#pragma warning(disable: 26000)
    // Name:        Invoke
    // Info:        Delegate Invoke implementation. Calls a JavaScript callback function.  
    // Return:      HRESULT
    CUnknownMethodImpl_ArgT_Prolog(Delegate, Invoke, this->paramsCount, this->sizeOfCallStack, 
        Var outVar = nullptr;
        , JSPUBLICERR_CantExecute
    )
    {
#ifdef ENABLE_JS_ETW
        if (eventInfo != nullptr)
        {
            if(EventEnabledJSCRIPT_PROJECTION_INVOKEEVENT_START()) {
                LPCWSTR runtimeClassName = StringOfId(scriptContext, signature->runtimeClassNameId);
                EventWriteJSCRIPT_PROJECTION_INVOKEEVENT_START(runtimeClassName, StringOfId(scriptContext, eventInfo->nameId));
            }
        }
        else
        {
            if(EventEnabledJSCRIPT_PROJECTION_INVOKEJSDELEGATE_START()) {
                LPCWSTR runtimeClassName = StringOfId(scriptContext, signature->runtimeClassNameId);
                EventWriteJSCRIPT_PROJECTION_INVOKEJSDELEGATE_START(runtimeClassName, StringOfId(scriptContext, signature->nameId));
            }
        }
#endif

        // Allocate array for JavaScript callback function arguments + this param
        AssertMsg(signature->inParameterCount < 65536, "Invalid metadata: ECMA-335 II.22.33: Param indices are 0 ... 65535");
        JsVarList jsCallbackParams((int)signature->inParameterCount + 1);

        // Add global object as "this" parameter
        jsCallbackParams.Add(scriptContext->GetLibrary()->GetUndefined());

        // Turn the in parameters into Vars
        ProjectionMarshaler marshal(CalleeRetainsOwnership, projectionContext, false);
        VerifyDeprecatedAttributeOnce(this->signature, scriptContext, DeprecatedInvocation_Delegate);
        Var evVar = NULL;

#if DBG
        ProjectionModel::AllowHeavyOperation allow;
#endif
        // All the out resources need to be released and out memory needs to be initialized to 0, 
        // if marshaling fails before we transfer ownership to the caller
        marshal.SetReleaseDelegateOutResources();

        // ARM only: allocate parameter locations array and initialize it.
        DefineAndInitParameterLocations(signature->GetParameters()->allParameters->Count());
        
        bool noErrorOnInitialize = true;

        // Initialize out parameters
        signature->GetParameters()->allParameters->Cast<RtABIPARAMETER>()->IterateN([&](int parameterLocationIndex, RtABIPARAMETER parameter) {
            DefineParameterLocationAsGetArrayItem(parameterLocations, parameterLocationIndex);
            size_t stackParameterCount = parameter->GetParameterOnStackCount();
            size_t sizeOnStack = parameter->GetSizeOnStack();
            if (parameter->isOut)
            {
                bool isArray = stackParameterCount==2;
                bool noError = true;
                RtCONCRETETYPE type = ConcreteType::From(parameter->type);
                if (isArray)
                {
                    bool isByRef = ByRefType::Is(type);
                    byte * lengthPointer;
                    byte * arrayPointer;
                    bool hasInLength = false;
                    uint32 lengthForArray = 0;
                    RtARRAYTYPE arrayType = isByRef ? ArrayType::From(ByRefType::From(type)->pointedTo) : ArrayType::From(type);
                    if (isByRef) 
                    { 
                        lengthPointer = GetOutParameterAddressFromParamIndexAndOffset(paramIndex, stackBytesRead, false, false); 
                        arrayPointer = GetOutParameterAddressFromParamIndexAndOffset(paramIndex + 1, stackBytesRead + sizeof(LPVOID), false, false); 
                    } 
                    else 
                    {
                        lengthPointer = GetInParameterAddressFromParamIndexAndOffset(paramIndex, stackBytesRead, false, false); 
                        arrayPointer = GetInParameterAddressFromParamIndexAndOffset(paramIndex + 1, stackBytesRead + sizeof(LPVOID), false, false); 
                        if (parameter->IsArrayParameterWithLengthAttribute())
                        {
                            // FillArray with length attribute. We need to only care about in as out values are not yet in effect
                            auto lengthParam = ((AbiArrayParameterWithLengthAttribute *)parameter)->GetLengthParameter(signature->GetParameters()->allParameters);
                            if (lengthParam->isIn)
                            {
                                hasInLength = true;
                                byte *lengthParamLocation;
                                GetArrayLengthParamLocation();
                                Var lengthVar = marshal.ReadOutParameter(nullptr, lengthParam, lengthParamLocation, lengthParam->GetSizeOnStack(), signature->nameId);
                                lengthForArray = (unsigned __int32)Js::JavascriptConversion::ToInt32(lengthVar, scriptContext);
                            }
                        }
                    } 
                    noError = marshal.InitializeDelegateOutArrayType(arrayType, lengthPointer, sizeof(uint), arrayPointer, sizeof(LPVOID), !isByRef, hasInLength, lengthForArray);
                }
                else {
                    byte * paramPointer = GetNextInParameterAddressFromParamType(type);
                    noError = marshal.InitializeDelegateOutType(parameter->type, paramPointer, sizeOnStack);
                }
                noErrorOnInitialize = noErrorOnInitialize && noError;
            }
            UpdateParameterReadByModelParameter(parameter);
        });

        if (!noErrorOnInitialize)
        {
            Js::JavascriptError::MapAndThrowError(scriptContext, E_POINTER);
        }

        paramIndex = 0;
        stackBytesRead = 0;
        signature->GetParameters()->allParameters->Cast<RtABIPARAMETER>()->IterateN([&](int parameterLocationIndex, RtABIPARAMETER parameter) {
            DefineParameterLocationAsGetArrayItem(parameterLocations, parameterLocationIndex);
            size_t stackParameterCount = parameter->GetParameterOnStackCount();
            size_t sizeOnStack = parameter->GetSizeOnStack();
            if (parameter->isIn)
            {
                bool isArray = stackParameterCount==2;
                RtCONCRETETYPE type = ConcreteType::From(parameter->type);
                if (isArray)
                {
                    Assert(!ByRefType::Is(type));
                    RtARRAYTYPE arrayType = ArrayType::From(type);
                    byte * lengthPointer = GetNextInParameterAddress(false, false); 
                    byte * arrayPointer = GetNextInParamAddressFromIndexAndOffset(1, sizeof(LPVOID), false, false); 
                    bool hasInLength = false;
                    uint32 lengthForArray = 0;
                    if (parameter->IsArrayParameterWithLengthAttribute())
                    {
                        // PassArray or FillArray with length attribute. We need to only care about in as out values are not yet in effect
                        auto lengthParam = ((AbiArrayParameterWithLengthAttribute *)parameter)->GetLengthParameter(signature->GetParameters()->allParameters);
                        if (lengthParam->isIn)
                        {
                            hasInLength = true;
                            byte *lengthParamLocation;
                            GetArrayLengthParamLocation();
                            Var lengthVar = marshal.ReadOutParameter(nullptr, lengthParam, lengthParamLocation, lengthParam->GetSizeOnStack(), signature->nameId);
                            lengthForArray = (unsigned __int32)Js::JavascriptConversion::ToInt32(lengthVar, scriptContext);
                        }
                    }
                    Var var = marshal.ReadOutArrayType(nullptr, arrayType, lengthPointer, sizeof(uint),  arrayPointer, sizeof(LPVOID), signature->nameId, false, parameter->isOut, false, hasInLength, lengthForArray);
                    if (jsCallbackParams.Count() == 2 && eventInfo != nullptr)
                    {
                        evVar = marshal.ReadOutArrayType(nullptr, arrayType, lengthPointer, sizeof(uint),  arrayPointer, sizeof(LPVOID), signature->nameId, false, parameter->isOut, true, hasInLength, lengthForArray);
                    }
                    jsCallbackParams.Add(var);
                } 
                else
                {
                    byte * paramPointer = GetNextInParameterAddressFromParamType(type);
                    Var var = marshal.ReadOutParameter(nullptr, parameter, paramPointer, sizeOnStack, signature->nameId);
                    if (jsCallbackParams.Count() == 2 && eventInfo != nullptr)
                    {
                        evVar = marshal.ReadOutParameter(nullptr, parameter, paramPointer, sizeOnStack, signature->nameId, false, true);
                    }
                    jsCallbackParams.Add(var);
                }
            }
            UpdateParameterReadByModelParameter(parameter);
        });

        // Do special things for if eventHandler
        if (eventInfo != nullptr)
        {
            Var evObject = GetEvObjectFromJsParams(jsCallbackParams, evVar);

            // Now call the JavaScript delegate callback function
            Var eventHandlerInvokeParams[2];
            eventHandlerInvokeParams[0] = jsCallbackParams.Values()[0];
            eventHandlerInvokeParams[1] = evObject;
            Js::CallInfo callInfo(Js::CallFlags_Value, 2);

            if (callback == nullptr || callback->Get() == nullptr)
            {
                Assert(SupportsWeakDelegate());
#if DBG_DUMP
                if (Js::Configuration::Global.flags.TraceProjection <= TraceLevel_Warning)
                {
                    Output::Print(_u("disconnected delegate \n"));
                    Output::Flush();
                }
#endif
                projectionContext->IncrementSQMCount(DISCONNECTED_DELEGATES, 1);
#ifdef WINRTFINDPREMATURECOLLECTION
                Assert(false);
#endif
                if (Js::Configuration::Global.flags.FailFastIfDisconnectedDelegate)
                {
                    ThrowFatalDisconnectedDelegateError(scriptContext);
                }
                else
                {
                    hr = RPC_E_DISCONNECTED;
                }
            }
            else
            {
                outVar = callback->Get()->CallFunction(Js::Arguments(callInfo, eventHandlerInvokeParams));
            }
        }
        else
        {
            // Now call the JavaScript delegate callback function
            Js::CallInfo callInfo(Js::CallFlags_Value, (ushort)jsCallbackParams.Count());

            // Only event handlers can get disconnected
            Assert(callback != nullptr && callback->Get() != nullptr);
            outVar = callback->Get()->CallFunction(Js::Arguments(callInfo, jsCallbackParams.Values()));
        }

        if (SUCCEEDED(hr))
        {
            bool outIsNullUndefined = false;
            if (Js::RecyclableObject::Is(outVar))
            {
                Js::TypeId outTypeId = Js::RecyclableObject::FromVar(outVar)->GetTypeId();
                outIsNullUndefined = (outTypeId == Js::TypeIds_Null || outTypeId == Js::TypeIds_Undefined);
            }

            auto isOutOnly = [](RtPARAMETER param) { return param->isOut && !param->isIn; };

            size_t outOnlyArgumentCount = signature->parameters->allParameters->CountWhere(isOutOnly);

            auto writeOut = [&](bool isArray, bool isByRef, int paramIndex, size_t stackOffset, int parameterLocationIndex, RtABIPARAMETER parameter, Var var) {
                DefineParameterLocationAsGetArrayItem(parameterLocations, parameterLocationIndex);
                RtTYPE type = parameter->type;
                if (isArray) 
                { 
                    byte * lengthPointer;
                    byte * arrayPointer;
                    RtARRAYTYPE arrayType = isByRef ? ArrayType::From(ByRefType::From(type)->pointedTo) : ArrayType::From(type);
                    if (isByRef) 
                    { 
                        lengthPointer = GetOutParameterAddressFromParamIndexAndOffset(paramIndex, stackOffset, false, false); 
                        arrayPointer = GetOutParameterAddressFromParamIndexAndOffset(paramIndex + 1, stackOffset + sizeof(LPVOID), false, false); 
                    } 
                    else 
                    {
                        lengthPointer = GetInParameterAddressFromParamIndexAndOffset(paramIndex, stackOffset, false, false); 
                        arrayPointer = GetInParameterAddressFromParamIndexAndOffset(paramIndex + 1, stackOffset + sizeof(LPVOID), false, false); 
                    } 

                    bool hasLength = false;
                    uint32 lengthForArray = 0;
                    if (parameter->IsArrayParameterWithLengthAttribute())
                    {
                        hasLength = true;

                        // FillArray or ReceiveArray with length attribute
                        auto lengthParam = ((AbiArrayParameterWithLengthAttribute *)parameter)->GetLengthParameter(signature->GetParameters()->allParameters);
                        Var lengthVar;
                        if (lengthParam->isIn)
                        {
                            // one of the in parameter, we already marshaled.
                            lengthVar = jsCallbackParams.Values()[lengthParam->inParameterIndex + 1];
                        }
                        else
                        {
                            // Out param - decode from the outVar
                            if (outOnlyArgumentCount==1 || outIsNullUndefined)
                            {
                                lengthVar = outVar;
                            }
                            else if  (outOnlyArgumentCount>1)
                            {
                                Js::PropertyRecord const * propertyRecord;
                                LPCWSTR parameterName = StringOfId(scriptContext, lengthParam->id);
                                scriptContext->GetOrAddPropertyRecord(parameterName, Js::JavascriptString::GetBufferLength(parameterName), &propertyRecord);
                                PropertyId propertyId = propertyRecord->GetPropertyId();
                                lengthVar = Js::JavascriptOperators::OP_GetProperty(outVar, propertyId, scriptContext);
                            }
                            else
                            {
                                Js::Throw::FatalInternalError();
                            }
                        }
                        lengthForArray = (unsigned __int32)Js::JavascriptConversion::ToInt32(lengthVar, scriptContext);
                    }
                    marshal.WriteInArrayTypeIndividual(var, arrayType, isByRef, lengthPointer, arrayPointer, true, hasLength, lengthForArray);
                } 
                else 
                { 
                    RtCONCRETETYPE paramType = isByRef ? ConcreteType::From(ByRefType::From(type)->pointedTo) : ConcreteType::From(type);
                    byte * parameterPointer = GetOutParameterAddressFromParamType(paramIndex, stackOffset);
                    marshal.WriteInType(var, paramType, parameterPointer, paramType->sizeOnStack, true);
                } 
            };

            paramIndex = 0;
            stackBytesRead = 0;
            signature->GetParameters()->allParameters->Cast<RtABIPARAMETER>()->IterateN([&](int parameterLocationIndex, RtABIPARAMETER parameter) {
                size_t stackParameterCount = parameter->GetParameterOnStackCount();
                if (parameter->isOut)
                {
                    bool isArray = stackParameterCount==2;
                    bool isByRef = ByRefType::Is(parameter->type);
                    if (parameter->isIn)
                    {
                        Var paramVar = jsCallbackParams.Values()[parameter->inParameterIndex + 1];
                        writeOut(isArray, isByRef, paramIndex, stackBytesRead, parameterLocationIndex, parameter, paramVar);
                    }
                    else
                    {
                        if (outOnlyArgumentCount==1 || outIsNullUndefined)
                        {
                            writeOut(isArray, isByRef, paramIndex, stackBytesRead, parameterLocationIndex, parameter, outVar);
                        }
                        else if  (outOnlyArgumentCount>1)
                        {
                            Js::PropertyRecord const * propertyRecord;
                            LPCWSTR parameterName = StringOfId(scriptContext, parameter->id);
                            scriptContext->GetOrAddPropertyRecord(parameterName, Js::JavascriptString::GetBufferLength(parameterName), &propertyRecord);
                            PropertyId propertyId = propertyRecord->GetPropertyId();
                            Var paramVar = Js::JavascriptOperators::OP_GetProperty(outVar, propertyId, scriptContext);
                            writeOut(isArray, isByRef, paramIndex, stackBytesRead, parameterLocationIndex, parameter, paramVar);
                        }
                    }
                } 
                UpdateParameterReadByModelParameter(parameter);
            });
        
            // Now that all out parameters have been successfully marshaled. Transfer ownership of them to the caller,
            // so the ProjectionMarshaler will not release them.
            marshal.TransferOwnershipOfDelegateOutTypes();
        }
#ifdef ENABLE_JS_ETW
        if (eventInfo != nullptr)
        {
            if(EventEnabledJSCRIPT_PROJECTION_INVOKEEVENT_STOP()) {
                LPCWSTR runtimeClassName = StringOfId(scriptContext, signature->runtimeClassNameId);
                EventWriteJSCRIPT_PROJECTION_INVOKEEVENT_STOP(runtimeClassName, StringOfId(scriptContext, eventInfo->nameId));
            }
        }
        else
        {
            if(EventEnabledJSCRIPT_PROJECTION_INVOKEJSDELEGATE_STOP()) {
                LPCWSTR runtimeClassName = StringOfId(scriptContext, signature->runtimeClassNameId);
                EventWriteJSCRIPT_PROJECTION_INVOKEJSDELEGATE_STOP(runtimeClassName, StringOfId(scriptContext, signature->nameId));
            }
        }
#endif
    }
    CUnknownMethodImpl_ArgT_ReportError_Epilog()
#pragma warning(pop)

    void Delegate::MarkScriptContextDependentResources(Recycler *recycler, bool inPartialCollect)
    {
        if (inPartialCollect || IsRooted())
        {
            StrongMark(recycler);
        }
        else if(callback)
        {
            WeakMark(recycler);
        }
    }

    bool Delegate::SupportsWeakDelegate()
    {
        return projectionContext->SupportsWeakDelegate();
    }

    bool Delegate::IsRooted()
    {
        // Delegate is rooted if:
        // 0. We can't support the weakDelegate
        // 1. This is not a event handling delegate
        // 2. There are more than one JSProxies for same winRT object so refCount on winrtObject is > 2 the one GC can track
        // 3. There are more than 1 references on this delegate which would mean somebody else has external reference to it.
        return (!SupportsWeakDelegate() || eventProjectionHandler == nullptr || eventProjectionHandler->GetGCTrackedRefCount() != 1 || m_pWeakReference->GetStrongRefCount() > 1);
    }

#ifdef WINRTFINDPREMATURECOLLECTION
    void Delegate::VerifyNotDisconnected()
    {
        Assert(callback != nullptr && callback->Get() != nullptr);
    }
#endif

    void Delegate::WeakMark(Recycler *recycler)
    {
        Assert(callback != nullptr);
        recycler->TryMarkNonInterior(callback);
    }

    void Delegate::StrongMark(Recycler *recycler)
    {
        if (callback)
        {
            WeakMark(recycler);

            // Mark function object only if we are rooted.
            auto jsFunc = callback->Get();
            if (jsFunc != nullptr)
            {
                recycler->TryMarkNonInterior(jsFunc);
            }
        }
    }

    void Delegate::RemoveStrongRef()
    {
        Assert(SupportsWeakDelegate());
        callback = nullptr;
    }

    USHORT Delegate::GetWinrtTypeFlags()
    {
        return PROFILER_HEAP_OBJECT_FLAGS_WINRT_DELEGATE;
    }

    UINT Delegate::GetHeapObjectRelationshipInfoSize()
    {
        return ActiveScriptProfilerHeapEnum::GetHeapObjectInternalPropertyInfoSize();
    }

    void Delegate::FillHeapObjectRelationshipInfo(ProfilerHeapObjectOptionalInfo *optionalInfo)
    {
        this->GetHeapEnum()->FillHeapObjectInternalUnnamedJSVarProperty(optionalInfo, GetCallback());
    }

    // Info:        This thunk is used if while projecting back the delegate object is not suppose to retain the identity. 
    //              The job of this thunk is to forward the call to the underlying function
    // Parameters:  standard thunk parameters
    Var DelegateForwarderThunk(Var method, Js::CallInfo callInfo, ...)
    {
#if DBG
        Js::JavascriptFunction* func = Js::JavascriptFunction::FromVar(method);
        Assert(Js::JavascriptOperators::GetTypeId(func) == Js::TypeIds_Function);
        Assert(func->IsWinRTFunction());
#endif
        ARGUMENTS(args, callInfo);
        Js::JavascriptWinRTFunction * function = Js::JavascriptWinRTFunction::FromVar(method);
        Js::JavascriptFunction *signature = reinterpret_cast<Js::JavascriptFunction *>(function->GetSignature());
        Assert(signature != NULL);
        return signature->CallFunction(args);
    }
}
