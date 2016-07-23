//---------------------------------------------------------------------------
// Copyright (C) Microsoft.  All rights reserved.
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "ProjectionToTypeScriptConverter.h"

using namespace Metadata;
using namespace ProjectionModel;
using namespace std;

NamespaceContext::NamespaceContext(IStringConverter& converter) :
    m_converter(converter)
{
}

void NamespaceContext::PushNamespace(MetadataString namespaceNameId)
{
    m_currentNamespace += namespaceNameId.ToString();
    m_currentNamespace += L'.';
}

void NamespaceContext::PopNamespace()
{
    // Trim back to after the second to last dot
    m_currentNamespace.resize(m_currentNamespace.rfind(L'.', m_currentNamespace.size() - 2) + 1);
}

MetadataString NamespaceContext::GetNameIdRelativeToCurrentContext(MetadataString fullyQualifiedNameId)
{
    // To avoid ambiguity during name resolution, only use a relative name
    // if the name is within the current namespace (no relative references to parent namespaces)

    auto fullyQualifiedName = fullyQualifiedNameId.ToString();

    if (m_currentNamespace.size() > 0 && m_currentNamespace.compare(0, m_currentNamespace.size(), fullyQualifiedName, m_currentNamespace.size()) == 0)
    {
        // Get the relative name
        auto relativeName = fullyQualifiedName + m_currentNamespace.size();

        Assert(*relativeName != L'\0');

        if (*relativeName != L'\0')
        {
            return relativeName;
        }
    }

    return fullyQualifiedName;
}

ProjectionToTypeScriptConverter::ProjectionToTypeScriptConverter(ArenaAllocator* alloc, TypeScriptEmitter& emitter, IndentingWriter& writer, IStringConverter& converter, bool emitAnyForUnresolvedTypes) :
    m_alloc(alloc),
    m_emitter(emitter),
    m_writer(writer),
    m_converter(converter),
    m_indexOfStringId(converter.IdOfString(L"IndexOf")),
    m_exclusiveToAttributeStringId(converter.IdOfString(L"Windows.Foundation.Metadata.ExclusiveToAttribute")),
    m_voidStringId(converter.IdOfString(L"Void")),
    m_namespaceContext(converter),
    m_emitAnyForUnresolvedTypes(emitAnyForUnresolvedTypes)
{
    EmitSpecialTypes();
}

void ProjectionToTypeScriptConverter::EmitTopLevelNamespace(MetadataStringId namespaceNameId, RtPROPERTIESOBJECT childProperties)
{
    m_emitter.OpenAmbientNamespace(namespaceNameId);
    m_namespaceContext.PushNamespace(namespaceNameId);

    EmitTypeDefinitions(childProperties);

    m_namespaceContext.PopNamespace();
    m_emitter.CloseNamespace();
}

void ProjectionToTypeScriptConverter::EmitSpecialTypes()
{
    m_writer.WriteLine(L"declare namespace Windows.Foundation.Projections {");
    m_writer.Indent();
    m_writer.WriteLine(L"class Promise<T> {");
    m_writer.Indent();
    m_writer.WriteLine(L"cancel(): void;");
    m_writer.WriteLine(L"done<U>(onComplete?: (value: T) => any, onError?: (error: any) => any, onProgress?: (progress: any) => void): void;");
    m_writer.WriteLine(L"then<U>(onComplete?: (value: T) => Promise<U>, onError?: (error: any) => Promise<U>, onProgress?: (progress: any) => void): Promise<U>;");
    m_writer.WriteLine(L"then<U>(onComplete?: (value: T) => Promise<U>, onError?: (error: any) => U, onProgress?: (progress: any) => void): Promise<U>;");
    m_writer.WriteLine(L"then<U>(onComplete?: (value: T) => Promise<U>, onError?: (error: any) => void, onProgress?: (progress: any) => void): Promise<U>;");
    m_writer.WriteLine(L"then<U>(onComplete?: (value: T) => U, onError?: (error: any) => Promise<U>, onProgress?: (progress: any) => void): Promise<U>;");
    m_writer.WriteLine(L"then<U>(onComplete?: (value: T) => U, onError?: (error: any) => U, onProgress?: (progress: any) => void): Promise<U>;");
    m_writer.WriteLine(L"then<U>(onComplete?: (value: T) => U, onError?: (error: any) => void, onProgress?: (progress: any) => void): Promise<U>;");
    m_writer.WriteLine(L"then<U>(onComplete?: (value: T) => void, onError?: (error: any) => Promise<U>, onProgress?: (progress: any) => void): Promise<U>;");
    m_writer.WriteLine(L"then<U>(onComplete?: (value: T) => void, onError?: (error: any) => U, onProgress?: (progress: any) => void): Promise<U>;");
    m_writer.WriteLine(L"then<U>(onComplete?: (value: T) => void, onError?: (error: any) => void, onProgress?: (progress: any) => void): Promise<U>;");
    m_writer.Unindent();
    m_writer.WriteLine(L"}");
    m_writer.Unindent();
    m_writer.WriteLine(L"}");
}

void ProjectionToTypeScriptConverter::EmitNamespace(MetadataStringId namespaceNameId, RtPROPERTIESOBJECT childProperties)
{
    m_emitter.OpenNamespace(namespaceNameId);
    m_namespaceContext.PushNamespace(namespaceNameId);

    EmitTypeDefinitions(childProperties);

    m_namespaceContext.PopNamespace();
    m_emitter.CloseNamespace();
}

