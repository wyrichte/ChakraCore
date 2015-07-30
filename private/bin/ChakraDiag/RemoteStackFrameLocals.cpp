//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

namespace JsDiag
{
    const CString RemoteStackFrameLocals::s_localsName(L"Locals");
    const CString RemoteStackFrameLocals::s_scopeName(L"[Scope]");
    const CString RemoteStackFrameLocals::s_globalsScopeName(L"[Globals]");

    void RemoteStackFrameLocals::Init(RemoteStackFrame* frame)
    {
        __super::Init(frame);
        TryEnsureWalker(); // pre-create walker. Need to initialize locals context.
    }

    bool_result RemoteStackFrameLocals::TryGetEnumeratorEx(JS_PROPERTY_MEMBERS members, _Outptr_ IJsEnumDebugProperty **ppEnum)
    {
        if (members == JS_PROPERTY_MEMBERS::JS_PROPERTY_MEMBERS_ARGUMENTS)
        {
            CComPtr<IJsDebugPropertyInternal> arguments;
            if (TryEnsureWalker()
                && GetWalker()->TryGetProperty(ArgumentsObjectProperty::GetDisplayName(), &arguments))
            {
                return SUCCEEDED(arguments->GetMembers(JS_PROPERTY_MEMBERS::JS_PROPERTY_MEMBERS_ALL, ppEnum));
            }

            return false;
        }

        return __super::TryGetEnumerator(ppEnum);
    }

    bool_result RemoteStackFrameLocals::TryCreateWalker(_Outptr_ WalkerType** ppWalker)
    {
        CreateComObject(m_frame, ppWalker);
        return true;
    }

    template <class T>
    void StackFrameWalker<T>::Init(RemoteStackFrame* frame)
    {
        __super::Init(frame->GetInspectionContext(), nullptr);
    }

    template <class T>
    ThreadContext* StackFrameWalker<T>::GetThreadContext(RemoteStackFrame* frame)
    {
        RemoteScriptContext scriptContext(
            frame->GetInspectionContext()->GetReader(),
            frame->GetRemoteFunctionBody()->ToTargetPtr()->m_scriptContext);
        return scriptContext.GetThreadContext();
    }

    template <class Walker>
    uint LocalsWalker::GetCount(Walker& walker)
    {
        return walker ? walker->GetCount() : 0;
    }

    template <class Walker>
    bool LocalsWalker::GetNextLocal(Walker& walker, uint& index, _Outptr_ IJsDebugPropertyInternal **ppDebugProperty)
    {
        if (walker)
        {
            if (walker->GetNextProperty(index, ppDebugProperty))
            {
                return true;
            }

            index -= walker->GetCount(); // Prepare for next walker
        }

        return false;
    }

    template <class Walker>
    bool_result LocalsWalker::TryGetProperty(Walker& walker, const CString& name, _Outptr_ IJsDebugPropertyInternal **ppDebugProperty)
    {
        return walker && walker->TryGetProperty(name, ppDebugProperty);
    }

    uint LocalsWalker::GetCount()
    {
        uint count = __super::GetCount();

        count += GetCount(m_globalsWalker);
        count += GetCount(m_regSlotLocalsWalker);
        count += GetCount(m_slotArrayLocalsWalker);
        count += GetCount(m_objectLocalsWalker);
        count += GetCount(m_extraScopeLocalsWalker);
        count += GetCount(m_scopeWalker);

        return count;
    }

    bool_result LocalsWalker::GetNextProperty(uint index, _Outptr_ IJsDebugPropertyInternal **ppDebugProperty)
    {
        *ppDebugProperty = NULL;

        if (__super::GetNextProperty(index, ppDebugProperty))
        {
            return true;
        }
        index -= __super::GetCount();

        return GetNextLocal(m_globalsWalker, index, ppDebugProperty)
            || GetNextLocal(m_extraScopeLocalsWalker, index, ppDebugProperty)
            || GetNextLocal(m_regSlotLocalsWalker, index, ppDebugProperty)
            || GetNextLocal(m_slotArrayLocalsWalker, index, ppDebugProperty)
            || GetNextLocal(m_objectLocalsWalker, index, ppDebugProperty)
            || GetNextLocal(m_scopeWalker, index, ppDebugProperty);
    }

    //
    // Used by expression evaluation to do scoped variable name lookup.
    //
    bool_result LocalsWalker::TryGetProperty(const CString& name, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty)
    {
        // this, arguments, local var
        if (__super::TryGetProperty(name, ppDebugProperty)
            || TryGetProperty(m_extraScopeLocalsWalker, name, ppDebugProperty)
            || TryGetProperty(m_regSlotLocalsWalker, name, ppDebugProperty)
            || TryGetProperty(m_slotArrayLocalsWalker, name, ppDebugProperty)
            || TryGetProperty(m_objectLocalsWalker, name, ppDebugProperty)
            || TryGetProperty(m_globalsWalker, name, ppDebugProperty)) // Check globals at last because it is more expensive
        {
            return true;
        }

        // Scopes
        return m_scopeWalker && m_scopeWalker->TryGetScopedProperty(name, ppDebugProperty);
    }

