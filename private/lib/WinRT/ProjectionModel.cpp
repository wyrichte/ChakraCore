//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2010 by Microsoft Corporation.  All rights reserved.
//---------------------------------------------------------------------------
#include "WinRTPch.h"

#ifdef PROJECTION_METADATA_TRACE
#define TRACE_METADATA(...) { Trace(__VA_ARGS__); }
#else
#define TRACE_METADATA(...)
#endif

namespace ProjectionModel
{
    using namespace Js;
    using namespace JsUtil;


#if DBG
    // debug helpers
    LPCWSTR ProjectionBuilder::TraceRtTYPE(_In_ RtTYPE type, _In_ Metadata::IStringConverter* strConverter, _In_ ArenaAllocator* allocator)
    {
        DefaultImmutableStringBuilder retValue;

        if (type != nullptr)
        {
            retValue.Append(_u("{typeName:\""));
            retValue.Append(strConverter->StringOfId(type->fullTypeNameId));
            retValue.Append(_u("(#"));
            retValue.AppendInt32(type->fullTypeNameId);
            retValue.Append(_u(")\", typeCode:"));
            retValue.AppendInt32(type->typeCode);
            retValue.Append(_u(", canMarshal:"));
            retValue.AppendInt32(type->canMarshal);
            retValue.Append(_u("}"));
        }
        else
        {
            retValue.Append(_u("\"<NULL>\""));
        }

        return retValue.Get(allocator);
    }

    LPCWSTR ProjectionBuilder::TraceRtPARAMETERS(_In_ RtPARAMETERS parameters, _In_ Metadata::IStringConverter* strConverter, _In_ ArenaAllocator* allocator)
    {
        DefaultImmutableStringBuilder retValue;

        if (parameters != nullptr)
        {
            retValue.Append(_u("{callPattern:"));
            retValue.Append(parameters->callPattern == nullptr ? _u("\"<NULL>\"") : parameters->callPattern);
            retValue.Append(_u(", sizeOfCallstack:"));
            retValue.AppendUInt64(parameters->sizeOfCallstack);
            retValue.Append(_u(", returnType:"));
            LPCWSTR returnTypeString = TraceRtTYPE(parameters->returnType, strConverter, allocator);
            retValue.AppendWithCopy(returnTypeString);
            DefaultImmutableStringBuilder::FreeString(allocator, returnTypeString, wcslen(returnTypeString));
            retValue.Append(_u(", parameters:["));
            bool isFirstProperty = true;
            parameters->allParameters->Iterate([&](RtPARAMETER parameter) {
                if (isFirstProperty == false)
                {
                    retValue.Append(_u(", "));
                }
                retValue.Append(_u("{id:\""));
                retValue.Append(strConverter->StringOfId(parameter->id));
                retValue.Append(_u("(#"));
                retValue.AppendInt32(parameter->id);
                retValue.Append(_u(")\", isIn:"));
                retValue.AppendBool(parameter->isIn);
                retValue.Append(_u(", isOut:"));
                retValue.AppendBool(parameter->isOut);
                retValue.Append(_u(", parameterType:"));
                retValue.AppendInt32(parameter->parameterType);
                retValue.Append(_u(", type:"));
                LPCWSTR parameterTypeString = TraceRtTYPE(parameter->type, strConverter, allocator);
                retValue.AppendWithCopy(parameterTypeString);
                DefaultImmutableStringBuilder::FreeString(allocator, parameterTypeString, wcslen(parameterTypeString));
                retValue.Append(_u("}"));

                isFirstProperty = false;
            });
            retValue.Append(_u("]}"));
        }
        else
        {
            retValue.Append(_u("\"<NULL>\""));
        }

        return retValue.Get(allocator);
    }

    LPCWSTR ProjectionBuilder::TraceRtOVERLOADEDMETHODSIGNATURE(_In_ RtOVERLOADEDMETHODSIGNATURE signature, _In_ Metadata::IStringConverter* strConverter, _In_ ArenaAllocator* allocator)
    {
        DefaultImmutableStringBuilder retValue;

        if (signature != nullptr)
        {
            retValue.Append(_u("{name:\""));
            retValue.Append(strConverter->StringOfId(signature->nameId));
            retValue.Append(_u("(#"));
            retValue.AppendInt32(signature->nameId);
            retValue.Append(_u(")\", signType:"));
            retValue.AppendInt32(signature->signatureType);
            retValue.Append(_u(", overloads:{"));

            if (signature->overloads != nullptr)
            {
                retValue.Append(_u("id:\""));
                retValue.Append(strConverter->StringOfId(signature->overloads->id));
                retValue.Append(_u("(#"));
                retValue.AppendInt32(signature->overloads->id);
                retValue.Append(_u(")\""));

                if (signature->overloads->overloads != nullptr)
                {
                    bool isFirstProperty = true;
                    retValue.Append(_u(", overloads:["));

                    signature->overloads->overloads->Iterate([&](RtABIMETHODSIGNATURE signature) {
                        if (isFirstProperty == false)
                        {
                            retValue.Append(_u(", "));
                        }
                        retValue.Append(_u("{signature:"));
                        LPCWSTR signatureString = TraceRtABIMETHODSIGNATURE(signature, strConverter, allocator);
                        retValue.AppendWithCopy(signatureString);
                        DefaultImmutableStringBuilder::FreeString(allocator, signatureString, wcslen(signatureString));
                        retValue.Append(_u("}"));

                        isFirstProperty = false;
                    });

                    retValue.Append(_u("]"));
                }
            }

            retValue.Append(_u("}, parameters:"));
            LPCWSTR parametersString = TraceRtPARAMETERS(signature->parameters, strConverter, allocator);
            retValue.AppendWithCopy(parametersString);
            DefaultImmutableStringBuilder::FreeString(allocator, parametersString, wcslen(parametersString));

            retValue.Append(_u("}"));
        }
        else
        {
            retValue.Append(_u("\"<NULL>\""));
        }

        return retValue.Get(allocator);
    }

    LPCWSTR ProjectionBuilder::TraceRtIID(_In_ RtIID rtIID, _In_ Metadata::IStringConverter* strConverter, _In_ ArenaAllocator* allocator)
    {
        DefaultImmutableStringBuilder retValue;

        if (rtIID != nullptr)
        {
            WCHAR guidStr[MaxGuidLength];

            retValue.Append(_u("{piid:\""));
            StringFromGUID2(rtIID->piid, guidStr, ARRAYSIZE(guidStr));
            retValue.Append(guidStr);
            retValue.Append(_u("\", instantiated:\""));
            StringFromGUID2(rtIID->instantiated, guidStr, ARRAYSIZE(guidStr));
            retValue.Append(guidStr);
            retValue.Append(_u("\"}"));
        }

        return retValue.Get(allocator);
    }

    LPCWSTR ProjectionBuilder::TraceRtABIMETHODSIGNATURE(_In_ RtABIMETHODSIGNATURE signature, _In_ Metadata::IStringConverter* strConverter, _In_ ArenaAllocator* allocator)
    {
        DefaultImmutableStringBuilder retValue;

        if (signature != nullptr)
        {
            LPCWSTR string = nullptr;

            retValue.Append(_u("{hasDefaultOverride:"));
            retValue.AppendBool(signature->hasDefaultOverloadAttribute);
            retValue.Append(_u(", iid:"));
            string = TraceRtIID(signature->iid, strConverter, allocator);
            retValue.AppendWithCopy(string);
            DefaultImmutableStringBuilder::FreeString(allocator, string, wcslen(string));
            retValue.Append(_u(", \"#in params\":"));
            retValue.AppendUInt64(signature->inParameterCount);
            retValue.Append(_u(", metadataName:\""));
            retValue.Append(strConverter->StringOfId(signature->metadataNameId));
            retValue.Append(_u("(#"));
            retValue.AppendInt32(signature->metadataNameId);
            retValue.Append(_u(")\", runtimeClassName:\""));
            retValue.Append(strConverter->StringOfId(signature->runtimeClassNameId));
            retValue.Append(_u("(#"));
            retValue.AppendInt32(signature->runtimeClassNameId);
            retValue.Append(_u(")\", parameters:"));
            string = TraceRtPARAMETERS(signature->parameters, strConverter, allocator);
            retValue.AppendWithCopy(string);
            DefaultImmutableStringBuilder::FreeString(allocator, string, wcslen(string));
            retValue.Append(_u("}"));
        }
        else
        {
            retValue.Append(_u("\"<NULL>\""));
        }

        return retValue.Get(allocator);
    }

    LPCWSTR ProjectionBuilder::TraceRtPROPERTIESOBJECT(_In_ RtPROPERTIESOBJECT properties, _In_ Metadata::IStringConverter* strConverter, _In_ ArenaAllocator* allocator)
    {
        DefaultImmutableStringBuilder retValue;

        if (properties != nullptr)
        {
            retValue.Append(_u("{type:"));
            retValue.AppendInt32(properties->type);
            retValue.Append(_u(", fields:["));

            bool isFirstProperty = true;
            properties->fields->Iterate([&](RtPROPERTY prop) {
                if (isFirstProperty == false)
                {
                    retValue.Append(_u(", "));
                }

                retValue.Append(_u("{name:\""));
                retValue.Append(strConverter->StringOfId(prop->identifier));
                retValue.Append(_u("(#"));
                retValue.AppendInt32(prop->identifier);
                retValue.Append(_u(")\""));
                isFirstProperty = false;

                // TODO: value?
                retValue.Append(_u("}"));
            });
            retValue.Append(_u("]}"));
        }
        else
        {
            retValue.Append(_u("\"<NULL>\""));
        }

        return retValue.Get(allocator);
    }

    void ProjectionBuilder::TraceRtOVERLOADPARENTPROPERTY(_In_ RtOVERLOADPARENTPROPERTY overloadParentProperty, _In_z_ LPWSTR prefixMessage, _In_ Metadata::IStringConverter* strConverter, _In_ ArenaAllocator* allocator)
    {
        DefaultImmutableStringBuilder retValue;
        prefixMessage = prefixMessage == nullptr ? _u("TraceRtOVERLOADPARENTPROPERTY()") : prefixMessage;

        if (overloadParentProperty != nullptr)
        {
            LPCWSTR signatureString = TraceRtOVERLOADEDMETHODSIGNATURE(overloadParentProperty->overloadConstructor->signature, strConverter, allocator);
            LPCWSTR propertiesString = TraceRtPROPERTIESOBJECT(overloadParentProperty->overloadConstructor->properties, strConverter, allocator);

            TRACE_METADATA(_u("%s: {name:\"%s(#%d)\", propertyType:\"%d ptOverloadParentProperty\", expr:%d, overloadConstructor: {signature:%s, properties:%s}}\n"),
                prefixMessage,
                strConverter->StringOfId(overloadParentProperty->identifier),
                overloadParentProperty->identifier,
                overloadParentProperty->propertyType,
                overloadParentProperty->expr,
                signatureString,
                propertiesString);
            DefaultImmutableStringBuilder::FreeString(allocator, propertiesString, wcslen(propertiesString));
            DefaultImmutableStringBuilder::FreeString(allocator, signatureString, wcslen(signatureString));
        }
        else
        {
            TRACE_METADATA(_u("%s: (no overloadParentProperty passed)\n"), prefixMessage);
            AssertMsg(false, "no overloadParentProperty passed");
        }
    }

    void ProjectionBuilder::TraceRtABIMETHODPROPERTY(_In_ RtABIMETHODPROPERTY methodProperty, _In_z_ LPWSTR prefixMessage, _In_ Metadata::IStringConverter* strConverter, _In_ ArenaAllocator* allocator)
    {
        DefaultImmutableStringBuilder retValue;
        prefixMessage = prefixMessage == nullptr ? _u("TraceRtABIMETHODPROPERTY()") : prefixMessage;

        if (methodProperty != nullptr)
        {
            LPCWSTR signatureString = TraceRtABIMETHODSIGNATURE(methodProperty->body->signature, strConverter, allocator);
            LPCWSTR propertiesString = TraceRtPROPERTIESOBJECT(methodProperty->body->properties, strConverter, allocator);

            TRACE_METADATA(_u("%s: {name:\"%s(#%d)\", propertyType:\"%d ptAbiMethodProperty\", signature:%s, properties:%s}\n"),
                prefixMessage,
                strConverter->StringOfId(methodProperty->identifier),
                methodProperty->identifier,
                methodProperty->propertyType,
                signatureString,
                propertiesString);
            DefaultImmutableStringBuilder::FreeString(allocator, propertiesString, wcslen(propertiesString));
            DefaultImmutableStringBuilder::FreeString(allocator, signatureString, wcslen(signatureString));
        }
        else
        {
            TRACE_METADATA(_u("%s: (no methodProperty passed)\n"), prefixMessage);
            AssertMsg(false, "no methodProperty passed");
        }
    }

    void ProjectionBuilder::TraceRtABIMETHODPROPERTY(_In_ ImmutableList<RtPROPERTY>* methodProperties, _In_z_ LPWSTR prefixMessage, _In_ Metadata::IStringConverter* strConverter, _In_ ArenaAllocator* allocator)
    {
        TRACE_METADATA(_u("%s invoked\n"), prefixMessage);

        methodProperties->Iterate([&](RtPROPERTY methodProperty) {
            if (AbiMethodProperty::Is(methodProperty))
            {
                RtABIMETHODPROPERTY abiMethodProperty = AbiMethodProperty::From(methodProperty);
                ProjectionBuilder::TraceRtABIMETHODPROPERTY(abiMethodProperty, prefixMessage, strConverter, allocator);
            }
        });

        TRACE_METADATA(_u("%s completed\n"), prefixMessage);
    }

    void ProjectionBuilder::TraceRtPROPERTY(_In_ RtPROPERTY prop, _In_z_ LPWSTR prefixMessage, _In_ Metadata::IStringConverter* strConverter, _In_ ArenaAllocator* allocator)
    {
        prefixMessage = prefixMessage == nullptr ? _u("TraceRtABIMETHODPROPERTY()") : prefixMessage;

        if (prop != nullptr)
        {
            switch( prop->propertyType )
            {
            case ptAbiMethodProperty:
                {
                    auto methodProperty = AbiMethodProperty::From(prop);
                    TraceRtABIMETHODPROPERTY(methodProperty, prefixMessage, strConverter, allocator);
                }
                break;

            case ptOverloadParentProperty:
                {
                    auto overloadParentProperty = OverloadParentProperty::From(prop);
                    TraceRtOVERLOADPARENTPROPERTY(overloadParentProperty, prefixMessage, strConverter, allocator);
                }
                break;

            default:
                TRACE_METADATA(_u("%s: {name:\"%s\", id:%d, \"unknown propertyType\":%d}\n"),
                    prefixMessage,
                    strConverter->StringOfId(prop->identifier),
                    prop->identifier,
                    prop->propertyType);
                break;
            }
        }
        else
        {
            TRACE_METADATA(_u("%s: (no RtPROPERTY passed)\n"), prefixMessage);
            AssertMsg(false, "no RtPROPERTY passed");
        }
    }
#endif // DBG

#if DBG
    __declspec(thread) int allowHeavyOperation = 0;
    __declspec(thread) bool weShouldNotBeParsingMetadata = false;
    __declspec(thread) int weAreParsingDelegateMetadata = false;
#endif

    CompareRefConstNames CompareRefConstNames::Instance;

    struct CheckForDuplicateTypeId
    {
        ImmutableList<DeferredTypeDefinitionCandidate>** list;
        int count;
        CheckForDuplicateTypeId(ImmutableList<DeferredTypeDefinitionCandidate> ** list)
            : list(list), count(0)
        {
        }

        bool Contains(DeferredTypeDefinitionCandidate type)
        {
            bool retValue = (*list)->ContainsWhere([&](DeferredTypeDefinitionCandidate listType) {
                bool value = listType.Equals(type);
                return value;
            });

            return retValue;
        }

        // Add a given type to the list of tracked types - in case of dup, return true, otherwise false
        //  (but will prepend the type info regardless)
        bool AddTypeId(MetadataStringId token, ImmutableList<RtTYPE>* genericParameters, ArenaAllocator* allocator, Metadata::IStringConverter* strConverter)
        {
            DeferredTypeDefinitionCandidate newDefinition(token, genericParameters);

            return this->AddTypeId(newDefinition, allocator, strConverter);
        }

        // Add a given type to the list of tracked types - in case of dup, return true, otherwise false
        //  (but will prepend the type info regardless)
        bool AddTypeId(DeferredTypeDefinitionCandidate& newDefinition, ArenaAllocator* allocator, Metadata::IStringConverter* strConverter)
        {
            Assert(strConverter != nullptr);
            Assert(allocator != nullptr);
            bool retValue = Contains(newDefinition);

            (*list) = (*list)->Prepend(newDefinition, allocator);
            ++count;

            LPCWSTR typeString = newDefinition.ToString(strConverter, allocator);

            if (retValue)
            {
                OUTPUT_TRACE(
                    ProjectionMetadataPhase,
                    _u("CheckForDuplicateTypeId::AddTypeId - Duplicate %s present and re-added\n"),
                    typeString);
            }
            else
            {
                OUTPUT_TRACE_DEBUGONLY(
                    ProjectionMetadataPhase,
                    _u("CheckForDuplicateTypeId::AddTypeId - New (unique for now) %s\n"),
                    typeString);
            }

            DefaultImmutableStringBuilder::FreeString(allocator, typeString, wcslen(typeString));

            return retValue;
        }

        ~CheckForDuplicateTypeId()
        {
            // Pop our token;
            while(count--)
            {
                (*list) = (*list)->GetTail();
            }
        }
    };

    // Info:        Return true if the two methods have the same name.
    // Parameters:  m1 - method1
    //              m2 - method2
    int MethodEqualsById(const Metadata::MethodProperties*m1, const Metadata::MethodProperties* m2)
    {
        return (m1->id == m2->id);
    }

    // Info:        Return true if the two metadat methods have the same arity.
    // Parameters:  m1 - method1
    //              m2 - method2
    int MethodEqualsByArity(const Metadata::MethodProperties * m1, const Metadata::MethodProperties * m2)
    {
        return m1->signature->parameterCount==m2->signature->parameterCount;
    }

    // Info:        Return true if the two properties have the same name
    // Parameters:  p1 - property1
    //              p2 - property2
    int PropertyEqualsById(RtPROPERTY p1, RtPROPERTY p2)
    {
        return (p1->identifier == p2->identifier);
    }

    // Info:        Return true if the two abi methods have the same arity.
    // Parameters:  m1 - method1
    //              m2 - method2
    int AbiMethodSignatureEqualsByArity(RtABIMETHODSIGNATURE m1, RtABIMETHODSIGNATURE m2)
    {
        return m1->inParameterCount == m2->inParameterCount;
    }

    struct CompareRefArities : public regex::Comparer<RtABIMETHODSIGNATURE *>
    {
        static CompareRefArities Instance;

        // Info:        Return true if arities are the same
        // Parameters:  v1 - group 1
        //              v2 - group 2
        bool Equals(RtABIMETHODSIGNATURE *v1, RtABIMETHODSIGNATURE *v2)
        {
            Assert(v1 != nullptr && v2 != nullptr);
            return (*v1)->inParameterCount == (*v2)->inParameterCount;
        }

        // Info:        Not called
        int GetHashCode(RtABIMETHODSIGNATURE *s)
        {
            Js::Throw::FatalProjectionError();
        }

        // Info:        Compare two arity groups
        // Parameters:  v1 - group 1
        //              v2 - group 2
        int Compare(RtABIMETHODSIGNATURE *v1, RtABIMETHODSIGNATURE *v2)
        {
            Assert(v1 != nullptr && v2 != nullptr);
            if ((*v1)->inParameterCount == (*v2)->inParameterCount)
            {
                return 0;
            } else if ((*v1)->inParameterCount < (*v2)->inParameterCount)
            {
                return -1;
            }
            return 1;
        }
    };

    CompareRefArities CompareRefArities::Instance;

    // IIDs
    InstantiatedIID simpleActivatableIID(__uuidof(IActivationFactory),__uuidof(IActivationFactory));
    InstantiatedIID nullIID(GUID_NULL, GUID_NULL);

    struct SpecializationParameterizedIIDFinder
    {
        static bool IsSpecializationParameterizedIID(const IID & piid)
        {
            return piid == IID_IVector1 || piid == IID_IVectorView1 || piid == __uuidof(IAsyncInfo) || piid == IID_IMap2 || piid == IID_IMapView2 || piid == Windows::Foundation::IID_IPropertyValue;
        }
    };

    struct IReferenceOrIReferenceArrayIIDFinder
    {
        static bool IsSpecializationParameterizedIID(const IID & piid)
        {
            return piid == IID_IReference1 || piid == IID_IReferenceArray1;
        }
    };

    // Info:        Get the full type name of a basic type
    LPCWSTR GetBasicTypeCorName(CorElementType typeCor)
    {
        switch(typeCor)
        {
            case ELEMENT_TYPE_CHAR: return _u("Char16");
            case ELEMENT_TYPE_STRING:  return _u("String");
            case ELEMENT_TYPE_BOOLEAN: return _u("Boolean");
            case ELEMENT_TYPE_U1: return _u("UInt8");
            case ELEMENT_TYPE_I2: return _u("Int16");
            case ELEMENT_TYPE_U2: return _u("UInt16");
            case ELEMENT_TYPE_I4: return _u("Int32");
            case ELEMENT_TYPE_U4: return _u("UInt32");
            case ELEMENT_TYPE_I8: return _u("Int64");
            case ELEMENT_TYPE_U8: return _u("UInt64");
            case ELEMENT_TYPE_R4: return _u("Single");
            case ELEMENT_TYPE_R8: return _u("Double");
            case ELEMENT_TYPE_OBJECT: return _u("Object");
            case ELEMENT_TYPE_VAR: return _u("T");
            default: Js::Throw::FatalProjectionError();
        }
    }

    // Info:        Get the full type name of a basic type
    LPCWSTR GetBasicTypeName(RtBASICTYPE bt)
    {
        return GetBasicTypeCorName(bt->typeCor);
    }

    // Info:        Determine and remember whether this type can be marshaled
    bool Type::CanMarshal(ProjectionBuilder * builder, bool allowMissingTypes, bool *outWasMissingType) const
    {
        if (!builder->IsWinRTAdaptiveAppsEnabled())
        {
            allowMissingTypes = false;
        }
        if (canMarshal == cmtDontKnowYet)
        {
            auto nc = const_cast<Type*>(this);
            bool wasMissingType = false;
            if (builder->CanMarshalType(this, false, allowMissingTypes, &wasMissingType))
            {
                nc->canMarshal = wasMissingType ? cmtMissingType : cmtYes;
            }
            else
            {
                nc->canMarshal = cmtNo;
            }
        }

        if (outWasMissingType != nullptr)
        {
            *outWasMissingType = canMarshal == cmtMissingType;
        }

        return canMarshal == cmtYes || (allowMissingTypes && canMarshal == cmtMissingType);
    }


    bool ConcreteType::IsBlittable(RtTYPE type)
    {
        if (BasicType::Is(type))
        {
            RtBASICTYPE basicType = BasicType::From(type);
            switch(basicType->typeCor)
            {
                case ELEMENT_TYPE_CHAR:
                case ELEMENT_TYPE_BOOLEAN:
                case ELEMENT_TYPE_U1:
                case ELEMENT_TYPE_I2:
                case ELEMENT_TYPE_U2:
                case ELEMENT_TYPE_I4:
                case ELEMENT_TYPE_U4:
                case ELEMENT_TYPE_I8:
                case ELEMENT_TYPE_U8:
                case ELEMENT_TYPE_R4:
                case ELEMENT_TYPE_R8:
                    return true;
                default:
                    return false;
            }
        }
        else if (StructType::Is(type))
        {
            RtSTRUCTTYPE structType = StructType::From(type);
            return structType->isBlittable;
        }
        else if (EnumType::Is(type))
        {
            return true;
        }
        else if (WindowsFoundationDateTimeType::Is(type))
        {
            return true; // int64 so can be just copied
        }
        else if (WindowsFoundationTimeSpanType::Is(type))
        {
            return true; // int64 so can be just copied
        }
        else if (WindowsFoundationEventRegistrationTokenType::Is(type))
        {
            return true; // int 64 so can be just copied
        }
        else if (WindowsFoundationHResultType::Is(type))
        {
            return true; // int 32 so can be just copied
        }

        return SystemGuidType::Is(type);
    }

    // Info:        Build the list of type names for conversion of piid to iid
    // Parameters:  typeNamesBuilder - array builder to append the type names
    //              genericParameters - types to specialize the topmost type in the list
    //              stringConverter - converter to get string from metadataStringId
    // Returns:     false if this type is not instantiable (for example, has free variables)
    template<class TAllocator>
    bool BuildTypeNameList(ImmutableArrayBuilder<LPCWSTR, TAllocator> &typeNamesBuilder, ImmutableList<RtTYPE> * genericParameters, Metadata::IStringConverter * stringConverter, ProjectionBuilder *builder, bool *isWebHidden = nullptr)
    {
        Assert(typeNamesBuilder.GetCount() > 0);

        bool isInstantiable = true;
        while(genericParameters && isInstantiable)
        {
            auto type = genericParameters->First();
            if (isWebHidden != nullptr && MissingNamedType::Is(type) && MissingNamedType::From(type)->isWebHidden)
            {
                *isWebHidden = true;
            }
            if (TypeDefinitionType::Is(type))
            {
                auto tdt = TypeDefinitionType::From(type);
                if (isWebHidden != nullptr && ClassType::Is(tdt) && !builder->TypePartiallyResolved(tdt->typeDef))
                {
                    //If we are constructing this type on the stack currently, then we don't need to check isWebHidden here, it will fail higher on the stack
                    auto expr = builder->IntermediateExprOfToken(MetadataStringIdNil, tdt->typeDef->td, tdt->typeDef->assembly, nullptr);
                    if (RuntimeClassConstructor::Is(expr))
                    {
                        auto ric = RuntimeClassConstructor::From(expr);
                        if (ric->defaultInterface.HasValue() && MissingInterfaceConstructor::Is(ric->defaultInterface.GetValue()))
                        {
                            *isWebHidden = MissingInterfaceConstructor::From(ric->defaultInterface.GetValue())->isWebHidden;
                        }
                    }
                }
                typeNamesBuilder.Append(stringConverter->StringOfId(tdt->typeDef->id));
                isInstantiable = BuildTypeNameList(typeNamesBuilder, tdt->genericParameters, stringConverter, builder, isWebHidden);
            }
            else if (type->fullTypeNameId != MetadataStringIdNil)
            {
                typeNamesBuilder.Append(stringConverter->StringOfId(type->fullTypeNameId));
                isInstantiable = ConcreteType::Is(type);
            }
            else
            {
                Js::Throw::FatalProjectionError();
            }
            genericParameters = genericParameters->GetTail();
        }

        return isInstantiable;
    }

    template<class TAllocator>
    HRESULT GetInstantiatedIIDAndTypeNameParts(
        __in ProjectionBuilder *builder,
        __in MetadataStringId typeId,
        __in ImmutableList<RtTYPE> *genericInstantiations,
        __in TAllocator * a,
        __out IID *instantiatedIID,
        __out_opt unsigned int *locatorNameCount,
        __out_opt LPCWSTR **locatorNames,
        bool *isWebHidden)
    {
        // Simple interface
        Assert(builder != nullptr);
        Assert(!genericInstantiations->IsEmpty());

        ImmutableArrayBuilder<LPCWSTR, TAllocator> typeNamesBuilder(a);
        LPCWSTR typeName = builder->stringConverter->StringOfId(typeId);
        typeNamesBuilder.Append(typeName);

        bool isInstantiable = BuildTypeNameList(typeNamesBuilder, genericInstantiations, builder->stringConverter, builder, isWebHidden);
        if (isInstantiable)
        {
            size_t typeNameCount = typeNamesBuilder.GetCount();
            LPCWSTR *typeNames = typeNamesBuilder.Get();
            AssertMsg(typeNameCount < 65536, "Invalid metadata: ECMA-335 II.22.20 defines GenericParam as having a 2-byte index");

            MetadataLocator locator(*builder);
            auto hr = builder->GetResolver()->GetRoParameterizedIIDDelayLoad()->RoGetParameterizedTypeInstanceIID(
                (uint)typeNameCount,
                typeNames,
                locator,
                instantiatedIID);

            if (SUCCEEDED(hr) && locatorNameCount != nullptr)
            {
                Assert(locatorNames != nullptr);

                *locatorNameCount = (uint)typeNameCount;
                *locatorNames = typeNames;
                typeNamesBuilder.DisableAutoDelete();
            }

            return hr;
        }

        return S_OK;
    }

    template HRESULT GetInstantiatedIIDAndTypeNameParts<HeapAllocator>(
        __in ProjectionBuilder *builder,
        __in MetadataStringId typeId,
        __in ImmutableList<RtTYPE> *genericInstantiations,
        __in HeapAllocator * a,
        __out IID *instantiatedIID,
        __out_opt unsigned int *locatorNameCount,
        __out_opt LPCWSTR **locatorNames,
        bool *isWebHidden);

