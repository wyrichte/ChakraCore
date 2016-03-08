//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2010 by Microsoft Corporation.  All rights reserved.
//
// Inflates .NET metadata so that it can be programmed.
//----------------------------------------------------------------------------

#pragma once

typedef __int32 MetadataStringId;
const MetadataStringId MetadataStringIdNil = 0;


#define UNAVAILABLE ((void *)-1)

namespace Metadata
{
    using namespace regex;
    class Assembly;

    struct IStringConverter
    {
        virtual MetadataStringId IdOfString(LPCWSTR sz) = 0;
        virtual LPCWSTR StringOfId(MetadataStringId id) = 0;
    };

    struct Type
    {
        byte typeCode;
        Type(byte typeCode) 
            : typeCode(typeCode) { }

        bool IsNumeric();
        bool IsFullyInstantiated() const;
        CorElementType GetCorElementType() const;
        static bool IsBasicTypeCode(CorElementType typeCode);
        bool IsBasicType();
    };

    struct Class : public Type
    {
        const mdToken classToken;
        Class(mdToken classToken) 
            : Type(ELEMENT_TYPE_CLASS), classToken(classToken) 
        { }
    };

    struct Value : public Type
    {
        const mdToken valueToken;
        Value(mdToken valueToken) 
            : Type(ELEMENT_TYPE_VALUETYPE), valueToken(valueToken) 
        {  }
    };

    struct ModOpt : public Type
    {
        const mdToken classToken;
        const Type * modded;
        ModOpt(mdToken classToken, Type * modded) 
            : Type(ELEMENT_TYPE_CMOD_OPT), classToken(classToken), modded(modded)
        {  }
    };

    struct ModReqd : public Type
    {
        const mdToken classToken;
        const Type * modded;
        ModReqd(mdToken classToken, Type * modded) 
            : Type(ELEMENT_TYPE_CMOD_REQD), classToken(classToken), modded(modded)
        {  }
    };

    struct SzArray : public Type
    {
        const Type * elementType;
        SzArray(Type * elementType) : Type(ELEMENT_TYPE_SZARRAY), elementType(elementType) 
        { }
    };

    struct MdArray : public Type
    {
        const Type * elementType;
        const int rank;
        const int boundsCount;
        const int * bounds;
        const int loCount;
        const int * lo;
        MdArray(Type * elementType, int rank, int boundsCount, int * bounds, int loCount, int * lo) 
            : Type(ELEMENT_TYPE_ARRAY), elementType(elementType), rank(rank), boundsCount(boundsCount), bounds(bounds), loCount(loCount), lo(lo)
        {  }
    };

    struct Ptr : public Type
    {
        const Type * pointedTo;
        Ptr(Type * pointedTo) 
            : Type(ELEMENT_TYPE_PTR), pointedTo(pointedTo) 
        { }
    };

    struct ByRef : public Type
    {
        const Type * pointedTo;
        ByRef(Type * pointedTo) 
            : Type(ELEMENT_TYPE_BYREF), pointedTo(pointedTo) 
        { }
    };

    struct TVar : public Type // Free generic variable
    {
        const int index;
        TVar(int index) 
            : Type(ELEMENT_TYPE_VAR), index(index)
        {  }
    };

    struct MVar : public Type // Generic parameter in a generic method definition, represented as number
    {
        const int index;
        MVar(int index) : Type(ELEMENT_TYPE_MVAR), index(index)
        {  }
    };

    struct GenericInstantiation : public Type // SomeType<Type1,Type2,etc>
    {
        const Type * parentType; // SomeType
        ImmutableList<const Type*> * genericParameters;
        GenericInstantiation(Type * parentType, ImmutableList<const Type*> * genericParameters) : 
            Type(ELEMENT_TYPE_GENERICINST), parentType(parentType), genericParameters(genericParameters)
        { }
    };

    struct AssemblyProperties
    {
        LPWSTR name;
        mdAssembly td;
        ULONG sizeName;
        LPCVOID publicKey;
        ULONG sizePublicKey;
        ULONG hashAlgorithmID;
        ASSEMBLYMETADATA metaData;
        DWORD flags;
        AssemblyProperties() {}
    };


    struct MethodProperties;
    struct PropertyProperties;
    struct EventProperties;
    struct ParameterProperties;
    
    struct TypeDefProperties
    {
#if DBG
        LPCWSTR typeName_Debug;
#endif
        MetadataStringId id;
        mdTypeDef td;
        DWORD flags;
        mdToken extends;
        ImmutableList<mdGenericParam> * genericParameterTokens;
        const Assembly & assembly;

