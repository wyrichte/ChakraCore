//---------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation.  All rights reserved.
//---------------------------------------------------------------------------
#include "stdafx.h"
#include <metahost.h>
#include <sdkddkver.h>
#include "tswrapper.h"

#ifdef NTDDI_VERSION
#undef NTDDI_VERSION
#endif
#define NTDDI_VERSION NTDDI_WIN8
#include <rometadata.h>

// Concatenate an element's name in the xml format to the given path and returns it
LPCWSTR TSWrapper::Documentation::ConcatElementName(_In_ LPCWSTR path, _In_ Element* element, _Inout_ ArenaAllocator* alloc)
{
    LPCWSTR elementName = element->GetNameInXmlFormat(alloc);
    
    if (wcslen(path) != 0)
    {
        elementName = StringUtils::Concat(L".", elementName, alloc);
    }

    return StringUtils::Concat(path, elementName, alloc);
}

// Creates an enum object and populates it with the given enum metadata information using Projection Model
TSWrapper::Enum* TSWrapper::TSObjectModel::ExploreEnum(_In_ RtENUM exploredEnum, _In_ LPCWSTR enumName)
{
    Enum* resultingEnum = Anew(alloc, Enum);
    resultingEnum->name = enumName;
    resultingEnum->metadataName = builder->stringConverter->StringOfId(exploredEnum->typeDef->id);
    resultingEnum->alloc = alloc;

    AnalyzeElementAttributes(resultingEnum, exploredEnum->typeDef);

    exploredEnum->properties->fields->Iterate(
        [&](RtPROPERTY prop)
        {
            ProjectionModel::RtABIFIELDPROPERTY abiFieldProperty = ProjectionModel::AbiFieldProperty::From(prop);
            EnumEntry* entry = Anew(alloc, EnumEntry);
            entry->name = builder->stringConverter->StringOfId(abiFieldProperty->fieldProperties->id);
            resultingEnum->AddEntry(entry, alloc);
        });

    resultingEnum->entries = resultingEnum->entries->Reverse(alloc);
   
    return resultingEnum;
}

// Creates a type object and populates it with the given type metadata information using Projection Model 
TSWrapper::Type* TSWrapper::TSObjectModel::ExploreType(_In_ RtTYPE exploredType)
{
    Type* resultingType = Anew(alloc, TSWrapper::Type);
    resultingType->alloc = alloc;
    LPCWSTR originalName = builder->stringConverter->StringOfId(exploredType->fullTypeNameId);

    if (ProjectionModel::TypeDefinitionType::Is(exploredType))
    {
        RtTYPEDEFINITIONTYPE typeDefinitionType = ProjectionModel::TypeDefinitionType::From(exploredType);
        LPCWSTR typeName = builder->stringConverter->StringOfId(typeDefinitionType->typeDef->id);

        if (wcscmp(typeName, L"Windows.Foundation.PropertyValue") == 0 || wcscmp(typeName, L"Windows.Foundation.IPropertyValue") == 0)
        {
            resultingType = TSWrapper::Type::GetBasicType(L"any", originalName, alloc);
        }
        else if (wcscmp(typeName, L"Windows.Foundation.IReference") == 0)
        {
            resultingType = ExploreType(typeDefinitionType->genericParameters->First());
            resultingType->isRefType = true;
        }
        else if (wcscmp(typeName, L"Windows.Foundation.IReferenceArray`1") == 0)
        {
        }
        else if (typeDefinitionType->genericParameters)
        {
            LPCWSTR genericTypeName = StringUtils::RemoveGenericsGraveAccent(typeName, alloc);
            resultingType = TSWrapper::Type::GetBasicType(genericTypeName, genericTypeName, alloc);
            typeDefinitionType->genericParameters->Reverse(alloc)->Iterate(
                [&](RtTYPE type)
                {
                    resultingType->AddSubType(ExploreType(type), alloc);
                });
        }
        else
        {
            resultingType = TSWrapper::Type::GetBasicType(typeName, originalName, alloc);
        }
    }
    else
    {
        switch (exploredType->typeCode)
        {
        case ProjectionModel::TypeCode::tcArrayType:
        {
            ProjectionModel::RtARRAYTYPE innerArrayType = ProjectionModel::ArrayType::From(exploredType);
            resultingType = ExploreType(innerArrayType->elementType);
            resultingType->arrayDepth++;
            break;
        }
        case ProjectionModel::TypeCode::tcBasicType:
        {
            ProjectionModel::RtBASICTYPE innerBasicType = ProjectionModel::BasicType::From(exploredType);
            LPCWSTR basicTypeName = StringUtils::Concat(L"System.", originalName, alloc);

            switch (innerBasicType->typeCor)
            {
            case CorElementType::ELEMENT_TYPE_STRING:
            {
                resultingType = TSWrapper::Type::GetBasicType(L"string", basicTypeName, alloc);
                break;
            }
            case CorElementType::ELEMENT_TYPE_BOOLEAN:
            {
                resultingType = TSWrapper::Type::GetBasicType(L"boolean", basicTypeName, alloc);
                break;
            }
            case CorElementType::ELEMENT_TYPE_CHAR:
            {
                basicTypeName = L"System.Char";
                resultingType = TSWrapper::Type::GetBasicType(L"any", basicTypeName, alloc);
                break;
            }
            case CorElementType::ELEMENT_TYPE_I1:
            case CorElementType::ELEMENT_TYPE_U1:
            case CorElementType::ELEMENT_TYPE_I2:
            case CorElementType::ELEMENT_TYPE_U2:
            case CorElementType::ELEMENT_TYPE_I4:
            case CorElementType::ELEMENT_TYPE_U4:
            case CorElementType::ELEMENT_TYPE_I8:
            case CorElementType::ELEMENT_TYPE_U8:
            case CorElementType::ELEMENT_TYPE_R4:
            case CorElementType::ELEMENT_TYPE_R8:
            {
                if (wcscmp(basicTypeName, L"System.UInt8") == 0)
                {
                    basicTypeName = L"System.Byte";
                }
                resultingType = TSWrapper::Type::GetBasicType(L"number", basicTypeName, alloc);
                break;
            }
            case CorElementType::ELEMENT_TYPE_VOID:
            {
                resultingType = TSWrapper::Type::GetBasicType(L"void", basicTypeName, alloc);
                break;
            }
            default:
            {
                resultingType = TSWrapper::Type::GetBasicType(L"any", basicTypeName, alloc);
            }
            }
            break;
        }
        case ProjectionModel::TypeCode::tcByRefType:
        {
            ProjectionModel::RtBYREFTYPE innerRefType = ProjectionModel::ByRefType::From(exploredType);
            resultingType = ExploreType(innerRefType->pointedTo);
            resultingType->isRefType = true;
            break;
        }
        case ProjectionModel::TypeCode::tcSystemGuidType:
        {
            resultingType = TSWrapper::Type::GetBasicType(L"string", StringUtils::Concat(L"System.", originalName, alloc), alloc);
            break;
        }
        case ProjectionModel::TypeCode::tcWindowsFoundationDateTimeType:
        {
            resultingType = TSWrapper::Type::GetBasicType(L"Date", originalName, alloc);
            break;
        }
        case ProjectionModel::TypeCode::tcHResult:
        {
            resultingType = TSWrapper::Type::GetBasicType(L"number", originalName, alloc);
            break;
        }
        case ProjectionModel::TypeCode::tcVoidType:
        {
            resultingType = TSWrapper::Type::GetBasicType(L"void", originalName, alloc);
            break;
        }
        case ProjectionModel::TypeCode::tcMissingNamedType:
        case ProjectionModel::TypeCode::tcClassType:
        case ProjectionModel::TypeCode::tcEnumType:
        case ProjectionModel::TypeCode::tcStructType:
        case ProjectionModel::TypeCode::tcInterfaceType:
        {
            resultingType = TSWrapper::Type::GetBasicType(builder->stringConverter->StringOfId(exploredType->fullTypeNameId), originalName, alloc);
            break;
        }
        case ProjectionModel::TypeCode::tcGenericParameterType:
        {
            resultingType = TSWrapper::Type::GetBasicType(originalName, originalName, alloc);
            resultingType->isGenericParameterType = true;
            
            break;
        }
        case ProjectionModel::TypeCode::tcGenericClassVarType:
        {
            resultingType = TSWrapper::Type::GetBasicType(originalName, originalName, alloc);

            auto genericClassVarType = ProjectionModel::GenericClassVarType::From(exploredType);
            resultingType->genericParameterIndex = genericClassVarType->var->index;
            break;
        }
        default:
        {
            resultingType = TSWrapper::Type::GetBasicType(L"any", originalName, alloc);
        }
        }
    }

    return resultingType;
}