void ProjectionToTypeScriptConverter::EmitTypeDefinitions(RtPROPERTIESOBJECT properties)
{
    properties->fields->Iterate([&](RtPROPERTY prop) {

        if (prop->propertyType == ptAbiNamespaceProperty)
        {
            EmitNamespace(prop->identifier, PropertiesObject::From(prop->expr));
        }
        else if (prop->propertyType == ptAbiTypeProperty)
        {
            if (prop->expr->type == exprEnum)
            {
                auto declaration = GetTypeScriptDeclarationForEnum(prop->identifier, Enum::From(prop->expr));
                m_emitter.EmitEnumDeclaration(*declaration);
            }
            else if (prop->expr->type == exprFunction)
            {
                auto functionType = TypeConstructor::From(prop->expr)->functionType;

                switch (functionType)
                {
                case functionStructConstructor:
                {
                    auto declaration = GetTypeScriptDeclarationForStruct(prop->identifier, StructConstructor::From(prop->expr));
                    m_emitter.EmitInterfaceDeclaration(*declaration);
                }
                break;
                case functionInterfaceConstructor:
                {
                    auto interfaceConstructor = InterfaceConstructor::From(prop->expr);
                    auto interfaceType = interfaceConstructor->interfaceType;
                    Assert(interfaceType == ifRuntimeInterfaceConstructor || interfaceType == ifMissingInterfaceConstructor);

                    if (interfaceType == ifRuntimeInterfaceConstructor)
                    {
                        auto interfaceTypeDef = interfaceConstructor->typeDef;
                        auto hasExclusiveToAttribute = interfaceTypeDef->assembly.CustomAttributes(interfaceTypeDef->td, 0)->ContainsWhere([&](const CustomAttributeProperties* customAttribute) {
                            return (customAttribute->attributeTypeId == m_exclusiveToAttributeStringId);
                        });

                        // Skip this interface if it's exclusive to a specific runtime class, therefore hidden from the user
                        if (!hasExclusiveToAttribute)
                        {
                            auto declaration = GetTypeScriptDeclarationForInterface(prop->identifier, RuntimeInterfaceConstructor::From(prop->expr));
                            m_emitter.EmitInterfaceDeclaration(*declaration);
                        }
                    }
                }
                break;
                case functionRuntimeClassConstructor:
                {
                    auto declaration = GetTypeScriptDeclarationForRuntimeClass(prop->identifier, RuntimeClassConstructor::From(prop->expr));
                    m_emitter.EmitClassDeclaration(*declaration);
                }
                break;
                case functionDelegateConstructor:
                {
                    auto declaration = GetTypeScriptDeclarationForDelegate(prop->identifier, DelegateConstructor::From(prop->expr));
                    m_emitter.EmitInterfaceDeclaration(*declaration);
                }
                break;
                case functionMissingTypeConstructor:
                {
                    // Type is hidden, skip emitting it
                }
                break;
                default:
                    Assert(false);
                }
            }
        }
    });
}

bool ProjectionToTypeScriptConverter::IsTypeVisible(RtTYPE type)
{
    if (MissingNamedType::Is(type) || MissingGenericInstantiationType::Is(type))
    {
        wcerr << L"warning: Reference to missing type: " << m_converter.StringOfId(type->fullTypeNameId) << endl;

        return false;
    }

    if (VoidType::Is(type) || GenericParameterType::Is(type) || WindowsFoundationDateTimeType::Is(type) ||
        WindowsFoundationTimeSpanType::Is(type) || WindowsFoundationEventRegistrationTokenType::Is(type) || WindowsFoundationHResultType::Is(type)
        || SystemGuidType::Is(type))
    {
        return true;
    }
    else if (ByRefType::Is(type))
    {
        return IsTypeVisible(ByRefType::From(type)->pointedTo);
    }
    else if (ArrayType::Is(type))
    {
        return IsTypeVisible(ArrayType::From(type)->elementType);
    }
    else if (TypeDefinitionType::Is(type))
    {
        auto typeDefType = TypeDefinitionType::From(type);

        if (!typeDefType->genericParameters->IsEmpty())
        {
            auto fullyQualifiedName = wstring(m_converter.StringOfId(typeDefType->typeId));

            if (fullyQualifiedName.compare(0, ARRAYSIZE(L"Windows.Foundation.IReference`1<") - 1, L"Windows.Foundation.IReference`1<") == 0)
            {
                Assert(typeDefType->genericParameters->Count() == 1);
                return IsTypeVisible(typeDefType->genericParameters->First());
            }
        }

        bool visible = true;
        if (!typeDefType->genericParameters->IsEmpty())
        {
            typeDefType->genericParameters->Iterate([&](RtTYPE genericParamType) {
                visible &= IsTypeVisible(genericParamType);
            });
        }

        return visible;
    }
    else if (BasicType::Is(type))
    {
        auto basicType = BasicType::From(type);

        switch (basicType->typeCor)
        {
        case ELEMENT_TYPE_STRING:
        case ELEMENT_TYPE_CHAR:
        case ELEMENT_TYPE_BOOLEAN:
        case ELEMENT_TYPE_I1:
        case ELEMENT_TYPE_U1:
        case ELEMENT_TYPE_I2:
        case ELEMENT_TYPE_U2:
        case ELEMENT_TYPE_I4:
        case ELEMENT_TYPE_U4:
        case ELEMENT_TYPE_I8:
        case ELEMENT_TYPE_U8:
        case ELEMENT_TYPE_R4:
        case ELEMENT_TYPE_R8:
        case ELEMENT_TYPE_OBJECT:
        case ELEMENT_TYPE_VAR:
            return true;
        default:
            // Unrecognized type
            Assert(false);
            return false;
        }
    }

    // Unrecognized type
    Assert(false);
    return false;
}

bool ProjectionToTypeScriptConverter::AreAllParameterTypesVisible(RtPARAMETERS params)
{
    bool allVisible = true;
    params->allParameters->Iterate([&](RtPARAMETER param) {
        allVisible &= IsTypeVisible(param->type);
    });

    return allVisible;
}