    void LocalsWalker::Init(RemoteStackFrame* frame)
    {
        auto context = frame->GetInspectionContext();
        auto reader = context->GetReader();

        RemoteDiagFrame* diagFrame = frame->GetTempDiagFrame();
        Assert(diagFrame);

        // Initialize locals context
        context->InitLocalsContext(diagFrame->GetScriptContext());

        RemoteFunctionBody* funcBody = diagFrame->GetRemoteFunctionBody();
        RemoteFunctionBody_SourceInfo sourceInfo(reader, funcBody);        
        bool isStrictMode = funcBody->ToTargetPtr()->GetIsStrictMode();

        RegSlot frameDisplayRegister = sourceInfo->frameDisplayRegister;
        RegSlot objectRegister = sourceInfo->objectRegister;

        RemoteFrameDisplay display(reader, diagFrame->GetFrameDisplay(frameDisplayRegister));
        uint scopeCount = display->GetLength();
        uint nextStartIndex = 0;

        // this
        InsertThis(frame, isStrictMode);

        // {exception}
        InsertException(frame);

        InsertReturnValue(frame);

        bool isGlobalFunc = funcBody->ToTargetPtr()->GetIsGlobalFunc();

        // In the eval function, we will not show global items directly, instead they should go as a group node.

        if (isGlobalFunc && !funcBody->ToTargetPtr()->IsEval()) // global func
        {
            CreateComObject(
                frame, reinterpret_cast<const DynamicObject*>(diagFrame->GetRootObject()),
                GetOwnerDebugProperty(),
                &m_globalsWalker);
        }

        // variables
        if (frameDisplayRegister != 0)
        {
            if (objectRegister != 0)
            {
                CreateComObject(
                    frame, reinterpret_cast<const DynamicObject*>(display.GetItem(nextStartIndex++)),
                    GetOwnerDebugProperty(),
                    &m_objectLocalsWalker);
            }
            else
            {
                UINT scopeSlotArraySize = funcBody->ToTargetPtr()->scopeSlotArraySize;
                if (scopeSlotArraySize > 0)
                {
                    CreateComObject(
                        frame, 
                        (Js::Var *)display.GetItem(nextStartIndex++), &m_slotArrayLocalsWalker);
                }
                else if (scopeCount > 0)
                {
                    nextStartIndex++; // A dummy scope with null register is created. Skip this.
                }
            }
        }

        if (funcBody->ToTargetPtr()->propertyIdOnRegSlotsContainer)
        {
            RemotePropertyIdOnRegSlotsContainer propIdContainer(reader, funcBody->ToTargetPtr()->propertyIdOnRegSlotsContainer);

            if (propIdContainer->length > 0)
            {
                CreateComObject(frame, propIdContainer, nullptr /*no debuggerscope*/, &m_regSlotLocalsWalker);
            }
        }

        // Inserting a fake 'arguments' (if needed)
        // Find if we have arguments var in above walkers, if we don't have one we will insert the fake one here.
        CComPtr<IJsDebugPropertyInternal> arguments;
        if (!isGlobalFunc && !TryGetProperty(ArgumentsObjectProperty::GetDisplayName(), &arguments))
        {
            InsertFakeArgumentsObject(frame, isStrictMode);
        }


        // variables from extra scopes
        if (sourceInfo->pScopeObjectChain)
        {
            CreateComObject(frame, sourceInfo->pScopeObjectChain, &m_extraScopeLocalsWalker);
        }

        // scopes, including [Globals]
        CreateComObject(frame, display, nextStartIndex, &m_scopeWalker);
    }

    void LocalsWalker::InsertException(RemoteStackFrame* frame)
    {
        auto context = frame->GetInspectionContext();
        auto reader = context->GetReader();

        RemoteDiagFrame* diagFrame = frame->GetTempDiagFrame();
        Assert(diagFrame);

        RemoteScriptContext scriptContext(reader, diagFrame->GetScriptContext());
        RemoteProbeContainer probeContainer(reader, scriptContext.GetProbeContainer());

        Js::Var exception = probeContainer.GetExceptionObject();
        if (exception)
        {
            CComPtr<IJsDebugPropertyInternal> prop;
            context->CreateDebugProperty(PROPERTY_INFO(CString(L"{exception}"), exception), /*parent*/nullptr, &prop);
            __super::InsertItem(prop);
        }
    }

    void LocalsWalker::InsertReturnValue(RemoteStackFrame* frame)
    {
        if (!frame->IsTopJavascriptFrame())
        {
            // supported only on the top javascript frame
            return;
        }

        auto context = frame->GetInspectionContext();
        auto reader = context->GetReader();

        RemoteDiagFrame* diagFrame = frame->GetTempDiagFrame();
        Assert(diagFrame);

        RemoteScriptContext scriptContext(reader, diagFrame->GetScriptContext());
        RemoteProbeContainer remoteProbeContainer(reader, scriptContext.GetProbeContainer());

        RemoteProbeManager remoteProbeManager(reader, remoteProbeContainer->pProbeManager);
        StepController *stepController = remoteProbeManager.GetFieldAddr<StepController>(offsetof(ProbeManager, stepController));

        RemoteStepController remoteStepController(reader, stepController);
        ReturnedValueList *returnedValueList = remoteStepController->GetReturnedValueList();
        if (returnedValueList != nullptr)
        {
            RemoteReturnedValueList remoteReturnValueList(reader, returnedValueList);
            for (int index = 0; index < remoteReturnValueList.Count(); index++)
            {
                ReturnedValue *returnValue = remoteReturnValueList.Item(index);
                RemoteReturnedValue remoteReturnedValue(reader, returnValue);

                CComPtr<IJsDebugPropertyInternal> prop;
                if (remoteReturnedValue->isValueOfReturnStatement)
                {
                    Js::Var object = frame->GetTempDiagFrame()->GetReg(Js::FunctionBody::ReturnValueRegSlot);
                    context->CreateDebugProperty(PROPERTY_INFO(CString(L"[Return value]"), object), /*parent*/nullptr, &prop);
                    __super::InsertItem(prop);
                }
                else
                {
                    CString name;
                    RemoteJavascriptFunction remoteJavascriptFunction(reader, remoteReturnedValue->calledFunction);
                    if (remoteJavascriptFunction.IsScriptFunction())
                    {
                        RemoteJavascriptFunction::TryReadDisplayName(reader, remoteReturnedValue->calledFunction, &name);
                    }
                    else
                    {
                        name = remoteJavascriptFunction.GetDisplayNameString(context);
                    }
                    if (!name.IsEmpty())
                    {
                        context->CreateDebugProperty(PROPERTY_INFO(CString(L"[" + name + " returned]"), remoteReturnedValue->returnedValue), /*parent*/nullptr, &prop);
                        __super::InsertItem(prop);
                    }
                }
            }
        }

    }