// Creates a function object and populates it with the given parameters and return type metadata information using Projection Model
TSWrapper::Function* TSWrapper::TSObjectModel::ExploreFunctionParameters(_In_ ImmutableList<RtPARAMETER>* parameters, _In_ RtTYPE returnType, _In_ LPCWSTR functionName, const int retValIndex)
{
    TSWrapper::Function* resultingFunction = Anew(alloc, Function);
    resultingFunction->name = functionName;
    resultingFunction->metadataName = functionName;
    resultingFunction->alloc = alloc;

    int outParametersCount = 0;

    int index = (int)parameters->Count() - retValIndex - 1;
    int genericsCount = 0;

    parameters->Reverse(alloc)->Iterate(
        [&](RtPARAMETER parameter)
        {
            if (parameter->isOut)
            {
                outParametersCount++;
            }
            TSWrapper::Field* functionParameter = Anew(alloc, Field);
            LPCWSTR parameterName = builder->stringConverter->StringOfId(parameter->id);
            functionParameter->name = parameterName;
            functionParameter->type = ExploreType(parameter->type);
            functionParameter->type->isInOnlyParameter = parameter->isIn && !parameter->isOut;
            functionParameter->type->isOutOnlyParameter = parameter->isOut && !parameter->isIn;
            functionParameter->type->isInOutParameter = parameter->isOut && parameter->isIn;
            functionParameter->hasRetValAttribute = index == 0;
            if (functionParameter->type->isGenericParameterType)
            {
                functionParameter->type->genericParameterIndex = genericsCount;
                genericsCount++;
            }

            resultingFunction->AddParameter(functionParameter, alloc);

            index--;
        });

    if (returnType)
    {
        ImmutableList<RtPARAMETER>* outParameters = parameters->Where(
            [&](RtPARAMETER parameter)
            {
                return parameter->isOut;
            },
            alloc)->Reverse(alloc);

        switch (outParameters->Count())
        {
        case 0:
        {
            resultingFunction->returnType = TSWrapper::Type::GetBasicType(L"void", L"Void", alloc);
            break;
        }
        case 1:
        {
            resultingFunction->returnType = ExploreType(returnType);
            break;
        }
        default:
        {
            Interface* outputInterface = Anew(alloc, Interface);

            outParameters->Iterate(
                [&](RtPARAMETER innerParameter)
                {
                    Field* field = Anew(alloc, Field);
                    field->name = builder->stringConverter->StringOfId(innerParameter->id);
                    field->type = ExploreType(innerParameter->type);
                    field->type->isInOutParameter = innerParameter->isIn;

                    outputInterface->AddField(field, alloc);
                });

            resultingFunction->returnType = TSWrapper::Type::GetObjectType(outputInterface, alloc);
            break;
        }
        }
    }

    return resultingFunction;
}

// Creates a function object and populates it with the given method signature metadata information using Projection Model
TSWrapper::Function* TSWrapper::TSObjectModel::ExploreFunction(_In_ RtABIMETHODSIGNATURE exploredFunctionSignature, _In_ LPCWSTR functionName)
{
    auto retValIndex = -1;
    if (exploredFunctionSignature->continuation && exploredFunctionSignature->continuation->returnMetadataType)
    {
        retValIndex = (int)exploredFunctionSignature->continuation->parameterProperties->Count();
    }

    auto parameters = exploredFunctionSignature->GetParameters();
    Function* resultingFunction = ExploreFunctionParameters(parameters->allParameters, parameters->returnType, functionName, retValIndex);
    resultingFunction->metadataName = builder->stringConverter->StringOfId(exploredFunctionSignature->metadataNameId);
    return resultingFunction;
}