auto_ptr<TypeScriptEnumDeclaration> ProjectionToTypeScriptConverter::GetTypeScriptDeclarationForEnum(MetadataString enumName, RtENUM enumDefinition)
{
    auto enumDeclaration = TypeScriptEnumDeclaration::Make(enumName);

    enumDefinition->properties->fields->Iterate([&](RtPROPERTY enumField) {
        enumDeclaration->AppendMember(enumField->identifier);
    });

    return enumDeclaration;
}

auto_ptr<TypeScriptClassDeclaration> ProjectionToTypeScriptConverter::GetTypeScriptDeclarationForRuntimeClass(MetadataString runtimeClassName, RtRUNTIMECLASSCONSTRUCTOR rtClassConstructor)
{
    bool extendsArray = false;
    bool extendsMap = false;

    RtTYPE promiseResultType = nullptr;

    auto specialization = rtClassConstructor->specialization;
    if (specialization != nullptr)
    {
        switch (specialization->specializationType)
        {
        case specVectorViewSpecialization:
        case specVectorSpecialization:
            // Vector and VectorView extend array
            extendsArray = true;
            break;
        case specMapSpecialization:
        case specMapViewSpecialization:
            extendsMap = true;
            break;
        case specPromiseSpecialization:
            rtClassConstructor->allInterfaces->IterateWhile([&](RtINTERFACECONSTRUCTOR rtInterface) {
                if (rtInterface->interfaceType == ifRuntimeInterfaceConstructor)
                {
                    promiseResultType = GetPromiseResultType(RuntimeInterfaceConstructor::From(rtInterface));
                }
                return promiseResultType == nullptr;
            });
            break;
        }
    }

    auto isActivatable = (rtClassConstructor->signature->signatureType == mstAbiMethodSignature || rtClassConstructor->signature->signatureType == mstOverloadedMethodSignature);

    Assert(isActivatable || rtClassConstructor->signature->signatureType == mstUncallableMethodSignature || rtClassConstructor->signature->signatureType == mstMissingTypeConstructorMethodSignature);

    // Static members
    auto staticMembers = GetPropertyAndMethodMembers(rtClassConstructor->properties->fields, false, FullyQualifiedNameBehavior::EmitAsIs);

    // Instance members
    auto implements = GetImplementedTypeScriptTypeList(rtClassConstructor->allInterfaces);

    // A fully qualified instance member name in a class indicates that multiple members with the same name have been inherited from different interfaces.
    // The name has then been disambiguated at either the WinMD or projection stage by qualifying it with the full namespace and interface name.
    // Emitting only this fully qualified name would produce a buggy TypeScript file, where the class incorrectly implements its parent interfaces
    // because it's missing the unqualified member name defined in the interfaces.
    //
    // Example (broken TypeScript):
    // 
    // interface Foo { conflictName: number; }
    // interface Bar { conflictName: number; }
    // class Baz implements Foo, Bar { "Foo.conflictName": number; "Bar.conflictName": number; }
    // 
    // To work around this problem, after emitting a fully qualified instance member name in a class, we will also emit the unqualified name, 
    // satisfying the interface definition. Because the parent interfaces may define multiple types for the property, the property type will be 
    // defined as "any". At runtime this property will actually be undefined.
    // 
    // The example above will become (correct TypeScript):
    // 
    // interface Foo { conflictName: number; }
    // interface Bar { conflictName: number; }
    // class Baz implements Foo, Bar { "Foo.conflictName": number; "Bar.conflictName": number; conflictName: any; }

    auto instanceMembers = GetPropertyAndMethodMembers(PropertiesObject::From(rtClassConstructor->prototype)->fields, extendsArray, implements->GetTypes().empty()? FullyQualifiedNameBehavior::EmitAsIs : FullyQualifiedNameBehavior::AlsoEmitUnqualifiedMember);

    // Index signatures, if any
    auto indexSignature = GetTypeScriptIndexSignatureOrNull(extendsMap ? specialization : nullptr);

    if (indexSignature.get() != nullptr)
    {
        instanceMembers->AddMember(move(indexSignature));
    }

    auto classDeclaration = TypeScriptClassDeclaration::Make(
        GetTypeScriptTypeDefinitionType(runtimeClassName, nullptr),
        !isActivatable,
        GetExtendedTypeScriptTypeList(nullptr, specialization, promiseResultType),
        move(implements),
        isActivatable ? GetConstructorOverloads(rtClassConstructor->signature) : TypeScriptConstructorOverloads::Make(),
        move(staticMembers),
        move(instanceMembers)
        );

    return classDeclaration;
}

static MetadataStringId GetMetadataNameOfProperty(RtPROPERTY property) {
    MetadataStringId metadataNameOfThisProperty = MetadataStringIdNil;
    switch (property->propertyType)
    {
    case ptAbiMethodProperty:
    {
        metadataNameOfThisProperty = AbiMethod::From(AbiMethodProperty::From(property)->body)->signature->metadataNameId;
    }
    break;
    case ptAbiPropertyProperty:
    {
        metadataNameOfThisProperty = AbiPropertyProperty::From(property)->metadataNameId;
    }
    break;
    case ptAbiEventHandlerProperty:
    {
        auto eventHandlerProperty = AbiEventHandlerProperty::From(property);
        metadataNameOfThisProperty = eventHandlerProperty->abiEvent->metadataNameId;
    }
    break;
    case ptOverloadParentProperty:
    {
        auto overloadParent = OverloadParentProperty::From(property);
        // Expand the overloads
        auto overloads = OverloadGroupConstructor::From(overloadParent->overloadConstructor)->signature->overloads;
        Assert(overloads->id == property->identifier);

        overloads->overloads->Iterate([&](RtMETHODSIGNATURE methodSig) {
            Assert(methodSig->signatureType == mstAbiMethodSignature);
            metadataNameOfThisProperty = AbiMethodSignature::From(methodSig)->metadataNameId;
        });
    }
    break;
    }

    return metadataNameOfThisProperty;
}