    private:
        ImmutableList<const MethodProperties*> * methods;
        ImmutableList<const PropertyProperties*> * properties;
        ImmutableList<const EventProperties*> * events;

    public:
        TypeDefProperties(const Assembly & assembly) : 
            assembly(assembly), 
            methods((ImmutableList<const MethodProperties*> *)UNAVAILABLE),
            properties((ImmutableList<const PropertyProperties*> *)UNAVAILABLE),
            events((ImmutableList<const EventProperties*> *)UNAVAILABLE)
        { }

        bool IsNested() const
        {
            auto visibility = (CorTypeAttr)(flags&tdVisibilityMask);
            switch(visibility)
            {
            case tdNestedPublic:
            case tdNestedPrivate:
            case tdNestedFamily:
            case tdNestedAssembly:
            case tdNestedFamANDAssem:
            case tdNestedFamORAssem: return TRUE;
            default: return FALSE;
            }
        }

        bool IsPublic() const
        {
            auto visibility = (CorTypeAttr)(flags&tdVisibilityMask);
            switch(visibility)
            {
            case tdPublic:
            case tdNestedPublic: return TRUE;
            default: return FALSE;
            }
        }

        bool IsClass() const
        {
            auto semantics = (CorTypeAttr)(flags&tdClassSemanticsMask);
            return semantics==tdClass;
        }

        bool IsWindowsRuntime() const
        {
            DWORD tdWindowsRuntime = 0x00004000;
            return (flags&tdWindowsRuntime)==tdWindowsRuntime;

        }

        ImmutableList<const MethodProperties*> * GetMethods() const;
        const MethodProperties * GetMethodByName(MetadataStringId nameId, MetadataStringId overloadNameId) const;
        const MethodProperties * GetMethodByToken(mdMethodDef md) const;
        ImmutableList<const PropertyProperties*> * GetProperties() const;
        ImmutableList<const EventProperties*> * GetEvents() const;
    };

    struct GenericParameterProperties
    {
        MetadataStringId id;
        mdGenericParam gp;
        ULONG sequence;
        DWORD flags;
        const Assembly & assembly;

        GenericParameterProperties(const Assembly & assembly) 
            : assembly(assembly) 
        { }
    };
    
    struct MemberProperties
    {
        mdToken mt;
        mdTypeDef classToken;
        MetadataStringId id;
        DWORD flags;
        const Assembly & assembly;

        MemberProperties(const Assembly & assembly) 
            : assembly(assembly) 
        { }

        bool IsField() const 
        {
            return TypeFromToken(mt) == mdtFieldDef;
        }

        BOOL IsStaticField() const 
        {
            Assert(IsField());
            return IsFdStatic(flags);
        }

        BOOL IsLiteralField() const 
        {
            Assert(IsField());
            return IsFdLiteral(flags);
        }

        BOOL IsSpecialNameField() const 
        {
            Assert(IsField());
            return IsFdSpecialName(flags);
        }

        BOOL IsRTSpecialNameField() const 
        {
            Assert(IsField());
            return IsFdRTSpecialName(flags);
        }     

        bool IsMethod() const 
        {
            return TypeFromToken(mt) == mdtMethodDef;
        }

        BOOL IsStaticMethod() const
        {
            Assert(IsMethod());
            return IsMdStatic(flags);
        }

        bool IsStaticConstructor() const;
        bool IsConstructor() const;
    };

    struct EventProperties
    {
        MetadataStringId id;            
        mdEvent ev;             
        mdTypeDef classToken;     
        DWORD flags;     
        mdToken eventType;      
        mdMethodDef addOn;          
        const MethodProperties * addOnMethod;
        mdMethodDef removeOn;       
        const MethodProperties * removeOnMethod;
        mdMethodDef fire;           
        const Assembly & assembly;

        EventProperties(const Assembly & assembly) 
            : assembly(assembly) 
        { }
    };

    struct CustomAttributeProperties
    {
        mdCustomAttribute attributeToken;
        MetadataStringId attributeTypeId;
        mdToken scopeToken;
        mdToken attributeTypeToken;
        const void * blob;
        ULONG blobSize;
        const Assembly & assembly;
        bool isMemberRef;
        PCCOR_SIGNATURE pSig;
        ulong sizeSig;

        CustomAttributeProperties(const Assembly & assembly) 
            : assembly(assembly) 
        { }
    };