    void LocalsWalker::InsertThis(RemoteStackFrame* frame, bool isStrictMode)
    {
        auto context = frame->GetInspectionContext();
        auto reader = context->GetReader();

        RemoteFunctionBody* funcBody = frame->GetRemoteFunctionBody();
        RemoteDiagFrame* diagFrame = frame->GetTempDiagFrame();
        Assert(diagFrame);

        Js::Var thisVar = NULL;
        Js::TypeId typeId = Js::TypeIds_Undefined;
        bool needToObject = false;

        // TODO: Need to handle lambda case here.  Use LocalsWalker in a mode that allows _lexicalThisSlotSymbol property;
        // but also need to identify function as Lambda and having captured this (need function's attributes)
        if (diagFrame->GetInParamCount() > 0)
        {
            RemoteArray<Js::Var> inParams(reader, diagFrame->GetInParams());
            thisVar = inParams[0];
            typeId = context->GetTypeId(thisVar);
        }

        if (isStrictMode)
        {
            if (typeId == Js::TypeIds_ActivationObject || !thisVar)
            {
                thisVar = context->GetJavascriptLibrary().GetUndefined();
            }
        }
        else
        {
            if (typeId == Js::TypeIds_Null || typeId == Js::TypeIds_Undefined || typeId == Js::TypeIds_ActivationObject)
            {
                //TODO: ModuleRoot
                thisVar = diagFrame->GetRootObject();
                thisVar = RemoteGlobalObject(reader, static_cast<Js::GlobalObject*>(thisVar)).ToThis();
            }
            else if (!context->IsObjectType(typeId))
            {
                needToObject = true;
            }
        }

        CComPtr<IJsDebugPropertyInternal> prop;
        context->CreateDebugProperty(PROPERTY_INFO(CString(L"this"), thisVar), /*parent*/nullptr, &prop);

        if (needToObject)
        {
            CComPtr<IJsDebugPropertyInternal> obj;
            if (!prop->TryToObject(&obj))
            {
                AssertMsg(false, "Only null/undefined would fail ToObject, but they should have been filtered out already.");
                DiagException::Throw(E_UNEXPECTED, DiagErrorCode::THIS_TO_OBJECT_FAIL);
            }
            prop = obj;
        }

        __super::InsertItem(prop);
    }

    void LocalsWalker::InsertFakeArgumentsObject(RemoteStackFrame* frame, bool isStrictMode)
    {
        CComPtr<IJsDebugPropertyInternal> argsObj;
        GetArgumentsObjectProperty(frame, isStrictMode, /*opt argumentsObject*/nullptr, &argsObj);
        __super::InsertItem(argsObj);
    }

    // Get existing arguments object from stack frame or create a fake debug property.
    //      When parameter "argumentsObject" is not null, prefer to return existing arguments object (and do not create debug property for it).
    //
    void LocalsWalker::GetArgumentsObjectProperty(RemoteStackFrame* frame, bool isStrictMode, _Out_opt_ Js::Var* argumentsObject, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty)
    {
        auto context = frame->GetInspectionContext();
        auto reader = context->GetReader();

        // Check if the argument object is populated on the frame or not. if that is the case, we can use that object.
        Js::Var argumentObjectOnSlot = frame->GetTempDiagFrame()->GetArgumentsObject();
        if (argumentObjectOnSlot)
        {
            if (argumentsObject)
            {
                *argumentsObject = argumentObjectOnSlot; // Return argumentsObject if preferred
                *ppDebugProperty = nullptr;
                return;
            }

            context->CreateDebugProperty(PROPERTY_INFO(ArgumentsObjectProperty::GetDisplayName(), argumentObjectOnSlot), /*parent*/nullptr, ppDebugProperty);
            return;
        }

        CComPtr<FakeObjectProperty> argsObj;

        CreateComObject(context,
            ArgumentsObjectProperty::GetDisplayName(), ArgumentsObjectProperty::GetDisplayType(),
            context->GetJavascriptLibrary().GetObjectPrototype(),
            /*parent*/nullptr,
            &argsObj);

        RemoteDiagFrame* diagFrame = frame->GetTempDiagFrame();
        Assert(diagFrame);

        uint paramCount = diagFrame->GetInParamCount();
        if (paramCount > 0)
        {
            paramCount--; // Skip "this"
            RemoteArray<Js::Var> inParams(reader, diagFrame->GetInParams() + 1);
            for (uint i = 0; i < paramCount; i++)
            {
                argsObj->AddItem(PROPERTY_INFO(i, inParams[i]));
            }
        }
        argsObj->AddProperty(PROPERTY_INFO(CString(L"length"), static_cast<double>(paramCount)));

        if (isStrictMode)
        {
            // In strict mode we should have 2 accessors. We can't inspect them, so effectively we can present 2 simple string properties.
            CString errMessage(context->GetDebugClient()->GetErrorString(DIAGERR_FunctionCallNotSupported));
            argsObj->AddProperty(PROPERTY_INFO(CString(L"callee"), errMessage));
            argsObj->AddProperty(PROPERTY_INFO(CString(L"caller"), errMessage));
        }
        else
        {
            argsObj->AddProperty(PROPERTY_INFO(CString(L"callee"), diagFrame->GetFunction()));
        }

        if (argumentsObject)
        {
            *argumentsObject = nullptr;
        }
        *ppDebugProperty = argsObj.Detach(); // Pass ownership
    }