auto_ptr<TypeScriptInterfaceDeclaration> ProjectionToTypeScriptConverter::GetTypeScriptDeclarationForInterface(MetadataString interfaceName, RtRUNTIMEINTERFACECONSTRUCTOR interfaceConstructor)
{
    bool extendsArray = false;
    bool extendsMap = false;

    RtTYPE promiseResultType = nullptr;

    auto specialization = interfaceConstructor->specialization;
    if (specialization != nullptr)
    {
        switch (specialization->specializationType)
        {
        case specVectorViewSpecialization:
        case specVectorSpecialization:
            // Vector and VectorView extend array
            extendsArray = true;
            break;
        case specMapSpecialization:
        case specMapViewSpecialization:
            extendsMap = true;
            break;
        case specPromiseSpecialization:
            promiseResultType = GetPromiseResultType(interfaceConstructor);
            break;
        }
    }

    ImmutableList<MetadataStringId>* ownPropertyNames = interfaceConstructor->ownProperties->Select<MetadataStringId>([](RtPROPERTY prop) { return GetMetadataNameOfProperty(prop); }, m_alloc);

    auto typeMembers = GetInterfaceMethodAndPropertyMembers(extendsArray, ownPropertyNames, interfaceConstructor->prototype->fields);

    // Index signature, if any
    auto indexSignature = GetTypeScriptIndexSignatureOrNull(extendsMap ? specialization : nullptr);

    if (indexSignature.get() != nullptr)
    {
        typeMembers->AddMember(move(indexSignature));
    }

    auto interfaceDeclaration = TypeScriptInterfaceDeclaration::Make(
        GetTypeScriptTypeDefinitionType(interfaceName, interfaceConstructor->genericParameters),
        GetExtendedTypeScriptTypeList(interfaceConstructor->requiredInterfaces, specialization, promiseResultType),
        move(typeMembers)
        );

    return interfaceDeclaration;
}

auto_ptr<TypeScriptInterfaceDeclaration> ProjectionToTypeScriptConverter::GetTypeScriptDeclarationForStruct(MetadataString structName, RtSTRUCTCONSTRUCTOR structConstructor)
{
    auto structDeclaration = TypeScriptInterfaceDeclaration::Make(structName);
    structConstructor->structType->fields->Iterate([&](RtPROPERTY field) {
        auto fieldType = AbiFieldProperty::From(field)->type;
        if (m_emitAnyForUnresolvedTypes || IsTypeVisible(fieldType))
        {
            structDeclaration->AddMember(TypeScriptPropertySignature::Make(field->identifier, GetTypeScriptType(fieldType), false /* optional */));
        }
    });

    return structDeclaration;
}

auto_ptr<TypeScriptInterfaceDeclaration> ProjectionToTypeScriptConverter::GetTypeScriptDeclarationForDelegate(MetadataString delegateName, RtDELEGATECONSTRUCTOR delegateConstructor)
{
    Assert(delegateConstructor->invokeInterface->interfaceType == ifRuntimeInterfaceConstructor);
    auto invokeInterfaceConstructor = RuntimeInterfaceConstructor::From(delegateConstructor->invokeInterface);

    auto interfaceDeclaration = TypeScriptInterfaceDeclaration::Make(GetTypeScriptTypeDefinitionType(delegateName, invokeInterfaceConstructor->genericParameters));

    auto invokeMethod = AbiMethodProperty::From(invokeInterfaceConstructor->ownProperties->ToSingle());
    auto params = invokeMethod->body->signature->GetParameters();

    if (m_emitAnyForUnresolvedTypes || AreAllParameterTypesVisible(params))
    {
        interfaceDeclaration->AddMember(TypeScriptCallSignature::Make(GetTypeScriptParameters(params), GetTypeScriptReturnType(params)));
    }

    return interfaceDeclaration;
}

auto_ptr<TypeScriptType> ProjectionToTypeScriptConverter::GetTypeScriptTypeDefinitionType(MetadataString name, ImmutableList<RtTYPE>* parameters)
{
    wstring typeName = name.ToString();

    if (!parameters->IsEmpty())
    {
        // If the name contains generic parameters, trim up until the backtick character
        auto backTickPos = typeName.find(L'`');

        if (backTickPos != wstring::npos)
        {
            typeName.resize(backTickPos);
        }
    }

    auto_ptr_vector<TypeScriptType> typeArguments;

    parameters->Iterate([&](RtTYPE genericParamType) {
        typeArguments.push_back(GetTypeScriptType(genericParamType));
    });

    return TypeScriptType::MakeTypeReference(typeName.c_str(), move(typeArguments));
}