    struct Parameter
    {
        bool byref;
        bool typedByRef;
        Option<Type> type; // This will be empty when typedByRef==true
        Parameter(Type * type,bool byref,bool typedByRef) 
            : byref(byref), typedByRef(typedByRef), type(type) {}
    };

    struct MethodDefSig
    {
        bool hasThis;
        bool explicitThis;
        bool defaultCall;
        bool varargCall;
        bool genericCall;
        int genericParameterCount;
        Type * returnType;
        int parameterCount;
        ImmutableList<const Parameter*> * parameters;
        MethodDefSig() : 
            hasThis(false), explicitThis(false), defaultCall(false), varargCall(false), genericCall(false), 
            genericParameterCount(0), parameterCount(0), returnType(nullptr), parameters(nullptr) {}
    };

    struct MethodProperties
    {
        MetadataStringId id;
        mdToken mb;
        mdTypeDef classToken;
        DWORD flags;
        MethodDefSig * signature;
        int methodIndex;
        bool isDefaultOverload;
        MetadataStringId overloadNameId;
        ImmutableList<const ParameterProperties*> * parameters;
        const Assembly & assembly;

        MethodProperties(const Assembly & assembly) : 
            assembly(assembly) ,
            parameters((ImmutableList<const ParameterProperties*> *)UNAVAILABLE)
        { }

        BOOL IsPublic() const 
        {
            return IsMdPublic(flags);
        }

        BOOL IsStatic() const
        {
            return IsMdStatic(flags);
        }

        bool IsStaticConstructor() const;

        bool IsConstructor() const; 

        BOOL IsSpecialName() const
        {
            return IsMdSpecialName(flags);
        }
        
        ImmutableList<const ParameterProperties*> * GetParameters() const;
    };

    struct MemberRefProperties
    {
        MetadataStringId id;
        mdMemberRef mr;
        mdToken classToken;
        PCCOR_SIGNATURE pvSig;
        ulong sizeSig;

        const Assembly & assembly;

        MemberRefProperties(const Assembly & assembly) 
            : assembly(assembly) 
        { }
    };

    struct FieldProperties
    {
        MetadataStringId id;
        mdTypeDef classToken;
        mdFieldDef fieldToken;
        DWORD flags;
        DWORD dwCPlusTypeFlag;
        UVCP_CONSTANT constantValue;
        Type * type;
        const Assembly & assembly;

        FieldProperties(const Assembly & assembly) 
            : assembly(assembly) 
        { }

        BOOL IsLiteral() const
        {
            return IsFdLiteral(flags);
        }

        BOOL IsPublic() const
        {
            return IsFdPublic(flags);
        }

    };

    struct PropertyProperties
    {
        MetadataStringId id;
        mdTypeDef classToken;
        mdProperty propertyToken;
        DWORD flags;
        mdMethodDef setter;
        const MethodProperties * setterMethod;
        mdMethodDef getter;
        const MethodProperties * getterMethod;
        bool hasThis;
        int paramCount;
        Type * type;
        ImmutableList<const Parameter*> * parameters;
        const Assembly & assembly;

        PropertyProperties(const Assembly & assembly) 
            : assembly(assembly) 
        { }
    };

    struct TypeRefProperties
    {
        MetadataStringId id;
        mdTypeRef referenceToken;
        mdToken resolutionScope;
        const Assembly & assembly;

        TypeRefProperties(const Assembly & assembly) 
            : assembly(assembly) 
        { }
    };

    struct ParameterProperties
    {
        MetadataStringId id;
        mdParamDef pt;
        mdMethodDef methodToken;
        ULONG sequence;
        DWORD flags;
        const Assembly & assembly;

        ParameterProperties(const Assembly & assembly) 
            : assembly(assembly) 
        { }

        bool IsIn() const
        {
            return IsPdIn(flags) ? true : false;
        }
        bool IsOut() const
        {
            return IsPdOut(flags) ? true : false;
        }
    };

    struct InterfaceImplProperties
    {
        mdInterfaceImpl implToken;
        mdTypeDef classToken;
        mdToken interfaceToken;
        const Assembly & assembly;

        InterfaceImplProperties(const Assembly & assembly) 
            : assembly(assembly) 
        { }
    };

    struct MethodImpl
    {
        mdToken methodBody;
        mdToken methodDecl;
        const Assembly & assembly;
        MethodImpl(const Assembly & assembly) 
            : assembly(assembly) 
        { }
    };