// Creates a list of overload functions objects and populates it with the given overload functions metadata information using Projection Model
ImmutableList<TSWrapper::Function*>* TSWrapper::TSObjectModel::ExploreOverloadFunctions(_In_ RtFUNCTION exploredOverloadFunctions)
{
    ImmutableList<Function*>* result = nullptr;
    auto overloadConstructor = ProjectionModel::OverloadGroupConstructor::From(exploredOverloadFunctions);
    overloadConstructor->signature->overloads->overloads->Reverse(alloc)->Iterate(
        [&](RtABIMETHODSIGNATURE overloadFunction)
        {
            LPCWSTR overloadFunctionName = builder->stringConverter->StringOfId(overloadFunction->nameId);
            result = result->Prepend(ExploreFunction(overloadFunction, overloadFunctionName), alloc);
        });

    return result;
}

// Creates a function object and populates it with the given function property metadata information using Projection Model
TSWrapper::Function* TSWrapper::TSObjectModel::ExploreFunctionProperty(_In_ RtPROPERTY functionProperty, _In_ LPCWSTR functionName)
{
    ProjectionModel::RtABIMETHODPROPERTY method = ProjectionModel::AbiMethodProperty::From(functionProperty);
    RtFUNCTION function = ProjectionModel::Function::From(method->expr);
    RtABIMETHOD abiMethod = ProjectionModel::AbiMethod::From(function);
    return ExploreFunction(abiMethod->signature, functionName);
}

// Populate the element's attribute information given the type definition metadata information of the element using Projection Model
void TSWrapper::TSObjectModel::AnalyzeElementAttributes(_Inout_ Element* element, _In_ const Metadata::TypeDefProperties* typeDef)
{
    element->hasWebHostHiddenAttribute = false;
    element->hasExclusiveToAttribute = false;

    auto attributes = typeDef->assembly.CustomAttributes(typeDef->td, 0);
    attributes->Iterate(
        [&](const Metadata::CustomAttributeProperties* attr)
        {
            LPCWSTR attributeName = builder->stringConverter->StringOfId(attr->attributeTypeId);

            if (wcscmp(attributeName, L"Windows.Foundation.Metadata.ExclusiveToAttribute") == 0)
            {
                element->hasExclusiveToAttribute = true;
            }
            if (wcscmp(attributeName, L"Windows.Foundation.Metadata.WebHostHiddenAttribute") == 0)
            {
                element->hasWebHostHiddenAttribute = true;
            }
            if (wcscmp(attributeName, L"Windows.Foundation.Metadata.OverloadAttribute") == 0)
            {
                element->hasOverloadAttribute = true;
            }
        });
}

// Creates an interface object and populates it with the given runtime interface constructor metadata information using Projection Model
TSWrapper::Interface* TSWrapper::TSObjectModel::ExploreRuntimeInterface(_In_ RtRUNTIMEINTERFACECONSTRUCTOR exploredInterface, _In_ LPCWSTR interfaceName)
{
    Interface* resultingInterface = Anew(alloc, Interface);
    resultingInterface->name = StringUtils::RemoveGenericsGraveAccent(interfaceName, alloc);
    resultingInterface->metadataName = builder->stringConverter->StringOfId(exploredInterface->typeDef->id);
    resultingInterface->alloc = alloc;

    AnalyzeElementAttributes(resultingInterface, exploredInterface->typeDef);
    
    exploredInterface->genericParameters->Reverse(alloc)->Iterate(
        [&](_In_ RtTYPE genericParameter){
            auto genericParameterType = ProjectionModel::GenericParameterType::From(genericParameter);
            resultingInterface->AddGenericParameter(builder->stringConverter->StringOfId(genericParameterType->fullTypeNameId), alloc);
        });

    /// explore constructors
    RtMETHODSIGNATURE methodSignature = exploredInterface->signature;
    switch (methodSignature->signatureType)
    {
    case ProjectionModel::MethodSignatureType::mstAbiMethodSignature:
    {
        resultingInterface->AddFunction(ExploreRuntimeClassConstructor(methodSignature), alloc);
        break;
    }
    case ProjectionModel::MethodSignatureType::mstOverloadedMethodSignature:
    {
        RtOVERLOADEDMETHODSIGNATURE overloadSignature = ProjectionModel::OverloadedMethodSignature::From(methodSignature);
        overloadSignature->overloads->overloads->Reverse(alloc)->Iterate(
            [&](_In_ RtABIMETHODSIGNATURE overloadedConstructor)
            {
                resultingInterface->AddFunction(ExploreRuntimeClassConstructor(overloadedConstructor), alloc);
            });
        break;
    }
    }

    /// explore fields, functions and events
    auto prototype = exploredInterface->prototype;
    auto prototypeBody = ProjectionModel::PropertiesObject::From(prototype);
    prototypeBody->fields->Reverse(alloc)->Iterate(
        [&](_In_ RtPROPERTY property)
        {
            LPCWSTR fieldName = builder->stringConverter->StringOfId(property->identifier);
            switch (property->propertyType)
            {
            case ProjectionModel::PropertyType::ptAbiPropertyProperty:
            {
                ProjectionModel::RtABIPROPERTYPROPERTY innerField = ProjectionModel::AbiPropertyProperty::From(property);
                resultingInterface->AddField(ExploreField(innerField, fieldName), alloc);
                break;
            }
            case ProjectionModel::PropertyType::ptAbiMethodProperty:
            {
                ProjectionModel::RtABIMETHODPROPERTY innerFunction = ProjectionModel::AbiMethodProperty::From(property);
                LPCWSTR innerFunctionName = builder->stringConverter->StringOfId(innerFunction->identifier);
                resultingInterface->AddFunction(ExploreFunctionProperty(property, innerFunctionName), alloc);
                break;
            }
            case ProjectionModel::PropertyType::ptOverloadParentProperty:
            {
                RtFUNCTION overloadFunctions = ProjectionModel::Function::From(property->expr);
                resultingInterface->AddFunctions(ExploreOverloadFunctions(overloadFunctions), alloc);
                break;
            }
            case ProjectionModel::PropertyType::ptAbiEventHandlerProperty:
            {
                auto eventHandler = ProjectionModel::AbiEventHandlerProperty::From(property);
                resultingInterface->AddEvent(ExploreEvent(eventHandler->abiEvent), alloc);
                break;
            }
            }
        });

    /// explore static fields and static functions
    auto statics = ProjectionModel::PropertiesObject::From(exploredInterface->properties);
    statics->fields->Reverse(alloc)->Iterate(
        [&](_In_ RtPROPERTY property)
        {
            LPCWSTR fieldName = builder->stringConverter->StringOfId(property->identifier);
            switch (property->propertyType)
            {
            case ProjectionModel::PropertyType::ptAbiPropertyProperty:
            {
                ProjectionModel::RtABIPROPERTYPROPERTY innerField = ProjectionModel::AbiPropertyProperty::From(property);
                resultingInterface->AddStaticField(ExploreField(innerField, fieldName), alloc);
                break;
            }
            case ProjectionModel::PropertyType::ptAbiMethodProperty:
            {
                resultingInterface->AddStaticFunction(ExploreFunctionProperty(property, fieldName), alloc);
                break;
            }
            case ProjectionModel::PropertyType::ptOverloadParentProperty:
            {
                RtFUNCTION overloadFunctions = ProjectionModel::Function::From(property->expr);
                resultingInterface->AddStaticFunctions(ExploreOverloadFunctions(overloadFunctions), alloc);
                break;
            }
            case ProjectionModel::PropertyType::ptAbiEventHandlerProperty:
            {
                auto eventHandler = ProjectionModel::AbiEventHandlerProperty::From(property);
                resultingInterface->AddStaticEvent(ExploreEvent(eventHandler->abiEvent), alloc);
                break;
            }
            }
        });

    bool implementsIIterable = false;

    exploredInterface->requiredInterfaces->Iterate(
        [&](_In_ RtINTERFACECONSTRUCTOR innerInterface)
        {
            RtTYPE interfaceType = innerInterface->signature->GetParameters()->returnType;
            LPCWSTR implementedInterfaceName = builder->stringConverter->StringOfId(interfaceType->fullTypeNameId);
            
            if (StringUtils::StartsWith(implementedInterfaceName, L"Windows.Foundation.Collections.IIterable`1"))
            {
                implementsIIterable = true;
            }

            resultingInterface->AddImplementedInterface(StringUtils::RemoveGenericsGraveAccent(implementedInterfaceName, alloc), alloc);
        });

    if (implementsIIterable || wcscmp(resultingInterface->name, L"IIterable") == 0)
    {
        resultingInterface->fields->Iterate(
            [&](_In_ Field* innerField)
            {
                innerField->isOptional = true;
            });

        resultingInterface->staticFields->Iterate(
            [&](_In_ Field* innerStaticField)
            {
                innerStaticField->isOptional = true;
            });

        resultingInterface->functions->Iterate(
            [&](_In_ Function* innerFunction)
            {
                innerFunction->isOptional = true;
            });

        resultingInterface->staticFunctions->Iterate(
            [&](_In_ Function* innerStaticFunction)
            {
                innerStaticFunction->isOptional = true;
            });
    }

    return resultingInterface;
}