auto_ptr<TypeScriptType> ProjectionToTypeScriptConverter::GetTypeScriptType(RtTYPE type)
{
    if (m_emitAnyForUnresolvedTypes)
    {
        if (!IsTypeVisible(type))
        {
            return TypeScriptType::Make(L"any");
        }
    }
    else
    {
        // This method should not be getting called for types that we can't emit
        Assert(IsTypeVisible(type));
    }

    if (ByRefType::Is(type))
    {
        return GetTypeScriptType(ByRefType::From(type)->pointedTo);
    }
    else if (ArrayType::Is(type))
    {
        return TypeScriptType::MakeArrayType(GetTypeScriptType(ArrayType::From(type)->elementType));
    }
    else if (TypeDefinitionType::Is(type))
    {
        auto typeDefType = TypeDefinitionType::From(type);

        if (!typeDefType->genericParameters->IsEmpty())
        {
            auto fullyQualifiedName = wstring(m_converter.StringOfId(typeDefType->typeId));

            if (fullyQualifiedName.compare(0, ARRAYSIZE(L"Windows.Foundation.IReference`1<") - 1, L"Windows.Foundation.IReference`1<") == 0)
            {
                Assert(typeDefType->genericParameters->Count() == 1);
                return GetTypeScriptType(typeDefType->genericParameters->First());
            }
        }

        auto relativeName = m_namespaceContext.GetNameIdRelativeToCurrentContext(typeDefType->typeId);

        return GetTypeScriptTypeDefinitionType(relativeName, typeDefType->genericParameters);
    }
    else if (VoidType::Is(type))
    {
        return TypeScriptType::Make(L"void");
    }
    else if (BasicType::Is(type))
    {
        auto basicType = BasicType::From(type);

        LPCWSTR typeName = nullptr;

        switch (basicType->typeCor)
        {
        case ELEMENT_TYPE_STRING:
        case ELEMENT_TYPE_CHAR:
            typeName = L"string";
            break;
        case ELEMENT_TYPE_BOOLEAN:
            typeName = L"boolean";
            break;
        case ELEMENT_TYPE_I1:
        case ELEMENT_TYPE_U1:
        case ELEMENT_TYPE_I2:
        case ELEMENT_TYPE_U2:
        case ELEMENT_TYPE_I4:
        case ELEMENT_TYPE_U4:
        case ELEMENT_TYPE_I8:
        case ELEMENT_TYPE_U8:
        case ELEMENT_TYPE_R4:
        case ELEMENT_TYPE_R8:
            typeName = L"number";
            break;
        case ELEMENT_TYPE_OBJECT:
        case ELEMENT_TYPE_VAR:
            typeName = L"any";
            break;
        default:
            throw UnrecognizedTypeError(type->fullTypeNameId);
        }

        return TypeScriptType::Make(typeName);
    }
    else if (GenericParameterType::Is(type))
    {
        return TypeScriptType::Make(type->fullTypeNameId);
    }
    else if (WindowsFoundationDateTimeType::Is(type))
    {
        return TypeScriptType::Make(L"Date");
    }
    else if (WindowsFoundationTimeSpanType::Is(type) || WindowsFoundationEventRegistrationTokenType::Is(type) || WindowsFoundationHResultType::Is(type))
    {
        return TypeScriptType::Make(L"number");
    }
    else if (SystemGuidType::Is(type))
    {
        return TypeScriptType::Make(L"string");
    }
    else
    {
        throw UnrecognizedTypeError(type->fullTypeNameId);
    }
}

auto_ptr<TypeScriptType> ProjectionToTypeScriptConverter::GetTypeScriptArrayTypeOrNull(RtSPECIALIZATION specialization)
{
    RtTYPE arrayType;
    if (VectorSpecialization::Is(specialization))
    {
        arrayType = VectorSpecialization::From(specialization)->getAt->GetParameters()->returnType;
    }
    else
    {
        arrayType = VectorViewSpecialization::From(specialization)->getAt->GetParameters()->returnType;
    }

    if (m_emitAnyForUnresolvedTypes || IsTypeVisible(arrayType))
    {
        return TypeScriptType::MakeTypeReference(L"Array", GetTypeScriptType(arrayType));
    }

    return auto_ptr<TypeScriptType>();
}

auto_ptr<TypeScriptParameterList> ProjectionToTypeScriptConverter::GetTypeScriptParameters(RtPARAMETERS params)
{
    auto_ptr_vector<TypeScriptParameter> typeScriptParams;

    params->allParameters->Iterate([&](RtPARAMETER param) {
        if (param->isIn)
        {
            typeScriptParams.push_back(TypeScriptParameter::Make(param->id, GetTypeScriptType(param->type)));
        }
    });

    return TypeScriptParameterList::Make(move(typeScriptParams));
}

auto_ptr<TypeScriptType> ProjectionToTypeScriptConverter::GetTypeScriptReturnType(RtPARAMETERS params)
{
    std::vector<RtPARAMETER> outParams;
    params->allParameters->Iterate([&](RtPARAMETER param) {
        if (!param->isIn && param->isOut)
        {
            outParams.push_back(param);
        }
    });

    if (outParams.size() < 2)
    {
        // Non-object type
        return GetTypeScriptType(params->returnType);
    }
    else
    {
        auto_ptr_vector<TypeScriptPropertySignature> objectTypeMembers;

        for (auto param : outParams)
        {
            objectTypeMembers.push_back(TypeScriptPropertySignature::Make(param->id, GetTypeScriptType(param->type), false /* optional */).release());
        }

        return TypeScriptType::MakeObjectType(move(objectTypeMembers));
    }
}

auto_ptr<TypeScriptTypeMemberList> ProjectionToTypeScriptConverter::GetTypeScriptMethodSignatures(MetadataString methodName, bool applyIndexOfSpecialization, RtMETHODSIGNATURE methodSignature, FullyQualifiedNameBehavior fullyQualifiedNameBehavior)
{
    auto methods = TypeScriptTypeMemberList::Make();

    auto params = methodSignature->GetParameters();

    if (m_emitAnyForUnresolvedTypes || AreAllParameterTypesVisible(params))
    {
        auto methodNameStr = methodName.ToString();

        // If this is the "indexOf" method of an Array specialization we need to define the
        // return type as "any" to avoid conflict between the IVector.indexOf and Array.indexOf properties
        bool forceReturnTypeToAny = applyIndexOfSpecialization && (wcscmp(methodNameStr, L"indexOf") == 0);

        if (forceReturnTypeToAny)
        {
            methods->AddMember(TypeScriptMethodSignature::Make(methodNameStr, GetTypeScriptParameters(params), TypeScriptType::Make(L"any"), GetTypeScriptReturnType(params)));
        }
        else
        {
            AddPropertyToMemberList(
                methods.get(),
                methodNameStr,
                fullyQualifiedNameBehavior,
                [&](MetadataString name, bool optional) {return TypeScriptMethodSignature::Make(name, GetTypeScriptParameters(params), GetTypeScriptReturnType(params), optional); },
                [&](MetadataString name, bool optional) {return TypeScriptMethodSignature::Make(name, GetTypeScriptParameters(params), GetTypeScriptReturnType(params), optional); }
            );            
        }
    }

    return methods;
}