    struct TypeRefList
    {
        mdTypeRef token;
        TypeRefList *next;

        TypeRefList(): token(mdTypeRefNil), next(nullptr) { }
    };

#define PrologBytes 0x0001
#define EnumFieldBytes  0x5553

    class Assembly
    {        
        friend struct TypeDefProperties;
        friend struct MethodProperties;
        
        typedef JsUtil::BaseDictionary<mdToken, void *, ArenaAllocator, PrimeSizePolicy> TOKENMAP;
        TOKENMAP * tokenCache;

        ArenaAllocator * a;
        
        IMetaDataAssemblyImport * metadata;
        static ULONG DecodeField(__in ULONG sigSize, __in_bcount(sigSize) PCCOR_SIGNATURE psig, __in ArenaAllocator * a, __out Type **pp);
        static ULONG DecodeProperty(__in ULONG sigSize, __in_bcount(sigSize) PCCOR_SIGNATURE psig, __in ArenaAllocator * a, __out bool * hasThis, __out int * paramCount, __out Type **type, __out ImmutableList<const Parameter*> ** parameters);
        static ULONG DecodeParameter(__in ULONG sigSize, __in_bcount(sigSize) PCCOR_SIGNATURE psig, __in ArenaAllocator * a, __out Parameter **pp);
        static ULONG DecodeParameters(__in ULONG sigSize, __in_bcount(sigSize) PCCOR_SIGNATURE psig, __in ArenaAllocator * a, __in int parameterCount, __out ImmutableList<const Parameter*> ** parameters);
        static ULONG DecodeMethodDefSig(__in ULONG sigSize, __in_bcount(sigSize) PCCOR_SIGNATURE psig, __in ArenaAllocator * a, __out MethodDefSig **pp);
        static ULONG DecodeType(__in ULONG sigSize, __in_bcount(sigSize) PCCOR_SIGNATURE psig, __in ArenaAllocator * a, __out Type **pp);
        static ULONG DecodeGenericInstantiation(__in ULONG sigSize, __in_bcount(sigSize) PCCOR_SIGNATURE pb, __in ArenaAllocator * a, __out GenericInstantiation **pp);

        TypeRefList systemGuid;
        TypeRefList windowsFoundationDateTime;
        TypeRefList windowsFoundationTimeSpan;
        TypeRefList systemValueType;
        TypeRefList systemEnum;
        TypeRefList systemDelegate;
        TypeRefList systemAttribute;
        TypeRefList systemObject;
        TypeRefList windowsFoundationHResult;
        TypeRefList windowsFoundationActivatableAttribute;
        TypeRefList windowsFoundationEventRegistrationToken;

        mdTypeDef typeDefWindowsFoundationHResult;
        mdTypeDef typeDefWindowsFoundationEventRegistrationToken;
        mdTypeDef typeDefWindowsFoundationIAsyncInfo;
        mdTypeDef typeDefWindowsFoundationDateTime;
        mdTypeDef typeDefWindowsFoundationTimeSpan;
       
        bool IsParticularTypeRefInChain(__in mdToken tr, __in const TypeRefList *compareTo) const
        {
            for ( ; compareTo; compareTo = compareTo->next)
                if (tr == compareTo->token) return true;
            return false;
        }

        void RecordReference(__in mdToken tr, TypeRefList &reference, __in ArenaAllocator * a)
        {
            if (reference.token == mdTypeRefNil)
            {
                reference.token = tr;
                Assert(!reference.next);
            }
            else 
            {
                Assert(a);
                auto ref = Anew(a, TypeRefList);
                ref->token = tr;
                ref->next = reference.next;
                reference.next = ref;
            }
        }

#ifdef PROJECTION_METADATA_TRACE
        void Trace(const char16 *form, ...) const
        {
            if (Js::Configuration::Global.flags.Trace.IsEnabled(Js::ProjectionMetadataPhase))
            {
                va_list argptr;
                va_start(argptr, form);
                Output::Print(_u("WinMD: %s: "), properties->name);
                Output::VPrint(form, argptr);
                Output::Flush();
            }
        }
#endif

    private:
        IMetaDataImport2 * realimport;
        IMetaDataImport2 * GetImport() const { return realimport; } 
        
    public:
        AssemblyProperties * properties;
        IStringConverter * stringConverter;
        bool isVersioned;