    // Returns the current scope chain for the passed in frame (if it exists).
    Js::ScopeObjectChain::ScopeObjectChainList* LocalsWalker::GetScopeChain(RemoteStackFrame* frame)
    {
        Assert(frame);

        InspectionContext* context = frame->GetInspectionContext();
        IVirtualReader* reader = context->GetReader();

        RemoteFunctionBody* funcBody = frame->GetRemoteFunctionBody();

        Assert(funcBody);

        RemoteFunctionBody_SourceInfo sourceInfo(reader, funcBody);
        if (sourceInfo->pScopeObjectChain == nullptr)
        {
            return nullptr;
        }

        Js::ScopeObjectChain::ScopeObjectChainList* scopeChain = RemoteData<ScopeObjectChain>(reader, sourceInfo->pScopeObjectChain)->pScopeChain;
        return scopeChain;
    }

    // Determines whether or not the frame has a local scope chain.
    bool LocalsWalker::HasScopeChain(RemoteStackFrame* frame)
    {
        return GetScopeChain(frame) != nullptr;
    }

    // Determines if the given property is valid for display in the locals window.
    // Cases in which the property is valid are:
    // 1. It is not represented by an internal property.
    // 2. It is a var property.
    // 3. It is a let/const property in scope and is not in a dead zone.
    bool LocalsWalker::IsPropertyValid(RemoteStackFrame* frame, Js::PropertyId propertyId, Js::RegSlot location, bool *isLetConst, bool* isInDeadZone)
    {
        // This check should have already occurred in the walkers.
        Assert(!Js::IsInternalPropertyId(propertyId));
        Assert(isLetConst);
        Assert(isInDeadZone);

        if (propertyId == Js::PropertyIds::_lexicalThisSlotSymbol)
        {
            return false;
        }

        if (propertyId == Js::PropertyIds::_superReferenceSymbol)
        {
            return false;
        }

        if (propertyId == Js::PropertyIds::_lexicalNewTargetSymbol)
        {
            return false;
        }

        Js::ScopeObjectChain::ScopeObjectChainList* scopeChain = GetScopeChain(frame);
        if (scopeChain == nullptr)
        {
            return true;
        }

        InspectionContext* context = frame->GetInspectionContext();
        IVirtualReader* reader = context->GetReader();

       *isLetConst = false;

        int offset = frame->GetByteCodeOffset();

        bool wasScopeFound = RemoteList<DebuggerScope*>(reader, scopeChain).MapUntil(
            [&](uint i, DebuggerScope* item)
        {
            RemoteDebuggerScope debuggerScope(reader, item);

            DebuggerScopeProperty debuggerScopeProperty;
            if (debuggerScope.ContainsProperty(frame, propertyId, location, &debuggerScopeProperty))
            {
                bool isOffsetInScope = debuggerScope->range.Includes(offset);
                // If the register is found in any scope, then it is let/const since these are
                // the only registers tracked by the scope chain.

                // For the Object scope, all the properties will have the same location (-1) so they can match. Use extra check below to determine if the property valid
                *isLetConst = isOffsetInScope || !debuggerScope->IsBlockObjectScope();

                if (isOffsetInScope)
                {
                    *isInDeadZone = debuggerScopeProperty.IsInDeadZone(offset);
                    return true;
                }
            }

            return false;
        });

        if (wasScopeFound)
        {
            return true;
        }

        // If the register was not found in any scopes, then it's a var and should be in scope.
        return !*isLetConst;
    }

