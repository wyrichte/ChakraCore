//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2010 by Microsoft Corporation.  All rights reserved.
//---------------------------------------------------------------------------
#include "ProjectionPch.h"
#include "ProjectionMarshaler.h"

#ifdef PROJECTION_METADATA_TRACE
#define TRACE_METADATA(...) { Trace(__VA_ARGS__); }
#else
#define TRACE_METADATA(...)
#endif


namespace Projection
{
    DEFINE_RECYCLER_TRACKER_PERF_COUNTER(ProjectionObjectInstance);

    using namespace ProjectionModel;

    struct FunctionOverload
    {
        size_t arity;
        Js::JavascriptWinRTFunction* function;
        FunctionOverload(size_t arity, Js::JavascriptWinRTFunction* function) : arity(arity), function(function) {}
    };

    struct OverloadSignature
    {
        ProjectionContext * projectionContext;
        MetadataStringId nameId;
        JsUtil::List<FunctionOverload*>* overloads;
        OverloadSignature(ProjectionContext * projectionContext, MetadataStringId nameId, JsUtil::List<FunctionOverload*>* overloads) : projectionContext(projectionContext), nameId(nameId), overloads(overloads) {}
    };

    struct FunctionOfSignatureContinuation
    {
        ProjectionWriter * projectionWriter;
        RtMETHODSIGNATURE signature;
        ThisInfo * thisInfo;
        bool boundsToUndefined;
        FunctionOfSignatureContinuation(ProjectionWriter * projectionWriter, RtMETHODSIGNATURE signature, ThisInfo * thisInfo, bool boundsToUndefined = false)
            : projectionWriter(projectionWriter), signature(signature), thisInfo(thisInfo), boundsToUndefined(boundsToUndefined)
        { }
    };

    Var ProjectionWriter::DelayedFunctionOfSignatureThunk(Var method, Js::CallInfo callInfo, ...)
    {
        Js::JavascriptWinRTFunction* function = Js::JavascriptWinRTFunction::FromVar(method);
        Js::ScriptContext* scriptContext = function->GetScriptContext();
        Js::JavascriptMethod newOriginalEntryPoint = nullptr;
        ARGUMENTS(args, callInfo);
        {
#if DBG
            ProjectionModel::DisallowParsingMetadata disallow(scriptContext->GetConfig()->IsWinRTAdaptiveAppsEnabled());
            ProjectionModel::AllowHeavyOperation allow;
#endif
            Assert(Js::JavascriptOperators::GetTypeId(function) == Js::TypeIds_Function);
            Assert(function->IsWinRTFunction());
            auto continuation = reinterpret_cast<FunctionOfSignatureContinuation*>(function->GetSignature());            

#if DBG
            LPCWSTR name = continuation->projectionWriter->projectionContext->StringOfId(continuation->signature->nameId);
            Assert(name);
#endif

            auto value = continuation->projectionWriter->ContinueFunctionOfSignature(continuation->signature, continuation->thisInfo, function->IsConstructorFunction(), continuation->boundsToUndefined);
            auto actualFunction = Js::JavascriptWinRTFunction::FromVar(value);
            Assert(!function->HasSharedType());

            // We need to change the entryPoint only if not crossSite, because if crosssite its going to be crosssite thunk and we dont want to change that
            if (!function->IsCrossSiteObject())
            {
                auto functionType = function->GetDynamicType();
                functionType->SetEntryPoint(actualFunction->GetEntryPoint());
            }

            auto functionInfo = function->GetWinRTFunctionInfo();
            newOriginalEntryPoint = actualFunction->GetFunctionInfo()->GetOriginalEntryPoint();
            functionInfo->SetOriginalEntryPoint(newOriginalEntryPoint);
            function->ChangeSignature(actualFunction->GetSignature());

            // Use CallFunction with newEntryPoint instead of directly calling function to avoid calling profileThunk again for this function
        }
        return Js::JavascriptFunction::CallFunction<true>(function, newOriginalEntryPoint, args);
    }

    // Info:        Called in the case of an unresolvable name conflict
    // Parameters:  standard thunk parameters
    Var UnresolvableNameConflictThunk(Var method, Js::CallInfo callInfo, ...)
    {
        Js::JavascriptWinRTFunction* func = Js::JavascriptWinRTFunction::FromVar(method);
        Assert(Js::JavascriptOperators::GetTypeId(func) == Js::TypeIds_Function);
        Assert(func->IsWinRTFunction());
        Js::JavascriptWinRTFunction * function = Js::JavascriptWinRTFunction::FromVar(method);
        Js::ScriptContext* scriptContext = function->GetScriptContext();

        // There was an unresolvable name conflict between two properties
        Js::JavascriptError::ThrowTypeError(scriptContext, VBSERR_ActionNotSupported);
    }

    // Info:        Specialized thunk for setting array length
    // Parameters:  standard thunk parameters
    Var ArraySetLengthThunk(Var method, Js::CallInfo callInfo, ...)
    {
        Js::JavascriptWinRTFunction* func = Js::JavascriptWinRTFunction::FromVar(method);
        Assert(Js::JavascriptOperators::GetTypeId(func) == Js::TypeIds_Function);
        Assert(func->IsWinRTFunction());
        ARGUMENTS(args, callInfo);
        Js::JavascriptWinRTFunction * function = Js::JavascriptWinRTFunction::FromVar(method);
        Js::ScriptContext* scriptContext = function->GetScriptContext();
        ThunkSignature<SpecialProjection*> * signature = reinterpret_cast<ThunkSignature<SpecialProjection*>*>(function->GetSignature());
        SpecialProjection * specialization = signature->baggage;

        Js::JavascriptErrorDebug::ClearErrorInfo(scriptContext);
        return VectorArray::SetLengthOfSpecialProjection(specialization, &args);
    }

    // Info:        Specialized thunk called in the case of conflict-by-arity in overload set
    // Parameters:  standard thunk parameters
    Var ArityGroupConflictThunk(Var method, Js::CallInfo callInfo, ...)
    {
        Js::JavascriptWinRTFunction* func = Js::JavascriptWinRTFunction::FromVar(method);
        Assert(Js::JavascriptOperators::GetTypeId(func) == Js::TypeIds_Function);
        Assert(func->IsWinRTFunction());
        ARGUMENTS(args, callInfo);
        Js::JavascriptWinRTFunction * function = Js::JavascriptWinRTFunction::FromVar(method);
        Js::ScriptContext* scriptContext = function->GetScriptContext();
        // ThunkSignature<RtARITYGROUP> * signature = reinterpret_cast<ThunkSignature<RtARITYGROUP>*>(function->GetSignature());

        // Arity group with more than one overload of the same arity is an automatic conflict.
        // The exception goes here in the thunk so that it occurs at call time.
        Js::JavascriptError::ThrowTypeError(scriptContext, VBSERR_ActionNotSupported);
    }