        Assembly(__in IMetaDataImport2 * import, __in IStringConverter * stringConverter, __in ArenaAllocator * a, bool isVersioned = false); 

        
        const TypeDefProperties * FindTopLevelTypeDefByName(LPCWSTR name) const;
        ImmutableList<const TypeDefProperties*> * TypeDefinitions() const;
        
        const Type * GetTypeSpec(__in mdTypeSpec ts) const;

        const TypeDefProperties * GetTypeDefProperties(__in mdTypeDef td) const;
        const InterfaceImplProperties * GetInterfaceImplProperties(__in mdInterfaceImpl ii) const;
        ImmutableList<mdInterfaceImpl> * InterfacesImplemented(__in mdTypeDef td) const;

        ImmutableList<const MethodImpl*> * Assembly::MethodImpls(__in mdTypeDef td) const;

        const MethodProperties* GetMethodProperties2(__in mdMethodDef mb) const;
        
        ImmutableList<const MemberProperties*> * Members(__in mdTypeDef td) const;
        ImmutableList<const GenericParameterProperties*> * GenericParameters(__in ImmutableList<mdGenericParam> *genericParamTokens) const;

        ImmutableList<const FieldProperties*> * Fields(__in mdTypeDef mt) const;
        const FieldProperties * GetFieldProperties(__in mdFieldDef fd) const;
        
        const MemberRefProperties* GetMemberRefProperties(__in mdMemberRef mr) const;

        ImmutableList<const CustomAttributeProperties*> * CustomAttributes(__in mdToken td, __in mdToken typeOfInterest) const;

        const TypeRefProperties * GetTypeRefProperties(__in mdTypeRef tr) const;

        void GetGuidAttributeValue(__in mdToken classToken, __in LPCWSTR attributeName, __out GUID & guid) const;        
        bool IsAttributePresent(__in mdToken token, __in LPCWSTR attributeName) const;
        
        LPCWSTR GetStringAttribute(__in mdToken token, __in LPCWSTR attributeName) const;
        bool GetInt32Attribute(__in mdToken token, __in LPCWSTR attributeName, INT32 &attributeValue) const;
        bool HasContractAttribute(__in mdToken token) const;
        bool GetDWORDAttribute(__in mdToken token, __in LPCWSTR attributeName, DWORD &attributeValue) const;
        mdTypeDef TryGetTypeByName(PCWSTR fullTypeName);

        
        void Uninitialize();
        
        ULONG VerifyNextInt16OfAttribute(ULONG dataSize, const byte* data, __int16 expectedValue) const;
        ULONG VerifyAttributeString(ULONG dataSize, const byte* data, ULONG * strLen) const;
        void VerifySimpleAttributeBytes(ULONG dataSize, const byte* data, ULONG expectedValueSize) const;
        static size_t GetBasicTypeSize(__in CorElementType typeCode);
        static size_t GetBasicTypeAlignment(__in CorElementType typeCode);
        
        // Return:      True, if tr is a type ref and it is equal to compareTo
        bool IsParticularTypeRef(__in mdToken tr, __in const TypeRefList& compareTo) const
        {
            return (mdtTypeRef & tr) == mdtTypeRef && 
                compareTo.token!=mdTypeRefNil 
                && (compareTo.token==tr || (compareTo.next && IsParticularTypeRefInChain(tr, compareTo.next)));
        }

        // Return:      True, if this is a value type
        bool IsValueTypeRef(__in mdTypeRef tr) const 
        {
            return IsParticularTypeRef(tr, systemValueType);
        }

        // Return:      True, if td is a type def and it is equal to compareTo
        bool IsParticularTypeDef(__in mdToken td, __in mdTypeDef compareTo) const
        {
            return (mdtTypeDef & td) == mdtTypeDef && 
                compareTo!=mdTypeDefNil 
                && compareTo==td;
        }

        // Return:      True, if this is an enum type
        bool IsEnumTypeRef(__in mdTypeRef tr) const 
        {
            return IsParticularTypeRef(tr, systemEnum);
        }

        // Return:      True, if this is a delegate type
        bool IsDelegateTypeRef(__in mdTypeRef tr) const 
        {
            return IsParticularTypeRef(tr, systemDelegate);
        }

        // Return:      True, if this is an attribute type
        bool IsAttributeTypeRef(__in mdTypeRef tr) const 
        {
            return IsParticularTypeRef(tr, systemAttribute);
        }

        // Return:      True, if this is an object type 
        bool IsObjectTypeRef(__in mdTypeRef tr) const 
        {
            return IsParticularTypeRef(tr, systemObject);
        }