    // Info:        Append a type to the given call pattern
    void ProjectionBuilder::AppendTypePattern(RtTYPE type, DefaultImmutableStringBuilder & pattern)
    {
        switch(type->typeCode)
        {
        case tcBasicType:
            pattern.Append(GetBasicTypeName(BasicType::From(type)));
            break;
        case tcArrayType:
            pattern.Append(_u("["));
            AppendTypePattern(ArrayType::From(type)->elementType, pattern);
            pattern.Append(_u("]"));
            break;
        case tcVoidType:
            pattern.Append(_u("Void"));
            break;
        case tcSystemGuidType:
            pattern.Append(_u("Guid"));
            break;
        case tcWindowsFoundationDateTimeType:
            pattern.Append(_u("DateTime"));
            break;
        case tcWindowsFoundationTimeSpanType:
            pattern.Append(_u("TimeSpan"));
            break;
        case tcWindowsFoundationEventRegistrationTokenType:
            pattern.Append(_u("EventRegistrationTokenType"));
            break;
        case tcWindowsFoundationHResultType:
            pattern.Append(_u("HResultTokenType"));
            break;
        case tcInterfaceType:
            {
                auto ic = InterfaceType::From(type);
                auto expr = IntermediateExprOfToken(MetadataStringIdNil, ic->typeDef->td, ic->typeDef->assembly, nullptr);
                if (RuntimeInterfaceConstructor::Is(expr))
                {
                    pattern.Append(_u("Interface"));
                    return;
                }
                else if (InterfaceConstructor::Is(expr))
                {
                    pattern.Append(_u("missingInterface("));
                    pattern.Append(stringConverter->StringOfId(InterfaceConstructor::From(expr)->typeId));
                    pattern.Append(_u(")"));
                    return;
                }
                Js::Throw::FatalInternalError();
            }
        case tcClassType:
            {
                auto rtc = ClassType::From(type);
                auto expr = IntermediateExprOfToken(MetadataStringIdNil, rtc->typeDef->td, rtc->typeDef->assembly, nullptr);
                if (RuntimeClassConstructor::Is(expr))
                {
                    auto rcc = RuntimeClassConstructor::From(expr);

                    if (rcc->specialization==nullptr)
                    {
                        pattern.Append(_u("Class"));
                        return;
                    }
                    switch(rcc->specialization->specializationType)
                    {
                    case specVectorSpecialization: pattern.Append(_u("Class(Vector)")); return;
                    case specVectorViewSpecialization: pattern.Append(_u("Class(VectorView)")); return;
                    case specPromiseSpecialization: pattern.Append(_u("Class(Promise)")); return;
                    case specPropertyValueSpecialization: pattern.Append(_u("Class(PropertyValue)")); return;
                    case specMapSpecialization: pattern.Append(_u("Class(Map)")); return;
                    case specMapViewSpecialization: pattern.Append(_u("Class(MapView)")); return;
                    default: Js::Throw::FatalProjectionError();
                    }
                }
            }
        case tcDelegateType:
            pattern.Append(_u("Delegate"));
            break;
        case tcStructType:
            pattern.Append(_u("{"));
            StructType::From(type)
                ->fields
                ->IterateBetween(
                    [&](RtABIFIELDPROPERTY field) {AppendTypePattern(field->type, pattern);},
                    [&](RtABIFIELDPROPERTY,RtABIFIELDPROPERTY) {pattern.Append(_u(","));}
            );
            pattern.Append(_u("}"));
            break;
        case tcEnumType:
            {
                auto enumType = EnumType::From(type);
                auto expr = IntermediateExprOfToken(MetadataStringIdNil, enumType->typeDef->td, enumType->typeDef->assembly, nullptr);
                if(Enum::Is(expr))
                {
                    Enum::From(expr)->baseTypeCode == ELEMENT_TYPE_I4 ? pattern.Append(_u("Int32")) : pattern.Append(_u("UInt32"));
                    return;
                }
                Js::Throw::FatalInternalError();
            }
        case tcByRefType:
            AppendTypePattern(ByRefType::From(type)->pointedTo, pattern);
            break;
        case tcMissingNamedType:
            pattern.Append(_u("missing("));
            pattern.Append(stringConverter->StringOfId(MissingNamedType::From(type)->fullTypeNameId));
            pattern.Append(_u(")"));
            break;
        case tcUnprojectableType:
        case tcGenericClassVarType:
        case tcGenericParameterType:
        case tcMissingGenericInstantiationType:
            pattern.Append(_u("uncallable"));
            break;
        default:
            Js::Throw::FatalProjectionError();
        }
    }

    // Info:        Get the call pattern of a signature
    LPCWSTR ProjectionBuilder::GetCallPatternOfSignature(ImmutableList<RtABIPARAMETER> * parameters)
    {
        DefaultImmutableStringBuilder pattern;
        auto appendPattern = [&](RtPARAMETER param) {
            if(param->isIn) pattern.Append(_u("+"));
            if(param->isOut) pattern.Append(_u("-"));
            AppendTypePattern(param->type, pattern);
        };

        parameters->Iterate(appendPattern);

        auto result = pattern.Get<ArenaAllocator>(allocator);
        return result;
    }

    // Info:        Get a name like SomeType<SomeType2,SomeType3>
    // Parameters:  parent - SomeType
    //              genericParameters - SomeType2, SomeType3, etc
    MetadataStringId GetGenericInstantiationNameFromParentName(MetadataStringId parentTypeName, ImmutableList<RtTYPE> * genericParameters, ArenaAllocator * allocator, Metadata::IStringConverter * stringConverter)
    {
        if(genericParameters->IsEmpty())
        {
            return parentTypeName;
        }

        DefaultImmutableStringBuilder instantiatedName;
        instantiatedName.Append(stringConverter->StringOfId(parentTypeName));
        instantiatedName.Append(_u("<"));
        genericParameters->IterateBetween([&](RtTYPE type){
            Js::VerifyCatastrophic(MetadataStringIdNil!=type->fullTypeNameId);
            // If this is a System.Guid type, append the display name used in metadata "System.Guid",
            // for consistency between deconflicted runtimeclass member names obtained from metadata
            // and deconflicted interface member names we generate.
            if (type->typeCode == tcSystemGuidType)
            {
                instantiatedName.Append(_u("System.Guid"));
            }
            else {
                instantiatedName.Append(stringConverter->StringOfId(type->fullTypeNameId));
            }
        }, [&](RtTYPE,RtTYPE){
            instantiatedName.Append(_u(","));
        });
        instantiatedName.Append(_u(">"));
        auto instantiatedNameStr = instantiatedName.Get<HeapAllocator>(&HeapAllocator::Instance);
        auto instantiatedNameId = stringConverter->IdOfString(instantiatedNameStr);
        HeapDeleteArray(wcslen(instantiatedNameStr) + 1, instantiatedNameStr);
        return instantiatedNameId;
    }

    // Info:        Get a name like SomeType<SomeType2,SomeType3>
    // Parameters:  parent - SomeType
    //              genericParameters - SomeType2, SomeType3, etc
    MetadataStringId GetGenericInstantiationNameFromType(RtTYPE parent, ImmutableList<RtTYPE> * genericParameters, ArenaAllocator * a, Metadata::IStringConverter * stringConverter)
    {
        Js::VerifyCatastrophic(TypeDefinitionType::Is(parent) || MissingNamedType::Is(parent));
        return GetGenericInstantiationNameFromParentName(TypeDefinitionType::Is(parent) ? TypeDefinitionType::From(parent)->typeDef->id : MissingNamedType::From(parent)->fullTypeNameId, genericParameters, a, stringConverter);
    }

    // Info:        Given an identifier get the correspond Assignment
    // Parameters:  identifier - the identifer
    // Returns:     Empty if there is no matching Assignment
    Option<Assignment> AssignmentSpace::GetAssignmentByIdentifier(LPCWSTR identifier, ArenaAllocator * a) const
    {
        auto result = vars
                ->WhereToOption<Assignment>([&](RtASSIGNMENT var) {
                    return wcscmp(identifier,var->identifier)==0;
            });

        return result;
    }

    // Info:        Given an identifier get the correspond Property
    // Parameters:  identifier - the identifer
    // Returns:     Empty if there is no matching Property
    Option<Property> PropertiesObject::GetFieldByIdentifier(MetadataStringId identifier, ArenaAllocator * a) const
    {
        auto result = fields
                ->WhereToOption<Property>([&](RtPROPERTY field) {
                    return (identifier == field->identifier);
            });

        return result;
    }

    // Info:        Get the last segment of a dot-delimited string.
    // Parameters:  name - the string.
    // Returns:     A pointer inside name right past the final dot.
    LPCWSTR LastSegmentByDot(LPCWSTR name)
    {
        if(name==nullptr)
        {
            Js::Throw::FatalProjectionError();
        }
        auto dot = wcslen(name)-1;
        while(dot>0 && name[dot]!='.')
        {
            --dot;
        }
        if (dot==0)
        {
            return name;
        }
        return &name[dot+1];
    }

    // Info:        Translate a metadata type into a projection type
    // Parameters:  nativeType - the metadata type
    //              assembly - assembly which holds the native type
    // Returns:     Projected JavaScript type (Number, etc)
    RtTYPE ProjectionBuilder::TypeOfType(const Metadata::Type * nativeType, const Metadata::Assembly & assembly, ImmutableList<RtTYPE> * genericParameters)
    {
        switch(nativeType->typeCode)
        {
            case ELEMENT_TYPE_VOID:
                return typeVoid;
            case ELEMENT_TYPE_CHAR:
                return typeCharProjectedAsString;
            case ELEMENT_TYPE_STRING:
                return typeString;
            case ELEMENT_TYPE_BOOLEAN:
                return typeBool;
            case ELEMENT_TYPE_U1:
                return typeByte;
            case ELEMENT_TYPE_I2:
                return typeInt16;
            case ELEMENT_TYPE_U2:
                return typeUint16;
            case ELEMENT_TYPE_I4:
                return typeInt32;
            case ELEMENT_TYPE_U4:
                return typeUint32;
            case ELEMENT_TYPE_I8:
                return typeInt64;
            case ELEMENT_TYPE_U8:
                return typeUint64;
            case ELEMENT_TYPE_R4:
                return typeFloat;
            case ELEMENT_TYPE_R8:
                return typeDouble;
            case ELEMENT_TYPE_OBJECT:
                return typeObject;
            case ELEMENT_TYPE_VALUETYPE:
                {
                    auto value = static_cast<const Metadata::Value*>(nativeType);
                    return TypeOfToken(value->valueToken, assembly, genericParameters, true /*value type*/);
                }
            case ELEMENT_TYPE_CLASS:
                {
                    auto cls = static_cast<const Metadata::Class*>(nativeType);
                    return TypeOfToken(cls->classToken, assembly, genericParameters);
                }
            case ELEMENT_TYPE_CMOD_OPT:
                {
                    auto modOpt = static_cast<const Metadata::ModOpt*>(nativeType);
                    return TypeOfType(modOpt->modded,assembly,genericParameters);
                }
            case ELEMENT_TYPE_CMOD_REQD:
                {
                    auto modReq = static_cast<const Metadata::ModReqd*>(nativeType);
                    return TypeOfType(modReq->modded,assembly,genericParameters);
                }
            case ELEMENT_TYPE_ARRAY:
                {
                    auto mdArray = static_cast<const Metadata::MdArray*>(nativeType);
                    auto elementType = TypeOfType(mdArray->elementType,assembly,genericParameters);
                    if (Type::IsMissing(elementType))
                    {
                        return elementType;
                    }
                    else
                    {
                        return RtAnew(allocator,ArrayType,arrayTypeNameId,elementType);
                    }
                }
            case ELEMENT_TYPE_SZARRAY:
                {
                    auto szArray = static_cast<const Metadata::SzArray*>(nativeType);
                    auto elementType = TypeOfType(szArray->elementType,assembly,genericParameters);
                    if (Type::IsMissing(elementType))
                    {
                        return elementType;
                    }
                    else
                    {
                        return RtAnew(allocator, ArrayType, arrayTypeNameId, elementType);
                    }
                }
            case ELEMENT_TYPE_PTR:
                {
                    auto ptr = static_cast<const Metadata::Ptr*>(nativeType);
                    return TypeOfType(ptr->pointedTo,assembly,genericParameters);
                }
            case ELEMENT_TYPE_BYREF:
                {
                    auto byRef = static_cast<const Metadata::ByRef*>(nativeType);
                    auto pointedTo = TypeOfType(byRef->pointedTo,assembly,genericParameters);
                    return RtAnew(allocator, ByRefType, byRefTypeNameId, pointedTo);
                }
            case ELEMENT_TYPE_VAR:
                {
                    auto var = static_cast<const Metadata::TVar*>(nativeType);
                    if (genericParameters)
                    {
                        return genericParameters->Nth(var->index);
                    }
                    return RtAnew(allocator, GenericClassVarType, varTypeNameId, var);
                }
            case ELEMENT_TYPE_GENERICINST:
                {
                    auto gi = static_cast<const Metadata::GenericInstantiation*>(nativeType);
                    auto gps = gi->genericParameters->Select<RtTYPE>([&](const Metadata::Type * type) {
                        return TypeOfType(type,assembly,genericParameters);
                    },allocator);
                    auto type = TypeOfType(gi->parentType,assembly,gps);

                    if(TypeDefinitionType::Is(type))
                    {
                        return type;
                    }
                    else
                    {
                        MetadataStringId fullNameId = GetGenericInstantiationNameFromType(type, gps, allocator, stringConverter);
                        auto missingNamedType = MissingNamedType::From(type);
                        return RtAnew(allocator, MissingGenericInstantiationType, missingNamedType, gps, fullNameId);
                    }
                }
            default:
                return typeUnprojectable;
        }

    }

    MetadataStringId ProjectionBuilder::GetBasicTypeIdFromTypeCor(CorElementType typeCor)
    {
        switch(typeCor)
        {
            case ELEMENT_TYPE_CHAR: return char16TypeId;
            case ELEMENT_TYPE_STRING:  return stringTypeId;
            case ELEMENT_TYPE_BOOLEAN: return booleanTypeId;
            case ELEMENT_TYPE_U1: return uint8TypeId;
            case ELEMENT_TYPE_I2: return int16TypeId;
            case ELEMENT_TYPE_U2: return uint16TypeId;
            case ELEMENT_TYPE_I4: return int32TypeId;
            case ELEMENT_TYPE_U4: return uint32TypeId;
            case ELEMENT_TYPE_I8: return int64TypeId;
            case ELEMENT_TYPE_U8: return uint64TypeId;
            case ELEMENT_TYPE_R4: return singleTypeId;
            case ELEMENT_TYPE_R8: return doubleTypeId;
            case ELEMENT_TYPE_OBJECT: return objectTypeId;
            case ELEMENT_TYPE_VAR: return varTypeNameId;
            default: Js::Throw::FatalProjectionError();
        }
    }

    RtBASICTYPE ProjectionBuilder::GetBasicType(CorElementType metadataType) const
    {
        switch(metadataType)
        {
            case ELEMENT_TYPE_CHAR: return this->typeCharProjectedAsString;
            case ELEMENT_TYPE_STRING:  return this->typeString;
            case ELEMENT_TYPE_BOOLEAN: return this->typeBool;
            case ELEMENT_TYPE_U1: return this->typeByte;
            case ELEMENT_TYPE_I2: return this->typeInt16;
            case ELEMENT_TYPE_U2: return this->typeUint16;
            case ELEMENT_TYPE_I4: return this->typeInt32;
            case ELEMENT_TYPE_U4: return this->typeUint32;
            case ELEMENT_TYPE_I8: return this->typeInt64;
            case ELEMENT_TYPE_U8: return this->typeUint64;
            case ELEMENT_TYPE_R4: return this->typeFloat;
            case ELEMENT_TYPE_R8: return this->typeDouble;
            case ELEMENT_TYPE_OBJECT: return this->typeObject;
            default: Js::Throw::FatalProjectionError();
        }
    }

    // Info:        Get a basic type (like int, string, etc)
    // Parameters:  metadataType - the type code
    RtBASICTYPE ProjectionBuilder::TypeOfBasicType(CorElementType metadataType)
    {
        auto size = Metadata::Assembly::GetBasicTypeSize(metadataType);
        auto naturalAlignment = Metadata::Assembly::GetBasicTypeAlignment(metadataType);
        auto result = RtAnew(allocator,BasicType,GetBasicTypeIdFromTypeCor(metadataType),metadataType,::Math::Align<uint>((uint32)size, (uint32)sizeof(LPVOID)), (UINT32)size, naturalAlignment); /* Type sizes always within UINT_MAX range */

        return result;
    }

    // Info:        Get a concrete type (will assert if not concrete)
    // Parameters:  nativeType - the native type to use
    const ConcreteType * ProjectionBuilder::ConcreteTypeOfType(const Metadata::Type * nativeType, const Metadata::Assembly & assembly, ImmutableList<RtTYPE> * genericParameters, bool* isWebHidden)
    {
        auto type = TypeOfType(nativeType,assembly,genericParameters);
        auto result = ConcreteType::From(type);
        if (isWebHidden != nullptr)
        {
            if (MissingNamedType::Is(type))
            {
                *isWebHidden = MissingNamedType::From(type)->isWebHidden;
            }
        }
        return result;
    }

    // Info:        Rule for decoding in\out values
    void DecodeInOut(bool propsIn, bool propsOut, bool isArray, bool * in, bool * out)
    {
        // Special handling for arrays
        //
        //                  To Parameter    | From Metadata
        //                  isIn    isOut   |   isIn    isOut   isByRef
        // FillArray        1        1        |   0        1        0               => Should behave like PassFillArray that is marshal in and out contents
        // ReceiveArray        0        1        |   0        1        1
        // PassArray        1        0        |   1        0        0

        *out = propsOut;
        *in = propsIn;
        if (!(*in) && !(*out))
        {
            *in = true;
        }
        if (isArray && *out) // FillArrayPattern - should convert to PassFillArray which is inout Array type
        {
            *in = true;
        }
    }

    // Info:        Translate a metadata parameter into a projection parameter
    // Parameters:  pt - token of parameter def
    //              parameter - parameter properties
    //              assembly - assembly which holds the native parameter
    //              genericParameters - generic parameters for the type
    //              inParameterIndex - the index in the inputParameterList - Need to be valid only for inParam
    //              hasRetVal - method hasReturnType ([retVal] attribute parameter)
    //              retValIndex - index of retVal parameter
    // Returns:     Projected JavaScript parameter
    RtABIPARAMETER ProjectionBuilder::ParameterOfParameter(
        const Metadata::ParameterProperties *props,
        const Metadata::Parameter * parameter,
        const Metadata::Assembly & assembly,
        ImmutableList<RtTYPE> * genericParameters,
        size_t inParameterIndex,
        size_t retValIndex)
    {
        AssertMsg(retValIndex < 65536, "Invalid metadata: ECMA-335 II.22.33: Param indices are encoded as 2-byte integers.");

        auto type =
            parameter->typedByRef
            ? typeObject
            : TypeOfType(parameter->type.GetValue(), assembly, genericParameters);

        bool isOut;
        bool isIn;
        auto typeCode =
            parameter->typedByRef
            ? ELEMENT_TYPE_OBJECT
            : parameter->type.GetValue()->typeCode;
        DecodeInOut(props->IsIn(), props->IsOut(), typeCode==ELEMENT_TYPE_ARRAY || typeCode == ELEMENT_TYPE_SZARRAY, &isIn, &isOut);

        if (ArrayType::Is(type) || (ByRefType::Is(type) && ArrayType::Is(ByRefType::From(type)->pointedTo)))
        {
            INT32 lengthIsParamterSequence = 0;
            if (props->assembly.GetInt32Attribute(props->pt, _u("Windows.Foundation.Metadata.LengthIsAttribute"), lengthIsParamterSequence))
            {
                // Array type with LengthIs parameter

                // the lengthIsParameterSequence is :
                // 0 if lengthIs target has retVal attribute specified
                // 1, 2, 3 ... n for first 1 - n parameters

                if (lengthIsParamterSequence == 0)
                {
                    lengthIsParamterSequence = (INT32)retValIndex; /* ECMA-335 II.22.33: Param indices are 0 ... 65535 */
                }
                else
                {
                    lengthIsParamterSequence--;
                }
                return Anew(allocator, AbiArrayParameterWithLengthAttribute, props->id, type, isIn, isOut, inParameterIndex, lengthIsParamterSequence);
            }
        }

        return Anew(allocator, AbiParameter, props->id, type, isIn, isOut, inParameterIndex);
    }

    // Info:        Give a set of parameters, construct a return type
    // Parameters:  outParameters - the parameters
    RtTYPE ProjectionBuilder::ReturnTypeOfOutParameters(ImmutableList<RtABIPARAMETER> * outParameters, const Metadata::Assembly & assembly)
    {
        if(outParameters->IsEmpty())
        {
            return typeVoid;
        }

        if (outParameters->Count()==1)
        {
            return outParameters->First()->type;
        }

        return typeMultiOut;
    }

    // Info:        Get the size of the stack for calling this abi method.
    // Parameters:  parameters - method parameters
    size_t GetSizeOfCallStack(ImmutableList<RtABIPARAMETER> * parameters)
    {
        size_t size = 0; // Does not count 'this'
        parameters->Iterate([&](RtPARAMETER parameter) {
            if(size!=0xffffffff && AbiParameter::Is(parameter) && ConcreteType::Is(parameter->type))
            {
                size += AbiParameter::From(parameter)->GetSizeOnStack();
            }
            else
            {
                size = 0xffffffff;
            }
        });
        return size;
    }

    // Info:        Translate a metadata method into a method signature
    // Parameters:  method - native method to project
    //              assembly - assembly which holds the native method
    // Returns:     Projected JavaScript function
    RtABIMETHODSIGNATURE ProjectionBuilder::MethodSignatureOfMethod(const Metadata::TypeDefProperties * parentType, RtIID iid, const Metadata::MethodProperties * method, ImmutableList<RtTYPE> * genericParameters, MethodKind methodKind)
    {
        Metadata::Type * returnMetadataType = nullptr;
        MetadataStringId returnTypeNameId = MetadataStringIdNil;

        auto parameters = method->GetParameters();

        if (method->signature->returnType->typeCode != ELEMENT_TYPE_VOID)
        {
            returnMetadataType = method->signature->returnType;
            returnTypeNameId = returnValueId;

            if (parameters->IsEmpty())
            {
                // Saw this case in IVector<T>::GetView()--I don't think there is a parameter token for the return value.
            }
            else
            {
                auto props = parameters->First();

                if (props->sequence==0)
                {
                    returnMetadataType = method->signature->returnType;
                    returnTypeNameId = props->id;
                    parameters = parameters->GetTail();
                }
            }
        }

        size_t computedInParameterCount = 0;

        parameters
        ->IterateWith(method->signature->parameters,
            [&](const Metadata::ParameterProperties* props, const Metadata::Parameter* parameter) {
                bool isOut;
                bool isIn;
                auto typeCode = parameter->type.GetValue()->typeCode;
                DecodeInOut(props->IsIn(), props->IsOut(), typeCode==ELEMENT_TYPE_ARRAY || typeCode == ELEMENT_TYPE_SZARRAY, &isIn, &isOut);
                if (isIn)
                {
                    ++computedInParameterCount;
                }
        });

        auto isDefaultOverload = method->isDefaultOverload;
        auto uniqueName = (method->overloadNameId == MetadataStringIdNil ? NULL : stringConverter->StringOfId(method->overloadNameId));

        CustomAttributeInfo customAttributeInfo;
        auto typeDefProps = method->assembly.GetTypeDefProperties(method->classToken);

        AnalyzeMethodCustomAttributes(method->mb, typeDefProps->id, &customAttributeInfo, method->assembly);
        ImmutableList<DeprecatedAttribute>* deprecatedAttributes = nullptr;
        if (customAttributeInfo.isDeprecated)
        {
            deprecatedAttributes = customAttributeInfo.deprecatedAttributes;
            if (parentType != nullptr)
            {
                // If we have deprecated attribute in the method, goes up to interface to see if it has excluesiveto attribute, if yes,
                // find the rtcName, and later on we'll print out rtcName instead of interface name.
                auto rtcNameId = AnalyzeInterfaceCustomAttributes(parentType->td, parentType->assembly);
                // TODO: The iterator takes the object instead of reference. we should change the iterator to take/pass & vnext.
                if (rtcNameId != MetadataStringIdNil)
                {
                    TRACE_METADATA(_u("found exclusiveto %s"), stringConverter->StringOfId(rtcNameId));
                    deprecatedAttributes->Iterate([&](DeprecatedAttribute& deprecatedAttribute)
                    {
                        deprecatedAttribute.rtcNameId = rtcNameId;
                    });
                }
            }
        }
        auto continuation = Anew(allocator, ReadSignatureContinuation, this, method, genericParameters, parameters, returnMetadataType, returnTypeNameId);
        auto signature = Anew(allocator, AbiMethodSignature, method->id, uniqueName, iid, method->methodIndex,
            computedInParameterCount,
            isDefaultOverload, MetadataStringIdNil, method->id,
            nullptr,
            continuation,
            deprecatedAttributes,
            methodKind
            );

        return signature;
    }

    // Info:        Lazy retrieval for parameters. This allows deferring assembly loads.
    RtPARAMETERS AbiMethodSignature::GetParameters() const
    {
#if DBG
        Js::VerifyCatastrophic(allowHeavyOperation);
#endif
        if (parameters==nullptr)
        {
            auto parameterProperties = continuation->parameterProperties;
            auto returnTypeNameId = continuation->returnTypeNameId;
            auto builder = continuation->builder;
            auto method = continuation->method;
            auto genericParameters = continuation->genericParameters;
            auto allocator = builder->allocator;
            size_t inParameterCount = 0;
            auto outParameters = ImmutableList<RtABIPARAMETER>::Empty();
            auto outParametersTail = outParameters;
            auto allParameters = ImmutableList<RtABIPARAMETER>::Empty();
            auto allParametersTail = allParameters;
            auto returnParameter = ImmutableList<RtABIPARAMETER>::Empty(); // This is the parameter that is in the return position in metadata and JS, but in the last parameter position in the WinRT class.

            size_t retValIndex = 0; // Initialize to zero and calculate only if we have return type
            if (continuation->returnMetadataType)
            {
                auto returnType = builder->TypeOfType(continuation->returnMetadataType, method->assembly, genericParameters);
                returnType = RtAnew(allocator, ByRefType, builder->byRefTypeNameId, returnType);

                // Since returnType is outParam we realy arent going to be using its inParameterIndex
                returnParameter = returnParameter->Prepend(Anew(allocator, AbiParameter, returnTypeNameId, returnType, false, true, 0),allocator);
                retValIndex = parameterProperties->Count();
            }

            parameterProperties
            ->IterateWith(method->signature->parameters,
                [&](const Metadata::ParameterProperties *props, const Metadata::Parameter* parameter) {
                    auto jsParameter = builder->ParameterOfParameter(props,parameter,method->assembly, genericParameters, inParameterCount, retValIndex);
                    allParameters = allParameters->Append(jsParameter, allocator, &allParametersTail);
                    if (jsParameter->isIn)
                    {
                        inParameterCount++;
                    }
                    else
                    {
                        outParameters = outParameters->Append(jsParameter, allocator, &outParametersTail);
                    }
            });

            allParameters = allParameters->AppendListToCurrentList(returnParameter, allParametersTail);
            outParameters = outParameters->AppendListToCurrentList(returnParameter, outParametersTail);

            auto returnType = builder->ReturnTypeOfOutParameters(outParameters,method->assembly);
            auto callSizeOnStack = GetSizeOfCallStack(allParameters);
            auto callPattern = builder->GetCallPatternOfSignature(allParameters);
            Js::VerifyCatastrophic(this->inParameterCount == inParameterCount);
            // -------------------------------------------------------------------------------------------------------------------
            // Breaking constness but laziness still preserves immutability semantic and value remains cacheable
            auto mthis = const_cast<AbiMethodSignature*>(this);
            mthis->parameters = Anew(allocator, Parameters, allParameters->Cast<RtPARAMETER>(), returnType, callSizeOnStack, callPattern);
            mthis->continuation = nullptr;
            // -------------------------------------------------------------------------------------------------------------------
        }
        return parameters;
    }

    // Info:        Given a maximum arity, produce a sequence of overload parameters
    // Parameters:  maxArity - the maximum arity
    // Returns:     List of parameters
    ImmutableList<RtSYNTHETICPARAMETER> * ProjectionBuilder::SyntheticOverloadParameters(size_t maxArity)
    {
        AssertMsg(maxArity < 65536, "Invalid metadata: ECMA-335 II.22.33: Param indices are encoded as 2-byte integers.");
        auto parameters = ImmutableList<RtSYNTHETICPARAMETER>::Empty();
        for(size_t i = 0; i<maxArity; ++i)
        {
            LPCWSTR base = _u("param");
            char16 scratch[20];
            _itow_s((int)maxArity - (int)i, scratch, 20, 10); /* ECMA-335 II.22.33: Arity constrained to 65535, conversion to int is safe */
            char16 paramName[25];
            swprintf_s(paramName, 25, _u("%s%s"), base, scratch);
            MetadataStringId paramNameId = stringConverter->IdOfString(paramName);

            auto parameter = Anew(allocator, SyntheticParameter, paramNameId, typeObject);
            parameters = parameters->Prepend(parameter, allocator);
        }
        return parameters;
    }

    // Info:        Create an overloaded method signature from an overload group.
    // Parameters:  overloadGroup - the overload group
    RtOVERLOADEDMETHODSIGNATURE ProjectionBuilder::OverloadedMethodSignatureOfOverloadGroup(const OverloadGroup * overloadGroup)
    {
        // Find max arity
        auto maxArity =
            (int)overloadGroup->overloads->Accumulate((size_t)0,[](size_t maxSoFar, RtABIMETHODSIGNATURE overload)->size_t {
                AssertMsg(overload->inParameterCount < 65536, "Invalid metadata: ECMA-335 II.22.33: Param indices are encoded as 2-byte integers.");
                return (size_t)max(maxSoFar,overload->inParameterCount);
        });

        auto parameters = SyntheticOverloadParameters(maxArity)->Cast<RtPARAMETER>();
        auto result = Anew(allocator,OverloadedMethodSignature,overloadGroup->id,overloadGroup,Anew(allocator, Parameters, parameters, typeObject));

        return result;
    }

    // Info:        Return a list with one function length property in it
    // Parameters:  length - the length
    ImmutableList<RtPROPERTY> * FunctionLengthPropertiesOfInt(int length, MetadataStringId lengthId, ArenaAllocator * allocator)
    {
        auto lengthProperty = RtAnew(allocator, FunctionLengthProperty, lengthId, Anew(allocator, Int32Literal, length)); // TODO: Could precreate 0-10 to reduce memory
        auto result = ToImmutableList<Property>(lengthProperty, allocator);

        return result;
    }

    // Info:        Return a properties object with one length property in it
    // Parameters:  length - the length
    RtPROPERTIESOBJECT FunctionLengthPropertiesObjectOfInt(int length, MetadataStringId lengthId, ArenaAllocator * allocator)
    {
        auto propertiesList = FunctionLengthPropertiesOfInt(length, lengthId, allocator);
        auto result = Anew(allocator, PropertiesObject, propertiesList);

        return result;
    }

    // Info:        Given a metadata overload group, construct a Property
    // Parameters:  metadataOverloadGroup - the metadataOverloadGroup
    // Returns:     Empty if there is no Property that should be projected
    RtPROPERTY ProjectionBuilder::FieldOfMetadataOverloadGroup(RtIID iid, const MetadataOverloadGroup * metadataOverloadGroup, const Metadata::TypeDefProperties * parentType, ImmutableList<RtTYPE> * genericParameters)
    {
        RtPROPERTY result;

        if (metadataOverloadGroup->arityGroups->Count()==1 &&
            metadataOverloadGroup->arityGroups->First()->methods->Count()==1)
        {
            result = PropertyOfMethod(metadataOverloadGroup->id, parentType, iid,
                metadataOverloadGroup->arityGroups->First()->methods->First(),genericParameters);
        }
        else
        {
            auto overloadGroup = OverloadGroupOfMetadataOverloadGroup(iid,metadataOverloadGroup, parentType->assembly, genericParameters);
            auto signature = OverloadedMethodSignatureOfOverloadGroup(overloadGroup);
            auto minimumArity = overloadGroup->overloads->Last()->inParameterCount; // Last is minimum because list is sorted descending by arity
            AssertMsg(minimumArity < 65536, "Invalid metadata: Maximum arity is 65535 (metadata specifies parameter name by 2-byte index)");
            auto properties = FunctionLengthPropertiesObjectOfInt((int)minimumArity, lengthId, allocator);
            auto function = Anew(allocator, OverloadGroupConstructor, signature, properties);

            result = RtAnew(allocator, OverloadParentProperty, overloadGroup->id, function);
        }

        return result;
    }

