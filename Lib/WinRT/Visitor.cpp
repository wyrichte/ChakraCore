//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------
#pragma once
#include "stdafx.h"

namespace ProjectionModel
{
    template<class T> 
    ImmutableList<const T *> * Visitor::VisitImmutableList(ImmutableList<const T *> *list)
    {
        bool fNew = false;
        const T *newVisitedType = nullptr;
        size_t index = 0;
        list->IterateWhile([&](const T * type)->bool {
            auto visitedType = VisitIndividual(type);
            if (visitedType != type)
            {
                fNew = true;
                newVisitedType = visitedType;
            }
            else
            {
                index++;
            }
            return !fNew;
        });

        if (fNew)
        {
            // We need to create new genericParameters list
            auto newList = ImmutableList<const T *>::Empty();
            auto newListTail = newList;
            list->IterateIn3Sets(index, [&](const T * type) {
                // These are the types that didnt change, so reuse them
                newList = newList->Append(type, a, &newListTail);
            }, [&](const T * type) {
                newList = newList->Append(newVisitedType, a, &newListTail);
            }, [&](const T * type) {
                auto visitedType = VisitIndividual(type);
                newList = newList->Append(visitedType, a, &newListTail);
            });

            Assert(newList->Count() == list->Count());
            return newList;
        }

        return list;
    }

    RtTYPE Visitor::VisitArrayType(RtARRAYTYPE type) 
    {
        auto elementType = VisitIndividual(type->elementType);
        if (elementType != type->elementType)
        {
            return RtAnew(a, ArrayType, type->fullTypeNameId, elementType);
        }
        return type;
    }

    RtTYPE Visitor::VisitMissingGenericInstantiationType(RtMISSINGGENERICINSTANTIATIONTYPE type) 
    {
        auto parent = VisitIndividual(type->parent);
        auto genericParameters = VisitImmutableList(type->genericParameters);
        if (parent != type->parent || genericParameters != type->genericParameters)
        {
            return RtAnew(a, MissingGenericInstantiationType, MissingNamedType::From(parent), genericParameters, type->instantiatedNameId);
        }

        return type;
    }

    RtTYPE Visitor::VisitGenericClassVarType(RtGENERICCLASSVARTYPE type)
    {
        return type;
    }

    RtTYPE Visitor::VisitSystemGuidType(RtSYSTEMGUIDTYPE type)
    {
        return type;
    }

    RtTYPE Visitor::VisitWindowsFoundationDateTimeType(RtWINDOWSFOUNDATIONDATETIMETYPE type)
    {
        return type;
    }

    RtTYPE Visitor::VisitWindowsFoundationTimeSpanType(RtWINDOWSFOUNDATIONTIMESPANTYPE type)
    {
        return type;
    }

    RtTYPE Visitor::VisitWindowsFoundationEventRegistrationTokenType(RtWINDOWSFOUNDATIONEVENTREGISTRATIONTOKENTYPE type)
    {
        return type;
    }

    RtTYPE Visitor::VisitWindowsFoundationHResultType(RtWINDOWSFOUNDATIONHRESULTTYPE type)
    {
        return type;
    }

    RtTYPE Visitor::VisitInterfaceType(RtINTERFACETYPE type)
    {
        auto genericParameters = VisitImmutableList(type->genericParameters);
        if (genericParameters != type->genericParameters)
        {
            return RtAnew(a, InterfaceType, type->typeId, type->typeDef, genericParameters, type->iid);
        }

        return type;
    }

    RtTYPE Visitor::VisitClassType(RtCLASSTYPE type)
    {
        return RtAnew(a, ClassType, type->typeId, type->typeDef);
    }

    RtTYPE Visitor::VisitStructType(RtSTRUCTTYPE type)
    {
        return type;
    }

    RtTYPE Visitor::VisitDelegateType(RtDELEGATETYPE type)
    {
        auto genericParameters = VisitImmutableList(type->genericParameters);

        if (genericParameters != type->genericParameters)
        {
            // The eventInfo would be actually visited as part of the VisitEvent
            return RtAnew(a, DelegateType, type->typeId, type->typeDef, genericParameters);
        }

        return type;
    }

    RtTYPE Visitor::VisitEnumType(RtENUMTYPE type)
    {
        return type;
    }

    RtTYPE Visitor::VisitByRefType(RtBYREFTYPE type)
    {
        auto pointedTo = VisitIndividual(type->pointedTo);
        if (pointedTo != type->pointedTo)
        {
            return RtAnew(a, ByRefType, type->fullTypeNameId, pointedTo);
        }

        return type;
    }

