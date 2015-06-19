//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// *******************************************************
// Represents the metadata locator which is used as a call back to api that get param iid
// *******************************************************

#include "stdafx.h"

namespace ProjectionModel
{
    STDMETHODIMP MetadataLocator::Locate(
            __in  PCWSTR nameElement,
            __in  IRoSimpleMetaDataBuilder& metaDataDestination) const
    {
        // nameElement can only be non basic types = typeDefType but since we spectialise date time we have to handle it specially.

        // We currently have only DateTime but if more add a switch here instead
        RtTYPE knownType = builder.GetKnownTypeByName(nameElement);
        if (knownType != nullptr)
        {
            // TODO: add HRESULT, EventRegistration etc to the knowntype
            switch (knownType->typeCode)
            {
            case tcWindowsFoundationDateTimeType:
                {
                    // Set the structure type
                    LPCWSTR fieldTypeName = builder.stringConverter->StringOfId(builder.GetWindowsFoundationDateTimeUnderlyingType()->fullTypeNameId);
                    HRESULT hr = metaDataDestination.SetStruct(builder.GetWindowsFoundationDateTimeTypeName(), 1, &fieldTypeName);
                    return hr;
                }
            case tcWindowsFoundationTimeSpanType:
                {
                    // Set the structure type
                    LPCWSTR fieldTypeName = builder.stringConverter->StringOfId(builder.GetWindowsFoundationTimeSpanUnderlyingType()->fullTypeNameId);
                    HRESULT hr = metaDataDestination.SetStruct(builder.GetWindowsFoundationTimeSpanTypeName(), 1, &fieldTypeName);
                    return hr;
                }
            case tcWindowsFoundationEventRegistrationTokenType:
                {
                    // Set the structure type
                    LPCWSTR fieldTypeName = builder.stringConverter->StringOfId(builder.GetWindowsFoundationEventRegistrationTokenUnderlyingType()->fullTypeNameId);
                    HRESULT hr = metaDataDestination.SetStruct(builder.GetWindowsFoundationEventRegistrationTokenTypeName(), 1, &fieldTypeName);
                    return hr;
                }
            case tcWindowsFoundationHResultType:
                {
                    // Set the structure type
                    LPCWSTR fieldTypeName = builder.stringConverter->StringOfId(builder.GetWindowsFoundationHResultUnderlyingType()->fullTypeNameId);
                    HRESULT hr = metaDataDestination.SetStruct(builder.GetWindowsFoundationHResultTypeName(), 1, &fieldTypeName);
                    return hr;
                }
            default:
                Assert(0);
            }
        }

        // TODO: consider using of typedefs instead of expr instead ?
        // Instead of reading expr we could be reading TypeDef and typedefs can to be modified to contain all the information depending on type 
        // eg. generic parameter count is already part of typedef now
        // We need more info for RuntimeClass - its default interface
        // For enum - its basetype.
        // Structs - members
        // So the probably the assembly information is union on TypeDefs and cached in when actually reading expr or here
        RtEXPR expr = builder.ExprOfPossiblyGenericTypename(nameElement);

        if (expr == nullptr)
        {
            // we couldn't fetch an type - reports a dummy one (this is for deferred ctor)
            Assert(builder.IsTypeMarkedForDeferredConstructionPass1(nameElement));

            // report HRESULT as 'placeholder' type
            LPCWSTR fieldTypeName = builder.stringConverter->StringOfId(builder.GetWindowsFoundationHResultUnderlyingType()->fullTypeNameId);
            HRESULT hr = metaDataDestination.SetStruct(builder.GetWindowsFoundationHResultTypeName(), 1, &fieldTypeName);
            return hr;
        }

        switch(expr->type)
        {
        case exprEnum:
            {
                auto _enum = Enum::From(expr);
                auto baseTypeName = _enum->baseTypeCode == ELEMENT_TYPE_I4 ? L"Int32" : L"UInt32";
                return metaDataDestination.SetEnum(builder.stringConverter->StringOfId(_enum->typeDef->id), baseTypeName);
            }
        case exprFunction:
            {
                auto function = Function::From(expr);
                switch(function->functionType)
                {
                case functionRuntimeClassConstructor:
                    {
                        auto runtimeClassConstructor = RuntimeClassConstructor::From(function);
                        
                        RtINTERFACECONSTRUCTOR defaultInterface = runtimeClassConstructor->defaultInterface.GetValue();
                        if (defaultInterface)
                        {
                            if (ifRuntimeInterfaceConstructor != defaultInterface->interfaceType)
                            {
                                // This is a missing type
                                return E_FAIL;
                            }
                            else
                            {
                                auto interfaceConstructor = RuntimeInterfaceConstructor::From(defaultInterface);
                                auto fullTypeName = builder.stringConverter->StringOfId(runtimeClassConstructor->typeDef->id);
                                if (interfaceConstructor->genericParameters->Count()>0)
                                {
                                    return metaDataDestination.SetRuntimeClassParameterizedDefault(fullTypeName, interfaceConstructor->nameCount, interfaceConstructor->locatorNames);
                                }
                                auto interfaceTypeName = builder.stringConverter->StringOfId(interfaceConstructor->signature->parameters->returnType->fullTypeNameId);
                                return metaDataDestination.SetRuntimeClassSimpleDefault(fullTypeName, interfaceTypeName, &interfaceConstructor->iid->instantiated);
                            }
                        }
                        // This is the MarshalAs path (a runtime class with no interfaces). 
                        auto fullTypeName = builder.stringConverter->StringOfId(runtimeClassConstructor->typeDef->id);
                        return metaDataDestination.SetRuntimeClassSimpleDefault(fullTypeName, fullTypeName, &GUID_NULL);
                    }
                case functionInterfaceConstructor:
                    {
                        if (MethodSignature::IsMissingTypeSignature(function->signature))
                        {
                            return E_FAIL;
                        }

                        auto interfaceConstructor = RuntimeInterfaceConstructor::From(function);
                        UINT32 parameterCount = interfaceConstructor->genericParameters->Count();
                        if (parameterCount>0)
                        {
                            return metaDataDestination.SetParameterizedInterface(interfaceConstructor->iid->piid, parameterCount);
                        }
                        return metaDataDestination.SetWinRtInterface(interfaceConstructor->iid->instantiated);
                    }
                case functionStructConstructor:
                    {
                        auto structConstructor = StructConstructor::From(function);
                        auto fullTypeName = builder.stringConverter->StringOfId(structConstructor->signature->parameters->returnType->fullTypeNameId);

                        // pass 1 - count the number of fields
                        // NOTE: If a field is of type G<T> - need to split the field as G and T (two distinct fields) - BLUE#256216
                        size_t fieldCount = 0;
                        structConstructor->structType->fields->IterateN([&](int index, RtPROPERTY prop) {
                            auto fieldProperty = AbiFieldProperty::From(prop);

                            // basic type of field
                            fieldCount++;

                            // expand for all generic parameters
                            if (TypeDefinitionType::Is(fieldProperty->type))
                            {
                                auto genericType = TypeDefinitionType::From(fieldProperty->type);
                                Assert(genericType != nullptr);
                                
                                size_t genericParametersCount = 0;
                                if (genericType->genericParameters != nullptr)
                                {
                                    genericParametersCount = genericType->genericParameters->Count();
                                }

                                fieldCount += genericParametersCount;
                            }
                        });

                        OUTPUT_TRACE_DEBUGONLY(Js::ProjectionMetadataPhase, L"Locate(): functionStructConstructor - setting up %d field(s) for type %s (#%d)\n", 
                            fieldCount, fullTypeName, structConstructor->signature->parameters->returnType->fullTypeNameId);

                        // pass 2 - allocate and fill
                        size_t currentFieldIndex = 0;
                        auto fieldTypeNames = new PCWSTR[fieldCount];
                        if (fieldTypeNames == nullptr)
                        {
                            return E_OUTOFMEMORY;
                        }

                        structConstructor->structType->fields->IterateN([&](int index, RtPROPERTY prop) {
                            auto fieldProperty = AbiFieldProperty::From(prop);

                            if (TypeDefinitionType::Is(fieldProperty->type))
                            {
                                // generic type (possibly)
                                auto genericType = TypeDefinitionType::From(fieldProperty->type);

                                Assert(currentFieldIndex < fieldCount);
                                Assert(genericType->typeDef != nullptr);
                                fieldTypeNames[currentFieldIndex++] = builder.stringConverter->StringOfId(genericType->typeDef->id);

                                OUTPUT_TRACE_DEBUGONLY(Js::ProjectionMetadataPhase, L"Locate(): functionStructConstructor field #%d/%d: G< >: %s (#%d)\n",
                                    currentFieldIndex, fieldCount, fieldTypeNames[currentFieldIndex-1], genericType->typeId);
                                
                                if (genericType->genericParameters != nullptr)
                                {
                                    genericType->genericParameters->Iterate([&](RtTYPE type) {
                                        Assert(currentFieldIndex < fieldCount);
                                        fieldTypeNames[currentFieldIndex++] = builder.stringConverter->StringOfId(type->fullTypeNameId);

                                        OUTPUT_TRACE_DEBUGONLY(Js::ProjectionMetadataPhase, L"Locate(): functionStructConstructor field #%d/%d:   T : %s (#%d)\n",
                                            currentFieldIndex, fieldCount, fieldTypeNames[currentFieldIndex-1], type->fullTypeNameId);
                                    });
                                }
                            }
                            else
                            {
                                // basic type of field
                                Assert(currentFieldIndex < fieldCount);
                                fieldTypeNames[currentFieldIndex++] = builder.stringConverter->StringOfId(fieldProperty->type->fullTypeNameId);

                                OUTPUT_TRACE_DEBUGONLY(Js::ProjectionMetadataPhase, L"Locate(): functionStructConstructor field #%d/%d: ----: %s (#%d)\n",
                                    currentFieldIndex, fieldCount, fieldTypeNames[currentFieldIndex-1], fieldProperty->type->fullTypeNameId);
                            }
                        });

                        auto hr = metaDataDestination.SetStruct(fullTypeName, fieldCount, fieldTypeNames);
                        delete [] fieldTypeNames;
                        return hr;
                    }
                case functionDelegateConstructor:
                    {
                        auto delegateConstructor = DelegateConstructor::From(function);
                        auto interfaceConstructor = RuntimeInterfaceConstructor::From(delegateConstructor->invokeInterface);
                        UINT32 parameterCount = interfaceConstructor->genericParameters->Count();
                        if (parameterCount>0)
                        {
                            return metaDataDestination.SetParameterizedDelegate(interfaceConstructor->iid->piid, parameterCount);
                        }
                        return metaDataDestination.SetDelegate(interfaceConstructor->iid->instantiated);

                    }
                case functionMissingTypeConstructor: // JsGen-only: found a type from another assembly.
                    return E_FAIL;
                default:
                    Js::Throw::FatalProjectionError();
                }
            }
        default: 
            Js::Throw::FatalProjectionError();
        }
    }
}