    void RegSlotLocalsWalker::Init(RemoteStackFrame* frame, const RemotePropertyIdOnRegSlotsContainer& propIdContainer, DebuggerScope * debuggerScope)
    {
        __super::Init(frame);

        InspectionContext* context = frame->GetInspectionContext();
        IVirtualReader* reader = context->GetReader();
        RemoteThreadContext threadContext(reader, GetThreadContext(frame));

        bool hasScopeChain = LocalsWalker::HasScopeChain(frame);

        int offset = frame->GetByteCodeOffset();

        uint length = propIdContainer->length;
        for (uint index = 0; index < length; index++)
        {
            Js::PropertyId propId;
            RegSlot reg;

            propIdContainer.FetchItemAt(index, frame->GetRemoteFunctionBody(), &propId, &reg);
            if (!Js::IsInternalPropertyId(propId))
            {
                const PropertyRecord* propertyName = threadContext.GetPropertyName(propId);

                bool shouldInsert = false;
                bool isInDeadZone = false;

                // Check if we're in a dead zone at block scope.
                if (debuggerScope != nullptr)
                {
                    RemoteDebuggerScope remoteScope(reader, debuggerScope);
                    shouldInsert = remoteScope.ContainsValidProperty(frame, propId, reg, offset, &isInDeadZone);
                }
                else
                {
                    bool isLetConst = false;
                    shouldInsert = LocalsWalker::IsPropertyValid(frame, propId, reg, &isLetConst, &isInDeadZone) && !isLetConst;
                }

                if (shouldInsert)
                {
                    // Need to check for the register being valid as it can be null.
                    // Some cases include non-static dead zone (for switch, for example).
                    Js::Var data = frame->GetTempDiagFrame()->GetReg(reg);

                    // If the user didn't supply an arguments object, a fake one will
                    // be created [later] when evaluating LocalsWalker::ShouldInsertFakeArguments().
                    if (!(propId == PropertyIds::arguments && data == nullptr))
                    {
                        if (context->GetJavascriptLibrary().IsUndeclBlockVar(data))
                        {
                            isInDeadZone = true;
                        }

                        if (isInDeadZone)
                        {
                            // Data can be null in the case of function let/const dead zone.
                            data = context->GetJavascriptLibrary().GetDebuggerDeadZoneBlockVariableString();
                        }
                        PROPERTY_INFO info(context->ReadPropertyName(propertyName), data);
                        __super::InsertItem(info);
                    }
                }
            }
        }
    }

    void SlotArrayLocalsWalker::Init(RemoteStackFrame* frame, void* slotArrayAddr)
    {
        __super::Init(frame);

        auto context = frame->GetInspectionContext();
        auto reader = context->GetReader();
        RemoteThreadContext threadContext(reader, GetThreadContext(frame));

        RemoteArray<Js::Var> slotArray(reader, reinterpret_cast<const Js::Var*>(slotArrayAddr));
        
        if(RemoteFunctionBody::Is(context, slotArray.Item(ScopeSlots::ScopeMetadataSlotIndex)))
        {
            RemoteFunctionBody funcBody(reader, reinterpret_cast<FunctionBody*>(slotArray.Item(ScopeSlots::ScopeMetadataSlotIndex)));            
            uint slotCount = (uint)slotArray.Item(ScopeSlots::EncodedSlotCountSlotIndex);
            if (funcBody->propertyIdsForScopeSlotArray)
            {
                RemoteArray<Js::PropertyId> propertyIdsForScopeSlotArray(reader, funcBody->propertyIdsForScopeSlotArray);

                bool hasScopeChain = LocalsWalker::HasScopeChain(frame);
                for (uint index = 0; index < slotCount; index++)
                {
                    Js::PropertyId propId = propertyIdsForScopeSlotArray.Item(index);
                    if (propId != Js::Constants::NoProperty && !Js::IsInternalPropertyId(propId))
                    {
                        const PropertyRecord* propertyName = threadContext.GetPropertyName(propId);

                        bool isLetConst = false;
                        bool isInDeadZone = false;
                        bool shouldInsert = !hasScopeChain || (LocalsWalker::IsPropertyValid(frame, propId, index, &isLetConst, &isInDeadZone));
                        if (shouldInsert)
                        {
                            Js::Var data = slotArray.Item(index + ScopeSlots::FirstSlotIndex);

                            if (context->GetJavascriptLibrary().IsUndeclBlockVar(data))
                            {
                                isInDeadZone = true;
                            }

                            if (isInDeadZone)
                            {
                                data = context->GetJavascriptLibrary().GetDebuggerDeadZoneBlockVariableString();
                            }

                            // TODO: Do we handle Js::PropertyIds::_lexicalThisSlotSymbol?
                            PROPERTY_INFO info(context->ReadPropertyName(propertyName), data);
                            __super::InsertItem(info);
                        }
                    }
                }
            }
        }
        else
        {
           auto context = frame->GetInspectionContext();
           auto reader = context->GetReader();

           RemoteThreadContext threadContext(reader, GetThreadContext(frame));
           RemoteArray<Js::Var> slotArray(reader, reinterpret_cast<const Js::Var*>(slotArrayAddr));

           Assert(!RemoteFunctionBody::Is(context, slotArray[ScopeSlots::ScopeMetadataSlotIndex]));

           DebuggerScope* debuggerScope = (DebuggerScope*)slotArray[ScopeSlots::ScopeMetadataSlotIndex];
           if(debuggerScope)
           {
               RemoteDebuggerScope remoteScope(reader, debuggerScope);

               if (remoteScope->scopeProperties != nullptr)
               {
                   RemoteDebuggerScopePropertyList scopePropertyList(reader, remoteScope->scopeProperties);            
                   uint offset = frame->GetByteCodeOffset();
                   scopePropertyList.Map( [&] (uint i, DebuggerScopeProperty& debuggerScopeProperty)
                   {
                       const PropertyRecord* propertyName = threadContext.GetPropertyName(debuggerScopeProperty.propId);
                       Js::Var data = slotArray.Item(debuggerScopeProperty.location + ScopeSlots::FirstSlotIndex);
                       if (context->GetJavascriptLibrary().IsUndeclBlockVar(data)) // Accounting for dead zone
                       {
                           // In a dead zone so replace the var value with the dead zone display string.
                           data = context->GetJavascriptLibrary().GetDebuggerDeadZoneBlockVariableString();
                       }

                       PROPERTY_INFO info(context->ReadPropertyName(propertyName), data);
                       this->InsertItem(info);
                   });
               }
           }
        }
    }