    RtTYPE Visitor::VisitMissingNamedType(RtMISSINGNAMEDTYPE type)
    {
        return type;
    }

    RtTYPE Visitor::VisitBasicType(RtBASICTYPE type)
    {
        return type;
    }

    RtTYPE Visitor::VisitUnprojectableType(RtUNPROJECTABLETYPE type)
    {
        return type;
    }

    RtTYPE Visitor::VisitVoidType(RtVOIDTYPE type)
    {
        return type;
    }

    RtTYPE Visitor::VisitGenericParameterType(RtGENERICPARAMETERTYPE type)
    {
        return type;
    }    

    RtEVENT Visitor::VisitIndividual(RtEVENT evnt)
    {
        auto addOn = evnt->addOn ? VisitMethodSignature(evnt->addOn) : nullptr;
        auto removeOn = evnt->removeOn ? VisitMethodSignature(evnt->removeOn) : nullptr;

        if (addOn != evnt->addOn || removeOn != evnt->removeOn)
        {
#if DBG
            return Anew(a, Event, addOn, removeOn, evnt->metadataNameId, evnt->nameId, evnt->nameStr);
#else
            return Anew(a, Event, addOn, removeOn, evnt->metadataNameId, evnt->nameId);
#endif
        }

        return evnt;
    }

    RtTYPE Visitor::VisitIndividual(RtTYPE type) 
    {
        switch(type->typeCode)
        {
            case tcArrayType: return VisitArrayType(ArrayType::From(type));
            case tcMissingGenericInstantiationType: return VisitMissingGenericInstantiationType(MissingGenericInstantiationType::From(type)); 
            case tcGenericClassVarType: return VisitGenericClassVarType(GenericClassVarType::From(type));
            case tcSystemGuidType: return VisitSystemGuidType(SystemGuidType::From(type));

            case tcWindowsFoundationDateTimeType: return VisitWindowsFoundationDateTimeType(WindowsFoundationDateTimeType::From(type));
            case tcWindowsFoundationTimeSpanType: return VisitWindowsFoundationTimeSpanType(WindowsFoundationTimeSpanType::From(type));
            case tcWindowsFoundationEventRegistrationTokenType: return VisitWindowsFoundationEventRegistrationTokenType(WindowsFoundationEventRegistrationTokenType::From(type));
            case tcWindowsFoundationHResultType: return VisitWindowsFoundationHResultType(WindowsFoundationHResultType::From(type));

            case tcInterfaceType: return VisitInterfaceType(InterfaceType::From(type));
            case tcClassType: return VisitClassType(ClassType::From(type));
            case tcStructType: return VisitStructType(StructType::From(type));
            case tcDelegateType: return VisitDelegateType(DelegateType::From(type));
            case tcEnumType: return VisitEnumType(EnumType::From(type));
            case tcByRefType: return VisitByRefType(ByRefType::From(type));
            case tcMissingNamedType: return VisitMissingNamedType(MissingNamedType::From(type));
            case tcBasicType: return VisitBasicType(BasicType::From(type)); 
            case tcUnprojectableType: return VisitUnprojectableType(UnprojectableType::From(type));
            case tcVoidType: return VisitVoidType(VoidType::From(type));
            case tcGenericParameterType: return VisitGenericParameterType(GenericParameterType::From(type));
        }
        Js::Throw::FatalProjectionError();
    }

    RtEXPR Visitor::VisitPropertiesObject(RtPROPERTIESOBJECT expr) 
    {
        auto fields = VisitImmutableList(expr->fields);
        if (fields != expr->fields)
        {
            return Anew(a, PropertiesObject, fields);        
        }

        return expr;
    }

    RtEXPR Visitor::VisitAssignmentSpace(RtASSIGNMENTSPACE expr) 
    {
        auto fields = VisitImmutableList(expr->vars);

        if (fields != expr->vars)
        {
            return Anew(a, AssignmentSpace, fields);        
        }
        return expr;
    }

    MetadataStringId Visitor::VisitPropertyName(MetadataStringId nameId)
    {
        return nameId;
    }

    RtPROPERTY Visitor::VisitAbiFieldProperty(RtABIFIELDPROPERTY prop)
    {
        auto name = VisitPropertyName(prop->identifier);
        auto expr = VisitExpr(prop->expr);
        auto type = VisitIndividual(prop->type);
        if (name != prop->identifier || expr != prop->expr || type != prop->type)
        {
            return RtAnew(a, AbiFieldProperty, name, expr, ConcreteType::From(type), prop->fieldProperties, prop->fieldOffset);
        }
        return prop;
    }