// Creates an interface object and populates it with the given interface constructor metadata information using Projection Model
TSWrapper::Interface* TSWrapper::TSObjectModel::ExploreInterface(_In_ const ProjectionModel::InterfaceConstructor* exploredInterface, _In_ LPCWSTR interfaceName)
{
    Interface* resultingInterface = Anew(alloc, Interface);
    resultingInterface->name = StringUtils::RemoveGenericsGraveAccent(interfaceName, alloc);
    resultingInterface->metadataName = builder->stringConverter->StringOfId(exploredInterface->typeDef->id);
    resultingInterface->alloc = alloc;

    AnalyzeElementAttributes(resultingInterface, exploredInterface->typeDef);

    exploredInterface->requiredInterfaces->Iterate(
        [&](_In_ RtINTERFACECONSTRUCTOR innerInterface)
        {
            RtTYPE interfaceType = innerInterface->signature->GetParameters()->returnType;
            LPCWSTR implementedInterfaceName = builder->stringConverter->StringOfId(interfaceType->fullTypeNameId);
            resultingInterface->AddImplementedInterface(StringUtils::RemoveGenericsGraveAccent(implementedInterfaceName, alloc), alloc);
        });

    if (exploredInterface->typeDef && exploredInterface->typeDef->genericParameterTokens)
    {
        auto genericParameters = exploredInterface->typeDef->assembly.GenericParameters(exploredInterface->typeDef->genericParameterTokens);

        genericParameters->Reverse(alloc)->Iterate(
            [&](const Metadata::GenericParameterProperties* genericParameter)
            {
                LPCWSTR genericParameterName = builder->stringConverter->StringOfId(genericParameter->id);
                resultingInterface->AddGenericParameter(genericParameterName, alloc);
            });
    }

    RtMETHODSIGNATURE methodSignature = exploredInterface->signature;
    
    switch (methodSignature->signatureType)
    {
    case ProjectionModel::MethodSignatureType::mstAbiMethodSignature:
    {
        resultingInterface->AddFunction(ExploreRuntimeClassConstructor(methodSignature), alloc);
        break;
    }
    case ProjectionModel::MethodSignatureType::mstOverloadedMethodSignature:
    {
        RtOVERLOADEDMETHODSIGNATURE overloadSignature = ProjectionModel::OverloadedMethodSignature::From(methodSignature);
        overloadSignature->overloads->overloads->Reverse(alloc)->Iterate(
            [&](_In_ RtABIMETHODSIGNATURE overloadedConstructor)
        {
            resultingInterface->AddFunction(ExploreRuntimeClassConstructor(overloadedConstructor), alloc);
        });
        break;
    }
    }

    auto prototype = exploredInterface->prototype;
    auto prototypeBody = ProjectionModel::PropertiesObject::From(prototype);
    
    prototypeBody->fields->Reverse(alloc)->Iterate(
        [&](_In_ RtPROPERTY property)
        {
            LPCWSTR fieldName = builder->stringConverter->StringOfId(property->identifier);
            switch (property->propertyType)
            {
            case ProjectionModel::PropertyType::ptAbiPropertyProperty:
            {
                ProjectionModel::RtABIPROPERTYPROPERTY innerField = ProjectionModel::AbiPropertyProperty::From(property);
                resultingInterface->AddField(ExploreField(innerField, fieldName), alloc);
                break;
            }
            case ProjectionModel::PropertyType::ptAbiMethodProperty:
            {
                resultingInterface->AddFunction(ExploreFunctionProperty(property, fieldName), alloc);
                break;
            }
            case ProjectionModel::PropertyType::ptAbiTypeProperty:
            {
                const RtDELEGATECONSTRUCTOR innerDelegate = ProjectionModel::DelegateConstructor::From(property->expr);
                resultingInterface->AddDelegate(ExploreDelegate(innerDelegate, fieldName), alloc);
                break;
            }
            case ProjectionModel::PropertyType::ptOverloadParentProperty:
            {
                RtFUNCTION overloadFunctions = ProjectionModel::Function::From(property->expr);
                resultingInterface->AddFunctions(ExploreOverloadFunctions(overloadFunctions), alloc);
                break;
            }
            case ProjectionModel::PropertyType::ptAbiEventHandlerProperty:
            {
                auto eventHandler = ProjectionModel::AbiEventHandlerProperty::From(property);
                resultingInterface->AddEvent(ExploreEvent(eventHandler->abiEvent), alloc);
                break;
            }
            }
        });

    if (exploredInterface->interfaceType == ProjectionModel::InterfaceConstructorType::ifRuntimeInterfaceConstructor)
    {
        resultingInterface->isHandler = true;
    }

    return resultingInterface;
}

