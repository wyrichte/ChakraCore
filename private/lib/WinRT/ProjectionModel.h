//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2010 by Microsoft Corporation.  All rights reserved.
//
// This is the projection of .NET metadata into JavaScript shape. For
// example,
//      - namespaces, classes and enums are var values
//      - overloads are mashed into a single function
//
// The object model is meant to be immutable and data-only because there are
// multiple unrelated behaviors possible. For example, dumping JavaScript text vs.
// code generating interop calls.
//----------------------------------------------------------------------------
#pragma once

// Define default GC Pressure
const INT32 DefaultGCPressure = -1;

// GUID size (in chars including \0 - ie. "{12345678-1234-1234-1234-123456789abc}\0")
const int MaxGuidLength = 39;

#include <WinMDDefsp.h>                 // For the shared [retval] parameter name

namespace ProjectionModel
{
    struct Type;
    struct ConcreteType;
    struct Expr;
    struct Function;
    struct Property;
    struct Parameter;
    struct PropertiesObject;
    struct Assignment;
    struct AssignmentSpace;
    struct OverloadGroup;
    struct MethodSignature;
    struct Event;
    struct InterfaceConstructor;
    struct AbiMethodSignature;
    struct AbiParameter;
    struct RuntimeInterfaceConstructor;
    struct RuntimeClassConstructor;
    struct StructConstructor;
    struct DelegateConstructor;
    struct TypeDefinitionType;
    struct InstantiatedIID;
    struct Specialization;
    struct OverloadedMethodSignature;
    struct Int32Literal;
    struct UInt32Literal;
    struct Enum;
    struct VectorSpecialization;
    struct VectorViewSpecialization;
    struct PromiseSpecialization;
    struct PropertyValueSpecialization;
    struct MapSpecialization;
    struct MapViewSpecialization;
    struct TypeConstructorMethodSignature;
    struct OverloadGroupConstructor;
    struct AbiMethod;
    struct SyntheticParameter;
    struct TypeConstructor;
    struct Parameters;
    struct ProjectionBuilder;
}

typedef const ProjectionModel::Type * RtTYPE;
typedef const ProjectionModel::ConcreteType * RtCONCRETETYPE;
typedef const ProjectionModel::Expr * RtEXPR;
typedef const ProjectionModel::Function * RtFUNCTION;
typedef const ProjectionModel::Property * RtPROPERTY;
typedef const ProjectionModel::Parameter * RtPARAMETER;
typedef const ProjectionModel::PropertiesObject * RtPROPERTIESOBJECT;
typedef const ProjectionModel::Assignment * RtASSIGNMENT;
typedef const ProjectionModel::AssignmentSpace * RtASSIGNMENTSPACE;
typedef const ProjectionModel::OverloadGroup * RtOVERLOADGROUP;
typedef const ProjectionModel::MethodSignature * RtMETHODSIGNATURE;
typedef const ProjectionModel::Event * RtEVENT;
typedef const ProjectionModel::InterfaceConstructor * RtINTERFACECONSTRUCTOR;
typedef const ProjectionModel::AbiMethodSignature * RtABIMETHODSIGNATURE;
typedef const ProjectionModel::AbiParameter * RtABIPARAMETER;
typedef const ProjectionModel::RuntimeInterfaceConstructor * RtRUNTIMEINTERFACECONSTRUCTOR;
typedef const ProjectionModel::RuntimeClassConstructor * RtRUNTIMECLASSCONSTRUCTOR;
typedef const ProjectionModel::StructConstructor * RtSTRUCTCONSTRUCTOR;
typedef const ProjectionModel::DelegateConstructor * RtDELEGATECONSTRUCTOR;
typedef const ProjectionModel::TypeDefinitionType * RtTYPEDEFINITIONTYPE;
typedef const ProjectionModel::InstantiatedIID * RtIID;
typedef const ProjectionModel::Specialization * RtSPECIALIZATION;
typedef const ProjectionModel::OverloadedMethodSignature * RtOVERLOADEDMETHODSIGNATURE;
typedef const ProjectionModel::Int32Literal * RtINT32LITERAL;
typedef const ProjectionModel::UInt32Literal * RtUINT32LITERAL;
typedef const ProjectionModel::Enum * RtENUM;
typedef const ProjectionModel::VectorSpecialization * RtVECTORSPECIALIZATION;
typedef const ProjectionModel::VectorViewSpecialization * RtVECTORVIEWSPECIALIZATION;
typedef const ProjectionModel::PromiseSpecialization * RtPROMISESPECIALIZATION;
typedef const ProjectionModel::PropertyValueSpecialization * RtPROPERTYVALUESPECIALIZATION;
typedef const ProjectionModel::MapSpecialization * RtMAPSPECIALIZATION;
typedef const ProjectionModel::MapViewSpecialization * RtMAPVIEWSPECIALIZATION;
typedef const ProjectionModel::TypeConstructorMethodSignature * RtTYPECONSTRUCTORMETHODSIGNATURE;
typedef const ProjectionModel::OverloadGroupConstructor * RtOVERLOADGROUPCONSTRUCTOR;
typedef const ProjectionModel::AbiMethod * RtABIMETHOD;
typedef const ProjectionModel::SyntheticParameter * RtSYNTHETICPARAMETER;
typedef const ProjectionModel::TypeConstructor * RtTYPECONSTRUCTOR;
typedef const ProjectionModel::Parameters * RtPARAMETERS;