    RtPROPERTY Visitor::VisitOverloadParentProperty(RtOVERLOADPARENTPROPERTY prop)
    {
        auto name = VisitPropertyName(prop->identifier);
        auto overloadConstructor = OverloadGroupConstructor::From(VisitOverloadGroupConstructor(prop->overloadConstructor));

        if (name != prop->identifier || overloadConstructor != prop->overloadConstructor)
        {
            return RtAnew(a, OverloadParentProperty, name, overloadConstructor);
        }
        return prop;
    }

    RtPROPERTY Visitor::VisitAbiMethodProperty(RtABIMETHODPROPERTY prop)
    {
        auto name = VisitPropertyName(prop->identifier);
        auto body = VisitExpr(prop->body);
        if (name != prop->identifier || body != prop->body)
        {
            return RtAnew(a, AbiMethodProperty, name, AbiMethod::From(body));
        }
        return prop;
    }

    RtPROPERTY Visitor::VisitAbiPropertyProperty(RtABIPROPERTYPROPERTY prop)
    {
        auto name = VisitPropertyName(prop->identifier);
        auto expr = VisitExpr(prop->expr);
        Option<AbiMethodSignature> getter = prop->getter.HasValue()
            ? AbiMethodSignature::From(VisitMethodSignature(prop->getter.GetValue()))
            : nullptr;

        Option<AbiMethodSignature> setter = prop->setter.HasValue()
            ? AbiMethodSignature::From(VisitMethodSignature(prop->setter.GetValue()))
            : nullptr;

        if (name != prop->identifier || 
            expr != prop->expr || 
            getter.GetValueOrNull() != prop->getter.GetValueOrNull() || 
            setter.GetValueOrNull() != prop->setter.GetValueOrNull())
        {
            return RtAnew(a, AbiPropertyProperty, name, expr, getter, setter, prop->metadataNameId);
        }
        return prop;
    }

    RtPROPERTY Visitor::VisitAbiArrayLengthProperty(RtABIARRAYLENGTHPROPERTY prop)
    {
        auto name = VisitPropertyName(prop->identifier);
        auto expr = VisitExpr(prop->expr);
        auto type = VisitIndividual(prop->type);
        auto getSize = AbiMethodSignature::From(VisitMethodSignature(prop->getSize));

        if (name != prop->identifier || expr != prop->expr || type != prop->type || getSize != prop->getSize)
        {
            return RtAnew(a, AbiArrayLengthProperty, name, expr, type, getSize);
        }

        return prop;
    }

    RtPROPERTY Visitor::VisitAbiTypeProperty(RtABITYPEPROPERTY prop)
    {
        auto identifier = VisitPropertyName(prop->identifier);
        auto expr = VisitExpr(prop->expr);
        if (identifier != prop->identifier || expr != prop->expr)
        {
            return RtAnew(a, AbiTypeProperty, identifier, expr);
        }
        
        return prop;
    }

    RtPROPERTY Visitor::VisitAbiNamespaceProperty(RtABINAMESPACEPROPERTY prop)
    {
        auto identifier = VisitPropertyName(prop->identifier);
        auto childNamespace = PropertiesObject::From(VisitPropertiesObject(prop->childNamespace));
        if (identifier != prop->identifier || childNamespace != prop->childNamespace)
        {
            return RtAnew(a, AbiNamespaceProperty, identifier, childNamespace);
        }

        return prop;
    }

    RtPROPERTY Visitor::VisitAbiAddEventListenerProperty(RtABIADDEVENTLISTENERPROPERTY prop)
    {
        auto name = VisitPropertyName(prop->identifier);
        auto events = VisitImmutableList(prop->events);
        auto expr = VisitExpr(prop->expr);
        if (name != prop->identifier || events != prop->events || expr != prop->expr)
        {
            return RtAnew(a, AbiAddEventListenerProperty, name, events, expr);
        }
        
        return prop;
    }

    RtPROPERTY Visitor::VisitAbiRemoveEventListenerProperty(RtABIREMOVEEVENTLISTENERPROPERTY prop)
    {
        auto name = VisitPropertyName(prop->identifier);
        auto events = VisitImmutableList(prop->events);
        auto expr = VisitExpr(prop->expr);
        if (name != prop->identifier || events != prop->events || expr != prop->expr)
        {
            return RtAnew(a, AbiRemoveEventListenerProperty, name, events, expr);
        }
        
        return prop;
    }