    void ActivationObjectWalker::Init(RemoteStackFrame* frame, const DynamicObject* var, IJsDebugPropertyInternal* ownerDebugProperty)
    {
        Assert(frame);
        Assert(var);

        // Ensure the frame is assigned before entering the base class init, since it it used for
        // valid property inspection.
        m_frame = frame;
        __super::Init(frame->GetInspectionContext(), var, ownerDebugProperty);
    }

    void GlobalObjectWalker::Init(RemoteStackFrame* frame, const DynamicObject* var, IJsDebugPropertyInternal* ownerDebugProperty)
    {
        Assert(frame);
        Assert(var);

        __super::Init(frame, var, ownerDebugProperty);

        // Get the global let/const variables
        CAtlArray<PROPERTY_INFO> itemsLetConst;
        auto listener = MakePropertyListener([&](const PROPERTY_INFO& info, const Js::PropertyId propertyId) -> bool
        {
            if (!m_context->GetJavascriptLibrary().IsUndeclBlockVar(info.data))
            {
                itemsLetConst.Add(info);
            }
            return true;
        });

        AutoPtr<ITypeHandler> typeHandler = m_context->CreateTypeHandler(var);
        if (typeHandler)
        {
            typeHandler->EnumLetConstGlobals(&listener);
        }

        // Prepend the global let/const variables
        m_items.InsertArrayAt(0, &itemsLetConst);
    }

    bool_result GlobalObjectWalker::TryGetProperty(const CString& name, _Out_ IJsDebugPropertyInternal** ppDebugProperty)
    {
        if (__super::TryGetProperty(name, ppDebugProperty))
        {
            return true;
        }

        // Special non-enumerable property lookup for GlobalObject.
        PROPERTY_INFO prop;
        {
            // TODO: Ideally we'd lookup by string name through type handler.
            auto listener = MakePropertyListener([&](const PROPERTY_INFO& info, const Js::PropertyId propertyId) -> bool
            {
                if (info.name == name)
                {
                    prop = info;
                    return false; // Found, stop enumeration
                }
                return true; // continue enumeration
            });

            DynamicObject* obj = (DynamicObject*)m_frame->GetTempDiagFrame()->GetRootObject();
            AutoPtr<ITypeHandler> typeHandler = m_context->CreateTypeHandler(obj);
            if (typeHandler)
            {
                typeHandler->EnumProperties(&listener, /*requireEnumerable*/false);
            }
        }

        if (!prop.IsEmpty())
        {
            m_context->CreateDebugProperty(prop, GetOwnerDebugProperty(), ppDebugProperty);
            return true;
        }
        return false;
    }

    void DiagScopeLocalsWalker::Init(RemoteStackFrame* frame, ScopeObjectChain* scopeObjectChain)
    {
        __super::Init(/*owner*/NULL);

        auto context = frame->GetInspectionContext();
        auto reader = context->GetReader();
        RemoteScriptContext scriptContext(
            frame->GetInspectionContext()->GetReader(),
            frame->GetRemoteFunctionBody()->ToTargetPtr()->m_scriptContext);
        RemoteThreadContext threadContext(reader, scriptContext.GetThreadContext());

        // Look for scopes which encompass current offset.
        RemoteDiagFrame* diagFrame = frame->GetTempDiagFrame();
        Assert(diagFrame);

        int offset = frame->GetByteCodeOffset();

        auto pScopeChain = RemoteData<ScopeObjectChain>(reader, scopeObjectChain)->pScopeChain;
        RemoteList<DebuggerScope*>(reader, pScopeChain).MapReverse(
            [&](uint i, DebuggerScope* item)
        {
            RemoteData<DebuggerScope> debuggerScope(reader, item);

            if (debuggerScope->range.Includes(offset)
             && (debuggerScope->IsOwnScope() || debuggerScope->scopeType == DiagBlockScopeDirect)
             && debuggerScope->scopeProperties != nullptr)
            {
                auto scopeProperties = RemoteList<DebuggerScopeProperty>(reader, debuggerScope->scopeProperties);

                // The catch/with scope object is stored as the only element of the scope properties list.
                Assert(scopeProperties);
                // BlockScopeDirect can have 0 property
                if (scopeProperties.Count() > 0)
                {
                    DebuggerScopeProperty prop = scopeProperties.Item(0);
                    CString name = context->ReadPropertyName(threadContext.GetPropertyName(prop.propId));

                    switch (debuggerScope->scopeType)
                    {
                    case DiagCatchScopeDirect:
                        {
                            Js::Var data = diagFrame->GetReg(prop.location);
                            InsertWalkerForProperty(context, PROPERTY_INFO(name, data));
                        }
                        break;
                    case DiagCatchScopeInSlot:
                        {
                            Js::Var* pArr = (Js::Var*)diagFrame->GetReg(debuggerScope->scopeLocation);
                            Js::Var data = RemoteArray<Js::Var>(reader, pArr).Item(Js::ScopeSlots::FirstSlotIndex);
                            InsertWalkerForProperty(context, PROPERTY_INFO(name, data));
                        }
                        break;
                    case DiagBlockScopeInSlot:
                        {
                            Js::Var* slotArrayAddr = (Js::Var*)diagFrame->GetReg(debuggerScope->scopeLocation);
                            CComPtr<SlotArrayLocalsWalker> propertyWalker;
                            CreateComObject(frame, slotArrayAddr, &propertyWalker);
                            this->InsertWalker(propertyWalker);
                        }
                        break;
                    case DiagCatchScopeInObject:
                        {
                            Js::Var object = diagFrame->GetReg(debuggerScope->scopeLocation);
                            PROPERTY_INFO info;
                            if (context->GetProperty(object, prop.propId, &info))
                            {
                                // Name is not populated as a part of GetProperty, lets populate it now.
                                info.name = name;
                                InsertWalkerForProperty(context, info);
                            }
                        }
                        break;
                    case DiagWithScope:
                        {
                            Js::Var object = diagFrame->GetReg(prop.location);
                            CComPtr<WithObjectLocalsWalker> walker;
                            CreateComObject(context, (const Js::DynamicObject *)object, &walker);
                            this->InsertWalker(walker);
                        }
                        break;
                    case DiagBlockScopeInObject:
                        {
                            const Js::DynamicObject* scopeObject = (const Js::DynamicObject*)diagFrame->GetReg(debuggerScope->scopeLocation);
                            CComPtr<ActivationObjectWalker> propertyWalker;
                            CreateComObject(frame, scopeObject, /*ownerProperty*/ nullptr, &propertyWalker);
                            this->InsertWalker(propertyWalker);
                        }
                        break;
                    case DiagBlockScopeDirect:
                        {                            
                            RemotePropertyIdOnRegSlotsContainer propIdContainer(reader, frame->GetRemoteFunctionBody()->ToTargetPtr()->propertyIdOnRegSlotsContainer);
                            Assert(propIdContainer->length > 0);

                            CComPtr<RegSlotLocalsWalker> propertyWalker;
                            CreateComObject(frame, propIdContainer, item, &propertyWalker);
                            this->InsertWalker(propertyWalker);
                        }
                        break;
                    default:
                        Assert(false);
                    }
                }
            }
        });
    }