auto_ptr<TypeScriptIndexSignature> ProjectionToTypeScriptConverter::GetTypeScriptIndexSignatureOrNull(RtSPECIALIZATION mapSpecialization)
{
    if (mapSpecialization != nullptr)
    {
        RtPARAMETERS lookupMethodParameters;
        if (MapSpecialization::Is(mapSpecialization))
        {
            lookupMethodParameters = MapSpecialization::From(mapSpecialization)->lookup->GetParameters();
        }
        else
        {
            lookupMethodParameters = MapViewSpecialization::From(mapSpecialization)->lookup->GetParameters();
        }

        auto inParameter = lookupMethodParameters->allParameters->WhereFirst([&](RtPARAMETER param) {
            return param->isIn;
        }).GetValue();

        auto indexType = (*inParameter)->type;
        auto returnType = lookupMethodParameters->returnType;
        if (m_emitAnyForUnresolvedTypes || (IsTypeVisible(indexType) && IsTypeVisible(returnType)))
        {
            // Force the return type to "any" as the interface may contain other members.
            return TypeScriptIndexSignature::Make(GetTypeScriptType(indexType), TypeScriptType::Make(L"any"));
        }
    }

    return auto_ptr<TypeScriptIndexSignature>();
}

auto_ptr<TypeScriptTypeList> ProjectionToTypeScriptConverter::GetExtendedTypeScriptTypeList(ImmutableList<RtINTERFACECONSTRUCTOR>* extendsInterfaces, RtSPECIALIZATION specialization, RtTYPE promiseResultType)
{
    auto_ptr_vector<TypeScriptType> extendedTypes;

    extendsInterfaces->Iterate([&](RtINTERFACECONSTRUCTOR parentInterface) {
        if (parentInterface->interfaceType != ifMissingInterfaceConstructor && parentInterface->interfaceType != ifMissingInstantiationConstructor)
        {
            auto interfaceType = parentInterface->signature->GetParameters()->returnType;
            if (IsTypeVisible(interfaceType))
            {
                extendedTypes.push_back(GetTypeScriptType(interfaceType));
            }
        }
    });

    if (specialization != nullptr)
    {
        switch (specialization->specializationType)
        {
        case specPromiseSpecialization:
            if (promiseResultType != nullptr && (m_emitAnyForUnresolvedTypes || IsTypeVisible(promiseResultType)))
            {
                auto_ptr_vector<TypeScriptType> typeArguments;
                typeArguments.push_back(GetTypeScriptType(promiseResultType));

                extendedTypes.push_back(TypeScriptType::MakeTypeReference(L"Windows.Foundation.Projections.Promise", move(typeArguments)));
            }
            break;
        case specVectorViewSpecialization:
        case specVectorSpecialization:
        {
            auto arrayType = GetTypeScriptArrayTypeOrNull(specialization);
            if (arrayType.get() != nullptr)
            {
                extendedTypes.push_back(move(arrayType));
            }
        }
        break;
        }
    }

    return TypeScriptTypeList::Make(move(extendedTypes));
}

auto_ptr<TypeScriptTypeList> ProjectionToTypeScriptConverter::GetImplementedTypeScriptTypeList(ImmutableList<RtINTERFACECONSTRUCTOR>* implementedInterfaces)
{
    auto_ptr_vector<TypeScriptType> implementedTypes;

    implementedInterfaces->Iterate([&](RtINTERFACECONSTRUCTOR parentInterface) {
        if (parentInterface->interfaceType != ifMissingInterfaceConstructor && parentInterface->interfaceType != ifMissingInstantiationConstructor)
        {
            if (parentInterface->typeDef->assembly.CustomAttributes(parentInterface->typeDef->td, 0)->ContainsWhere([&](const CustomAttributeProperties* customAttribute) {
                return (customAttribute->attributeTypeId == m_exclusiveToAttributeStringId);
            }))
            {
                // This interface is exclusive to this runtime class. No need to declare it.
                return;
            }

            auto interfaceType = parentInterface->signature->GetParameters()->returnType;
            if (IsTypeVisible(interfaceType))
            {
                implementedTypes.push_back(GetTypeScriptType(interfaceType));
            }
        }
    });

    return TypeScriptTypeList::Make(move(implementedTypes));
}

template <typename MakeMemberFunction, typename MakeAnyMemberFunction>
void ProjectionToTypeScriptConverter::AddPropertyToMemberList(
    TypeScriptTypeMemberList* memberList, 
    MetadataString propertyName, 
    FullyQualifiedNameBehavior fullyQualifiedNameBehavior,
    MakeMemberFunction makeMember,
    MakeAnyMemberFunction makeAnyMember
    )
{
    auto propertyNameString = propertyName.ToString();
    auto shortName = GetLastPartIfFullyQualified(propertyNameString);
    if (shortName != propertyNameString && fullyQualifiedNameBehavior == FullyQualifiedNameBehavior::EmitAsOptional)
    {
        memberList->AddMember(makeMember(propertyName, true));
    }
    else
    {
        memberList->AddMember(makeMember(propertyName, false));

        if (shortName != propertyNameString && fullyQualifiedNameBehavior == FullyQualifiedNameBehavior::AlsoEmitUnqualifiedMember)
        {
            auto unqualifiedAny = makeAnyMember(shortName, false);

            if (!memberList->ContainsMember(*unqualifiedAny))
            {
                memberList->AddMember(move(unqualifiedAny));
            }
        }
    }
}