    RtPROPERTY Visitor::VisitAbiEventHandlerProperty(RtABIEVENTHANDLERPROPERTY prop)
    {
        MetadataStringId nameId = VisitPropertyName(prop->identifier);
        RtEVENT eventInfo = VisitIndividual(prop->abiEvent);
        RtEXPR expr = VisitExpr(prop->expr);
        if (nameId != prop->identifier || eventInfo != prop->abiEvent || expr != prop->expr)
        {
            return RtAnew(a, AbiEventHandlerProperty, nameId, expr, eventInfo);
        }
        return prop;
    }

    RtPROPERTY Visitor::VisitUnresolvableNameConflictProperty(RtUNRESOLVABLENAMECONFLICTPROPERTY prop)
    {
        auto identifier = VisitPropertyName(prop->identifier);
        auto expr = VisitExpr(prop->expr);
        auto conflictingProperties = VisitImmutableList(prop->conflictingProperties);
        if (identifier != prop->identifier || expr != prop->expr || conflictingProperties != prop->conflictingProperties)
        {
            return RtAnew(a, UnresolvableNameConflictProperty, identifier, expr, conflictingProperties);
        }

        return prop;
    }

    RtPROPERTY Visitor::VisitFunctionLengthProperty(RtFUNCTIONLENGTHPROPERTY prop)
    {
        auto name = VisitPropertyName(prop->identifier);
        if (name != prop->identifier)
        {
            return RtAnew(a, FunctionLengthProperty, name, prop->value);
        }

        return prop;
    }    

    RtPROPERTY Visitor::VisitIndividual(RtPROPERTY prop) 
    {
        switch(prop->propertyType)
        {
            case ptAbiFieldProperty: return VisitAbiFieldProperty(AbiFieldProperty::From(prop));
            case ptOverloadParentProperty: return VisitOverloadParentProperty(OverloadParentProperty::From(prop));
            case ptAbiMethodProperty: return VisitAbiMethodProperty(AbiMethodProperty::From(prop));
            case ptAbiPropertyProperty: return VisitAbiPropertyProperty(AbiPropertyProperty::From(prop));
            case ptAbiArrayLengthProperty: return VisitAbiArrayLengthProperty(AbiArrayLengthProperty::From(prop));
            case ptAbiTypeProperty: return VisitAbiTypeProperty(AbiTypeProperty::From(prop));
            case ptAbiNamespaceProperty: return VisitAbiNamespaceProperty(AbiNamespaceProperty::From(prop));
            case ptAbiAddEventListenerProperty: return VisitAbiAddEventListenerProperty(AbiAddEventListenerProperty::From(prop));
            case ptAbiRemoveEventListenerProperty: return VisitAbiRemoveEventListenerProperty(AbiRemoveEventListenerProperty::From(prop));
            case ptAbiEventHandlerProperty: return VisitAbiEventHandlerProperty(AbiEventHandlerProperty::From(prop));
            case ptUnresolvableNameConflictProperty: return VisitUnresolvableNameConflictProperty(UnresolvableNameConflictProperty::From(prop));
            case ptFunctionLengthProperty: return VisitFunctionLengthProperty(FunctionLengthProperty::From(prop));
        }
        Js::Throw::FatalProjectionError();
    }

    RtASSIGNMENT Visitor::VisitIndividual(RtASSIGNMENT assignment) 
    {
        auto expr = VisitExpr(assignment->expr);
        if (expr != assignment->expr)
        {
            return Anew(a, Assignment, assignment->identifier, expr);
        }
        return assignment;
    }

    RtPARAMETERS Visitor::VisitParameters(RtPARAMETERS parameters)
    {
        auto allParameters = VisitImmutableList(parameters->allParameters);
        auto returnType = VisitIndividual(parameters->returnType);
        if (allParameters != parameters->allParameters || returnType != parameters->returnType)
        {
            return Anew(a, Parameters, allParameters, returnType, parameters->sizeOfCallstack, parameters->callPattern);
        }

        return parameters;
    }

    RtPARAMETER Visitor::VisitIndividual(RtPARAMETER parameter) { return parameter; }

    RtABIMETHODSIGNATURE Visitor::VisitIndividual(RtABIMETHODSIGNATURE signature)
    {
        return signature;
    }