    // Add a walker that contains only one property
    void DiagScopeLocalsWalker::InsertWalkerForProperty(InspectionContext* context, const PROPERTY_INFO& prop)
    {
        CComPtr<SimplePropertyCollectionWalker> walker;
        CreateComObject(context, /*ownerDebugProperty*/nullptr, &walker);
        walker->InsertItem(prop);

        InsertWalker(walker); // Add new walker into collection
    }

    void WithObjectLocalsWalker::Init(InspectionContext* context, const Js::DynamicObject* object)
    {
        __super::Init(context, /*ownerDebugProperty*/nullptr);

        // Flatten all the properties on the with object and its prototype
        RemoteForInObjectEnumerator forIn(context, (const Js::DynamicObject *)object);
        forIn.Enumerate([&](const PROPERTY_INFO& prop) -> bool
        {
            this->InsertItem(prop);
            return true; // return true to continue enumeration
        },
            /*requireEnumerable*/false); // Enumerate all properties, this is only for expression evaluation lookup
    }

    void ScopeSlotsProperty::Init(RemoteStackFrame* frame, Js::Var instance)
    {
        __super::Init(frame, instance);
    }

    void ScopeSlotsProperty::ReadValue()
    {
        auto context = m_frame->GetInspectionContext();
        auto reader = context->GetReader();

        RemoteArray<Js::Var> slots(reader, reinterpret_cast<Js::Var*>(m_instance));
        if(RemoteFunctionBody::Is(context, slots.Item(ScopeSlots::ScopeMetadataSlotIndex)))
        {
            RemoteFunctionBody funcBody(
                reader,
                reinterpret_cast<FunctionBody*>(slots.Item(ScopeSlots::ScopeMetadataSlotIndex)));

            // Read the function name from the function body display string.
            CString displayName;
            if (funcBody.TryReadDisplayName(&displayName))
            {
                m_value = displayName.AllocSysString();
            }
            else
            {
                __super::ReadValue();
            }
        }
        else
        {
            __super::ReadValue();
        }
        
    }

    bool_result ScopeSlotsProperty::TryCreateWalker(_Outptr_ WalkerType** ppWalker)
    {
        CreateComObject(m_frame, m_instance, ppWalker);
        return true;
    }

    void ScopeObjectProperty::ReadValue()
    {
        auto context = GetInspectionContext();
        auto reader = context->GetReader();

        PROPERTY_INFO arguments;
        if (context->GetProperty(m_instance, PropertyIds::arguments, &arguments)
            && arguments.HasData())
        {
            PROPERTY_INFO callee;
            if (context->GetProperty(arguments.data, PropertyIds::callee, &callee)
                && callee.HasData()
                && context->GetTypeId(callee.data) == Js::TypeIds_Function)
            {
                CString name;
                if (RemoteJavascriptFunction::TryReadDisplayName(context->GetReader(), callee.data, &name))
                {
                    m_value = name.AllocSysString();
                    return; // done
                }
            }
        }

        // Something wrong...
        __super::ReadValue();
    }

    bool_result ScopeObjectProperty::TryCreateWalker(_Outptr_ WalkerType** ppWalker)
    {
        CreateComObject(m_frame, reinterpret_cast<DynamicObject*>(m_instance), /*ownerDebugProperty*/nullptr, ppWalker);
        return true;
    }