auto_ptr<TypeScriptTypeMemberList> ProjectionToTypeScriptConverter::GetPropertyAndMethodMembers(ImmutableList<RtPROPERTY>* properties, bool applyIndexOfSpecialization, FullyQualifiedNameBehavior fullyQualifiedNameBehavior)
{
    auto memberList = TypeScriptTypeMemberList::Make();

    properties->Iterate([&](RtPROPERTY property) {
        switch (property->propertyType)
        {
        case ptOverloadParentProperty:
        {
            auto overloadParent = OverloadParentProperty::From(property);
            // Expand the overloads
            auto overloads = OverloadGroupConstructor::From(overloadParent->overloadConstructor)->signature->overloads;

            overloads->overloads->Iterate([&](RtMETHODSIGNATURE methodSig) {
                auto typeScriptMethodSigs = GetTypeScriptMethodSignatures(overloads->id, applyIndexOfSpecialization, methodSig, fullyQualifiedNameBehavior);

                memberList->AddMembers(move(typeScriptMethodSigs));
            });
        }
        break;
        case ptAbiAddEventListenerProperty:
        {
            AbiAddEventListenerProperty::From(property)->events->Iterate([&](RtEVENT event) {
                auto eventListenerMethod = GetEventListenerMethodSignatureOrNull(property->identifier, event);
                if (eventListenerMethod.get() != nullptr)
                {
                    memberList->AddMember(move(eventListenerMethod));
                }
            });

            memberList->AddMember(GetGeneralEventListenerMethodSignature(property->identifier));
        }
        break;
        case ptAbiRemoveEventListenerProperty:
        {
            AbiRemoveEventListenerProperty::From(property)->events->Iterate([&](RtEVENT event) {
                auto eventListenerMethod = GetEventListenerMethodSignatureOrNull(property->identifier, event);
                if (eventListenerMethod.get() != nullptr)
                {
                    memberList->AddMember(move(eventListenerMethod));
                }
            });

            memberList->AddMember(GetGeneralEventListenerMethodSignature(property->identifier));
        }
        break;
        case ptAbiMethodProperty:
        {
            auto typeScriptMethodSigs = GetTypeScriptMethodSignatures(property->identifier, applyIndexOfSpecialization, AbiMethod::From(AbiMethodProperty::From(property)->body)->signature, fullyQualifiedNameBehavior);
            
            memberList->AddMembers(move(typeScriptMethodSigs));
        }
        break;
        case ptAbiPropertyProperty:
        {
            auto propertyType = AbiPropertyProperty::From(property)->GetPropertyType();
            if (m_emitAnyForUnresolvedTypes || IsTypeVisible(propertyType))
            {
                AddPropertyToMemberList(
                    memberList.get(), 
                    property->identifier,
                    fullyQualifiedNameBehavior, 
                    [&](MetadataString name, bool optional) { return TypeScriptPropertySignature::Make(name, GetTypeScriptType(propertyType), optional); },
                    [&](MetadataString name, bool optional) { return TypeScriptPropertySignature::Make(name, TypeScriptType::Make(L"any"), false); }
                    );
            }
        }
        break;
        case ptAbiEventHandlerProperty:
        {
            auto eventHandlerProperty = AbiEventHandlerProperty::From(property);
            auto eventHandlerType = eventHandlerProperty->abiEvent->addOn->GetParameters()->allParameters->WhereSingle([](RtPARAMETER param) {
                return param->isIn;
            })->type;

            if (m_emitAnyForUnresolvedTypes || IsTypeVisible(eventHandlerType))
            {
                AddPropertyToMemberList(
                    memberList.get(),
                    property->identifier,
                    fullyQualifiedNameBehavior,
                    [&](MetadataString name, bool optional) { return TypeScriptPropertySignature::Make(name, GetTypeScriptType(eventHandlerType), optional); },
                    [&](MetadataString name, bool optional) { return TypeScriptPropertySignature::Make(name, TypeScriptType::MakeFunctionType(TypeScriptParameterList::Make(L"args"), TypeScriptType::Make(L"any")), optional); }
                );
            }
        }
        break;
        case ptFunctionLengthProperty:
        case ptNone:
        break;
        default:
            throw UnexpectedPropertyError(property->identifier, property->propertyType);
        }
    });

    return memberList;
}

auto_ptr<TypeScriptTypeMemberList> ProjectionToTypeScriptConverter::GetInterfaceMethodAndPropertyMembers(bool extendsArray, ImmutableList<MetadataStringId>* metadataNameFilter, ImmutableList<RtPROPERTY>* properties)
{
    auto filteredProperties = properties->Where([&](RtPROPERTY property) {
        auto nameId = GetMetadataNameOfProperty(property);
        MetadataString metadataNameOfThisProperty;
        if (nameId != MetadataStringIdNil)
        {
            metadataNameOfThisProperty = nameId;
        }

        // Only include properties that are the interface's own
        if (metadataNameOfThisProperty.ToId() != MetadataStringIdNil && !metadataNameFilter->ContainsWhere([&](MetadataStringId propId) { return propId == metadataNameOfThisProperty.ToId(); }) &&
            (!extendsArray || metadataNameOfThisProperty.ToId() != m_indexOfStringId))
        {
            return false;
        }

        return true;

    }, m_alloc);

    // A fully qualified type member name in an interface indicates that this interface has inherited a member with the same name from a parent interface.
    // The name has then been disambiguated at either the WinMD or projection stage by qualifying it with the full namespace and interface name.
    //
    // For some method overloads, at the interface level the same name is considered a conflict, but at the class level it becomes
    // a simple method overload (this may be a bug in the WinRT projection layer).
    // 
    // Then emitting the fully qualified name within the interface may produce a buggy TypeScript file, where the class incorrectly implements its parent interface
    // because it's missing the fully member name defined in the interface. 
    //
    // Example (broken TypeScript):
    // 
    // interface Foo { conflictName(param: number): void; }
    // interface Bar extends Foo { "Foo.conflictName"(param: number): void; "Bar.conflictName"(param1: number, param2: string): void; }
    // class Baz implements Foo, Bar { conflictName(param: number): void; conflictName(param1: number, param2: string): void; }
    // 
    // To work around this problem, fully qualified members of an interface will always be defined as optional. 
    // A runtime class that implements this interface then may or may not define them.
    // 
    // The example above will become (correct TypeScript):
    // 
    // interface Foo { conflictName(param: number): void; }
    // interface Bar extends Foo { "Foo.conflictName"?(param: number): void; "Bar.conflictName"?(param1: number, param2: string): void; }
    // class Baz implements Foo, Bar { conflictName(param: number): void; conflictName(param1: number, param2: string): void; }

    return GetPropertyAndMethodMembers(filteredProperties, extendsArray, FullyQualifiedNameBehavior::EmitAsOptional);
}