    RtMETHODSIGNATURE Visitor::VisitTypeConstructorMethodSignature(const TypeConstructorMethodSignature * signature)
    {
        auto parameters = VisitParameters(signature->parameters);
        if (parameters != signature->parameters)
        {
            return Anew(a, TypeConstructorMethodSignature, signature->nameId, parameters);
        }
        return signature;
    }

    RtMETHODSIGNATURE Visitor::VisitMissingTypeConstructorMethodSignature(const MissingTypeConstructorMethodSignature * signature)
    {
        auto parameters = VisitParameters(signature->parameters);
        if (parameters != signature->parameters)
        {
            return Anew(a, MissingTypeConstructorMethodSignature, signature->nameId, parameters);
        }
        return signature;
    }

    RtMETHODSIGNATURE Visitor::VisitOverloadedMethodSignature(const OverloadedMethodSignature * signature)
    {
        auto parameters = VisitParameters(signature->parameters);
        auto overloads = VisitOverloadGroup(signature->overloads);
        if (parameters != signature->parameters || overloads != signature->overloads)
        {
            return Anew(a, OverloadedMethodSignature, signature->nameId, overloads, parameters);
        }
        return signature;
    }

    RtMETHODSIGNATURE Visitor::VisitMissingDelegateInvokeMethodSignature(const MissingDelegateInvokeMethodSignature * signature)
    {
        auto parameters = VisitParameters(signature->parameters);
        if (parameters != signature->parameters)
        {
            return Anew(a, MissingDelegateInvokeMethodSignature, signature->interfaceName, signature->nameId, parameters);
        }
        return signature;
    }

    RtMETHODSIGNATURE Visitor::VisitGenericDelegateInvokeMethodSignature(const GenericDelegateInvokeMethodSignature * signature)
    {
        auto git = VisitIndividual(signature->delegateType);
        auto parameters = VisitParameters(signature->parameters);
        if (git != signature->delegateType || parameters != signature->parameters)
        {
            return Anew(a, GenericDelegateInvokeMethodSignature, signature->nameId, DelegateType::From(git), parameters);
        }
        return signature;
    }

    RtMETHODSIGNATURE Visitor::VisitUncallableMethodSignature(const UncallableMethodSignature * signature)
    {
        return signature;
    }
     
    RtMETHODSIGNATURE Visitor::VisitMethodSignature(RtMETHODSIGNATURE signature) 
    {
        switch(signature->signatureType)
        {
            case mstAbiMethodSignature: return VisitIndividual(AbiMethodSignature::From(signature));
            case mstTypeConstructorMethodSignature: return VisitTypeConstructorMethodSignature(TypeConstructorMethodSignature::From(signature));
            case mstMissingTypeConstructorMethodSignature: return VisitMissingTypeConstructorMethodSignature(MissingTypeConstructorMethodSignature::From(signature));
            case mstOverloadedMethodSignature: return VisitOverloadedMethodSignature(OverloadedMethodSignature::From(signature));
            case mstMissingDelegateInvokeMethodSignature: return VisitMissingDelegateInvokeMethodSignature(MissingDelegateInvokeMethodSignature::From(signature));
            case mstGenericDelegateInvokeMethodSignature: return VisitGenericDelegateInvokeMethodSignature(GenericDelegateInvokeMethodSignature::From(signature));
            case mstUncallableMethodSignature: return VisitUncallableMethodSignature(UncallableMethodSignature::From(signature));
        }
        Js::Throw::FatalProjectionError();
    }

    RtOVERLOADGROUP Visitor::VisitOverloadGroup(RtOVERLOADGROUP overloads) 
    {
        auto arityGroups = VisitImmutableList(overloads->overloads);
        if (arityGroups != overloads->overloads)
        {
            return Anew(a, OverloadGroup, overloads->id, arityGroups);
        }

        return overloads;
    }

    RtEXPR Visitor::VisitOverloadGroupConstructor(RtOVERLOADGROUPCONSTRUCTOR function) 
    {
        auto properties = function->properties ? PropertiesObject::From(VisitPropertiesObject(function->properties)) : nullptr;
        auto signature = VisitMethodSignature(function->signature); 

        if (properties != function->properties || signature != function->signature)
        {
            return Anew(a, OverloadGroupConstructor, OverloadedMethodSignature::From(signature), properties);
        }

        return function;
    }

    RtEXPR Visitor::VisitAbiMethod(const AbiMethod * function) 
    {
        auto properties = function->properties ? PropertiesObject::From(VisitPropertiesObject(function->properties)) : nullptr;
        auto signature = VisitMethodSignature(function->signature); 
        if (properties != function->properties || signature != function->signature)
        {
            return Anew(a, AbiMethod, AbiMethodSignature::From(signature), properties);
        }

        return function;
    }