    // Info:        Translate a metadata method into a projection property
    // Parameters:  id - property name id of the parameter
    //              iid - the iid of the method
    //              method - the metadata method
    //              genericParameters - generic parameters to instantiate the method
    // Returns:     The property representing the method
    RtABIMETHODPROPERTY ProjectionBuilder::PropertyOfMethod(MetadataStringId id, const Metadata::TypeDefProperties * parentType, RtIID iid, const Metadata::MethodProperties * method, ImmutableList<RtTYPE> * genericParameters)
    {
        auto signature = MethodSignatureOfMethod(parentType, iid, method,genericParameters);
        AssertMsg(signature->inParameterCount < 65536, "Invalid metadata: Maximum arity is 65535 (metadata specifies parameter name by 2-byte index)");
        auto properties = FunctionLengthPropertiesObjectOfInt((int)signature->inParameterCount, lengthId, allocator);
        auto function = Anew(allocator, AbiMethod, signature, properties);
        auto result = RtAnew(allocator, AbiMethodProperty, id, function);

        return result;
    }

    // Info:        Translate a literal field blob into an expr.
    // Parameters:  typeFlag - a CorTypeFlags flag which describes the field type
    //              blob - the blob
    // Returns:     An expr representing the literal
    RtEXPR ProjectionBuilder::LiteralOfFieldBlob(DWORD typeFlag, UVCP_CONSTANT blob)
    {
        switch(typeFlag)
        {
        case ELEMENT_TYPE_I4:
            {
                return Anew(allocator, Int32Literal, *reinterpret_cast<int const *>(blob));
            }
            break;
        case ELEMENT_TYPE_U4:
            {
                return Anew(allocator, UInt32Literal, *reinterpret_cast<unsigned int const *>(blob));
            }
            break;
        }
        Js::Throw::FatalProjectionError();
    }

    // Info:        Project metadata field properties
    // Parameters:  fieldProps - the parameters
    //              fieldOffset - the index of the field
    RtABIFIELDPROPERTY ProjectionBuilder::PropertyOfFieldProperties(const Metadata::FieldProperties * fieldProps, size_t fieldOffset, ImmutableList<RtTYPE> * genericParameters)
    {
        auto type = ConcreteTypeOfType(fieldProps->type, fieldProps->assembly, genericParameters);

        if (fieldProps->IsLiteral())
        {
            // This is likely an enum value.
            auto literalExpr = LiteralOfFieldBlob(fieldProps->dwCPlusTypeFlag, fieldProps->constantValue);
            return RtAnew(allocator, AbiFieldProperty, fieldProps->id, literalExpr, type, fieldProps, fieldOffset);
        }

        // This is likely a struct field.
        return RtAnew(allocator, AbiFieldProperty, fieldProps->id, exprNull, type, fieldProps, fieldOffset);
    }

    // Info:        Convert a metadata typedef into method signature that will construct that type
    // Parameters:  typeDef - metadata type definition
    const TypeConstructorMethodSignature * ProjectionBuilder::MethodSignatureOfType(const Metadata::TypeDefProperties * typeDef, ImmutableList<RtTYPE> * genericParameters)
    {
        auto returnType = TypeOfToken(typeDef->td, typeDef->assembly, genericParameters);
        auto result = Anew(allocator, TypeConstructorMethodSignature, ctorId, Anew(allocator, Parameters, nullptr, returnType));

        return result;
    }

    // Info:        Get a constructor that is known to be a struct
    // Parameters:  type - metadata props
    RtEXPR ProjectionBuilder::ConstructorOfStruct(const Metadata::TypeDefProperties * typeDef)
    {
        if (this->IsTypeWebHidden(typeDef)
            || !IsWithinTargetVersion(typeDef->td, typeDef->assembly))
        {
            return ConstructorOfMissingTypeDef(typeDef, true);
        }

        auto type = StructTypeOfTypeDef(typeDef);

        if (Type::IsMissing(type))
        {
            // This struct should be hidden
            return ConstructorOfMissingNamedType(MissingNamedType::From(type));
        }
        else
        {
            auto typeSignature = MethodSignatureOfType(typeDef, nullptr);
            auto structType = StructType::From(type);
            auto structConstructor = Anew(allocator, StructConstructor, structType->typeId, typeSignature, emptyPropertiesObject, structType);

            if (CanMarshalExpr(structConstructor, true))
            {
                return structConstructor;
            }

            return ConstructorOfMissingTypeDef(typeDef);
        }
    }

    // Info:        Concatenate a path, then a dot, then an identifier
    // Parameters:  path - the path
    //              name - the identifier name
    MetadataStringId PathDotName(MetadataStringId pathId, MetadataStringId nameId, ArenaAllocator * a, Metadata::IStringConverter * stringConverter)
    {
        LPCWSTR path = stringConverter->StringOfId(pathId);
        LPCWSTR name = stringConverter->StringOfId(nameId);
        size_t qualifiedNameLen = wcslen(path) + wcslen(name) + 2;
        AutoHeapString qualifiedName;
        qualifiedName.CreateNew(qualifiedNameLen);
        swprintf_s(qualifiedName.Get(), qualifiedName.GetLength(), _u("%s%c%s"), path, _u('.'), name);
        return stringConverter->IdOfString(qualifiedName.Get());
    }

    // Info:        Given a type that is known to be an enum, return a Function.
    // Parameters:  type - the enum
    // Returns:     a Function
    RtEXPR ProjectionBuilder::EnumOfEnum(const Metadata::TypeDefProperties * type)
    {
        if (this->IsTypeWebHidden(type)
            || !IsWithinTargetVersion(type->td, type->assembly))
        {
            // This enum should be hidden
            return ConstructorOfMissingTypeDef(type, true);
        }

        auto enumMembers = type->assembly.Members(type->td);

        // Get the enum base type from value__ field
        auto baseTypeMember =
            enumMembers->WhereSingle([&](const Metadata::MemberProperties * member) {
                    return member->IsSpecialNameField() || member->IsRTSpecialNameField();
                });

        Js::VerifyCatastrophic(baseTypeMember != nullptr);

        auto baseTypeField = type->assembly.GetFieldProperties(baseTypeMember->mt);

        auto fields =
            enumMembers->Where([&](const Metadata::MemberProperties * member) -> bool {
                return member->IsLiteralField() && IsWithinTargetVersion(member->mt, member->assembly);
            },allocator)->Select<RtPROPERTY>([&](const Metadata::MemberProperties* member)->RtPROPERTY {
                auto fieldProps = member->assembly.GetFieldProperties(member->mt);
                return PropertyOfFieldProperties(fieldProps, 0xfffffff,nullptr);
            },allocator);

        auto conflictsWithBuiltIn = [&](RtPROPERTY a)->bool{
            return builtInInstanceProperties->ContainsWhere([&](MetadataStringId id) {
                return (id == a->identifier);
            });
        };

        // Deconflict a list of fields.
        auto deconflict = [&](MetadataStringId fullTypeNameId, ImmutableList<RtPROPERTY> * fields) -> ImmutableList<RtPROPERTY> * {
            auto alloc=allocator;
            auto strConverter = stringConverter;
            return fields->SelectInPlace([&](RtPROPERTY prop) -> RtPROPERTY {
                if (conflictsWithBuiltIn(prop))
                {
                    return CreateRenamedPropertyIdentifierAsCopy(prop, PathDotName(fullTypeNameId, prop->identifier, alloc, strConverter), alloc);
                }
                return prop;});
        };

        fields = CamelCaseProperties(fields, builtInInstanceProperties);
        fields = deconflict(type->id, fields);

        auto body = Anew(allocator, PropertiesObject, fields);
        auto result = Anew(allocator, Enum, type->id, type, body, (CorElementType)baseTypeField->type->typeCode);

        return result;
    }

    // Info:        Get a constructor that is known to be a basic type
    // Parameters:  basicType - basic type
    RtFUNCTION ProjectionBuilder::ConstructorOfBasicType(RtBASICTYPE basicType)
    {
        auto signature = Anew(allocator, TypeConstructorMethodSignature, ctorId, Anew(allocator, Parameters, nullptr, basicType));
        auto result = Anew(allocator, BasicTypeConstructor, signature);

        return result;
    }

    // Info:        Given a set of methods that all have the same arity. Choose the one which is the default. If no default then choose the first
    // Parameters:  groupOfSameArity - group of methods
    inline RtABIMETHODSIGNATURE ChooseSingleOverloadInGroupWithSameArity(ImmutableList<RtABIMETHODSIGNATURE> * groupOfSameArity, ArenaAllocator * a)
    {
        auto signaturesWithDefaultOverload =
            groupOfSameArity->WhereInPlace([&](RtABIMETHODSIGNATURE method) {
                return method->hasDefaultOverloadAttribute;
        });

        // Take the first default overload if there is one, otherwise pick a random overload.
        // RIDL is supposed to enforce that there is exactly one overload per arity.
        auto choice =
            (!signaturesWithDefaultOverload->IsEmpty())
            ? signaturesWithDefaultOverload->First()
            : groupOfSameArity->First();

        return choice;
    }

    // Info:        Return a projected arity group for the given MetadataArityGroup
    // Parameters:  metadataArityGroup - the metadata arity group
    // Returns:     the resulting ArityGroup
    RtABIMETHODSIGNATURE ProjectionBuilder::MethodSignatureOfMetadataArityGroup(RtIID iid, const MetadataArityGroup * metadataArityGroup, const Metadata::Assembly & assembly, ImmutableList<RtTYPE> * genericParameters)
    {
        auto signatures =
            metadataArityGroup->methods
            ->Select<RtABIMETHODSIGNATURE>([&](const Metadata::MethodProperties * method) {
                // when there is need for dis-ambiguity, use the interface name to be more precise.
                return MethodSignatureOfMethod(nullptr, iid, method, genericParameters);
        },allocator);

        auto result = ChooseSingleOverloadInGroupWithSameArity(signatures, allocator);

        return result;
    }

    // Info:        Return a projected overload group for the given MetadataOverloadGroup
    // Parameters:  metadataOverloadGroup - the metadata overload group
    // Returns:     the resulting ArityGroup
    RtOVERLOADGROUP ProjectionBuilder::OverloadGroupOfMetadataOverloadGroup(RtIID iid, const MetadataOverloadGroup * metadataOverloadGroup, const Metadata::Assembly & assembly, ImmutableList<RtTYPE> * genericParameters)
    {
        auto overloads =
            metadataOverloadGroup->arityGroups
            ->Select<RtABIMETHODSIGNATURE>([&](const MetadataArityGroup * arityGroup) {
                return MethodSignatureOfMetadataArityGroup(iid,arityGroup,assembly,genericParameters);
        },allocator)
            ->ReverseSortCurrentList(&CompareRefArities::Instance); // Leave the overload group ordered by descending arity

        auto result = Anew(allocator, OverloadGroup, metadataOverloadGroup->id, overloads);

        return result;
    }

    // Group methods by name
    struct MetadataOverloadGroupById
    {
        MetadataStringId id;
        ImmutableList<const Metadata::MethodProperties*> * methods;
        MetadataOverloadGroupById(ImmutableList<const Metadata::MethodProperties*> * methods) : methods(methods)
        {
            id = methods->First()->id;
        }
    };

    // Info:        Get the 'requires' interfaces for the given metadata
    // Parameters:  token - token
    ImmutableList<RtINTERFACECONSTRUCTOR> * ProjectionBuilder::ImplementedInterfaceConstructors(mdToken token, const Metadata::Assembly & assembly, ImmutableList<RtTYPE> * genericParameters, Option<InterfaceConstructor> * defaultInterface)
    {
        if (mdtTypeDef == TypeFromToken(token))
        {
            auto type = assembly.GetTypeDefProperties(token);
            Js::VerifyCatastrophic(type != nullptr);

            CheckForDuplicateTypeId checker(&implementedInterfaceConstructorsCheck);
            if (checker.AddTypeId(type->id, genericParameters, allocator, this->stringConverter) )
            {
                OUTPUT_TRACE(ProjectionMetadataPhase, _u("Detected duplicate type %s (#%d)\n"), this->stringConverter->StringOfId(type->id), type->id);
            }

            auto interfacesImplemented = assembly.InterfacesImplemented(token);

            auto result =
                    interfacesImplemented->SelectNotNull<RtINTERFACECONSTRUCTOR>([&](mdInterfaceImpl ii)->RtINTERFACECONSTRUCTOR {
                        if (!IsWithinTargetVersion(ii, assembly))
                        {
                            OUTPUT_TRACE(Js::ProjectionMetadataPhase,
                                _u("IsWithinTargetVersion(%s (#%d), mdInterfaceImpl=0x%04X) == false\n"),
                                this->stringConverter->StringOfId(type->id),
                                type->id,
                                ii);
                            return nullptr;
                        }

                        auto props = assembly.GetInterfaceImplProperties(ii);
                        RtINTERFACECONSTRUCTOR constructor = InterfaceConstructorOfToken(props->interfaceToken,assembly,genericParameters);
                        Js::VerifyCatastrophic(constructor->functionType == functionInterfaceConstructor);

                        auto constructedInterface = InterfaceConstructor::From(constructor);

                        if (nullptr != defaultInterface)
                        {
                            // See if this is the default interface
                            if (assembly.IsAttributePresent(ii, _u("Windows.Foundation.Metadata.DefaultAttribute")))
                            {
                                *defaultInterface = constructedInterface;
                            }
                        }

                        return constructedInterface;
            },allocator);

            return result;
        }
        else if (mdtTypeRef == TypeFromToken(token))
        {
            auto result = DoWithTypeFromOtherAssembly<ImmutableList<RtINTERFACECONSTRUCTOR>*>(assembly, token,
                [&](const Metadata::Assembly & otherAssembly, mdTypeDef td) { return ImplementedInterfaceConstructors(td, otherAssembly,genericParameters, defaultInterface);},
                // Missing type case
                [&](LPCWSTR typeName) -> ImmutableList<RtINTERFACECONSTRUCTOR>* {
                    auto typeId = stringConverter->IdOfString(typeName);
                    auto constructor = Anew(allocator, MissingInterfaceConstructor, typeId, uncallableMethodSignature, emptyPropertiesObject);
                    return ToImmutableList(InterfaceConstructor::From(constructor),allocator);
            }
            );

            return result;
        }
        else
        {
            Js::Throw::FatalProjectionError();
        }
    }

    // Info:        Get the specialization IID from the closure of interfaces.
    //              Assumes distinctClosure has only unique interfaces. Returns GUID_NULL if there is 0 or 2+ specializations
    // Parameters:  distinctClosure - the initial set of interfaces
    template<class TF>
    RtIID GetSpecializationIID(RtIID parentIID, ImmutableList<RtINTERFACECONSTRUCTOR> * distinctClosure, ArenaAllocator * a)
    {
        RtIID specializationIID = nullptr;
        bool multipleSpecialization = false;

        // Determine whether two identifiers are the same.
        auto setIfSpecializationIID = [&](RtIID iid) -> bool {
            if (iid && TF::IsSpecializationParameterizedIID(iid->piid))
            {
                if (specializationIID == nullptr)
                {
                    specializationIID = iid;
                }
                else
                {
                    // more than 1 specilization on single type
                    specializationIID = nullptr;
                    multipleSpecialization = true;
                }
            }

            return !multipleSpecialization;
        };

        setIfSpecializationIID(parentIID);
        distinctClosure->IterateWhile([&](RtINTERFACECONSTRUCTOR iface)-> bool {
            if (RuntimeInterfaceConstructor::Is(iface))
            {
                return setIfSpecializationIID(RuntimeInterfaceConstructor::From(iface)->iid);
            }
            return true;
        });
        return specializationIID;
    }

    // Info:        Get the closure of all interfaces
    // Parameters:  initial - the initial set of interfaces
    ImmutableList<RtINTERFACECONSTRUCTOR> * GetInterfaceClosure(ImmutableList<RtINTERFACECONSTRUCTOR> * initial, ArenaAllocator * allocator)
    {
        auto known = ImmutableList<RtINTERFACECONSTRUCTOR>::Empty();

        SList<ImmutableList<RtINTERFACECONSTRUCTOR> *, HeapAllocator> notYetVisited(&HeapAllocator::Instance);
        notYetVisited.Push(initial);
        while(!notYetVisited.Empty())
        {
            auto currentList = notYetVisited.Head();
            if (currentList->IsEmpty())
            {
                notYetVisited.Pop();
                continue;
            }

            auto current = currentList->First();
            notYetVisited.Head() = currentList->GetTail();

            if(!known->ContainsWhere([&](RtINTERFACECONSTRUCTOR known){
                return AreSameInterfaceConstructor(known, current);
            }))
            {
                known = known->Prepend(current, allocator);
                auto directRequired = current->requiredInterfaces;
                notYetVisited.Push(directRequired);
            }
        }
        return known;
    }

    // Info:        Find a matching method signautre in the given set of interfaces and extra parent properties
    // Parameters:  name - the method name
    //              instantiated - the instantiated IID to search for
    //              interfaces - the interfaces to search
    //              parentProperties - the parent properties to search
    RtABIMETHODSIGNATURE FindMethodSignature(MetadataStringId nameId, const IID & instantiated, ImmutableList<RtINTERFACECONSTRUCTOR> * interfaces, ImmutableList<RtPROPERTY> * parentProperties, ArenaAllocator * allocator)
    {
        auto matching = FindMatchingMethodSignatures(nameId, instantiated, parentProperties, allocator);
        auto next = interfaces;
        while(matching->IsEmpty() && next)
        {
            matching = FindMatchingMethodSignatures(nameId, instantiated, next->First()->ownProperties, allocator);
            next = next->GetTail();
        }
        if (matching->Count()==0)
        {
            return nullptr;
        }
        return matching->First();
    }

    // Info:        Get the specialization for the interface
    RtSPECIALIZATION ProjectionBuilder::GetSpecialization(ImmutableList<RtPROPERTY> * parentProperties, ImmutableList<RtINTERFACECONSTRUCTOR> * interfaces, RtIID parentIID)
    {
        auto specializationIID = GetSpecializationIID<SpecializationParameterizedIIDFinder>(parentIID, interfaces, allocator);
        if (specializationIID)
        {
            auto instantiated = specializationIID->instantiated;
            if (specializationIID->piid == IID_IVector1)
            {
                auto getSize = FindMethodSignature(getSizeMethodId, instantiated, interfaces, parentProperties, allocator);
                auto getAt = FindMethodSignature(getAtMethodId, instantiated, interfaces, parentProperties, allocator);
                auto setAt = FindMethodSignature(setAtMethodId, instantiated, interfaces, parentProperties, allocator);
                auto append = FindMethodSignature(appendMethodId, instantiated, interfaces, parentProperties, allocator);
                auto removeAtEnd = FindMethodSignature(removeAtEndMethodId, instantiated, interfaces, parentProperties, allocator);
                auto lengthProperty = RtAnew(allocator, AbiArrayLengthProperty, lengthId, exprNull, typeInt32, getSize);
                return Anew(allocator, VectorSpecialization, specializationIID, lengthProperty, getAt, setAt, append, removeAtEnd);
            } else if (specializationIID->piid == IID_IVectorView1)
            {
                auto getSize = FindMethodSignature(getSizeMethodId, instantiated, interfaces, parentProperties, allocator);
                auto getAt = FindMethodSignature(getAtMethodId, instantiated, interfaces, parentProperties, allocator);
                auto lengthProperty = RtAnew(allocator, AbiArrayLengthProperty, lengthId, exprNull, typeInt32, getSize);
                return Anew(allocator, VectorViewSpecialization, specializationIID, lengthProperty, getAt);
            } else if (specializationIID->piid == __uuidof(IAsyncInfo))
            {
                return Anew(allocator, PromiseSpecialization, specializationIID);
            } else if (specializationIID->piid == IID_IMap2 || specializationIID->piid == IID_IMapView2)
            {
                // Get the right iterable interface - we need to handle this specially since we dont know the instantiated IID of the iterable easily.
                // The below logic is needed to make sure if RC with both IMap and IIterable of some other kind wont result in picking up wrong First method.

                ImmutableList<RtINTERFACECONSTRUCTOR> *interfacesToSearchInForIterble;
                if (parentIID != nullptr && instantiated == parentIID->instantiated)
                {
                    // If parent was IMap, Iterable is one of the interfaces list
                    interfacesToSearchInForIterble = interfaces;
                }
                else
                {
                    // Find the IMap interface from the interfaces list and then get its Interface closure to find out the IIterable.
                    auto mapInterface = interfaces->WhereFirst([&] (RtINTERFACECONSTRUCTOR ic) {
                        return (ic->interfaceType == ifRuntimeInterfaceConstructor) && (RuntimeInterfaceConstructor::From(ic)->iid->instantiated == specializationIID->instantiated);
                    });
                    Js::VerifyCatastrophic(mapInterface.HasValue());
                    interfacesToSearchInForIterble = (*(mapInterface.GetValue()))->requiredInterfaces;
                }

                Js::VerifyCatastrophic(!interfacesToSearchInForIterble->IsEmpty());

                auto iterableInterface = interfacesToSearchInForIterble->WhereFirst([&] (RtINTERFACECONSTRUCTOR ic) {
                    return (ic->interfaceType == ifRuntimeInterfaceConstructor) && (RuntimeInterfaceConstructor::From(ic)->iid->piid == IID_Iterable1);
                });
                Js::VerifyCatastrophic(iterableInterface.HasValue());
                auto icIterable =  RuntimeInterfaceConstructor::From(*(iterableInterface.GetValue()));
                if (wcsncmp(stringConverter->StringOfId(icIterable->typeId), _u("Windows.Foundation.Collections.IIterable`1<Windows.Foundation.Collections.IKeyValuePair`2<String,"),
                    97 /* length of string :  "Windows.Foundation.Collections.IIterable`1<Windows.Foundation.Collections.IKeyValuePair`2<String," */) == 0)
                {
                    if (specializationIID->piid == IID_IMap2)
                    {
                        auto hasKey = FindMethodSignature(hasKeyMethodId, instantiated, interfaces, parentProperties, allocator);
                        auto lookup = FindMethodSignature(lookupMethodId, instantiated, interfaces, parentProperties, allocator);
                        auto insert = FindMethodSignature(insertMethodId, instantiated, interfaces, parentProperties, allocator);
                        auto remove = FindMethodSignature(removeMethodId, instantiated, interfaces, parentProperties, allocator);
                        auto first = FindMethodSignature(firstMethodId, icIterable->iid->instantiated, icIterable->requiredInterfaces, icIterable->ownProperties, allocator);

                        return Anew(allocator, MapSpecialization, specializationIID, hasKey, insert, lookup, remove, first);
                    }
                    else
                    {
                        auto hasKey = FindMethodSignature(hasKeyMethodId, instantiated, interfaces, parentProperties, allocator);
                        auto lookup = FindMethodSignature(lookupMethodId, instantiated, interfaces, parentProperties, allocator);
                        auto first = FindMethodSignature(firstMethodId, icIterable->iid->instantiated, icIterable->requiredInterfaces, icIterable->ownProperties, allocator);
                        return Anew(allocator, MapViewSpecialization, specializationIID, hasKey, lookup, first);
                    }
                }
                else
                {
                    return nullptr;
                }
            } else
            {
                // This is propertyValue, find out if it has IReference or IReferenceArray?

                // IReference/Array, IPropetyValue and PropertyValue do not have async/vector/vectorview
                RtABIMETHODSIGNATURE get_Value = nullptr;
                auto referenceSpecializationIID = GetSpecializationIID<IReferenceOrIReferenceArrayIIDFinder>(parentIID, interfaces, allocator);
                if (referenceSpecializationIID)
                {
                    specializationIID = referenceSpecializationIID;
                    get_Value = FindMethodSignature(getValueMethodId, specializationIID->instantiated, interfaces, parentProperties, allocator);
                }
                return Anew(allocator, PropertyValueSpecialization, specializationIID, get_Value);
            }
        }
        return nullptr;
    }