    bool_result WithObjectScopeProperty::TryCreateWalker(_Outptr_ WalkerType** ppWalker)
    {
        CreateComObject(m_frame->GetInspectionContext(), reinterpret_cast<DynamicObject*>(m_instance), ppWalker);
        return true;
    }

    bool_result GlobalsScopeProperty::TryCreateWalker(_Outptr_ WalkerType** ppWalker)
    {
        Js::Var globalObject = m_frame->GetTempDiagFrame()->GetRootObject();
        CreateComObject(m_frame, reinterpret_cast<DynamicObject*>(globalObject), /*ownerDebugProperty*/nullptr, ppWalker);
        return true;
    }

    void LocalScopeWalker::Init(RemoteStackFrame* frame, const RemoteFrameDisplay& display, uint startIndex)
    {
        auto context = frame->GetInspectionContext();
        auto reader = context->GetReader();
        const Js::Var nullVar = context->GetJavascriptLibrary().GetNull();
        uint scopeCount = display->GetLength();

        for (uint index = startIndex; index < scopeCount; index++)
        {
            Js::Var currentScopeObject = display.GetItem(index);

            if (currentScopeObject != NULL && currentScopeObject != nullVar) // Skip null (dummy scope)
            {
                CComPtr<IJsDebugPropertyInternal> pDebugProperty;
                ScopeType type = GetScopeType(context, currentScopeObject);
                switch(type)
                {
                    case ScopeType_ActivationObject:
                        CreateComObject<ScopeObjectProperty>(frame, currentScopeObject, &pDebugProperty);
                        break;
                    case ScopeType_SlotArray:
                        CreateComObject<ScopeSlotsProperty>(frame, currentScopeObject, &pDebugProperty);
                        break;
                    case ScopeType_WithScope:
                        CreateComObject<WithObjectScopeProperty>(frame, currentScopeObject, &pDebugProperty);
                        InsertHiddenScope(pDebugProperty);
                        continue; // "with" scope goes to hidden scopes only
                }
                __super::InsertItem(pDebugProperty);
            }
        }

        // Add [Globals] scope if it is a non-global func or global function in the eval code.
        if (!frame->GetRemoteFunctionBody()->ToTargetPtr()->GetIsGlobalFunc() || frame->GetRemoteFunctionBody()->ToTargetPtr()->IsEval())
        {
            CComPtr<IJsDebugPropertyInternal> pDebugProperty;
            CreateComObject<GlobalsScopeProperty>(frame, &pDebugProperty);
            __super::InsertItem(pDebugProperty);
        }
    }

    // Insert a hidden scope (for expression evaluation) and record its position relative to visible scopes
    void LocalScopeWalker::InsertHiddenScope(IJsDebugPropertyInternal* scope)
    {
        Assert(scope);
        m_hiddenScopes.Add(HiddenScope(m_items.GetCount(), scope));
    }

    // Retrieve a hidden scope. If no more, return an empty hidden scope (which has a huge invalid hiddenScope.index).
    LocalScopeWalker::HiddenScope LocalScopeWalker::GetHiddenScope(size_t index) const
    {
        return index < m_hiddenScopes.GetCount() ? m_hiddenScopes[index] : HiddenScope();
    }

    // Try lookup a property by name in scopes. Used by expression evaluation.
    bool_result LocalScopeWalker::TryGetScopedProperty(const CString& name, _Outptr_ IJsDebugPropertyInternal** ppDebugProperty)
    {
        size_t hiddenItemIndex = 0;
        HiddenScope hiddenScope = GetHiddenScope(hiddenItemIndex++); // Retrieve first hidden scope if any

        const size_t visibleCount = m_items.GetCount();
        for (size_t i = 0; i <= visibleCount; i++) // Note "<=" here. Needed to cover hidden scopes after m_items.
        {
            // Process hidden scopes at this position first
            while (hiddenScope.index <= i)
            {
                if (hiddenScope.scope->TryGetProperty(name, ppDebugProperty))
                {
                    return true;
                }
                hiddenScope = GetHiddenScope(hiddenItemIndex++); // Retrieve next hidden scope if any
            }

            if (i < visibleCount
                && m_items[i]->TryGetProperty(name, ppDebugProperty))
            {
                return true;
            }
        }

        Assert(hiddenScope.IsEmpty() && hiddenItemIndex >= m_hiddenScopes.GetCount());
        return false;
    }

    ScopeType LocalScopeWalker::GetScopeType(InspectionContext* context, Js::Var instance)
    {
        if(IsActivationObject(context, instance))
        {
            return ScopeType_ActivationObject;
        }
        else
        {
            size_t slotCount = VirtualReader::ReadVirtual<size_t>(context->GetReader(), instance);
            if(slotCount <= ScopeSlots::MaxEncodedSlotCount)
            {
                return ScopeType_SlotArray;
            }
        }
        return ScopeType_WithScope;
    }

    bool LocalScopeWalker::IsActivationObject(InspectionContext* context, Js::Var instance)
    {
        const void* vtable = RemoteRecyclableObject(context->GetReader(), static_cast<RecyclableObject*>(instance)).ReadVTable();
        return context->IsVTable(vtable, Diag_FirstActivationObject, Diag_LastActivationObject);
    }

} // namespace JsDiag.