    RtEXPR Visitor::VisitSystemGuidConstructor(const SystemGuidConstructor * function) 
    {
        auto signature = VisitMethodSignature(function->signature); 
        if (signature != function->signature)
        {
            return Anew(a, SystemGuidConstructor, function->typeId, TypeConstructorMethodSignature::From(signature));
        }

        return function;
    }

    RtEXPR Visitor::VisitWindowsFoundationDateTimeConstructor(const WindowsFoundationDateTimeConstructor * function)
    {
        auto signature = VisitMethodSignature(function->signature);
        if (signature != function->signature)
        {
            return Anew(a, WindowsFoundationDateTimeConstructor, function->typeId, TypeConstructorMethodSignature::From(signature));
        }

        return function;
    }

    RtEXPR Visitor::VisitWindowsFoundationTimeSpanConstructor(const WindowsFoundationTimeSpanConstructor * function)
    {
        auto signature = VisitMethodSignature(function->signature);
        if (signature != function->signature)
        {
            return Anew(a, WindowsFoundationTimeSpanConstructor, function->typeId, TypeConstructorMethodSignature::From(signature));
        }

        return function;
    }

    RtEXPR Visitor::VisitWindowsFoundationEventRegistrationTokenConstructor(const WindowsFoundationEventRegistrationTokenConstructor * function)
    {
        auto signature = VisitMethodSignature(function->signature);
        if (signature != function->signature)
        {
            return Anew(a, WindowsFoundationEventRegistrationTokenConstructor, function->typeId, TypeConstructorMethodSignature::From(signature));
        }

        return function;
    }

    RtEXPR Visitor::VisitWindowsFoundationHResultConstructor(const WindowsFoundationHResultConstructor * function)
    {
        auto signature = VisitMethodSignature(function->signature);
        if (signature != function->signature)
        {
            return Anew(a, WindowsFoundationHResultConstructor, function->typeId, TypeConstructorMethodSignature::From(signature));
        }

        return function;
    }

    RtEXPR Visitor::VisitBasicTypeConstructor(const BasicTypeConstructor * function) 
    {
        auto signature = VisitMethodSignature(function->signature); 
        if (signature != function->signature)
        {
            return Anew(a, BasicTypeConstructor, TypeConstructorMethodSignature::From(signature));
        }

        return function;
    }

    RtEXPR Visitor::VisitNativeTypeConstructor(const NativeTypeConstructor * function)
    {
        return function;
    }

    RtEXPR Visitor::VisitMissingTypeConstructor(const MissingTypeConstructor * function)
    {
        auto signature = VisitMethodSignature(function->signature);
        if (signature != function->signature)
        {
            return Anew(a, MissingTypeConstructor, function->typeId, MissingTypeConstructorMethodSignature::From(signature));
        }

        return function;
    }

    RtEXPR Visitor::VisitStructConstructor(RtSTRUCTCONSTRUCTOR function) 
    {
        auto structType = StructType::From(VisitIndividual(function->structType));
        auto properties = function->properties ? PropertiesObject::From(VisitPropertiesObject(function->properties)) : nullptr;
        auto signature = VisitMethodSignature(function->signature);

        if (structType != function->structType || properties != function->properties || signature != function->signature)
        {
#if JSGEN
            return Anew(a, StructConstructor, function->typeId, TypeConstructorMethodSignature::From(signature), properties, structType, function->supportedOnAttributes);
#else
            return Anew(a, StructConstructor, function->typeId, TypeConstructorMethodSignature::From(signature), properties, structType);
#endif	
        }

        return function;
    }

    RtEXPR Visitor::VisitEnum(RtENUM _enum)
    {
        auto properties = PropertiesObject::From(VisitPropertiesObject(_enum->properties));
        if (properties != _enum->properties)
        {	
#if JSGEN
            return Anew(a, Enum, _enum->typeId, _enum->typeDef, properties, _enum->baseTypeCode, _enum->supportedOnAttributes);
#else
            return Anew(a, Enum, _enum->typeId, _enum->typeDef, properties, _enum->baseTypeCode);
#endif		
        }

        return _enum;
    }