    // Info:        Attempt to construct an unconstructable runtime class
    // Parameters:  standard thunk parameters
    Var UnconstructableClassThunk(Var method, Js::CallInfo callInfo, ...)
    {
        Js::JavascriptWinRTFunction* func = Js::JavascriptWinRTFunction::FromVar(method);
        Assert(Js::JavascriptOperators::GetTypeId(func) == Js::TypeIds_Function);
        Assert(func->IsWinRTFunction());
        ARGUMENTS(args, callInfo);
        Js::JavascriptWinRTFunction * function = Js::JavascriptWinRTFunction::FromVar(method);
        Js::ScriptContext* scriptContext = function->GetScriptContext();
        auto typeName = (LPCWSTR)function->GetSignature();
        // Attempt to construct an unconstructable runtime class
        Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_UnconstructableClass, typeName);
    }

    // Info:        Thunk which dispatches to an overload by arity
    // Parameters:  standard thunk parameters
    Var OverloadMethodThunk(Var method, Js::CallInfo callInfo, ...)
    {
        Js::JavascriptWinRTFunction* func = Js::JavascriptWinRTFunction::FromVar(method);
        Assert(Js::JavascriptOperators::GetTypeId(func) == Js::TypeIds_Function);
        Assert(func->IsWinRTFunction());
        ARGUMENTS(args, callInfo);
        Js::JavascriptWinRTFunction * function = Js::JavascriptWinRTFunction::FromVar(method);
        Js::ScriptContext* scriptContext = function->GetScriptContext();
        Var result = NULL;
        OverloadSignature* signature = (OverloadSignature*)function->GetSignature();
        JsUtil::List<FunctionOverload*>* constructors = signature->overloads;
        size_t arity = args.Info.Count - 1; // Minus one for 'this'
        Js::JavascriptWinRTFunction* arityMatch = nullptr;
        size_t matchArgCount = 0;

        for (int i = 0; i < constructors->Count(); i++)
        {
            FunctionOverload* overload = constructors->Item(i);
            Js::JavascriptWinRTFunction* constructorFunc = overload->function;
            Assert(constructorFunc->IsWinRTFunction());
            Js::JavascriptWinRTFunction * constructorFunction = Js::JavascriptWinRTFunction::FromVar(constructorFunc);

            size_t argumentCount = overload->arity;

            if (size_t(argumentCount) <= arity)
            {
                if (arityMatch == nullptr)
                {
                    arityMatch = constructorFunction;
                    matchArgCount = argumentCount;
                }
                else if (matchArgCount < argumentCount)
                {
                    arityMatch = constructorFunction;
                    matchArgCount = argumentCount;
                }
            }
        }

        if (arityMatch)
        {
            result = arityMatch->CallFunction(args);
        }
        else
        {
            // No arity matched.
            Js::JavascriptError::ThrowError(scriptContext, JSERR_WinRTFunction_TooFewArguments, StringOfId(signature->projectionContext->GetScriptContext(), signature->nameId));
        }
        return result;
    }

    // Info:        Helper to get PropertyId for a string
    // Parameters:  propertyName - the property name
    PropertyId ProjectionWriter::IdOfString(LPCWSTR propertyName)
    {
        return Projection::IdOfString(projectionContext->GetScriptContext(), propertyName);
    }
    // Info:        Helper to get the string for a PropertyId
    // Parameters:  id - the property id
    LPCWSTR ProjectionWriter::StringOfId(PropertyId id)
    {
        return Projection::StringOfId(projectionContext->GetScriptContext(), id);
    }

    // Info:        Build a direct function
    // Parameters:  signature - signature as Var
    //              entryPoint - jump to
    //              nameId - name of the function
    Js::JavascriptWinRTFunction * ProjectionWriter::BuildDirectFunction(Var signature, void * entryPoint, PropertyId nameId, bool fConstructor)
    {
        auto result = projectionContext->CreateWinRTFunction(reinterpret_cast<Js::JavascriptMethod>(entryPoint), nameId, signature, fConstructor);
        return result;
    }

    // Info:        Helper function for setting property on an object
    // Parameters:  object - object to set a property on
    //              id - the id of the propert
    //              value - value to set
    void ProjectionWriter::SetProperty(Js::DynamicObject * object, PropertyId id, Var value)
    {
        if (id == lengthId && Js::JavascriptWinRTFunction::Is(object))
        {
            auto wasSet = object->SetPropertyWithAttributes(id, value, PropertyNone, NULL, Js::PropertyOperation_None, Js::SideEffects_None);
            if (!wasSet)
            {
                Js::VerifyCatastrophic(wasSet);
            }
            return;
        }

        Js::ScriptContext * scriptContext = projectionContext->GetScriptContext();
        BOOL wasSet = Js::JavascriptOperators::SetProperty(object, object, id, value, scriptContext);
        if (!wasSet)
        {
            Js::VerifyCatastrophic(wasSet);
        }

        // Set whether or not the projection prototype is configurable based on the isConfigurable flag set on IActiveScriptProjection::SetProjectionHost
        object->SetWritable(id, projectionContext->AreProjectionPrototypesConfigurable());
        object->SetConfigurable(id, projectionContext->AreProjectionPrototypesConfigurable());
    }

    // Info:        Create an unconstructable runtime class error function
    // Parameters:  typeName - name of the runtime class
    Js::JavascriptWinRTFunction * ProjectionWriter::UnconstructableClassThunkOfTypename(LPCWSTR typeName, bool fConstructor)
    {
        PropertyId nameId = IdOfString(typeName);
        return BuildDirectFunction((Var)typeName, UnconstructableClassThunk, nameId, fConstructor);
    }

    Js::JavascriptWinRTFunction * ProjectionWriter::ContinueFunctionOfSignature(RtMETHODSIGNATURE signature, ThisInfo * thisInfo, bool fConstructor, bool boundsToUndefined)
    {
        Js::JavascriptWinRTFunction * function = nullptr;
        switch(signature->signatureType)
        {
        case mstAbiMethodSignature:
            {
                RtABIMETHODSIGNATURE method = AbiMethodSignature::From(signature);
                Assert(method->GetParameters()->sizeOfCallstack!=0xffffffff);
                PropertyId nameId = signature->nameId;
                Signature * thunkSignature = RecyclerNew(recycler, Signature, projectionContext, thisInfo, method, boundsToUndefined);
                AssertMsg(CanResolveParamaters(signature, thisInfo->thisType == thisDelegate), "We should have determined if the parameters are resolveable by now.");
                function = TryGetProjectionFastPath(nameId, method, thunkSignature, thisInfo, projectionContext, fConstructor);
                if (function==nullptr)
                {
                    function = BuildDirectFunction((Var)thunkSignature, MethodSignatureThunk, nameId, fConstructor);
                }
                break;
            }
        case mstOverloadedMethodSignature:
            {
                RtOVERLOADEDMETHODSIGNATURE overloadMethodSignature = OverloadedMethodSignature::From(signature);
                PropertyId nameId = signature->nameId;
                JsUtil::List<FunctionOverload*> * constructors = RecyclerNew(recycler, JsUtil::List<FunctionOverload*>, recycler, JsUtil::List<FunctionOverload *>::DefaultIncrement);
                overloadMethodSignature->overloads->overloads->Iterate([&](RtABIMETHODSIGNATURE arityGroup) {
                    Js::JavascriptWinRTFunction * thunk = FunctionOfSignature(arityGroup, nullptr, thisInfo, false, boundsToUndefined);
                    if (thunk != nullptr)
                    {
                        FunctionOverload * overload = RecyclerNew(recycler, FunctionOverload, arityGroup->inParameterCount, thunk);
                        constructors->Add(overload);
                    }
                });
                AssertMsg(constructors->Count() > 0, "We should have checked that the overloads have at least one valid signature.");
                function = BuildDirectFunction((Var)RecyclerNew(recycler, OverloadSignature, projectionContext, nameId, constructors), OverloadMethodThunk, nameId, fConstructor);
                break;
            }
        default:
            Js::Throw::FatalProjectionError();
        }
        return function;
    }

    bool ProjectionWriter::CanResolveParamaters(RtMETHODSIGNATURE signature, bool isDelegate)
    {
        if (!projectionContext->GetScriptContext()->GetConfig()->IsWinRTAdaptiveAppsEnabled())
        {
            return true;
        }
        if (!projectionContext->GetScriptContext()->GetConfig()->AreWinRTDelegatesInterfaces())
        {
            isDelegate = false;
        }
#if DBG
        AllowHeavyOperation allowHeavy;
        LPCWSTR methodName = StringOfId(signature->nameId);
        if (wcscmp(_u("maxVersionInterfaceVectorIn"), methodName) == 0)
        {
            Assert(methodName != nullptr);
        }
#endif 
        auto parameters = signature->GetParameters();

        bool canCreateFunction = true;
        
        auto canTypeBeResolved = [&](RtTYPE type){
            if (Type::IsMissing(type))
            {
                auto missingType = MissingNamedType::Is(type)
                    ? MissingNamedType::From(type)
                    : MissingGenericInstantiationType::From(type)->parent;

                if (MissingNamedType::From(missingType)->isWebHidden)
                {
                    return true;
                }
                //Delegates are unique in their resolvability, and if the configFlag above is enabled, then we may succesfully resolve a delegate even if one or more of its parameters is unresolved
                // Excluding structs and enums, however, because we can't determine their stack size
                if (isDelegate && !MissingNamedType::From(missingType)->isStruct)
                {
                    return true;
                }

                canCreateFunction = false;
                return false;
            }

            if (isDelegate || !DelegateType::Is(type) && !InterfaceType::Is(type) && !ClassType::Is(type))
            {
                return true;
            }

            auto typeDefType = TypeDefinitionType::From(type);
            RtEXPR expr = nullptr;

            if (SUCCEEDED(projectionContext->GetExpr(typeDefType->typeId, typeDefType->typeDef->id, nullptr, typeDefType->genericParameters, &expr)))
            {
                if (RuntimeClassConstructor::Is(expr))
                {
                    auto rtc = RuntimeClassConstructor::From(expr);
                    if (rtc->defaultInterface.HasValue())
                    {
                        auto defaultInterface = rtc->defaultInterface.GetValue();
                        if (MissingInterfaceConstructor::Is(defaultInterface))
                        {
                            canCreateFunction = MissingInterfaceConstructor::From(defaultInterface)->isWebHidden;
                            return canCreateFunction;
                        }
                        canCreateFunction = (defaultInterface->typeDef != nullptr); //It's null if type is missing
                        return canCreateFunction;
                    }
                }
                else if (RuntimeInterfaceConstructor::Is(expr))
                {
                    auto ric = RuntimeInterfaceConstructor::From(expr);
                    canCreateFunction = (ric->typeDef != nullptr); //It's null if type is missing
                    return canCreateFunction;
                }
                else if (DelegateConstructor::Is(expr))
                {
                    auto delConstructor = DelegateConstructor::From(expr);
                    RtPROPERTY invokeProperty = delConstructor->invokeInterface->prototype->fields->First();
                    RtABIMETHODPROPERTY invokeAbiProperty = AbiMethodProperty::From(invokeProperty);
                    canCreateFunction = CanResolveParamaters(invokeAbiProperty->body->signature, true);
                    return canCreateFunction;
                }
                else if (MissingInterfaceConstructor::Is(expr))
                {
                    canCreateFunction = MissingInterfaceConstructor::From(expr)->isWebHidden;
                    return canCreateFunction;
                }
                else if (MissingTypeConstructor::Is(expr))
                {
                    canCreateFunction = MissingTypeConstructor::From(expr)->isWebHidden;
                    return canCreateFunction;
                }
            }
            canCreateFunction = false;
            return false;
        };

        auto checkParameters = [&](RtPARAMETER parameter){
            RtTYPE type = ByRefType::Is(parameter->type) ? ByRefType::From(parameter->type)->pointedTo : parameter->type;
            return canTypeBeResolved(type);
        };

        

        parameters->allParameters->IterateWhile(checkParameters);

        return canCreateFunction;
    }

    // Info:        Convert a method signature into a Javascript function
    // Parameters:  signature - signature
    //              properties - properties on the function
    //              thisInfo - describes what kind of 'this'
    Js::JavascriptWinRTFunction * ProjectionWriter::FunctionOfSignature(RtMETHODSIGNATURE signature, RtPROPERTIESOBJECT properties, ThisInfo * thisInfo, bool fConstructor, bool boundsToUndefined)
    {
        Js::JavascriptWinRTFunction * function = nullptr;

        switch(signature->signatureType)
        {
        case mstOverloadedMethodSignature:
            {
                bool atLeastOneAvailable = false;
                OverloadedMethodSignature::From(signature)->overloads->overloads->IterateWhile([&](RtABIMETHODSIGNATURE arityGroup) {
                    if (CanResolveParamaters(arityGroup, thisInfo->thisType == thisDelegate))
                    {
                        atLeastOneAvailable = true;
                        return false;
                    }
                    return true;
                });

                if (!atLeastOneAvailable)
                {
                    return nullptr;
                }
            } //fall through
        case mstAbiMethodSignature:
            {
                auto nameId = signature->nameId;
                
#if DBG
                LPCWSTR name = projectionContext->StringOfId(nameId);
                Assert(name);
#endif

                if (!CanResolveParamaters(signature, thisInfo->thisType == thisDelegate))
                {
                    return nullptr;
                }

                auto continuation = RecyclerNew(recycler, FunctionOfSignatureContinuation, this, signature, thisInfo, boundsToUndefined);

                function = BuildDirectFunction((Var)continuation, DelayedFunctionOfSignatureThunk, nameId, fConstructor);
                break;
            }
        case mstUncallableMethodSignature:
        case mstTypeConstructorMethodSignature:
        case mstMissingTypeConstructorMethodSignature:
            {

                switch(thisInfo->thisType)
                {
                case thisRuntimeClass:
                    {
                        function = UnconstructableClassThunkOfTypename(StringOfId(thisInfo->typeId), fConstructor);
                        break;
                    }
                default:
                    function = UnconstructableClassThunkOfTypename(StringOfId(signature->GetParameters()->returnType->fullTypeNameId), fConstructor);
                    break;
                }
                break;
            }
        default:
            Js::Throw::FatalProjectionError();
        }

        if (properties)
        {
            ApplyPropertiesObjectToJsObject(function, properties, thisInfo);
        }
        return function;

    }

    // Info:        Convert a model specialization into a set of Javascript functions in support of that specialization
    // Parameters:  specialization - model specialization
    //              properties - properties on the function
    //              thisInfo - describes what kind of 'this'
    SpecialProjection * ProjectionWriter::SpecializationToSpecialProjection(ThisInfo * thisInfo, Js::DynamicObject *prototypeObject)
    {
#if DBG
        switch(thisInfo->specialization->specializationType)
        {
        case specVectorSpecialization:
        case specVectorViewSpecialization:
            Assert(thisInfo->IsArray());
            // Let it fall through to create the projection
        case specPromiseSpecialization:
        case specPropertyValueSpecialization:
        case specMapSpecialization:
        case specMapViewSpecialization:
            break;

        default:
            Assert(false);
        }
#endif
        VectorProjectionFunctions * vectorFunctions = nullptr;
        if (thisInfo->specialization->specializationType == specVectorSpecialization ||
            thisInfo->specialization->specializationType == specVectorViewSpecialization)
        {
            vectorFunctions = RecyclerNew(recycler, VectorProjectionFunctions);
        }
        return RecyclerNew(recycler, SpecialProjection, thisInfo, prototypeObject, projectionContext, vectorFunctions);
    }

    bool ProjectionWriter::GetRuntimeClassTypeInformation(PropertyId typeNameId, HTYPE * htype, RuntimeClassTypeInformation ** typeInformation)
    {
        if (recyclerData->runtimeClassTypeInformation->TryGetValue(typeNameId, typeInformation))
        {
            *htype = (*typeInformation)->GetHTYPE();
            return true;
        }
        return false;
    }

    // Info:        Make sure the given runtime class exists in the type caches (if possible)
    // Parameters:  type - the model type
    //              htype - receives the htype
    //              typeInformation - receives the type information
    bool ProjectionWriter::TryEnsureRuntimeClassTypeExists(RtTYPEDEFINITIONTYPE type, HTYPE * htype, RuntimeClassTypeInformation ** typeInformation)
    {
        Assert(ClassType::Is(type) || InterfaceType::Is(type));
        RtEXPR expr = nullptr;

        if(SUCCEEDED(projectionContext->GetExpr(type->typeId, type->typeDef->id, nullptr, type->genericParameters, &expr)))
        {
            if (RuntimeClassConstructor::Is(expr))
            {
                auto rtc = RuntimeClassConstructor::From(expr);
                if (rtc->defaultInterface.HasValue())
                {
                    auto defaultInterface = rtc->defaultInterface.GetValue();
                    if (RuntimeInterfaceConstructor::Is(defaultInterface))
                    {
                        auto ric = RuntimeInterfaceConstructor::From(defaultInterface);
                        GetOrCreateRuntimeClassTypeInformation(type->typeId, StringOfId(type->typeId), ric->iid,
                            rtc->specialization, rtc->prototype, rtc->properties, rtc->signature, rtc->hasEventHandlers, rtc->gcPressure, htype, typeInformation);
                        return true;
                    }
                }
            }
            else if (RuntimeInterfaceConstructor::Is(expr))
            {
                auto ric = RuntimeInterfaceConstructor::From(expr);
                GetOrCreateRuntimeClassTypeInformation(type->typeId, StringOfId(type->typeId), ric->iid,
                            ric->specialization, ric->prototype, ric->properties, ric->signature, ric->hasEventHandlers, DefaultGCPressure, htype, typeInformation);
                return true;
            }
        }
        return false;
    }

    // Info:        Add known methods to a projection object prototype.
    void ProjectionWriter::AddKnownPrototypeMethods(__in ProjectionContext *projectionContext, __in Var prototype, __in RuntimeClassTypeInformation *pTypeInformation)
    {
#ifndef DISABLE_ISTRINGABLE_QI
        if (!pTypeInformation->GetThisInfo()->IsArray())
        {
            auto config = projectionContext->GetScriptContext()->GetConfig()->GetProjectionConfig();
            if (config && config->IsTargetWindowsBlueOrLater())
            {
                AddToString(projectionContext, prototype);
            }
        }
#endif // !defined(DISABLE_ISTRINGABLE_QI)
    }