// Creates a field object and populates it with the given abi property property metadata information using Projection Model
TSWrapper::Field* TSWrapper::TSObjectModel::ExploreField(_In_ ProjectionModel::RtABIPROPERTYPROPERTY prop, _In_ LPCWSTR fieldName)
{
    Field* resultingField = Anew(alloc, Field);
    resultingField->name = fieldName;
    resultingField->type = ExploreType(prop->GetPropertyType());
    resultingField->metadataName = builder->stringConverter->StringOfId(prop->metadataNameId);
    resultingField->alloc = alloc;

    return resultingField;
} 

// Creates a function object and populates it with the given method signature metadata information using Projection Model
TSWrapper::Function* TSWrapper::TSObjectModel::ExploreRuntimeClassConstructor(_In_ RtMETHODSIGNATURE methodSignature)
{
    Function* classConstructor = ExploreFunctionParameters(methodSignature->GetParameters()->allParameters, nullptr, L"constructor", /*retValIndex = */-1);
    classConstructor->isConstructor = true;
    return classConstructor;
}

// Creates a class object and populates it with the given runtime class constructor metadata information using Projection Model
TSWrapper::Class* TSWrapper::TSObjectModel::ExploreRuntimeClass(_In_ RtRUNTIMECLASSCONSTRUCTOR exploredClass, _In_ LPCWSTR runtimeClassName)
{
    Class* resultingClass = Anew(alloc, Class);
    resultingClass->name = runtimeClassName;
    resultingClass->alloc = alloc;

    AnalyzeElementAttributes(resultingClass, exploredClass->typeDef);

    /// explore constructors
    RtMETHODSIGNATURE methodSignature = exploredClass->signature;
    switch (methodSignature->signatureType)
    {
    case ProjectionModel::MethodSignatureType::mstAbiMethodSignature:
    {
        Function* constructor = ExploreRuntimeClassConstructor(methodSignature);
        if (constructor)
        {
            resultingClass->AddFunction(constructor, alloc);
        }
        break;
    }
    case ProjectionModel::MethodSignatureType::mstOverloadedMethodSignature:
    {
        RtOVERLOADEDMETHODSIGNATURE overloadSignature = ProjectionModel::OverloadedMethodSignature::From(methodSignature);
        overloadSignature->overloads->overloads->Reverse(alloc)->Iterate(
            [&](_In_ RtABIMETHODSIGNATURE overloadedConstructor)
            {
                Function* constructor = ExploreRuntimeClassConstructor(overloadedConstructor);
                if (constructor)
                {
                    resultingClass->AddFunction(constructor, alloc);
                }
            }
            );
        break;
    }
    case ProjectionModel::MethodSignatureType::mstUncallableMethodSignature:
    {
        Function* uncallableConstructor = Anew(alloc, Function);
        uncallableConstructor->name = L"constructor";
        uncallableConstructor->metadataName = L"constructor";
        uncallableConstructor->isConstructor = true;
        resultingClass->AddFunction(uncallableConstructor, alloc);
    }
    }

    /// explore fields, functions and overloads
    auto prototype = exploredClass->prototype;
    auto prototypeBody = ProjectionModel::PropertiesObject::From(prototype);
    prototypeBody->fields->Reverse(alloc)->Iterate(
        [&](_In_ RtPROPERTY property)
        {
            LPCWSTR fieldName = builder->stringConverter->StringOfId(property->identifier);
            switch (property->propertyType)
            {
            case ProjectionModel::PropertyType::ptAbiPropertyProperty:
            {
                ProjectionModel::RtABIPROPERTYPROPERTY innerField = ProjectionModel::AbiPropertyProperty::From(property);
                resultingClass->AddField(ExploreField(innerField, fieldName), alloc);
                break;
            }
            case ProjectionModel::PropertyType::ptAbiMethodProperty:
            {
                resultingClass->AddFunction(ExploreFunctionProperty(property, fieldName), alloc);
                break;
            }
            case ProjectionModel::PropertyType::ptOverloadParentProperty:
            {
                RtFUNCTION innerFunction = ProjectionModel::Function::From(property->expr);
                resultingClass->AddFunctions(ExploreOverloadFunctions(innerFunction), alloc);
                break;
            }
            case ProjectionModel::PropertyType::ptAbiEventHandlerProperty:
            {
                auto eventHandler = ProjectionModel::AbiEventHandlerProperty::From(property);
                resultingClass->AddEvent(ExploreEvent(eventHandler->abiEvent), alloc);
                break;
            }
            }
        });

    /// explore static fields, static functions and overloads
    auto statics = ProjectionModel::PropertiesObject::From(exploredClass->properties);
    statics->fields->Reverse(alloc)->Iterate(
        [&](_In_ RtPROPERTY property)
        {
            LPCWSTR fieldName = builder->stringConverter->StringOfId(property->identifier);

            switch (property->propertyType)
            {
            case ProjectionModel::PropertyType::ptAbiPropertyProperty:
            {
                ProjectionModel::RtABIPROPERTYPROPERTY innerField = ProjectionModel::AbiPropertyProperty::From(property);
                resultingClass->AddStaticField(ExploreField(innerField, fieldName), alloc);
                break;
            }
            case ProjectionModel::PropertyType::ptAbiMethodProperty:
            {
                Function* innerFunction = ExploreFunctionProperty(property, fieldName);
                resultingClass->AddStaticFunction(innerFunction, alloc);
                break;
            }
            case ProjectionModel::PropertyType::ptOverloadParentProperty:
            {
                RtFUNCTION overloadFunctions = ProjectionModel::Function::From(property->expr);
                resultingClass->AddStaticFunctions(ExploreOverloadFunctions(overloadFunctions), alloc);
                break;
            }
            case ProjectionModel::PropertyType::ptAbiEventHandlerProperty:
            {
                auto eventHandler = ProjectionModel::AbiEventHandlerProperty::From(property);
                resultingClass->AddStaticEvent(ExploreEvent(eventHandler->abiEvent), alloc);
                break;
            }
            }
        });

    bool implementsIIterable = false;

    /// explore implements
    exploredClass->allInterfaces->Reverse(alloc)->Iterate(
        [&](_In_ RtINTERFACECONSTRUCTOR innerInterface)
        {
            if (innerInterface->interfaceType == ProjectionModel::InterfaceConstructorType::ifRuntimeInterfaceConstructor)
            {
                RtTYPE interfaceType = innerInterface->signature->GetParameters()->returnType;
                LPCWSTR implementedInterfaceName = builder->stringConverter->StringOfId(interfaceType->fullTypeNameId);

                if (StringUtils::StartsWith(implementedInterfaceName, L"Windows.Foundation.Collections.IIterable`1"))
                {
                    implementsIIterable = true;
                }

                Interface* implementedInterface = Anew(alloc, Interface);
                implementedInterface->name = StringUtils::AddSpaceAfterComma(StringUtils::RemoveGenericsGraveAccent(implementedInterfaceName, alloc), alloc);
                implementedInterface->metadataName = implementedInterface->name;
                AnalyzeElementAttributes(implementedInterface, innerInterface->typeDef);

                if (!implementedInterface->hasExclusiveToAttribute)
                {
                    resultingClass->AddImplementedInterface(implementedInterface, alloc);
                }
            }
        });

    if (implementsIIterable)
    {
        resultingClass->fields->Iterate(
            [&](_In_ Field* innerField)
            {
                innerField->isOptional = true;
            });

        resultingClass->staticFields->Iterate(
            [&](_In_ Field* innerStaticField)
            {
                innerStaticField->isOptional = true;
            });

        resultingClass->functions->Iterate(
            [&](_In_ Function* innerFunction)
            {
                innerFunction->isOptional = true;
            });

        resultingClass->staticFunctions->Iterate(
            [&](_In_ Function* innerStaticFunction)
            {
                innerStaticFunction->isOptional = true;
            });
    }

    return resultingClass;
}