    RtRUNTIMEINTERFACECONSTRUCTOR Visitor::VisitRuntimeInterfaceConstructor(RtRUNTIMEINTERFACECONSTRUCTOR iface)
    {
        auto properties = iface->properties ? PropertiesObject::From(VisitPropertiesObject(iface->properties)) : nullptr;
        auto signature = VisitMethodSignature(iface->signature);
        auto prototype = VisitExpr(iface->prototype);
        auto ownProperties = VisitImmutableList(iface->ownProperties);
        auto requiredInterfaces = VisitImmutableList(iface->requiredInterfaces);
        if (properties != iface->properties || signature != iface->signature || 
            prototype != iface->prototype || ownProperties != iface->ownProperties || 
            requiredInterfaces != iface->requiredInterfaces)
        {
            return Anew(a, RuntimeInterfaceConstructor, 
                iface->typeId,
                iface->typeDef,
                TypeConstructorMethodSignature::From(signature), 
                iface->iid, iface->genericParameters, 
                properties,
                PropertiesObject::From(prototype), 
                ownProperties,
                requiredInterfaces, 
                iface->locatorNames, 
                iface->nameCount,
                iface->specialization,
                iface->hasEventHandlers);
        }
        return iface;
    }

    const MissingInstantiationConstructor * Visitor::VisitMissingInstantiationConstructor(const MissingInstantiationConstructor * iface)
    {
        auto signature = VisitMethodSignature(iface->signature);
        auto prototype = VisitExpr(iface->prototype);

        if (signature != iface->signature || prototype != iface->prototype)
        {
            return Anew(a, MissingInstantiationConstructor, iface->typeId, TypeConstructorMethodSignature::From(signature), PropertiesObject::From(prototype));
        }
        return iface;
    }

    const MissingInterfaceConstructor * Visitor::VisitMissingInterfaceConstructor(const MissingInterfaceConstructor * iface)
    {
        auto signature = VisitMethodSignature(iface->signature);
        auto prototype = VisitExpr(iface->prototype);
        if (signature != iface->signature || prototype != iface->prototype)
        {
            return Anew(a, MissingInterfaceConstructor, iface->typeId, signature, PropertiesObject::From(prototype));
        }
        return iface;
    }

    RtINTERFACECONSTRUCTOR Visitor::VisitIndividual(RtINTERFACECONSTRUCTOR constructor) 
    {
        switch(constructor->interfaceType)
        {
            case ifRuntimeInterfaceConstructor: return VisitRuntimeInterfaceConstructor(RuntimeInterfaceConstructor::From(constructor));
            case ifMissingInstantiationConstructor: return VisitMissingInstantiationConstructor(MissingInstantiationConstructor::From(constructor));
            case ifMissingInterfaceConstructor: return VisitMissingInterfaceConstructor(MissingInterfaceConstructor::From(constructor));
        }
        Js::Throw::FatalProjectionError();
    }

    RtEXPR Visitor::VisitGenericClassVarConstructor(const GenericClassVarConstructor * function)
    {
        auto signature = VisitMethodSignature(function->signature);
        if (signature != function->signature)
        {
            return Anew(a, GenericClassVarConstructor, TypeConstructorMethodSignature::From(signature));
        }
        return function;
    }

    RtEXPR Visitor::VisitArrayConstructor(const ArrayConstructor * function)
    {
        auto signature = VisitMethodSignature(function->signature);
        if (signature != function->signature)
        {
            return Anew(a, ArrayConstructor, TypeConstructorMethodSignature::From(signature));
        }
        return function;
    }

    RtEXPR Visitor::VisitVoidConstructor(const VoidConstructor * function) 
    {
        auto signature = VisitMethodSignature(function->signature);
        if (signature != function->signature)
        {
            return Anew(a, VoidConstructor, TypeConstructorMethodSignature::From(signature));
        }
        return function;
    }

    RtEXPR Visitor::VisitRuntimeClassConstructor(RtRUNTIMECLASSCONSTRUCTOR function) 
    {
        auto properties = function->properties ? PropertiesObject::From(VisitPropertiesObject(function->properties)) : nullptr;
        auto signature = VisitMethodSignature(function->signature);
        auto prototype = VisitExpr(function->prototype);
        if (properties != function->properties || signature != function->signature || prototype != function->prototype)
        {
            return Anew(a, RuntimeClassConstructor, function->typeId, function->typeDef, signature, properties, prototype, 
                function->defaultInterface, function->specialization, function->hasEventHandlers, function->gcPressure, 
                function->allInterfaces, function->staticInterfaces);
        }
        return function;
    }