        // Return:      True, if this is an hresult type 
        bool IsWindowsFoundationHResultTypeRef(__in mdTypeRef tr) const 
        {
            return IsParticularTypeRef(tr, windowsFoundationHResult);
        }

        // Return:      True, if this is a System.Guid type 
        bool IsSystemGuidTypeRef(__in mdTypeRef tr) const 
        {
            return IsParticularTypeRef(tr, systemGuid);
        }

        bool IsWindowsFoundationDateTimeTypeRef(__in mdTypeRef tr) const
        {
            return IsParticularTypeRef(tr, windowsFoundationDateTime);
        }

        bool IsWindowsFoundationTimeSpanTypeRef(__in mdTypeRef tr) const
        {
            return IsParticularTypeRef(tr, windowsFoundationTimeSpan);
        }

        // Return:      True, if this is a Windows.Foundation.Metadata.ActivatableAttribute type 
        bool IsActivatableTypeRef(__in mdTypeRef tr) const 
        {
            return IsParticularTypeRef(tr, windowsFoundationActivatableAttribute);
        }

        // Return:      True, if this is a Windows.Foundation.EventRegistrationToken type 
        bool IsWindowsFoundationEventRegistrationTokenTypeRef(__in mdTypeRef tr) const 
        {
            return IsParticularTypeRef(tr, windowsFoundationEventRegistrationToken);
        }

        // Return:      True, if this is an hresult type 
        bool IsWindowsFoundationHResultTypeDef(__in mdTypeDef td) const 
        {
            return IsParticularTypeDef(td, typeDefWindowsFoundationHResult);
        }

        // Return:      True, if this is a Windows.Foundation.EventRegistrationToken type 
        bool IsWindowsFoundationEventRegistrationTokenTypeDef(__in mdTypeDef td) const 
        {
            return IsParticularTypeDef(td, typeDefWindowsFoundationEventRegistrationToken);
        }

        // Return:      True, if this is a Windows.Foundation.IAsyncInfo type 
        bool IsIAsyncInfoTypeDef(__in mdTypeDef td) const 
        {
            return IsParticularTypeDef(td, typeDefWindowsFoundationIAsyncInfo);
        }
       
        bool IsWindowsFoundationDateTimeTypeDef(__in mdTypeDef tr) const
        {
            return IsParticularTypeDef(tr, typeDefWindowsFoundationDateTime);
        }

        bool IsWindowsFoundationTimeSpanTypeDef(__in mdTypeDef tr) const
        {
            return IsParticularTypeDef(tr, typeDefWindowsFoundationTimeSpan);
        }

    private:
        ImmutableList<const MethodProperties*> * Methods(__in mdTypeDef mt) const;
        
        ImmutableList<mdAssemblyRef> * ReferenceTokens() const;
        ImmutableList<mdTypeDef> * TypeDefinitionTokens() const;
        ImmutableList<mdToken> * MemberTokens(__in mdTypeDef td) const;
        ImmutableList<mdGenericParam> * GenericParameterTokens(__in mdToken token) const;
        const GenericParameterProperties * GetGenericParameterProperties(__in mdGenericParam gp) const;
        ImmutableList<mdFieldDef> * FieldTokens(__in mdTypeDef td) const;
        ImmutableList<mdMethodDef> * MethodTokens(__in mdTypeDef td) const;
        ImmutableList<mdToken> * EventTokens(__in mdTypeDef td) const;        
        AssemblyProperties * GetAssemblyProperties(__in mdAssembly td) const;
        ImmutableList<mdProperty> * PropertyTokens(__in mdTypeDef td) const;
        const MemberProperties * GetMemberProperties(__in mdToken mt) const;
        const EventProperties * GetEventProperties2(__in mdEvent mt, __in const TypeDefProperties * type) const;
        const PropertyProperties* GetPropertyProperties2(__in mdProperty pt, __in const TypeDefProperties * type) const;        
        MethodProperties* GetMethodProperties(__in mdMethodDef mb) const;
        ImmutableList<mdCustomAttribute> * CustomAttributeTokens(__in mdToken td, __in mdToken typeOfInterest) const;
        const CustomAttributeProperties* GetCustomAttributeProperties(__in mdCustomAttribute cv) const;
        mdAssembly GetAssemblyToken() const;
        const ParameterProperties* GetParameterProperties(__in mdParamDef pt) const;
        ImmutableList<mdParamDef> * ParameterTokens(__in mdMethodDef mb) const;

    };
}