    // Info:        Resolve name conflicts in a set of interfaces
    // Parameters:  interfaces - the interfaces
    void ProjectionBuilder::ResolveNameConflicts(ImmutableList<RtINTERFACECONSTRUCTOR> * interfaces, MetadataStringId parentTypeId, RtIID parentIID, ImmutableList<RtPROPERTY> * parentProperties,
        ImmutableList<RtPROPERTY> ** resultProperties, RtSPECIALIZATION* specialization, bool isStatic)
    {
        // Determine whether two identifiers are the same.
        auto areSameIdentifier = [&](MetadataStringId a, MetadataStringId b) {
            return (a == b);
        };

        ImmutableList<MetadataStringId> * builtInProperties;
        if (isStatic)
        {
            builtInProperties = builtInConstructorProperties;
        }
        else
        {
            builtInProperties = builtInInstanceProperties;
        }

        // Perform camel casing
        parentProperties = CamelCaseProperties(parentProperties, builtInProperties);
        interfaces = interfaces->SelectInPlace([&](RtINTERFACECONSTRUCTOR ic) {
                return CamelCaseInterfaceConstructor(ic, builtInProperties);
            });

        auto priorProperties = parentProperties;

        // Get the specialization IID if there is one.
        *specialization = GetSpecialization(parentProperties, interfaces, parentIID);

        // Get the names of parent properties and built in properties
        auto priorPropertyNames = priorProperties->Select<MetadataStringId>([&](RtPROPERTY prop) {
            return prop->identifier;
        },allocator);

        // The preexisting property names
        priorPropertyNames = priorPropertyNames->AppendListToCurrentList(builtInProperties);

        // Conflicting name getter
        auto getConflictingNames = [&](ImmutableList<MetadataStringId> * allNames, ArenaAllocator * a) -> ImmutableList<MetadataStringId> * {
            return allNames->GroupBy(areSameIdentifier, allocator)
                ->WhereInPlace([&](ImmutableList<MetadataStringId> * group) {
                    return group->Count()>1;
            })
                ->Select<MetadataStringId>([&](ImmutableList<MetadataStringId> * group) {
                    return group->First();
            }, allocator);
        };

        // Build a list of names which conflict.
        auto conflictingNames = getConflictingNames(
            interfaces->Accumulate(priorPropertyNames,
            [&](ImmutableList<MetadataStringId> * prior, RtINTERFACECONSTRUCTOR ic) -> ImmutableList<MetadataStringId> *  {
                auto names = ic->ownProperties->Select<MetadataStringId>([&](RtPROPERTY prop) {
                    return prop->identifier;
                },allocator);
                return names->AppendListToCurrentList(prior);
        }), allocator);


        // Returns true if this a Property with a conflicting name.
        auto isConflictingProperty = [&](RtPROPERTY prop) {
            return conflictingNames->ContainsWhere([&](MetadataStringId name) {
                return areSameIdentifier(name,prop->identifier);
            });
        };

        // Returns true if this a Property with a conflicting name from parent properties or built in properties
        auto isConflictingPropertyFromParentOrBuildIn = [&](RtPROPERTY prop) {
            return priorPropertyNames->ContainsWhere([&](MetadataStringId name) {
                return areSameIdentifier(name,prop->identifier);
            });
        };

        // Returns true if the property is event handler attaching or detaching field
        auto isEventField = [&](RtPROPERTY prop) {
            return (prop->propertyType == ptAbiAddEventListenerProperty || prop->propertyType == ptAbiRemoveEventListenerProperty || prop->propertyType == ptAbiEventHandlerProperty);
        };

        struct EventListenerInfo
        {
            MetadataStringId fullTypeNameId;
            RtPROPERTY prop;

            EventListenerInfo(MetadataStringId fullTypeNameId, RtPROPERTY prop) : fullTypeNameId(fullTypeNameId), prop(prop)
            { }
        };
        ImmutableList<EventListenerInfo *> *eventProperties = ImmutableList<EventListenerInfo *>::Empty();

        struct ConflictingPropertyInfo
        {
            // interface full name
            MetadataStringId fullNameId;
            RtPROPERTY prop;

            ConflictingPropertyInfo(MetadataStringId fullNameId, RtPROPERTY prop) : fullNameId(fullNameId), prop(prop)
            { }
        };

        // track all static conflicting properties (renamed and to be aliased)
        ImmutableList<ConflictingPropertyInfo*> *conflictingStaticProperties = ImmutableList<ConflictingPropertyInfo*>::Empty();

        // track other conflicting property names (renamed and not-static)
        ImmutableList<MetadataStringId> *conflictingOtherNames = ImmutableList<MetadataStringId>::Empty();

        // Deconflict a single interface.
        auto deconflict = [&](MetadataStringId fullTypeNameId, ImmutableList<RtPROPERTY> * fields) -> ImmutableList<RtPROPERTY> * {
            auto alloc=allocator;
            auto strConverter = stringConverter;

            // Since the input to this is going to be interface's own properties, we cant modify existing fields, we need new, so use Select and not SelectInPlace
            return fields->Select<RtPROPERTY>([&](RtPROPERTY prop) -> RtPROPERTY {
                if (isEventField(prop))
                {
                    if (prop->propertyType == ptAbiAddEventListenerProperty)
                    {
                        auto eventListenerInfo = Anew(alloc, EventListenerInfo, fullTypeNameId, prop);
                        eventProperties = eventProperties->Prepend(eventListenerInfo, alloc);

                        // append to conflicting names
                        conflictingOtherNames = conflictingOtherNames->Prepend(prop->identifier, alloc);
                    }
                    return prop;
                }
                else if (isConflictingProperty(prop))
                {
                    MetadataStringId pathDotName = PathDotName(fullTypeNameId, prop->identifier, alloc, strConverter);

                    TraceRtPROPERTY(prop, _u("RTProperty with name conflict"), strConverter, alloc);
                    TRACE_METADATA(_u("RTProperty conflict resolution: %s -> %s\n"), stringConverter->StringOfId(prop->identifier), stringConverter->StringOfId(pathDotName));

                    bool isConflictingFromParentOrBuildin = isConflictingPropertyFromParentOrBuildIn(prop);
                    TRACE_METADATA(_u("RTProperty - isStatic=%d, prop->propertyType=%d, isConflictingPropertyFromParentOrBuildIn()=%d\n"),
                        isStatic,
                        prop->propertyType,
                        isConflictingFromParentOrBuildin);

                    // only track static name conflicts, as instance conflicts are handled by ExprOfRuntimeClass()
                    if (isStatic &&
                        (prop->propertyType == ptAbiMethodProperty || prop->propertyType == ptOverloadParentProperty) &&
                        (isConflictingFromParentOrBuildin == false))
                    {
                        // length and other non-AbiMethod properties can be ignored, as not related to overloading (see BLUE#70546) and causing side effects
                        auto conflictingStaticProperty = Anew(alloc, ConflictingPropertyInfo, pathDotName, prop);
                        conflictingStaticProperties = conflictingStaticProperties->Prepend(conflictingStaticProperty, alloc);
                    }
                    else
                    {
                        // track all other short names that indicates the alias short-names that cannot be used
                        conflictingOtherNames = conflictingOtherNames->Prepend(prop->identifier, alloc);
                    }

                    return CreateRenamedPropertyIdentifierAsCopy(prop, pathDotName, alloc);
                }
                else
                {
                    return prop;
                }
            },
                allocator);
        };

        // Deconflict parent properties
        auto deconflictedParentProperties = deconflict(parentTypeId, priorProperties);

        // Rename property names that conflict -  - these still have multiple addEventListener and removeEventListener
        auto allMembers = interfaces->Accumulate(deconflictedParentProperties,
            [&](ImmutableList<RtPROPERTY> * prior, RtINTERFACECONSTRUCTOR ic) -> ImmutableList<RtPROPERTY> * {
                if (RuntimeInterfaceConstructor::Is(ic))
                {
                    auto rtic = RuntimeInterfaceConstructor::From(ic);
                    return deconflict(
                        rtic->typeId,
                        rtic->ownProperties
                        )->AppendListToCurrentList(prior);
                }
                return prior;

        });

        // Resolve event name conflicts - more than one source for eventlisteners
        if (eventProperties->Count() > 1)
        {
            // Get all events in a list
            ImmutableList<RtEVENT> *allEvents = ImmutableList<RtEVENT>::Empty();
            ImmutableList<MetadataStringId> *allEventFullTypeNameIds = ImmutableList<MetadataStringId>::Empty();
            // Event handlers list that we are going to build
            ImmutableList<RtPROPERTY> *eventHandlerProperties = ImmutableList<RtPROPERTY>::Empty();
            auto eventHandlerPropertiesTail = eventHandlerProperties;

            eventProperties->Iterate(
                [&](EventListenerInfo *addListenerPropertyInfo) {
                    MetadataStringId fullTypeNameId = addListenerPropertyInfo->fullTypeNameId;
                    auto alloc = allocator;
                    AbiAddEventListenerProperty::From(addListenerPropertyInfo->prop)->events
                        ->Iterate(
                        [&](RtEVENT iteratingEvent) {
                            allEvents = allEvents->Prepend(iteratingEvent, alloc);
                            allEventFullTypeNameIds = allEventFullTypeNameIds->Prepend(fullTypeNameId, alloc);
                    });
            });

            // Find conflicting event names and rename them
            auto priorEventNames = allEvents->Select<MetadataStringId>([&](RtEVENT selectedEvent) {
                return selectedEvent->nameId;
            },allocator);

            auto conflictingEventNames = getConflictingNames(priorEventNames, allocator);

            // Returns true if this a event with a conflicting name.
            auto isConflictingEvent = [&](RtEVENT eventToCheck) {
                return conflictingEventNames->ContainsWhere([&](MetadataStringId nameId) {
                    return areSameIdentifier(nameId, eventToCheck->nameId);
                });
            };

            ImmutableList<RtEVENT> *deconflictedEvents = ImmutableList<RtEVENT>::Empty();

            // Deconflict the event Names.
            allEvents->IterateWith(allEventFullTypeNameIds,
                [&](RtEVENT selectedEvent, MetadataStringId fullTypeNameId) {
                    RtEVENT prependEvent = selectedEvent;
                    RtPROPERTY prependEventHandlerProperty = nullptr;
                    DefaultImmutableStringBuilder onEventHandlerName;
                    if (isConflictingEvent(selectedEvent))
                    {
                        auto newNameId =  PathDotName(fullTypeNameId, selectedEvent->nameId, allocator, stringConverter);
                        prependEvent = RenameEventName(prependEvent, newNameId, allocator
#if DBG
                            , stringConverter->StringOfId(newNameId)
#endif
                            );

                        // get the name for on eventhandler
                        onEventHandlerName.Append(stringConverter->StringOfId(fullTypeNameId));
                        onEventHandlerName.Append(_u(".on"));
                        onEventHandlerName.Append(stringConverter->StringOfId(selectedEvent->nameId));
                    }
                    else
                    {
                        // get the name for on eventhandler
                        onEventHandlerName.Append(_u("on"));
                        onEventHandlerName.Append(stringConverter->StringOfId(selectedEvent->nameId));
                    }

                    deconflictedEvents = deconflictedEvents->Prepend(prependEvent, allocator);
                    prependEventHandlerProperty = RtAnew(allocator, AbiEventHandlerProperty, stringConverter->IdOfString(onEventHandlerName.Get<ArenaAllocator>(allocator)), exprNull, prependEvent);
                    eventHandlerProperties = eventHandlerProperties->Append(prependEventHandlerProperty, allocator, &eventHandlerPropertiesTail);
            });

            // Remove the addEventListner, removeEventListener and oneventname properties
            allMembers = allMembers->WhereInPlace([&](RtPROPERTY prop) {
                return !isEventField(prop);
            });

            // Create the new addEventListener and removeEventListener properties for the new list of events
            auto adder = RtAnew(allocator, AbiAddEventListenerProperty, addEventListenerId, deconflictedEvents, exprNull);
            auto remover = RtAnew(allocator, AbiRemoveEventListenerProperty, removeEventListenerId, deconflictedEvents, exprNull);

            // Update with the AbiEventHandler Properties

            allMembers = allMembers->Prepend(adder, allocator);
            allMembers = allMembers->Prepend(remover, allocator);
            allMembers = eventHandlerProperties->AppendListToCurrentList(allMembers, eventHandlerPropertiesTail);
        }

        // Lastly, check to see that there are no remaining name conflicts.
        // This can happen, for example, when property and a method have the same name.

        // Auto-alias - only considering the static properties being renamed, w/o sharing a same alias as event(s) and other non-ptAbiMethodProperty properties
        //   - this is a BEST EFFORT basis
        ImmutableList<ConflictingPropertyInfo*>* renamedPropertiesToBeRemoved = ImmutableList<ConflictingPropertyInfo*>::Empty();
        ImmutableList<MetadataStringId>* aliasedProperties = ImmutableList<MetadataStringId>::Empty();

        ResolveAliases(conflictingStaticProperties->WhereInPlace([&](ConflictingPropertyInfo* propToBeAliased){
            // detect if property to be statically aliased is already exposed as 'non-static' -- if so, do not alias
            size_t countMatches = conflictingOtherNames->CountWhere([&](MetadataStringId otherName) {
                return areSameIdentifier(propToBeAliased->prop->identifier, otherName);
            });
            bool canBeSafelyProjectedAsAlias = (countMatches == 0);

            if (canBeSafelyProjectedAsAlias)
            {
                renamedPropertiesToBeRemoved = renamedPropertiesToBeRemoved->Prepend(propToBeAliased, allocator);
            }

            return canBeSafelyProjectedAsAlias;
        })->Select<RtPROPERTY>([&](ConflictingPropertyInfo* propToBeAliased) -> RtPROPERTY {
            return propToBeAliased->prop;
            }, allocator), &aliasedProperties)                        // /ResolveAliases()
            ->Iterate([&](RtPROPERTY prop) {
                allMembers = allMembers->Prepend(prop, allocator);
        });

        // removed renamed properties, once we are sure we were able to alias their short form
        if (renamedPropertiesToBeRemoved->IsEmpty() == false)
        {
            renamedPropertiesToBeRemoved->Iterate([&](ConflictingPropertyInfo* conflictingPropertyInfoToBeRemoved) {
                Assert(conflictingPropertyInfoToBeRemoved != nullptr);
                TRACE_METADATA(_u("Renamed property %s (#%d) from projected properties - to be removed?\n"),
                    stringConverter->StringOfId(conflictingPropertyInfoToBeRemoved->fullNameId),
                    conflictingPropertyInfoToBeRemoved->fullNameId);

                RtPROPERTY match = nullptr;
                allMembers->Iterate([&](RtPROPERTY allMembersProp) {
                    if (areSameIdentifier(conflictingPropertyInfoToBeRemoved->fullNameId, allMembersProp->identifier))
                    {
                        Assert(match == nullptr);
                        match = allMembersProp;
                    }
                });

                if (match != nullptr)
                {
                    // make sure that the property really got aliased by the ResolveAliases() method
                    size_t numberAliasedMatch = aliasedProperties->CountWhere([&](MetadataStringId aliasedMetadataStringId) {
                        Assert(conflictingPropertyInfoToBeRemoved->prop != nullptr);
                        return areSameIdentifier(conflictingPropertyInfoToBeRemoved->prop->identifier, aliasedMetadataStringId);
                    });

                    TRACE_METADATA(_u("Renamed property %s (#%d) from projected properties was aliased as %s (#%d) %d time(s)\n"),
                        stringConverter->StringOfId(conflictingPropertyInfoToBeRemoved->fullNameId),
                        conflictingPropertyInfoToBeRemoved->fullNameId,
                        stringConverter->StringOfId(conflictingPropertyInfoToBeRemoved->prop->identifier),
                        conflictingPropertyInfoToBeRemoved->prop->identifier,
                        numberAliasedMatch);

                    if (numberAliasedMatch > 0)
                    {
                        allMembers->RemoveValueInPlace(match);
                        TraceRtPROPERTY(match, _u("Removed renamed property"), stringConverter, allocator);
                    }
                }
            });
        }

        auto finalDeconflictedMembers =
            allMembers->GroupBy([&](RtPROPERTY a, RtPROPERTY b) {
                return areSameIdentifier(a->identifier,b->identifier);
        },allocator)->Select<RtPROPERTY>([&](ImmutableList<RtPROPERTY> * group)->RtPROPERTY {
            auto first = group->First();
            if (group->Count()==1)
            {
                return first;
            }
            return RtAnew(allocator, UnresolvableNameConflictProperty, first->identifier, exprNull, group);
        },allocator);

        // Put them in alphabetic order
        CompareRefProperties comparer(stringConverter);
        *resultProperties = finalDeconflictedMembers->SortCurrentList(&comparer);
    }

    // Info:        Get the members of a the given runtime class in terms of properties
    // Parameters:  type - must be a runtime class
    void ProjectionBuilder::GetInterfacesOfRuntimeClass(const Metadata::TypeDefProperties * type, Option<InterfaceConstructor> * defaultInterface, RtSPECIALIZATION* specialization, ImmutableList<RtINTERFACECONSTRUCTOR> ** allInterfaces)
    {
        *defaultInterface = nullptr;
        auto firstLevelInterfaces = ImplementedInterfaceConstructors(type->td, type->assembly, nullptr, defaultInterface);
        if (firstLevelInterfaces && !defaultInterface->HasValue())
        {
            *defaultInterface = firstLevelInterfaces->First();
        }
        *allInterfaces = GetInterfaceClosure(firstLevelInterfaces,allocator);
        *specialization = GetSpecialization(nullptr, *allInterfaces, nullptr);
    }

    // Info:        Given a set of interface tokens, get the complete closure of interface constructors.
    // Parameters:  firstLevelTokens - list of tokens
    ImmutableList<RtINTERFACECONSTRUCTOR> * ProjectionBuilder::GetInterfaceClosureFromFirstLevelTypeDefs(ImmutableList<const Metadata::TypeDefProperties *> * firstLevelInterfaceTypeDefs)
    {
        // Get all the interface constructors
        auto firstLevelInterfaces =
            firstLevelInterfaceTypeDefs
            ->Select<RtINTERFACECONSTRUCTOR>([&](const Metadata::TypeDefProperties * typeDef)->RtINTERFACECONSTRUCTOR {
                if (typeDef)
                {
                    return InterfaceConstructorOfToken(typeDef->td,typeDef->assembly,nullptr);
                }
                auto typeId = unknownStaticInterfaceId; // Would be better to plumb the name through here.
                auto result = Anew(allocator, MissingInterfaceConstructor, typeId, uncallableMethodSignature, emptyPropertiesObject);
                return result;
            },allocator);
        return GetInterfaceClosure(firstLevelInterfaces, allocator);
    }

    // Info:        The method signature of the runtime class.
    // Parameters:  type - must be a runtime class
    void ProjectionBuilder::GetSignatureOfRuntimeClass(const Metadata::TypeDefProperties * type,
        ImmutableList<const Metadata::TypeDefProperties *> * factoryInterfaceTypeDefs, CustomAttributeInfo& customAttributeInfo, RtMETHODSIGNATURE * resultSignature, int * length)
    {
        *resultSignature = nullptr;
        *length = -1;

        auto constructorName = LastSegmentByDot(stringConverter->StringOfId(type->id));
        auto constructorNameId = stringConverter->IdOfString(constructorName);
        ImmutableList<DeprecatedAttribute>* deprecatedAttributes = nullptr;
        if (customAttributeInfo.isDeprecated)
        {
            deprecatedAttributes = customAttributeInfo.deprecatedAttributes;
        }

        // Get all interface constructors for factories
        ExtractSignatures extract(allocator);
        auto interfaceClosure = GetInterfaceClosureFromFirstLevelTypeDefs(factoryInterfaceTypeDefs);
        extract.VisitImmutableList(interfaceClosure);

        // Rename signatures to constructor name
        auto allSignatures = extract.signatures->SelectInPlace([&](RtABIMETHODSIGNATURE old) {
            return Anew(allocator, AbiMethodSignature, stringConverter->IdOfString(constructorName), old->uniqueName, old->iid, old->vtableIndex, old->inParameterCount,
                old->hasDefaultOverloadAttribute, old->runtimeClassNameId, ctorMetadataId, old->parameters, old->continuation, deprecatedAttributes, old->methodKind);
        });

        if (customAttributeInfo.isSimpleActivatable)
        {
            // Make a simple activatable signature.
            auto rttype = TypeOfToken(type->td, type->assembly, nullptr);
            auto byRefType = RtAnew(allocator, ByRefType, byRefTypeNameId, rttype);
            auto parameter = Anew(allocator, AbiParameter, constructorResultId, byRefType, false, true, 0);
            auto allParams = ToImmutableList(parameter, allocator);
            auto signature = Anew(allocator, AbiMethodSignature, constructorNameId, nullptr, &simpleActivatableIID, 0, 0, false, type->id,
                ctorMetadataId, Anew(allocator, Parameters, allParams->Cast<RtPARAMETER>(), rttype, sizeof(LPVOID), _u("-Class")), nullptr, deprecatedAttributes, MethodKind_Normal);
            allSignatures = allSignatures->Prepend(signature, allocator);
        }

        switch(allSignatures->Count())
        {
        case 0: return; // Unconstructable
        case 1:
            {
                *resultSignature = allSignatures->ToSingle();
                AssertMsg(allSignatures->ToSingle()->inParameterCount < 65536, "Invalid metadata: Maximum arity is 65535.");
                *length = (int)allSignatures->ToSingle()->inParameterCount;
                return;
            }
        }

        // Handle the overload case.

        auto areSameArity = [&](RtABIMETHODSIGNATURE a, RtABIMETHODSIGNATURE b) {
            return a->inParameterCount==b->inParameterCount;
        };

        auto arityGroups =
            allSignatures
            ->GroupBy(areSameArity, allocator)
            ->Select<RtABIMETHODSIGNATURE>([&](ImmutableList<RtABIMETHODSIGNATURE> * group)->RtABIMETHODSIGNATURE {
                auto signatureOfArbitraryChoice = group->First();
                return signatureOfArbitraryChoice;
            }, allocator)
            ->ReverseSortCurrentList(&CompareRefArities::Instance); // Leave the overload group ordered by descending arity

        auto overloadGroup = Anew(allocator, OverloadGroup, stringConverter->IdOfString(constructorName), arityGroups);
        *resultSignature = OverloadedMethodSignatureOfOverloadGroup(overloadGroup);
        AssertMsg(arityGroups->Last()->inParameterCount < 65536, "Invalid metadata: Maximum arity is 65535.");
        *length = (int)arityGroups->Last()->inParameterCount;
        return;
    }

    // Info:        Helper function that given an attribute derived from System.Type, returns the
    //              type def token for the interface specified by name in the attribute value blob, if
    //              the version number is not greater than the target version
    // Parameters:  attr - The attribute
    const Metadata::TypeDefProperties * ProjectionBuilder::TryGetInterfaceTypeDefPropertiesFromAttribute(const Metadata::CustomAttributeProperties * attr, bool contractVersioned)
    {
        const byte* bytes = reinterpret_cast<const byte*>(attr->blob);
        ULONG verifiedBytes = 0;
        // Verify Prolog
        verifiedBytes += attr->assembly.VerifyNextInt16OfAttribute(attr->blobSize, bytes, PrologBytes);
        // Obtain the string of the interface type name
        ULONG strLen;
        verifiedBytes += attr->assembly.VerifyAttributeString(attr->blobSize - verifiedBytes, &(bytes[verifiedBytes]), &strLen);

        char* stringArg = (char *)&(bytes[verifiedBytes - strLen]);
        Js::VerifyCatastrophic(contractVersioned || attr->blobSize == verifiedBytes + sizeof(DWORD) + 2 /* Number of named args - in our case 0 */);

        // Check version number before continuing
        if (!contractVersioned && attr->assembly.isVersioned && (*((DWORD *)(bytes + verifiedBytes)) > targetVersion))
        {
            return nullptr;
        }

        char16* interfaceName = HeapNewArray(char16, strLen+1);
        int chInterfaceName = MultiByteToWideChar(CP_UTF8, 0, (char*)(stringArg), strLen, interfaceName, strLen + 1);
        Js::VerifyCatastrophic(chInterfaceName > 0);
        __analysis_assume((ULONG)chInterfaceName <= strLen);
        interfaceName[chInterfaceName] = _u('\0');

        const Metadata::TypeDefProperties* result = DoWithTypenameFromOtherAssembly<const Metadata::TypeDefProperties *>(interfaceName,
            [](const Metadata::TypeDefProperties * typeDef) { return typeDef; },
            [](LPCWSTR name)->const Metadata::TypeDefProperties * {return nullptr;});

        HeapDeleteArray(strLen+1, interfaceName);
        return result;
    }

    // Info:        The runtimeclassname from exclusiveto attribute. MIDL put in the fully qualified name.
    // Parameters:  attr - The attribute
    MetadataStringId ProjectionBuilder::TryGetRuntimeClassNamePropertyFromAttribute(const Metadata::CustomAttributeProperties * attr)
    {
        auto bytes = reinterpret_cast<const byte*>(attr->blob);
        ULONG verifiedBytes = 0;
        // Verify Prolog
        verifiedBytes += attr->assembly.VerifyNextInt16OfAttribute(attr->blobSize, bytes, PrologBytes);
        // Obtain the string of the interface type name
        ULONG strLen;
        verifiedBytes += attr->assembly.VerifyAttributeString(attr->blobSize - verifiedBytes, &(bytes[verifiedBytes]), &strLen);
        char* stringArg = (char *)&(bytes[verifiedBytes-strLen]);

        // Check version number before continuing
        if(attr->assembly.isVersioned && (*((DWORD *)(bytes + verifiedBytes)) > targetVersion))
        {
            return MetadataStringIdNil;
        }

        auto stringName = HeapNewArray(char16, strLen+1);
        auto chStringName = MultiByteToWideChar(CP_UTF8, 0, (char*)(stringArg), strLen, stringName, strLen+1);
        Js::VerifyCatastrophic(chStringName > 0);
        __analysis_assume((ULONG)chStringName <=strLen);
        stringName[chStringName] = _u('\0');

        MetadataStringId rtcNameId = stringConverter->IdOfString(stringName);
        TRACE_METADATA(_u("runtime class name from exclusiveto: %s, id=%d n"), stringName, rtcNameId);
        HeapDeleteArray(strLen+1, stringName);
        return rtcNameId;
    }


    // Info:        Helper function that returns whether a type is in the
    //              Windows namespace, based on id
    // Parameters:  typeNameId - The id representing the type name
    bool ProjectionBuilder::InWindowsNamespace(MetadataStringId typeNameId)
    {
        LPCWSTR typeName = stringConverter->StringOfId(typeNameId);

        auto dot = wcschr(typeName, _u('.'));
        if (nullptr != dot)
        {
            auto length = dot-typeName;
            if (length == 7)
            {
                // check if it is Windows
                return (wcsncmp(typeName, _u("Windows"), 7) == 0);
            }
        }

        return false;
    }

    // Info:        Helper function that, given an attribute with a single named arg of type enum,
    //              returns the value of the named arg
    // Parameters:  attr - The attribute
    INT32 ProjectionBuilder::TryGetValueFromCustomEnumAttribute(const Metadata::CustomAttributeProperties * attr)
    {
        auto bytes = reinterpret_cast<const byte*>(attr->blob);
        ULONG verifiedBytes = 0;
        // Verify Prolog
        verifiedBytes += attr->assembly.VerifyNextInt16OfAttribute(attr->blobSize, bytes, PrologBytes);
        // Verify number of named arguments (1)
        verifiedBytes += attr->assembly.VerifyNextInt16OfAttribute(attr->blobSize - verifiedBytes, &(bytes[verifiedBytes]), 0x0001);
        // Verify number of named arg is enum field
        verifiedBytes += attr->assembly.VerifyNextInt16OfAttribute(attr->blobSize - verifiedBytes, &(bytes[verifiedBytes]), EnumFieldBytes);
        // First String is for the enum name
        ULONG strLen;
        verifiedBytes += attr->assembly.VerifyAttributeString(attr->blobSize - verifiedBytes, &(bytes[verifiedBytes]), &strLen);
        // Second String is the field name
        verifiedBytes += attr->assembly.VerifyAttributeString(attr->blobSize - verifiedBytes, &(bytes[verifiedBytes]), &strLen);
        // Last 4 bytes are the value
        Js::VerifyCatastrophic(attr->blobSize == verifiedBytes + sizeof(INT32));
        return (*(INT32*)(bytes + verifiedBytes));
    }

    // Info:        Helper function that given an attribute, returns the UINT value of the attribute
    // Parameters:  attr - The attribute
    DWORD ProjectionBuilder::TryGetDWORDValueFromAttribute(const Metadata::CustomAttributeProperties * attr)
    {
        auto bytes = reinterpret_cast<const byte*>(attr->blob);
        attr->assembly.VerifySimpleAttributeBytes(attr->blobSize, bytes, sizeof(DWORD));
        // Return actual data from blob
        return *((DWORD *)(bytes + 2));
    }

    // According to MIDL spec, the blob is a binary serialization of the following elements:
    //The fixed prologue 0x0001
    //An ECMA 335 SerString for the message.
    //A uint32 value for deprecation type (corresponding to the metadata DeprecationType enum described below).
    //A version entry (should be 64 bits for any blob output by MidlRT).
    //A uint16 0 representing the number of named parameters in the blob.

    void ProjectionBuilder::TryGetDeprecatedValueFromCustomAttribute(const Metadata::CustomAttributeProperties * attr, MetadataStringId classId, CustomAttributeInfo* customAttributeInfo)
    {
        AttributeConstructorType constructorType = GetDeprecatedAttributeConstructorType(attr);
        bool isContractVersioned = (constructorType & AttributeConstructorType::ContractVersioned) != 0;

        auto bytes = reinterpret_cast<const byte*>(attr->blob);
        ULONG verifiedBytes = 0;
        // Verify Prolog
        verifiedBytes += attr->assembly.VerifyNextInt16OfAttribute(attr->blobSize, bytes, PrologBytes);
        // Obtain the deprecated info string
        ULONG strLen;
        verifiedBytes += attr->assembly.VerifyAttributeString(attr->blobSize - verifiedBytes, &(bytes[verifiedBytes]), &strLen);
        char* deprecatedText = (char *)&(bytes[verifiedBytes-strLen]);
        Js::VerifyCatastrophic(isContractVersioned || attr->blobSize == verifiedBytes + sizeof(UINT) + sizeof(UINT) + sizeof(USHORT));
        // Get the deprecated type
        UINT deprecatedType = *((UNALIGNED UINT*)(bytes + verifiedBytes));
        verifiedBytes += sizeof(UINT);
        // Get the attribute version
        UINT version = *((UNALIGNED UINT*)(bytes + verifiedBytes));
        verifiedBytes += sizeof(UINT);

        // If the attribute is contract versioned, skip the version check
        // The basic idea is if the type is platform versioned (old-style), then we respect the version attribute
        // to maintain Windows 8.1 compat. Otherwise, if it's contract versioned, we simply expose the type.
        if (!isContractVersioned)
        {
            // Verify number of named arguments is 0
            verifiedBytes += attr->assembly.VerifyNextInt16OfAttribute(attr->blobSize - verifiedBytes, &(bytes[verifiedBytes]), 0);

            // Check version number before continuing
            if (attr->assembly.isVersioned && (version > targetVersion))
            {
                return;
            }
        }

        DeprecatedAttribute deprecatedAttribute;
        deprecatedAttribute.infoString = AnewArrayZ(allocator, char16, strLen+1);
        Assert(strLen != 0);
        auto chName = MultiByteToWideChar(CP_UTF8, 0, deprecatedText, strLen, deprecatedAttribute.infoString, strLen+1);
        __analysis_assume(chName < (INT)strLen);
        deprecatedAttribute.infoString[chName] = _u('\0');

        switch (deprecatedType)
        {
        case 0:
            deprecatedAttribute.deprecateType = DeprecateType::Deprecate_deprecate;
            break;
        case 1:
            deprecatedAttribute.deprecateType = DeprecateType::Deprecate_remove;
            break;
        default:
            Js::VerifyCatastrophic(false);
        }
        deprecatedAttribute.version = version;

        deprecatedAttribute.classId = classId;
        customAttributeInfo->isDeprecated = true;
        customAttributeInfo->deprecatedAttributes = customAttributeInfo->deprecatedAttributes->Prepend(deprecatedAttribute, allocator);

        return ;
    }

    // Given an attribute, figure out what kind of constructor it has
    ProjectionBuilder::AttributeConstructorType ProjectionBuilder::GetActivatableAttributeConstructorType(
        const Metadata::CustomAttributeProperties * attr)
    {
        Assert(attr->isMemberRef);

        PCCOR_SIGNATURE sig = attr->pSig;
        ulong callConv = CorSigUncompressData(sig);
        ulong argCount = CorSigUncompressData(sig);

        CorElementType argType = CorSigUncompressElementType(sig);
        Assert(argType == ELEMENT_TYPE_VOID);

        CorElementType param0 = CorSigUncompressElementType(sig);
        CorElementType param1 = CorElementType::ELEMENT_TYPE_VOID;

        AttributeConstructorType constructorType = AttributeConstructorType::Invalid;

        if (param0 == ELEMENT_TYPE_U4)
        {
            if (argCount == 2)
            {
                param1 = CorSigUncompressElementType(sig);
                constructorType = AttributeConstructorType::ContractVersioned;
            }
            else if (argCount == 1)
            {
                constructorType = AttributeConstructorType::PlatformVersioned;
            }
        }
        else if (param0 == ELEMENT_TYPE_CLASS)
        {
            /* mdToken typeToken = */ CorSigUncompressToken(sig);

            if (argCount == 3)
            {
                param1 = CorSigUncompressElementType(sig);

                Assert(param1 == ELEMENT_TYPE_U4);

                constructorType = AttributeConstructorType::FactoryContractVersioned;
            }
            else if (argCount == 2)
            {
                param1 = CorSigUncompressElementType(sig);

                Assert(param1 == ELEMENT_TYPE_U4);
                constructorType = AttributeConstructorType::FactoryPlatformVersioned;
            }
        }

        if (constructorType == AttributeConstructorType::Invalid)
        {
            // Unexpected ActivatableAttribute constructor
            Js::Throw::FatalProjectionError();
        }

        TRACE_METADATA(_u("Param 0 type: %x, 1 type: %x, conv: %x, argCount: %d\n"), param0, param1, callConv, argCount);

        return constructorType;
    }

    ProjectionBuilder::AttributeConstructorType ProjectionBuilder::GetStaticAttributeConstructorType(
        const Metadata::CustomAttributeProperties * attr)
    {
        Assert(attr->isMemberRef);

        PCCOR_SIGNATURE sig = attr->pSig;
        /* ulong callConv = */ CorSigUncompressData(sig);
        ulong argCount = CorSigUncompressData(sig);

        // Only 2 versions of the constructor are known to us
        Assert(argCount == 2 || argCount == 3);
        return (argCount == 2 ? AttributeConstructorType::PlatformVersioned : AttributeConstructorType::ContractVersioned);
    }

    ProjectionBuilder::AttributeConstructorType ProjectionBuilder::GetDeprecatedAttributeConstructorType(
        const Metadata::CustomAttributeProperties * attr)
    {
        Assert(attr->isMemberRef);

        PCCOR_SIGNATURE sig = attr->pSig;
        /* ulong callConv = */ CorSigUncompressData(sig);
        ulong argCount = CorSigUncompressData(sig);

        // Only 3 versions of the constructor are known to us
        Assert(argCount == 3 || argCount == 4);
        return (argCount == 3 ? AttributeConstructorType::PlatformVersioned : AttributeConstructorType::ContractVersioned);
    }

    // Info:        Look at the attributes of this runtime class and gather the indicated results
    // Parameters:  typeDef - typedef token.
    //              factoryInterfaceTokens - receives the factory interface tokens.
    //              staticInterfaceTokens - receives the factory interface tokens.
    //              isSimpleActivatable - true if this runtime class is also simple activatable
    //              isWebHostHidden - true if this runtime class is webhosthidden
    //              isAllowForWeb - true if this runtime class is allowforweb
    //              gcPressure - the value of the GCPressure attribute, if one exists
    //              version - the value of the version attribute for this runtime class
    //              assembly - the assembly containing the typeDef
    void ProjectionBuilder::AnalyzeRuntimeClassCustomAttributes(
        mdTypeDef typeDef,
        MetadataStringId typeNameId,
        ImmutableList<const Metadata::TypeDefProperties *> ** factoryInterfaces,
        ImmutableList<const Metadata::TypeDefProperties *> ** staticInterfaces,
        __inout CustomAttributeInfo* customAttributeInfo,
        const Metadata::Assembly & assembly)
    {
        customAttributeInfo->isSimpleActivatable = false;
        customAttributeInfo->isWebHostHidden = false;
        customAttributeInfo->isAllowForWeb = false;
        customAttributeInfo->gcPressure = DefaultGCPressure;
        customAttributeInfo->version = 0;
        customAttributeInfo->isDeprecated = false;

        ImmutableList<const Metadata::CustomAttributeProperties*>* attributes = assembly.CustomAttributes(typeDef,0);
        attributes->Iterate([&](const Metadata::CustomAttributeProperties * attr) {
            if (activatableAttributeId == attr->attributeTypeId)
            {
                AttributeConstructorType type = this->GetActivatableAttributeConstructorType(attr);

                bool isContractVersioned = (type & AttributeConstructorType::ContractVersioned) != 0;

                if (type == AttributeConstructorType::PlatformVersioned || type == AttributeConstructorType::ContractVersioned)
                {
                    if (isContractVersioned || (!assembly.isVersioned || (TryGetDWORDValueFromAttribute(attr) <= targetVersion)))
                    {
                        TRACE_METADATA(_u("isSimpleActivatable = true\n"));
                        customAttributeInfo->isSimpleActivatable = true;
                    }
                }
                else if (type == AttributeConstructorType::FactoryPlatformVersioned || type == AttributeConstructorType::FactoryContractVersioned)
                {
                    // Derives from type
                    // Get the interface def from the attribute
                    auto interfaceDef = TryGetInterfaceTypeDefPropertiesFromAttribute(attr, isContractVersioned);
                    if (interfaceDef)
                    {
                        TRACE_METADATA(_u("Found factory interface %s\n"), interfaceDef->typeName_Debug);
                        *factoryInterfaces = (*factoryInterfaces)->Prepend(interfaceDef, allocator);
                    }
                }
                else
                {
                    // Unexpected ActivatableAttribute constructor
                    Js::Throw::FatalProjectionError();
                }
            }
            else if (staticAttributeId == attr->attributeTypeId)
            {
                AttributeConstructorType type = this->GetStaticAttributeConstructorType(attr);

                // Get the interface token from the attribute
                auto interfaceDef = TryGetInterfaceTypeDefPropertiesFromAttribute(attr, (type & AttributeConstructorType::ContractVersioned) != 0);
                if (interfaceDef)
                {
                    TRACE_METADATA(_u("Found static interface %s\n"), interfaceDef->typeName_Debug);
                    *staticInterfaces = (*staticInterfaces)->Prepend(interfaceDef, allocator);
                }
            }
            else if (webHostHiddenAttributeId == attr->attributeTypeId && !IgnoreWebHidden())
            {
                TRACE_METADATA(_u("isWebHostHidden = true\n"));
                customAttributeInfo->isWebHostHidden = true;
            }
            else if (allowForWebAttributeId == attr->attributeTypeId && EnforceAllowForWeb())
            {
                TRACE_METADATA(_u("isAllowForWeb = true\n"));
                customAttributeInfo->isAllowForWeb = true;
            }
            else if (gcPressureAttributeId == attr->attributeTypeId)
            {
                if (InWindowsNamespace(typeNameId))
                {
                    customAttributeInfo->gcPressure = TryGetValueFromCustomEnumAttribute(attr);
                    TRACE_METADATA(_u("gcPressure = %d\n"), customAttributeInfo->gcPressure);
                }
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
                else if (Js::Configuration::Global.flags.EnableThirdPartyGCPressure)
                {
                    customAttributeInfo->gcPressure = TryGetValueFromCustomEnumAttribute(attr);
                    TRACE_METADATA(_u("gcPressure = %d\n"), customAttributeInfo->gcPressure);
                }
#endif
            }
            else if (deprecatedAttributeId == attr->attributeTypeId)
            {
                TryGetDeprecatedValueFromCustomAttribute(attr, typeNameId, customAttributeInfo);
            }
            else if (assembly.isVersioned && (versionAttributeId == attr->attributeTypeId))
            {
                customAttributeInfo->version = TryGetDWORDValueFromAttribute(attr);
                TRACE_METADATA(_u("version = %d\n"), customAttributeInfo->version);
            }
            else if (contractVersionAttributeId == attr->attributeTypeId)
            {
                TRACE_METADATA(_u("Contract attribute seen\n"));
                customAttributeInfo->isContractVersioned = true;
            }
            else if (previousContractVersionAttributeId == attr->attributeTypeId)
            {
                TRACE_METADATA(_u("Previous contract attribute seen\n"));
            }

            // Ignore other attributes

        });
    }