#ifndef DISABLE_ISTRINGABLE_QI
    // Info:        For WinRT projected objects which are NOT explicitly projected as an array, add a toString method which will
    //              perform the appropriate QI for IStringable.
    void ProjectionWriter::AddToString(__in ProjectionContext *projectionContext, __in Var prototype)
    {
        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();        

        auto toStringId = projectionContext->toStringId;
        Js::DynamicObject* prototypeObject = Js::DynamicObject::FromVar(prototype);

        // If prototype object already includes the toString function (from metadata perhaps) do not add it again.
        if (prototypeObject->HasOwnProperty(toStringId))
        {
            return;
        }

        auto toStringMethod = projectionContext->CreateWinRTFunction(reinterpret_cast<Js::JavascriptMethod>(ProjectionObjectInstance::ToStringThunk), toStringId, nullptr, false);
        Var varLength = Js::JavascriptNumber::ToVar(0, scriptContext);

        SetProperty(toStringMethod, Js::PropertyIds::length, varLength);
        SetProperty(prototypeObject, toStringId, toStringMethod);
    }
#endif // !defined(DISABLE_ISTRINGABLE_QI)

    // Info:        Get or create type information based on individual information
    // Parameters:  typeName - name of the type
    //              specialization - specialization information
    //              prototype - prototype expression
    //              properties - properties on the object itself
    //              signature - optional signature for the construct (empty if not constructable)
    //              hasEventHandlers - true if there are event handlers
    //              htype - receives the htype
    //              typeInformation - auxilary type information
    void ProjectionWriter::GetOrCreateRuntimeClassTypeInformation(MetadataStringId typeNameId, LPCWSTR typeName, RtIID defaultInterface, RtSPECIALIZATION specialization, RtEXPR prototype, RtPROPERTIESOBJECT properties, Option<MethodSignature> signature, bool hasEventHandlers, INT32 gcPressure,
        HTYPE * htype, RuntimeClassTypeInformation ** ppTypeInformation)
    {
        if(GetRuntimeClassTypeInformation(typeNameId, htype, ppTypeInformation))
        {
            return;
        }

        // Create new type info
        auto scriptContext = projectionContext->GetScriptContext();
        auto typeId = scriptContext->CreateTypeId();
        auto typeInformation = RecyclerNew(recycler, RuntimeClassTypeInformation);
        typeInformation->SetHasEventHandlers(hasEventHandlers);
        typeInformation->SetGCPressure(gcPressure);
        *ppTypeInformation = typeInformation;

        // Figure out ThisInfo
        RuntimeClassThis * runtimeClassThisInfo = nullptr;
        Js::DynamicObject * prototypeObject = nullptr;
        InspectableObjectTypeOperations * typeOperations = nullptr;

        if (specialization || defaultInterface)
        {
            if (hasEventHandlers)
            {
                typeInformation->SetThisInfo(RecyclerNew(recycler, UnknownEventHandlingThis, typeNameId, defaultInterface, specialization));
            }
            else
            {
                typeInformation->SetThisInfo(RecyclerNew(recycler, UnknownThis, typeNameId, defaultInterface, specialization));
            }
        }
        else
        {
            typeInformation->SetThisInfo(hasEventHandlers ? recyclerData->unknownEventHandlingThis : recyclerData->unknownThis);
        }

        // Compute the prototype
        Var prototypeVar;
        if (prototype)
        {
            prototypeVar = ExprToJsVar(prototype, typeInformation->GetThisInfo());
            if (prototypeVar == nullptr)
            {
                AssertMsg(false, "prototypeVar should not be null");
                Js::Throw::FatalProjectionError();
            }
        }
        else
        {
            auto hr = VectorArray::CreateProtypeObject(typeInformation->GetThisInfo()->IsArray(), projectionContext, &prototypeVar);
            IfFailedMapAndThrowHr(scriptContext, hr);
        }

        // Add any well-known prototype methods to this prototype Var
        AddKnownPrototypeMethods(projectionContext, prototypeVar, typeInformation);

        typeInformation->SetPrototypeVar(prototypeVar);

        // Set constructor and prototype relationship
        prototypeObject = Js::DynamicObject::FromVar(typeInformation->GetPrototypeVar());

        if (specialization)
        {
            typeInformation->SetSpecialProjection(SpecializationToSpecialProjection(typeInformation->GetThisInfo(), prototypeObject));
        }

#if DBG
        runtimeClassThisInfo = RecyclerNewFinalized(recycler, RuntimeClassThis, typeName, typeNameId);
#else
        runtimeClassThisInfo = RecyclerNewFinalized(recycler, RuntimeClassThis, typeNameId);
#endif

        typeInformation->SetRuntimeClassThisInfo(runtimeClassThisInfo);

        bool canProjectConstructor =
            signature.HasValue() &&
            projectionContext->IsWinRTConstructorAllowed();

        if (canProjectConstructor)
        {
            typeInformation->SetConstructorFunction(FunctionOfSignature(signature.GetValue(), properties, runtimeClassThisInfo, true));
        }
        
        if (typeInformation->GetConstructorFunction() == nullptr)
        {
            typeInformation->SetConstructorFunction(UnconstructableClassThunkOfTypename(typeName, true));
            if (properties)
            {
                ApplyPropertiesObjectToJsObject(typeInformation->GetConstructorFunction(), properties, runtimeClassThisInfo);
            }
        }
        Js::JavascriptWinRTConstructorFunction::FromVar(typeInformation->GetConstructorFunction())->SetTypeInformation(typeInformation);
        JS_ETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_WINRT_RUNTIMECLASS_OBJECT(typeInformation->GetConstructorFunction(), typeName));

        // Constructor of prototype
        Js::JavascriptOperators::InitProperty(prototypeObject, Js::PropertyIds::constructor, typeInformation->GetConstructorFunction());
        prototypeObject->SetEnumerable(Js::PropertyIds::constructor, false);

        if (projectionContext->AreProjectionPrototypesConfigurable())
        {
            // if design/unit test mode - allow projections to be configurable/writable
#if DBG
            auto wasSet = 
#endif
            typeInformation->GetConstructorFunction()->SetPropertyWithAttributes(Js::PropertyIds::prototype, prototypeObject, PropertyBuiltInMethodDefaults, NULL);
#if DBG
            Assert(wasSet);
#endif
        }
        else
        {
            // otherwise - lock them down

            // Prototype of constructor
            typeInformation->GetConstructorFunction()->SetPropertyWithAttributes(Js::PropertyIds::prototype, prototypeObject, PropertyNone, NULL);

            BOOL succeeded = prototypeObject->PreventExtensions();
            Js::VerifyCatastrophic(succeeded);

            // Prevent Extensions on constructor
            succeeded = typeInformation->GetConstructorFunction()->PreventExtensions();
            Js::VerifyCatastrophic(succeeded);
        }

        // Make the type operations
        HRESULT hr = InspectableObjectTypeOperations::Create(projectionContext, typeInformation->GetSpecialProjection(), &typeOperations);
        Assert(SUCCEEDED(hr));

        ScriptEngine *scriptEngine = projectionContext->GetScriptEngine();
        IfNullMapAndThrowHr(scriptContext, scriptEngine, E_ACCESSDENIED);

        // Since right now we want the created type to last till scriptengine's context, we want to pass bindReference parameter as true
        hr = scriptEngine->CreateTypeFromScript((::TypeId)typeId, typeInformation->GetPrototypeVar(), NULL, typeOperations, FALSE, typeNameId, TRUE, htype);
        IfFailedMapAndThrowHr(scriptContext, hr);

        // Set the type infos
        typeInformation->SetHTYPE(*htype);
        recyclerData->runtimeClassTypeInformation->Add(typeNameId, typeInformation);
    }

    EventProjectionHandler *ProjectionWriter::GetEventHandlerFromWeakReference(Js::PropertyId typeId, IWeakReference *weakReference)
    {
        EventProjectionHandler *eventProjectionHandler = GetExistingEventHandlerFromWeakReference(weakReference);
        HRESULT hr = S_OK;
        if (eventProjectionHandler == nullptr)
        {
            Js::ScriptContext* scriptContext = projectionContext->GetScriptContext();
            BEGIN_LEAVE_SCRIPT(scriptContext)
            {
                BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT
                {
                    // Create a new one.
                    eventProjectionHandler = RecyclerNew(scriptContext->GetRecycler(), EventProjectionHandler, typeId);
                    recyclerData->m_eventHandlerCache->Add(weakReference, eventProjectionHandler);
                    weakReference->AddRef();
                }
                END_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)
            }
            END_LEAVE_SCRIPT(scriptContext)
            IfFailedMapAndThrowHr(scriptContext, hr);
        }

        return eventProjectionHandler;
    }

    EventProjectionHandler *ProjectionWriter::GetExistingEventHandlerFromWeakReference(IWeakReference *weakReference)
    {
        EventProjectionHandler *eventProjectionHandler = nullptr;
        if (recyclerData->m_eventHandlerCache->TryGetValue(weakReference, &eventProjectionHandler))
        {
            return eventProjectionHandler;
        }
        return nullptr;
    }

    void ProjectionWriter::RemoveEventHandlerFromWeakReference(IWeakReference *weakReference)
    {
        recyclerData->m_eventHandlerCache->Remove(weakReference);
        weakReference->Release();
    }

    void ProjectionWriter::RemoveInspectableFromCache(PROJECTIONINSTANCEKEYVALUEPAIR &keyValuePair)
    {
        recyclerData->m_InspectablesCache->Remove(keyValuePair);
    }

    // Try to see if the unknown is already in the cache
    bool ProjectionWriter::TryGetTypedInstanceFromCache(IUnknown *unknown, Var *result, bool allowExtensions)
    {
        RecyclerWeakReference<Js::DynamicObject> *weakRef = nullptr;
        bool fFound = recyclerData->m_InspectablesCache->TryGetValue(unknown, &weakRef);
        // The object might be marked for disposing and hence weakRef might not be alive.
        if (fFound && weakRef->Get() != nullptr)
        {
            *result = weakRef->Get();
            return true;
        }

        return false;
    }

    // Returns an object containing source information for the async operation.
    Var ProjectionWriter::GetAsyncOperationSource()
    {
        Js::ScriptContext* scriptContext = projectionContext->GetScriptContext();

        // Create object for storing Async Operation source information
        Js::DynamicObject * obj = scriptContext->GetLibrary()->CreateObject();

        Js::JavascriptExceptionContext exceptionContext;
        Js::JavascriptExceptionOperators::WalkStackForExceptionContext(*scriptContext, exceptionContext, nullptr, Js::JavascriptExceptionOperators::DefaultStackTraceLimit, NULL, false);
        Js::JavascriptExceptionOperators::AddStackTraceToObject(obj, exceptionContext.GetStackTrace(), *scriptContext, false);

        return obj;
    }

    Var ProjectionWriter::CreateProjectionObjectTypedInstance(
        __in PropertyId typeNameId,
        __in HTYPE htype,
        __in RuntimeClassTypeInformation * typeInformation,
        __in IUnknown* unknown,
        __in bool allowIdentity,
        __in bool allowExtensions,
        __in_opt ConstructorArguments* constructorArguments)
    {
        Assert(unknown != nullptr);

        Js::ScriptContext* scriptContext = projectionContext->GetScriptContext();
        auto specialProjection = typeInformation->GetSpecialProjection();
        auto thisInfo = typeInformation->GetThisInfo();

        if (specialProjection && specialProjection->thisInfo->specialization->specializationType == specPropertyValueSpecialization)
        {
            RtPROPERTYVALUESPECIALIZATION propertyValueSpecialization = PropertyValueSpecialization::From(specialProjection->thisInfo->specialization);

            // IReference<T> or IReferenceArray<T> : call get_Value()
            if (propertyValueSpecialization->getValue)
            {
#ifdef ENABLE_JS_ETW
                if (EventEnabledJSCRIPT_PROJECTION_REFERENCEORARRAYGETVALUE_START())
                {
                    EventWriteJSCRIPT_PROJECTION_REFERENCEORARRAYGETVALUE_START(StringOfId(propertyValueSpecialization->getValue->runtimeClassNameId));
                }
#endif
                Js::CallInfo callInfo(Js::CallFlags_Value, 1);
                Var undefinedVar = {projectionContext->GetScriptContext()->GetLibrary()->GetUndefined()};
                Js::Arguments args(callInfo, &undefinedVar);

                ProjectionMethodInvoker invoker(propertyValueSpecialization->getValue, projectionContext);
                VerifyDeprecatedAttributeOnce(propertyValueSpecialization->getValue, scriptContext, DeprecatedInvocation_Class);
                IUnknown *unknownReference = nullptr;
                HRESULT hr = QueryInterfaceAfterLeaveScript(scriptContext, unknown, propertyValueSpecialization->getValue->iid->instantiated, (void**)&unknownReference);
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);

                // InvokeUnknown takes ownership of this unknown and would release it.
                hr = invoker.InvokeUnknown(unknownReference, propertyValueSpecialization->getValue->vtableIndex+6, args);
                Var result = invoker.ReadOutOrThrow(hr, false, args);

#ifdef ENABLE_JS_ETW
                if (EventEnabledJSCRIPT_PROJECTION_REFERENCEORARRAYGETVALUE_STOP())
                {
                    EventWriteJSCRIPT_PROJECTION_REFERENCEORARRAYGETVALUE_STOP(StringOfId(propertyValueSpecialization->getValue->runtimeClassNameId));
                }
#endif
                return result;
            }
            else
            {
                // PropertyValue only
                HRESULT hr = S_OK;
                HSTRING propertyValueGRCN = nullptr;
                Windows::Foundation::IPropertyValue *propertyValue = nullptr;

                // This is a QI so we do not want to mark the callout for debug-stepping (via MarkerForExternalDebugStep).
                Js::JavascriptErrorDebug::ClearErrorInfo(scriptContext);
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = unknown->QueryInterface(Windows::Foundation::IID_IPropertyValue, (void**)&propertyValue);

                    if (SUCCEEDED(hr))
                    {
                        hr = propertyValue->GetRuntimeClassName(&propertyValueGRCN);
                        if (FAILED(hr))
                        {
                            propertyValue->Release();
                        }
                    }
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);

                AutoHSTRING runtimeClassHString(projectionContext->GetThreadContext()->GetWinRTStringLibrary());
                runtimeClassHString.Initialize(propertyValueGRCN);

                ProjectionMarshaler marshal(CalleeTransfersOwnership, projectionContext, false);
                marshal.RecordToUndo(propertyValue, true);
                return marshal.ReadPropertyValueVarFromRuntimeClassName(runtimeClassHString, unknown, propertyValue, allowIdentity, allowExtensions);
            }
        }

        ProjectionObjectInstance * instanceObject = nullptr;
        HRESULT hr = ProjectionObjectInstance::Create(htype, typeInformation->HasEventHandlers(), unknown, thisInfo->defaultInterface ? thisInfo->defaultInterface->instantiated : GUID_NULL, projectionContext, &instanceObject, allowIdentity, typeInformation->GCPressure());
        IfFailedMapAndThrowHr(scriptContext, hr);
        Js::DynamicObject *resultObject = instanceObject;
        JS_ETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_WINRT_JSPROXY_OBJECT(instanceObject, StringOfId(typeNameId)));

        // Apply specialization if any.
        if (specialProjection)
        {
            switch(specialProjection->thisInfo->specialization->specializationType)
            {
            case specVectorSpecialization:
                {
                    Assert(thisInfo->IsArray());
                    RtVECTORSPECIALIZATION vector = VectorSpecialization::From(specialProjection->thisInfo->specialization);
                    PropertyId propertyId = vector->length->identifier;
                    ThunkSignature<SpecialProjection*> * signature = RecyclerNew(recycler, ThunkSignature<SpecialProjection*>, projectionContext, specialProjection);
                    Js::JavascriptWinRTFunction * getter = FunctionOfSignature(vector->length->getSize, nullptr, thisInfo, false);
                    Assert(getter != nullptr);
                    Js::JavascriptWinRTFunction * setter = BuildDirectFunction(signature, ArraySetLengthThunk, propertyId, false);

                    Js::VerifyCatastrophic(instanceObject->SetAccessors(propertyId, getter, setter));
                    instanceObject->SetConfigurable(propertyId, false);
                    instanceObject->SetEnumerable(propertyId, false);
                    break;
                }
            case specVectorViewSpecialization:
                {
                    Assert(thisInfo->IsArray());
                    RtVECTORVIEWSPECIALIZATION vectorView = VectorViewSpecialization::From(specialProjection->thisInfo->specialization);
                    PropertyId propertyId = vectorView->length->identifier;
                    ThunkSignature<SpecialProjection*> * signature = RecyclerNew(recycler, ThunkSignature<SpecialProjection*>, projectionContext, specialProjection);
                    Js::JavascriptWinRTFunction * getter = FunctionOfSignature(vectorView->length->getSize, nullptr, thisInfo, false);
                    Assert(getter != nullptr);
                    Js::JavascriptWinRTFunction * setter = BuildDirectFunction(signature, ArraySetLengthThunk, propertyId, false);

                    Js::VerifyCatastrophic(instanceObject->SetAccessors(propertyId, getter, setter));
                    instanceObject->SetConfigurable(propertyId, false);
                    instanceObject->SetEnumerable(propertyId, false);
                    break;
                }
            case specPromiseSpecialization:
                {
                    auto promiseMaker = GetPromiseMaker();
                    LPCWSTR typeName = StringOfId(typeNameId);
                    Js::JavascriptString * asyncOpTypeVar = Js::JavascriptString::NewCopyBuffer(typeName, wcslen(typeName), scriptContext);
                    Var asyncOpSourceVar = scriptContext->IsScriptContextInDebugMode() ? GetAsyncOperationSource() : scriptContext->GetLibrary()->GetUndefined();

                    Var asyncOpCausalityIdVar;
                    if (AsyncDebug::IsAsyncDebuggingEnabled(scriptContext) && constructorArguments && constructorArguments->type == ConstructorArgumentType_Promise)
                    {
                        auto asyncOpId = ((PromiseConstructorArguments*) constructorArguments)->asyncOperationId;
                        asyncOpCausalityIdVar =  Js::JavascriptNumber::ToVar(asyncOpId, scriptContext);
                    }
                    else
                    {
                        asyncOpCausalityIdVar = Js::TaggedInt::ToVarUnchecked((int)AsyncDebug::InvalidAsyncOperationId);
                    }

                    Js::CallInfo callInfo(Js::CallFlags_New, 5);
                    Var args[5] = {scriptContext->GetLibrary()->GetUndefined(), instanceObject, asyncOpTypeVar, asyncOpSourceVar, asyncOpCausalityIdVar};
                    auto result = Js::JavascriptWinRTFunction::CallAsConstructor(promiseMaker, /* overridingNewTarget = */nullptr, Js::Arguments(callInfo, args), projectionContext->GetScriptContext());
                    resultObject = Js::DynamicObject::FromVar(result);
                    break;
                }
            case specMapSpecialization:
                {
                    // We dont want to preventExtensions because we can extend this object so return from here
                    allowExtensions = true;
                    break;
                }
            case specMapViewSpecialization:
                {
                    // Map view doesnt allow extending the objects
                    break;
                }
            case specPropertyValueSpecialization:
                {
                    Assert(false);
                    // Let it fall through to throw catastrophic
                }
            default:
                    Js::Throw::FatalProjectionError();
                }
        }

        // only prevent extensions for projected result objects when not in design/unit test mode
        if (!allowExtensions && !projectionContext->AreProjectionPrototypesConfigurable())
        {
            BOOL succeeded = instanceObject->PreventExtensions();
            Js::VerifyCatastrophic(succeeded);
        }

        // Add it to the cache.
        if (allowIdentity)
        {
#if DBG
            RecyclerWeakReference<Js::DynamicObject> *tempWeakRef;
            Assert(!recyclerData->m_InspectablesCache->TryGetValue(unknown, &tempWeakRef) || tempWeakRef->Get() == nullptr);
#endif

            RecyclerWeakReference<Js::DynamicObject> *weakRef = recycler->CreateWeakReferenceHandle<Js::DynamicObject>(resultObject);
            instanceObject->SetWeakReference(weakRef);
            recyclerData->m_InspectablesCache->Item(unknown, weakRef);
        }

        return resultObject;
    }

    // Info:        Create new type instance for a runtime class or interface.
    // Parameters:  typeName - name of the type
    //              specialization - any specialization on this type
    //              prototype - expression describing the prototype for this type
    //              hasEventHandlers - true if there are event handlers on the type
    // Returns:     The resulting instance
    Var ProjectionWriter::CreateNewTypeInstance(
        __in MetadataStringId typeNameId, 
        __in_z LPCWSTR typeName, 
        __in RtIID defaultInterface, 
        __in RtSPECIALIZATION specialization, 
        __in RtEXPR prototype, 
        __in RtPROPERTIESOBJECT properties, 
        __in regex::Option<ProjectionModel::MethodSignature> signature, 
        __in bool hasEventHandlers, 
        __in INT32 gcPressure, 
        __in IUnknown *unknown, 
        __in bool allowIdentity, 
        __in bool allowExtensions,
        __in_opt ConstructorArguments* constructorArguments)
    {
        HTYPE htype = nullptr;
        RuntimeClassTypeInformation * typeInformation = nullptr;

        GetOrCreateRuntimeClassTypeInformation(typeNameId, typeName, defaultInterface, specialization, prototype, properties, signature, hasEventHandlers, gcPressure, &htype, &typeInformation);
        return CreateProjectionObjectTypedInstance(typeNameId, htype, typeInformation, unknown, allowIdentity, allowExtensions, constructorArguments);
    }

    // Info:        Build up a runtime class constructor.
    // Parameters:  typeName - name of the type
    //              specialization - any specialization on this type
    //              prototype - expression describing the prototype for this type
    //              properties - properties on the function
    //              signature - The constructor signature. Will be empty if the type is unconstructable
    //              hasEventHandlers - true if there are event handlers on the type
    // Returns:     The resulting function
    Js::JavascriptWinRTFunction * ProjectionWriter::RuntimeClassInfoToFunction(MetadataStringId typeNameId, LPCWSTR typeName, RtIID defaultInterface, RtSPECIALIZATION specialization, RtEXPR prototype, RtPROPERTIESOBJECT properties, Option<MethodSignature> signature, bool hasEventHandlers, INT32 gcPressure)
    {
        HTYPE htype = nullptr;
        RuntimeClassTypeInformation * typeInformation = nullptr;
        GetOrCreateRuntimeClassTypeInformation(typeNameId, typeName, defaultInterface, specialization, prototype, properties, signature, hasEventHandlers, gcPressure,
            &htype, &typeInformation);
        return typeInformation->GetConstructorFunction();
    }

    HRESULT ProjectionWriter::GetStructHType(RtSTRUCTTYPE structType, HTYPE *htype)
    {
        IfNullReturnError(htype, E_POINTER);

        PropertyId typeNameId = structType->typeDef->id;

        HRESULT hr = S_OK;
        if (!htypesStruct->TryGetValue(typeNameId, htype))
        {
            // Since right now we want the created type to last till scriptengine's context, we want to pass bindReference parameter as true
            ScriptEngine *scriptEngine = projectionContext->GetScriptEngine();
            IfNullReturnError(scriptEngine, E_ACCESSDENIED);

            hr = scriptEngine->CreateTypeFromScript((::TypeId)projectionContext->GetScriptContext()->CreateTypeId(), NULL, NULL, nullptr, FALSE, typeNameId, TRUE, htype);
            IfFailedReturn(hr);

            htypesStruct->Add(typeNameId, *htype);
        }

        return hr;
    }

    // Info:        Convert a model enum into a Javascript object
    // Parameters:  _enum - the model enum
    //              thisInfo - describes what kind of 'this'
    // Returns:     The resulting object
    Var ProjectionWriter::EnumToObject(RtENUM _enum, ThisInfo * thisInfo)
    {
        PropertyId typeNameId = _enum->typeDef->id;

        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
        Assert(!_enum->typeDef->assembly.IsAttributePresent(_enum->typeDef->td, _u("Windows.Foundation.Metadata.WebHostHiddenAttribute")));

        HRESULT hr = S_OK;
        Assert(scriptContext->GetThreadContext()->IsScriptActive());

        HTYPE htype = nullptr;
        if (!htypesEnum->TryGetValue(typeNameId, &htype))
        {
            // Since right now we want the created type to last till scriptengine's context, we want to pass bindReference parameter as true
            ScriptEngine *scriptEngine = projectionContext->GetScriptEngine();
            IfNullMapAndThrowHr(scriptContext, scriptEngine, E_ACCESSDENIED);

            hr = scriptEngine->CreateTypeFromScript((::TypeId)scriptContext->CreateTypeId(), NULL, NULL, nullptr, FALSE, typeNameId, TRUE, &htype);
            IfFailedMapAndThrowHr(scriptContext, hr);
            if (SUCCEEDED(hr))
            {
                htypesEnum->Add(typeNameId, htype);
            }
        }

        // We get script engine again because during the dictionary lookup/add,
        // we might run into recycler and engine might get closed
        ScriptEngine *scriptEngine = projectionContext->GetScriptEngine();
        IfNullMapAndThrowHr(scriptContext, scriptEngine, E_ACCESSDENIED);

        Var object = RecyclerNew(recycler, Js::ExternalObject, (Js::ExternalType *)htype);
        JS_ETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_WINRT_ENUM_OBJECT(object, StringOfId(typeNameId)));

        ApplyPropertiesObjectToJsObject(object, _enum->properties, thisInfo);

        Js::DynamicObject * dynamicObject = Js::DynamicObject::FromVar(object);
        if (!this->projectionContext->AreProjectionPrototypesConfigurable())
        {
            BOOL succeeded = dynamicObject->PreventExtensions();
            Js::VerifyCatastrophic(succeeded);
        }

        return object; 
    }

    // Info:        Convert a model function into a Javascript function
    // Parameters:  function - the model function
    //              thisInfo - describes what kind of 'this'
    // Returns:     The resulting function
    Js::JavascriptWinRTFunction * ProjectionWriter::FunctionToJsFunction(RtFUNCTION function, ThisInfo * thisInfo)
    {
        switch(function->functionType)
        {
        case functionRuntimeClassConstructor:
            {
                RtRUNTIMECLASSCONSTRUCTOR constructor = RuntimeClassConstructor::From(function);
                RtIID defaultInterfaceIID = nullptr;
                if (constructor->defaultInterface.HasValue())
                {
                    auto defaultInterface = constructor->defaultInterface.GetValue();
                    if (RuntimeInterfaceConstructor::Is(defaultInterface))
                    {
                        defaultInterfaceIID = RuntimeInterfaceConstructor::From(defaultInterface)->iid;
                    }
                }
                return RuntimeClassInfoToFunction(constructor->typeId, StringOfId(constructor->typeDef->id), defaultInterfaceIID, constructor->specialization, constructor->prototype, constructor->properties, function->signature, constructor->hasEventHandlers, constructor->gcPressure);
            }
        case functionAbiMethod:
        case functionOverloadGroupConstructor:
            return FunctionOfSignature(function->signature, function->properties, thisInfo, false);
        default:
            // Delegates, interfaces and structs are not namespace elements
            return nullptr;
        }
    }


    // Info:        Apply a single model property to the given object
    // Parameters:  prop - the model property
    //              dynamicObject - the object to apply the property to
    //              thisInfo - describes what kind of 'this'
    void ProjectionWriter::ApplyPropertyToJsObject(RtPROPERTY prop, Js::DynamicObject * dynamicObject, ThisInfo * thisInfo)
    {
        PropertyId propertyId = prop->identifier;

        ProjectionModel::ProjectionBuilder::TraceRtPROPERTY(prop, _u("ProjectionWriter::ApplyPropertyToJsObject"), (Metadata::IStringConverter*)this->projectionContext, this->projectionContext->ProjectionAllocator());

        switch(prop->propertyType)
        {
        case ptAbiAddEventListenerProperty:
            {
                Assert(thisInfo->CanHoldEventCookies());
                RtABIADDEVENTLISTENERPROPERTY adder = AbiAddEventListenerProperty::From(prop);
                EventsSignature * signature = RecyclerNew(recycler, EventsSignature, projectionContext, thisInfo, adder->events);
                Js::JavascriptWinRTFunction * function = BuildDirectFunction(signature, AddEventListenerThunk, propertyId, false);
                SetProperty(function, lengthId, two);
                SetProperty(dynamicObject, propertyId, function);
                return;
            }
        case ptAbiRemoveEventListenerProperty:
            {
                Assert(thisInfo->CanHoldEventCookies());
                RtABIREMOVEEVENTLISTENERPROPERTY remover = AbiRemoveEventListenerProperty::From(prop);
                EventsSignature * signature = RecyclerNew(recycler, EventsSignature, projectionContext, thisInfo, remover->events);
                Js::JavascriptWinRTFunction * function = BuildDirectFunction(signature, RemoveEventListenerThunk, propertyId, false);
                SetProperty(function, lengthId, two);
                SetProperty(dynamicObject, propertyId, function);
                return;
            }
        case ptAbiEventHandlerProperty:
            {
                Assert(thisInfo->CanHoldEventCookies());
                RtABIEVENTHANDLERPROPERTY eventHandler = AbiEventHandlerProperty::From(prop);
                EventHandlerSignature *signature = RecyclerNew(recycler, EventHandlerSignature, projectionContext, thisInfo, eventHandler->abiEvent, prop->identifier);

                AutoHeapString getterName;
                auto eventName = StringOfId(eventHandler->abiEvent->metadataNameId);
                getterName.CreateNew(4 /* size of 'get' + '\0' */  + Js::JavascriptString::GetBufferLength(eventName));
                getterName.CreateNew(getterName.GetLength());
                LPWSTR getterNameStr = getterName.Get();
                wcscpy_s(getterNameStr, getterName.GetLength(), _u("get"));
                wcscat_s(getterNameStr, getterName.GetLength(), eventName);

                Js::JavascriptWinRTFunction * getter = BuildDirectFunction(signature, GetEventHandlerThunk, IdOfString(getterNameStr), false);

                getterNameStr[0] = _u('s');
                Js::JavascriptWinRTFunction * setter = BuildDirectFunction(signature, SetEventHandlerThunk, IdOfString(getterNameStr), false);
                SetProperty(setter, lengthId, one);

                Js::VerifyCatastrophic(dynamicObject->SetAccessors(propertyId, getter, setter));
                dynamicObject->SetConfigurable(propertyId, projectionContext->AreProjectionPrototypesConfigurable());

                return;
            }
        case ptAbiMethodProperty:
            {
                auto abiMethod = AbiMethodProperty::From(prop);
                auto functionVar = ExprToJsVar(prop->expr, thisInfo);
                if (functionVar != nullptr)
                {
                    auto function = Js::JavascriptWinRTFunction::FromVar(functionVar);
                    SetProperty(dynamicObject, propertyId, function);
                    SetProperty(function, lengthId, Js::JavascriptNumber::ToVar(abiMethod->body->signature->inParameterCount, projectionContext->GetScriptContext()));
                }
                return;
            }
        case ptOverloadParentProperty:
            {
                auto overload = OverloadParentProperty::From(prop);
                auto functionVar = ExprToJsVar(prop->expr, thisInfo);
                if (functionVar != nullptr)
                {
                    auto function = Js::JavascriptWinRTFunction::FromVar(functionVar);
                    auto length =
                        FunctionLengthProperty::From(
                        overload->overloadConstructor->properties->fields->WhereSingle([&](RtPROPERTY field) {
                        return wcscmp(_u("length"), StringOfId(field->identifier)) == 0;
                    }))->value->value;
                    SetProperty(dynamicObject, propertyId, function);
                    SetProperty(function, lengthId, Js::JavascriptNumber::ToVar(length, projectionContext->GetScriptContext()));
                }
                return;
            }
        case ptAbiFieldProperty:
        case ptFunctionLengthProperty:
            {
                Var value = ExprToJsVar(prop->expr, thisInfo);
                if (value != nullptr) //Nullptr means unresolvable
                {
                    SetProperty(dynamicObject, propertyId, value);
                }
                return;
            }
        case ptAbiPropertyProperty:
            {
                auto abiprop = AbiPropertyProperty::From(prop);

                Js::JavascriptWinRTFunction * getter = nullptr;
                if (abiprop->getter.HasValue())
                {
                    getter = FunctionOfSignature(abiprop->getter.GetValue(), nullptr, thisInfo, false);
                    if (getter == nullptr)
                    {
                        return; //Type of one of getter arguments can't be resolved
                    }
                }
                Js::JavascriptWinRTFunction * setter = nullptr;
                if (abiprop->setter.HasValue())
                {
                    setter = FunctionOfSignature(abiprop->setter.GetValue(), nullptr, thisInfo, false);
                    if (setter == nullptr)
                    {
                        return; //Type of one of setter arguments can't be resolved
                    }
                }
                Js::VerifyCatastrophic(dynamicObject->SetAccessors(propertyId, getter, setter));
                dynamicObject->SetConfigurable(propertyId, projectionContext->AreProjectionPrototypesConfigurable());
                return;
            }
        case ptUnresolvableNameConflictProperty:
            {
                // In this case, there are two or more properties with unresolvable conflicts.
                Js::JavascriptWinRTFunction * unresolvableThunk = BuildDirectFunction(nullptr, UnresolvableNameConflictThunk, propertyId, false);
                Js::VerifyCatastrophic(dynamicObject->SetAccessors(propertyId, unresolvableThunk, unresolvableThunk));
                dynamicObject->SetConfigurable(propertyId, false);
                return;
            }
        case ptAbiArrayLengthProperty:
            // Not added to the prototype
            return;
        }
        Js::Throw::FatalProjectionError();
    }


    // Info:        Apply a model properties object to the given Var
    // Parameters:  propertiesObject - the model properties object
    //              object - the object to apply the property to
    //              thisInfo - describes what kind of 'this'
    void ProjectionWriter::ApplyPropertiesObjectToJsObject(Var object, RtPROPERTIESOBJECT propertiesObject, ThisInfo * thisInfo)
    {
        Js::DynamicObject * dynamicObject = Js::DynamicObject::FromVar(object);
        propertiesObject->fields->Iterate([&](RtPROPERTY prop) {
#if DBG
            LPCWSTR name = projectionContext->StringOfId(prop->identifier);
            Assert(name);
#endif
            ApplyPropertyToJsObject(prop, dynamicObject, thisInfo);
        });
    }

    // Info:        Convert a properties object to a Var
    // Parameters:  propertiesObject - the model properties object
    //              thisInfo - describes what kind of 'this'
    // Returns:     The resulting Var
    Var ProjectionWriter::PropertiesObjectToJsVar(__in RtPROPERTIESOBJECT propertiesObject, __in ThisInfo * thisInfo)
    {
        Var object;
        HRESULT hr = VectorArray::CreateProtypeObject(thisInfo->IsArray(), projectionContext, &object);
        IfFailedMapAndThrowHr(projectionContext->GetScriptContext(), hr);
        ApplyPropertiesObjectToJsObject(object, propertiesObject, thisInfo);
        return object;
    }

    // Info:        Convert a model expression into a Var
    // Parameters:  expr - the model expression
    //              thisInfo - describes what kind of 'this'
    // Returns:     The resulting Var
    Var ProjectionWriter::ExprToJsVar(__in RtEXPR expr, __in ThisInfo * thisInfo)
    {
        switch(expr->type)
        {

        case exprFunction:
            {
                RtFUNCTION function = Function::From(expr);
                return FunctionToJsFunction(function, thisInfo);
            }
        case exprPropertiesObject:
            {
                RtPROPERTIESOBJECT propertiesObject = PropertiesObject::From(expr);
                return PropertiesObjectToJsVar(propertiesObject, thisInfo);
            }
        case exprNullLiteral:
            {
                return projectionContext->GetScriptContext()->GetLibrary()->GetNull();
            }
        case exprUInt32Literal:
            {
                RtUINT32LITERAL uint32Literal = UInt32Literal::From(expr);
                return Js::JavascriptNumber::ToVar(uint32Literal->value, projectionContext->GetScriptContext());
            }
        case exprInt32Literal:
            {
                RtINT32LITERAL int32Literal = Int32Literal::From(expr);
                return Js::JavascriptNumber::ToVar(int32Literal->value, projectionContext->GetScriptContext());
            }
        case exprEnum:
            {
                RtENUM _enum = Enum::From(expr);
                return EnumToObject(_enum, thisInfo);
            }

        }
        Js::Throw::FatalProjectionError();
    }

    // Info:        Attempt to convert a model expr into a Var
    // Parameters:  expr - the model expression
    //              result - receives the result
    // Returns:     true if there was a conversion
    Var ProjectionWriter::WriteExpr(__in RtEXPR expr)
    {
        return ExprToJsVar(expr, recyclerData->namespaceThis);
    }

    // ScriptSite is being closed, so close the access to items that could no longer use scriptSite
    void ProjectionWriter::MarkForClose(bool disableUnregister)
    {
        criticalSectionForUnknownsToMark->EnterCriticalSection();

        // Close all the unknowns
        unknownsToMark->Map([](CUnknownImpl * const& element)
        {
            Assert(element != nullptr);
            element->MarkForClose();
        });

#if DBG
        isMarkedForClose = true;
#endif
        this->eventHandlingInstanceList.Clear();

        // Set the flag to not unregister unknowns
        if (disableUnregister)
        {
            criticalSectionForUnknownsToMark->DisableUnregister();
        }

        criticalSectionForUnknownsToMark->LeaveCriticalSection();
    }

    // Info:        Closes the projection writer which then frees recycler allocated resources
    // Returns:     HRESULT
    HRESULT ProjectionWriter::Close()
    {
        // Cleanup resources from delegates, events, collections etc
        HRESULT hr = S_OK;
        BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT_NESTED
        {
            Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
            // No need to unregister unknowns anymore - set the flag
            MarkForClose(true);
            unknownsToMark->Clear();

            // Cleanup weak references
            int nObjects = recyclerData->m_eventHandlerCache->Count();
            for (int iIndex = 0, iObject = 0; iObject < nObjects; iIndex++)
            {
                IWeakReference *weakReference = recyclerData->m_eventHandlerCache->GetKeyAt(iIndex);
                if (weakReference == nullptr) //Already removed entry
                {
                    continue;
                }

                // We can collect this object so remove it from the dictionary
                EventProjectionHandler *eventProjectionHandler = recyclerData->m_eventHandlerCache->GetValueAt(iIndex);
                recyclerData->m_eventHandlerCache->Remove(weakReference);

                // Remove the event handlers if any
                CComPtr<IInspectable> inspectable = nullptr;
                Assert(!scriptContext->GetThreadContext()->IsScriptActive());
                hr = weakReference->Resolve(__uuidof(IInspectable), &inspectable);

                if (SUCCEEDED(hr) && inspectable != nullptr)
                {
                    JS_ETW(const char16* typeName = scriptContext->GetPropertyName(eventProjectionHandler->GetTypeId())->GetBuffer());
                    JS_ETW(EventWriteJSCRIPT_PROJECTION_REMOVEALLEVENTSANDEVENTHANDLERS_START(typeName));
                    eventProjectionHandler->RemoveAllEventsAndEventHandlers(inspectable, scriptContext);
                    JS_ETW(EventWriteJSCRIPT_PROJECTION_REMOVEALLEVENTSANDEVENTHANDLERS_STOP(typeName));
                }

                Assert(!scriptContext->GetThreadContext()->IsScriptActive());
                weakReference->Release();
                iObject++;
            }

            // Cleanup events on factory and statics
            runtimeClassThisToCleanupOnClose->Map([scriptContext](int index, RuntimeClassThis * runtimeClassThis)
            {
                JS_ETW(EventWriteJSCRIPT_PROJECTION_REMOVEALLEVENTSANDEVENTHANDLERS_START(scriptContext->GetPropertyName(runtimeClassThis->typeId)->GetBuffer()));
                runtimeClassThis->GetEventProjectionHandler()->RemoveAllEventsAndEventHandlers(runtimeClassThis->factory, scriptContext);
                JS_ETW(EventWriteJSCRIPT_PROJECTION_REMOVEALLEVENTSANDEVENTHANDLERS_STOP(scriptContext->GetPropertyName(runtimeClassThis->typeId)->GetBuffer()));
            });
            runtimeClassThisToCleanupOnClose->Clear();

            // Cleanup weak refernce to WeakPropertyBag list
            int cWeakPropertyBags = recyclerData->weakReferenceToWeakPropertyBagMap->Count();
            for (int iIndex = 0, iObject = 0; iObject < cWeakPropertyBags; iIndex++)
            {
                IWeakReference *weakReference = recyclerData->weakReferenceToWeakPropertyBagMap->GetKeyAt(iIndex);
                if (weakReference == nullptr) //Already removed entry
                {
                    continue;
                }

                // We can release this weakReference
                recyclerData->m_eventHandlerCache->Remove(weakReference);
                Assert(!scriptContext->GetThreadContext()->IsScriptActive());
                weakReference->Release();
                iObject++;
            }

            if (propertyValueFactory != nullptr)
            {
                Assert(!scriptContext->GetThreadContext()->IsScriptActive());
                propertyValueFactory->Release();
                propertyValueFactory = nullptr;
            }

            ThreadContext* threadContext = ThreadContext::GetContextForCurrentThread();
            threadContext->RemoveExternalWeakReferenceCache(this);

            recyclerData->m_InspectablesCache = nullptr;
            recyclerData->m_eventHandlerCache = nullptr;
            recyclerData->weakReferenceToWeakPropertyBagMap = nullptr;

            criticalSectionForUnknownsToMark->Release();
            criticalSectionForUnknownsToMark = nullptr;
        }
        recyclerData.Unroot(recycler);
        END_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)
        return hr;
    }

    // Collect the objects that can be claimed (which do not have outstanding references remaining)
    void ProjectionWriter::MarkNow(Recycler *recycler, bool inPartialCollect)
    {
        if (inPartialCollect)
        {
            Assert(supportsWeakDelegate || rootInPartialGCInstanceList.Count() == 0);

            // All the objects and all the delegates are rooted.
            rootInPartialGCInstanceList.Iterate([&](EventHandlingProjectionObjectInstance *instance) {
                Assert(instance->supportsRefCountProbe);
                instance->MarkAsRoot();
            });
        }
        else
        {
            Assert(supportsWeakDelegate || eventHandlingInstanceList.Count() == 0);

            // Go through all the winrtObjects and determine if they need to be rooted
            // Also mark the event handlers on the rooted winRT objects
            eventHandlingInstanceList.Iterate([&](EventHandlingProjectionObjectInstance *instance) {
                instance->TrackRefCount(recycler);
            });
        }

        // Clear the list of new instances since last GC
        rootInPartialGCInstanceList.Clear();

        // Mark the unknown impls
        criticalSectionForUnknownsToMark->EnterCriticalSection();
        unknownsToMark->Map([recycler, inPartialCollect](CUnknownImpl * const& element)
        {
            Assert(element != nullptr);
            element->MarkScriptContextDependentResources(recycler, inPartialCollect);
        });
        criticalSectionForUnknownsToMark->LeaveCriticalSection();
    }

    void ProjectionWriter::ResolveNow(Recycler *recycler)
    {
        if (recyclerData->m_eventHandlerCache == nullptr)
        {
            return;
        }

        fResolvingNow = true;
        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
        int nObjects = recyclerData->m_eventHandlerCache->Count();

        // The object count can either increase, decrease or stay same when the delegate is called while we are waiting for weakreference to be resolved

        // If new objects are added or if the cache doesnt change at all, it doesnt affect this routine.
        // - This can happen if delegate was called and didnt do any new event handling things, or added new event handlers but never gced.
        // For these scenarios, our routine is already tolerent

        // If some objects are removed from the cache
        // - This can happen if delegate was called and there was another round of GC - detect this using the 'fResolvingNow' reset.
        // - Or this can happen if there was the ScriptContext::Close - this can be decided using 'm_eventHandlerCache' being set to nullptr

        for (int iIndex = 0, iObject = 0; recyclerData->m_eventHandlerCache != nullptr && iObject < nObjects; iIndex++)
        {
            IWeakReference *weakReference = recyclerData->m_eventHandlerCache->GetKeyAt(iIndex);
            if (weakReference == nullptr) //Already removed entry
            {
                continue;
            }

            IInspectable *inspectable = NULL;
            HRESULT hr = S_OK;
            Assert(!scriptContext->GetThreadContext()->IsScriptActive());

            // This call can result in re-entrant resolve now if WeakReference is on different thread of out of Proc.
            // Delegate can be invoked and may result in gc which inturn calls resolve now.
            hr = weakReference->Resolve(__uuidof(IInspectable), &inspectable);

            // There was another round of resolve now so dont continue resolving futher here.
            if (!fResolvingNow)
            {
                if (SUCCEEDED(hr) && inspectable != nullptr)
                {
                    inspectable->Release();
                }
                break;
            }

            if (FAILED(hr) || inspectable == nullptr)
            {
                // We can collect this object so remove it from the dictionary
                recyclerData->m_eventHandlerCache->Remove(weakReference);
                weakReference->Release();
            }
            else
            {
                // The object is still alive: Mark the object as accessible
                inspectable->Release();
            }

            // The above release call can also make this function re-entrant so check if we really need to continue
            if (!fResolvingNow)
            {
                break;
            }

            iObject++;
        }

        int cWeakPropertyBags = (recyclerData->weakReferenceToWeakPropertyBagMap != nullptr) ? recyclerData->weakReferenceToWeakPropertyBagMap->Count() : 0;
        for (int iIndex = 0, iObject = 0; recyclerData->weakReferenceToWeakPropertyBagMap != nullptr && iObject < cWeakPropertyBags; iIndex++)
        {
            IWeakReference *weakReference = recyclerData->weakReferenceToWeakPropertyBagMap->GetKeyAt(iIndex);
            if (weakReference == nullptr) //Already removed entry
            {
                continue;
            }

            IInspectable *inspectable = NULL;
            HRESULT hr = S_OK;
            Assert(!scriptContext->GetThreadContext()->IsScriptActive());

            // This call can result in re-entrant resolve now if WeakReference is on different thread of out of Proc.
            // Delegate can be invoked and may result in gc which inturn calls resolve now.
            hr = weakReference->Resolve(__uuidof(IInspectable), &inspectable);

            // There was another round of resolve now so dont continue resolving futher here.
            if (!fResolvingNow)
            {
                if (SUCCEEDED(hr) && inspectable != nullptr)
                {
                    inspectable->Release();
                }
                break;
            }

            if (FAILED(hr) || inspectable == nullptr)
            {
                // We can collect this object so remove it from the dictionary
                recyclerData->weakReferenceToWeakPropertyBagMap->Remove(weakReference);
                weakReference->Release();
            }
            else if (SUCCEEDED(hr))
            {
                // The object is still alive: Mark the object as accessible
                inspectable->Release();
            }

            // The above release call can also make this function re-entrant so check if we really need to continue
            if (!fResolvingNow)
            {
                break;
            }

            iObject++;
        }

#ifdef WINRTFINDPREMATURECOLLECTION
        criticalSectionForUnknownsToMark->EnterCriticalSection();
        unknownsToMark->Map([&](CUnknownImpl * const& element)
        {
            Assert(element != nullptr);
            element->VerifyNotDetached ();
        });
        criticalSectionForUnknownsToMark->LeaveCriticalSection();
#endif

        fResolvingNow = false;
    }

    void ProjectionWriter::RegisterUnknownToCleanOnClose(CUnknownImpl *unknownImpl)
    {
        unknownsToMark->Item(unknownImpl);
    }

    void ProjectionWriter::UnRegisterUnknownToCleanOnClose(CUnknownImpl *unknownImpl)
    {
        unknownsToMark->Remove(unknownImpl);
    }

    void ProjectionWriter::AddRuntimeClassThisToCleanupOnClose(RuntimeClassThis *runtimeClassThis)
    {
        if (!runtimeClassThisToCleanupOnClose->Contains(runtimeClassThis))
        {
            runtimeClassThisToCleanupOnClose->Add(runtimeClassThis);
        }
    }

    Js::JavascriptFunction* ProjectionWriter::GetPromiseMaker()
    {
        return projectionContext->GetProjectionExternalLibrary()->GetWinRTPromiseConstructor();
    }

    Windows::Foundation::IPropertyValueStatics *ProjectionWriter::GetPropertyValueFactory()
    {
        if (propertyValueFactory == nullptr)
        {
            projectionContext->CreateTypeFactoryInstance(_u("Windows.Foundation.PropertyValue"), Windows::Foundation::IID_IPropertyValueStatics, (IUnknown **)&propertyValueFactory);
        }

        return propertyValueFactory;
    }

    void ProjectionWriter::AddArrayProjection(PropertyId propertyId, ArrayProjection *arrayProjection)
    {
        recyclerData->m_mapArrayProjection->Add(propertyId, arrayProjection);
    }

    ArrayProjection *ProjectionWriter::GetArrayProjection(PropertyId propertyId)
    {
        ArrayProjection *arrayProjection = nullptr;
        recyclerData->m_mapArrayProjection->TryGetValue(propertyId, &arrayProjection);
        return arrayProjection;
    }

    // Info:        Construct the projection writer
    // Parameters:  projectionContext - context for the given projection writer
    // Returns:
    ProjectionWriter::ProjectionWriter(ProjectionContext * projectionContext)
        : recycler(projectionContext->GetScriptContext()->GetRecycler()),
          projectionContext(projectionContext),
          propertyValueFactory(nullptr),
          fResolvingNow(false),
          eventHandlingInstanceList(projectionContext->ProjectionAllocator()),
          rootInPartialGCInstanceList(projectionContext->ProjectionAllocator())
    {
        Assert(projectionContext);

#if DBG
        supportsWeakDelegate = projectionContext->SupportsWeakDelegate();
        isMarkedForClose = false;
#endif

        lengthId = IdOfString(_u("length"));
        targetId = IdOfString(_u("target"));
        detailId = IdOfString(_u("detail"));
        typeId = IdOfString(_u("type"));

        ArenaAllocator * alloc = projectionContext->ProjectionAllocator();
        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();

        criticalSectionForUnknownsToMark = new CriticalSectionForUnknownImpl();
        IfNullMapAndThrowHr(scriptContext, criticalSectionForUnknownsToMark, E_OUTOFMEMORY);

        recyclerData.Root(RecyclerNew(recycler, RecyclerData), recycler);        

        recyclerData->namespaceThis = RecyclerNew(recycler, NamespaceThis);

        recyclerData->unknownThis = RecyclerNew(recycler, UnknownThis, MetadataStringIdNil, nullptr, nullptr);
        recyclerData->unknownEventHandlingThis = RecyclerNew(recycler, UnknownEventHandlingThis, MetadataStringIdNil, nullptr, nullptr);

        // Currently all types are alive for complete lifetime of scriptcontext so recyclable list wouldnt hurt
        recyclerData->runtimeClassTypeInformation = RecyclerNew(recycler, TYPEINFORMATIONMAP , recycler, 8);
        htypesEnum = Anew(alloc, HTYPEMAP , alloc);
        htypesStruct = Anew(alloc, HTYPEMAP , alloc);
        recyclerData->m_InspectablesCache =  RecyclerNew(recycler, PROJECTIONINSTANCEMAP, recycler, 8);
        recyclerData->m_eventHandlerCache =  RecyclerNew(recycler, EVENTHANDLERMAP , recycler, 2);
        unknownsToMark = Anew(alloc, CUNKNOWNIMPLHASHSET, alloc);
        runtimeClassThisToCleanupOnClose = RUNTIMECLASSTHISLIST::New(alloc);
        recyclerData->m_mapArrayProjection = RecyclerNew(recycler, ArrayTypeMap, recycler,  8);
        recyclerData->weakReferenceToWeakPropertyBagMap = RecyclerNew(recycler, WEAKREFERENCETOWEAKPROPERTYBAGMAP, recycler, 2);

        recyclerData->inUseUnknownReleasers = RecyclerNew(recycler, UNKNOWNSRELEASERLIST, recycler);
        recyclerData->unUsedUnknownReleasers = RecyclerNew(recycler, UNKNOWNSRELEASERLIST, recycler);

        two = Js::JavascriptNumber::ToVar(2, scriptContext); // ints are tagged ones so they dont need recycler addref
        one = Js::JavascriptNumber::ToVar(1, scriptContext);

        ThreadContext* threadContext = ThreadContext::GetContextForCurrentThread();
        threadContext->AddExternalWeakReferenceCache(this);
    }

    UnknownsReleaser *ProjectionWriter::GetUnknownsReleaser()
    {
        UnknownsReleaser *unknownsReleaser = nullptr;

        // Try to use one of the unused releaser
        for (int unUsedReleaserCount = recyclerData->unUsedUnknownReleasers->Count() ; unUsedReleaserCount > 0; unUsedReleaserCount--)
        {
            unknownsReleaser = recyclerData->unUsedUnknownReleasers->Item(unUsedReleaserCount - 1);
            if (unknownsReleaser != nullptr)
            {
                recyclerData->unUsedUnknownReleasers->Item(unUsedReleaserCount - 1, nullptr);
                break;
            }
        }

        // If there wasnt unknownsReleaser that we could reuse, create new one
        if (unknownsReleaser == nullptr)
        {
            // No unUsedUnknownsReleaser - so create new one.
            unknownsReleaser = RecyclerNewFinalized(recycler, UnknownsReleaser, projectionContext->GetScriptContext()->GetThreadContext()->GetPageAllocator());

            // Creating space for unUsedUnknownsReleasers so that in case we need to add unknownsReleaser to unused list later,
            // it doesnt throw exception for getting new buffer
            recyclerData->unUsedUnknownReleasers->Add(nullptr);
        }

        recyclerData->inUseUnknownReleasers->Add(unknownsReleaser);
        return unknownsReleaser;
    }

    void ProjectionWriter::SetUnknownsReleaserAsUnused(
#if DBG
        UnknownsReleaser *unknownsReleaser
#endif
        )
    {
        Assert(recyclerData->inUseUnknownReleasers->Count() > 0);

        UnknownsReleaser *lastUnknownsReleaser = recyclerData->inUseUnknownReleasers->Item(recyclerData->inUseUnknownReleasers->Count() - 1);
        Assert(unknownsReleaser == lastUnknownsReleaser);

#if DBG
        int fAddedToUnsedReleasers = false;
#endif

        // Add it to unused UnknownsReleaser
        for (int iIndex = 0; iIndex < recyclerData->unUsedUnknownReleasers->Count(); iIndex++)
        {
            // Found the reuse slot, update it
            if (recyclerData->unUsedUnknownReleasers->Item(iIndex) == nullptr)
            {
                recyclerData->unUsedUnknownReleasers->Item(iIndex, lastUnknownsReleaser);
#if DBG
                fAddedToUnsedReleasers = true;
#endif
                break;
            }
        }

        Assert(fAddedToUnsedReleasers);
        recyclerData->inUseUnknownReleasers->RemoveAtEnd();
    }

    void ProjectionWriter::SetUnknownsListForDisposingLater(
#if DBG
        UnknownsReleaser *unknownsReleaser,
#endif
        ImmutableList<IUnknown *> *unknowns,
        ImmutableList<IUnknown **> *unknownRefs,
        ArenaAllocator *alloc)
    {
        Assert(recyclerData->inUseUnknownReleasers->Count() > 0);

        UnknownsReleaser *lastUnknownsReleaser = recyclerData->inUseUnknownReleasers->Item(recyclerData->inUseUnknownReleasers->Count() - 1);
        Assert(unknownsReleaser == lastUnknownsReleaser);

        lastUnknownsReleaser->SetUnknownsList(unknowns, unknownRefs, alloc);
        recyclerData->inUseUnknownReleasers->RemoveAtEnd();

        Assert(recyclerData->unUsedUnknownReleasers->Item(recyclerData->unUsedUnknownReleasers->Count() - 1) == nullptr);
        recyclerData->unUsedUnknownReleasers->RemoveAtEnd();
    }

    void UnknownsReleaser::SetUnknownsList(ImmutableList<IUnknown *> *unknowns, ImmutableList<IUnknown **> *unknownRefs, ArenaAllocator *allocatorOfUnknown)
    {
        Assert(this->unknowns == nullptr);
        Assert(this->unknownRefs == nullptr);
        this->unknowns = unknowns;
        this->unknownRefs = unknownRefs;
        this->allocator.Move(allocatorOfUnknown);
    }

    void UnknownsReleaser::Dispose(bool isShutdown)
    {
        if (!isShutdown)
        {
            if (unknowns)
            {
                unknowns->Iterate([&](IUnknown * unknown) {
                    unknown->Release();
                });
                unknowns = nullptr;
            }

            if (unknownRefs)
            {
                unknownRefs->Iterate([&](IUnknown ** unknown) {
                    if (*unknown)
                    {
                        (*unknown)->Release();
                    }
                });
                unknownRefs = nullptr;
            }

            allocator.Clear();
        }
    }

    HostProfilerHeapObject* CreateWinrtConstructorObjectElement(ActiveScriptProfilerHeapEnum* heapEnum, Js::RecyclableObject* obj)
    {
        auto function = Js::JavascriptWinRTConstructorFunction::FromVar(obj);
        auto scriptContext = function->GetScriptContext();
        auto typeInformation = (Projection::RuntimeClassTypeInformation *)function->GetTypeInformation();
        Assert(typeInformation != nullptr);
        auto runtimeClassThisInfo = typeInformation->GetRuntimeClassThisInfo();

        HRESULT hr = S_OK;

        HostProfilerHeapObject **externalObjects = nullptr;
        USHORT externalObjectCount = 0;
        HostProfilerHeapObject *mainHeapInfo = nullptr;

        uint eventCount = runtimeClassThisInfo->GetEventAndEventHandlerCount();
        IUnknown *unknown = nullptr;
        UINT allocSize;
        // Main HeapObjectInfo
        if (runtimeClassThisInfo->factory == nullptr)
        {
            Assert(eventCount == 0);
            // Here we can populate only our own information;
            allocSize = offsetof(HostProfilerHeapObject, optionalInfo);
        }
        else
        {
            {
                hr = runtimeClassThisInfo->factory->QueryInterface(&unknown);
                if (SUCCEEDED(hr))
                {
                    unknown->Release();
                }
            }
            IFFAILGO(hr);

            externalObjectCount = 1;
            allocSize = sizeof(HostProfilerHeapObject *) * externalObjectCount;
            externalObjects = (HostProfilerHeapObject **)CoTaskMemAlloc(allocSize);
            IFNULLMEMGO(externalObjects);
            memset(externalObjects, 0, allocSize);

            // External Object information
            allocSize = offsetof(HostProfilerHeapObject, optionalInfo) // size till optionalInfo
                + offsetof(ProfilerHeapObjectOptionalInfo, eventList.elements) // size of eventCount
                + (sizeof(PROFILER_HEAP_OBJECT_RELATIONSHIP) * eventCount); // size to hold events
            externalObjects[0] = (HostProfilerHeapObject *)CoTaskMemAlloc(allocSize);
            IFNULLMEMGO(externalObjects[0]);
            memset(externalObjects[0], 0, allocSize);

            // External object information for native pointer
            externalObjects[0]->externalAddress = unknown;
            externalObjects[0]->typeNameId = runtimeClassThisInfo->GetTypeId();
            externalObjects[0]->flags = PROFILER_HEAP_OBJECT_FLAGS_NEW_STATE_UNAVAILABLE | PROFILER_HEAP_OBJECT_FLAGS_EXTERNAL_UNKNOWN | PROFILER_HEAP_OBJECT_FLAGS_WINRT_RUNTIMECLASS | PROFILER_HEAP_OBJECT_FLAGS_SIZE_UNAVAILABLE;
            externalObjects[0]->optionalInfoCount = 1;

            // Event information optionalInfo
            ProfilerHeapObjectOptionalInfo *optionalInfo = (ProfilerHeapObjectOptionalInfo *)((byte *)(externalObjects[0]) + offsetof(HostProfilerHeapObject, optionalInfo));
            optionalInfo->infoType = PROFILER_HEAP_OBJECT_OPTIONAL_INFO_WINRTEVENTS;
            runtimeClassThisInfo->PopulateProfilerEventInfo(heapEnum, &(optionalInfo->eventList));

            // Size for main information
            allocSize = offsetof(HostProfilerHeapObject, optionalInfo) // size till optionalInfo
                + ActiveScriptProfilerHeapEnum::GetHeapObjectInternalPropertyInfoSize(); // InternalPropertySize
        }
        mainHeapInfo = (HostProfilerHeapObject *)CoTaskMemAlloc(allocSize);
        IFNULLMEMGO(mainHeapInfo);
        memset(mainHeapInfo, 0, allocSize);

        // Main HeapObject information
        mainHeapInfo->typeNameId = runtimeClassThisInfo->GetTypeId();
        mainHeapInfo->flags = PROFILER_HEAP_OBJECT_FLAGS_WINRT_RUNTIMECLASS;

        if (unknown != nullptr)
        {
            mainHeapInfo->optionalInfoCount = 1;
            ProfilerHeapObjectOptionalInfo *optionalInfo = (ProfilerHeapObjectOptionalInfo *)((byte *)(mainHeapInfo) + offsetof(HostProfilerHeapObject, optionalInfo));
            heapEnum->FillHeapObjectInternalUnnamedExternalProperty(optionalInfo, (PROFILER_EXTERNAL_OBJECT_ADDRESS)unknown);
            mainHeapInfo->externalObjectCount = externalObjectCount;
            mainHeapInfo->externalObjects = externalObjects;
        }

        return mainHeapInfo;

LReturn:
        if (externalObjects)
        {
            for (UINT i=0; i < externalObjectCount; i++)
            {
                if (externalObjects[i])
                {
                    CoTaskMemFree(externalObjects[i]);
                }
            }
            CoTaskMemFree(externalObjects);
        }
        if (mainHeapInfo != nullptr)
        {
            CoTaskMemFree(mainHeapInfo);
        }
        Js::JavascriptErrorDebug::MapAndThrowError(scriptContext, hr);
    }

    void ProjectionWriter::ReportUnknownImpls(ActiveScriptProfilerHeapEnum *profilerEnum)
    {
        criticalSectionForUnknownsToMark->EnterCriticalSection();
        unknownsToMark->Map([profilerEnum](CUnknownImpl * const& element)
        {
            Assert(element != nullptr);
            profilerEnum->VisitRoot(element);
        });
        criticalSectionForUnknownsToMark->LeaveCriticalSection();
    }

    void ProjectionWriter::OnNewEventHandlingInstance(EventHandlingProjectionObjectInstance *instance)
    {
        Assert((instance->isRooted && !instance->supportsRefCountProbe) || (!instance->isRooted && instance->supportsRefCountProbe));

        if (instance->supportsRefCountProbe)
        {
            Assert(supportsWeakDelegate);
            rootInPartialGCInstanceList.Prepend(instance);
            eventHandlingInstanceList.Prepend(instance);
        }
    }

    void ProjectionWriter::OnDisposeEventHandlingInstance(EventHandlingProjectionObjectInstance *instance)
    {
        Assert(supportsWeakDelegate);
        Assert(instance->supportsRefCountProbe);
        rootInPartialGCInstanceList.Remove(instance);
        Assert(!isMarkedForClose || eventHandlingInstanceList.Count() == 0);
        eventHandlingInstanceList.Remove(instance);
    }

    void ProjectionWriter::GetWeakPropertyBagFromWeakRef(bool fCreate, IWeakReference *weakReference, WEAKPROPERTYBAG **weakPropertyBag)
    {
        Assert(weakPropertyBag != nullptr);
        Js::ScriptContext* scriptContext = projectionContext->GetScriptContext();

        if (!recyclerData->weakReferenceToWeakPropertyBagMap->TryGetValue(weakReference, weakPropertyBag))
        {
            if (fCreate)
            {
                // Create a new one.
                Recycler *recycler = scriptContext->GetRecycler();
                *weakPropertyBag = RecyclerNew(recycler, WEAKPROPERTYBAG, recycler);

                recyclerData->weakReferenceToWeakPropertyBagMap->Add(weakReference, *weakPropertyBag);
                weakReference->AddRef();
            }
        }
    }

    bool ProjectionWriter::GetWeakPropertyBagFromUnknown(bool fCreate, IUnknown *unknown, WEAKPROPERTYBAG **weakPropertyBag)
    {
        bool fWeakReferencedObject = false;
        HRESULT hr = S_OK;
        Js::ScriptContext* scriptContext = projectionContext->GetScriptContext();

        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            CComPtr<IWeakReferenceSource> weakReferenceSource;
            CComPtr<IWeakReference> weakReference;
            if (SUCCEEDED(unknown->QueryInterface(__uuidof(IWeakReferenceSource), (void**)&weakReferenceSource))
                && SUCCEEDED(weakReferenceSource->GetWeakReference(&weakReference)))
            {
                BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT
                {
                    fWeakReferencedObject = true;
                    GetWeakPropertyBagFromWeakRef(fCreate, weakReference, weakPropertyBag);
                }
                END_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)
            }
        }
        END_LEAVE_SCRIPT(scriptContext)
        IfFailedMapAndThrowHr(scriptContext, hr);

        return fWeakReferencedObject;
    }

    Var ProjectionWriter::GetWeakWinRTProperty(ProjectionObjectInstance *instance, const Js::PropertyRecord *propertyRecord)
    {
        Assert(instance != nullptr);

        // PropertyRecord is not present, then the property isnt around either
        auto scriptContext = projectionContext->GetScriptContext();
        if (propertyRecord != nullptr)
        {
            // Look for this instance's propertyBag
            auto weakPropertyBag = instance->GetWeakPropertyBag(false);
            RecyclerWeakReference<Js::RecyclableObject> *weakProperty = nullptr;
            if (weakPropertyBag != nullptr && weakPropertyBag->TryGetValue(propertyRecord, &weakProperty))
            {
                if (Js::TaggedNumber::Is(weakProperty))
                {
                    return weakProperty;
                }

                Js::RecyclableObject *weakPropertyValue = weakProperty->Get();
                if (weakPropertyValue)
                {
                    return weakPropertyValue;
                }
            }
        }

        return scriptContext->GetLibrary()->GetNull();
    }

    void ProjectionWriter::SetWeakWinRTProperty(ProjectionObjectInstance *instance, const Js::PropertyRecord *propertyRecord, Var propertyValue)
    {
        Assert(instance != nullptr);
        Assert(propertyRecord != nullptr);


        // Get for this instance's propertyBag
        auto weakPropertyBag = instance->GetWeakPropertyBag(true);
        RecyclerWeakReference<Js::RecyclableObject> *weakProperty = nullptr;
        if (Js::TaggedNumber::Is(propertyValue))
        {
            weakProperty = (RecyclerWeakReference<Js::RecyclableObject> *)propertyValue;
        }
        else
        {
            Js::RecyclableObject *recyclablePropertyValue = Js::RecyclableObject::FromVar(propertyValue);
            weakProperty = recycler->CreateWeakReferenceHandle<Js::RecyclableObject>(recyclablePropertyValue);
        }
        weakPropertyBag->Item(propertyRecord, weakProperty);
    }
}