#define PROJECTIONMODEL_SPECIALIZATION_METHODS(TypeName) \
    static bool Is(RtSPECIALIZATION specialization) {return specialization->specializationType == spec##TypeName;}\
    static const TypeName * From(RtSPECIALIZATION specialization) { Js::VerifyCatastrophic(Is(specialization)); return static_cast<const TypeName*>(specialization); } \

#define PROJECTIONMODEL_TYPE_METHOD_FROM(TypeName) \
    static const TypeName * From(RtTYPE type) { Js::VerifyCatastrophic(Is(type)); return static_cast<const TypeName*>(type); } \

#define PROJECTIONMODEL_EXPR_METHODS(TypeName) \
    static bool Is(RtEXPR expr) {return expr->type == expr##TypeName;}\
    static const TypeName * From(RtEXPR expr) { Js::VerifyCatastrophic(Is(expr)); return static_cast<const TypeName*>(expr); } \

#define PROJECTIONMODEL_FUNCTION_METHOD_FROM(TypeName) \
    static const TypeName * From(RtEXPR expr) {Js::VerifyCatastrophic(Is(expr)); return static_cast<const TypeName*>(expr); } \

#define PROJECTIONMODEL_FUNCTION_METHODS(TypeName) \
    static bool Is(RtEXPR expr) {if (Function::Is(expr)) {return Function::From(expr)->functionType==function##TypeName;} else {return false;}} \
    PROJECTIONMODEL_FUNCTION_METHOD_FROM(TypeName) \

#define PROJECTIONMODEL_INTERFACECONSTRUCTOR_METHODS(TypeName) \
    static bool Is(RtEXPR expr) {if (InterfaceConstructor::Is(expr)) { auto ic = InterfaceConstructor::From(expr); return ic->interfaceType == if##TypeName; } return false; } \
    static const TypeName * From(RtEXPR expr) {Js::VerifyCatastrophic(Is(expr)); return static_cast<const TypeName*>(expr); } \

#define PROJECTIONMODEL_METHODSIGNATURE_METHODS(TypeName) \
    static bool Is(RtMETHODSIGNATURE signature) { return signature->signatureType==mst##TypeName; } \
    static const TypeName * From(RtMETHODSIGNATURE signature) { Js::VerifyCatastrophic(Is(signature)); return static_cast<const TypeName*>(signature); } \


#define PROJECTIONMODEL_PARAMETER_METHODS(TypeName) \
    static bool Is(RtPARAMETER parameter) {return parameter->parameterType == pt##TypeName;} \
    static const TypeName * From(RtPARAMETER parameter) {Js::VerifyCatastrophic(Is(parameter)); return static_cast<const TypeName*>(parameter); } \

#define PROJECTIONMODEL_RTCLASS_BEGIN(className) \
    class className \
    {

#define PROJECTIONMODEL_RTCLASS_END(className,typeDefName,baseTypeDef,enumPrefix,typeField) \
    public: \
        typedef const className::Impl * TConst; \
        typedef className::Impl TForConstructionOnly; \
        static TConst From(baseTypeDef base) \
        { \
            Js::VerifyCatastrophic(Is(base)); \
            return TConst(base); \
        } \
        static bool Is(baseTypeDef base) \
        { \
            return base->typeField == enumPrefix##className; \
        } \
        static TForConstructionOnly * UnsafeRemoveConstness(TConst item) \
        { \
            return const_cast<TForConstructionOnly*>(item); \
        } \
    }; \
    typedef className::TConst typeDefName;

#define PROJECTIONMODEL_TYPE_BEGIN(className) PROJECTIONMODEL_RTCLASS_BEGIN(className)
#define PROJECTIONMODEL_TYPE_END(className, typeDefName) PROJECTIONMODEL_RTCLASS_END(className, typeDefName, RtTYPE, tc, typeCode)
#define PROJECTIONMODEL_PROPERTY_BEGIN(className) PROJECTIONMODEL_RTCLASS_BEGIN(className)
#define PROJECTIONMODEL_PROPERTY_END(className, typeDefName) PROJECTIONMODEL_RTCLASS_END(className, typeDefName, RtPROPERTY, pt, propertyType)

namespace ProjectionModel
{
    using namespace regex;

    // An IID that has gone through PIID to IID process
    struct InstantiatedIID
    {
        IID piid;
        IID instantiated;
        InstantiatedIID(IID piid, IID instantiated)
            : piid(piid), instantiated(instantiated)
        {}
        InstantiatedIID()
            : piid(IID_NULL), instantiated(IID_NULL)
        {}
    };

    // piids of well-known types
    const GUID IID_IVector1 = { 0x913337E9, 0x11A1, 0x4345, { 0xA3, 0xA2, 0x4E, 0x7F, 0x95, 0x6E, 0x22, 0x2D } };
    const GUID IID_IIterable1 = { 0xFAA585EA, 0x6214, 0x4217, { 0xAF, 0xDA, 0x7F, 0x46, 0xDE, 0x58, 0x69, 0xB3 } };
    const GUID IID_IIterator1 = { 0x6A79E863, 0x4300, 0x459A, { 0x99, 0x66, 0xCB, 0xB6, 0x60, 0x96, 0x3E, 0xE1 } };
    const GUID IID_IVectorView1 = { 0xBBE1FA4C, 0xB0E3, 0x4583, { 0xBA, 0xEF, 0x1F, 0x1B, 0x2E, 0x48, 0x3E, 0x56 } };
    const GUID IID_IReference1 = { 0x61C17706, 0x2D65, 0x11E0, { 0x9A, 0xE8, 0xD4, 0x85, 0x64, 0x01, 0x54, 0x72 } };
    const GUID IID_IReferenceArray1 = { 0x61C17707, 0x2D65, 0x11E0, { 0x9A, 0xE8, 0xD4, 0x85, 0x64, 0x01, 0x54, 0x72 } };
    const GUID IID_IMap2 = {0x3C2925FE, 0x8519, 0x45C1, { 0xAA, 0x79, 0x19, 0x7B, 0x67, 0x18, 0xC1, 0xC1 } };
    const GUID IID_IMapView2 = { 0xE480CE40, 0xA338, 0x4ADA, { 0xAD, 0xCF, 0x27, 0x22, 0x72, 0xE4, 0x8C, 0xB9 } };
    const GUID IID_Iterable1 = { 0xFAA585EA, 0x6214, 0x4217, {0xAF, 0xDA, 0x7F, 0x46, 0xDE, 0x58, 0x69, 0xB3 } };

    enum ExprType
    {
        exprNullLiteral,
        exprInt32Literal,
        exprUInt32Literal,
        exprPropertiesObject,
        exprAssignmentSpace,
        exprFunction,
        exprModOptExpr,
        exprByRefExpr,
        exprEnum
    };

    enum MethodKind
    {
        MethodKind_Normal,
        MethodKind_Getter,
        MethodKind_Setter
    };

    enum DeprecateType
    {
        Deprecate_deprecate,
        Deprecate_remove
    };

#if JSGEN
    enum Platform
    {
        Platform_windows,
        Platform_windowsPhone
    };

    struct SupportedOnAttribute
    {
        Platform platform;
        UINT version;
        SupportedOnAttribute(SupportedOnAttribute* src)
        {
            this->platform = src->platform;
            this->version = src->version;
        }
        SupportedOnAttribute()
        {
            platform = Platform::Platform_windows;
            version = 0;
        }
    };
#endif

    struct DeprecatedAttribute
    {
        LPWSTR infoString;
        MetadataStringId classId;
        MetadataStringId rtcNameId;
        DeprecateType deprecateType;
        UINT version;
        DeprecatedAttribute(DeprecatedAttribute* src)
        {
            this->infoString = src->infoString;
            this->deprecateType = src->deprecateType;
            this->version = src->version;
            this->rtcNameId = src->rtcNameId;
            Assert(src->infoString != nullptr);
        }
        DeprecatedAttribute()
        {
            infoString = nullptr;
            deprecateType = DeprecateType::Deprecate_deprecate;
            this->rtcNameId = MetadataStringIdNil;
            version = 0;
        }
    };

    struct Expr
    {
        ExprType type;
        Expr(ExprType type) : type(type) {}
    };

    struct Int32Literal : Expr
    {
        int value;
        Int32Literal(int value)
            : Expr(exprInt32Literal), value(value)
        {  }

        PROJECTIONMODEL_EXPR_METHODS(Int32Literal)
    };

    struct UInt32Literal : Expr
    {
        unsigned int value;
        UInt32Literal(unsigned int value)
            : Expr(exprUInt32Literal), value(value)
        {  }
        PROJECTIONMODEL_EXPR_METHODS(UInt32Literal)
    };

    struct ModOptExpr : Expr
    {
        RtEXPR modded;
        const Metadata::ModOpt * nativeModOpt;
        ModOptExpr(RtEXPR modded, const Metadata::ModOpt * nativeModOpt)
            : Expr(exprModOptExpr), modded(modded), nativeModOpt(nativeModOpt)
        { }
        PROJECTIONMODEL_EXPR_METHODS(ModOptExpr)
    };

    struct ByRefExpr : Expr
    {
        RtEXPR pointedTo;
        const Metadata::ByRef * nativeByRef;
        ByRefExpr(RtEXPR pointedTo, const Metadata::ByRef * nativeByRef)
            : Expr(exprByRefExpr), pointedTo(pointedTo), nativeByRef(nativeByRef)
        { }
        PROJECTIONMODEL_EXPR_METHODS(ByRefExpr)
    };

    struct Enum : Expr
    {
        MetadataStringId typeId;
        const Metadata::TypeDefProperties * typeDef;
        RtPROPERTIESOBJECT properties;
        CorElementType baseTypeCode;  // Should be ELEMENT_TYPE_I4 or ELEMENT_TYPE_U4
        Enum(MetadataStringId typeId, const Metadata::TypeDefProperties * typeDef, RtPROPERTIESOBJECT properties, CorElementType baseTypeCode)
            : Expr(exprEnum), typeId(typeId), typeDef(typeDef), properties(properties), baseTypeCode(baseTypeCode)	
        {
            Js::VerifyCatastrophic(typeDef);
            Js::VerifyCatastrophic(properties);
            Js::VerifyCatastrophic(baseTypeCode==ELEMENT_TYPE_I4 || baseTypeCode==ELEMENT_TYPE_U4);
        }
        PROJECTIONMODEL_EXPR_METHODS(Enum)
    };

    // var identifier = expr
    struct Assignment
    {
        LPCWSTR identifier;
        RtEXPR expr;
        Assignment(LPCWSTR identifier, RtEXPR expr) : identifier(identifier), expr(expr)
        {
            Js::VerifyCatastrophic(identifier);
            Js::VerifyCatastrophic(expr);
        }
    };

    // Represents a series of Assignment expressions. This is a useful simplification of general function body.
    struct AssignmentSpace : Expr
    {
        ImmutableList<RtASSIGNMENT> * vars;

        AssignmentSpace(ImmutableList<RtASSIGNMENT> * vars)
            : vars(vars), Expr(exprAssignmentSpace)
        {  }
        Option<Assignment> GetAssignmentByIdentifier(LPCWSTR identifier, ArenaAllocator * a) const;

        PROJECTIONMODEL_EXPR_METHODS(AssignmentSpace)
    };


    enum TypeCode // The JavaScript type
    {
        tcArrayType,
        tcMissingGenericInstantiationType,
        tcGenericClassVarType,
        tcSystemGuidType,
        tcWindowsFoundationDateTimeType,
        tcWindowsFoundationTimeSpanType,
        tcWindowsFoundationEventRegistrationTokenType,
        tcWindowsFoundationHResultType,
        tcInterfaceType,
        tcClassType,
        tcStructType,
        tcDelegateType,
        tcEnumType,
        tcByRefType,
        tcMissingNamedType,
        tcBasicType, // Int, string, etc.
        tcUnprojectableType,
        tcVoidType,
        tcMultiOutType,
        tcGenericParameterType, // Parameter of typedef
        tcHResult,
    };

    enum CanMarshalType
    {
        cmtDontKnowYet,
        cmtMissingType,
        cmtYes,
        cmtNo
    };

    struct Type
    {
        TypeCode typeCode;
        MetadataStringId fullTypeNameId;
        CanMarshalType canMarshal;

        static bool IsMissing(RtTYPE type)
        {
            switch(type->typeCode)
            {
                case tcMissingGenericInstantiationType:
                case tcMissingNamedType:
                    return true;
                default:
                    return false;
           }
        }

        bool CanMarshal(ProjectionBuilder * builder, bool allowMissingTypes = false, bool *outWasMissingType = nullptr) const;

    protected:
        Type(TypeCode typeCode, MetadataStringId fullTypeNameId)
            : typeCode(typeCode), fullTypeNameId(fullTypeNameId), canMarshal(cmtDontKnowYet)
        { }
    };

    // This is a real type that can be passed into and out of ABI methods
    struct ConcreteType : Type
    {
        size_t sizeOnStack;
        size_t storageSize;
        size_t naturalAlignment;

        static bool Is(RtTYPE type)
        {
            switch(type->typeCode)
            {
                case tcArrayType:
                case tcSystemGuidType:
                case tcWindowsFoundationDateTimeType:
                case tcWindowsFoundationTimeSpanType:
                case tcWindowsFoundationEventRegistrationTokenType:
                case tcWindowsFoundationHResultType:
                case tcInterfaceType:
                case tcClassType:
                case tcStructType:
                case tcBasicType:
                case tcEnumType:
                case tcDelegateType:
                case tcMissingGenericInstantiationType:
                case tcMissingNamedType:
                case tcByRefType:
                    return true;
                case tcUnprojectableType:
                case tcGenericClassVarType:
                case tcVoidType:
                case tcGenericParameterType:
                    return false;
                default:
                    Js::VerifyCatastrophic(0);
                    throw 0;
           }
        }

        static const ConcreteType * From(RtTYPE type)
        {
            Js::VerifyCatastrophic(Is(type));
            return static_cast<const ConcreteType*>(type);
        }

        static bool IsBlittable(RtTYPE type);

    protected:
        ConcreteType(TypeCode typeCode, MetadataStringId fullTypeNameId, size_t sizeOnStack, size_t storageSize, size_t naturalAlignment)
            : Type(typeCode, fullTypeNameId), sizeOnStack(sizeOnStack), storageSize(storageSize), naturalAlignment(naturalAlignment)
        {
            Js::VerifyCatastrophic(ConcreteType::Is(this));
        }
    };

    PROJECTIONMODEL_TYPE_BEGIN(MissingNamedType)
        struct Impl : ConcreteType
        {
            MetadataStringId typeId;
            bool isStruct;
            bool isWebHidden;
            Impl(MetadataStringId typeId, MetadataStringId fullTypeNameId, bool isStruct = false, bool isWebHidden = false)
                : ConcreteType(tcMissingNamedType, fullTypeNameId, isStruct ? 0 : sizeof(void *), 0, 0), typeId(typeId), isStruct(isStruct), isWebHidden(isWebHidden)
            { }
        };
    PROJECTIONMODEL_TYPE_END(MissingNamedType, RtMISSINGNAMEDTYPE)

    PROJECTIONMODEL_TYPE_BEGIN(BasicType)
        struct Impl : ConcreteType
        {
            CorElementType typeCor;

            Impl(MetadataStringId fullTypeNameId, CorElementType typeCor,size_t sizeOnStack, size_t storageSize, size_t naturalAlignment)
                : ConcreteType(tcBasicType, fullTypeNameId,sizeOnStack,storageSize,naturalAlignment), typeCor(typeCor)
            {
                Js::VerifyCatastrophic(Metadata::Type::IsBasicTypeCode(typeCor));
            }
        };
    PROJECTIONMODEL_TYPE_END(BasicType, RtBASICTYPE)

    PROJECTIONMODEL_TYPE_BEGIN(VoidType)
        struct Impl : Type
        {
            Impl(MetadataStringId voidTypeNameId)
                : Type(tcVoidType, voidTypeNameId)
            { }
        };
    PROJECTIONMODEL_TYPE_END(VoidType, RtVOIDTYPE)

    PROJECTIONMODEL_TYPE_BEGIN(MultiOutType)
        struct Impl : Type
        {
            Impl()
                : Type(tcMultiOutType, MetadataStringIdNil)
            { }
        };
    PROJECTIONMODEL_TYPE_END(MultiOutType, RtMULTIOUTTYPE)

    PROJECTIONMODEL_TYPE_BEGIN(UnprojectableType)
        struct Impl : Type
        {
            Impl(MetadataStringId unprojectableTypeNameId)
                : Type(tcUnprojectableType, unprojectableTypeNameId)
            {  }
        };
    PROJECTIONMODEL_TYPE_END(UnprojectableType, RtUNPROJECTABLETYPE)

    PROJECTIONMODEL_TYPE_BEGIN(ArrayType)
        struct Impl : ConcreteType
        {
            RtTYPE elementType;
            Impl(MetadataStringId arrayTypeNameId, RtTYPE elementType)
                : ConcreteType(tcArrayType, arrayTypeNameId,
                sizeof(LPVOID) + sizeof(LPVOID), // Size of element count + size of array pointer
                sizeof(LPVOID) + sizeof(LPVOID), // Size of element count + size of array pointer
                __alignof(LPVOID)
                ), elementType(elementType)
            { }
        };
    PROJECTIONMODEL_TYPE_END(ArrayType, RtARRAYTYPE)

    // A type which has a backing type definition
    struct TypeDefinitionType : ConcreteType
    {
        MetadataStringId typeId;
        const Metadata::TypeDefProperties * typeDef;
        ImmutableList<RtTYPE> * genericParameters;

        static bool Is(RtTYPE type)
        {
            return type->typeCode == tcInterfaceType || type->typeCode == tcClassType
                || type->typeCode == tcStructType || type->typeCode == tcEnumType || type->typeCode == tcDelegateType;
        }

        static RtTYPEDEFINITIONTYPE From(RtTYPE type)
        {
            Js::VerifyCatastrophic(Is(type));
            return static_cast<const TypeDefinitionType*>(type);
        }

    protected:
        TypeDefinitionType(TypeCode typeCode, MetadataStringId typeId, const Metadata::TypeDefProperties * typeDef, ImmutableList<RtTYPE> * genericParameters, size_t sizeOnStack, size_t storageSize, size_t naturalAlignment)
            : ConcreteType(typeCode,typeId,sizeOnStack,storageSize, naturalAlignment), typeId(typeId), typeDef(typeDef), genericParameters(genericParameters)
        {
            Js::VerifyCatastrophic(typeDef);
            Js::VerifyCatastrophic(TypeDefinitionType::Is(this));
        }
    };

    PROJECTIONMODEL_TYPE_BEGIN(InterfaceType)
        struct Impl : TypeDefinitionType
        {
            IID iid;
            Impl(MetadataStringId typeId, const Metadata::TypeDefProperties * typeDef, ImmutableList<RtTYPE> * genericParameters, IID iid)
                : TypeDefinitionType(tcInterfaceType,typeId,typeDef,genericParameters,sizeof(LPVOID),sizeof(LPVOID),__alignof(LPVOID)), iid(iid)
            {  }
        };
    PROJECTIONMODEL_TYPE_END(InterfaceType, RtINTERFACETYPE)

    PROJECTIONMODEL_TYPE_BEGIN(ClassType)
        struct Impl : TypeDefinitionType
        {
            Impl(MetadataStringId typeId, const Metadata::TypeDefProperties * typeDef)
                : TypeDefinitionType(tcClassType,typeId,typeDef,nullptr,sizeof(LPVOID),sizeof(LPVOID),__alignof(LPVOID))
            {  }
        };
    PROJECTIONMODEL_TYPE_END(ClassType, RtCLASSTYPE)

    PROJECTIONMODEL_TYPE_BEGIN(DelegateType)
        struct Impl : TypeDefinitionType
        {
            Impl(MetadataStringId typeId, const Metadata::TypeDefProperties * typeDef, ImmutableList<RtTYPE> * genericParameters)
                : TypeDefinitionType(tcDelegateType, typeId, typeDef, genericParameters, sizeof(LPVOID), sizeof(LPVOID), __alignof(LPVOID))
            {  }
        };
    PROJECTIONMODEL_TYPE_END(DelegateType, RtDELEGATETYPE)

#if defined(_M_ARM32_OR_ARM64)
    // Type of the struct, used to take care of "homogenous floating point structs" for ARM.
    // - HFP structs are special case in ARM calling convention -- these are passed in VFP registers.
    // - HFP structs:
    //   - contain 1-4 either float or double fields, no other fields.
    //   - or contain inner HFS structs of the same type (float/double) AND so that total field count in all outer+inner structs is 1-4.
    //   - or contain empty structs.
    enum StructFieldType
    {
        structFieldTypeNonHFP,
        structFieldTypeHFPFloat,
        structFieldTypeHFPDouble,
        structFieldTypeEmpty
    };
#endif



    PROJECTIONMODEL_TYPE_BEGIN(EnumType)
        struct Impl : TypeDefinitionType
        {
            Impl(MetadataStringId typeId, const Metadata::TypeDefProperties * typeDef)
                : TypeDefinitionType(tcEnumType,typeId, typeDef,nullptr, ::Math::Align<uint>(sizeof(int), sizeof(LPVOID)),sizeof(int),__alignof(int))
            {  }
        };
    PROJECTIONMODEL_TYPE_END(EnumType, RtENUMTYPE)


    PROJECTIONMODEL_TYPE_BEGIN(ByRefType)
        struct Impl : ConcreteType
        {
            RtTYPE pointedTo;
            Impl(MetadataStringId byRefTypeNameId, RtTYPE pointedTo)
                : ConcreteType(tcByRefType, byRefTypeNameId,sizeof(LPVOID),sizeof(LPVOID),__alignof(LPVOID)), pointedTo(pointedTo)
            {
                Js::VerifyCatastrophic(pointedTo);
            }
        };
    PROJECTIONMODEL_TYPE_END(ByRefType, RtBYREFTYPE)

    PROJECTIONMODEL_TYPE_BEGIN(GenericParameterType)
        struct Impl : Type
        {
            const Metadata::GenericParameterProperties* properties;
            Impl(const Metadata::GenericParameterProperties* properties)
                : Type(tcGenericParameterType, properties->id), properties(properties)
            {
                Js::VerifyCatastrophic(properties);
            }
        };
    PROJECTIONMODEL_TYPE_END(GenericParameterType, RtGENERICPARAMETERTYPE)

    PROJECTIONMODEL_TYPE_BEGIN(MissingGenericInstantiationType)
        struct Impl : ConcreteType
        {
            MetadataStringId instantiatedNameId;
            RtMISSINGNAMEDTYPE parent;
            ImmutableList<RtTYPE> * genericParameters; // Possibly with free variables

            Impl(RtMISSINGNAMEDTYPE parent, ImmutableList<RtTYPE> * genericParameters, MetadataStringId instantiatedNameId)
                : ConcreteType(tcMissingGenericInstantiationType, instantiatedNameId,0,0,0),
                  parent(parent), genericParameters(genericParameters), instantiatedNameId(instantiatedNameId)
            {
                Js::VerifyCatastrophic(instantiatedNameId != MetadataStringIdNil);
            }

        };
    PROJECTIONMODEL_TYPE_END(MissingGenericInstantiationType, RtMISSINGGENERICINSTANTIATIONTYPE)

    PROJECTIONMODEL_TYPE_BEGIN(GenericClassVarType)
        struct Impl : Type
        {
            const Metadata::TVar * var;
            Impl(MetadataStringId varTypeNameId, const Metadata::TVar * var)
                : Type(tcGenericClassVarType, varTypeNameId),var(var)
            {
                Js::VerifyCatastrophic(var);
                Js::VerifyCatastrophic(var->GetCorElementType()==ELEMENT_TYPE_VAR);
            }

        };
    PROJECTIONMODEL_TYPE_END(GenericClassVarType, RtGENERICCLASSVARTYPE)

    PROJECTIONMODEL_TYPE_BEGIN(SystemGuidType)
        struct Impl : ConcreteType
        {
            Impl(MetadataStringId guidTypeNameId)
                : ConcreteType(tcSystemGuidType, guidTypeNameId,
#if defined(_M_IX86_OR_ARM32)
                sizeof(IID), sizeof(GUID), __alignof(GUID)
    #else
                sizeof(LPVOID), sizeof(IID), __alignof(LPVOID)
    #endif
                )
            { }
        };
    PROJECTIONMODEL_TYPE_END(SystemGuidType, RtSYSTEMGUIDTYPE)

    PROJECTIONMODEL_TYPE_BEGIN(WindowsFoundationDateTimeType)
        struct Impl : ConcreteType
        {
            Impl(MetadataStringId dateTimeTypeNameId)
                : ConcreteType(tcWindowsFoundationDateTimeType, dateTimeTypeNameId,
                sizeof(INT64), sizeof(INT64), __alignof(INT64)
                )
            { }

        };
    PROJECTIONMODEL_TYPE_END(WindowsFoundationDateTimeType, RtWINDOWSFOUNDATIONDATETIMETYPE)

    PROJECTIONMODEL_TYPE_BEGIN(WindowsFoundationTimeSpanType)
        struct Impl : ConcreteType
        {
            Impl(MetadataStringId timeSpanTypeNameId)
                : ConcreteType(tcWindowsFoundationTimeSpanType, timeSpanTypeNameId,
                sizeof(INT64), sizeof(INT64), __alignof(INT64)
                )
            { }
        };
    PROJECTIONMODEL_TYPE_END(WindowsFoundationTimeSpanType, RtWINDOWSFOUNDATIONTIMESPANTYPE)

    PROJECTIONMODEL_TYPE_BEGIN(WindowsFoundationEventRegistrationTokenType)
        struct Impl : ConcreteType
        {
            Impl(MetadataStringId eventRegistrationTokenTypeNameId)
                : ConcreteType(tcWindowsFoundationEventRegistrationTokenType, eventRegistrationTokenTypeNameId,
                sizeof(INT64), sizeof(INT64), __alignof(INT64)
                )
            { }
        };
    PROJECTIONMODEL_TYPE_END(WindowsFoundationEventRegistrationTokenType, RtWINDOWSFOUNDATIONEVENTREGISTRATIONTOKENTYPE)

    PROJECTIONMODEL_TYPE_BEGIN(WindowsFoundationHResultType)
        struct Impl : ConcreteType
        {
            Impl(MetadataStringId hResultTypeNameId)
                : ConcreteType(tcWindowsFoundationHResultType, hResultTypeNameId,
                sizeof(INT32), sizeof(INT32), __alignof(INT64)
                )
            { }
        };
    PROJECTIONMODEL_TYPE_END(WindowsFoundationHResultType, RtWINDOWSFOUNDATIONHRESULTTYPE)

    enum MethodSignatureType
    {
        mstAbiMethodSignature,
        mstTypeConstructorMethodSignature,
        mstMissingTypeConstructorMethodSignature,
        mstOverloadedMethodSignature,
        mstMissingDelegateInvokeMethodSignature,
        mstGenericDelegateInvokeMethodSignature,
        mstUncallableMethodSignature
    };

    struct Parameters
    {
        LPCWSTR callPattern;
        ImmutableList<RtPARAMETER> * allParameters;
        RtTYPE returnType;
        size_t sizeOfCallstack;
        Parameters(ImmutableList<RtPARAMETER> * allParameters, RtTYPE returnType)
            : allParameters(allParameters), returnType(returnType), sizeOfCallstack(0xffffffff), callPattern(nullptr)
        { }
        Parameters(ImmutableList<RtPARAMETER> * allParameters, RtTYPE returnType, size_t sizeOfCallstack, LPCWSTR callPattern)
            : allParameters(allParameters), returnType(returnType), sizeOfCallstack(sizeOfCallstack), callPattern(callPattern)
        { }
    };

    struct MethodSignature
    {
        MethodSignatureType signatureType;
        MetadataStringId nameId;
        // All the method can be marked as deprecated. We don't support
        // deprecated structu/enum etc. at this point.
        // We can have a bigger container for other optional attributes
        // in the future.
        ImmutableList<ProjectionModel::DeprecatedAttribute>* deprecatedAttributes;
        virtual RtPARAMETERS GetParameters() const = 0; // Virtual to support laziness

    protected:
        MethodSignature(MethodSignatureType signatureType, MetadataStringId nameId, ImmutableList<DeprecatedAttribute>* deprecatedAttributes)
            : signatureType(signatureType), nameId(nameId), deprecatedAttributes(deprecatedAttributes)
        {
            Js::VerifyCatastrophic(nameId != MetadataStringIdNil);
        }

    public:
        static bool IsMissingTypeSignature(RtMETHODSIGNATURE methodSignature)
        {
            switch(methodSignature->signatureType)
            {
            case mstMissingTypeConstructorMethodSignature:
            case mstMissingDelegateInvokeMethodSignature:
            case mstUncallableMethodSignature:
                return true;
            default:
                return false;
            }
        }
    };

    struct MethodSignatureWithParameters : MethodSignature
    {
        RtPARAMETERS parameters;

        static bool Is(RtMETHODSIGNATURE signature)
        {
            return signature->signatureType == mstTypeConstructorMethodSignature
                || signature->signatureType == mstMissingTypeConstructorMethodSignature
                || signature->signatureType == mstUncallableMethodSignature
                || signature->signatureType == mstOverloadedMethodSignature
                || signature->signatureType == mstMissingDelegateInvokeMethodSignature
                || signature->signatureType == mstGenericDelegateInvokeMethodSignature;
        }

        static const MethodSignatureWithParameters * From(RtMETHODSIGNATURE signature)
        {
            Js::VerifyCatastrophic(Is(signature));
            return (const MethodSignatureWithParameters*)(signature);
        }

        virtual RtPARAMETERS GetParameters() const
        {
            return parameters;

        }

    protected:
        MethodSignatureWithParameters(MethodSignatureType signatureType, MetadataStringId nameId, RtPARAMETERS parameters,
            ImmutableList<DeprecatedAttribute>* deprecatedAttributes = nullptr)
            : MethodSignature(signatureType, nameId, deprecatedAttributes), parameters(parameters)
        {
            Js::VerifyCatastrophic(parameters);
        }
    };

    struct UncallableMethodSignature : MethodSignatureWithParameters
    {

        UncallableMethodSignature(MetadataStringId uncallableMethodSignatureId, RtPARAMETERS parameters,
        ImmutableList<DeprecatedAttribute>* deprecatedAttributes = nullptr)
        : MethodSignatureWithParameters(mstUncallableMethodSignature, uncallableMethodSignatureId, parameters, deprecatedAttributes)
        {  }
        PROJECTIONMODEL_METHODSIGNATURE_METHODS(UncallableMethodSignature)
    };

    struct OverloadedMethodSignature : MethodSignatureWithParameters
    {
        RtOVERLOADGROUP overloads;
        OverloadedMethodSignature(MetadataStringId nameId, RtOVERLOADGROUP overloads, RtPARAMETERS parameters,
            ImmutableList<DeprecatedAttribute>* deprecatedAttributes = nullptr)
            : MethodSignatureWithParameters(mstOverloadedMethodSignature, nameId, parameters, deprecatedAttributes),
            overloads(overloads)
        {
            Js::VerifyCatastrophic(overloads);
        }
        PROJECTIONMODEL_METHODSIGNATURE_METHODS(OverloadedMethodSignature)
    };

    struct MissingDelegateInvokeMethodSignature : MethodSignatureWithParameters
    {
        LPCWSTR interfaceName; // The interface with an Invoke method
        MissingDelegateInvokeMethodSignature(LPCWSTR interfaceName, MetadataStringId invokeId, RtPARAMETERS parameters,
            ImmutableList<DeprecatedAttribute>* deprecatedAttributes = nullptr)
            : MethodSignatureWithParameters(mstMissingDelegateInvokeMethodSignature, invokeId, parameters, deprecatedAttributes)
        {  }
        PROJECTIONMODEL_METHODSIGNATURE_METHODS(MissingDelegateInvokeMethodSignature)
    };

    struct GenericDelegateInvokeMethodSignature : MethodSignatureWithParameters
    {
        RtDELEGATETYPE delegateType;
        GenericDelegateInvokeMethodSignature(MetadataStringId invokeId, RtDELEGATETYPE delegateType, RtPARAMETERS parameters,
            ImmutableList<DeprecatedAttribute>* deprecatedAttributes = nullptr)
            : MethodSignatureWithParameters(mstGenericDelegateInvokeMethodSignature, invokeId, parameters, deprecatedAttributes),
            delegateType(DelegateType::From(parameters->returnType))
        {  }
        PROJECTIONMODEL_METHODSIGNATURE_METHODS(GenericDelegateInvokeMethodSignature)
    };

    struct MissingTypeConstructorMethodSignature : MethodSignatureWithParameters
    {
        RtMISSINGNAMEDTYPE returnType;
        MissingTypeConstructorMethodSignature(MetadataStringId ctorId, RtPARAMETERS parameters,
            ImmutableList<DeprecatedAttribute>* deprecatedAttributes = nullptr)
            : MethodSignatureWithParameters(mstMissingTypeConstructorMethodSignature, ctorId, parameters, deprecatedAttributes),
            returnType(MissingNamedType::From(parameters->returnType))
        {  }
        PROJECTIONMODEL_METHODSIGNATURE_METHODS(MissingTypeConstructorMethodSignature)
    };

    struct TypeConstructorMethodSignature : MethodSignatureWithParameters
    {
        TypeConstructorMethodSignature(MetadataStringId ctorId, RtPARAMETERS parameters, ImmutableList<DeprecatedAttribute>* deprecatedAttributes = nullptr)
        : MethodSignatureWithParameters(mstTypeConstructorMethodSignature, ctorId, parameters, deprecatedAttributes)
        {  }
        PROJECTIONMODEL_METHODSIGNATURE_METHODS(TypeConstructorMethodSignature)
    };

    struct ReadSignatureContinuation
    {
        ProjectionBuilder * builder;
        const Metadata::MethodProperties * method;
        ImmutableList<RtTYPE> * genericParameters;
        ImmutableList<const Metadata::ParameterProperties*> * parameterProperties;
        const Metadata::Type * returnMetadataType;
        MetadataStringId returnTypeNameId;
        ReadSignatureContinuation(ProjectionBuilder * builder, const Metadata::MethodProperties * method, ImmutableList<RtTYPE> * genericParameters,
            ImmutableList<const Metadata::ParameterProperties*> * parameterProperties, const Metadata::Type * returnMetadataType, MetadataStringId returnTypeNameId)
            : builder(builder), method(method), genericParameters(genericParameters), parameterProperties(parameterProperties),
            returnMetadataType(returnMetadataType), returnTypeNameId(returnTypeNameId)
        { }
    };

    struct AbiMethodSignature sealed : MethodSignature
    {
        LPCWSTR uniqueName; // Will be nullptr if there was no OverloadAttribute on the metadata method
        int vtableIndex; // Relative to end of IInspectable (first abi method is 0)
        RtIID iid;
        bool hasDefaultOverloadAttribute;
        MetadataStringId runtimeClassNameId; // Will be MetadataStringIdNil if this is not a factory method.
        MetadataStringId metadataNameId; // The name of the method in metadata (no camel-casing)
        size_t inParameterCount; // The number of in parameters
        RtPARAMETERS parameters;
        const ReadSignatureContinuation * continuation;
        MethodKind methodKind;

        AbiMethodSignature(MetadataStringId nameId, LPCWSTR uniqueName, RtIID iid, int vtableIndex, size_t inParameterCount,
            bool hasDefaultOverloadAttribute, MetadataStringId runtimeClassNameId, MetadataStringId metadataNameId,
            RtPARAMETERS parameters, const ReadSignatureContinuation * continuation, ImmutableList<DeprecatedAttribute>* deprecatedAttributes,
            MethodKind methodKind)
            : MethodSignature(mstAbiMethodSignature, nameId, deprecatedAttributes),
                uniqueName(uniqueName),
                vtableIndex(vtableIndex),
                iid(iid),
                inParameterCount(inParameterCount),
                hasDefaultOverloadAttribute(hasDefaultOverloadAttribute),
                runtimeClassNameId(runtimeClassNameId),
                metadataNameId(metadataNameId),
                parameters(parameters),
                continuation(continuation),
                methodKind(methodKind)
        {
            Assert(vtableIndex >= 0);
        }

        AbiMethodSignature(MetadataStringId nameId, const AbiMethodSignature* that)
            : MethodSignature(mstAbiMethodSignature, nameId, that->deprecatedAttributes),
                uniqueName(that->uniqueName),
                vtableIndex(that->vtableIndex),
                iid(that->iid),
                inParameterCount(that->inParameterCount),
                hasDefaultOverloadAttribute(that->hasDefaultOverloadAttribute),
                runtimeClassNameId(that->runtimeClassNameId),
                metadataNameId(that->metadataNameId),
                parameters(that->parameters),
                continuation(that->continuation),
                methodKind(that->methodKind)
        {
        }


        virtual RtPARAMETERS GetParameters() const;
        PROJECTIONMODEL_METHODSIGNATURE_METHODS(AbiMethodSignature)
    };

    struct OverloadGroup
    {
        MetadataStringId id;
        ImmutableList<RtABIMETHODSIGNATURE> * overloads; // Should be sorted by descending arity
        OverloadGroup(MetadataStringId id, ImmutableList<RtABIMETHODSIGNATURE> * overloads)
            : id(id), overloads(overloads)
        { }
    };

    enum FunctionType
    {
        functionOverloadGroupConstructor,
        functionAbiMethod,
        functionSystemGuidConstructor,
        functionBasicTypeConstructor,
        functionNativeTypeConstructor,
        functionMissingTypeConstructor,
        functionStructConstructor,
        functionInterfaceConstructor,
        functionGenericClassVarConstructor,
        functionArrayConstructor,
        functionVoidConstructor,
        functionRuntimeClassConstructor,
        functionDelegateConstructor,
        functionWindowsFoundationDateTimeConstructor,
        functionWindowsFoundationTimeSpanConstructor,
        functionWindowsFoundationEventRegistrationTokenConstructor,
        functionWindowsFoundationHResultConstructor,
    };

    struct Function : Expr
    {
        FunctionType functionType;
        RtMETHODSIGNATURE signature;
        RtPROPERTIESOBJECT properties;

        PROJECTIONMODEL_EXPR_METHODS(Function)
    protected:
        Function(FunctionType functionType, RtMETHODSIGNATURE signature, RtPROPERTIESOBJECT properties)
            : Expr(exprFunction), functionType(functionType),signature(signature),properties(properties)
        { }
    };

    struct OverloadGroupConstructor : Function
    {
        RtOVERLOADEDMETHODSIGNATURE signature;
        OverloadGroupConstructor(RtOVERLOADEDMETHODSIGNATURE signature, RtPROPERTIESOBJECT properties)
            : Function(functionOverloadGroupConstructor,signature,properties), signature(signature)
        { }
        PROJECTIONMODEL_FUNCTION_METHODS(OverloadGroupConstructor)
    };

    struct AbiMethod : Function
    {
        RtABIMETHODSIGNATURE signature;
        AbiMethod(RtABIMETHODSIGNATURE signature, RtPROPERTIESOBJECT properties)
            : Function(functionAbiMethod,signature,properties),
            signature(signature)
        {
            Js::VerifyCatastrophic(signature);
            Js::VerifyCatastrophic(properties);
        }
        PROJECTIONMODEL_FUNCTION_METHODS(AbiMethod)
    };

    struct TypeConstructor : Function
    {
        MetadataStringId typeId;
        TypeConstructor(FunctionType functionType, MetadataStringId typeId, RtMETHODSIGNATURE signature, RtPROPERTIESOBJECT properties)
            : Function(functionType, signature, properties), typeId(typeId)
        {
            Js::VerifyCatastrophic(typeId!=MetadataStringIdNil);
        }

        static bool Is(RtEXPR expr)
        {
            if (!Function::Is(expr))
            {
                return false;
            }
            auto function = Function::From(expr);
            return
                   function->functionType==functionMissingTypeConstructor
                || function->functionType==functionStructConstructor
                || function->functionType==functionRuntimeClassConstructor
                || function->functionType==functionDelegateConstructor
                || function->functionType==functionInterfaceConstructor
                || function->functionType==functionSystemGuidConstructor
                || function->functionType==functionWindowsFoundationDateTimeConstructor
                || function->functionType==functionWindowsFoundationTimeSpanConstructor
                || function->functionType==functionWindowsFoundationEventRegistrationTokenConstructor
                || function->functionType==functionWindowsFoundationHResultConstructor;
        }

        static RtTYPECONSTRUCTOR From(RtEXPR expr)
        {
            Js::VerifyCatastrophic(Is(expr));
            return (RtTYPECONSTRUCTOR)expr;
        }
    };

    struct MissingTypeConstructor : TypeConstructor
    {
        bool isWebHidden;
        MissingTypeConstructor(MetadataStringId typeId, RtMETHODSIGNATURE signature, bool isWebHidden = false)
            : TypeConstructor(functionMissingTypeConstructor, typeId, signature, nullptr), isWebHidden(isWebHidden)
        { }
        PROJECTIONMODEL_FUNCTION_METHODS(MissingTypeConstructor)
    };


    struct RuntimeClassConstructor : TypeConstructor
    {
        const Metadata::TypeDefProperties * typeDef;
        RtEXPR prototype;
        Option<InterfaceConstructor> defaultInterface; // May be empty. For example, type is MarshalAs
        RtSPECIALIZATION specialization; // If exists, this is the specialization interface to use (ie IVector<int>)
        bool hasEventHandlers;
        ImmutableList<RtINTERFACECONSTRUCTOR> * allInterfaces;
        ImmutableList<RtINTERFACECONSTRUCTOR> * staticInterfaces;
        INT32 gcPressure;
        RuntimeClassConstructor(MetadataStringId typeId,
            const Metadata::TypeDefProperties * typeDef,
            RtMETHODSIGNATURE signature,
            RtPROPERTIESOBJECT properties,
            RtEXPR prototype,
            Option<InterfaceConstructor> defaultInterface,
            RtSPECIALIZATION specialization,
            bool hasEventHandlers,
            INT32 gcPressure,
            ImmutableList<RtINTERFACECONSTRUCTOR> * allInterfaces,
            ImmutableList<RtINTERFACECONSTRUCTOR> * staticInterfaces)
            : TypeConstructor(functionRuntimeClassConstructor,typeId,signature,properties),
                typeDef(typeDef),
                prototype(prototype),
                defaultInterface(defaultInterface),
                specialization(specialization),
                hasEventHandlers(hasEventHandlers),
                gcPressure(gcPressure),
                allInterfaces(allInterfaces),
                staticInterfaces(staticInterfaces)
        {
            Js::VerifyCatastrophic(this->typeDef);
            Js::VerifyCatastrophic(this->prototype);
        }
        PROJECTIONMODEL_FUNCTION_METHODS(RuntimeClassConstructor)
    };

    struct DelegateConstructor : TypeConstructor
    {
        LPCWSTR simpleName;
        RtINTERFACECONSTRUCTOR invokeInterface; // Treat delegate as interface with single Invoke method.
        DelegateConstructor(MetadataStringId typeId, LPCWSTR simpleName, RtMETHODSIGNATURE signature, RtINTERFACECONSTRUCTOR invokeInterface)
            : TypeConstructor(functionDelegateConstructor, typeId, signature, nullptr), simpleName(simpleName), invokeInterface(invokeInterface)
        {
            Js::VerifyCatastrophic(invokeInterface);
        }

        PROJECTIONMODEL_FUNCTION_METHODS(DelegateConstructor)
    };

    struct SystemGuidConstructor : TypeConstructor
    {
        const TypeConstructorMethodSignature * signature;
        RtSYSTEMGUIDTYPE guidReturnType;
        SystemGuidConstructor(MetadataStringId typeId, const TypeConstructorMethodSignature * signature)
            : TypeConstructor(functionSystemGuidConstructor, typeId, signature, nullptr),
              signature(signature),
              guidReturnType(SystemGuidType::From(signature->parameters->returnType))
        { }
        PROJECTIONMODEL_FUNCTION_METHODS(SystemGuidConstructor)
    };

    struct WindowsFoundationDateTimeConstructor : TypeConstructor
    {
        const TypeConstructorMethodSignature * signature;
        RtWINDOWSFOUNDATIONDATETIMETYPE dateTimeReturnType;
        WindowsFoundationDateTimeConstructor(MetadataStringId typeId, const TypeConstructorMethodSignature * signature)
            : TypeConstructor(functionWindowsFoundationDateTimeConstructor, typeId, signature, nullptr),
              signature(signature),
              dateTimeReturnType(WindowsFoundationDateTimeType::From(signature->parameters->returnType))
        { }
        PROJECTIONMODEL_FUNCTION_METHODS(WindowsFoundationDateTimeConstructor)
    };

    struct WindowsFoundationTimeSpanConstructor : TypeConstructor
    {
        const TypeConstructorMethodSignature * signature;
        RtWINDOWSFOUNDATIONTIMESPANTYPE timeSpanReturnType;
        WindowsFoundationTimeSpanConstructor(MetadataStringId typeId, const TypeConstructorMethodSignature * signature)
            : TypeConstructor(functionWindowsFoundationTimeSpanConstructor, typeId, signature, nullptr),
              signature(signature),
              timeSpanReturnType(WindowsFoundationTimeSpanType::From(signature->parameters->returnType))
        { }
        PROJECTIONMODEL_FUNCTION_METHODS(WindowsFoundationTimeSpanConstructor)
    };

    struct WindowsFoundationEventRegistrationTokenConstructor : TypeConstructor
    {
        const TypeConstructorMethodSignature * signature;
        RtWINDOWSFOUNDATIONEVENTREGISTRATIONTOKENTYPE eventRegistrationTokenType;
        WindowsFoundationEventRegistrationTokenConstructor(MetadataStringId typeId, const TypeConstructorMethodSignature * signature)
            : TypeConstructor(functionWindowsFoundationEventRegistrationTokenConstructor, typeId, signature, nullptr),
              signature(signature),
              eventRegistrationTokenType(WindowsFoundationEventRegistrationTokenType::From(signature->parameters->returnType))
        { }
        PROJECTIONMODEL_FUNCTION_METHODS(WindowsFoundationEventRegistrationTokenConstructor)
    };

    struct WindowsFoundationHResultConstructor : TypeConstructor
    {
        const TypeConstructorMethodSignature * signature;
        RtWINDOWSFOUNDATIONHRESULTTYPE eventRegistrationTokenType;
        WindowsFoundationHResultConstructor(MetadataStringId typeId, const TypeConstructorMethodSignature * signature)
            : TypeConstructor(functionWindowsFoundationHResultConstructor, typeId, signature, nullptr),
              signature(signature),
              eventRegistrationTokenType(WindowsFoundationHResultType::From(signature->parameters->returnType))
        { }
        PROJECTIONMODEL_FUNCTION_METHODS(WindowsFoundationHResultConstructor)
    };

    struct BasicTypeConstructor : Function
    {
        const TypeConstructorMethodSignature * signature;
        RtBASICTYPE basicReturnType;
        BasicTypeConstructor(const TypeConstructorMethodSignature * signature)
            : Function(functionBasicTypeConstructor,signature,nullptr),
            basicReturnType(BasicType::From(signature->parameters->returnType))
        {
            Js::VerifyCatastrophic(Metadata::Type::IsBasicTypeCode(basicReturnType->typeCor));
        }
        PROJECTIONMODEL_FUNCTION_METHODS(BasicTypeConstructor)
    };

    struct NativeTypeConstructor : Function
    {
        const Metadata::Type * nativeType;
        NativeTypeConstructor(const Metadata::Type * nativeType)
            : Function(functionNativeTypeConstructor,nullptr,nullptr), nativeType(nativeType)
        {  }
        PROJECTIONMODEL_FUNCTION_METHODS(NativeTypeConstructor)
    };


    struct GenericClassVarConstructor : Function
    {
        const TypeConstructorMethodSignature * signature;
        RtGENERICCLASSVARTYPE var;
        GenericClassVarConstructor(const TypeConstructorMethodSignature * signature)
            : Function(functionGenericClassVarConstructor,signature,nullptr),
            signature(signature),
            var(GenericClassVarType::From(signature->parameters->returnType))
        { }
        PROJECTIONMODEL_FUNCTION_METHODS(GenericClassVarConstructor)
    };

    struct ArrayConstructor : Function
    {
        RtARRAYTYPE arr;
        ArrayConstructor(const TypeConstructorMethodSignature * signature)
            : Function(functionArrayConstructor,signature,nullptr),
            arr(ArrayType::From(signature->parameters->returnType))
        { }
        PROJECTIONMODEL_FUNCTION_METHODS(ArrayConstructor)
    };

    struct VoidConstructor : Function
    {
        const TypeConstructorMethodSignature * signature;
        RtVOIDTYPE voidReturnType;
        VoidConstructor(const TypeConstructorMethodSignature * signature)
            : Function(functionVoidConstructor,signature,nullptr),
            signature(signature), voidReturnType(VoidType::From(signature->parameters->returnType))
        { }
        PROJECTIONMODEL_FUNCTION_METHODS(VoidConstructor)
    };

    enum InterfaceConstructorType
    {
        ifRuntimeInterfaceConstructor = 1000,
        ifMissingInstantiationConstructor = 1001,
        ifMissingInterfaceConstructor = 1002
    };

    struct InterfaceConstructor : TypeConstructor
    {
        InterfaceConstructorType interfaceType;
        const Metadata::TypeDefProperties * typeDef; // Null if the type is missing
        LPCWSTR typeName;
        RtPROPERTIESOBJECT prototype;
        ImmutableList<RtPROPERTY> * ownProperties; // Properties of just this interface, not children
        ImmutableList<RtINTERFACECONSTRUCTOR> * requiredInterfaces;
        PROJECTIONMODEL_FUNCTION_METHODS(InterfaceConstructor)
    protected:
        InterfaceConstructor(InterfaceConstructorType interfaceType, MetadataStringId typeId, const Metadata::TypeDefProperties * typeDef, RtMETHODSIGNATURE signature, RtPROPERTIESOBJECT properties, RtPROPERTIESOBJECT prototype,
            ImmutableList<RtPROPERTY> * ownProperties, ImmutableList<RtINTERFACECONSTRUCTOR> * requiredInterfaces)
            : TypeConstructor(functionInterfaceConstructor,typeId,signature,properties), typeDef(typeDef), interfaceType(interfaceType),
              prototype(prototype), requiredInterfaces(requiredInterfaces), ownProperties(ownProperties)
        {
            Js::VerifyCatastrophic(prototype);
        }
    };

    struct MissingInterfaceConstructor : InterfaceConstructor
    {
        bool isWebHidden;
        MissingInterfaceConstructor(MetadataStringId typeId, RtMETHODSIGNATURE signature, RtPROPERTIESOBJECT body, bool isWebHidden = false)
            : InterfaceConstructor(ifMissingInterfaceConstructor, typeId, nullptr, signature, nullptr, body, ImmutableList<RtPROPERTY>::Empty(), ImmutableList<RtINTERFACECONSTRUCTOR>::Empty())
            , isWebHidden(isWebHidden)
        {}
        PROJECTIONMODEL_INTERFACECONSTRUCTOR_METHODS(MissingInterfaceConstructor);
    };

    struct RuntimeInterfaceConstructor : InterfaceConstructor
    {
        RtIID iid;
        const TypeConstructorMethodSignature * signature;
        ImmutableList<RtTYPE> * genericParameters;
        LPCWSTR * locatorNames; // Will be nullptr in the case of non-concrete instantiation
        size_t nameCount;
        RtSPECIALIZATION specialization; // If exists, this is the specialization interface to use (ie IVector<int>)
        bool hasEventHandlers;
        RuntimeInterfaceConstructor(MetadataStringId typeId, const Metadata::TypeDefProperties * typeDef, const TypeConstructorMethodSignature * signature, RtIID iid, ImmutableList<RtTYPE> * genericParameters,
            RtPROPERTIESOBJECT properties, RtPROPERTIESOBJECT prototype, ImmutableList<RtPROPERTY> * ownProperties, ImmutableList<RtINTERFACECONSTRUCTOR> * requiredInterfaces,
            LPCWSTR * locatorNames, size_t nameCount, RtSPECIALIZATION specialization, bool hasEventHandlers)
            : InterfaceConstructor(ifRuntimeInterfaceConstructor, typeId, typeDef, signature, properties, prototype, ownProperties, requiredInterfaces),
            signature(signature), iid(iid), genericParameters(genericParameters), locatorNames(locatorNames), nameCount(nameCount),
            specialization(specialization), hasEventHandlers(hasEventHandlers)
        {
            Js::VerifyCatastrophic(mstMissingTypeConstructorMethodSignature != signature->signatureType);
            Js::VerifyCatastrophic(mstUncallableMethodSignature != signature->signatureType);
            Js::VerifyCatastrophic(this->typeDef);
            Js::VerifyCatastrophic(this->signature);
        }
        PROJECTIONMODEL_INTERFACECONSTRUCTOR_METHODS(RuntimeInterfaceConstructor);
    };

    struct MissingInstantiationConstructor : InterfaceConstructor
    {
        RtMISSINGGENERICINSTANTIATIONTYPE instantiatedGenericType;
        const TypeConstructorMethodSignature * signature;

        MissingInstantiationConstructor(MetadataStringId typeId, const TypeConstructorMethodSignature * signature, RtPROPERTIESOBJECT body)
            : InterfaceConstructor(ifMissingInstantiationConstructor, typeId, nullptr, signature, nullptr, body, ImmutableList<RtPROPERTY>::Empty(), nullptr),
            signature(signature),
            instantiatedGenericType(MissingGenericInstantiationType::From(signature->parameters->returnType))
        {
            Js::VerifyCatastrophic(instantiatedGenericType);
        }
        PROJECTIONMODEL_INTERFACECONSTRUCTOR_METHODS(MissingInstantiationConstructor);
    };

    enum ParameterType
    {
        ptAbiParameter,
        ptSyntheticParameter
    };

    struct Parameter
    {
        ParameterType parameterType;
        MetadataStringId id;
        RtTYPE type;
        bool isIn;
        bool isOut; // isIn=true, isOut=true corresponds to FillPassArray pattern

        virtual bool IsArrayParameterWithLengthAttribute() const { return false; }

    protected:
        Parameter(ParameterType parameterType, MetadataStringId id, RtTYPE type, bool isIn, bool isOut)
            : parameterType(parameterType), id(id), type(type), isIn(isIn), isOut(isOut)
        {
            Js::VerifyCatastrophic(isIn || isOut); // Can't be false, false
        }
    };

    struct SyntheticParameter : Parameter
    {
        SyntheticParameter(MetadataStringId id, RtTYPE type)
            : Parameter(ptSyntheticParameter,id,type, true, false)
        { }
        PROJECTIONMODEL_PARAMETER_METHODS(SyntheticParameter)
    };


    struct AbiParameter : Parameter
    {
        size_t inParameterIndex;

        AbiParameter(MetadataStringId id, RtTYPE type, bool isIn, bool isOut, size_t inParameterIndex)
            : Parameter(ptAbiParameter,id,type, isIn, isOut), inParameterIndex(inParameterIndex)
        {  }
        PROJECTIONMODEL_PARAMETER_METHODS(AbiParameter)

        size_t GetParameterOnStackCount() const
        {
            if (ByRefType::Is(type))
            {
                auto byRefType = ByRefType::From(type);
                if (ArrayType::Is(byRefType->pointedTo))
                {
                    // ReceiveArray pattern.
                    return 2;
                }
            }

            if (ArrayType::Is(type))
            {
                // PassArray or FillArray pattern
                return 2;
            }

            return 1;
        }

        size_t GetSizeOnStack() const
        {
            if (!isIn)
            {
                if (ByRefType::Is(type))
                {
                    auto byRefType = ByRefType::From(type);

                    if (ArrayType::Is(byRefType->pointedTo))
                    {
                        // ReceiveArray pattern.
                        return ArrayType::From(byRefType->pointedTo)->sizeOnStack;
                    }
                }

                // Otherwise, out parameters are pointer sized.
                return sizeof(LPVOID);
            }

            auto concrete = ConcreteType::From(type);
            return concrete->sizeOnStack;
        }
    };

    struct AbiArrayParameterWithLengthAttribute : AbiParameter
    {
        UINT32 lengthIsParameter;

        AbiArrayParameterWithLengthAttribute(MetadataStringId id, RtTYPE type, bool isIn, bool isOut, size_t inParameterIndex, INT32 lengthIsParameter)
            : AbiParameter(id, type, isIn, isOut, inParameterIndex), lengthIsParameter(lengthIsParameter)
        {  }

        virtual bool IsArrayParameterWithLengthAttribute() const { return true; }

        RtABIPARAMETER GetLengthParameter(ImmutableList<RtPARAMETER> * allParameters) const
        {
            Assert(allParameters->Count() >= lengthIsParameter);
            Assert(AbiParameter::Is(allParameters->Nth(lengthIsParameter)));
            return AbiParameter::From(allParameters->Nth(lengthIsParameter));
        }
    };

    struct Event
    {
        RtMETHODSIGNATURE addOn;
        RtMETHODSIGNATURE removeOn;
        MetadataStringId metadataNameId; // The name from metadata
        MetadataStringId nameId; // The name that would be projected - fully qualified if there are name conflicts
#if DBG
        LPCWSTR nameStr;
#endif

        Event(RtMETHODSIGNATURE addOn, RtMETHODSIGNATURE removeOn, MetadataStringId metadataNameId, MetadataStringId nameId
#if DBG
            , LPCWSTR nameStr
#endif
            )
            : addOn(addOn), removeOn(removeOn), nameId(nameId), metadataNameId(metadataNameId)
#if DBG
            , nameStr(nameStr)
#endif
        { }
    };

    enum PropertyType
    {
        ptNone,
        ptAbiFieldProperty,
        ptOverloadParentProperty,
        ptAbiMethodProperty,
        ptAbiPropertyProperty,
        ptAbiArrayLengthProperty,
        ptAbiTypeProperty,
        ptAbiNamespaceProperty,
        ptAbiAddEventListenerProperty,
        ptAbiRemoveEventListenerProperty,
        ptAbiEventHandlerProperty,
        ptUnresolvableNameConflictProperty,
        ptFunctionLengthProperty
    };


    // identifier : Expr
    struct Property
    {
        PropertyType propertyType;
        MetadataStringId identifier;
        RtEXPR expr;
    protected:
        Property(PropertyType propertyType, MetadataStringId identifier, RtEXPR expr)
            : identifier(identifier), propertyType(propertyType), expr(expr)
        {
            Assert(propertyType != ptNone);
            Js::VerifyCatastrophic(identifier);
            Js::VerifyCatastrophic(expr);
        }
    };

    // Represents a series of Property definitions.
    struct PropertiesObject : Expr
    {
        ImmutableList<RtPROPERTY> * fields;
        PropertiesObject(ImmutableList<RtPROPERTY> * fields)
         : Expr(exprPropertiesObject), fields(fields)
        {  }
        Option<Property> GetFieldByIdentifier(MetadataStringId identifier, ArenaAllocator * a) const;
        PROJECTIONMODEL_EXPR_METHODS(PropertiesObject)
    };

    PROJECTIONMODEL_PROPERTY_BEGIN(AbiTypeProperty)
        struct Impl : Property
        {
            Impl(MetadataStringId identifier, RtEXPR expr)
                : Property(ptAbiTypeProperty, identifier, expr)
            { }
        };
    PROJECTIONMODEL_PROPERTY_END(AbiTypeProperty, RtABITYPEPROPERTY)

    PROJECTIONMODEL_PROPERTY_BEGIN(AbiNamespaceProperty)
        struct Impl : Property
        {
            RtPROPERTIESOBJECT childNamespace;
            Impl(MetadataStringId identifier, RtPROPERTIESOBJECT childNamespace)
                : Property(ptAbiNamespaceProperty, identifier, childNamespace), childNamespace(childNamespace)
            { }
        };
    PROJECTIONMODEL_PROPERTY_END(AbiNamespaceProperty, RtABINAMESPACEPROPERTY)

    PROJECTIONMODEL_PROPERTY_BEGIN(OverloadParentProperty)
        struct Impl : Property
        {
            RtOVERLOADGROUPCONSTRUCTOR overloadConstructor;
            Impl(MetadataStringId identifier, RtOVERLOADGROUPCONSTRUCTOR overloadConstructor)
                : Property(ptOverloadParentProperty, identifier, overloadConstructor), overloadConstructor(overloadConstructor)
            {
                Js::VerifyCatastrophic(overloadConstructor);
            }
        };
    PROJECTIONMODEL_PROPERTY_END(OverloadParentProperty, RtOVERLOADPARENTPROPERTY)

    PROJECTIONMODEL_PROPERTY_BEGIN(AbiAddEventListenerProperty)
        struct Impl : Property
        {
            ImmutableList<RtEVENT> * events;
            Impl(MetadataStringId identifier, ImmutableList<RtEVENT> * events, RtEXPR body)
                : Property(ptAbiAddEventListenerProperty, identifier, body), events(events)
            {
                Js::VerifyCatastrophic(events->Count()>0);
            }
        };
    PROJECTIONMODEL_PROPERTY_END(AbiAddEventListenerProperty, RtABIADDEVENTLISTENERPROPERTY)

    PROJECTIONMODEL_PROPERTY_BEGIN(AbiRemoveEventListenerProperty)
        struct Impl : Property
        {
            ImmutableList<RtEVENT> * events;
            Impl(MetadataStringId identifier, ImmutableList<RtEVENT> * events, RtEXPR body)
                : Property(ptAbiRemoveEventListenerProperty, identifier, body), events(events)
            {
                Js::VerifyCatastrophic(events->Count()>0);
            }
        };
    PROJECTIONMODEL_PROPERTY_END(AbiRemoveEventListenerProperty, RtABIREMOVEEVENTLISTENERPROPERTY)

    PROJECTIONMODEL_PROPERTY_BEGIN(AbiEventHandlerProperty)
        struct Impl : Property
        {
            RtEVENT abiEvent;
            Impl(MetadataStringId identifier, RtEXPR body, RtEVENT abiEvent)
                : Property(ptAbiEventHandlerProperty, identifier, body), abiEvent(abiEvent)
            {
                Js::VerifyCatastrophic(abiEvent != NULL);
            }
        };
    PROJECTIONMODEL_PROPERTY_END(AbiEventHandlerProperty, RtABIEVENTHANDLERPROPERTY)

    PROJECTIONMODEL_PROPERTY_BEGIN(AbiFieldProperty)
        struct Impl : Property
        {
            const ConcreteType * type;
            const Metadata::FieldProperties * fieldProperties;
            size_t fieldOffset;
            Impl(MetadataStringId identifier, RtEXPR body, const ConcreteType * type, const Metadata::FieldProperties * fieldProperties, size_t fieldOffset)
                : Property(ptAbiFieldProperty, identifier, body), type(type),
                  fieldProperties(fieldProperties), fieldOffset(fieldOffset)
            {
                Js::VerifyCatastrophic(type);
            }
        };
    PROJECTIONMODEL_PROPERTY_END(AbiFieldProperty, RtABIFIELDPROPERTY)

    PROJECTIONMODEL_PROPERTY_BEGIN(AbiMethodProperty)
        struct Impl : Property
        {
            RtABIMETHOD body;
            Impl(MetadataStringId identifier, RtABIMETHOD body)
                : Property(ptAbiMethodProperty, identifier, body), body(body)
            {
                Js::VerifyCatastrophic(body);
            }
        };
    PROJECTIONMODEL_PROPERTY_END(AbiMethodProperty, RtABIMETHODPROPERTY)

    // Property which holds an AbiProperty with getter and setter
    PROJECTIONMODEL_PROPERTY_BEGIN(AbiPropertyProperty)
        struct Impl : Property
        {
            Option<AbiMethodSignature> getter;
            Option<AbiMethodSignature> setter; // Empty if there is no setter.
            MetadataStringId metadataNameId; // Property name from metadata (no camel casing)

            Impl(MetadataStringId identifier, RtEXPR body, Option<AbiMethodSignature> getter, Option<AbiMethodSignature> setter, MetadataStringId metadataNameId)
                : Property(ptAbiPropertyProperty, identifier, body), getter(getter), setter(setter), metadataNameId(metadataNameId)
            {
                Js::VerifyCatastrophic(getter.HasValue() || setter.HasValue()); // Can't both be missing
            }

            // Get the property type. This is a deferred operation.
            RtTYPE GetPropertyType() const
            {
                if (setter.HasValue())
                {
                    auto setterType = setter.GetValue()->GetParameters()->allParameters->First()->type;
                    return setterType;
                }
                auto getterType = getter.GetValue()->GetParameters()->returnType;
                return ByRefType::From(getterType)->pointedTo;
            }
        };
    PROJECTIONMODEL_PROPERTY_END(AbiPropertyProperty, RtABIPROPERTYPROPERTY)

    // Special semantics of array length property
    PROJECTIONMODEL_PROPERTY_BEGIN(AbiArrayLengthProperty)
        struct Impl : Property
        {
            RtTYPE type;
            RtABIMETHODSIGNATURE getSize;

            Impl(MetadataStringId identifier, RtEXPR body, RtTYPE type, RtABIMETHODSIGNATURE getSize)
                : Property(ptAbiArrayLengthProperty, identifier, body), type(type), getSize(getSize)
            {
                Js::VerifyCatastrophic(type);
                Js::VerifyCatastrophic(getSize);
            }
        };
    PROJECTIONMODEL_PROPERTY_END(AbiArrayLengthProperty, RtABIARRAYLENGTHPROPERTY)

    PROJECTIONMODEL_PROPERTY_BEGIN(UnresolvableNameConflictProperty)
        struct Impl : Property
        {
            ImmutableList<RtPROPERTY> * conflictingProperties;

            Impl(MetadataStringId identifier, RtEXPR body, ImmutableList<RtPROPERTY> * conflictingProperties)
                : Property(ptUnresolvableNameConflictProperty, identifier, body), conflictingProperties(conflictingProperties)
            {
                Js::VerifyCatastrophic(conflictingProperties);
            }
        };
    PROJECTIONMODEL_PROPERTY_END(UnresolvableNameConflictProperty, RtUNRESOLVABLENAMECONFLICTPROPERTY)

    PROJECTIONMODEL_PROPERTY_BEGIN(FunctionLengthProperty)
        struct Impl : Property
        {
            const Int32Literal * value;
           Impl(MetadataStringId identifier, const Int32Literal * value)
                : Property(ptFunctionLengthProperty, identifier, value), value(value)
            {
                Js::VerifyCatastrophic(value);
            }
        };
    PROJECTIONMODEL_PROPERTY_END(FunctionLengthProperty, RtFUNCTIONLENGTHPROPERTY)

    PROJECTIONMODEL_TYPE_BEGIN(StructType)
        struct Impl : TypeDefinitionType
        {
            bool isPassByReference;
            bool isBlittable;
    #if defined(_M_ARM32_OR_ARM64)
            StructFieldType structType; // Used for HPF structs: see the enum.
            int hfpFieldCount;          // Used for HPF structs: total number of floating point fields in this HFP struct, including inner HFPs.
    #endif

            ImmutableList<RtABIFIELDPROPERTY> * fields;
            Impl(MetadataStringId typeId, const Metadata::TypeDefProperties * typeDef, ImmutableList<RtABIFIELDPROPERTY> * fields, size_t sizeOnStack, size_t storageSize, size_t naturalAlignment, bool isPassByReference, bool isBlittable
    #if defined(_M_ARM32_OR_ARM64)
                , StructFieldType structType_, int hfpFieldCount_
    #endif
                )
                : TypeDefinitionType(tcStructType, typeId, typeDef, nullptr, sizeOnStack, storageSize, naturalAlignment), fields(fields), isPassByReference(isPassByReference), isBlittable(isBlittable)
    #if defined(_M_ARM32_OR_ARM64)
                , structType(structType_), hfpFieldCount(hfpFieldCount_)
    #endif
            { }
        };
    PROJECTIONMODEL_TYPE_END(StructType, RtSTRUCTTYPE)

    struct StructConstructor : TypeConstructor
    {
        RtSTRUCTTYPE structType;
        const TypeConstructorMethodSignature * signature;

        StructConstructor(MetadataStringId typeId, const TypeConstructorMethodSignature * signature, RtPROPERTIESOBJECT properties, RtSTRUCTTYPE structType)
            : TypeConstructor(functionStructConstructor, typeId, signature, properties), signature(signature), structType(structType)
        {
            Js::VerifyCatastrophic(this->structType);
        }
        PROJECTIONMODEL_FUNCTION_METHODS(StructConstructor)
    };

    enum SpecializationType
    {
        specVectorSpecialization,
        specVectorViewSpecialization,
        specPromiseSpecialization,
        specPropertyValueSpecialization,
        specMapSpecialization,
        specMapViewSpecialization
    };

    struct Specialization
    {
        SpecializationType specializationType;
        RtIID iid;
        Specialization(SpecializationType specializationType, RtIID iid)
            : specializationType(specializationType), iid(iid)
        {
            Js::VerifyCatastrophic(iid);
        }
    };

    struct VectorSpecialization : Specialization
    {
        RtABIARRAYLENGTHPROPERTY length;
        RtABIMETHODSIGNATURE getAt;
        RtABIMETHODSIGNATURE setAt;
        RtABIMETHODSIGNATURE append;
        RtABIMETHODSIGNATURE removeAtEnd;
        VectorSpecialization(RtIID iid, RtABIARRAYLENGTHPROPERTY length, RtABIMETHODSIGNATURE getAt, RtABIMETHODSIGNATURE setAt, RtABIMETHODSIGNATURE append, RtABIMETHODSIGNATURE removeAtEnd)
            : Specialization(specVectorSpecialization,iid), length(length), getAt(getAt), setAt(setAt), append(append), removeAtEnd(removeAtEnd)
        {
            Js::VerifyCatastrophic(getAt);
            Js::VerifyCatastrophic(setAt);
            Js::VerifyCatastrophic(append);
            Js::VerifyCatastrophic(removeAtEnd);
        }
        PROJECTIONMODEL_SPECIALIZATION_METHODS(VectorSpecialization);
    };

    struct VectorViewSpecialization : Specialization
    {
        RtABIARRAYLENGTHPROPERTY length;
        RtABIMETHODSIGNATURE getAt;
        VectorViewSpecialization(RtIID iid, RtABIARRAYLENGTHPROPERTY length, RtABIMETHODSIGNATURE getAt)
            : Specialization(specVectorViewSpecialization,iid), length(length), getAt(getAt)
        {
            Js::VerifyCatastrophic(getAt);
        }
        PROJECTIONMODEL_SPECIALIZATION_METHODS(VectorViewSpecialization);
    };

    struct PromiseSpecialization : Specialization
    {
        PromiseSpecialization(RtIID iid)
            : Specialization(specPromiseSpecialization,iid)
        {
        }
        PROJECTIONMODEL_SPECIALIZATION_METHODS(PromiseSpecialization);
    };

    struct PropertyValueSpecialization : Specialization
    {
        RtABIMETHODSIGNATURE getValue;

        PropertyValueSpecialization(RtIID iid, RtABIMETHODSIGNATURE getValue)
            : Specialization(specPropertyValueSpecialization, iid), getValue(getValue)
        {
        }
        PROJECTIONMODEL_SPECIALIZATION_METHODS(PropertyValueSpecialization);
    };

    struct MapSpecialization : Specialization
    {
        RtABIMETHODSIGNATURE hasKey;
        RtABIMETHODSIGNATURE insert;
        RtABIMETHODSIGNATURE lookup;
        RtABIMETHODSIGNATURE remove;
        RtABIMETHODSIGNATURE first;

        MapSpecialization(RtIID iid, RtABIMETHODSIGNATURE hasKey, RtABIMETHODSIGNATURE insert, RtABIMETHODSIGNATURE lookup, RtABIMETHODSIGNATURE remove, RtABIMETHODSIGNATURE first)
            : Specialization(specMapSpecialization, iid), hasKey(hasKey), insert(insert), lookup(lookup), remove(remove), first(first)
        {
        }
        PROJECTIONMODEL_SPECIALIZATION_METHODS(MapSpecialization);
    };

    struct MapViewSpecialization : Specialization
    {
        RtABIMETHODSIGNATURE hasKey;
        RtABIMETHODSIGNATURE lookup;
        RtABIMETHODSIGNATURE first;

        MapViewSpecialization(RtIID iid, RtABIMETHODSIGNATURE hasKey, RtABIMETHODSIGNATURE lookup, RtABIMETHODSIGNATURE first)
            : Specialization(specMapViewSpecialization, iid), hasKey(hasKey), lookup(lookup), first(first)
        {
        }
        PROJECTIONMODEL_SPECIALIZATION_METHODS(MapViewSpecialization);
    };

    struct MetadataArityGroup
    {
        size_t arity;
        ImmutableList<const Metadata::MethodProperties*> * methods;
        MetadataArityGroup(ImmutableList<const Metadata::MethodProperties*> * methods) : methods(methods)
        {
            arity = methods->First()->signature->parameters->CountWhere([&](const Metadata::Parameter * parameter) {
                return !parameter->byref;
            });
        }
    };

    struct MetadataOverloadGroup
    {
        MetadataStringId id;
        ImmutableList<const MetadataArityGroup*> * arityGroups;
        MetadataOverloadGroup(MetadataStringId id, ImmutableList<const MetadataArityGroup*> * arityGroups)
            : id(id), arityGroups(arityGroups)
        { }
    };

    struct CompareRefConstNames : public regex::Comparer<LPCWSTR *>
    {
        static CompareRefConstNames Instance;

        // Info:        Return true if names are the same
        // Parameters:  v1 - name 1
        //              v2 - name 2
        bool Equals(LPCWSTR *v1, LPCWSTR *v2)
        {
            Assert(v1 != nullptr && v2 != nullptr);
            Assert((*v1) != nullptr);
            Assert((*v2) != nullptr);
            return wcscmp(*v1, *v2)==0;
        }

        // Info:        Not called
        int GetHashCode(LPCWSTR *s)
        {
            Assert(0);
            return 0;
        }

        // Info:        Compare two names
        // Parameters:  v1 - name 1
        //              v2 - name 2
        int Compare(LPCWSTR *v1, LPCWSTR *v2)
        {
            Assert(v1 != nullptr && v2 != nullptr);
            Assert((*v1) != nullptr);
            Assert((*v2) != nullptr);
            return wcscmp(*v1, *v2);
        }
    };

    struct CompareRefProperties : public regex::Comparer<RtPROPERTY *>
    {
    private:
        Metadata::IStringConverter * stringConverter;

    public:
        CompareRefProperties(Metadata::IStringConverter * stringConverter) : stringConverter(stringConverter) { }

        // Info:        Return true if property identifiers are the same
        // Parameters:  v1 - property 1
        //              v2 - property 2
        bool Equals(RtPROPERTY *v1, RtPROPERTY *v2)
        {
            Assert(v1 != nullptr && v2 != nullptr);
            Assert((*v1) != nullptr);
            Assert((*v2) != nullptr);
            AssertMsg((*v1)->propertyType != ptNone, "v1 uninitialized?");
            AssertMsg((*v2)->propertyType != ptNone, "v2 uninitialized?");
            return ((*v1)->identifier == (*v2)->identifier);
        }

        // Info:        Not called
        int GetHashCode(RtPROPERTY *s)
        {
            Js::Throw::FatalProjectionError();
        }

        // Info:        Compare two property identifiers
        // Parameters:  v1 - property 1
        //              v2 - property 2
        int Compare(RtPROPERTY *v1, RtPROPERTY *v2)
        {
            Assert(v1 != nullptr && v2 != nullptr);
            Assert((*v1) != nullptr);
            Assert((*v2) != nullptr);
            AssertMsg((*v1)->propertyType != ptNone, "v1 uninitialized?");
            AssertMsg((*v2)->propertyType != ptNone, "v2 uninitialized?");
            return wcscmp(stringConverter->StringOfId((*v1)->identifier), stringConverter->StringOfId((*v2)->identifier));
        }
    };

    struct ITypeResolver
    {
        virtual HRESULT ResolveTypeName(MetadataStringId typeId, LPCWSTR typeName, Metadata::TypeDefProperties ** typeDef) = 0;
        virtual Js::DelayLoadWinRtRoParameterizedIID *GetRoParameterizedIIDDelayLoad() = 0;
    };

    // Info:        Parse a dotted string. Return empty if there is no dot.
    // Parameters:  name - name to parse
    //              createString - function that creates string
    // Returns:     Empty if there is no dot
    template<class F>
    Option<wchar_t> GetToDot(LPCWSTR name, F createString)
    {
        auto dot = wcschr(name,L'.');
        if (nullptr != dot)
        {
            auto length = (int)(dot-name+1);
            auto shortName = createString(length);
            wmemcpy_s(shortName, length, name, (length-1));
            shortName[length-1] = 0;
            return shortName;
        }
        else
        {
            return nullptr; // Means there was no dot.
        }
    }

    struct DeferredTypeDefinitionCandidate
    {
        MetadataStringId typeId;
        ImmutableList<RtTYPE> *genericParameters;

        DeferredTypeDefinitionCandidate()
            : typeId(0), genericParameters(ImmutableList<RtTYPE>::Empty())
        {
            Assert(false);
        }

        DeferredTypeDefinitionCandidate(MetadataStringId typeId, ImmutableList<RtTYPE> *genericParameters)
        {
            Assert(typeId != MetadataStringIdNil);

            this->typeId = typeId;
            this->genericParameters = (genericParameters != nullptr) ? genericParameters : ImmutableList<RtTYPE>::Empty();
        }

        bool Equals(DeferredTypeDefinitionCandidate& c)
        {
            return this->Equals(c.typeId, c.genericParameters);
        }

        bool Equals(MetadataStringId typeId, ImmutableList<RtTYPE> *genericParameters)
        {
            bool retValue = typeId == this->typeId;

            if (retValue)
            {
                genericParameters = (genericParameters != nullptr) ? genericParameters : ImmutableList<RtTYPE>::Empty();
                size_t count = genericParameters->Count();

                retValue = count == this->genericParameters->Count();
                if (retValue)
                {
                    size_t sameIdsCount = 0;
                    this->genericParameters->IterateWith(genericParameters, [&](RtTYPE a, RtTYPE b)
                    {
                        if (a == b)
                        {
                            sameIdsCount++;
                        }
                    });

                    retValue = sameIdsCount == count;
                }
            }

            return retValue;
        }

        // Returns friendly string representation of the DeferredTypeDefinitionCandidate instance
        // Note: returned string to be free-d using DefaultImmutableStringBuilder::FreeString(allocator, ..., wcslen(...));
        // e.g. IEnumerable<Foo> (#22<#23>)
        LPCWSTR ToString(Metadata::IStringConverter *strConverter, _In_ ArenaAllocator* allocator)
        {
            Js::VerifyCatastrophic(strConverter != nullptr);
            Js::VerifyCatastrophic(allocator != nullptr);

            DefaultImmutableStringBuilder retValue;

            retValue.Append(strConverter->StringOfId(this->typeId));
            if (!this->genericParameters->IsEmpty())
            {
                retValue.Append(L"<");
                bool isFirst = true;
                this->genericParameters->Iterate([&](RtTYPE genericParameter) {
                    if (isFirst)
                    {
                        isFirst = false;
                    }
                    else
                    {
                        retValue.Append(L",");
                    }

                    retValue.Append(strConverter->StringOfId(genericParameter->fullTypeNameId));
                });
                retValue.Append(L">");
            }

            // followed by ids
            retValue.Append(L" (#");
            retValue.AppendInt32(this->typeId);
            if (!this->genericParameters->IsEmpty())
            {
                retValue.Append(L"<#");
                bool isFirst = true;
                this->genericParameters->Iterate([&](RtTYPE genericParameter) {
                    if (isFirst)
                    {
                        isFirst = false;
                    }
                    else
                    {
                        retValue.Append(L",#");
                    }

                    retValue.AppendInt32(genericParameter->fullTypeNameId);
                });
                retValue.Append(L">");
            }
            retValue.Append(L")");

            return retValue.Get(allocator);
        }
    };

    enum DeferredProjectionConstructorExprType
    {
        dpcetNone = 0,
        dpcetRuntimeClass,
        dpcetInterface,
    };

    typedef struct DeferredProjectionConstructorExpr
    {
        DeferredProjectionConstructorExprType deferredType;
        MetadataStringId typeId;
        RtEXPR minimalRtEXPR;

        DeferredProjectionConstructorExpr(DeferredProjectionConstructorExprType deferredType, MetadataStringId typeId)
        {
            Assert(deferredType != DeferredProjectionConstructorExprType::dpcetNone);
            Assert(typeId != MetadataStringIdNil);

            this->deferredType = deferredType;
            this->typeId = typeId;
            this->minimalRtEXPR = nullptr;
        }

        // Indicates whether PASS1 stage (break cycles in dependencies, low-fi interface ctors)
        bool IsPass1() const
        {
            return this->minimalRtEXPR == nullptr;
        }

        // Indicates whether PASS2 stage (low-fi interface ctor available for hi-fi interface ctor)
        bool IsPass2() const
        {
            return this->minimalRtEXPR != nullptr;
        }
    } DeferredProjectionConstructorExpr;


    MetadataStringId GetGenericInstantiationNameFromParentName(MetadataStringId parentTypeName, ImmutableList<RtTYPE> * genericParameters, ArenaAllocator * a, Metadata::IStringConverter * stringConverter);

    #define RtAnew(allocator, T, ...) static_cast<T::TConst>(Anew(allocator, T::TForConstructionOnly, __VA_ARGS__))

    typedef Tuple<MetadataStringId, ImmutableList<RtTYPE>*> TypeDefInstantiation;
    typedef JsUtil::BaseDictionary<MetadataStringId, InterfaceConstructor*, ArenaAllocator, PrimeSizePolicy> INTERFACECONSTRUCTORMAP;
    typedef JsUtil::BaseDictionary<MetadataStringId, RtTYPECONSTRUCTOR, ArenaAllocator, PrimeSizePolicy> RUNTIMECLASSCONSTRUCTORMAP;
    typedef JsUtil::BaseDictionary<MetadataStringId, Expr*, ArenaAllocator, PrimeSizePolicy> EXPRMAP;
    typedef JsUtil::BaseDictionary<MetadataStringId, Type*, ArenaAllocator, PrimeSizePolicy> TYPEMAP;
    typedef JsUtil::BaseDictionary<MetadataStringId, MetadataStringId, ArenaAllocator, PrimeSizePolicy> CAMELCASINGMAP;
#if DBG
    typedef JsUtil::BaseDictionary<MetadataStringId, MetadataStringId, ArenaAllocator, PrimeSizePolicy> ALIASMAP;
#endif

    typedef JsUtil::BaseDictionary<MetadataStringId, DeferredProjectionConstructorExpr*, ArenaAllocator, PrimeSizePolicy> DEFERREDPROJECTIONCTORMAP;

    typedef JsUtil::Stack<MetadataStringId, ArenaAllocator> PENDINGTYPESTACK;

    template<class TAllocator>
    HRESULT GetInstantiatedIIDAndTypeNameParts(
        __in ProjectionBuilder *builder,
        __in MetadataStringId typeId,
        __in ImmutableList<RtTYPE> *genericInstantiations,
        __in TAllocator * a,
        __out IID *instantiatedIID,
        __out_opt unsigned int *locatorNameCount,
        __out_opt LPCWSTR **locatorNames,
        bool *isWebHidden = nullptr);

    struct ProjectionBuilder
    {
        friend struct AbiMethodSignature;
        friend class AllowHeavyOperation;
        friend struct Type;

        typedef struct CustomAttributeInfo
        {
            bool isWebHostHidden;
            bool isAllowForWeb;
            bool isSimpleActivatable;
            bool isDeprecated;
            bool isContractVersioned;
            DWORD version;
            INT32 gcPressure;
            ImmutableList<DeprecatedAttribute>* deprecatedAttributes;
            CustomAttributeInfo()
            {
                isWebHostHidden = false;
                isAllowForWeb = false;
                isSimpleActivatable = false;
                isDeprecated = false;
                isContractVersioned = false;
                version = 0;
                gcPressure = 0;
                deprecatedAttributes = ImmutableList<DeprecatedAttribute>::Empty();
            };
        } CustomAttributeInfo;

        ProjectionBuilder(ITypeResolver * resolver, Metadata::IStringConverter * stringConverter, ArenaAllocator * allocator, DWORD targetVersion, bool isWinRTAdaptiveAppsEnabled)
            : resolver(resolver), stringConverter(stringConverter), allocator(allocator), builtInConstructorProperties(nullptr), targetVersion(targetVersion),
              builtInInstanceProperties(nullptr),implementedInterfaceConstructorsCheck(nullptr), currentImplementedRuntimeClassInterfaceConstructors(nullptr)
              ,ignoreWebHidden(false), enforceAllowForWeb(false), isWinRTAdaptiveAppsEnabled(isWinRTAdaptiveAppsEnabled)
        {
            Js::VerifyCatastrophic(resolver);
            Js::VerifyCatastrophic(stringConverter);

            interfaceCache = Anew(allocator, INTERFACECONSTRUCTORMAP, allocator);
            runtimeClassCache = Anew(allocator, RUNTIMECLASSCONSTRUCTORMAP, allocator);
            typeIdToExpr = Anew(allocator, EXPRMAP, allocator);
            typeIdToType = Anew(allocator, TYPEMAP, allocator);
            idToCamelCasedId = Anew(allocator, CAMELCASINGMAP, allocator);
#if DBG
            typeIdToExprTypeId = Anew(allocator, ALIASMAP, allocator);
#endif
            deferredConstructorMap = Anew(allocator, DEFERREDPROJECTIONCTORMAP, allocator);

            pendingStructTypes = Anew(allocator, PENDINGTYPESTACK, allocator);

            //Pre-compute ids of string literals
            constructorId = stringConverter->IdOfString(L"constructor");
            prototypeId = stringConverter->IdOfString(L"prototype");
            lengthId = stringConverter->IdOfString(L"length");
            returnValueId = stringConverter->IdOfString(szDefaultRetValParameterName);
            iteratorId = stringConverter->IdOfString(L"Windows.Foundation.Collections.IIterator`1");
            iteratorInstantiatedId = stringConverter->IdOfString(L"Windows.Foundation.Collections.IIterator`1<Windows.Foundation.Collections.IKeyValuePair`2<String,Object>>");
            addEventListenerId = stringConverter->IdOfString(L"addEventListener");
            removeEventListenerId = stringConverter->IdOfString(L"removeEventListener");
            unknownStaticInterfaceId = stringConverter->IdOfString(L"Unknown static interface");
            constructorResultId = stringConverter->IdOfString(L"constructorResult");
            invokeId = stringConverter->IdOfString(L"Invoke");
            ctorId = stringConverter->IdOfString(L".ctor");
            ctorMetadataId = stringConverter->IdOfString(L"#ctor");
            activatableAttributeId = stringConverter->IdOfString(L"Windows.Foundation.Metadata.ActivatableAttribute");
            deprecatedAttributeId = stringConverter->IdOfString(L"Windows.Foundation.Metadata.DeprecatedAttribute");
            staticAttributeId = stringConverter->IdOfString(L"Windows.Foundation.Metadata.StaticAttribute");
            webHostHiddenAttributeId = stringConverter->IdOfString(L"Windows.Foundation.Metadata.WebHostHiddenAttribute");
            allowForWebAttributeId = stringConverter->IdOfString(L"Windows.Foundation.Metadata.AllowForWebAttribute");
            gcPressureAttributeId = stringConverter->IdOfString(L"Windows.Foundation.Metadata.GCPressureAttribute");
            versionAttributeId = stringConverter->IdOfString(L"Windows.Foundation.Metadata.VersionAttribute");
            contractVersionAttributeId = stringConverter->IdOfString(L"Windows.Foundation.Metadata.ContractVersionAttribute");
            previousContractVersionAttributeId = stringConverter->IdOfString(L"Windows.Foundation.Metadata.PreviousContractVersionAttribute");
            uncallableMethodSignatureId = stringConverter->IdOfString(L"{UncallableMethodSignature}");
            exclusiveToAttributeId = stringConverter->IdOfString(L"Windows.Foundation.Metadata.ExclusiveToAttribute");

            getSizeMethodId = stringConverter->IdOfString(L"get_Size");
            getAtMethodId = stringConverter->IdOfString(L"GetAt");
            setAtMethodId = stringConverter->IdOfString(L"SetAt");
            appendMethodId = stringConverter->IdOfString(L"Append");
            removeAtEndMethodId = stringConverter->IdOfString(L"RemoveAtEnd");
            hasKeyMethodId = stringConverter->IdOfString(L"HasKey");
            insertMethodId = stringConverter->IdOfString(L"Insert");
            lookupMethodId = stringConverter->IdOfString(L"Lookup");
            removeMethodId = stringConverter->IdOfString(L"Remove");
            firstMethodId = stringConverter->IdOfString(L"First");
            getValueMethodId = stringConverter->IdOfString(L"get_Value");

            stringTypeId = stringConverter->IdOfString(L"String");
            booleanTypeId = stringConverter->IdOfString(L"Boolean");
            uint8TypeId = stringConverter->IdOfString(L"UInt8");
            int16TypeId = stringConverter->IdOfString(L"Int16");
            int32TypeId = stringConverter->IdOfString(L"Int32");
            int64TypeId = stringConverter->IdOfString(L"Int64");
            singleTypeId = stringConverter->IdOfString(L"Single");
            doubleTypeId = stringConverter->IdOfString(L"Double");
            uint16TypeId = stringConverter->IdOfString(L"UInt16");
            uint32TypeId = stringConverter->IdOfString(L"UInt32");
            uint64TypeId = stringConverter->IdOfString(L"UInt64");
            objectTypeId = stringConverter->IdOfString(L"Object");
            char16TypeId = stringConverter->IdOfString(L"Char16");
            
            dateTimeTypeId = stringConverter->IdOfString(GetWindowsFoundationDateTimeTypeName());
            timeSpanTypeId = stringConverter->IdOfString(GetWindowsFoundationTimeSpanTypeName());
            eventRegistrationTokenTypeId = stringConverter->IdOfString(GetWindowsFoundationEventRegistrationTokenTypeName());
            hResultTypeId = stringConverter->IdOfString(GetWindowsFoundationHResultTypeName());

            arrayTypeNameId = stringConverter->IdOfString(L"Array");
            byRefTypeNameId = stringConverter->IdOfString(L"ByRef");
            varTypeNameId = stringConverter->IdOfString(L"T");
            iBufferId = stringConverter->IdOfString(L"Windows.Storage.Streams.IBuffer");
            capacityId = stringConverter->IdOfString(L"Capacity");
            byteLengthId = stringConverter->IdOfString(L"byteLength");

            exprNull = Anew(allocator, Expr, exprNullLiteral);
            typeVoid = RtAnew(allocator, VoidType, stringConverter->IdOfString(L"Void"));
            typeMultiOut = RtAnew(allocator, MultiOutType);
            typeString = TypeOfBasicType(ELEMENT_TYPE_STRING);
            typeBool = TypeOfBasicType(ELEMENT_TYPE_BOOLEAN);
            typeByte = TypeOfBasicType(ELEMENT_TYPE_U1);
            typeInt16 = TypeOfBasicType(ELEMENT_TYPE_I2);
            typeInt32 = TypeOfBasicType(ELEMENT_TYPE_I4);
            typeInt64 = TypeOfBasicType(ELEMENT_TYPE_I8);
            typeFloat = TypeOfBasicType(ELEMENT_TYPE_R4);
            typeDouble = TypeOfBasicType(ELEMENT_TYPE_R8);
            typeUint16 = TypeOfBasicType(ELEMENT_TYPE_U2);
            typeUint32 = TypeOfBasicType(ELEMENT_TYPE_U4);
            typeUint64 = TypeOfBasicType(ELEMENT_TYPE_U8);
            typeObject = TypeOfBasicType(ELEMENT_TYPE_OBJECT);
            typeCharProjectedAsString = TypeOfBasicType(ELEMENT_TYPE_CHAR);
            typeSystemGuid = RtAnew(allocator, SystemGuidType, stringConverter->IdOfString(L"Guid"));

            typeWindowsFoundationDateTime = RtAnew(allocator, WindowsFoundationDateTimeType, dateTimeTypeId);
            typeWindowsFoundationTimeSpan = RtAnew(allocator, WindowsFoundationTimeSpanType, timeSpanTypeId);
            typeWindowsFoundationEventRegistrationToken = RtAnew(allocator, WindowsFoundationEventRegistrationTokenType, eventRegistrationTokenTypeId);
            typeWindowsFoundationHResult = RtAnew(allocator, WindowsFoundationHResultType, hResultTypeId);

            emptyPropertiesObject = Anew(allocator, PropertiesObject, nullptr);

            voidParameters = Anew(allocator, Parameters, nullptr, typeVoid);

            constructorChar = ConstructorOfBasicType(typeCharProjectedAsString);
            constructorByte = ConstructorOfBasicType(typeByte);
            constructorInt16 = ConstructorOfBasicType(typeInt16);
            constructorInt32 = ConstructorOfBasicType(typeInt32);
            constructorInt64 = ConstructorOfBasicType(typeInt64);
            constructorFloat = ConstructorOfBasicType(typeFloat);
            constructorDouble = ConstructorOfBasicType(typeDouble);
            constructorString = ConstructorOfBasicType(typeString);
            constructorUint16 = ConstructorOfBasicType(typeUint16);
            constructorUint32 = ConstructorOfBasicType(typeUint32);
            constructorUint64 = ConstructorOfBasicType(typeUint64);
            constructorBool = ConstructorOfBasicType(typeBool);
            constructorObject = ConstructorOfBasicType(typeObject);
            constructorVoid = Anew(allocator, VoidConstructor, Anew(allocator, TypeConstructorMethodSignature, ctorId, voidParameters));
            constructorSystemGuid = Anew(allocator, SystemGuidConstructor, stringConverter->IdOfString(L"System.Guid"), Anew(allocator, TypeConstructorMethodSignature, ctorId, Anew(allocator, Parameters, nullptr, typeSystemGuid)));

            constructorWindowsFoundationDateTime = Anew(allocator, WindowsFoundationDateTimeConstructor, dateTimeTypeId, Anew(allocator, TypeConstructorMethodSignature, ctorId, Anew(allocator, Parameters, nullptr, typeWindowsFoundationDateTime)));
            constructorWindowsFoundationTimeSpan = Anew(allocator, WindowsFoundationTimeSpanConstructor, timeSpanTypeId, Anew(allocator, TypeConstructorMethodSignature, ctorId, Anew(allocator, Parameters, nullptr, typeWindowsFoundationTimeSpan)));
            constructorWindowsFoundationEventRegistrationToken = Anew(allocator, WindowsFoundationEventRegistrationTokenConstructor, eventRegistrationTokenTypeId, Anew(allocator, TypeConstructorMethodSignature, ctorId, Anew(allocator, Parameters, nullptr, typeWindowsFoundationEventRegistrationToken)));
            constructorWindowsFoundationHResult = Anew(allocator, WindowsFoundationHResultConstructor, hResultTypeId, Anew(allocator, TypeConstructorMethodSignature, ctorId, Anew(allocator, Parameters, nullptr, typeWindowsFoundationHResult)));

            // These are the built in properties of functions. Used to resolve name conflicts.
            MetadataStringId constructorBuiltIns[] = {constructorId,prototypeId,lengthId};
            MetadataStringId instanceBuiltIns[] = {constructorId,prototypeId};
            builtInConstructorProperties = builtInConstructorProperties->PrependArray(constructorBuiltIns,sizeof(constructorBuiltIns)/sizeof(MetadataStringId),allocator);
            builtInInstanceProperties = builtInInstanceProperties->PrependArray(instanceBuiltIns,sizeof(instanceBuiltIns)/sizeof(MetadataStringId),allocator);

            typeUnprojectable = RtAnew(allocator, UnprojectableType, stringConverter->IdOfString(L"Unprojectable"));
            noFields = nullptr;
            noAssignments = nullptr;
            noGenericParameters = ImmutableList<const Metadata::GenericParameterProperties*>::Empty();
            emptyAssignmentSpace = Anew(allocator, AssignmentSpace, noAssignments);

            uncallableMethodSignature = Anew(allocator, UncallableMethodSignature, uncallableMethodSignatureId, voidParameters);

        }

        void ClearCaches()
        {
            interfaceCache->Clear();
            runtimeClassCache->Clear();
            typeIdToExpr->Clear();
            typeIdToType->Clear();

            Assert(deferredConstructorMap != nullptr);
#if DBG
            // for debug builds, print out the leaked deferred Ctors
            if (deferredConstructorMap->Count() > 0)
            {
                Output::Print(L"Leaked ProjectionModel::deferredConstructorMap:\n");
                size_t count = deferredConstructorMap->Count();
                for (size_t offset=0; offset<count; offset++)
                {
                    DeferredProjectionConstructorExpr* value = deferredConstructorMap->GetValueAt((int)offset);
                    Assert(value != nullptr);
                    Output::Print(L"deferredConstructorMap[%d/%d]: %s (#%d) - deferredType=%d\n", 
                        offset, count, this->stringConverter->StringOfId(value->typeId), value->typeId, value->deferredType);
                }
                Output::Flush();
            }
#endif
            Assert(deferredConstructorMap->Count() == 0);
            deferredConstructorMap->Clear();
        }

        RtASSIGNMENTSPACE AddFromMetadataImport(Option<AssignmentSpace> varSpace, Metadata::Assembly * assembly);
        RtEXPR ExprOfToken(MetadataStringId typeId, mdToken token, const Metadata::Assembly & assembly, ImmutableList<RtTYPE> * genericParameters);
        RtEXPR ExprOfPossiblyGenericTypename(LPCWSTR typeName);
        RtEXPR TryGetExprOfTypeId(MetadataStringId typeId);
        RtTYPE TypeOfToken(mdToken token, const Metadata::Assembly & assembly, ImmutableList<RtTYPE> * genericParameters, bool valueType = false);
        MetadataStringId GetBasicTypeIdFromTypeCor(CorElementType typeCor);
        RtBASICTYPE TypeOfBasicType(CorElementType metadataType);
        RtTYPE GetSystemGuidType() { return typeSystemGuid; }
        RtTYPE GetBasicAndKnownTypeByName(LPCWSTR fullTypeName);
        RtTYPE GetKnownTypeByName(LPCWSTR fullTypeName);

        RtTYPE GetWindowsFoundationDateTimeType() const { return typeWindowsFoundationDateTime; }
        RtTYPE GetWindowsFoundationTimeSpanType() const { return typeWindowsFoundationTimeSpan; }
        RtTYPE GetWindowsFoundationEventRegistrationTokenType() const { return typeWindowsFoundationEventRegistrationToken; }
        RtTYPE GetWindowsFoundationHResultType() const { return typeWindowsFoundationHResult; }

        LPCWSTR GetWindowsFoundationDateTimeTypeName() const { return L"Windows.Foundation.DateTime"; }
        LPCWSTR GetWindowsFoundationTimeSpanTypeName() const { return L"Windows.Foundation.TimeSpan"; }
        LPCWSTR GetWindowsFoundationEventRegistrationTokenTypeName() const { return L"Windows.Foundation.EventRegistrationToken"; }
        LPCWSTR GetWindowsFoundationHResultTypeName() const { return L"Windows.Foundation.HResult"; }

        RtBASICTYPE GetBasicType(CorElementType metadataType) const;
        RtBASICTYPE GetChar16BasicType() const { return this->typeCharProjectedAsString; }
        RtBASICTYPE GetByteBasicType() const { return this->typeByte; }
        RtBASICTYPE GetInt16BasicType() const { return this->typeInt16; }
        RtBASICTYPE GetInt32BasicType() const { return this->typeInt32; }
        RtBASICTYPE GetInt64BasicType() const { return this->typeInt64; }
        RtBASICTYPE GetFloatBasicType() const { return this->typeFloat; }
        RtBASICTYPE GetDoubleBasicType() const { return this->typeDouble; }
        RtBASICTYPE GetStringBasicType() const { return this->typeString; }
        RtBASICTYPE GetUint16BasicType() const { return this->typeUint16; }
        RtBASICTYPE GetUint32BasicType() const { return this->typeUint32; }
        RtBASICTYPE GetUint64BasicType() const { return this->typeUint64; }
        RtBASICTYPE GetBoolBasicType() const { return this->typeBool; }
        RtBASICTYPE GetObjectBasicType() const { return this->typeObject; }

        RtBASICTYPE GetWindowsFoundationDateTimeUnderlyingType() const { return typeInt64; }
        RtBASICTYPE GetWindowsFoundationTimeSpanUnderlyingType() const { return typeInt64; }
        RtBASICTYPE GetWindowsFoundationEventRegistrationTokenUnderlyingType() const { return typeInt64; }
        RtBASICTYPE GetWindowsFoundationHResultUnderlyingType() const { return typeInt32; }

        bool IsWithinTargetVersion(mdToken token, const Metadata::Assembly & assembly);
        bool CanMarshalExpr(RtEXPR expr, bool fAllowGenericType = false, bool allowMissingTypes = false, bool *outWasMissingType = nullptr, bool allowWebHidden = false);

        HRESULT GetInstantiatedIID(
            __in MetadataStringId typeId,
            __in ImmutableList<RtTYPE> *genericInstantiations,
            __in HeapAllocator * a,
            __out IID *instantiatedIID)
        {
            return ProjectionModel::GetInstantiatedIIDAndTypeNameParts<HeapAllocator>(this, typeId, genericInstantiations, a, instantiatedIID, nullptr, nullptr);
        }

        Metadata::IStringConverter * stringConverter;
        ITypeResolver * GetResolver() { return resolver; }
        bool IsWinRTAdaptiveAppsEnabled() const { return isWinRTAdaptiveAppsEnabled; }

    private:
        bool CanMarshalType(RtTYPE type, bool fAllowGenericType = false, bool allowMissingTypes = false, bool *outWasMissingType = nullptr, bool allowWebHidden = false);
        

        ImmutableList<DeferredTypeDefinitionCandidate> * implementedInterfaceConstructorsCheck;
        ImmutableList<DeferredTypeDefinitionCandidate> * currentImplementedRuntimeClassInterfaceConstructors;        // keeps track of the current RTCs being ctor-ed

        RtBASICTYPE typeCharProjectedAsString;
        RtBASICTYPE typeByte;
        RtBASICTYPE typeInt16;
        RtBASICTYPE typeInt32;
        RtBASICTYPE typeInt64;
        RtBASICTYPE typeFloat;
        RtBASICTYPE typeDouble;
        RtBASICTYPE typeString;
        RtBASICTYPE typeUint16;
        RtBASICTYPE typeUint32;
        RtBASICTYPE typeUint64;
        RtBASICTYPE typeBool;
        RtTYPE typeVoid;
        RtTYPE typeMultiOut;
        RtBASICTYPE typeObject;
        RtTYPE typeSystemGuid;

        RtTYPE typeWindowsFoundationDateTime;
        RtTYPE typeWindowsFoundationTimeSpan;
        RtTYPE typeWindowsFoundationEventRegistrationToken;
        RtTYPE typeWindowsFoundationHResult;

        RtPARAMETERS voidParameters;

        RtFUNCTION constructorChar;
        RtFUNCTION constructorByte;
        RtFUNCTION constructorInt16;
        RtFUNCTION constructorInt32;
        RtFUNCTION constructorInt64;
        RtFUNCTION constructorFloat;
        RtFUNCTION constructorDouble;
        RtFUNCTION constructorString;
        RtFUNCTION constructorUint16;
        RtFUNCTION constructorUint32;
        RtFUNCTION constructorUint64;
        RtFUNCTION constructorBool;
        RtFUNCTION constructorVoid;
        RtFUNCTION constructorObject;
        RtFUNCTION constructorSystemGuid;
        
        RtFUNCTION constructorWindowsFoundationDateTime;
        RtFUNCTION constructorWindowsFoundationTimeSpan;
        RtFUNCTION constructorWindowsFoundationEventRegistrationToken;
        RtFUNCTION constructorWindowsFoundationHResult;

        RtTYPE typeUnprojectable;
        RtTYPE typeNumber;
        RtTYPE typeTypedObject;
        RtEXPR exprNull;

        MetadataStringId constructorId;
        MetadataStringId prototypeId;
        MetadataStringId lengthId;
        MetadataStringId returnValueId;
        MetadataStringId iteratorId;
        MetadataStringId iteratorInstantiatedId;
        MetadataStringId addEventListenerId;
        MetadataStringId removeEventListenerId;
        MetadataStringId unknownStaticInterfaceId;
        MetadataStringId constructorResultId;
        MetadataStringId invokeId;
        MetadataStringId activatableAttributeId;
        MetadataStringId deprecatedAttributeId;
        MetadataStringId staticAttributeId;
        MetadataStringId webHostHiddenAttributeId;
        MetadataStringId allowForWebAttributeId;
        MetadataStringId gcPressureAttributeId;
        MetadataStringId versionAttributeId;
        MetadataStringId previousContractVersionAttributeId;
        MetadataStringId contractVersionAttributeId;
        MetadataStringId uncallableMethodSignatureId;
        MetadataStringId ctorId;
        MetadataStringId ctorMetadataId;
        MetadataStringId exclusiveToAttributeId;

        MetadataStringId getSizeMethodId;
        MetadataStringId getAtMethodId;
        MetadataStringId setAtMethodId;
        MetadataStringId appendMethodId;
        MetadataStringId removeAtEndMethodId;
        MetadataStringId hasKeyMethodId;
        MetadataStringId insertMethodId;
        MetadataStringId lookupMethodId;
        MetadataStringId removeMethodId;
        MetadataStringId firstMethodId;
        MetadataStringId getValueMethodId;

        MetadataStringId stringTypeId;
        MetadataStringId booleanTypeId;
        MetadataStringId uint8TypeId;
        MetadataStringId int16TypeId;
        MetadataStringId int32TypeId;
        MetadataStringId int64TypeId;
        MetadataStringId singleTypeId;
        MetadataStringId doubleTypeId;
        MetadataStringId uint16TypeId;
        MetadataStringId uint32TypeId;
        MetadataStringId uint64TypeId;
        MetadataStringId objectTypeId;
        MetadataStringId char16TypeId;
        MetadataStringId guidTypeId;
        
        MetadataStringId dateTimeTypeId;
        MetadataStringId timeSpanTypeId;
        MetadataStringId eventRegistrationTokenTypeId;
        MetadataStringId hResultTypeId;

        MetadataStringId arrayTypeNameId;
        MetadataStringId byRefTypeNameId;
        MetadataStringId varTypeNameId;
        MetadataStringId iBufferId;
        MetadataStringId capacityId;
        MetadataStringId byteLengthId;

        BOOL    ignoreWebHidden;    // TODO: have a enum for more properties?
        BOOL    enforceAllowForWeb;
        ITypeResolver * resolver;
        const bool isWinRTAdaptiveAppsEnabled;
        DWORD targetVersion;
        ArenaAllocator * allocator;
        INTERFACECONSTRUCTORMAP * interfaceCache; // Key is interface type id (Can this fold into typeIdToType?)
        RUNTIMECLASSCONSTRUCTORMAP * runtimeClassCache; // Key is class type id (Can this fold into typeIdToType?)
        EXPRMAP * typeIdToExpr;
        TYPEMAP * typeIdToType;
        CAMELCASINGMAP * idToCamelCasedId;
#if DBG
        ALIASMAP * typeIdToExprTypeId; // Keep track of aliases as a sanity check in DBG mode only
#endif

        DEFERREDPROJECTIONCTORMAP* deferredConstructorMap;

        PENDINGTYPESTACK* pendingStructTypes; // Keep track of struct types that are being resolved to detect cycles

        RtMETHODSIGNATURE uncallableMethodSignature;
        ImmutableList<RtPROPERTY> * noFields;
        ImmutableList<RtASSIGNMENT> * noAssignments;
        ImmutableList<const Metadata::GenericParameterProperties*> * noGenericParameters;
        ImmutableList<MetadataStringId> * builtInConstructorProperties;
        ImmutableList<MetadataStringId> * builtInInstanceProperties;
        AssignmentSpace * emptyAssignmentSpace;
        RtPROPERTIESOBJECT emptyPropertiesObject;

        TypeDefInstantiation InstantiateTypeDefinition(const Metadata::TypeDefProperties * type, ImmutableList<RtTYPE> * genericParameters);
        RtFUNCTION ConstructorOfCorElementType(CorElementType typeCode, const Metadata::Assembly & assembly);
    public:
        RtEXPR ExprOfType(const Metadata::Type * type, const Metadata::Assembly & assembly, ImmutableList<RtTYPE> * genericParameters);
    private:
        RtEXPR ExprOfTypeDefProperties(const Metadata::TypeDefProperties * type, ImmutableList<RtTYPE> * genericParameters);
        RtTYPE TypeOfTypeDef(const Metadata::TypeDefProperties * type, ImmutableList<RtTYPE> * genericParameters);
        RtTYPE StructTypeOfTypeDef(const Metadata::TypeDefProperties * typeDef);
        RtTYPE TypeOfType(const Metadata::Type * nativeType, const Metadata::Assembly & assembly, ImmutableList<RtTYPE> * genericParameters);
        const ConcreteType * ConcreteTypeOfType(const Metadata::Type * nativeType, const Metadata::Assembly & assembly, ImmutableList<RtTYPE> * genericParameters, bool* isWebHidden = nullptr);
        RtABIPARAMETER ParameterOfParameter(const Metadata::ParameterProperties *props, const Metadata::Parameter * parameter, const Metadata::Assembly & assembly, ImmutableList<RtTYPE> * genericParameters, size_t inParameterIndex, size_t retValIndex);
        RtTYPE ReturnTypeOfOutParameters(ImmutableList<RtABIPARAMETER> * outParameters, const Metadata::Assembly & assembly);
        RtABIMETHODSIGNATURE MethodSignatureOfMethod(const Metadata::TypeDefProperties * parentType, RtIID iid, const Metadata::MethodProperties * method, ImmutableList<RtTYPE> * genericParameters, MethodKind methodKind = MethodKind_Normal);
        RtABIMETHODSIGNATURE MethodSignatureOfMetadataArityGroup(RtIID iid, const MetadataArityGroup * metadataArityGroup, const Metadata::Assembly & assembly, ImmutableList<RtTYPE> * genericParameters);
        RtOVERLOADGROUP OverloadGroupOfMetadataOverloadGroup(RtIID iid, const MetadataOverloadGroup * metadataOverloadGroup, const Metadata::Assembly & assembly, ImmutableList<RtTYPE> * genericParameters);
        ImmutableList<RtSYNTHETICPARAMETER> * ProjectionBuilder::SyntheticOverloadParameters(size_t maxArity);
        RtOVERLOADEDMETHODSIGNATURE OverloadedMethodSignatureOfOverloadGroup(const OverloadGroup * overloadGroup);
        RtPROPERTY FieldOfMetadataOverloadGroup(RtIID iid, const MetadataOverloadGroup * overloadGroup, const Metadata::TypeDefProperties * type, ImmutableList<RtTYPE> * genericParameters);
        RtEXPR LiteralOfFieldBlob(DWORD typeFlag, UVCP_CONSTANT blob);
        RtABIFIELDPROPERTY PropertyOfFieldProperties(const Metadata::FieldProperties * fieldProps, size_t fieldOffset, ImmutableList<RtTYPE> * genericParameters);
        RtABIMETHODPROPERTY PropertyOfMethod(MetadataStringId id, const Metadata::TypeDefProperties * parentType, RtIID iid, const Metadata::MethodProperties * method, ImmutableList<RtTYPE> * genericParameters);
        const TypeConstructorMethodSignature * MethodSignatureOfType(const Metadata::TypeDefProperties * type, ImmutableList<RtTYPE> * genericParameters);
        ImmutableList<RtINTERFACECONSTRUCTOR> * ImplementedInterfaceConstructors(mdToken token, const Metadata::Assembly & assembly, ImmutableList<RtTYPE> * genericParameters, Option<InterfaceConstructor> * defaultInterface);
        const MissingInstantiationConstructor * ConstructorOfMissingGenericInstantiationType(RtMISSINGGENERICINSTANTIATIONTYPE git);
        RtEXPR ConstructorOfStruct(const Metadata::TypeDefProperties * type);
        RtEXPR EnumOfEnum(const Metadata::TypeDefProperties * type);
        RtFUNCTION ConstructorOfBasicType(RtBASICTYPE basicType);
        RtSPECIALIZATION GetSpecialization(ImmutableList<RtPROPERTY> * parentProperties, ImmutableList<RtINTERFACECONSTRUCTOR> * interfaces, RtIID parentIID);
        void ResolveNameConflicts(ImmutableList<RtINTERFACECONSTRUCTOR> * interfaces, MetadataStringId parentTypeId, RtIID parentIID, ImmutableList<RtPROPERTY> * parentProperties, ImmutableList<RtPROPERTY> ** resultProperties, RtSPECIALIZATION* specialization, bool isStatic = false);
        void GetInterfacesOfRuntimeClass(const Metadata::TypeDefProperties * type, Option<InterfaceConstructor> * defaultInterface, RtSPECIALIZATION* specialization, ImmutableList<RtINTERFACECONSTRUCTOR> ** allInterfaces);
        ImmutableList<RtINTERFACECONSTRUCTOR> * GetInterfaceClosureFromFirstLevelTypeDefs(ImmutableList<const Metadata::TypeDefProperties *> * firstLevelInterfaces);
        void GetSignatureOfRuntimeClass(const Metadata::TypeDefProperties * type, ImmutableList<const Metadata::TypeDefProperties *> * factoryInterfaceTypeDefs, CustomAttributeInfo& customAttributeInfo, RtMETHODSIGNATURE * resultSignature, int * length);
        void GetPropertiesOfRuntimeClass(const Metadata::TypeDefProperties * type, ImmutableList<const Metadata::TypeDefProperties *> * staticInterfaceTypeDefs, int signatureLength, RtPROPERTIESOBJECT * propertiesObject, ImmutableList<RtINTERFACECONSTRUCTOR> ** allInterfaces);
        void InterfaceMethodPropsOfClassMethodDef(const Metadata::MethodProperties * classMethod, ImmutableList<const Metadata::MethodImpl*> * methodImpls, ImmutableList<RtINTERFACECONSTRUCTOR> * allInterfaces, AbiMethodSignature const ** signature, MetadataStringId * disambiguationNameId);
        RtTYPECONSTRUCTOR ConstructorOfMissingTypeDef(const Metadata::TypeDefProperties * type, bool isWebHidden = false);
        RtTYPECONSTRUCTOR ConstructorOfMissingNamedType(RtMISSINGNAMEDTYPE missingType);
        RtPROPERTY PropertyOfPropertyGroup(ImmutableList<RtPROPERTY> * propertyGroup);
        void InjectIBufferByteLengthProperty(const Metadata::TypeDefProperties * type, ImmutableList<RtPROPERTY> * & instanceProperties);
        RtEXPR ExprOfRuntimeClass(const Metadata::TypeDefProperties * type);
        RtEXPR ConstructorOfDelegate(const Metadata::TypeDefProperties * type, ImmutableList<RtTYPE> * genericParameters);
        ImmutableList<const MetadataOverloadGroup*> * MethodsOfInterface(const Metadata::TypeDefProperties * interfaceDef, MetadataStringId invokeId);
        RtABIPROPERTYPROPERTY PropertyOfPropertyProperties(const Metadata::TypeDefProperties * type, RtIID iid, const Metadata::PropertyProperties * propertyProperties, ImmutableList<RtTYPE> * genericParameters);
        ImmutableList<RtABIPROPERTYPROPERTY> * PropertiesOfInterface(RtIID iid, const Metadata::TypeDefProperties * type, ImmutableList<RtTYPE> * genericParameters);
        RtEVENT EventOfEventProperties(RtIID iid, MetadataStringId eventMetadataNameId, RtABIMETHODSIGNATURE addOn, RtABIMETHODSIGNATURE removeOn, ImmutableList<RtTYPE> * genericParameters
#if DBG
            , LPCWSTR eventNameStr
#endif
            );
        ImmutableList<RtEVENT> * EventsOfInterface(RtIID iid, const Metadata::TypeDefProperties * type, ImmutableList<RtTYPE> * genericParameters);
        ImmutableList<RtPROPERTY> * PropertiesOfEvents(ImmutableList<RtEVENT> * events);
        RtINTERFACECONSTRUCTOR ConstructorOfInterface(const Metadata::TypeDefProperties * type, ImmutableList<RtTYPE> * genericParameters);
        RtINTERFACECONSTRUCTOR InterfaceConstructorOfToken(mdToken token, const Metadata::Assembly & assembly, ImmutableList<RtTYPE> * genericParameters);
    public:
        RtEXPR IntermediateExprOfToken(MetadataStringId typeId, mdToken token, const Metadata::Assembly & assembly, ImmutableList<RtTYPE> * genericParameters);
    private:
        RtPROPERTIESOBJECT ApplyTypeToPropertiesObject(RtPROPERTIESOBJECT currentSpace, LPCWSTR remainingName, const Metadata::TypeDefProperties * type);
        RtASSIGNMENTSPACE ApplyTypeInAssignmentSpace(RtASSIGNMENTSPACE currentSpace, LPCWSTR remainingName, const Metadata::TypeDefProperties * type);
        ImmutableList<RtEVENT> * LowerCaseEvents(ImmutableList<RtEVENT> * events);
        ImmutableList<RtPROPERTY> * CamelCaseProperties(ImmutableList<RtPROPERTY> * fields, __in_opt ImmutableList<MetadataStringId> * reservedNames = nullptr);
        RtINTERFACECONSTRUCTOR CamelCaseInterfaceConstructor(RtINTERFACECONSTRUCTOR ic, __in_opt ImmutableList<MetadataStringId> * reservedNames = nullptr);
        MetadataStringId GetProjectionTypeId(mdToken token, const Metadata::Assembly & assembly);

        MetadataStringId TryGetRuntimeClassNamePropertyFromAttribute(const Metadata::CustomAttributeProperties * attr);
        const Metadata::TypeDefProperties * TryGetInterfaceTypeDefPropertiesFromAttribute(const Metadata::CustomAttributeProperties * attr, bool contractVersioned = false);

        bool InWindowsNamespace(MetadataStringId typeNameId);
        INT32 TryGetValueFromCustomEnumAttribute(const Metadata::CustomAttributeProperties * attr);
        void TryGetDeprecatedValueFromCustomAttribute(const Metadata::CustomAttributeProperties * attr, MetadataStringId classId, CustomAttributeInfo* customAttributeInfo);
        void TryGetExclusiveToFromCustomAttribute(const Metadata::CustomAttributeProperties * attr, CustomAttributeInfo* customAttributeInfo);
        DWORD TryGetDWORDValueFromAttribute(const Metadata::CustomAttributeProperties * attr);

        enum AttributeConstructorType
        {
            Invalid                     = 0,
            ContractVersionedBit        = 1,
            PlatformVersionedBit        = 2,
            FactoryConstructorBit       = 4,

            ContractVersioned           = ContractVersionedBit,
            PlatformVersioned           = PlatformVersionedBit,
            FactoryContractVersioned    = FactoryConstructorBit | ContractVersionedBit,
            FactoryPlatformVersioned    = FactoryConstructorBit | PlatformVersionedBit
        };

        AttributeConstructorType GetActivatableAttributeConstructorType(const Metadata::CustomAttributeProperties * attribute);
        AttributeConstructorType GetStaticAttributeConstructorType(const Metadata::CustomAttributeProperties * attribute);
        AttributeConstructorType GetDeprecatedAttributeConstructorType(const Metadata::CustomAttributeProperties * attribute);

        void AnalyzeRuntimeClassCustomAttributes(mdTypeDef typeDef,
            MetadataStringId typeNameId, ImmutableList<const Metadata::TypeDefProperties *> ** factoryInterfaces,
            ImmutableList<const Metadata::TypeDefProperties *> ** staticInterfaces,
            __inout CustomAttributeInfo* customAttributeInfo,
            const Metadata::Assembly & assembly);
        void AnalyzeMethodCustomAttributes(
            mdTypeDef typeDef,
            MetadataStringId classId,
            __inout CustomAttributeInfo* customAttributeInfo,
            const Metadata::Assembly & assembly);
        MetadataStringId AnalyzeInterfaceCustomAttributes(
            mdTypeDef typeDef,
            const Metadata::Assembly & assembly);

        LPCWSTR GetCallPatternOfSignature(ImmutableList<RtABIPARAMETER> * parameters);

        void AppendTypePattern(RtTYPE type, DefaultImmutableStringBuilder & pattern);

        // Deferred projection constructor related methods
        DeferredProjectionConstructorExpr* GetFromDeferredConstructorMap(MetadataStringId typeId) const;
        void DeleteFromDeferredConstructorMap(MetadataStringId typeId);
        bool IsCurrentImplementedRuntimeClassInterfaceConstructorsContains(MetadataStringId typeId) const;

    public:
        bool IsTypeMarkedForDeferredConstructionPass1(LPCWSTR typeName) const;

        void SetIgnoreWebHidden(BOOL shouldIgnore) { ignoreWebHidden = shouldIgnore; }
        BOOL IgnoreWebHidden() const { return ignoreWebHidden; }

        void SetEnforceAllowForWeb(BOOL shouldEnforce) { enforceAllowForWeb = shouldEnforce; }
        BOOL EnforceAllowForWeb() const { return enforceAllowForWeb; }
    private:


#if defined(_M_ARM32_OR_ARM64)
        void GetHFPData(ImmutableList<RtABIFIELDPROPERTY>* properties, StructFieldType* structType, int* hfpFieldCount);
#endif

        // Info:        Given a type name, retrieve its type from the host. If no host, call fmissing.
        // Parameter:   typeName - the name of the type to check
        //              fpresent - Path called if the type exists
        //              fmissing - Path called if the type is missing
        template<typename TOut, typename FPresent, typename FMissing>
        TOut DoWithTypenameFromOtherAssembly(LPCWSTR typeName, FPresent fpresent, FMissing fmissing)
        {
            Js::VerifyCatastrophic(typeName);
            Metadata::TypeDefProperties * typeDef;
            auto hr = resolver->ResolveTypeName(stringConverter->IdOfString(typeName), typeName, &typeDef);
            if(FAILED(hr))
            {
                return fmissing(typeName);
            }
            return fpresent(typeDef);
        }
        public:
        // Info:        Given a type ref name, retrieve its type from the host. If no host, call fmissing.
        // Parameter:   typeName - the name of the type to check
        //              fpresent - Path called if the type exists
        //              fmissing - Path called if the type is missing
        template<typename TOut, typename FPresent, typename FMissing>
        TOut DoWithTypeFromOtherAssembly(const Metadata::Assembly & thisAssembly, mdTypeRef tr, FPresent fpresent, FMissing fmissing)
        {
            Js::VerifyCatastrophic((mdtTypeRef & tr) == mdtTypeRef);
            auto props = thisAssembly.GetTypeRefProperties(tr);
            Metadata::TypeDefProperties * typeDef;
            auto hr = resolver->ResolveTypeName(props->id, nullptr, &typeDef);
            if(FAILED(hr))
            {
                return fmissing(stringConverter->StringOfId(props->id));
            }
            return fpresent(typeDef->assembly, typeDef->td);
        }
        private:
#ifdef PROJECTION_METADATA_TRACE
        static void Trace(const wchar_t *form, ...) // const
        {
            if (Js::Configuration::Global.flags.Trace.IsEnabled(Js::ProjectionMetadataPhase))
            {
                va_list argptr;
                va_start(argptr, form);
                Output::Print(L"ProjectionModel: ");
                Output::VPrint(form, argptr);
                Output::Flush();
            }
        }
#endif

        BOOL IsTypeAllowedForWeb(const Metadata::TypeDefProperties * type);
        ImmutableList<RtPROPERTY>* PropertiesOfOverload(_In_ RtOVERLOADPARENTPROPERTY overloadedParentProperty);
        ImmutableList<RtPROPERTY>* PropertiesOfOverloadsAndMethods(_In_ ImmutableList<RtPROPERTY>* propertiesWithOverloadsAndMethods);
        ImmutableList<RtPROPERTY>* ResolveAliases(_In_ ImmutableList<RtPROPERTY>* properties, ImmutableList<MetadataStringId>** propertiesAliasedSuccessfully = nullptr);

    public:
        bool TypePartiallyResolved(const Metadata::TypeDefProperties * type);

        BOOL IsTypeWebHidden(const Metadata::TypeDefProperties * type);
#if DBG
        // debug helpers
        static LPCWSTR TraceRtTYPE(_In_ RtTYPE type, _In_ Metadata::IStringConverter* strConverter, _In_ ArenaAllocator* allocator);
        static LPCWSTR TraceRtPARAMETERS(_In_ RtPARAMETERS parameters, _In_ Metadata::IStringConverter* strConverter, _In_ ArenaAllocator* allocator);
        static LPCWSTR TraceRtOVERLOADEDMETHODSIGNATURE(_In_ RtOVERLOADEDMETHODSIGNATURE signature, _In_ Metadata::IStringConverter* strConverter, _In_ ArenaAllocator* allocator);
        static LPCWSTR TraceRtIID(_In_ RtIID rtIID, _In_ Metadata::IStringConverter* strConverter, _In_ ArenaAllocator* allocator);
        static LPCWSTR TraceRtABIMETHODSIGNATURE(_In_ RtABIMETHODSIGNATURE signature, _In_ Metadata::IStringConverter* strConverter, _In_ ArenaAllocator* allocator);
        static LPCWSTR TraceRtPROPERTIESOBJECT(_In_ RtPROPERTIESOBJECT properties, _In_ Metadata::IStringConverter* strConverter, _In_ ArenaAllocator* allocator);
        static void TraceRtOVERLOADPARENTPROPERTY(_In_ RtOVERLOADPARENTPROPERTY overloadParentProperty, _In_z_ LPWSTR prefixMessage, _In_ Metadata::IStringConverter* strConverter, _In_ ArenaAllocator* allocator);
        static void TraceRtABIMETHODPROPERTY(_In_ RtABIMETHODPROPERTY methodProperty, _In_z_ LPWSTR prefixMessage, _In_ Metadata::IStringConverter* strConverter, _In_ ArenaAllocator* allocator);
        static void TraceRtABIMETHODPROPERTY(_In_ ImmutableList<RtPROPERTY>* methodProperties, _In_z_ LPWSTR prefixMessage, _In_ Metadata::IStringConverter* strConverter, _In_ ArenaAllocator* allocator);
        static void TraceRtPROPERTY(_In_ RtPROPERTY prop, _In_z_ LPWSTR prefixMessage, _In_ Metadata::IStringConverter* strConverter, _In_ ArenaAllocator* allocator);
#else // !DBG
        static __inline void TraceRtOVERLOADPARENTPROPERTY(_In_ RtOVERLOADPARENTPROPERTY overloadParentProperty, _In_z_ LPWSTR prefixMessage, _In_ Metadata::IStringConverter* strConverter, _In_ ArenaAllocator* allocator) { };
        static __inline void TraceRtABIMETHODPROPERTY(_In_ RtABIMETHODPROPERTY methodProperty, _In_z_ LPWSTR prefixMessage, _In_ Metadata::IStringConverter* strConverter, _In_ ArenaAllocator* allocator) { };
        static __inline void TraceRtABIMETHODPROPERTY(_In_ ImmutableList<RtPROPERTY>* methodProperties, _In_z_ LPWSTR prefixMessage, _In_ Metadata::IStringConverter* strConverter, _In_ ArenaAllocator* allocator) { };
        static __inline void TraceRtPROPERTY(_In_ RtPROPERTY prop, _In_z_ LPWSTR prefixMessage, _In_ Metadata::IStringConverter* strConverter, _In_ ArenaAllocator* allocator) { };
#endif
    };
#if DBG
    __declspec(thread) extern int allowHeavyOperation;

    class AllowHeavyOperation
    {
    public:
        AllowHeavyOperation()
        {
            ++allowHeavyOperation;
        }
        ~AllowHeavyOperation()
        {
            --allowHeavyOperation;
            Js::VerifyCatastrophic(allowHeavyOperation>=0);
        }
    };

    __declspec(thread) extern bool weShouldNotBeParsingMetadata;
    __declspec(thread) extern int weAreParsingDelegateMetadata;

    class DisallowParsingMetadata
    {

    public:
        static void VerifyParsingAllowed()
        {
            AssertMsg(!weShouldNotBeParsingMetadata || /*We have a delegate parsing on stack*/ weAreParsingDelegateMetadata > 0, "WinRT Parsing has been blocked, but we are trying to parse.");
        }

        DisallowParsingMetadata(bool enabled)
        {
            AssertMsg(!weShouldNotBeParsingMetadata, "We should not be having nested undeferral?");
            if (enabled)
            {
                weShouldNotBeParsingMetadata = true;
            }
        }
        ~DisallowParsingMetadata()
        {
            weShouldNotBeParsingMetadata = false;
        }
    };

    class AllowParsingMetadata
    {
        bool oldWeShouldNotBeParsingMetadata;
    public:
        AllowParsingMetadata()
            :oldWeShouldNotBeParsingMetadata(weShouldNotBeParsingMetadata)
        {
            weShouldNotBeParsingMetadata = false;
        }
        ~AllowParsingMetadata()
        {
            weShouldNotBeParsingMetadata = oldWeShouldNotBeParsingMetadata;
        }
    };

    class ParsingDelegateMetadata
    {
    public:
        ParsingDelegateMetadata()
        {
            ++weAreParsingDelegateMetadata;
        }
        ~ParsingDelegateMetadata()
        {
            --weAreParsingDelegateMetadata;
        }
    };

#endif
}