    // Info:        Look at the attributes of this method and gather the indicated results
    // Parameters:  typeDef - typedef token.
    //              assembly - the assembly containing the typeDef
    void ProjectionBuilder::AnalyzeMethodCustomAttributes(
        mdTypeDef typeDef,
        MetadataStringId classId,
        __inout CustomAttributeInfo* customAttributeInfo,
        const Metadata::Assembly & assembly)
    {
        auto attributes = assembly.CustomAttributes(typeDef,0);
        attributes->Iterate([&](const Metadata::CustomAttributeProperties * attr) {
            if (deprecatedAttributeId == attr->attributeTypeId)
            {
                TryGetDeprecatedValueFromCustomAttribute(attr, classId, customAttributeInfo);
            }
            // Ignore other attributes

        });
    }


    // Info:        Look at the attributes of this interface and see if we have exclusiveto attribute.
    // Parameters:  typeDef - typedef token.
    //              assembly - the assembly containing the typeDef
    MetadataStringId ProjectionBuilder::AnalyzeInterfaceCustomAttributes(
        mdTypeDef typeDef,
        const Metadata::Assembly & assembly)
    {
        auto attributes = assembly.CustomAttributes(typeDef,0);
        MetadataStringId rtcNameId = MetadataStringIdNil;
        attributes->IterateWhile([&](const Metadata::CustomAttributeProperties * attr)->bool {
            if (exclusiveToAttributeId  == attr->attributeTypeId)
            {
                Assert(rtcNameId == MetadataStringIdNil);
                rtcNameId = TryGetRuntimeClassNamePropertyFromAttribute(attr);
                return false;
            }
            return true;
            // Ignore other attributes

        });
        return rtcNameId;
    }

    // Info:        Walk through all abi method signatures and add the runtime class name for this
    //              runtime class.
    // Parameters:  body - expression to walk through
    //              runtimeClassName - the new runtime class name
    RtPROPERTIESOBJECT BindMethodSignaturesToFactoryName(RtPROPERTIESOBJECT properties, MetadataStringId runtimeClassNameId, ArenaAllocator * allocator)
    {
        struct Replacer : Visitor
        {
            MetadataStringId runtimeClassNameId;
            Replacer(MetadataStringId runtimeClassNameId, ArenaAllocator *allocator )
                : Visitor(allocator), runtimeClassNameId(runtimeClassNameId) { }
            virtual RtABIMETHODSIGNATURE VisitIndividual(RtABIMETHODSIGNATURE signature)
            {
                return Anew(a, AbiMethodSignature, *signature);
            }
        };

        Replacer replacer(runtimeClassNameId, allocator);
        return PropertiesObject::From(replacer.VisitPropertiesObject(properties));
    }

    // Info:        Get the body expression for this runtime class constructor
    // Parameters:  type - type definition which must be a runtime class
    void ProjectionBuilder::GetPropertiesOfRuntimeClass(const Metadata::TypeDefProperties * type, ImmutableList<const Metadata::TypeDefProperties *> * staticInterfaceTypeDefs,
        int signatureLength, RtPROPERTIESOBJECT * propertiesObject, ImmutableList<RtINTERFACECONSTRUCTOR> ** staticInterfaces)
    {
        // Get all the interface constructors for statics
        *staticInterfaces = GetInterfaceClosureFromFirstLevelTypeDefs(staticInterfaceTypeDefs);

        // Condense interface members into properties. Resolve name conflicts along the way.
        ImmutableList<RtPROPERTY> * properties;
        RtSPECIALIZATION specialization; // We don't statically specialize
        auto baseProperties = signatureLength!=-1 ? FunctionLengthPropertiesOfInt(signatureLength, lengthId, allocator) : nullptr;
        ResolveNameConflicts(*staticInterfaces, type->id, nullptr, baseProperties, &properties, &specialization, true);

        auto unboundBody = Anew(allocator, PropertiesObject, properties);

        // Bind the type name into all method signatures.
        *propertiesObject = BindMethodSignaturesToFactoryName(unboundBody, type->id, allocator);
    }

    // Info:        Convert a given RtOVERLOADPARENTPROPERTY into matching list of RtABIMETHODPROPERTY
    // Parameters:  overloadedParentProperty - RtOVERLOADPARENTPROPERTY to be converted
    ImmutableList<RtPROPERTY>* ProjectionBuilder::PropertiesOfOverload(_In_ RtOVERLOADPARENTPROPERTY overloadedParentProperty)
    {
        Assert(overloadedParentProperty != nullptr);
        Assert(overloadedParentProperty->overloadConstructor != nullptr);
        Assert(overloadedParentProperty->overloadConstructor->signature != nullptr);
        Assert(overloadedParentProperty->overloadConstructor->signature->overloads != nullptr);
        Assert(overloadedParentProperty->overloadConstructor->signature->overloads->overloads != nullptr);

        ImmutableList<RtPROPERTY>* retValue = ImmutableList<RtPROPERTY>::Empty();

        TraceRtOVERLOADPARENTPROPERTY(overloadedParentProperty, _u("PropertiesOfOverload() invoked"), stringConverter, allocator);

        if (overloadedParentProperty != nullptr &&
            overloadedParentProperty->overloadConstructor != nullptr &&
            overloadedParentProperty->overloadConstructor->signature != nullptr &&
            overloadedParentProperty->overloadConstructor->signature->overloads != nullptr &&
            overloadedParentProperty->overloadConstructor->signature->overloads->overloads != nullptr)
        {
            auto overloadGroup = overloadedParentProperty->overloadConstructor->signature->overloads->overloads;
            overloadGroup->Iterate([&](RtABIMETHODSIGNATURE signature) {
                MetadataStringId id = overloadedParentProperty->identifier;
                AssertMsg(signature->inParameterCount < 65536, "Invalid metadata: Maximum arity is 65535.");
                auto properties = FunctionLengthPropertiesObjectOfInt((int)signature->inParameterCount, lengthId, allocator);
                auto function = Anew(allocator, AbiMethod, signature, properties);

                auto newAbiMethodProperty = RtAnew(allocator, AbiMethodProperty, id, function);

                TraceRtABIMETHODPROPERTY(newAbiMethodProperty, _u("PropertiesOfOverload() generated"), stringConverter, allocator);

                retValue = retValue->Prepend(newAbiMethodProperty, allocator);
            });
        }

        return retValue;
    }

    // Info:        Convert a given list of RtOVERLOADPARENTPROPERTY and/or RtABIMETHODPROPERTY into matching list of RtABIMETHODPROPERTY
    // Parameters:  propertiesWithOverloadsAndMethods - list of RtOVERLOADPARENTPROPERTY and/or RtABIMETHODPROPERTY to be converted
    ImmutableList<RtPROPERTY>* ProjectionBuilder::PropertiesOfOverloadsAndMethods(_In_ ImmutableList<RtPROPERTY>* propertiesWithOverloadsAndMethods)
    {
        Assert(propertiesWithOverloadsAndMethods != nullptr);

        auto abiMethods = propertiesWithOverloadsAndMethods->Where([&](RtPROPERTY prop) -> bool {
            return AbiMethodProperty::Is(prop);
        }, allocator);

        auto accumulatedAbiMethods = propertiesWithOverloadsAndMethods->WhereInPlace([&](RtPROPERTY prop) -> bool {
            return OverloadParentProperty::Is(prop);
        })->Accumulate(abiMethods,
            [&](ImmutableList<RtPROPERTY> * prior, RtPROPERTY overloadedProperty) -> ImmutableList<RtPROPERTY>* {
                RtOVERLOADPARENTPROPERTY parentOverload = OverloadParentProperty::From(overloadedProperty);
                auto overloadedAsAbiMethods = PropertiesOfOverload(parentOverload);
                return prior->AppendListToCurrentList(overloadedAsAbiMethods);
        });

        TraceRtABIMETHODPROPERTY(accumulatedAbiMethods, _u("PropertiesOfOverloadsAndMethods() resultset"), stringConverter, allocator);

        return accumulatedAbiMethods;
    }

    // Info:        Handle overloading by arity for properties with same name
    // Parameters:  properties - list of properties to be properly aliased (in case multiple properties with same name)
    //              propertiesAliasedSuccessfully - bookkeeping of properties successfully aliased by method
    ImmutableList<RtPROPERTY>* ProjectionBuilder::ResolveAliases(_In_ ImmutableList<RtPROPERTY>* properties, ImmutableList<MetadataStringId>** propertiesAliasedSuccessfully)
    {
        if (propertiesAliasedSuccessfully != nullptr)
        {
            // if passed in, should be empty
            AssertMsg((*propertiesAliasedSuccessfully)->IsEmpty() == true, "propertiesAliasedSuccessfully should be empty!");
        }

        CompareRefProperties comparer(stringConverter);
        ImmutableList<RtPROPERTY>* instanceFields =
            properties
                ->SortCurrentList(&comparer)
                ->GroupByAdjacentOnCurrentList(PropertyEqualsById,allocator)
                ->Select<RtPROPERTY>([&](ImmutableList<RtPROPERTY> * propertyGroup)->RtPROPERTY {
                    size_t propertyGroupCount = propertyGroup->Count();
                    TRACE_METADATA(_u("ResolveAliases - PROP group %s (#%d) with %d member(s)\n"),
                        propertyGroupCount == 0 ? _u("<NULL>") : stringConverter->StringOfId(propertyGroup->First()->identifier),
                        propertyGroupCount == 0 ? 0 : propertyGroup->First()->identifier,
                        propertyGroupCount);

                    if(propertyGroupCount == 1)
                    {
                        TraceRtPROPERTY(propertyGroup->First(), _u("ResolveAliases - PROP"), stringConverter, allocator);
                        return propertyGroup->First();
                    }
                    else
                    {
                        auto alloc = this->allocator;

                        if (propertyGroup->Any([](RtPROPERTY prop) {
                            return !AbiMethodProperty::Is(prop);
                        }))
                        {
                            // !AbiMethodProperty are present
                            if (propertyGroup->Any([](RtPROPERTY prop) {
                                return !AbiMethodProperty::Is(prop) && !OverloadParentProperty::Is(prop);
                            }))
                            {
                                // something beside AbiMethodProperty and OverloadParentProperty is present
                                if (propertyGroup->TrueForAll([](RtPROPERTY prop) {
                                    return AbiPropertyProperty::Is(prop);
                                }))
                                {
                                    return PropertyOfPropertyGroup(propertyGroup);
                                }
                                // Handle case in which properties or events appear with same name as a method or eachother.
                                // This is technically disallowed, but seen in a C# hybrid winmd.
                                TraceRtPROPERTY(propertyGroup->First(), _u("ResolveAliases - Unresolvable"), stringConverter, alloc);

                                return RtAnew(alloc, UnresolvableNameConflictProperty, propertyGroup->First()->identifier, exprNull, propertyGroup);
                            }
                            else
                            {
                                // only AbiMethodProperty and/or OverloadParentProperty are present - normalize to AbiMethodProperty only
                                propertyGroup = PropertiesOfOverloadsAndMethods(propertyGroup);
                            }
                        }

                        bool hasMultipleOverloadsNotToBeProjected = false;

                        auto arityGroups =
                            propertyGroup->Select<RtABIMETHODSIGNATURE>([&](RtPROPERTY prop)->RtABIMETHODSIGNATURE {
                                return AbiMethodProperty::From(prop)->body->signature;
                            },alloc)
                            ->ReverseSortCurrentList(&CompareRefArities::Instance)
                            ->GroupByAdjacentOnCurrentList(AbiMethodSignatureEqualsByArity,alloc)
                            ->Select<RtABIMETHODSIGNATURE>([&](ImmutableList<RtABIMETHODSIGNATURE> * groupsOfSameArity) {
                                Assert(groupsOfSameArity->Count() != 0);

                                // FOR A GIVEN ARITY of an overload
                                // DO NOT PROJECT IF:
                                //    multiple overloads across interfaces (Count() > 1 && Count(sig->iid->instantiated) > 1)
                                if (groupsOfSameArity->Count() != 1)
                                {
                                    // count # interfaces
                                    RtIID primaryInterfaceForSameArity = groupsOfSameArity->First()->iid;
                                    Assert(primaryInterfaceForSameArity != nullptr);
                                    size_t countOtherInterfacesForSameArity = groupsOfSameArity->CountWhere([&](RtABIMETHODSIGNATURE sig) {
                                        Assert(sig->iid != nullptr);
                                        if (sig->iid->instantiated != primaryInterfaceForSameArity->instantiated )
                                        {
                                            // not the primary interface
                                            return 1;
                                        }
                                        else
                                        {
                                            // primary interface match
                                            return 0;
                                        }
                                    });

                                    if (countOtherInterfacesForSameArity > 0)
                                    {
                                        // shadowing
#if DBG
                                        TRACE_METADATA(_u("ResolveAliases - ArityGroup with %d parameters - no aliasing due to overloads across multiple interfaces\n"),
                                            groupsOfSameArity->First()->inParameterCount);
                                        int index = 0;
                                        AssertMsg(groupsOfSameArity->Count() < 65537, "Invalid metadata: Maximum number of arity groups is 65536; arity may be 0...65535, due to 2-byte encoding of method parameter name indices.");
                                        int count = (int)groupsOfSameArity->Count();
                                        groupsOfSameArity->Iterate([&](RtABIMETHODSIGNATURE sig) {
                                            LPCWSTR str = TraceRtABIMETHODSIGNATURE(sig, stringConverter, alloc);
                                            TRACE_METADATA(_u("ResolveAliases - %d#%d - %s\n"), index, count, str);
                                            index++;
                                            DefaultImmutableStringBuilder::FreeString(allocator, str, wcslen(str));
                                        });
#endif

                                        hasMultipleOverloadsNotToBeProjected = true;
                                    }
                                }

                                return ChooseSingleOverloadInGroupWithSameArity(groupsOfSameArity, alloc);
                            },alloc);

                        auto nameId = propertyGroup->First()->identifier;

                        // if shadowing, no projection of aliases
                        if (hasMultipleOverloadsNotToBeProjected)
                        {
                            RtPROPERTY prop = propertyGroup->First();
                            Assert(prop != nullptr);

                            ProjectionModel::Property* ignoredProperty = const_cast<ProjectionModel::Property*>(prop);
                            ignoredProperty->propertyType = ptNone;
                            TRACE_METADATA(_u("ResolveAliases - marking %s (#%d) as not to be aliased\n"), stringConverter->StringOfId(prop->identifier), prop->identifier);

                            return ignoredProperty;
                        }
                        else if (propertiesAliasedSuccessfully != nullptr)
                        {
                            // bookmark the new alias!
                            *propertiesAliasedSuccessfully = (*propertiesAliasedSuccessfully)->Prepend(nameId, alloc);
                        }

                        auto overloadGroup = Anew(alloc, OverloadGroup, nameId, arityGroups);
                        auto maxArity = arityGroups->First()->inParameterCount;
                        auto syntheticParameters = SyntheticOverloadParameters(maxArity);
                        auto overloadMethodSignature = Anew(alloc, OverloadedMethodSignature, nameId, overloadGroup, Anew(alloc, Parameters, syntheticParameters->Cast<RtPARAMETER>(), typeObject));
                        auto minimumArity = overloadGroup->overloads->Last()->inParameterCount; // Last is minimum because list is sorted descending by arity
                        AssertMsg(minimumArity < 65536, "Invalid metadata: Maximum arity is 65535.");
                        auto overloadFunctionProperties = FunctionLengthPropertiesObjectOfInt((int)minimumArity, lengthId, alloc);
                        auto overloadGroupConstructor = Anew(alloc, OverloadGroupConstructor, overloadMethodSignature, overloadFunctionProperties);
                        RtOVERLOADPARENTPROPERTY newOverloadParentProperty = RtAnew(alloc, OverloadParentProperty, nameId, overloadGroupConstructor);

                        TraceRtPROPERTY(newOverloadParentProperty, _u("ResolveAliases - new OPP"), stringConverter, alloc);

                        return newOverloadParentProperty;
                    }
                }, allocator)
                    ->WhereInPlace([&](_In_ RtPROPERTY prop) {
                        Assert(prop != nullptr);

                        return prop->propertyType != ptNone;
                });

        instanceFields = instanceFields->SortCurrentList(&comparer);

        return instanceFields;
    }

    // Info:        Returns true if the given properties object has event handlers
    // Parameters:  properties - the properties object
    bool HasEventHandlers(RtPROPERTIESOBJECT properties, ArenaAllocator * alloc)
    {
        struct FindEventHandlers : Visitor
        {
            bool hasEventHandlers;
            FindEventHandlers(ArenaAllocator * a)
                : Visitor(a), hasEventHandlers(false)
            { }
            virtual RtPROPERTY VisitAbiAddEventListenerProperty(RtABIADDEVENTLISTENERPROPERTY prop)
            {
                hasEventHandlers = true;
                return prop;
            }
        };
        FindEventHandlers find(alloc);
        find.VisitPropertiesObject(properties);
        return find.hasEventHandlers;
    }

    bool CharIsLowerCase(char16 c)
    {
        char16 upper = c;
#if DBG
        DWORD converted =
#endif
            CharUpperBuffW(&upper, 1);

        Assert(converted == 1);

        return (upper != c);
    }

    // Info:        Camel case an identifier name
    // Parameters:  identifier - the identifier
    MetadataStringId CamelCaseString(MetadataStringId identifier, ArenaAllocator * a, Metadata::IStringConverter * stringConverter, CAMELCASINGMAP * idToCamelCasedId)
    {
        MetadataStringId camelCasedStringId;
        if (idToCamelCasedId->TryGetValue(identifier, &camelCasedStringId))
        {
            return camelCasedStringId;
        }

        LPCWSTR name = stringConverter->StringOfId(identifier);
        auto nameLength = wcslen(name);
        AutoHeapString camelCasedName;
        camelCasedName.CreateNew(nameLength+1);
        wcscpy_s(camelCasedName.Get(), nameLength + 1, name);

        // Camel case the last segment if there are dots
        auto camelInsertionPoint = const_cast<char16*>(LastSegmentByDot(camelCasedName.Get()));
        nameLength = wcslen(camelInsertionPoint);

        // Lowercase the entire uppercase prefix of the identifier,
        // unless the identifier begins with exactly 3 uppercase letters followed by a lowercase letter.
        // For this case, lowercase only the first two letters of the 3-letter prefix.
        char16 currCharLower = camelInsertionPoint[0];

#if DBG
        DWORD converted =
#endif
            CharLowerBuffW(&currCharLower, 1);

        Assert(converted == 1);

        for (size_t i=0; (i < nameLength) && (currCharLower != camelInsertionPoint[i]); i++)
        {
            // The current character is uppercase
            if (i == (nameLength - 1))
            {
                // If this is the last character, always lowercase current character.
                camelInsertionPoint[i] = currCharLower;
                break;
            }

            Assert(i < (nameLength-1));

            // Indexing to (i+1) is now safe, because we will not reach this point if this was the last character
            char16 nextCharLower = camelInsertionPoint[i + 1];
#if DBG
            converted =
#endif
                CharLowerBuffW(&nextCharLower, 1);

            Assert(converted == 1);

            if (nextCharLower != camelInsertionPoint[i+1])
            {
                // If next character is uppercase, always lowercase current character.
                camelInsertionPoint[i] = currCharLower;
            }
            else
            {
                // If this is the third character of an uppercase prefix and the next character is lowercase,
                // do nothing. Otherwise, lowercase the current character.
                if ((i != 2) || !CharIsLowerCase(camelInsertionPoint[i+1]))
                {
                    camelInsertionPoint[i] = currCharLower;
                }
                // The next character is not uppercase, so we are done camelCasing the identifier
                break;
            }
            currCharLower = nextCharLower;
        }

        // Get the string id for the camel cased name and add it to the dictionary
        camelCasedStringId = stringConverter->IdOfString(camelCasedName.Get());
        idToCamelCasedId->Item(identifier, camelCasedStringId);

        return camelCasedStringId;
    };

    // Info:        Look up an interface method from a class method and a list of method impls
    // Parameters:  assembly - assembly of the interface
    //              mb - class method def
    //              methodImpls - class method impls
    //              signature - receives the signature of the method
    //              disambiguationName - fully qualified name that can be used to disambiguate this name if it conflicts
    void ProjectionBuilder::InterfaceMethodPropsOfClassMethodDef(
        const Metadata::MethodProperties * classMethod,
        ImmutableList<const Metadata::MethodImpl*> * methodImpls,
        ImmutableList<RtINTERFACECONSTRUCTOR> * allInterfaces,
        AbiMethodSignature const ** signature,
        MetadataStringId * disambiguationNameId)
    {
        const Metadata::Assembly & assembly = classMethod->assembly;
        mdMethodDef mb = classMethod->mb;

        *signature = nullptr;
        *disambiguationNameId = MetadataStringIdNil;
        auto methodImpl = methodImpls->WhereSingle([&](const Metadata::MethodImpl * methodImpl) {
            return methodImpl->methodBody == mb;
        });

        auto isInterfaceValid = [&](RtIID iid) -> bool
        {
            return allInterfaces->ContainsWhere([&](RtINTERFACECONSTRUCTOR ic) -> bool {
                return (ic->interfaceType == ifRuntimeInterfaceConstructor) && (RuntimeInterfaceConstructor::From(ic)->iid->instantiated == iid->instantiated);
            });
        };

        auto createSignature = [&](const Metadata::MethodProperties * methodProps)
        {
            // Find the interface by iterating our interfaces
            auto foundInterface = allInterfaces->WhereFirst([&](RtINTERFACECONSTRUCTOR ic) -> bool {
                return (ic->interfaceType == ifRuntimeInterfaceConstructor &&
                    &ic->typeDef->assembly == &methodProps->assembly &&
                    ic->typeDef->td == methodProps->classToken);
            });

            if (foundInterface.HasValue())
            {
                auto runtimeInterface = RuntimeInterfaceConstructor::From(*(foundInterface.GetValue()));

                *signature = MethodSignatureOfMethod(runtimeInterface->typeDef, runtimeInterface->iid, methodProps, nullptr);
                *disambiguationNameId = runtimeInterface->typeDef->id;
            }
        };

        if ((mdtMethodDef & methodImpl->methodDecl) == mdtMethodDef)
        {
            const Metadata::MethodProperties * methodProps = assembly.GetMethodProperties2(methodImpl->methodDecl);
            return createSignature(methodProps);
        } else if ((mdtMemberRef & methodImpl->methodDecl) == mdtMemberRef)
        {
            auto memberRef = assembly.GetMemberRefProperties(methodImpl->methodDecl);
            if ((memberRef->classToken & mdtTypeSpec) == mdtTypeSpec)
            {
                auto ic = InterfaceConstructorOfToken(memberRef->classToken, assembly, nullptr);
                if (!RuntimeInterfaceConstructor::Is(ic))
                {
                    return;
                }
                auto ric = RuntimeInterfaceConstructor::From(ic);
                if (!isInterfaceValid(ric->iid))
                {
                    return;
                }

                auto methodProps = ic->typeDef->GetMethodByName(memberRef->id, classMethod->overloadNameId);

                *signature = MethodSignatureOfMethod(ric->typeDef, ric->iid, methodProps, ric->genericParameters);
                *disambiguationNameId = ric->typeId;

                return;
            }

            DoWithTypeFromOtherAssembly<void>(memberRef->assembly,memberRef->classToken,
                [&](const Metadata::Assembly & otherAssembly, mdTypeDef td)->void {
                        auto methodProps = otherAssembly.GetTypeDefProperties(td)->GetMethodByName(memberRef->id, classMethod->overloadNameId);
                        return createSignature(methodProps);
                     },
                    [&](LPCWSTR typeName)->void {
                        // Missing type
                        return;
                    }
                );
        } else
        {
            Js::Throw::FatalProjectionError();
        }
    };

    // Info:        Inject byteLength (a clone of the Capacity propery) into an IBuffer and resolve conflicts.
    // Parameters:  type - type definition which must be a runtime class
    //              instanceProperties - properties of the class instance
    void ProjectionBuilder::InjectIBufferByteLengthProperty(const Metadata::TypeDefProperties * type,
                                                            ImmutableList<RtPROPERTY> * & instanceProperties)
    {
        TRACE_METADATA(_u("IBuffer byteLength injection: IBuffer interface detected, beginning property injection\n"));

        // Check for an existing byteLength property
        bool conflict = instanceProperties->ContainsWhere([&](RtPROPERTY property)->bool {
            return (CamelCaseString(property->identifier, allocator, stringConverter, idToCamelCasedId) == byteLengthId);
        });

        if (conflict)
        {
            TRACE_METADATA(_u("IBuffer byteLength injection: Conflicting byteLength property detected, resolving conflict\n"));

            // Generate the new id name
            MetadataStringId newName = PathDotName(type->id, byteLengthId, allocator, stringConverter);
            Assert(!instanceProperties->ContainsWhere([&](RtPROPERTY property)->bool {
                return (property->identifier == newName);
            }));

            // Replace the existing name
            instanceProperties->SelectInPlace([&](RtPROPERTY property)->RtPROPERTY {
                MetadataStringId camelCasedId = CamelCaseString(property->identifier, allocator, stringConverter, idToCamelCasedId);
                if (camelCasedId == byteLengthId) {
                    TRACE_METADATA(_u("IBuffer byteLength injection: Conflict resolved; %s -> %s\n"),
                            stringConverter->StringOfId(camelCasedId),
                            stringConverter->StringOfId(newName));
                    return CreateRenamedPropertyIdentifierAsCopy(property, newName, allocator);
                }
                return property;
            });
        }

        // We have an IBuffer.. find capacity prop and inject byteLength - NB: similar code in ConstructorOfInterface
        RtPROPERTY capacityProperty = instanceProperties->SelectNotNull<RtPROPERTY>([&](RtPROPERTY property)->RtPROPERTY {
            if (property->identifier == capacityId)
            {
                return CreateRenamedPropertyIdentifierAsCopy(property, byteLengthId, allocator);
            }
            return nullptr;
        }, allocator)->ToSingle();

        instanceProperties = instanceProperties->Prepend(capacityProperty, allocator);
        TRACE_METADATA(_u("IBuffer byteLength injection: Capacity clone byteLength has been successfully injected.\n"));
    }