    RtEXPR Visitor::VisitDelegateConstructor(RtDELEGATECONSTRUCTOR function) 
    {
        auto signature = VisitMethodSignature(function->signature);
        auto invokeInterface = VisitIndividual(function->invokeInterface);
        if (signature != function->signature || invokeInterface != function->invokeInterface)
        {
            return Anew(a, DelegateConstructor, function->typeId, function->simpleName, signature, InterfaceConstructor::From(invokeInterface));
        }
        return function;
    }

    RtEXPR Visitor::VisitFunction(RtFUNCTION function)
    {
        switch(function->functionType)
        {
        case functionOverloadGroupConstructor: return VisitOverloadGroupConstructor(OverloadGroupConstructor::From(function));
        case functionAbiMethod: return VisitAbiMethod(AbiMethod::From(function));
        case functionSystemGuidConstructor: return VisitSystemGuidConstructor(SystemGuidConstructor::From(function));
        case functionWindowsFoundationDateTimeConstructor: return VisitWindowsFoundationDateTimeConstructor(WindowsFoundationDateTimeConstructor::From(function)); 
        case functionWindowsFoundationTimeSpanConstructor: return VisitWindowsFoundationTimeSpanConstructor(WindowsFoundationTimeSpanConstructor::From(function)); 
        case functionWindowsFoundationEventRegistrationTokenConstructor: return VisitWindowsFoundationEventRegistrationTokenConstructor(WindowsFoundationEventRegistrationTokenConstructor::From(function)); 
        case functionWindowsFoundationHResultConstructor: return VisitWindowsFoundationHResultConstructor(WindowsFoundationHResultConstructor::From(function)); 
        case functionBasicTypeConstructor: return VisitBasicTypeConstructor(BasicTypeConstructor::From(function));
        case functionNativeTypeConstructor: return VisitNativeTypeConstructor(NativeTypeConstructor::From(function));
        case functionMissingTypeConstructor: return VisitMissingTypeConstructor(MissingTypeConstructor::From(function));
        case functionStructConstructor: return VisitStructConstructor(StructConstructor::From(function));
        case functionInterfaceConstructor: return VisitIndividual(InterfaceConstructor::From(function));
        case functionGenericClassVarConstructor: return VisitGenericClassVarConstructor(GenericClassVarConstructor::From(function));
        case functionArrayConstructor: return VisitArrayConstructor(ArrayConstructor::From(function));
        case functionVoidConstructor: return VisitVoidConstructor(VoidConstructor::From(function));
        case functionRuntimeClassConstructor: return VisitRuntimeClassConstructor(RuntimeClassConstructor::From(function));
        case functionDelegateConstructor: return VisitDelegateConstructor(DelegateConstructor::From(function));
        }
        Js::Throw::FatalProjectionError();
    }

    RtEXPR Visitor::VisitNullLiteral(RtEXPR expr) 
    {
        return expr;
    }

    RtEXPR Visitor::VisitInt32lLiteral(const Int32Literal * expr) 
    {
        return expr;
    }

    RtEXPR Visitor::VisitUInt32lLiteral(const UInt32Literal * expr)
    {
        return expr;
    }

    RtEXPR Visitor::VisitModOptExpr(const ModOptExpr * expr)
    {
        auto modded = VisitExpr(expr->modded);
        if (modded != expr->modded)
        {
            return Anew(a, ModOptExpr, modded, expr->nativeModOpt);
        }
        return expr;
    }

    RtEXPR Visitor::VisitByRefExpr(const ByRefExpr * expr)
    {
        auto pointedTo = VisitExpr(expr->pointedTo);
        if (pointedTo != expr->pointedTo)
        {
            return Anew(a, ByRefExpr, pointedTo, expr->nativeByRef);
        }
        return expr;
    }

    RtEXPR Visitor::VisitExpr(RtEXPR expr)
    {
        switch(expr->type)
        {
            case exprNullLiteral: return VisitNullLiteral(expr);
            case exprInt32Literal: return VisitInt32lLiteral(Int32Literal::From(expr));
            case exprUInt32Literal: return VisitUInt32lLiteral(UInt32Literal::From(expr));
            case exprPropertiesObject: return VisitPropertiesObject(PropertiesObject::From(expr));
            case exprAssignmentSpace: return VisitAssignmentSpace(AssignmentSpace::From(expr));
            case exprFunction: return VisitFunction(Function::From(expr));
            case exprModOptExpr: return VisitModOptExpr(ModOptExpr::From(expr));
            case exprByRefExpr: return VisitByRefExpr(ByRefExpr::From(expr));
            case exprEnum: return VisitEnum(Enum::From(expr));

        }
        Js::Throw::FatalProjectionError();
    }
}