// Creates an interface object and populates it with the given struct constructor metadata information using Projection Model
TSWrapper::Interface* TSWrapper::TSObjectModel::ExploreStruct(_In_ RtSTRUCTCONSTRUCTOR exploredStruct, _In_ LPCWSTR structName)
{
    Interface* resultingInterface = Anew(alloc, Interface);
    resultingInterface->name = structName;
    resultingInterface->metadataName = builder->stringConverter->StringOfId(exploredStruct->typeId);
    resultingInterface->alloc = alloc;

    AnalyzeElementAttributes(resultingInterface, exploredStruct->structType->typeDef);

    auto structConstructor = ProjectionModel::TypeConstructorMethodSignature::From(exploredStruct->signature);
    
    if (structConstructor->signatureType == ProjectionModel::MethodSignatureType::mstTypeConstructorMethodSignature)
    {
        Function* constructor = ExploreFunctionParameters(structConstructor->parameters->allParameters, structConstructor->parameters->returnType, L"constructor", /*retValIndex = */-1);
        constructor->returnType = nullptr;
        constructor->isConstructor = true;
        resultingInterface->AddFunction(constructor, alloc);
    }

    exploredStruct->structType->fields->Iterate(
        [&](RtPROPERTY prop)
        {
            ProjectionModel::RtABIFIELDPROPERTY innerField = ProjectionModel::AbiFieldProperty::From(prop);
            Field* field = Anew(alloc, Field);

            field->name = builder->stringConverter->StringOfId(innerField->identifier);
            field->metadataName = builder->stringConverter->StringOfId(innerField->fieldProperties->id);
            field->type = ExploreType(innerField->type);
            field->isField = true;

            resultingInterface->AddField(field, alloc);
        });

    return resultingInterface;
}

// Creates an interface object and populates it with the given delegate constructor metadata information using Projection Model
TSWrapper::Interface* TSWrapper::TSObjectModel::ExploreDelegate(_In_ RtDELEGATECONSTRUCTOR exploredDelegate, _In_ LPCWSTR delegateName)
{
    RtINTERFACECONSTRUCTOR invokeInterfaceConstructor = exploredDelegate->invokeInterface;
    return ExploreInterface(invokeInterfaceConstructor, delegateName);
}

// Creates an event object and populates it with the given event metadata information using Projection Model
TSWrapper::Event* TSWrapper::TSObjectModel::ExploreEvent(RtEVENT evnt)
{
    auto addOn = ProjectionModel::AbiMethodSignature::From(evnt->addOn);
    auto handlerparameter = ProjectionModel::AbiParameter::From(addOn->GetParameters()->allParameters->First());

    if (ProjectionModel::DelegateType::Is(handlerparameter->type))
    {
        auto handlertype = ProjectionModel::DelegateType::From(handlerparameter->type);
        auto handlerexpr = builder->ExprOfToken(handlertype->typeId, handlertype->typeDef->td, handlertype->typeDef->assembly, handlertype->genericParameters);
        auto handlerfunction = ProjectionModel::Function::From(handlerexpr);
        if (ProjectionModel::DelegateConstructor::Is(handlerfunction))
        {
            auto handlerdelegate = ProjectionModel::DelegateConstructor::From(handlerfunction);
            if (handlerdelegate->invokeInterface)
            {
                auto invokeinterface = ProjectionModel::RuntimeInterfaceConstructor::From(handlerdelegate->invokeInterface);
                auto invokemethod = ProjectionModel::AbiMethodProperty::From(invokeinterface->ownProperties->First());
                auto invokemethodbody = ProjectionModel::AbiMethod::From(invokemethod->body);
                auto invokemethodparameters = invokemethodbody->signature->GetParameters();
                auto inparameters = invokemethodparameters->allParameters->Where([&](RtPARAMETER param) { return param->isIn; }, alloc);

                Function* eventFunction = Anew(alloc, Function);
                eventFunction->name = L"ev";
                eventFunction->metadataName = L"ev";
                eventFunction->returnType = Type::GetBasicType(L"any", L"any", alloc);

                if (inparameters->Count() >= 2)
                {
                    Event* resultingEvent = Anew(alloc, Event);
                    resultingEvent->name = evnt->nameStr;
                    resultingEvent->metadataName = builder->stringConverter->StringOfId(evnt->metadataNameId);
                    resultingEvent->eventType = ExploreType(inparameters->Nth(1)->type);

                    return resultingEvent;
                }
            }
        }
    }

    return nullptr;
}