    // Info:        Get the constructor function for this runtime class
    // Parameters:  type - type definition which must be a runtime class
    RtEXPR ProjectionBuilder::ExprOfRuntimeClass(const Metadata::TypeDefProperties * type)
    {
        // Analyze attributes on the runtime class
        ImmutableList<const Metadata::TypeDefProperties*> * factoryInterfaceTypeDefs = nullptr;
        ImmutableList<const Metadata::TypeDefProperties*> * staticInterfaceTypeDefs = nullptr;

        TRACE_METADATA(_u("Enter ExprOfRuntimeClass(%s (#%d))\n"), type->typeName_Debug, type->id);

        RtTYPECONSTRUCTOR cachedExpr;
        if (runtimeClassCache->TryGetValue(type->id, &cachedExpr))
        {
            Js::VerifyCatastrophic(cachedExpr->typeId == type->id);

            TRACE_METADATA(_u("Leave ExprOfRuntimeClass(%s (#%d)) (found in cache)\n"), type->typeName_Debug, type->id);

            return cachedExpr;
        }

        CustomAttributeInfo customAttributeInfo ;
        AnalyzeRuntimeClassCustomAttributes(type->td, type->id, &factoryInterfaceTypeDefs, &staticInterfaceTypeDefs, &customAttributeInfo, type->assembly);
        bool isHidden = customAttributeInfo.isWebHostHidden ||
                        !type->IsWindowsRuntime() ||
                        (EnforceAllowForWeb() && !customAttributeInfo.isAllowForWeb) ||
                        (!customAttributeInfo.isContractVersioned &&
                            type->assembly.isVersioned &&
                            customAttributeInfo.version > targetVersion);

        RtMETHODSIGNATURE signature;
        int signatureLength = 0;

        if (isHidden)
        {
            TRACE_METADATA(_u("Leave ExprOfRuntimeClass(%s (#%d)) (hidden type)\n"), type->typeName_Debug, type->id);

            auto result = ConstructorOfMissingTypeDef(type, true /*isWebHidden*/);
            runtimeClassCache->Item(type->id, result);
            return result;
        }
        else
        {
            GetSignatureOfRuntimeClass(type, factoryInterfaceTypeDefs, customAttributeInfo, &signature, &signatureLength);
        }

        RuntimeClassConstructor* result = nullptr;

        {
            // bookmark the current RTC being constructed
            CheckForDuplicateTypeId checker(&currentImplementedRuntimeClassInterfaceConstructors);
            DeferredTypeDefinitionCandidate typeCandidate(type->id, nullptr);
            Assert(checker.Contains(typeCandidate) == false);
            checker.AddTypeId(typeCandidate, allocator, stringConverter);

            RtPROPERTIESOBJECT body;
            ImmutableList<RtINTERFACECONSTRUCTOR> * staticInterfaces;
            GetPropertiesOfRuntimeClass(type, staticInterfaceTypeDefs, signatureLength, &body, &staticInterfaces);
            RtPROPERTIESOBJECT prototype = nullptr;
            Option<InterfaceConstructor> defaultInterface;
            RtSPECIALIZATION specialization;
            ImmutableList<RtINTERFACECONSTRUCTOR> * allInterfaces;

            GetInterfacesOfRuntimeClass(type, &defaultInterface, &specialization, &allInterfaces);

            // ---------------------------------------------------------------------------------------------------------------------------
            // Read and map to class metadata
            auto methodImpls = type->assembly.MethodImpls(type->td);

            // A method that checks camel-cased version name and adds interface name to qualify if the methods conflict
            auto fullyQualifyIfConflicts = [&](MetadataStringId interfaceNameId, MetadataStringId nameId, ImmutableList<RtPROPERTY> * generatedProperties)->MetadataStringId {
                bool conflict = builtInInstanceProperties->ContainsWhere([&](MetadataStringId builtIn) {
                    return (nameId == builtIn);
                });

                if (!conflict && generatedProperties)
                {
                    if (generatedProperties->ContainsWhere([&](RtPROPERTY prop) {
                        return (prop->identifier == nameId);}))
                    {
                        conflict = true;
                    }
                }

                if (conflict)
                {
                    return PathDotName(interfaceNameId, nameId, allocator, stringConverter);
                }

                return nameId;
            };

            // Gather the events
            auto metadataEvents = type->GetEvents();

            auto instanceEvents =
                metadataEvents->SelectNotNull<RtEVENT>([&](const Metadata::EventProperties * eventProperties)->RtEVENT {
                    if (!eventProperties->addOnMethod->IsStatic() && IsWithinTargetVersion(eventProperties->ev, eventProperties->assembly))
                    {
                        RtABIMETHODSIGNATURE addOn;
                        MetadataStringId disambiguationNameId;
                        InterfaceMethodPropsOfClassMethodDef(eventProperties->addOnMethod, methodImpls, allInterfaces, &addOn, &disambiguationNameId);
                        if (addOn) // Can be nullptr if the type is missing
                        {
                            RtABIMETHODSIGNATURE removeOn = nullptr;
                            if (eventProperties->removeOnMethod != NULL)
                            {
                                InterfaceMethodPropsOfClassMethodDef(eventProperties->removeOnMethod, methodImpls, allInterfaces, &removeOn, &disambiguationNameId);
                            }
                            // Events must have both an addOn and removeOn method
                            Js::VerifyCatastrophic(removeOn);

                            // Always use the class name for the event
                            auto nameId = eventProperties->id;
                            return EventOfEventProperties(addOn->iid, nameId, addOn, removeOn, nullptr
    #if DBG
                                , stringConverter->StringOfId(nameId)
    #endif
                                );
                        }
                    }
                    return nullptr;
                }, allocator);

            instanceEvents = LowerCaseEvents(instanceEvents);
            auto instanceEventProperties = PropertiesOfEvents(instanceEvents);

            // Gather the methods
            auto methods = type->GetMethods();

            auto instanceMethods = methods->SelectNotNull<RtPROPERTY>([&](const Metadata::MethodProperties * method)->RtPROPERTY {
                if (!method->IsSpecialName() && !method->IsStatic() && !method->IsConstructor() && IsWithinTargetVersion(method->mb, method->assembly))
                {
                    RtABIMETHODSIGNATURE signature;
                    MetadataStringId disambiguationNameId;
                    InterfaceMethodPropsOfClassMethodDef(method, methodImpls, allInterfaces, &signature, &disambiguationNameId);
                    if (signature) // Can be null if the type is missing
                    {
                        // Always use the class name for the method.
                        // Must fully qualify if it conflicts with an event property (i.e. addEventListener).
                        auto nameId = fullyQualifyIfConflicts(disambiguationNameId, method->id, instanceEventProperties);
                        AssertMsg(signature->inParameterCount < 65536, "Invalid metadata: Maximum arity is 65535.");
                        auto properties = FunctionLengthPropertiesObjectOfInt((int)signature->inParameterCount, lengthId, allocator);
                        auto body = Anew(allocator, AbiMethod, signature, properties);
                        return RtAnew(allocator, AbiMethodProperty, nameId, body);
                    }
                }
                return nullptr;
            }, allocator);

            // Gather the properties
            auto properties = type->GetProperties();

            auto instanceProperties =
                properties->SelectNotNull<RtPROPERTY>([&](const Metadata::PropertyProperties * propertyProperties)->RtPROPERTY {
                    if (propertyProperties->hasThis && IsWithinTargetVersion(propertyProperties->propertyToken, propertyProperties->assembly))
                    {
                        RtABIMETHODSIGNATURE getter = nullptr;
                        RtABIMETHODSIGNATURE setter = nullptr;
                        MetadataStringId disambiguationNameId = MetadataStringIdNil;
                        if (propertyProperties->getterMethod != NULL) // This check was need for a C# abi that had no getter
                        {
                            InterfaceMethodPropsOfClassMethodDef(propertyProperties->getterMethod, methodImpls, allInterfaces, &getter, &disambiguationNameId);
                        }
                        if (propertyProperties->setterMethod != NULL)
                        {
                            InterfaceMethodPropsOfClassMethodDef(propertyProperties->setterMethod, methodImpls, allInterfaces, &setter, &disambiguationNameId);
                        }
                        if (getter || setter)
                        {
                             // Always use the class name for the property.
                             // Must fully qualify if it conflicts with an event property (i.e. addEventListener).
                             auto nameId = fullyQualifyIfConflicts(disambiguationNameId, propertyProperties->id, instanceEventProperties);
                             return RtAnew(allocator, AbiPropertyProperty, nameId, exprNull, getter, setter, propertyProperties->id);
                        }
                    }
                    return nullptr;
                }, allocator);


            auto instanceFields = instanceMethods->AppendListToCurrentList(instanceProperties);

            // Check for IBuffer interface and inject byteLength property if necessary.
            bool implementsIBuffer = allInterfaces->ContainsWhere([&](RtINTERFACECONSTRUCTOR ctor) {
                return (ctor->typeId == iBufferId);
            });
            if (implementsIBuffer)
            {
                InjectIBufferByteLengthProperty(type, instanceFields);
            }

            // Sort and group overloads
            CompareRefProperties comparer(stringConverter);

            instanceFields = ResolveAliases(instanceFields);
            instanceFields = instanceEventProperties->AppendListToCurrentList(instanceFields);
            instanceFields = CamelCaseProperties(instanceFields, builtInInstanceProperties);
            instanceFields = instanceFields->SortCurrentList(&comparer);

            prototype = Anew(allocator, PropertiesObject, instanceFields);
            // Done maping to class metadata
            // ---------------------------------------------------------------------------------------------------------------------------

            // If the default interface is hidden, and the rtc is not explicity hidden, thet rtc is still not constructable
            if (!isHidden && defaultInterface.HasValue())
            {
                RtINTERFACECONSTRUCTOR defaultInterfaceValue = defaultInterface.GetValue();
                if (defaultInterfaceValue && (ifRuntimeInterfaceConstructor != defaultInterfaceValue->interfaceType))
                {
                    bool isWebHidden = false;
                    if (MissingInterfaceConstructor::Is(defaultInterfaceValue))
                    {
                        isWebHidden = MissingInterfaceConstructor::From(defaultInterfaceValue)->isWebHidden;
                    }
                    auto returnType = RtAnew(allocator, MissingNamedType, type->id, type->id, false, isWebHidden);
                    signature = Anew(allocator, MissingTypeConstructorMethodSignature, ctorId, Anew(allocator, Parameters, nullptr, returnType));
                }
            }

            signature = signature
                ? signature
                : uncallableMethodSignature; // No constructors so it is not callable

            result = Anew(allocator, RuntimeClassConstructor, type->id, type, signature, body, prototype, defaultInterface,
                specialization, HasEventHandlers(prototype, allocator),
                customAttributeInfo.gcPressure,
                allInterfaces, staticInterfaces);
        }

        // cleared bookmark the current RTC being constructed
        Assert(IsCurrentImplementedRuntimeClassInterfaceConstructorsContains(type->id) == false);
        Assert(result != nullptr);

        // deferred logic - pass 1 or pass 2
        DeferredProjectionConstructorExpr* deferredCtorExpr = GetFromDeferredConstructorMap(type->id);
        if (deferredCtorExpr != nullptr)
        {
            if (deferredCtorExpr->IsPass1())
            {
                OUTPUT_TRACE(Js::ProjectionMetadataPhase, _u("DeferredCTOR - PASS 1 (w/o cycles) completed for RTC %s (#%d) - deferredType = %d\n"),
                    this->stringConverter->StringOfId(deferredCtorExpr->typeId), deferredCtorExpr->typeId, deferredCtorExpr->deferredType);
                Assert(deferredCtorExpr->deferredType == dpcetRuntimeClass);
                // store the 'low-fi' RtEXPR, and restart the ConstructorOfInterface logic
                Assert(deferredCtorExpr->minimalRtEXPR == nullptr);
                deferredCtorExpr->minimalRtEXPR = result;

                // call the same method for doing pass 2
                TRACE_METADATA(_u("Leave ExprOfRuntimeClass for %s (#%d) (built from scratch - deferred - PASS 1 - no cache)\n"), type->typeName_Debug, type->id);
                auto postDeferResult = ExprOfRuntimeClass(type);

                return postDeferResult;
            }
            else
            {
                Assert(deferredCtorExpr->IsPass2());
                Assert(deferredCtorExpr->deferredType == dpcetRuntimeClass);
                OUTPUT_TRACE(Js::ProjectionMetadataPhase, _u("DeferredCTOR - PASS 2 completed for RTC %s (#%d) - deferredType = %d\n"),
                    this->stringConverter->StringOfId(deferredCtorExpr->typeId), deferredCtorExpr->typeId, deferredCtorExpr->deferredType);
                // deferred ctor initialization completed - clean up
                DeleteFromDeferredConstructorMap(type->id);
            }
        }

        // add to cache - either no deferred parsing or deferred parsing pass 2 completed
        runtimeClassCache->Item(type->id, result);

        TRACE_METADATA(_u("Leave ExprOfRuntimeClass(%s (#%d)) (built from scratch)\n"), type->typeName_Debug, type->id);

        return result;
    }

    RtPROPERTY ProjectionBuilder::PropertyOfPropertyGroup(ImmutableList<RtPROPERTY> * propertyGroup)
    {
        // Is only valid to combine two properties of the same name, where one has only a getter and the other has only a setter.
        if (propertyGroup->Count() != 2)
        {
            return RtAnew(allocator, UnresolvableNameConflictProperty, propertyGroup->First()->identifier, exprNull, propertyGroup);
        }

        bool validCombination = true;
        MetadataStringId metadataNameId = MetadataStringIdNil;
        MetadataStringId typeNameId = MetadataStringIdNil;
        RtABIMETHODSIGNATURE getter = nullptr;
        RtABIMETHODSIGNATURE setter = nullptr;

#if DBG
        AllowHeavyOperation allow;
#endif

        propertyGroup->Iterate([&](RtPROPERTY prop) {
            auto abiprop = AbiPropertyProperty::From(prop);
            // Get the metadataNameId from the first AbiPropertyProperty in the list
            if (metadataNameId == MetadataStringIdNil)
            {
                metadataNameId = abiprop->metadataNameId;
            }
            // Verify both properties are of the same type
            if (typeNameId == MetadataStringIdNil)
            {
                typeNameId = abiprop->GetPropertyType()->fullTypeNameId;
            }
            else if (abiprop->GetPropertyType()->fullTypeNameId != typeNameId)
            {
                validCombination = false;
                return;
            }
            // Obtain the getter/setter and verify that there is only one of each for this property group
            if (abiprop->getter.HasValue())
            {
                if (getter)
                {
                    validCombination = false;
                    return;
                }
                getter = abiprop->getter.GetValue();
            }
            if (abiprop->setter.HasValue())
            {
                if (setter)
                {
                    validCombination = false;
                    return;
                }
                setter = abiprop->setter.GetValue();
            }
        });

        // If this is group of properties can be combined, return the resulting combined property type
        if (validCombination)
        {
            return RtAnew(allocator, AbiPropertyProperty, propertyGroup->First()->identifier, exprNull, getter, setter, metadataNameId);
        }
        // Otherwise, this is an unresolvable name conflict
        return RtAnew(allocator, UnresolvableNameConflictProperty, propertyGroup->First()->identifier, exprNull, propertyGroup);
    }

    // Returns true if there is no version attribute or if the version attribute is less than or equal to the target version.
    bool ProjectionBuilder::IsWithinTargetVersion(mdToken token, const Metadata::Assembly & assembly)
    {
        DWORD version;
        if (assembly.HasContractAttribute(token))
        {
            return true;
        }

        return !assembly.isVersioned || !assembly.GetDWORDAttribute(token, _u("Windows.Foundation.Metadata.VersionAttribute"), version) || (version <= targetVersion);
    }

    bool ProjectionBuilder::CanMarshalExpr(RtEXPR expr, bool fAllowGenericType, bool allowMissingTypes, bool *outWasMissingType, bool allowWebHidden)
    {
        if (outWasMissingType != nullptr)
        {
            *outWasMissingType = false;
        }
        switch (expr->type)
        {
        case exprNullLiteral:
        case exprInt32Literal:
        case exprUInt32Literal:
        case exprEnum:
            return true;

        case exprFunction:
            {
                RtFUNCTION function = Function::From(expr);

                switch(function->functionType)
                {
                case functionMissingTypeConstructor:
                    if (!allowWebHidden && MissingTypeConstructor::Is(expr) && MissingTypeConstructor::From(expr)->isWebHidden)
                    {
                        return false;
                    }
                    if (outWasMissingType != nullptr)
                    {
                        *outWasMissingType = true;
                    }
                    return allowMissingTypes;

                case functionDelegateConstructor:
                    {
                        RtDELEGATECONSTRUCTOR delegateConstructor = DelegateConstructor::From(function);
                        RtRUNTIMEINTERFACECONSTRUCTOR invokeInterface = RuntimeInterfaceConstructor::From(delegateConstructor->invokeInterface);
                        Assert(CanMarshalExpr(invokeInterface, fAllowGenericType, allowMissingTypes, nullptr, allowWebHidden));

                        RtPROPERTY invokeProperty = invokeInterface->prototype->fields->First();
                        RtABIMETHODPROPERTY invokeAbiProperty = AbiMethodProperty::From(invokeProperty);
#if DBG
                        AllowHeavyOperation allow;
#endif
                        bool cannotMarshalParameter = (invokeAbiProperty->body->signature->GetParameters()->allParameters->ContainsWhere([&](RtPARAMETER parameter) {
                            return !CanMarshalType(parameter->type, fAllowGenericType, true, nullptr, allowWebHidden); //Allow delegate's arguments to be unresolvable(missing), and we don't care if it actually contained a missing type
                        }));

                        return !cannotMarshalParameter;
                    }

                case functionStructConstructor:
                    {
                        RtSTRUCTCONSTRUCTOR structConstructor = StructConstructor::From(expr);
                        bool cannotMarshalFields = (structConstructor->structType->fields->ContainsWhere([&](RtABIFIELDPROPERTY structField) {
                            bool oneIsMissingType = false;
                            bool toReturn = !CanMarshalExpr(structField->expr, fAllowGenericType, allowMissingTypes, &oneIsMissingType, allowWebHidden);
                            if (oneIsMissingType && outWasMissingType != nullptr)
                            {
                                *outWasMissingType = true;
                            }
                            return toReturn;
                        }));

                        return !cannotMarshalFields;
                    }

                case functionInterfaceConstructor:
                    if (!allowWebHidden && MissingInterfaceConstructor::Is(function) && MissingInterfaceConstructor::From(function)->isWebHidden)
                    {
                        return false;
                    }
                    if (outWasMissingType != nullptr)
                    {
                        *outWasMissingType = MethodSignature::IsMissingTypeSignature(function->signature);
                        return allowMissingTypes || !*outWasMissingType;
                    }
                    return allowMissingTypes || !MethodSignature::IsMissingTypeSignature(function->signature);

                case functionRuntimeClassConstructor:
                    {
                        RtRUNTIMECLASSCONSTRUCTOR rtc = RuntimeClassConstructor::From(function);
                        return rtc->defaultInterface.HasValue() ? CanMarshalExpr(rtc->defaultInterface.GetValue(), fAllowGenericType, allowMissingTypes, outWasMissingType, allowWebHidden) : true;
                    }
                }
                Js::Throw::FatalProjectionError();
            }
        }
        Js::Throw::FatalProjectionError();
    }

    bool ProjectionBuilder::CanMarshalType(RtTYPE type, bool fAllowGenericType, bool allowMissingTypes, bool *outWasMissingType, bool allowWebHidden)
    {
        bool localAllowMissingTypes = allowMissingTypes;
        switch(type->typeCode)
        {
        case tcMissingNamedType:
            {
                auto missingType = MissingNamedType::From(type);
                if (!missingType->isWebHidden || allowWebHidden)
                {
                    if (outWasMissingType != nullptr)
                    {
                        *outWasMissingType = true;
                    }
                    if (allowMissingTypes && !missingType->isStruct)
                    {
                        return true;
                    }
                }
            }
        case tcMissingGenericInstantiationType:
        case tcUnprojectableType:
        case tcVoidType:
            return false;

        case tcGenericClassVarType:
        case tcGenericParameterType:
            return fAllowGenericType;

        case tcStructType:
            {
                localAllowMissingTypes = false;
            }
        case tcInterfaceType:
        case tcDelegateType:
        case tcClassType:
            {
                RtTYPEDEFINITIONTYPE typeDefinitionType = TypeDefinitionType::From(type);
                RtEXPR expr = ExprOfToken(typeDefinitionType->typeId, typeDefinitionType->typeDef->td, typeDefinitionType->typeDef->assembly, typeDefinitionType->genericParameters);
                return CanMarshalExpr(expr, fAllowGenericType, localAllowMissingTypes, outWasMissingType, allowWebHidden); //Struct's can't be missing
            }

        case tcArrayType:
            {
                RtARRAYTYPE arrayType = ArrayType::From(type);
                return CanMarshalType(arrayType->elementType, fAllowGenericType, allowMissingTypes, outWasMissingType, allowWebHidden);
            }

        case tcByRefType:
            {
                RtBYREFTYPE byRefType = ByRefType::From(type);
                return CanMarshalType(byRefType->pointedTo, fAllowGenericType, allowMissingTypes, outWasMissingType, allowWebHidden);
            }

        case tcSystemGuidType:
        case tcWindowsFoundationDateTimeType:
        case tcWindowsFoundationTimeSpanType:
        case tcWindowsFoundationEventRegistrationTokenType:
        case tcWindowsFoundationHResultType:
        case tcEnumType:
        case tcBasicType:
            return true;
        }

        Js::Throw::FatalProjectionError();
    }

    // Info:        Get the constructor function for this delegate
    // Parameters:  type - type definition which must be a runtime class
    RtEXPR ProjectionBuilder::ConstructorOfDelegate(const Metadata::TypeDefProperties * type, ImmutableList<RtTYPE> * genericParameters)
    {
#if DBG
        ParsingDelegateMetadata parsingDelegate;
#endif
        auto delegateAsInterface = ConstructorOfInterface(type,genericParameters); // Treat as an interface with one parameter

        if (ifRuntimeInterfaceConstructor == delegateAsInterface->interfaceType)
        {
            auto simpleName = LastSegmentByDot(stringConverter->StringOfId(type->id));
            auto delegateConstructor = Anew(allocator, DelegateConstructor, delegateAsInterface->typeId, simpleName, delegateAsInterface->signature, delegateAsInterface);
            if (CanMarshalExpr(delegateConstructor, true, false, nullptr, true))
            {
                return delegateConstructor;
            }
        }

        bool isWebHidden = false;
        if (MissingInterfaceConstructor::Is(delegateAsInterface) && MissingInterfaceConstructor::From(delegateAsInterface)->isWebHidden)
        {
            isWebHidden = true;
        }
        auto missingType = RtAnew(allocator, MissingNamedType, delegateAsInterface->typeId, delegateAsInterface->typeId, false, isWebHidden);
        return ConstructorOfMissingNamedType(missingType);
    }

    RtTYPECONSTRUCTOR ProjectionBuilder::ConstructorOfMissingTypeDef(const Metadata::TypeDefProperties * type, bool isWebHidden)
    {
        auto missingType = RtAnew(allocator, MissingNamedType, type->id, type->id, false, isWebHidden);
        return ConstructorOfMissingNamedType(missingType);
    }

    RtTYPECONSTRUCTOR ProjectionBuilder::ConstructorOfMissingNamedType(RtMISSINGNAMEDTYPE missingType)
    {
        auto signature = Anew(allocator, MissingTypeConstructorMethodSignature, ctorId, Anew(allocator, Parameters, nullptr, missingType));
        return Anew(allocator, MissingTypeConstructor, missingType->typeId, signature, missingType->isWebHidden);
    }

    // Info:        Project methods of interface. Resolve overloads including arity grouping.
    // Parameters:  iface - the interface token
    // Returns:     list of overload groups, one for each member function
    ImmutableList<const MetadataOverloadGroup*> * ProjectionBuilder::MethodsOfInterface(const Metadata::TypeDefProperties * interfaceDef, MetadataStringId invokeId)
    {
        auto methods = interfaceDef->GetMethods();

        auto interfaceMethods = methods->Where([&](const Metadata::MethodProperties * method)->bool {
            if (IsMdSpecialName(method->flags))
            {
                // Remove property accessors and event accessors. Leave delegate Invoke
                if (invokeId == method->id)
                {
                    return true;
                }
                return false;
            }
            return true;
        },allocator);

        // Group members by name.
        auto overloadByName = interfaceMethods->GroupBy(MethodEqualsById,allocator)
            ->Select<const MetadataOverloadGroupById *>([&](ImmutableList<const Metadata::MethodProperties *> * group) {
                return Anew(allocator, MetadataOverloadGroupById, group);
             }, allocator);

        // Now, group those groups by arity.
        auto overloadsByNameThenArity =
            overloadByName->Accumulate(ImmutableList<const MetadataOverloadGroup *>::Empty(),
                [&](ImmutableList<const MetadataOverloadGroup*> * prior,  const MetadataOverloadGroupById * overloadGroup) -> ImmutableList<const MetadataOverloadGroup*>* {

                auto groupByArity = overloadGroup->methods->GroupBy(MethodEqualsByArity,allocator)
                    ->Select<const MetadataArityGroup *>([&](ImmutableList<const Metadata::MethodProperties *> * group) {
                        return Anew(allocator, MetadataArityGroup, group);
                    }, allocator);

                auto newGroup = Anew(allocator, MetadataOverloadGroup, overloadGroup->id, groupByArity);
                return prior->Prepend(newGroup,allocator);
            });

        TRACE_METADATA(_u("MethodsOfInterface(%s, %s), result count: %d\n"), interfaceDef->typeName_Debug, stringConverter->StringOfId(invokeId), overloadsByNameThenArity->Count());

        return overloadsByNameThenArity;
    }

    // Info:        Get a modely property from metadata property
    // Parameters:  iid - The iid of the method's interface
    //              propertyProperties - the metadata property
    //              genericParameters - the generic instantiation properties
    RtABIPROPERTYPROPERTY ProjectionBuilder::PropertyOfPropertyProperties(const Metadata::TypeDefProperties * type, RtIID iid, const Metadata::PropertyProperties * propertyProperties, ImmutableList<RtTYPE> * genericParameters)
    {
        RtABIMETHODSIGNATURE getter = nullptr;
        if (!IsNilToken(propertyProperties->getter))
        {
            getter = MethodSignatureOfMethod(type, iid, propertyProperties->getterMethod, genericParameters, MethodKind_Getter);
        }

        RtABIMETHODSIGNATURE setter = nullptr;
        if (!IsNilToken(propertyProperties->setter))
        {
            setter = MethodSignatureOfMethod(type, iid, propertyProperties->setterMethod, genericParameters, MethodKind_Setter);
        }

        auto result = RtAnew(allocator, AbiPropertyProperty, propertyProperties->id, exprNull, getter, setter, propertyProperties->id);

#if DBG
        WCHAR guidStr[MaxGuidLength];
        StringFromGUID2(iid->piid, guidStr, ARRAYSIZE(guidStr));

        TRACE_METADATA(_u("PropertyOfPropertyProperties(%s, %s, count=%d), result: %d\n"),
            guidStr,
            stringConverter->StringOfId(propertyProperties->id),
            genericParameters->Count(),
            stringConverter->StringOfId(result->metadataNameId));
#endif

        return result;
    }

    // Info:        Get properties from a typedef known to be an interface
    // Parameters:  type - the interface type
    ImmutableList<RtABIPROPERTYPROPERTY> * ProjectionBuilder::PropertiesOfInterface(RtIID iid, const Metadata::TypeDefProperties * type, ImmutableList<RtTYPE> * genericParameters)
    {
        auto properties = type->GetProperties();

        auto result = properties->Select<RtABIPROPERTYPROPERTY>([&](const Metadata::PropertyProperties * propertyProperties)->RtABIPROPERTYPROPERTY {
            return PropertyOfPropertyProperties(type, iid, propertyProperties, genericParameters);
        }, allocator);

        return result;
    }

    // Info:        Create an event from a metadata event
    // Parameters:  iid - RtIID of the owning interface
    //              eventMetadataName - metadata name of the event
    //              addOn - signature of the add method
    //              removeOn - signature of the remove methoid
    //              genericParameters - the generic instantiation properties
    RtEVENT ProjectionBuilder::EventOfEventProperties(RtIID iid, MetadataStringId eventMetadataNameId, RtABIMETHODSIGNATURE addOn, RtABIMETHODSIGNATURE removeOn, ImmutableList<RtTYPE> * genericParameters
#if DBG
            , LPCWSTR eventNameStr
#endif
        )
    {
#if DBG
        auto result = Anew(allocator, Event, addOn, removeOn, eventMetadataNameId, eventMetadataNameId, eventNameStr);
#else
        auto result = Anew(allocator, Event, addOn, removeOn, eventMetadataNameId, eventMetadataNameId);
#endif

        return result;
    }

    // Info:        Get the events for an interface
    // Parameters:  type - typedef for the interface
    //              genericParameters - the generic instantiation properties
    ImmutableList<RtEVENT> * ProjectionBuilder::EventsOfInterface(RtIID iid, const Metadata::TypeDefProperties * type, ImmutableList<RtTYPE> * genericParameters)
    {
        auto events = type->GetEvents();

        ImmutableList<RtEVENT> * interfaceEvents = events->Select<RtEVENT>([&](const Metadata::EventProperties * evnt)->RtEVENT {
            auto addOnProps = evnt->addOnMethod;
            auto removeOnProps = evnt->removeOnMethod;
            auto addOn = MethodSignatureOfMethod(type, iid, addOnProps, genericParameters);
            auto removeOn = MethodSignatureOfMethod(type, iid, removeOnProps, genericParameters);
            auto result = EventOfEventProperties(iid, evnt->id, addOn, removeOn, genericParameters
#if DBG
                , stringConverter->StringOfId(evnt->id)
#endif
            );

            return result;
        }, allocator);

        // Lower case the events
        auto result = LowerCaseEvents(interfaceEvents);
        return result;
    }

    // Info:        Get the properties corresponding to a list of events
    // Parameters:  events - the events
    ImmutableList<RtPROPERTY> * ProjectionBuilder::PropertiesOfEvents(ImmutableList<RtEVENT> * events)
    {
        auto eventFields = ImmutableList<RtPROPERTY>::Empty();
        if (events->Count()>0)
        {
            auto adder = RtAnew(allocator, AbiAddEventListenerProperty, addEventListenerId, events, exprNull);
            auto remover = RtAnew(allocator, AbiRemoveEventListenerProperty, removeEventListenerId, events, exprNull);

            eventFields = eventFields->Prepend(adder, allocator);
            eventFields = eventFields->Prepend(remover, allocator);

            // Add the event handlers
            events->Iterate([&](RtEVENT currentEvent) {
                LPCWSTR currentEventName = stringConverter->StringOfId(currentEvent->nameId);
                DefaultImmutableStringBuilder onEventHandlerName;
                auto eventInsertionPoint = LastSegmentByDot(currentEventName);
                AutoHeapString dottedName;
                if (eventInsertionPoint!=currentEventName)
                {
                    dottedName.CreateNew(wcslen(currentEventName)-wcslen(eventInsertionPoint) + 1);
                    auto dottedNameStr = dottedName.Get();
                    wcsncpy_s(dottedNameStr, dottedName.GetLength(), currentEventName, dottedName.GetLength() - 1);
                    onEventHandlerName.Append(dottedNameStr);
                }
                onEventHandlerName.Append(_u("on"));
                onEventHandlerName.Append(eventInsertionPoint);
                MetadataStringId onEventHandlerId = stringConverter->IdOfString(onEventHandlerName.Get<ArenaAllocator>(allocator));
                RtPROPERTY eventHandlerProperty = RtAnew(allocator, AbiEventHandlerProperty, onEventHandlerId, exprNull, currentEvent);
                eventFields = eventFields->Prepend(eventHandlerProperty, allocator);
            });
        }
        return eventFields;
    }

    // Info:        Instantiate the given type definiton
    // Parameters:  type - typedef
    //              genericParameters - outer scope parameters
    TypeDefInstantiation ProjectionBuilder::InstantiateTypeDefinition(const Metadata::TypeDefProperties * type, ImmutableList<RtTYPE> * genericParameters)
    {
        auto assemblyGenericParameters = type->assembly.GenericParameters(type->genericParameterTokens);

        auto genericInstantiations = assemblyGenericParameters->Select<RtTYPE>([&](const Metadata::GenericParameterProperties* properties) {
            return genericParameters->Nth(properties->sequence);
        }, allocator);

        auto typeId = GetGenericInstantiationNameFromParentName(type->id, genericInstantiations, allocator, stringConverter);
        auto result = TypeDefInstantiation(typeId, genericInstantiations);

        return result;
    }