auto_ptr<TypeScriptConstructorOverloads> ProjectionToTypeScriptConverter::GetConstructorOverloads(RtMETHODSIGNATURE methodSignature)
{
    auto_ptr_vector<TypeScriptParameterList> overloads;
    switch (methodSignature->signatureType)
    {
    case mstAbiMethodSignature:
    {
        auto parameters = methodSignature->GetParameters();

        if (m_emitAnyForUnresolvedTypes || AreAllParameterTypesVisible(parameters))
        {
            overloads.push_back(GetTypeScriptParameters(parameters));
        }
    }
    break;
    case mstOverloadedMethodSignature:
    {
        OverloadedMethodSignature::From(methodSignature)->overloads->overloads->Iterate([&](RtMETHODSIGNATURE overloadSignature) {
            overloads.insert(GetConstructorOverloads(overloadSignature)->MoveOverloadParameterLists());
        });
    }
    break;
    default:
        Assert(false);
    }

    return TypeScriptConstructorOverloads::Make(move(overloads));
}

auto_ptr<TypeScriptMethodSignature> ProjectionToTypeScriptConverter::GetEventListenerMethodSignatureOrNull(MetadataStringId methodName, RtEVENT event)
{
    auto listenerType = event->addOn->GetParameters()->allParameters->WhereSingle([](RtPARAMETER param) {
        return param->isIn;
    })->type;

    if (m_emitAnyForUnresolvedTypes || IsTypeVisible(listenerType))
    {
        auto_ptr_vector<TypeScriptParameter> parameterList;
        parameterList.push_back(TypeScriptParameter::Make(L"type", event->nameId));
        parameterList.push_back(TypeScriptParameter::Make(L"listener", GetTypeScriptType(listenerType)));

        return TypeScriptMethodSignature::Make(methodName, TypeScriptParameterList::Make(move(parameterList)), TypeScriptType::Make(L"void"), false /* optional */);
    }

    return auto_ptr<TypeScriptMethodSignature>();
}

auto_ptr<TypeScriptMethodSignature> ProjectionToTypeScriptConverter::GetGeneralEventListenerMethodSignature(const MetadataStringId methodName)
{
    // Event listener can be any function type, so the non-specialized add/remove event listener signature 
    // should accept any function type
    auto listenerType = TypeScriptType::MakeFunctionType(TypeScriptParameterList::Make(L"args"), TypeScriptType::Make(L"any"));

    auto_ptr_vector<TypeScriptParameter> parameterList;
    parameterList.push_back(TypeScriptParameter::Make(L"type", TypeScriptType::Make(L"string")));
    parameterList.push_back(TypeScriptParameter::Make(L"listener", move(listenerType)));

    return TypeScriptMethodSignature::Make(methodName, TypeScriptParameterList::Make(move(parameterList)), TypeScriptType::Make(L"void"), false /* optional */);
}

RtTYPE ProjectionToTypeScriptConverter::GetPromiseResultType(RtRUNTIMEINTERFACECONSTRUCTOR interfaceConstructor)
{
    wstring interfaceName = m_converter.StringOfId(interfaceConstructor->typeId);

    static const wstring iAsyncAction = L"Windows.Foundation.IAsyncAction";
    static const wstring iAsyncActionWithProgressPrefix = L"Windows.Foundation.IAsyncActionWithProgress`1";
    static const wstring iAsyncOperationPrefix = L"Windows.Foundation.IAsyncOperation`1<";
    static const wstring iAsyncOperationWithProgressPrefix = L"Windows.Foundation.IAsyncOperationWithProgress`2<";

    if (interfaceName.compare(0, iAsyncAction.size(), iAsyncAction) == 0 ||
        interfaceName.compare(0, iAsyncActionWithProgressPrefix.size(), iAsyncActionWithProgressPrefix) == 0)
    {
        return RtAnew(m_alloc, VoidType, m_voidStringId);
    }
    else if (interfaceName.compare(0, iAsyncOperationPrefix.size(), iAsyncOperationPrefix) == 0 ||
        interfaceName.compare(0, iAsyncOperationWithProgressPrefix.size(), iAsyncOperationWithProgressPrefix) == 0)
    {
        return interfaceConstructor->genericParameters->First();
    }

    return nullptr;
}

LPCWSTR ProjectionToTypeScriptConverter::GetLastPartIfFullyQualified(LPCWSTR propertyName)
{
    auto lastDot = wcsrchr(propertyName, L'.');
    if (lastDot != nullptr)
    {
        return lastDot + 1;
    }
    else
    {
        return propertyName;
    }
}