// Creates a module object and populates it with the given properties object metadata information using Projection Model
TSWrapper::Module* TSWrapper::TSObjectModel::ExploreNamespace(_In_ RtPROPERTIESOBJECT exploredNamespace, _In_ LPCWSTR namespaceName)
{
    Module* resultingModule = Anew(alloc, Module);
    resultingModule->name = namespaceName;
    resultingModule->alloc = alloc;

    exploredNamespace->fields->Reverse(alloc)->Iterate(
        [&](RtPROPERTY field)
        {
            LPCWSTR fieldName = builder->stringConverter->StringOfId(field->identifier);

            switch (field->propertyType)
            {
            case ProjectionModel::PropertyType::ptAbiTypeProperty:
            {
                switch (field->expr->type)
                {
                /// Runtime Class, Struct, Interface, Delegate
                case ProjectionModel::ExprType::exprFunction:
                {
                    RtFUNCTION constructor = ProjectionModel::Function::From(field->expr);

                    switch (constructor->functionType)
                    {
                    case ProjectionModel::FunctionType::functionRuntimeClassConstructor:
                    {
                        RtRUNTIMECLASSCONSTRUCTOR innerRuntimeClass = ProjectionModel::RuntimeClassConstructor::From(constructor);
                        resultingModule->AddClass(ExploreRuntimeClass(innerRuntimeClass, fieldName));
                        break;
                    }
                    case ProjectionModel::FunctionType::functionStructConstructor:
                    {
                        RtSTRUCTCONSTRUCTOR innerStruct = ProjectionModel::StructConstructor::From(constructor);
                        resultingModule->AddInterface(ExploreStruct(innerStruct, fieldName));
                        break;
                    }
                    case ProjectionModel::FunctionType::functionInterfaceConstructor:
                    {
                        if (ProjectionModel::RuntimeInterfaceConstructor::Is(constructor))
                        {
                            RtRUNTIMEINTERFACECONSTRUCTOR innerInterface = ProjectionModel::RuntimeInterfaceConstructor::From(constructor);
                            resultingModule->AddInterface(ExploreRuntimeInterface(innerInterface, fieldName));
                        }
                        else if (ProjectionModel::InterfaceConstructor::Is(constructor))
                        {
                            const ProjectionModel::InterfaceConstructor* innerInterface = ProjectionModel::InterfaceConstructor::From(constructor);
                            if (innerInterface->interfaceType == ProjectionModel::InterfaceConstructorType::ifRuntimeInterfaceConstructor)
                            {
                                resultingModule->AddInterface(ExploreInterface(innerInterface, fieldName));
                            }
                        }
                        break;
                    }
                    case ProjectionModel::FunctionType::functionDelegateConstructor:
                    {
                        const RtDELEGATECONSTRUCTOR innerDelegate = ProjectionModel::DelegateConstructor::From(constructor);
                        resultingModule->AddInterface(ExploreDelegate(innerDelegate, fieldName));
                        break;
                    }
                    }
                    break;
                }

                /// Enum
                case ProjectionModel::ExprType::exprEnum:
                {
                    resultingModule->AddEnum(ExploreEnum(ProjectionModel::Enum::From(field->expr), fieldName));
                    break;
                }
                }
                break;
            }
            /// Namespace
            case ProjectionModel::PropertyType::ptAbiNamespaceProperty:
            {
                RtPROPERTIESOBJECT innerNamespace = ProjectionModel::PropertiesObject::From(field->expr);
                resultingModule->AddModule(ExploreNamespace(innerNamespace, fieldName));
                break;
            }
            /// Field
            case ProjectionModel::PropertyType::ptAbiPropertyProperty:
            {
                ProjectionModel::RtABIPROPERTYPROPERTY innerField = ProjectionModel::AbiPropertyProperty::From(field);
                resultingModule->AddField(ExploreField(innerField, fieldName));
                break;
            }
            }
        });

    return resultingModule;
}

// Searches for the documentation for a function element and save it in the function element
void TSWrapper::TSObjectModel::AttachDocumentation(_In_ Function* documentedFunction, _In_ LPCWSTR path)
{
    LPCWSTR fullName = Documentation::ConcatElementName(path, documentedFunction, alloc);
    documentedFunction->documentation = Documentation::GetDocumentation(xmlReader->GetValue(documentedFunction->GetDocumentationTypePrefix(), fullName), alloc);
}

// Searches for the documentation for a field element and save it in the field element
void TSWrapper::TSObjectModel::AttachDocumentation(_In_ Field* documentedField, _In_ LPCWSTR path)
{
    LPCWSTR fullName = Documentation::ConcatElementName(path, documentedField, alloc);
    documentedField->documentation = Documentation::GetDocumentation(xmlReader->GetValue(documentedField->GetDocumentationTypePrefix(), fullName), alloc);
}

// Searches for the documentation for an event element and save it in the event element
void TSWrapper::TSObjectModel::AttachDocumentation(_In_ Event* documentedEvent, _In_ LPCWSTR path)
{
    LPCWSTR fullName = Documentation::ConcatElementName(path, documentedEvent, alloc);
    documentedEvent->documentation = Documentation::GetDocumentation(xmlReader->GetValue(documentedEvent->GetDocumentationTypePrefix(), fullName), alloc);
}