    // Info:        Get a constructor for a type known to be an interface
    // Parameters:  type - typedef for the interface
    RtINTERFACECONSTRUCTOR ProjectionBuilder::ConstructorOfInterface(const Metadata::TypeDefProperties * type, ImmutableList<RtTYPE> * genericParameters)
    {
        TRACE_METADATA(_u("Enter ConstructorOfInterface for %s (#%d)\n"), type->typeName_Debug, type->id);

        // Get the type for this interface
        auto instantiation = InstantiateTypeDefinition(type, genericParameters);
        auto typeId = instantiation.First();
        auto genericInstantiations = instantiation.Second();

        InterfaceConstructor * constructor = nullptr;
        if (interfaceCache->TryGetValue(typeId, &constructor))
        {
            Js::VerifyCatastrophic(constructor->typeId == typeId);

            TRACE_METADATA(_u("Leave ConstructorOfInterface for %s (#%d) (found in cache)\n"), type->typeName_Debug, type->id);

            return constructor;
        }

        // If this interface should be hidden, return the MissingInterfaceConstructor
        if (!type->IsWindowsRuntime() ||
            this->IsTypeWebHidden(type) ||
            !IsWithinTargetVersion(type->td, type->assembly))
        {
            auto result = Anew(allocator, MissingInterfaceConstructor, typeId, uncallableMethodSignature, emptyPropertiesObject, true);
            interfaceCache->Item(typeId, result);

            TRACE_METADATA(_u("Leave ConstructorOfInterface for %s (#%d) (hidden interface)\n"), type->typeName_Debug, type->id);

            return result;
        }

        // Get the iid for this interface
        auto iid = Anew(allocator, InstantiatedIID);
        type->assembly.GetGuidAttributeValue(type->td,  _u("Windows.Foundation.Metadata.GuidAttribute"), iid->piid);
        iid->instantiated = iid->piid;

#if OUTPUT_TRACE_DEBUGONLY_ENABLED
        {
            WCHAR guidStr[MaxGuidLength];
            StringFromGUID2(iid->piid, guidStr, ARRAYSIZE(guidStr));
            TRACE_METADATA(_u("ConstructorOfInterface %s (#%d) - piid = %s\n"), type->typeName_Debug, type->id, guidStr);
        }
#endif

        unsigned int locatorNameCount = 0;
        LPCWSTR *locatorNames = nullptr;

        // for deferred construction of generic types - make sure the G<T> is not true (with T being deferred)
        bool willNotCacheResult = false;

        // If simple interface the instantiated IID is same as parametrised iid.
        if (!genericInstantiations->IsEmpty())
        {
            auto parentNameId = type->id; // Ok to convert metadata name to winrt name because there is no chance of basic types in generic parent
            bool isWebHidden = false;
            HRESULT hr = GetInstantiatedIIDAndTypeNameParts(this, parentNameId, genericInstantiations, allocator, &iid->instantiated, &locatorNameCount, &locatorNames, &isWebHidden);

            if (S_OK != hr)
            {
                auto result = Anew(allocator, MissingInterfaceConstructor, typeId, uncallableMethodSignature, emptyPropertiesObject, isWebHidden);
                interfaceCache->Item(typeId, result);
                return result;
            }

            // go thru all genericInstantiations types, and if present in the deferred ctor map, mark them as non-cachable (for now)
            willNotCacheResult = genericInstantiations->ContainsWhere([&](RtTYPE genericInstantiationType) {
                DeferredProjectionConstructorExpr* deferredExpr = GetFromDeferredConstructorMap(genericInstantiationType->fullTypeNameId);
                if (deferredExpr != nullptr)
                {
                    OUTPUT_TRACE_DEBUGONLY(Js::ProjectionMetadataPhase,
                        _u("ConstructorOfInterface %s (#%d) using deferred type as generic instantiation type %s (#%d) currently deferred initialized\n"),
                        type->typeName_Debug, type->id,
                        this->stringConverter->StringOfId(genericInstantiationType->fullTypeNameId), genericInstantiationType->fullTypeNameId);
                    return true;
                }
                return false;
            });
        }

        // Abi Methods
        auto methods = MethodsOfInterface(type, invokeId);
        auto methodFields =
            methods
                ->Select<RtPROPERTY>([&](const MetadataOverloadGroup* overloadGroup) {
                    return FieldOfMetadataOverloadGroup(iid, overloadGroup, type, genericInstantiations);

            }, allocator);

        // Abi Properties
        auto propertyFields= PropertiesOfInterface(iid, type, genericInstantiations)->Cast<RtPROPERTY>();

        // Abi Events
        auto events = EventsOfInterface(iid, type, genericInstantiations);
        auto eventFields = PropertiesOfEvents(events);

        // All properties
        auto ownProperties = eventFields->AppendListToCurrentList(propertyFields->AppendListToCurrentList(methodFields));

        // Handle IBuffer case
        if (typeId == iBufferId)
        {
            TRACE_METADATA(_u("IBuffer byteLength injection: IBuffer interface detected, beginning property injection\n"));
            // We have an IBuffer.. find capacity prop
            RtPROPERTY capacityProperty = ownProperties->SelectNotNull<RtPROPERTY>([&](RtPROPERTY property)->RtPROPERTY {
                if (property->identifier == capacityId)
                {
                    return CreateRenamedPropertyIdentifierAsCopy(property, byteLengthId, allocator);
                }
                return nullptr;
            }, allocator)->ToSingle();

            ownProperties = ownProperties->Prepend(capacityProperty, allocator);
            TRACE_METADATA(_u("IBuffer byteLength injection: Capacity clone byteLength has been successfully injected.\n"));
        }

        // Get all parent properties
        auto interfaceType = TypeDefinitionType::From(TypeOfTypeDef(type, genericInstantiations));
        auto firstLevelInterfaces = ImplementedInterfaceConstructors(type->td, type->assembly, genericInstantiations, nullptr);
        auto allInterfaces = GetInterfaceClosure(firstLevelInterfaces, allocator);
        ImmutableList<RtPROPERTY> * allProperties;
        RtSPECIALIZATION specialization;
        ResolveNameConflicts(allInterfaces, TypeDefinitionType::From(interfaceType)->typeId, iid, ownProperties, &allProperties, &specialization);

        bool implementsIBuffer = allInterfaces->ContainsWhere([&](RtINTERFACECONSTRUCTOR ctor) {
            return (ctor->typeId == iBufferId);
        });
        if (implementsIBuffer)
        {
            Assert(!allInterfaces->ContainsWhere([&](RtINTERFACECONSTRUCTOR ctor) {
               return (ctor->typeId == byteLengthId);
            }));

            // Un-rename any IBuffer byteLength properties that were conflicted.
            allProperties->SelectInPlace([&](RtPROPERTY property)->RtPROPERTY {
               if (property->identifier == conflictediBufferId) {
                   TRACE_METADATA(_u("IBuffer byteLength injection: Fully qualified IBuffer byteLength property detected; renaming back to 'byteLength'\n"));
                   return CreateRenamedPropertyIdentifierAsCopy(property, byteLengthId, allocator);
               }
               return property;
            });
        }

        auto prototype = Anew(allocator, PropertiesObject, allProperties);
        auto typeSignature = MethodSignatureOfType(type, genericInstantiations);
        auto properties = FunctionLengthPropertiesObjectOfInt(0, lengthId, allocator);
#if DBG
        if(interfaceType->typeId!=typeId)
        {
            LPCWSTR idFromName = stringConverter->StringOfId(typeId);
            LPCWSTR idFromType = stringConverter->StringOfId(interfaceType->typeId);
            idFromName;
            idFromType;
            Js::VerifyCatastrophic(0);
        }
#endif

        auto result = Anew(allocator, RuntimeInterfaceConstructor, interfaceType->typeId,
            type, typeSignature, iid, genericInstantiations, properties, prototype, ownProperties,
            firstLevelInterfaces,locatorNames, locatorNameCount, specialization, HasEventHandlers(prototype,allocator));

        // deferred logic - pass 1 or pass 2
        DeferredProjectionConstructorExpr* deferredCtorExpr = GetFromDeferredConstructorMap(interfaceType->typeId);
        if (deferredCtorExpr != nullptr)
        {
            if (deferredCtorExpr->IsPass1())
            {
                OUTPUT_TRACE(Js::ProjectionMetadataPhase, _u("DeferredCTOR - PASS 1 (w/o cycles) completed for %s (#%d) - deferredType = %d\n"),
                    this->stringConverter->StringOfId(deferredCtorExpr->typeId), deferredCtorExpr->typeId, deferredCtorExpr->deferredType);
                Assert(deferredCtorExpr->deferredType == dpcetInterface);
                // store the 'low-fi' RtEXPR, and restart the ConstructorOfInterface logic
                Assert(deferredCtorExpr->minimalRtEXPR == nullptr);
                deferredCtorExpr->minimalRtEXPR = result;

                // call the same method for doing pass 2
                TRACE_METADATA(_u("Leave ConstructorOfInterface for %s (#%d) (built from scratch - deferred - PASS 1 - no cache)\n"), type->typeName_Debug, type->id);
                auto postDeferResult = ConstructorOfInterface(type, genericParameters);

                return postDeferResult;
            }
            else
            {
                Assert(deferredCtorExpr->IsPass2());
                Assert(deferredCtorExpr->deferredType == dpcetInterface);
                OUTPUT_TRACE(Js::ProjectionMetadataPhase, _u("DeferredCTOR - PASS 2 completed for %s (#%d) - deferredType = %d\n"),
                    this->stringConverter->StringOfId(deferredCtorExpr->typeId), deferredCtorExpr->typeId, deferredCtorExpr->deferredType);

                // make sure we add it to the cache
                Assert(willNotCacheResult == false);

                // deferred ctor initialization completed - clean up
                DeleteFromDeferredConstructorMap(interfaceType->typeId);
            }
        }

        // add to cache - either no deferred parsing or deferred parsing pass 2 completed (unless told otherwise)
        if (willNotCacheResult == false)
        {
            interfaceCache->Item(interfaceType->typeId, result);
        }
        else
        {
            OUTPUT_TRACE_DEBUGONLY(Js::ProjectionMetadataPhase, _u("ConstructorOfInterface - bypass caching for %s (#%d)\n"), type->typeName_Debug, type->id);
        }

        TRACE_METADATA(_u("Leave ConstructorOfInterface for %s (#%d) (built from scratch)\n"), type->typeName_Debug, type->id);

        return result;
    }

    // Info:        Indicates whether the typeId is being tracked for being RTC-ctor-ed
    // Parameters:  typeId - metadataId for the RTC
    bool ProjectionBuilder::IsCurrentImplementedRuntimeClassInterfaceConstructorsContains(MetadataStringId typeId) const
    {
        if (currentImplementedRuntimeClassInterfaceConstructors == nullptr)
        {
            return false;
        }
        else
        {
            return this->currentImplementedRuntimeClassInterfaceConstructors->ContainsWhere([&](DeferredTypeDefinitionCandidate rtcType) { return (rtcType.typeId == typeId); });
        }
    }

    // Info:        Get the deferred projection constructor related to a given metadataStringId, nullptr otherwise
    // Parameters:  typeId - metadataId for the interface
    DeferredProjectionConstructorExpr* ProjectionBuilder::GetFromDeferredConstructorMap(MetadataStringId typeId) const
    {
        DeferredProjectionConstructorExpr* retValue = nullptr;
        Assert(this->deferredConstructorMap != nullptr);

        if (this->deferredConstructorMap->TryGetValue<MetadataStringId>(typeId, &retValue))
        {
            OUTPUT_TRACE_DEBUGONLY(Js::ProjectionMetadataPhase, _u("GetFromDeferredConstructorMap(%s (#%d)) - cache hit\n"), this->stringConverter->StringOfId(typeId), typeId);
        }
        else
        {
            OUTPUT_TRACE_DEBUGONLY(Js::ProjectionMetadataPhase, _u("GetFromDeferredConstructorMap(%s (#%d)) - cache miss\n"), this->stringConverter->StringOfId(typeId), typeId);
        }

        return retValue;
    }

    // Info:        Delete the deferred projection constructor related to a given metadataStringId, if present from the global deferred map
    // Parameters:  typeId - metadataId for the interface
    void ProjectionBuilder::DeleteFromDeferredConstructorMap(MetadataStringId typeId)
    {
        Assert(this->deferredConstructorMap != nullptr);
        bool deleted = this->deferredConstructorMap->Remove(typeId);
        OUTPUT_TRACE_DEBUGONLY(Js::ProjectionMetadataPhase, _u("DeleteFromDeferredConstructorMap(%s (#%d)) - deleted = %d\n"), this->stringConverter->StringOfId(typeId), typeId, deleted);
        Assert(deleted);
    }

    // Info:        Indicates whether a given typename is marked as being constructed in a deferred fashion, currently in PASS 1
    // Parameters:  typeName - full name for the interface
    bool ProjectionBuilder::IsTypeMarkedForDeferredConstructionPass1(LPCWSTR typeName) const
    {
        bool retValue = false;

        Assert(this->deferredConstructorMap != nullptr);
        MetadataStringId typeId = this->stringConverter->IdOfString(typeName);
        DeferredProjectionConstructorExpr* deferredExpr = GetFromDeferredConstructorMap(typeId);
        if (deferredExpr != nullptr)
        {
            retValue = deferredExpr->IsPass1();
        }

        return retValue;
    }

    // Info:        Get a constructor for a basic type
    // Parameters:  typeCode - typecode for the basic type
    RtFUNCTION ProjectionBuilder::ConstructorOfCorElementType(CorElementType typeCode, const Metadata::Assembly & assembly)
    {
        switch(typeCode)
        {
            case ELEMENT_TYPE_VOID:
                return constructorVoid;
            case ELEMENT_TYPE_CHAR:
                return constructorChar;
            case ELEMENT_TYPE_STRING:
                return constructorString;
            case ELEMENT_TYPE_BOOLEAN:
                return constructorBool;
            case ELEMENT_TYPE_U1:
                return constructorByte;
            case ELEMENT_TYPE_I2:
                return constructorInt16;
            case ELEMENT_TYPE_U2:
                return constructorUint16;
            case ELEMENT_TYPE_I4:
                return constructorInt32;
            case ELEMENT_TYPE_U4:
                return constructorUint32;
            case ELEMENT_TYPE_I8:
                return constructorInt64;
            case ELEMENT_TYPE_U8:
                return constructorUint64;
            case ELEMENT_TYPE_R4:
                return constructorFloat;
            case ELEMENT_TYPE_R8:
                return constructorDouble;
            case ELEMENT_TYPE_OBJECT:
                return constructorObject;
            default:
                Js::Throw::FatalProjectionError();
        }
    }

    // Info:        Make a constructor for a missing generic instantiation type
    // Parameters:  git - the return type
    const MissingInstantiationConstructor * ProjectionBuilder::ConstructorOfMissingGenericInstantiationType(RtMISSINGGENERICINSTANTIATIONTYPE git)
    {
        auto signature = Anew(allocator, TypeConstructorMethodSignature, ctorId, Anew(allocator, Parameters, nullptr, git));
        auto missingPropertiesObject = Anew(allocator, PropertiesObject, ImmutableList<RtPROPERTY>::Empty());
        return Anew(allocator, MissingInstantiationConstructor, git->instantiatedNameId, signature, missingPropertiesObject);
    }

    // Info:        Convert a metadate type into a constructor
    // Parameters:  type - the return type
    RtEXPR ProjectionBuilder::ExprOfType(const Metadata::Type * type, const Metadata::Assembly & assembly, ImmutableList<RtTYPE> * genericParameters)
    {
        auto cet = type->GetCorElementType();
        if(Metadata::Type::IsBasicTypeCode(cet))
        {
            return ConstructorOfCorElementType(cet,assembly);
        } else
        {
            switch(cet)
            {
                case ELEMENT_TYPE_VALUETYPE: {
                    auto value = static_cast<const Metadata::Value*>(type);
                    return IntermediateExprOfToken(MetadataStringIdNil, value->valueToken, assembly, genericParameters);
                }
                case ELEMENT_TYPE_CLASS: {
                    auto cls = static_cast<const Metadata::Class*>(type);
                    return IntermediateExprOfToken(MetadataStringIdNil, cls->classToken, assembly, genericParameters);
                }
                case ELEMENT_TYPE_GENERICINST:  {
                    auto gi = TypeOfType(type,assembly,genericParameters);
                    if (MissingGenericInstantiationType::Is(gi))
                    {
                        auto git = MissingGenericInstantiationType::From(gi);
                        return ConstructorOfMissingGenericInstantiationType(git);
                    }
                    auto git = InterfaceType::From(gi);
                    return ConstructorOfInterface(git->typeDef, git->genericParameters);
                }
                case ELEMENT_TYPE_CMOD_OPT: {
                    auto modOpt = static_cast<const Metadata::ModOpt*>(type);
                    auto modded = ExprOfType(modOpt->modded,assembly,genericParameters);
                    return Anew(allocator,ModOptExpr,modded,modOpt);
                }
                case ELEMENT_TYPE_BYREF: {
                    auto byRef = static_cast<const Metadata::ByRef*>(type);
                    auto pointedTo = ExprOfType(byRef->pointedTo,assembly,genericParameters);
                    return Anew(allocator,ByRefExpr,pointedTo,byRef);
                }
                case ELEMENT_TYPE_VAR: {
                    auto vartype = GenericClassVarType::From(TypeOfType(type,assembly,genericParameters));
                    auto signature = Anew(allocator, TypeConstructorMethodSignature, ctorId, Anew(allocator, Parameters, nullptr, vartype));
                    return Anew(allocator,GenericClassVarConstructor,signature);
                }
               case ELEMENT_TYPE_SZARRAY: {
                    auto sz = ArrayType::From(TypeOfType(type,assembly,genericParameters));
                    auto signature = Anew(allocator, TypeConstructorMethodSignature, ctorId, Anew(allocator, Parameters, nullptr, sz));
                    return Anew(allocator,ArrayConstructor,signature);
                }
                default:
                    return Anew(allocator, NativeTypeConstructor, type);
            }
        }
    }

    // Info:        Get the generic parameters of the given typedef
    // Parameters:  type - native type
    ImmutableList<RtTYPE> * GenericParametersOfTypeDefToken(const Metadata::TypeDefProperties * type, ArenaAllocator * allocator)
    {
        return
            type->assembly.GenericParameters(type->genericParameterTokens)
                ->Select<RtTYPE>([&](const Metadata::GenericParameterProperties* properties) {
                    return RtAnew(allocator, GenericParameterType, properties);
            }, allocator);
    }

    // Info:        Get the generic parameters of the given typedef
    // Parameters:  type - native type
    ImmutableList<RtTYPE> * GenericParametersOfTypeDef(const Metadata::TypeDefProperties * type, ArenaAllocator * allocator)
    {
        return GenericParametersOfTypeDefToken(type, allocator);
    }

    // Info:        Get the expr of the the given type id (if it exists)
    // Parameters:  type - native type
    RtEXPR ProjectionBuilder::TryGetExprOfTypeId(MetadataStringId typeId)
    {
        Expr * expr = nullptr;
        if(typeIdToExpr->TryGetValue(typeId, &expr))
        {
            return expr;
        }
        return nullptr;
    }

    // Info:        Translate a type name into a projected function body
    // Parameters:  typeName - the type name
    // Returns:     Projected JavaScript function
    RtEXPR ProjectionBuilder::ExprOfPossiblyGenericTypename(LPCWSTR typeName)
    {
        Js::VerifyCatastrophic(typeName);
        auto typeId = stringConverter->IdOfString(typeName);
        Expr * result = nullptr;
        if (typeIdToExpr->TryGetValue(typeId, &result))
        {
            return result;
        }

        OUTPUT_TRACE_DEBUGONLY(Js::ProjectionMetadataPhase, _u("ExprOfPossiblyGenericTypename(%s) - ResolveTypeName started\n"), typeName);

        Metadata::TypeDefProperties * typeDef;
        auto hr = resolver->ResolveTypeName(typeId, typeName, &typeDef);

        OUTPUT_TRACE_DEBUGONLY(Js::ProjectionMetadataPhase, _u("ExprOfPossiblyGenericTypename(%s) - ResolveTypeName returned 0x%08x\n"), typeName, hr);

        if(FAILED(hr))
        {
#if DBG
            // for debug builds, print out all known typeIdToExpr
            if (typeIdToExpr->Count() > 0)
            {
                Output::Print(_u("ProjectionModel::typeIdToExpr:\n"));
                size_t count = typeIdToExpr->Count();
                for (size_t offset=0; offset<count; offset++)
                {
                    const MetadataStringId metadataStringId = typeIdToExpr->GetKeyAt((int)offset);
                    Assert(metadataStringId != MetadataStringIdNil);
                    Output::Print(_u("typeIdToExpr[%d/%d]: %s (#%d)\n"),
                        offset, count, this->stringConverter->StringOfId(metadataStringId), metadataStringId);
                }

                Output::Flush();
            }
#endif

            auto returnType = RtAnew(allocator, MissingNamedType, typeId, typeId);
            result = (Expr *) ConstructorOfMissingNamedType(returnType);
            typeIdToExpr->Item(typeId, result);
            return result;
        }

        auto genericParameters = GenericParametersOfTypeDefToken(typeDef, allocator);

        // check if already being projected - this is to break cyclic dependency cycles in inheritence chain
        CheckForDuplicateTypeId checker(&implementedInterfaceConstructorsCheck);
        DeferredTypeDefinitionCandidate deferredTypeCandidate(typeId, genericParameters);
        DeferredProjectionConstructorExpr* deferredCtor = GetFromDeferredConstructorMap(typeId);
        if (checker.Contains(deferredTypeCandidate) || deferredCtor != nullptr )
        {
            bool contains = interfaceCache->ContainsKey(typeId);
            AssertMsg(contains == false, "interface cache");     // the type being projected shouldn't be in either cache
            contains = runtimeClassCache->ContainsKey(typeId);
            AssertMsg(contains == false, "rtc cache");     // the type being projected shouldn't be in either cache

            // trigger the deferred projection constructor logic
            if (deferredCtor == nullptr)
            {
                // no hit yet - this is PASS 1 - bookmark the type information and break the cyclic dependency in type projection
                DeferredProjectionConstructorExprType deferredCtorType =
                    IsCurrentImplementedRuntimeClassInterfaceConstructorsContains(typeId) ? dpcetRuntimeClass : dpcetInterface;
                OUTPUT_TRACE_DEBUGONLY(Js::ProjectionMetadataPhase, _u("ExprOfPossiblyGenericTypename(%s (#%d)) - deferredCTOR PASS 1 - returning low-fi RtEXPR - deferredCtorType = %d\n"), typeName, typeId, deferredCtorType);
                deferredCtor = Anew(allocator, DeferredProjectionConstructorExpr, deferredCtorType, typeId);
                Assert(deferredCtor != nullptr);
                this->deferredConstructorMap->AddNew(typeId, deferredCtor);
                Assert(deferredCtor->IsPass1());

                // report null - as we need to substitute a 'dummy type'
                return nullptr;
            }
            else if (deferredCtor->IsPass1())
            {
                // PASS 1 - (multiple referencing) - like for RTC*
                OUTPUT_TRACE_DEBUGONLY(Js::ProjectionMetadataPhase, _u("ExprOfPossiblyGenericTypename(%s (#%d)) - deferredCTOR PASS 1 - returning low-fi RtEXPR - deferredCtorType = %d\n"), typeName, typeId, deferredCtor->deferredType);

                // report null - as we need to substitute a 'dummy type'
                return nullptr;
            }
            else
            {
                // pass 2 - will stop recursion here, directly returning the low-fi RtEXPR from PASS 1
                Assert(deferredCtor->IsPass2());
                Assert(deferredCtor->minimalRtEXPR != nullptr);

                OUTPUT_TRACE_DEBUGONLY(Js::ProjectionMetadataPhase, _u("ExprOfPossiblyGenericTypename(%s (#%d)) - deferredCTOR PASS 2 - returning low-fi RtEXPR - deferredCtorType = %d\n"), typeName, typeId, deferredCtor->deferredType);

                // do not cache it in typeIdToExpr
                return deferredCtor->minimalRtEXPR;
            }
        }

        typeId = genericParameters ? MetadataStringIdNil : typeId;
        return ExprOfToken(typeId, typeDef->td, typeDef->assembly, genericParameters);
    }

    // Info:        Translate a metadata type into a projected function body
    // Parameters:  type - native type
    //              assembly - assembly which holds the native member
    // Returns:     Projected JavaScript function
    RtEXPR ProjectionBuilder::ExprOfTypeDefProperties(const Metadata::TypeDefProperties * type, ImmutableList<RtTYPE> * genericParameters)
    {
        if (genericParameters->IsEmpty())
        {
            genericParameters = GenericParametersOfTypeDef(type, allocator);
        }

        if (type->assembly.IsEnumTypeRef(type->extends))
        {
            return EnumOfEnum(type);
        } else if (type->assembly.IsValueTypeRef(type->extends))
        {
            return ConstructorOfStruct(type);
        } else if (type->assembly.IsDelegateTypeRef(type->extends))
        {
            return ConstructorOfDelegate(type, genericParameters);
        } else if (type->IsClass())
        {
            return ExprOfRuntimeClass(type);
        } else
        {
            return ConstructorOfInterface(type, genericParameters);
        }
    }

    // Info:        Translate a metadata typeDef into a type
    // Parameters:  typeDef - native type
    //              genericParameters - generic parameters
    RtTYPE ProjectionBuilder::TypeOfTypeDef(const Metadata::TypeDefProperties * typeDef, ImmutableList<RtTYPE> * genericParameters)
    {
        // Generate the typeId
        auto instantiation = InstantiateTypeDefinition(typeDef, genericParameters);
        auto typeId = instantiation.First();
        auto genericInstantiations = instantiation.Second();

        Type * result = nullptr;
        if (typeIdToType->TryGetValue(typeId, &result))
        {
#if DBG
            auto foundTypeId = MetadataStringIdNil;
            if (TypeDefinitionType::Is(result))
            {
                auto tdt = TypeDefinitionType::From(result);
                foundTypeId = tdt->typeId;
            }
            else if (MissingNamedType::Is(result))
            {
                auto mnt = MissingNamedType::From(result);
                foundTypeId = mnt->typeId;
            }
            else
            {
                Js::VerifyCatastrophic(0);
            }
            Js::VerifyCatastrophic(foundTypeId == typeId);
#endif
            return result;
        }

        auto addType = [&](RtTYPE type)->RtTYPE {
#if DBG
            auto foundTypeId = MetadataStringIdNil;
            if (TypeDefinitionType::Is(type))
            {
                auto tdt = TypeDefinitionType::From(type);
                foundTypeId = tdt->typeId;
            }
            else if (MissingNamedType::Is(type))
            {
                auto mnt = MissingNamedType::From(type);
                foundTypeId = mnt->typeId;
            }
            else
            {
                Js::VerifyCatastrophic(0);
            }
#endif
            typeIdToType->Item(typeId, const_cast<Type*>(type));
            return type;
        };

        if (!typeDef->IsWindowsRuntime() ||
            this->IsTypeWebHidden(typeDef) ||
            !IsWithinTargetVersion(typeDef->td, typeDef->assembly))
        {
            return addType(RtAnew(allocator, MissingNamedType, typeId, typeId, false, true));
        }
        if (typeDef->assembly.IsValueTypeRef(typeDef->extends))
        {
            return addType(StructTypeOfTypeDef(typeDef));
        }
        if (typeDef->assembly.IsEnumTypeRef(typeDef->extends))
        {
            return addType(RtAnew(allocator, EnumType, typeId, typeDef));
        }
        if (typeDef->assembly.IsDelegateTypeRef(typeDef->extends))
        {
            return addType(RtAnew(allocator, DelegateType, typeId, typeDef, genericInstantiations));
        }
        if(IsTdClass(typeDef->flags))
        {
            // Only enforce AllowForWeb if the type is a class
            if (!this->IsTypeAllowedForWeb(typeDef))
            {
                return addType(RtAnew(allocator, MissingNamedType, typeId, typeId, false, true /*isWebHidden is similar to !allowedForWeb*/));
            }
            else
            {
                return addType(RtAnew(allocator, ClassType, typeId, typeDef));
            }
        }

        GUID iid;
        typeDef->assembly.GetGuidAttributeValue(typeDef->td,  _u("Windows.Foundation.GuidAttribute"), iid);
        return addType(RtAnew(allocator, InterfaceType, typeId, typeDef, genericInstantiations, iid));
    }


    // Info:        Translate a metadata struct typeDef into a type
    // Parameters:  typeDef - native type
    RtTYPE ProjectionBuilder::StructTypeOfTypeDef(const Metadata::TypeDefProperties * type)
    {
        if (pendingStructTypes->Contains(type->id))
        {
            // This is a circular reference to a struct type, which is not allowed in metadata.
            Js::Throw::FatalProjectionError();
        }
        AutoStackItem<MetadataStringId, PENDINGTYPESTACK> pendingStructType(pendingStructTypes, type->id);

        auto fields = type->assembly.Fields(type->td);
        size_t fieldOffset = 0;
        uint structAlignment = 0;
        bool isBlittable = true;

        bool isWebHidden = false;
        // If any of the fields are hidden, the struct should be hidden
        if(fields->ContainsWhere([&](const Metadata::FieldProperties * field) {
            return (Type::IsMissing(ConcreteTypeOfType(field->type, field->assembly, nullptr, &isWebHidden)));
        }))
        {
            // Since we have a hidden field, return the hidden type
            return RtAnew(allocator, MissingNamedType, type->id, type->id, true, isWebHidden);
        };

        auto properties =
            fields->WhereSelect<RtPROPERTY>([&](const Metadata::FieldProperties * field) {
                return field->IsPublic();
            }, [&](const Metadata::FieldProperties * field)->RtABIFIELDPROPERTY {
                auto type = ConcreteTypeOfType(field->type, field->assembly, nullptr);
                isBlittable = isBlittable & ConcreteType::IsBlittable(type);
                AssertMsg(fieldOffset < INT_MAX, "Invalid metadata: Maximum size of type spec is 2gb");
                fieldOffset = ::Math::Align<uint>((uint)fieldOffset, (uint)type->naturalAlignment);

                auto prop =  PropertyOfFieldProperties(field, fieldOffset, nullptr);
                if (type->naturalAlignment > structAlignment)
                {
                    structAlignment = (uint)type->naturalAlignment; // We always provide naturalAlignment and is 0...8
                }

                fieldOffset = fieldOffset + type->storageSize;
                return prop;

            }, allocator);

        auto conflictsWithBuiltIn = [&](RtPROPERTY a)->bool{
            return builtInInstanceProperties->ContainsWhere([&](MetadataStringId nameId) {
                return (nameId == a->identifier);
            });
        };

        // Deconflict a list of fields.
        auto deconflict = [&](MetadataStringId fullTypeNameId, ImmutableList<RtPROPERTY> * fields) -> ImmutableList<RtPROPERTY> * {
            auto alloc=allocator;
            auto strConverter = stringConverter;
            return fields->Select<RtPROPERTY>([&](RtPROPERTY prop) -> RtPROPERTY {
                if (conflictsWithBuiltIn(prop))
                {
                    return CreateRenamedPropertyIdentifierAsCopy(prop, PathDotName(fullTypeNameId, prop->identifier, alloc, strConverter), alloc);
                }
                return prop;}, allocator);
        };

        auto fieldProperties = CamelCaseProperties(properties, builtInInstanceProperties);
        auto deconflictedFieldProperties = deconflict(type->id, fieldProperties)->Cast<RtABIFIELDPROPERTY>();
        auto structSize = ::Math::Align<uint>((uint)fieldOffset, (uint)structAlignment); // fieldOffset / structAlignment asserted-over above in the decl of properties.
        auto sizeOnStack = ::Math::Align<uint>(structSize, sizeof(LPVOID));
        auto isPassByReference = false;
#if _M_X64
        // Struct is not pass by value and always need to be passed by reference when passed as parameter
        if (structSize != 1 && structSize != 2 && structSize != 4 && structSize != 8)
        {
            // Always pass reference to the struct
            sizeOnStack = sizeof(LPVOID);
            isPassByReference = true;
        }
#endif
#if _M_ARM64
        // Struct is not pass by value and always need to be passed by reference when passed as parameter
        if (structSize > 16)
        {
            // Always pass reference to the struct
            sizeOnStack = sizeof(LPVOID);
            isPassByReference = true;
        }
#endif

#if defined(_M_ARM32_OR_ARM64)
        // Account for HFP structs.
        StructFieldType structType;
        int hfpFieldCount;
        this->GetHFPData(deconflictedFieldProperties, &structType, &hfpFieldCount);
        Js::VerifyCatastrophic(!(structType == structFieldTypeHFPFloat || structType == structFieldTypeHFPDouble)
            || hfpFieldCount <= 4); // Got HPF struct which has more than 4 fields total. This should never be the case.

        return RtAnew(allocator, StructType, type->id, type, deconflictedFieldProperties, sizeOnStack, structSize, structAlignment, isPassByReference, isBlittable, structType, hfpFieldCount);
#else
        return RtAnew(allocator, StructType, type->id, type, deconflictedFieldProperties, sizeOnStack, structSize, structAlignment, isPassByReference, isBlittable);
#endif
    }

#if defined(_M_ARM32_OR_ARM64)
    // Get HFP data for a struct given its fields.
    // In:
    //   - fields: list of fields (properties) of the struct.
    // Out:
    //   - structType: receives the type of the struct.
    //   - hfpFieldCount: if the struct is HFP, receives HFP field count in this struct, including HFP/empty inner structs,
    //                    otherwise receives 0.
    // Assumptions:
    // - we rely on all inner structs having already been processed at this time, so that we don't recurse into them.
    void ProjectionBuilder::GetHFPData(ImmutableList<RtABIFIELDPROPERTY>* fields, StructFieldType* structType, int* hfpFieldCount)
    {
        Js::VerifyCatastrophic(fields);
        Js::VerifyCatastrophic(structType);
        Js::VerifyCatastrophic(hfpFieldCount);
        *structType = structFieldTypeEmpty;
        *hfpFieldCount = 0;

        // Notes on the lambda:
        // - return value 'true' is used to continue iteration, 'false' - to stop iteration.
        fields->IterateWhile([&](RtABIFIELDPROPERTY currentProperty) -> bool
        {
            Js::VerifyCatastrophic(currentProperty);
            RtTYPE fieldType = currentProperty->type;
            if (BasicType::Is(fieldType))
            {
                CorElementType baseType = BasicType::From(currentProperty->type)->typeCor;
                if (baseType == ELEMENT_TYPE_R4 &&
                    (*structType == structFieldTypeEmpty || *structType == structFieldTypeHFPFloat && *hfpFieldCount < 4))
                {
                    if (*structType == structFieldTypeEmpty)
                    {
                        *structType = structFieldTypeHFPFloat;
                    }
                    ++*hfpFieldCount;
                    return true;
                }
                else if (baseType == ELEMENT_TYPE_R8 &&
                    (*structType == structFieldTypeEmpty || *structType == structFieldTypeHFPDouble && *hfpFieldCount < 4))
                {
                    if (*structType == structFieldTypeEmpty)
                    {
                        *structType = structFieldTypeHFPDouble;
                    }
                    ++*hfpFieldCount;
                    return true;
                }
            }
            else if (StructType::Is(fieldType)) // Process inner field which is a struct as well.
            {
                RtSTRUCTTYPE structField = StructType::From(fieldType);
                if (structField->structType == structFieldTypeHFPFloat &&
                    (*structType == structFieldTypeEmpty || *structType == structFieldTypeHFPFloat && *hfpFieldCount + structField->hfpFieldCount <= 4) ||
                    structField->structType == structFieldTypeHFPDouble &&
                    (*structType == structFieldTypeEmpty || *structType == structFieldTypeHFPDouble && *hfpFieldCount + structField->hfpFieldCount <= 4))
                {
                    if (*structType == structFieldTypeEmpty)
                    {
                        *structType = structField->structType;
                    }
                    *hfpFieldCount += structField->hfpFieldCount;
                    return true;
                }
                else if (structField->structType == structFieldTypeEmpty)
                {
                    // Empty structs make no change. We are still HFP.
                    return true;
                }
            } // if.

            *structType = structFieldTypeNonHFP;
            *hfpFieldCount = 0;
            return false;
        });
    }
#endif

    // Info:        Convert a token into a type
    // Parameters:  token - the token
    RtTYPE ProjectionBuilder::TypeOfToken(mdToken token, const Metadata::Assembly & assembly, ImmutableList<RtTYPE> * genericParameters, bool valueType)
    {
        if (mdtTypeDef == TypeFromToken(token))
        {
            if (assembly.IsWindowsFoundationHResultTypeDef(token))
            {
                return typeInt32;
            } else if (assembly.IsWindowsFoundationEventRegistrationTokenTypeDef(token))
            {
                return typeWindowsFoundationEventRegistrationToken;
            } else if (assembly.IsWindowsFoundationDateTimeTypeDef(token))
            {
                return typeWindowsFoundationDateTime;
            } else if (assembly.IsWindowsFoundationTimeSpanTypeDef(token))
            {
                return typeWindowsFoundationTimeSpan;
            }

            auto typeDef = assembly.GetTypeDefProperties(token);
            return TypeOfTypeDef(typeDef, genericParameters);
        }
        else if (mdtTypeRef == TypeFromToken(token))
        {
            if (assembly.IsSystemGuidTypeRef(token))
            {
                return typeSystemGuid;
            }
            if (assembly.IsWindowsFoundationDateTimeTypeRef(token))
            {
                return typeWindowsFoundationDateTime;
            }
            if (assembly.IsWindowsFoundationTimeSpanTypeRef(token))
            {
                return typeWindowsFoundationTimeSpan;
            }

            if (assembly.IsWindowsFoundationHResultTypeRef(token))
            {
                return typeInt32; // Since HRESULT cannot be parameter to the parameterizedInterface we dont need to create new type for it
            }
            if (assembly.IsWindowsFoundationEventRegistrationTokenTypeRef(token))
            {
                return typeWindowsFoundationEventRegistrationToken;
            }

            return DoWithTypeFromOtherAssembly<RtTYPE>(assembly, token,
                // Deep traversal into another assembly
                [&](const Metadata::Assembly & otherAssembly, mdTypeDef td) { return TypeOfToken(td, otherAssembly, genericParameters); },
                // Missing type case
                [&](LPCWSTR typeName) -> RtTYPE {
                    auto typeId = stringConverter->IdOfString(typeName);
                    return RtAnew(allocator, MissingNamedType, typeId, typeId, valueType /*treat value types like structs*/);
                }
            );

        }
        else if (mdtTypeSpec == TypeFromToken(token))
        {
            auto type = assembly.GetTypeSpec(token);
            return TypeOfType(type, assembly,genericParameters);
        }
        Js::Throw::FatalProjectionError();
    }

    // Info:        Get an interface constructor from a token known to be an interface
    // Parameters:  token - the token
    RtINTERFACECONSTRUCTOR ProjectionBuilder::InterfaceConstructorOfToken(mdToken token, const Metadata::Assembly & assembly, ImmutableList<RtTYPE> * genericParameters)
    {
        if (mdtTypeDef == TypeFromToken(token))
        {
            auto type = assembly.GetTypeDefProperties(token);
            return ConstructorOfInterface(type, genericParameters);
        }
        else if (mdtTypeRef == TypeFromToken(token))
        {
            auto result =
                DoWithTypeFromOtherAssembly<RtINTERFACECONSTRUCTOR>(assembly,token,
                    [&](const Metadata::Assembly & otherAssembly, mdTypeDef td) { return InterfaceConstructorOfToken(td, otherAssembly, genericParameters);},
                    [&](LPCWSTR typeName)->RtINTERFACECONSTRUCTOR {
                        auto typeId = stringConverter->IdOfString(typeName);
                        auto returnType = RtAnew(allocator, MissingNamedType, typeId, typeId);
                        auto signature = Anew(allocator, MissingTypeConstructorMethodSignature, ctorId, Anew(allocator, Parameters, nullptr, returnType));
                        return Anew(allocator, MissingInterfaceConstructor, typeId, signature, emptyPropertiesObject);
                    }
                );
            return InterfaceConstructor::From(result);
        }
        else if (mdtTypeSpec == TypeFromToken(token))
        {
            auto type = assembly.GetTypeSpec(token);
            return InterfaceConstructor::From(ExprOfType(type, assembly, genericParameters));
        }
        Js::Throw::FatalProjectionError();
    }

    // Info:        Get the type id from an expr
    // Parameters:  expr - the expr
    MetadataStringId GetTypeIdOfExpr(RtEXPR expr)
    {
        auto typeIdFromExpr = MetadataStringIdNil;
        if (Enum::Is(expr))
        {
            typeIdFromExpr = Enum::From(expr)->typeId;
        }
        else if(TypeConstructor::Is(expr))
        {
            auto tc = TypeConstructor::From(expr);
            typeIdFromExpr = tc->typeId;
        }
        return typeIdFromExpr;
    }

    // Info:        Get a constructor for the given token. Caches the result.
    // Parameters:  token - the token
    RtEXPR ProjectionBuilder::IntermediateExprOfToken(MetadataStringId typeId, mdToken token, const Metadata::Assembly & assembly, ImmutableList<RtTYPE> * genericParameters)
    {
        Expr * expr;
        if(typeId!=MetadataStringIdNil && typeIdToExpr->TryGetValue(typeId, &expr))
        {
 #if DBG
            auto typeIdFromExpr = GetTypeIdOfExpr(expr);
            if(typeIdFromExpr != typeId)
            {
                // If the typeId does not match the expr's typeId, ensure it is an alias of that id
                LPCWSTR idFromExpr = stringConverter->StringOfId(typeIdFromExpr);
                LPCWSTR idFromCaller = stringConverter->StringOfId(typeId);
                idFromExpr;
                idFromCaller;
                MetadataStringId aliasOf;
                Js::VerifyCatastrophic(typeIdToExprTypeId->TryGetValue(typeId, &aliasOf));
                Js::VerifyCatastrophic(aliasOf == typeIdFromExpr);
            }
#endif
            return expr;
        }

        // Helper function which adds this constructor to the cache and returns.
        auto addExpr = [&](RtEXPR expr)->RtEXPR {
            auto typeIdFromExpr = GetTypeIdOfExpr(expr);

            bool exprAlreadyCached = false;
            Expr * exprToAlias;
            if(typeId != MetadataStringIdNil && typeIdFromExpr != typeId)
            {
                exprAlreadyCached = typeIdToExpr->TryGetValue(typeIdFromExpr, &exprToAlias);
#if DBG
                LPCWSTR idFromExpr = stringConverter->StringOfId(typeIdFromExpr);
                LPCWSTR idFromCaller = stringConverter->StringOfId(typeId);
                idFromExpr;
                idFromCaller;
                if(exprAlreadyCached)
                {
                    // The expr should match the one already cached for this type
                    Js::VerifyCatastrophic(exprToAlias == expr);
                }
                // Add the typeId as an alias of the typeIdFromExpr
                typeIdToExprTypeId->Item(typeId, typeIdFromExpr);
#endif
                // Type name seen formatted differently, but still legal
                typeIdToExpr->Item(typeId, const_cast<Expr*>(expr));
            }

#if DBG
            //We may not always hit the codepath above to check if the expr has been chached or not so adding the check here
            if (!typeIdToExpr->TryGetValue(typeIdFromExpr, &exprToAlias))
            {
                if (
                    !(MissingInterfaceConstructor::Is(expr) && MissingInterfaceConstructor::From(expr)->isWebHidden) &&
                    !(MissingTypeConstructor::Is(expr) && MissingTypeConstructor::From(expr)->isWebHidden) &&
                    !(RuntimeClassConstructor::Is(expr) && RuntimeClassConstructor::From(expr)->defaultInterface.HasValue()
                    && MissingInterfaceConstructor::Is(RuntimeClassConstructor::From(expr)->defaultInterface.GetValue())
                    && MissingInterfaceConstructor::From(RuntimeClassConstructor::From(expr)->defaultInterface.GetValue())->isWebHidden)
                    )
                {
                    //We have parsed a non-webHidden type
                    DisallowParsingMetadata::VerifyParsingAllowed();
                }

            }
#endif
            if (!exprAlreadyCached)
            {
                typeIdToExpr->Item(typeIdFromExpr, const_cast<Expr*>(expr));
            }
            return expr;
        };

        if (mdtTypeDef == TypeFromToken(token))
        {
            if (assembly.IsWindowsFoundationHResultTypeDef(token))
            {
                return addExpr(constructorInt32);
            } else if (assembly.IsWindowsFoundationEventRegistrationTokenTypeDef(token))
            {
                return addExpr(constructorWindowsFoundationEventRegistrationToken);
            }  else if (assembly.IsWindowsFoundationDateTimeTypeDef(token))
            {
                return addExpr(constructorWindowsFoundationDateTime);
            }  else if (assembly.IsWindowsFoundationTimeSpanTypeDef(token))
            {
                return addExpr(constructorWindowsFoundationTimeSpan);
            }

            auto type = assembly.GetTypeDefProperties(token);
            return addExpr(ExprOfTypeDefProperties(type,genericParameters));
        }
        else if (mdtTypeRef == TypeFromToken(token))
        {
            // The actual definition of Windows.Foundation.HResult is
            // public struct HResult
            // {
            //     public int Value;
            // }
            // We don't want the natural projection so create a special projection
            // that treats it as int32.
            if (assembly.IsWindowsFoundationHResultTypeRef(token))
            {
                return addExpr(constructorInt32);
            } else if (assembly.IsSystemGuidTypeRef(token))
            {
                // System.Guid exists in mscorlib.dll which is not WinRT-resolvable. Make a fake one here.
                return addExpr(constructorSystemGuid);
            } else if (assembly.IsWindowsFoundationEventRegistrationTokenTypeRef(token))
            {
                return addExpr(constructorWindowsFoundationEventRegistrationToken);
            } else if (assembly.IsWindowsFoundationDateTimeTypeRef(token))
            {
                return addExpr(constructorWindowsFoundationDateTime);
            } else if (assembly.IsWindowsFoundationTimeSpanTypeRef(token))
            {
                return addExpr(constructorWindowsFoundationTimeSpan);
            } else
            {
                return DoWithTypeFromOtherAssembly<RtEXPR>(assembly,token,
                        [&](const Metadata::Assembly & otherAssembly, mdTypeDef td) {
                            return IntermediateExprOfToken(MetadataStringIdNil, td, otherAssembly, genericParameters);
                    },
                        [&](LPCWSTR typeName) -> RtEXPR {
                            auto typeId = stringConverter->IdOfString(typeName);
                            auto returnType = RtAnew(allocator, MissingNamedType, typeId, typeId);
                            return addExpr(ConstructorOfMissingNamedType(returnType));
                    }
                );
            }
        }
        else if (mdtTypeSpec == TypeFromToken(token))
        {
            auto type = assembly.GetTypeSpec(token);
            return addExpr(ExprOfType(type, assembly,genericParameters));
        }
        Js::Throw::FatalProjectionError();
    }

    // Info:        Get a constructor for the given token.
    // Parameters:  token - the token
    RtEXPR ProjectionBuilder::ExprOfToken(MetadataStringId typeId, mdToken token, const Metadata::Assembly & assembly, ImmutableList<RtTYPE> * genericParameters)
    {
        return IntermediateExprOfToken(typeId, token, assembly, genericParameters);
    }

    // Info:        Apply the given remaining name and type into the record space.
    // Parameters:  propertiesObject - prior record space
    //              remaining - the remaining parts of the dotted name
    //              type - the type to project
    // Returns:     a new record space with the type in it
    RtPROPERTIESOBJECT ProjectionBuilder::ApplyTypeToPropertiesObject(RtPROPERTIESOBJECT propertiesObject, LPCWSTR remainingName, const Metadata::TypeDefProperties * type)
    {
        AutoHeapString heapString;
        auto nextFragment = GetToDot(remainingName, [&](int strLength)->LPWSTR {
            heapString.CreateNew(strLength);
            return heapString.Get();
        });

        if (!nextFragment.HasValue())
        {
            // We are at the leaf of the identifier. Create a function for the type.
            auto expr = ExprOfTypeDefProperties(type, nullptr);
            auto field = RtAnew(allocator, AbiTypeProperty, stringConverter->IdOfString(remainingName), expr);
            auto fields =
                propertiesObject->fields
                ->Prepend(field, allocator);
            return Anew(allocator,PropertiesObject,fields);
        }
        else
        {
            auto fragment = nextFragment.GetValue();
            auto length = wcslen(fragment);
            if (length == 0)
            {
                // No support for empty identifiers
                return propertiesObject;
            }

            auto childFieldOpt = propertiesObject->GetFieldByIdentifier(stringConverter->IdOfString(fragment), allocator);
            if (childFieldOpt.HasValue())
            {
                auto childField = childFieldOpt.GetValue();
                auto nextRemainingName = remainingName + length + 1;
                auto oldChildSpace = (PropertiesObject*)childField->expr;
                if(oldChildSpace->type == exprPropertiesObject)
                {
                    auto newChildSpace = ApplyTypeToPropertiesObject(oldChildSpace, nextRemainingName, type);
                    auto field = RtAnew(allocator, AbiNamespaceProperty, stringConverter->IdOfString(fragment), newChildSpace);
                    auto fields =
                        propertiesObject->fields
                        ->Where([&](RtPROPERTY field) {return field!=childField;}, allocator)
                        ->Prepend(field, allocator);
                    return Anew(allocator,PropertiesObject,fields);
                }
                else
                {
                    // NOTE: This path means there is a type with the same name as a namespace. For now, the first one seen is the winner.
                    return propertiesObject;
                }
            }
            auto childSpace = Anew(allocator, PropertiesObject, noFields);
            auto field = RtAnew(allocator, AbiNamespaceProperty, stringConverter->IdOfString(fragment), childSpace);
            auto fields =
                propertiesObject->fields
                ->Prepend(field,allocator);
            auto newPropertiesObject = Anew(allocator, PropertiesObject, fields);
            return ApplyTypeToPropertiesObject(newPropertiesObject, remainingName, type);
        }
    }

    // Info:        Add the next dot-delimited fragment of remaining name into the AssignmentSpace
    // Parameters:  currentSpace - the record space to add into
    //              remainingName - the portion of the dotted name remaining to be resolved
    //              type - native type
    //              assembly - assembly which holds the native member
    RtASSIGNMENTSPACE ProjectionBuilder::ApplyTypeInAssignmentSpace(RtASSIGNMENTSPACE currentSpace, LPCWSTR remainingName, const Metadata::TypeDefProperties * type)
    {
        if (!type->IsNested() && type->IsWindowsRuntime())
        {
            auto nextFragment = GetToDot(remainingName, [&](int strLength)->LPWSTR {
                return AnewArray(allocator, char16, strLength);
            });

            if (!nextFragment.HasValue())
            {
                // No support for types in global namespace.
                return currentSpace;
            }
            else
            {
                auto fragment = nextFragment.GetValue();
                auto length = wcslen(fragment);
                if (length == 0)
                {
                    // No support for empty identifiers
                    return currentSpace;
                }

                auto childAssignmentOpt = currentSpace->GetAssignmentByIdentifier(fragment, allocator);
                if (childAssignmentOpt.HasValue())
                {
                    auto childAssignment = childAssignmentOpt.GetValue();
                    auto nextRemainingName = remainingName + length + 1;
                    auto oldChildSpace = (PropertiesObject*)childAssignment->expr;
                    if(oldChildSpace->type == exprPropertiesObject)
                    {
                        auto newChildSpace = ApplyTypeToPropertiesObject(oldChildSpace, nextRemainingName, type);
                        auto var = Anew(allocator, Assignment, fragment, newChildSpace);
                        auto vars =
                            currentSpace->vars
                                ->Where([&](RtASSIGNMENT var) {return var!=childAssignment;}, allocator)
                                ->Prepend(var, allocator);
                        return Anew(allocator,AssignmentSpace,vars);
                    }
                    else
                    {
                        // NOTE: This path means there is a type with the same name as a namespace. For now, the first one seen is the winner.
                        return currentSpace;
                    }
                }
                auto childSpace = Anew(allocator, PropertiesObject, noFields);
                auto var = Anew(allocator, Assignment, fragment, childSpace);
                auto vars = currentSpace->vars->Prepend(var, allocator);
                auto newAssignmentspace = Anew(allocator, AssignmentSpace, vars);
                return ApplyTypeInAssignmentSpace(newAssignmentspace, remainingName, type);
            }
        }
        return currentSpace;
    }

    // Info:        Apply the given remaining assembly into the given (optional) basis assignment space.
    // Parameters:  basisAssignmentSpace - prior assignment space
    //              fileName - the file name to apply
    // Returns:     a new assignment space with the assembly's types in it
    RtASSIGNMENTSPACE ProjectionBuilder::AddFromMetadataImport(Option<AssignmentSpace> basisAssignmentSpace, Metadata::Assembly * assembly)
    {
        auto varspace = basisAssignmentSpace.GetValueOrDefaultValue(emptyAssignmentSpace);

        assembly->TypeDefinitions()
            ->Iterate([&](const Metadata::TypeDefProperties * type) {
                varspace = ApplyTypeInAssignmentSpace(varspace, stringConverter->StringOfId(type->id), type);
        });
        return varspace;
    }

    // Info:        Lower case a list of events
    // Parameters:  events - the list of events
    // Returns:     Lower-cased events
    ImmutableList<RtEVENT> * ProjectionBuilder::LowerCaseEvents(ImmutableList<RtEVENT> * events)
    {
        typedef Tuple<MetadataStringId,RtEVENT> NamedEvent;

        // Lower case an identifier name
        auto lowerCaser = [&](MetadataStringId identifier)->MetadataStringId {
            LPCWSTR identifierString = stringConverter->StringOfId(identifier);
            auto identifierLength = wcslen(identifierString)+1;
            AutoHeapString lowerCasedIdentifier;
            lowerCasedIdentifier.CreateNew(identifierLength);
            wcscpy_s(lowerCasedIdentifier.Get(), lowerCasedIdentifier.GetLength(), identifierString);

            // Lower case the last segment
            auto eventInsertionPoint = const_cast<char16*>(LastSegmentByDot(lowerCasedIdentifier.Get()));

            // Lower case entire identifier
            CharLowerW(eventInsertionPoint);

            return stringConverter->IdOfString(lowerCasedIdentifier.Get());
        };


        auto namedEvents = events->Select<NamedEvent>([&](RtEVENT evnt) {
            return NamedEvent(lowerCaser(evnt->nameId), evnt);
        }, allocator);

        // Determine whether two identifiers are the same.
        auto areSameEventIdentifier = [&](NamedEvent a, NamedEvent b) {
            return (a.First() == b.First());
        };

        auto eventsGroupedByIdentifier = namedEvents->GroupBy(areSameEventIdentifier, allocator);
        auto newEvents = eventsGroupedByIdentifier->Accumulate(ImmutableList<RtEVENT>::Empty(),
            [&](ImmutableList<RtEVENT> * prior, ImmutableList<NamedEvent> * g)->ImmutableList<RtEVENT>* {
            if (g->Count()==1)
            {
                auto oldEvt = g->ToSingle();
                auto newNameId = oldEvt.First();
                auto newEvt = RenameEventName(oldEvt.Second(), newNameId, allocator
#if DBG
                            , stringConverter->StringOfId(newNameId)
#endif
                    );
                return prior->Prepend(newEvt, allocator);
            }

            Js::Throw::FatalProjectionError();
        });

        return newEvents;
    }

    // Info:        Camel case a list of properties
    // Parameters:  fields - the list of properties
    //              reservedNames - list of reserved names
    // Returns:     The camel-cased properties
    ImmutableList<RtPROPERTY> * ProjectionBuilder::CamelCaseProperties(ImmutableList<RtPROPERTY> * fields, __in_opt ImmutableList<MetadataStringId> * reservedNames)
    {
        typedef Tuple<MetadataStringId,RtPROPERTY> NamedProperty;

        // Determine whether two identifiers are the same.
        auto areSamePropertyIdentifier = [&](NamedProperty p1, NamedProperty p2) {
            return (p1.First() == p2.First());
        };

        auto conflictsWithReservedName = [&](RtPROPERTY prop)->bool{
            if (reservedNames == nullptr) { return false; }
            auto stringConverter = this->stringConverter;
            auto idToCCId = idToCamelCasedId;
            return reservedNames->ContainsWhere([&](MetadataStringId id) {
                return (id == CamelCaseString(prop->identifier, allocator, stringConverter, idToCCId));
            });
        };

        // Returns true if the property is event handler attaching or detaching field
        auto isEventField = [&](RtPROPERTY prop) {
            return (prop->propertyType == ptAbiAddEventListenerProperty || prop->propertyType == ptAbiRemoveEventListenerProperty || prop->propertyType == ptAbiEventHandlerProperty);
        };

        auto propertiesWithCamelCase =
            fields->Select<NamedProperty>([&](RtPROPERTY prop) {
                        return NamedProperty(CamelCaseString(prop->identifier, allocator, stringConverter, idToCamelCasedId), prop);
                    }, allocator);

        auto propertiesGroupedByIdentifier = propertiesWithCamelCase->GroupBy(areSamePropertyIdentifier, allocator);
        propertiesWithCamelCase->FreeList(allocator);
        propertiesWithCamelCase = nullptr;

        auto newProperties = propertiesGroupedByIdentifier->Accumulate(ImmutableList<RtPROPERTY>::Empty(),
            [&](ImmutableList<RtPROPERTY> * prior, ImmutableList<NamedProperty> * g)->ImmutableList<RtPROPERTY>* {
            if (g->Count()==1)
            {
                auto camelCasedId = g->ToSingle().First();
                auto oldProp = g->ToSingle().Second();

                g->FreeList(allocator);
                g = nullptr;

                if (isEventField(oldProp) || ((reservedNames != nullptr) && conflictsWithReservedName(oldProp)))
                {
                    return prior->Prepend(oldProp, allocator);
                }
                auto newProp = CreateRenamedPropertyIdentifierAsCopy(oldProp, camelCasedId, allocator);
                return prior->Prepend(newProp, allocator);
            }

            // Assert Here after the midlrt changes reach the branch
            // Note, AppendListToCurrentList does not copy the appended list,
            // so don't need to free the result of the Select
            auto result = g->Select<RtPROPERTY>([](NamedProperty namedProp) { return namedProp.Second(); }, allocator)
                    ->AppendListToCurrentList(prior); // Two or more properties that conflict only by camelCase

            g->FreeList(allocator);
            g = nullptr;

            return result;
        });

        propertiesGroupedByIdentifier->FreeList(allocator);
        propertiesGroupedByIdentifier = nullptr;

        return newProperties;
    }

    RtINTERFACECONSTRUCTOR ProjectionBuilder::CamelCaseInterfaceConstructor(RtINTERFACECONSTRUCTOR ic, __in_opt ImmutableList<MetadataStringId> * reservedNames)
    {
        if (ic->interfaceType == ifRuntimeInterfaceConstructor)
        {
            auto rtic = RuntimeInterfaceConstructor::From(ic);
            auto newOwnProperties = CamelCaseProperties(ic->ownProperties, reservedNames);
            return Anew(allocator, RuntimeInterfaceConstructor, rtic->typeId, rtic->typeDef, rtic->signature, rtic->iid, rtic->genericParameters,
                    rtic->properties, rtic->prototype, newOwnProperties, rtic->requiredInterfaces, rtic->locatorNames, rtic->nameCount,
                    rtic->specialization, rtic->hasEventHandlers);
        }
        return ic;
    }

    RtTYPE ProjectionBuilder::GetKnownTypeByName(LPCWSTR fullTypeName)
    {
        if (wcscmp(fullTypeName, GetWindowsFoundationDateTimeTypeName()) == 0)
        {
            return GetWindowsFoundationDateTimeType();
        }
        if (wcscmp(fullTypeName, GetWindowsFoundationTimeSpanTypeName()) == 0)
        {
            return GetWindowsFoundationTimeSpanType();
        }
        if (wcscmp(fullTypeName, GetWindowsFoundationEventRegistrationTokenTypeName()) == 0)
        {
            return GetWindowsFoundationEventRegistrationTokenType();
        }
        if (wcscmp(fullTypeName, GetWindowsFoundationHResultTypeName()) == 0)
        {
            return GetWindowsFoundationHResultType();
        }
        // TODO :
        // _u("Windows.Foundation.IAsyncInfo") ? probably
        return nullptr;
    }

    RtTYPE ProjectionBuilder::GetBasicAndKnownTypeByName(LPCWSTR fullTypeName)
    {
        if (wcscmp(fullTypeName, _u("Int32")) == 0)
        {
            return typeInt32;
        }
        else if (wcscmp(fullTypeName, _u("String")) == 0)
        {
            return typeString;
        }
        else if (wcscmp(fullTypeName, _u("Object")) == 0)
        {
            return typeObject;
        }
        else if (wcscmp(fullTypeName, _u("Guid")) == 0)
        {
            return typeSystemGuid;
        }
        else if (wcscmp(fullTypeName, _u("UInt32")) == 0)
        {
            return typeUint32;
        }
        else if (wcscmp(fullTypeName, _u("Boolean")) == 0)
        {
            return typeBool;
        }
        else if (wcscmp(fullTypeName, _u("Double")) == 0)
        {
            return typeDouble;
        }
        else if (wcscmp(fullTypeName, _u("Single")) == 0)
        {
            return typeFloat;
        }
        else if (wcscmp(fullTypeName, _u("UInt8")) == 0)
        {
            return typeByte;
        }
        else if (wcscmp(fullTypeName, _u("Int64")) == 0)
        {
            return typeInt64;
        }
        else if (wcscmp(fullTypeName, _u("UInt64")) == 0)
        {
            return typeUint64;
        }
        else if (wcscmp(fullTypeName, _u("Char16")) == 0)
        {
            return typeCharProjectedAsString;
        }
        else if (wcscmp(fullTypeName, _u("Int16")) == 0)
        {
            return typeInt16;
        }
        else if (wcscmp(fullTypeName, _u("UInt16")) == 0)
        {
            return typeUint16;
        }

        return GetKnownTypeByName(fullTypeName);
    }

    inline BOOL ProjectionBuilder::IsTypeWebHidden(const Metadata::TypeDefProperties * type)
    {
        return type->assembly.IsAttributePresent(type->td, _u("Windows.Foundation.Metadata.WebHostHiddenAttribute")) && !IgnoreWebHidden();
    }

    bool ProjectionBuilder::TypePartiallyResolved(const Metadata::TypeDefProperties * type)
    {
        if (runtimeClassCache->ContainsKey(type->id))
        {
            return false;
        }

        CheckForDuplicateTypeId checker(&currentImplementedRuntimeClassInterfaceConstructors);
        DeferredTypeDefinitionCandidate typeCandidate(type->id, nullptr);
        return checker.Contains(typeCandidate);
    }

    inline BOOL ProjectionBuilder::IsTypeAllowedForWeb(const Metadata::TypeDefProperties * type)
    {
        return !EnforceAllowForWeb() || type->assembly.IsAttributePresent(type->td, _u("Windows.Foundation.Metadata.AllowForWebAttribute"));
    }
}