// Searches for the documentation for a class element and save it in the class element as well as attach documentation for its inner elements
void TSWrapper::TSObjectModel::AttachDocumentation(_In_ Class* documentedClass, _In_ LPCWSTR path)
{
    LPCWSTR fullName = Documentation::ConcatElementName(path, documentedClass, alloc);
    documentedClass->documentation = Documentation::GetDocumentation(xmlReader->GetValue(documentedClass->GetDocumentationTypePrefix(), fullName), alloc);

    documentedClass->fields->Iterate(
        [&](_In_ Field* innerField)
        {
            AttachDocumentation(innerField, fullName);
        });

    documentedClass->functions->Iterate(
        [&](_In_ Function* innerFunction)
        {
            AttachDocumentation(innerFunction, fullName);
        });

    documentedClass->staticFunctions->Iterate(
        [&](_In_ Function* innerStaticFunction)
        {
            AttachDocumentation(innerStaticFunction, fullName);
        });

    documentedClass->staticFields->Iterate(
        [&](_In_ Field* innerStaticField)
        {
            AttachDocumentation(innerStaticField, fullName);
        });

    documentedClass->events->Iterate(
        [&](_In_ Event* innerEvent)
        {
            AttachDocumentation(innerEvent, fullName);
        });

    documentedClass->staticEvents->Iterate(
        [&](_In_ Event* innerStaticEvent)
        {
            AttachDocumentation(innerStaticEvent, fullName);
        });
}

// Searches for the documentation for an interface element and save it in the interface element as well as attach documentation for its inner elements
void TSWrapper::TSObjectModel::AttachDocumentation(_In_ Interface* documentedInterface, _In_ LPCWSTR path)
{
    LPCWSTR fullName = documentedInterface->metadataName;
    documentedInterface->documentation = Documentation::GetDocumentation(xmlReader->GetValue(documentedInterface->GetDocumentationTypePrefix(), fullName), alloc);

    documentedInterface->fields->Iterate(
        [&](_In_ Field* innerField)
        {
            AttachDocumentation(innerField, fullName);
        });

    documentedInterface->functions->Iterate(
        [&](_In_ Function* innerFunction)
        {
            AttachDocumentation(innerFunction, fullName);
        });

    documentedInterface->staticFunctions->Iterate(
        [&](_In_ Function* innerStaticFunction)
        {
            AttachDocumentation(innerStaticFunction, fullName);
        });

    documentedInterface->staticFields->Iterate(
        [&](_In_ Field* innerStaticField)
        {
            AttachDocumentation(innerStaticField, fullName);
        });

    documentedInterface->events->Iterate(
        [&](_In_ Event* innerEvent)
        {
            AttachDocumentation(innerEvent, fullName);
        });

    documentedInterface->staticEvents->Iterate(
        [&](_In_ Event* innerStaticEvent)
        {
            AttachDocumentation(innerStaticEvent, fullName);
        });
}

// Searches for the documentation for an enum entry element and save it in the enum entry element
void TSWrapper::TSObjectModel::AttachDocumentation(_In_ EnumEntry* documentedEnumEntry, _In_ LPCWSTR path)
{
    LPCWSTR fullName = Documentation::ConcatElementName(path, documentedEnumEntry, alloc);
    documentedEnumEntry->documentation = Documentation::GetDocumentation(xmlReader->GetValue(documentedEnumEntry->GetDocumentationTypePrefix(), fullName), alloc);
}

// Searches for the documentation for a class element and save it in the class element as well as attach documentation for its inner entries
void TSWrapper::TSObjectModel::AttachDocumentation(_In_ Enum* documentedEnum, _In_ LPCWSTR path)
{
    LPCWSTR fullName = documentedEnum->metadataName;
    documentedEnum->documentation = Documentation::GetDocumentation(xmlReader->GetValue(documentedEnum->GetDocumentationTypePrefix(), fullName), alloc);

    documentedEnum->entries->Iterate(
        [&](_In_ EnumEntry* enumEntry)
        {  
            AttachDocumentation(enumEntry, fullName);
        });
}

// Searches for the documentation for a module element and save it in the module element as well as attach documentation for its inner elements
void TSWrapper::TSObjectModel::AttachDocumentation(_In_ Module* documentedModule, _In_ LPCWSTR path)
{
    LPCWSTR fullName = Documentation::ConcatElementName(path, documentedModule, alloc);
    documentedModule->documentation = Documentation::GetDocumentation(xmlReader->GetValue(documentedModule->GetDocumentationTypePrefix(), fullName), alloc);

    documentedModule->modules->Iterate(
        [&](_In_ Module* innerModule)
        {
            AttachDocumentation(innerModule, fullName);
        });

    documentedModule->classes->Iterate(
        [&](_In_ Class* innerClass)
        {
            AttachDocumentation(innerClass, fullName);
        });

    documentedModule->interfaces->Iterate(
        [&](_In_ Interface* innerInterface)
        {
            AttachDocumentation(innerInterface, fullName);
        });

    documentedModule->enums->Iterate(
        [&](_In_ Enum* innerEnum)
        {
            AttachDocumentation(innerEnum, fullName);
        });

    documentedModule->fields->Iterate(
        [&](_In_ Field* innerField)
        {
            AttachDocumentation(innerField, fullName);
        });

    documentedModule->functions->Iterate(
        [&](_In_ Function* innerFunction)
        {
            AttachDocumentation(innerFunction, fullName);
        });
}

TSWrapper::TSObjectModel::TSObjectModel(_In_ ArenaAllocator* allocator, _In_ Wrapper::MetadataContext* resolver, _In_ Settings* settings, _In_ ProjectionModel::ProjectionBuilder* builder)
{
    this->alloc = allocator;
    this->resolver = resolver;
    this->settings = settings;
    this->builder = builder;

#if DBG
    ProjectionModel::AllowHeavyOperation allow;
#endif

    expr = nullptr;
    auto assemblies = this->resolver->assemblies;
    while (assemblies)
    {
        auto assembly = assemblies->First();
        expr = this->builder->AddFromMetadataImport(expr, assembly);
        assemblies = assemblies->GetTail();
    }

    modules = nullptr;
    expr->vars->Iterate(
        [&](RtASSIGNMENT var)
        {
            modules = modules->Prepend(ExploreNamespace(ProjectionModel::PropertiesObject::From(var->expr), var->identifier), alloc);
        });

    this->xmlReader = Anew(alloc, XmlReader::XmlReader);
    this->xmlReader->Initialize(settings->xmlDocumentationFiles, alloc);

    modules->Iterate(
        [&](_In_ Module* module)
        { 
            AttachDocumentation(module, L"");
        });
}