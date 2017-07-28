//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2010 by Microsoft Corporation.  All rights reserved.
//---------------------------------------------------------------------------

#include "WinRTPch.h"
#include "Metadata.h"

#ifdef PROJECTION_METADATA_TRACE
#define TRACE_METADATA(...) { Trace(__VA_ARGS__); }
#else
#define TRACE_METADATA(...)
#endif

namespace Metadata
{
    using namespace JsUtil;
    using namespace Js;

    // Info:        Determines whether this is a numeric type
    bool Type::IsNumeric()
    {
        switch(typeCode)
        {
        default:
            return false;
        case ELEMENT_TYPE_U1:    
        case ELEMENT_TYPE_I2:    
        case ELEMENT_TYPE_U2:    
        case ELEMENT_TYPE_I4:    
        case ELEMENT_TYPE_U4:    
        case ELEMENT_TYPE_I8:    
        case ELEMENT_TYPE_U8:    
        case ELEMENT_TYPE_R4:    
        case ELEMENT_TYPE_R8:
        case ELEMENT_TYPE_I:
        case ELEMENT_TYPE_U:
            return true;
        }
    }

    // Info:        Determines whether this is a fully instantiated type
    bool Type::IsFullyInstantiated() const
    {
        switch(typeCode)
        {
        case ELEMENT_TYPE_VALUETYPE: 
        case ELEMENT_TYPE_CLASS:
        case ELEMENT_TYPE_CHAR:
        case ELEMENT_TYPE_BOOLEAN:
        case ELEMENT_TYPE_U1:
        case ELEMENT_TYPE_I2:
        case ELEMENT_TYPE_U2:
        case ELEMENT_TYPE_I4:
        case ELEMENT_TYPE_U4:
        case ELEMENT_TYPE_I8:
        case ELEMENT_TYPE_U8:
        case ELEMENT_TYPE_STRING:
        case ELEMENT_TYPE_R4:
        case ELEMENT_TYPE_R8:
        case ELEMENT_TYPE_SZARRAY:
            return true;
        case ELEMENT_TYPE_GENERICINST: {
            auto t = static_cast<const GenericInstantiation*>(this);
            if (!t->parentType->IsFullyInstantiated())
            {
                return false;
            }
            auto allParametersInstantiated = t->genericParameters->Accumulate(true, [&](bool soFar, const Type * type) {
                return soFar && type->IsFullyInstantiated();
            });

            return allParametersInstantiated; 
        }
        case ELEMENT_TYPE_CMOD_OPT: {
            auto t = static_cast<const ModOpt*>(this);
            return t->modded->IsFullyInstantiated();
        }
        case ELEMENT_TYPE_BYREF: {
            auto t = static_cast<const ByRef*>(this);
            return t->pointedTo->IsFullyInstantiated();
        }
        case ELEMENT_TYPE_VAR:
            return false;
        default:
            Assert(0);
            return 0;
        }
    }

    // Info:        Get the cor element type
    CorElementType Type::GetCorElementType() const
    {
        return (CorElementType)typeCode;
    }

    // Info:        Determine whether this is a basic type code
    // Parameters:  typeCode
    bool Type::IsBasicTypeCode(CorElementType typeCode)
    {
        switch(typeCode)
        {
        default:
            return false;
        case ELEMENT_TYPE_U1:    
        case ELEMENT_TYPE_I2:    
        case ELEMENT_TYPE_U2:    
        case ELEMENT_TYPE_I4:    
        case ELEMENT_TYPE_U4:    
        case ELEMENT_TYPE_I8:    
        case ELEMENT_TYPE_U8:    
        case ELEMENT_TYPE_R4:    
        case ELEMENT_TYPE_R8:
        case ELEMENT_TYPE_I:
        case ELEMENT_TYPE_U:
        case ELEMENT_TYPE_STRING:
        case ELEMENT_TYPE_CHAR:
        case ELEMENT_TYPE_BOOLEAN:
        case ELEMENT_TYPE_OBJECT:
            return true;
        }
    }

    // Info:        Determine whether this is a basic type
    bool Type::IsBasicType()
    {
        auto typeCode = GetCorElementType();
        return IsBasicTypeCode(typeCode);
    }

    bool MemberProperties::IsStaticConstructor() const
    {
        return IsMdClassConstructorW(flags, assembly.stringConverter->StringOfId(id));
    }

    bool MemberProperties::IsConstructor() const 
    {
        return IsMdInstanceInitializerW(flags, assembly.stringConverter->StringOfId(id));
    }

    bool MethodProperties::IsStaticConstructor() const
    {
        return IsMdClassConstructorW(flags, assembly.stringConverter->StringOfId(id));
    }

    bool MethodProperties::IsConstructor() const 
    {
        return IsMdInstanceInitializerW(flags, assembly.stringConverter->StringOfId(id));
    }

    template< size_t N >
    inline bool EqualsLiteral(const char *name, const char (&w)[N]) { return ::strncmp(name, w, N) == 0; }

    template< size_t N >
    inline bool EqualsLiteral(const char16 *name, const char16 (&w)[N]) { return ::wcsncmp(name, w, N) == 0; }

    // Info:        Construct an assembly reader
    // Parameters:  import - The metadata importer
    //              a - The allocator to use
    Assembly::Assembly(__in IMetaDataImport2 * import, __in IStringConverter * stringConverter, __in ArenaAllocator * a, bool isVersioned) : 
        a(a), realimport(import), stringConverter(stringConverter), isVersioned(isVersioned)
    {
        HRESULT hr = GetImport()->QueryInterface(IID_IMetaDataAssemblyImport, (void**)&metadata); 
        Js::VerifyOkCatastrophic(hr);

        tokenCache = Anew(a, TOKENMAP, a);
        
        auto token = GetAssemblyToken();
        properties = GetAssemblyProperties(token);

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        if (Js::Configuration::Global.flags.EnableVersioningAllAssemblies)
        {
            this->isVersioned = true;
        }
#endif

        // Find type refs of concern
        const auto chunkSize = 128;
        mdTypeRef refs[chunkSize];
        auto count = 0uL;
        HCORENUM he = nullptr;

        while ((hr = GetImport()->EnumTypeRefs(&he, refs, chunkSize, &count)) == S_OK && count > 0)
        {
            for (auto i = 0uL; i < count; i++)
            {
                mdTypeRef typeRef = refs[i];
                
                MDUTF8CSTR name;
                hr = GetImport()->GetNameFromToken(typeRef, &name);
                Js::VerifyOkCatastrophic(hr);
                
                bool possible = false;
                switch (name[0])
                {
                case 'A':
                    switch (name[1])
                    {
                    case 'c':
                        possible = EqualsLiteral(name, "ActivatableAttribute");
                        break;
                    case 't':
                        possible = EqualsLiteral(name, "Attribute");
                        break;
                    }
                    break;
                case 'D':
                    possible = EqualsLiteral(name, "DateTime");
                    break;
                case 'E':
                    switch (name[1])
                    {
                    case 'n':
                        possible = EqualsLiteral(name, "Enum");
                        break;
                    case 'v':
                        possible = EqualsLiteral(name, "EventRegistrationToken");
                        break;
                    }
                    break;
                case 'G':
                    possible = EqualsLiteral(name, "Guid");
                    break;
                case 'H':
                    possible = EqualsLiteral(name, "HResult");
                    break;
                case 'M':
                    possible = EqualsLiteral(name, "MulticastDelegate");
                    break;
                case 'O':
                    possible = EqualsLiteral(name, "Object");
                    break;
                case 'T':
                    possible = EqualsLiteral(name, "TimeSpan");
                    break;
                case 'V':
                    possible = EqualsLiteral(name, "ValueType");
                    break;
                }

                if (possible)
                {
                    const ULONG MaxRefSize = 50;
                    char16 referenceName[MaxRefSize + 1];
                    ULONG nameSize;
                    mdToken referenceScope;
                    hr = GetImport()->GetTypeRefProps(typeRef, &referenceScope, referenceName, MaxRefSize, &nameSize);
                    Js::VerifyOkCatastrophic(hr);
                
                    // nameSize includes the null terminator
                    switch (nameSize) 
                    {
                    case 12:
                        switch (referenceName[7]) 
                        {
                        case 'E':
                            if (EqualsLiteral(referenceName, _u("System.Enum")))
                                RecordReference(typeRef, systemEnum, a);
                            break;
                        case 'G':
                            if (EqualsLiteral(referenceName, _u("System.Guid"))) 
                                RecordReference(typeRef, systemGuid, a);
                            break;
                        }
                        break;
            
                    case 14:
                        if (EqualsLiteral(referenceName, _u("System.Object")))
                            RecordReference(typeRef, systemObject, a);
                        break;

                    case 17:
                        switch (referenceName[7])
                        {
                        case 'A':
                            if (EqualsLiteral(referenceName, _u("System.Attribute")))
                                RecordReference(typeRef, systemAttribute, a);
                            break;
                        case 'V':
                            if (EqualsLiteral(referenceName, _u("System.ValueType")))
                                RecordReference(typeRef, systemValueType, a);
                            break;
                        }
                        break;

                    case 25:
                        if (EqualsLiteral(referenceName, _u("System.MulticastDelegate")))
                            RecordReference(typeRef, systemDelegate, a);
                        break;

                    case 27:
                        if (EqualsLiteral(referenceName, _u("Windows.Foundation.HResult")))
                            RecordReference(typeRef, windowsFoundationHResult, a);
                        break;

                    case 28:
                        switch (referenceName[19]) 
                        {
                        case 'D':
                            if (EqualsLiteral(referenceName, _u("Windows.Foundation.DateTime")))
                                RecordReference(typeRef, windowsFoundationDateTime, a);
                            break;
                        case 'T':
                            if (EqualsLiteral(referenceName, _u("Windows.Foundation.TimeSpan")))
                                RecordReference(typeRef, windowsFoundationTimeSpan, a);
                            break;
                        }
                        break;
            
                    case 42:
                        if (EqualsLiteral(referenceName, _u("Windows.Foundation.EventRegistrationToken")))
                            RecordReference(typeRef, windowsFoundationEventRegistrationToken, a);
                        break;

                    case 49:
                        if (EqualsLiteral(referenceName, _u("Windows.Foundation.Metadata.ActivatableAttribute")))
                            RecordReference(typeRef, windowsFoundationActivatableAttribute, a);
                        break;
                    }
                }
            }
        }

        Js::VerifyOkCatastrophic(hr);

        metadata->CloseEnum(he);

#ifdef DEBUG
        // Verify that FindTypeRef returns one of the tokens found by EnumTypeRef.
        auto verifyTypeRef=[&](mdToken resolutionScope, LPCWSTR typeName, TypeRefList& typeRef) {
            mdTypeRef foundToken;
            auto hr = GetImport()->FindTypeRef(resolutionScope, typeName, &foundToken);
            if (CLDB_E_RECORD_NOTFOUND!=hr) // The error code means the type isn't in the given assembly.               
            {
                Js::VerifyOkCatastrophic(hr);
                Assert(IsParticularTypeRef(foundToken, typeRef));
            }
        };

        auto referenceTokens = ReferenceTokens();

        referenceTokens->Iterate([&](mdToken assembly) {
            verifyTypeRef(assembly, _u("System.Guid"), systemGuid);
            verifyTypeRef(assembly, _u("Windows.Foundation.DateTime"), windowsFoundationDateTime);
            verifyTypeRef(assembly, _u("Windows.Foundation.TimeSpan"), windowsFoundationTimeSpan);
            verifyTypeRef(assembly, _u("System.Enum"), systemEnum);
            verifyTypeRef(assembly, _u("System.ValueType"), systemValueType);
            verifyTypeRef(assembly, _u("System.MulticastDelegate"), systemDelegate);
            verifyTypeRef(assembly, _u("System.Attribute"), systemAttribute);
            verifyTypeRef(assembly, _u("System.Object"), systemObject);
            verifyTypeRef(assembly, _u("Windows.Foundation.HResult"), windowsFoundationHResult); 
            verifyTypeRef(assembly, _u("Windows.Foundation.Metadata.ActivatableAttribute"), windowsFoundationActivatableAttribute); 
            verifyTypeRef(assembly, _u("Windows.Foundation.EventRegistrationToken"), windowsFoundationEventRegistrationToken); 
        });
#endif

        // Find type defs of concern
        typeDefWindowsFoundationHResult = mdTypeDefNil;
        GetImport()->FindTypeDefByName(_u("Windows.Foundation.HResult"), NULL, &typeDefWindowsFoundationHResult);
        typeDefWindowsFoundationEventRegistrationToken = mdTypeDefNil;
        GetImport()->FindTypeDefByName(_u("Windows.Foundation.EventRegistrationToken"), NULL, &typeDefWindowsFoundationEventRegistrationToken);
        typeDefWindowsFoundationIAsyncInfo = mdTypeDefNil;
        GetImport()->FindTypeDefByName(_u("Windows.Foundation.IAsyncInfo"), NULL, &typeDefWindowsFoundationIAsyncInfo);
        typeDefWindowsFoundationDateTime = mdTypeDefNil;
        GetImport()->FindTypeDefByName(_u("Windows.Foundation.DateTime"), NULL, &typeDefWindowsFoundationDateTime);
        typeDefWindowsFoundationTimeSpan = mdTypeDefNil;
        GetImport()->FindTypeDefByName(_u("Windows.Foundation.TimeSpan"), NULL, &typeDefWindowsFoundationTimeSpan);
    }

    // Info:        Release hed resources.
    void Assembly::Uninitialize()
    {
        if(nullptr!=metadata)
        {
            metadata->Release();
            metadata=nullptr;
            realimport=nullptr;
            a=nullptr;
        }
    }

    // Info:        Decode the given field
    // Parameters:  sigSize - the size, in bytes, of the remaining signature
    //              psig - the signature
    //              pp - receives the result
    // Returns:     The number of bytes read
    ULONG Assembly::DecodeField(__in ULONG sigSize, __in_bcount(sigSize) PCCOR_SIGNATURE psig, __in ArenaAllocator * a, __in Type **pp) 
    {
        BYTE * pb = (BYTE*)psig;
        Js::VerifyCatastrophic(pb[0] == IMAGE_CEE_CS_CALLCONV_FIELD);
        return DecodeType(sigSize-1, pb + 1, a, pp) + 1;
    }

    // Info:        Decode the given properties
    // Parameters:  sigSize - the size, in bytes, of the remaining signature
    //              psig - the signature
    //              pp - receives the result
    // Returns:     The number of bytes read
    ULONG Assembly::DecodeProperty(__in ULONG sigSize, __in_bcount(sigSize) PCCOR_SIGNATURE psig, ArenaAllocator * a, __out bool * hasThis, __out int * paramCount, __out Type **type, __out ImmutableList<const Parameter*> ** parameters) 
    {
        auto pb = (BYTE*)psig;
        ULONG cb = 0;
        if (pb[0] == (IMAGE_CEE_CS_CALLCONV_HASTHIS|IMAGE_CEE_CS_CALLCONV_PROPERTY))
        {
            *hasThis = true;
        }
        else if (pb[0] == IMAGE_CEE_CS_CALLCONV_PROPERTY)
        {
            *hasThis = false;
        }
        else
        {
            Js::Throw::FatalProjectionError();
        }
        ++cb;

        if (cb<sigSize)
        {
            *paramCount = *(pb+cb);
            ++cb;

            cb+=DecodeType(sigSize-cb,pb+cb,a,type);

            cb+=DecodeParameters(sigSize-cb,pb+cb,a,*paramCount,parameters);			

            Js::VerifyCatastrophic(cb==sigSize);
        }
        else
        {
            Js::Throw::FatalProjectionError();
        }
        return cb;
    }

    // Info:        Decode the given generic instantiation
    // Parameters:  sigSize - the size, in bytes, of the remaining signature
    //              psig - the signature
    //              pp - receives the result
    // Returns:     The number of bytes read
    ULONG Assembly::DecodeGenericInstantiation(__in ULONG sigSize, __in_bcount(sigSize) PCCOR_SIGNATURE pb, __in ArenaAllocator * a, __out GenericInstantiation **pp) 
    {
        ULONG cb = 0;
        ++cb;
        Type * parentType = nullptr;
        cb += DecodeType(sigSize-cb,pb+cb,a,&parentType);

        Js::VerifyCatastrophic(cb<sigSize);
        int genericParameterCount = *(pb+cb);
        ++cb;

        auto parms = ImmutableList<const Type*>::Empty();
        auto tail = parms;
        for(int i=0;i<genericParameterCount;++i)
        {
            Type * parm = nullptr;
            cb+=DecodeType(sigSize-cb,pb+cb,a,&parm);
            parms = parms->Append(parm, a, &tail);
        }
        *pp  = Anew(a, GenericInstantiation, parentType, parms);
        return cb;
    }

    // Info:        Given a PCCOR_SIGNATURE for a type, decode it into a Type instance.
    // Parameters:  psig - The signature blob
    //              a - The allocator to use
    //              pp - Receives the typ instance.
    // Return:      The number of bytes consumed from the blob
    ULONG Assembly::DecodeType(__in ULONG sigSize, __in_bcount(sigSize) PCCOR_SIGNATURE psig, __in ArenaAllocator * a, __out Type **pp) 
    {
        // This function handles section 23.2.12 Type of
        // http://www.ecma-international.org/publications/files/ECMA-ST/Ecma-335.pdf
            
        BYTE * pb = (BYTE*)psig;
        ULONG cb = 0;
        *pp = nullptr;
        auto cet = (CorElementType)pb[0];

        switch(cet)
        {
            case ELEMENT_TYPE_VOID:
            case ELEMENT_TYPE_BOOLEAN:
            case ELEMENT_TYPE_CHAR:   
            case ELEMENT_TYPE_U1:    
            case ELEMENT_TYPE_I2:    
            case ELEMENT_TYPE_U2:    
            case ELEMENT_TYPE_I4:    
            case ELEMENT_TYPE_U4:    
            case ELEMENT_TYPE_I8:    
            case ELEMENT_TYPE_U8:    
            case ELEMENT_TYPE_R4:    
            case ELEMENT_TYPE_R8:
            case ELEMENT_TYPE_STRING:
            case ELEMENT_TYPE_I:
            case ELEMENT_TYPE_U:
            case ELEMENT_TYPE_TYPEDBYREF:
            case ELEMENT_TYPE_OBJECT:
                ++cb;
                *pp = (Type*)pb;
                return cb;
            case ELEMENT_TYPE_CLASS: {
                mdToken token;
                ++cb;
                cb += CorSigUncompressToken(pb+cb,&token);
                *pp = Anew(a, Class, token);
                return cb; }
            case ELEMENT_TYPE_VALUETYPE: {
                mdToken token;
                ++cb;
                cb += CorSigUncompressToken(pb+cb,&token);
                *pp = Anew(a, Value, token);
                return cb; }
            case ELEMENT_TYPE_CMOD_OPT: {
                mdToken token;
                ++cb;
                cb += CorSigUncompressToken(pb+cb,&token);
                Type * modded = nullptr;
                cb += DecodeType(sigSize-cb,pb+cb,a,&modded);
                *pp = Anew(a, ModOpt, token, modded);
                return cb; }
            case ELEMENT_TYPE_CMOD_REQD: {
                mdToken token;
                ++cb;
                cb += CorSigUncompressToken(pb+cb,&token);
                Type * modded = nullptr;
                cb += DecodeType(sigSize-cb,pb+cb,a,&modded);
                *pp = Anew(a, ModReqd, token, modded);
                return cb; }
            case ELEMENT_TYPE_ARRAY: {
                ++cb;
                Type * elementType = nullptr;
                cb += DecodeType(sigSize-cb,pb+cb, a, &elementType);
                Js::VerifyCatastrophic(cb<sigSize);
                auto rank = *(pb+cb);
                ++cb;
                Js::VerifyCatastrophic(cb<sigSize);
                auto boundsCount = *(pb+cb);
                ++cb;
                auto bounds = AnewArray(a,int,boundsCount);
                for (int i = 0; i< boundsCount; ++i)
                {
                    Js::VerifyCatastrophic(cb<sigSize);
                    bounds[i] = *(pb+cb);
                    ++cb;
                }
                Js::VerifyCatastrophic(cb<sigSize);
                auto loCount = *(pb+cb);
                ++cb;
                auto lo = AnewArray(a,int, loCount);
                for (int i = 0; i< loCount; ++i)
                {
                    Js::VerifyCatastrophic(cb<sigSize);
                    lo[i] = *(pb+cb);
                    ++cb;
                }
                *pp = Anew(a, MdArray, elementType, rank,
                                        boundsCount, bounds, 
                                        loCount, lo);
                return cb;
            }
            case ELEMENT_TYPE_SZARRAY: {
                ++cb;
                Type * elementType = nullptr;
                cb += DecodeType(sigSize-cb,pb+cb, a, &elementType);
                *pp = Anew(a, SzArray, elementType);
                return cb; }
            case ELEMENT_TYPE_PTR: {
                ++cb;
                Type * pointedTo = nullptr;
                cb += DecodeType(sigSize-cb,pb+cb, a, &pointedTo);
                *pp = Anew(a,Ptr,pointedTo);
                return cb; }
            case ELEMENT_TYPE_BYREF: {
                ++cb;
                Type * pointedTo = nullptr;
                cb += DecodeType(sigSize-cb,pb+cb, a, &pointedTo);
                *pp = Anew(a, ByRef, pointedTo);
                return cb; }
            case ELEMENT_TYPE_VAR: {
                ++cb;
                if (cb>=sigSize)
                {
                    Js::Throw::FatalProjectionError();
                }
                int index = *(pb+cb);
                ++cb;
                *pp = Anew(a, TVar, index);
                return cb; }
            case ELEMENT_TYPE_MVAR: {
                ++cb;
                if (cb>=sigSize)
                {
                    Js::Throw::FatalProjectionError();
                }
                int index = *(pb+cb);
                ++cb;
                *pp = Anew(a, MVar, index);
                return cb; }
            case ELEMENT_TYPE_GENERICINST: {
                // We have something in the form of GenericType<ConcreteType1,ConcreteType2>
                GenericInstantiation * gi = nullptr;
                auto result = DecodeGenericInstantiation(sigSize-cb,pb+cb,a,&gi);
                *pp=gi;
                return cb + result;
                }

            default: 
                Js::Throw::FatalProjectionError();
        }
    }

    // Info:        Given a PCCOR_SIGNATURE for a parameter, decode it into a Parameter instance.
    // Parameters:  psig - The signature blob
    //              a - The allocator to use
    //              pp - Receives the Parameter instance.
    // Return:      The number of bytes consumed from the blob
    ULONG Assembly::DecodeParameter(__in ULONG sigSize, __in_bcount(sigSize) PCCOR_SIGNATURE psig, __in ArenaAllocator * a, __out Parameter **pp) 
    {
        // This function handles section 23.2.10 Param of
        // http://www.ecma-international.org/publications/files/ECMA-ST/Ecma-335.pdf
        auto pb = (BYTE*)psig;
        ULONG cb = 0;

        switch(*pb)
        {
        case ELEMENT_TYPE_BYREF:
            {
                ++cb;
                Type * pointedTo = nullptr;
                cb += DecodeType(sigSize-cb,pb+cb,a,&pointedTo);
                auto byref = Anew(a, ByRef, pointedTo);
                *pp = Anew(a, Parameter, byref, true, false);
                return cb;
            }
        case ELEMENT_TYPE_TYPEDBYREF:
            ++cb;
            *pp = Anew(a, Parameter, nullptr, true, true);
            return cb;
        default:
            {
                Type * type = nullptr;
                cb += DecodeType(sigSize-cb,pb+cb,a,&type);
                *pp = Anew(a, Parameter, type, false, false);
                return cb;
            }
        }
        
    }

    // Info:        Given a parameter blob, build a Parameter list over it.
    // Parameters:  count - The number of parameters
    //              parameterSize - Number of bytes in the blob
    //              pb - The parameter blob
    // Return:      The list containing the parameters read
    ULONG Assembly::DecodeParameters(__in ULONG sigSize, __in_bcount(sigSize) PCCOR_SIGNATURE psig, __in ArenaAllocator * a, __in int parameterCount, __out ImmutableList<const Parameter*> ** parameters)
    {
        auto out = ImmutableList<const Parameter*>::Empty();
        auto tail = out;
        auto pb = (BYTE*)psig;
        ULONG cb = 0;
        int currentIndex=0;

        while(currentIndex<parameterCount)
        {
            Parameter * currentParameter;
            cb +=  DecodeParameter(sigSize-cb, pb+cb, a, &currentParameter);
            out = out->Append(currentParameter, a, &tail);
            ++currentIndex;
        }

        *parameters = out;
        Assert(cb==sigSize); // Should have read all the bytes
        return cb;
    }

    // Info:        Decode the method definition signature
    // Parameters:  PCCOR_SIGNATURE - the signature blob
    //              a - The allocator to use
    //              sizeSig - The size of the signature
    //              pp - receives the method definition signature
    ULONG Assembly::DecodeMethodDefSig(__in ULONG sigSize, __in_bcount(sigSize) PCCOR_SIGNATURE psig, __in ArenaAllocator * a, __out MethodDefSig **pp) 
    {
        // This function handles section 23.2.1 Param of
        // http://www.ecma-international.org/publications/files/ECMA-ST/Ecma-335.pdf
        auto mds = Anew(a, MethodDefSig);
        BYTE * pb = (BYTE*)psig;
        ULONG cb = 0;
        *pp=mds;
        auto cc = (CorCallingConvention)pb[cb];


        if (cc & IMAGE_CEE_CS_CALLCONV_HASTHIS)
        {
            mds->hasThis = true;
        }
            
        if (cc & IMAGE_CEE_CS_CALLCONV_EXPLICITTHIS)
        {
            mds->hasThis = true;
            mds->explicitThis = true;
        }

        if (cc & IMAGE_CEE_CS_CALLCONV_GENERIC)
        {
            mds->genericCall = true;
        }

        switch(cc & IMAGE_CEE_CS_CALLCONV_MASK)
        {
        case IMAGE_CEE_CS_CALLCONV_DEFAULT:
            mds->defaultCall = true;
            break;
        case IMAGE_CEE_CS_CALLCONV_VARARG:
            mds->varargCall = true;
            break;
        default:
            Js::Throw::FatalProjectionError();

        }
            
        // Calling convention is one byte
        ++cb;

        // If we have a generic method signature then we need to get the generic method count.
        if (mds->genericCall) 
        {
            if(cb>=sigSize)
            {
                Js::Throw::FatalProjectionError();
            }
            mds->genericParameterCount = *(pb+cb);
            ++cb;
        }

        // Regular parameter count
        if (cb>=sigSize)
        {
            Js::Throw::FatalProjectionError();
        }
        mds->parameterCount = *(pb+cb);
        ++cb;

        // Return type
        cb+=DecodeType(sigSize-cb,pb+cb, a, &mds->returnType);

        // Decode parameters
        cb+=DecodeParameters(sigSize-cb,pb+cb,a,mds->parameterCount,&mds->parameters);			

        Js::VerifyCatastrophic(cb==sigSize);
        return sigSize;
    }


    // -- Metadata functions -----------------------------------------------------------------------------------------------------------

    // Info:        Get the assembly reference tokens for this assembly
    // Return:      Lst containing the tokens
    ImmutableList<mdAssemblyRef> * Assembly::ReferenceTokens() const
    { 
        const auto chunkSize = 128;
        mdAssemblyRef refs[chunkSize];
        auto count = 0uL;
        HCORENUM he = nullptr;
        auto result = ImmutableList<mdAssemblyRef>::Empty();
        HRESULT hr;

        while ((hr = metadata->EnumAssemblyRefs(&he, refs, chunkSize, &count)) == S_OK && count > 0)
        {
            result = result->PrependArray(refs, count, a);
        }

        Js::VerifyOkCatastrophic(hr);

        metadata->CloseEnum(he);

        return result;
    }

    // Info:        Get the assembly token for this assembly
    // Return:      Assembly token
    mdAssembly Assembly::GetAssemblyToken() const
    {
        mdAssembly asmToken; 
        HRESULT hr = metadata->GetAssemblyFromScope(&asmToken);
        Js::VerifyOkCatastrophic(hr);
        return asmToken;
    }

    // Info:        Get assembly properties
    // Parameters:  mdAssembly - the token
    AssemblyProperties * Assembly::GetAssemblyProperties(__in mdAssembly td) const
    {
        Assert((mdtAssembly & td) == mdtAssembly);

        auto props = Anew(a,AssemblyProperties);
        props->sizeName = 0;
        auto hr = metadata->GetAssemblyProps(td, nullptr, &props->sizePublicKey, &props->hashAlgorithmID, nullptr, 0, &props->sizeName, nullptr, &props->flags);
        Js::VerifyOkCatastrophic(hr);
        Js::VerifyCatastrophic(props->sizeName > 0);
        props->td = td;
        props->name = AnewArrayZ(a, char16, props->sizeName);
        hr = metadata->GetAssemblyProps(td, &props->publicKey, &props->sizePublicKey, &props->hashAlgorithmID, props->name, props->sizeName, &props->sizeName, nullptr, &props->flags);
        Js::VerifyOkCatastrophic(hr);
        return props;
    }

    // Info:        Get the tokens of types defined in this assembly
    // Return:      List of tokens
    ImmutableList<mdTypeDef> * Assembly::TypeDefinitionTokens() const 
    {
        const auto chunkSize = 128;
        mdTypeDef refs[chunkSize];
        auto count = 0uL;
        HCORENUM he = nullptr;
        auto result = ImmutableList<mdTypeDef>::Empty();
        HRESULT hr;

        while ((hr = GetImport()->EnumTypeDefs(&he, refs, chunkSize, &count)) == S_OK && count > 0)
        {
            result = result->PrependArray(refs, count, a);
        }

        Js::VerifyOkCatastrophic(hr);

        GetImport()->CloseEnum(he);
        
        TRACE_METADATA(_u("TypeDefinitionTokens(), result count: %d\n"), result->Count());

        return result;
    }

    // Info:        Get type definition properties for a token
    // Parameter:   td - the typedef token to get more info about
    // Return:      Typedef properties for this token
    const TypeDefProperties * Assembly::GetTypeDefProperties(__in mdTypeDef td) const
    {
        Assert((mdtTypeDef & td) == mdtTypeDef);

        void * cachedItem;
        if (tokenCache->TryGetValue(td, &cachedItem)) 
        {            
            auto props = (TypeDefProperties *)cachedItem;
            Assert(props->td == td);
            
            TRACE_METADATA(_u("Cached: GetTypeDefProperties(%d), result: %s\n"), td, stringConverter->StringOfId(props->id));

            return props;
        }
        
        auto props = Anew(a,TypeDefProperties,*this);
        ulong sizeName = 0;
        auto hr = GetImport()->GetTypeDefProps(td, nullptr, 0, &sizeName, nullptr, nullptr);
        Js::VerifyOkCatastrophic(hr);
        props->td = td;
        Js::VerifyCatastrophic(sizeName > 0);
        AutoHeapString name;
        name.CreateNew(sizeName);
        /* cast of size_t -> ULONG: AutoHeapString.GetLength() returns the same size that it was initialized with */
        hr = GetImport()->GetTypeDefProps(td, name.Get(), (ULONG)name.GetLength(), &sizeName, &props->flags, &props->extends);
        Js::VerifyOkCatastrophic(hr);
        props->id = stringConverter->IdOfString(name.Get());
#if DBG
        props->typeName_Debug = stringConverter->StringOfId(props->id);
#endif
        props->genericParameterTokens = GenericParameterTokens(td);

        tokenCache->Item(td, props);
        
        TRACE_METADATA(_u("Fetched: GetTypeDefProperties(%d), result: %s\n"), td, name.Get());

        return props;
    }

    // Info:        Get all the types exposed by this assembly
    // Return:      List of type definitions
    ImmutableList<const TypeDefProperties*> * Assembly::TypeDefinitions() const
    {
        auto typeDefinitionTokens = TypeDefinitionTokens();

        auto result = typeDefinitionTokens->Select<const TypeDefProperties*>([&](mdTypeDef td) {
            return GetTypeDefProperties(td);
        },a);

        return result;
    }

    // Info:        Get all members exposed by this type
    // Parameter:   td - the typedef token to get members of
    // Return:      List of member tokens
    ImmutableList<mdToken> * Assembly::MemberTokens(__in mdTypeDef td) const
    {
        Assert((mdtTypeDef & td) == mdtTypeDef);
        const auto chunkSize = 32;
        mdToken refs[chunkSize];
        auto count = 0uL;
        HCORENUM he = nullptr;
        auto * result = ImmutableList<mdToken>::Empty();
        auto tail = result;
        HRESULT hr;

        while ((hr = GetImport()->EnumMembers(&he, td, refs, chunkSize, &count)) == S_OK && count > 0)
        {
            result = result->AppendArrayToCurrentList(refs, count, a, &tail);
        }

        Js::VerifyOkCatastrophic(hr);

        GetImport()->CloseEnum(he);
        
        TRACE_METADATA(_u("MemberTokens(%d), result count: %d\n"), td, result->Count());

        return result;
    }
    
    // Info:        Get properties for the given member token
    // Parameter:   mt - a member token
    // Return:      Member properties
    const MemberProperties * Assembly::GetMemberProperties(__in mdToken mt) const
    {
        auto props = Anew(a,MemberProperties,*this);
        ulong sizeName = 0;
        auto hr = GetImport()->GetMemberProps(mt, &props->classToken, nullptr, 0, &sizeName, &props->flags, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        Js::VerifyOkCatastrophic(hr);
        props->mt = mt;
        Js::VerifyCatastrophic(sizeName > 0);
        AutoHeapString name;
        name.CreateNew(sizeName);
        // name was sizeName which was provided within the range of a ULONG, therefore it is still a ULONG when retrieved.
        hr = GetImport()->GetMemberProps(mt, &props->classToken, name.Get(), (ULONG)name.GetLength(), &sizeName, &props->flags, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        Js::VerifyOkCatastrophic(hr);
        props->id = stringConverter->IdOfString(name.Get());

        TRACE_METADATA(_u("GetMemberProperties(%d), result: %s\n"), mt, name.Get());
        
        return props;
    }

    // Info:        Get all member properties for the given type
    // Parameter:   td - a typedef token
    // Return:      List of member propertes
    ImmutableList<const MemberProperties*> * Assembly::Members(__in mdTypeDef td) const
    {
        Assert((mdtTypeDef & td) == mdtTypeDef);

        auto memberTokens = MemberTokens(td);

        auto result = memberTokens->Select<const MemberProperties*>([&](mdTypeDef td) {
            return GetMemberProperties(td);
        },a);

        return result;
    }

    // Info:        Get all events exposed by this type
    // Parameter:   td - the typedef token to get events of
    // Return:      List of event tokens
    ImmutableList<mdToken> * Assembly::EventTokens(__in mdTypeDef td) const
    {
        Assert((mdtTypeDef & td) == mdtTypeDef);
        const auto chunkSize = 4;
        mdToken refs[chunkSize];
        auto count = 0uL;
        HCORENUM he = nullptr;
        auto * result = ImmutableList<mdToken>::Empty();
        HRESULT hr;

        while ((hr = GetImport()->EnumEvents(&he, td, refs, chunkSize, &count)) == S_OK && count > 0)
        {
            result = result->PrependArray(refs, count, a);
        }

        Js::VerifyOkCatastrophic(hr);

        GetImport()->CloseEnum(he);
        
        TRACE_METADATA(_u("EventTokens(%d), result count: %d\n"), td, result->Count());
        
        return result;
    }

    // Info:        Get properties for the given event token
    // Parameter:   mt - a event token
    // Return:      Event properties
    const EventProperties * Assembly::GetEventProperties2(__in mdEvent mt, __in const TypeDefProperties * type) const
    {
        Assert((mdtEvent & mt) == mdtEvent);

        auto props = Anew(a,EventProperties, *this);
        ulong sizeName = 0;
        auto hr = GetImport()->GetEventProps(mt, &props->classToken, nullptr, 0, &sizeName, &props->flags,
            &props->eventType, &props->addOn, &props->removeOn, &props->fire, nullptr, 0, nullptr);
        Js::VerifyOkCatastrophic(hr);
        props->ev = mt;
        Js::VerifyCatastrophic(sizeName > 0);
        AutoHeapString name;
        name.CreateNew(sizeName);
        // name was sizeName which was provided within the range of a ULONG, therefore it is still a ULONG when retrieved.
        hr = GetImport()->GetEventProps(mt, &props->classToken, name.Get(), (ULONG)name.GetLength(), &sizeName, &props->flags,
                &props->eventType, &props->addOn, &props->removeOn, &props->fire, nullptr, 0, nullptr);
        Js::VerifyOkCatastrophic(hr);
        props->id = stringConverter->IdOfString(name.Get());

        // Resolve method ids
        props->addOnMethod = type->GetMethodByToken(props->addOn);
        props->removeOnMethod = type->GetMethodByToken(props->removeOn);
        Js::VerifyCatastrophic(props->addOnMethod && props->removeOnMethod);
        
        TRACE_METADATA(_u("GetEventProperties2(%d), result: %s\n"), mt, name.Get());
        
        return props;
    }
    
    // Info:        Get generic parameter tokens for the given type or method.
    // Parameter:   token - the token
    // Return:      List of generic parameter tokens
    ImmutableList<mdGenericParam> * Assembly::GenericParameterTokens(__in mdToken token) const
    {
        Assert((mdtTypeDef & token) == mdtTypeDef || (mdtMethodDef & token) == mdtMethodDef);
        const auto chunkSize = 4;
        mdGenericParam refs[chunkSize];
        auto count = 0uL;
        HCORENUM he = nullptr;
        auto result = ImmutableList<mdGenericParam>::Empty();
        auto tail = result;
        HRESULT hr;

        while ((hr = GetImport()->EnumGenericParams(&he, token, refs, chunkSize, &count)) == S_OK && count > 0)
        {
            result = result->AppendArrayToCurrentList(refs, count, a, &tail);
        }

        Js::VerifyOkCatastrophic(hr);

        GetImport()->CloseEnum(he);
        
        TRACE_METADATA(_u("GenericParameterTokens(%d), result count: %d\n"), token, result->Count());
        
        return result;
    }

    // Info:        Get generic parameter properties
    // Parameter:   gp - the token
    const GenericParameterProperties * Assembly::GetGenericParameterProperties(__in mdGenericParam gp) const
    {
        Assert((mdtGenericParam & gp) == mdtGenericParam);

        auto props = Anew(a,GenericParameterProperties,*this);
        ulong sizeName = 0;
        auto hr = GetImport()->GetGenericParamProps(gp, &props->sequence, &props->flags, nullptr, nullptr, nullptr, 0, &sizeName);
        Js::VerifyOkCatastrophic(hr);
        props->gp = gp;
        Js::VerifyCatastrophic(sizeName > 0);
        AutoHeapString name;
        name.CreateNew(sizeName);
        // name was sizeName which was provided within the range of a ULONG, therefore it is still a ULONG when retrieved.
        hr = GetImport()->GetGenericParamProps(gp, &props->sequence, &props->flags, nullptr, nullptr, name.Get(), (ULONG)name.GetLength(), &sizeName);
        Js::VerifyOkCatastrophic(hr);
        props->id = stringConverter->IdOfString(name.Get());

        TRACE_METADATA(_u("GetGenericParameterProperties(%d), result: %s\n"), gp, name.Get());
        
        return props;
    }

    // Info:        Get a list of all generic parameter properties.
    // Parameter:   td - the token
    ImmutableList<const GenericParameterProperties*> * Assembly::GenericParameters(__in ImmutableList<mdGenericParam> *genericParamTokens) const
    {
        auto result = genericParamTokens->Select<const GenericParameterProperties*>([&](mdGenericParam gp) {
            return GetGenericParameterProperties(gp);
        },a);

        return result;
    }

    // Info:        Get all member properties for the given type
    // Parameter:   td - a typedef token
    // Return:      List of member propertes
    ImmutableList<mdMethodDef> * Assembly::MethodTokens(__in mdTypeDef td) const
    {
        const auto chunkSize = 4;
        mdMethodDef refs[chunkSize];
        auto count = 0uL;
        HCORENUM he = nullptr;
        auto result = ImmutableList<mdMethodDef>::Empty();
        auto tail = result;
        HRESULT hr;

        while ((hr = GetImport()->EnumMethods(&he, td, refs, chunkSize, &count)) == S_OK && count > 0)
        {
            result = result->AppendArrayToCurrentList(refs, count, a, &tail);
        }

        Js::VerifyOkCatastrophic(hr);

        GetImport()->CloseEnum(he);
        
        TRACE_METADATA(_u("MethodTokens(%d), result count: %d\n"), td, result->Count());
        
        return result;
    }

    // Info:        Get all field tokens for this type
    // Parameter:   td - a typedef token
    // Return:      List of field propertes
    ImmutableList<mdFieldDef> * Assembly::FieldTokens(__in mdTypeDef td) const
    {
        const auto chunkSize = 4;
        mdFieldDef refs[chunkSize];
        auto count = 0uL;
        HCORENUM he = nullptr;
        auto result = ImmutableList<mdFieldDef>::Empty();
        auto tail = result;
        HRESULT hr;

        while(true)
        {
            hr = GetImport()->EnumFields(&he, td, refs, chunkSize, &count);

            // A demo to test bug 405425 with fault injection
            INJECT_FAULT(Js::FaultInjection::Global.EnumFields_Fail, []()->bool{
                return true; // always inject
            }, {
                hr = E_FAIL; // make the fault that means the call into rometadata function failed
            });


            if(hr == S_OK && count > 0){
                result = result->AppendArrayToCurrentList(refs, count, a, &tail);
            } else {
                break;
            }
        }

        Js::VerifyOkCatastrophic(hr);

        GetImport()->CloseEnum(he);
        
        TRACE_METADATA(_u("FieldTokens(%d), result count: %d\n"), td, result->Count());
        
        return result;
    }

    // Info:        Find a top level type by name
    // Parameter:   name - the name of the type
    // Return:      TypeDefProperties if present or nullptr
    const TypeDefProperties * Assembly::FindTopLevelTypeDefByName(LPCWSTR name) const
    {
        mdTypeDef typeDefToken;
        auto hr = GetImport()->FindTypeDefByName(name, NULL, &typeDefToken);

        if (SUCCEEDED(hr))
        {
            TRACE_METADATA(_u("FindTopLevelTypeDefByName(\"%s\") succeeded, result: %d\n"), name, typeDefToken);
            
            return GetTypeDefProperties(typeDefToken);
        }

        TRACE_METADATA(_u("FindTopLevelTypeDefByName(\"%s\") failed\n"), name);
        
        return nullptr;
    }


    // Info:        Get method properties for the given token
    // Parameter:   mb - a method def token
    // Return:      The method properties
    MethodProperties* Assembly::GetMethodProperties(__in mdMethodDef mb) const
    {
        Assert((mdtMethodDef & mb) == mdtMethodDef); 

        auto props = Anew(a,MethodProperties,*this);
        ulong sizeName = 0;
        ulong sizeSig;
        PCCOR_SIGNATURE sigBlob;
        
        auto hr = GetImport()->GetMethodProps(mb, &props->classToken, nullptr, 0, &sizeName, &props->flags, nullptr, &sizeSig, nullptr, nullptr);
        Js::VerifyOkCatastrophic(hr);
        props->mb = mb;
        Js::VerifyCatastrophic(sizeName > 0);
        AutoHeapString name;
        name.CreateNew(sizeName);
        // name was sizeName which was provided within the range of a ULONG, therefore it is still a ULONG when retrieved.
        hr = GetImport()->GetMethodProps(mb, &props->classToken, name.Get(), (ULONG)name.GetLength(), &sizeName, &props->flags, &sigBlob, &sizeSig, nullptr, nullptr);
        Js::VerifyOkCatastrophic(hr);
        props->id = stringConverter->IdOfString(name.Get());

        DecodeMethodDefSig(sizeSig, sigBlob, a, &props->signature);

        // Get overload attributes
        // Note, if the method is a "special name", then it's either a property, an event, or the delegate Invoke method
        // These can't be overloaded, so don't retrieve their overload attributes.
        if (props->IsSpecialName())
        {
            props->isDefaultOverload = false;
            props->overloadNameId = MetadataStringIdNil;            
        }
        else
        {
            props->isDefaultOverload = IsAttributePresent(mb, _u("Windows.Foundation.Metadata.DefaultOverloadAttribute"));
            props->overloadNameId = MetadataStringIdNil;
            LPCWSTR overloadName = GetStringAttribute(mb, _u("Windows.Foundation.Metadata.OverloadAttribute"));
            if (overloadName != NULL)
            {
                props->overloadNameId = stringConverter->IdOfString(overloadName);
            }
        }
        
        // Note the method index is not filled in here.  It's filled in either by Assembly::Methods.
        props->methodIndex = -1;
        
        TRACE_METADATA(_u("GetMethodProperties(%d), result: %s\n"), mb, name.Get());
        
        return props; 
    }

    // Info:        Get method properties for the given token
    // Parameter:   mb - a method def token
    // Return:      The method properties
    const MethodProperties* Assembly::GetMethodProperties2(__in mdMethodDef mb) const
    {
        Assert((mdtMethodDef & mb) == mdtMethodDef); 

        // First, resolve to a type.
        // This ensures that the type itself is fully constructed.
        mdTypeDef typeToken;
        auto hr = GetImport()->GetMethodProps(mb, &typeToken, nullptr, 0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        Js::VerifyOkCatastrophic(hr);
        const TypeDefProperties * type = GetTypeDefProperties(typeToken);
        
        // Now, find the method within the type.
        auto props = type->GetMethodByToken(mb);

        TRACE_METADATA(_u("GetMethodProperties2(%d), result: %s\n"), mb, stringConverter->StringOfId(props != nullptr ? props->id : MetadataStringIdNil));
        
        return props;
    }

    // Info:        Get all methods for the given type
    // Parameter:   td - a typedef token
    // Return:      List of methods
    ImmutableList<const MethodProperties*> * Assembly::Methods(__in mdTypeDef td) const
    {
        Assert((mdtTypeDef & td) == mdtTypeDef);

        int index = 0;        
        auto methodTokens = MethodTokens(td);

        auto result = methodTokens->Select<const MethodProperties*>([&](mdToken mb) {
            auto props = GetMethodProperties(mb);

            Assert(props->classToken == td);
                
            props->methodIndex = index;
            index++;
                                
            return props;
        },a);

        return result;
    }

    // Info:        Get all field properties for the field
    // Parameter:   fd - a field def token
    // Return:      Field properties
    const FieldProperties* Assembly::GetFieldProperties(__in mdFieldDef fd) const
    {
        Assert((mdtFieldDef & fd) == mdtFieldDef);

        auto props = Anew(a,FieldProperties,*this); 
        ulong sizeName = 0;
        ulong sizeSig;
        PCCOR_SIGNATURE sigBlob;
        
        auto hr = GetImport()->GetFieldProps(fd, &props->classToken, nullptr, 0, &sizeName, &props->flags, nullptr, &sizeSig, &props->dwCPlusTypeFlag, nullptr, nullptr);
        Js::VerifyOkCatastrophic(hr);
        props->fieldToken = fd;
        Js::VerifyCatastrophic(sizeName > 0);
        AutoHeapString name;
        name.CreateNew(sizeName);
        // name was sizeName which was provided within the range of a ULONG, therefore it is still a ULONG when retrieved.
        hr = GetImport()->GetFieldProps(fd, &props->classToken, name.Get(), (ULONG)name.GetLength(), &sizeName, &props->flags, &sigBlob, &sizeSig, &props->dwCPlusTypeFlag, &props->constantValue, nullptr);
        Js::VerifyOkCatastrophic(hr);
        props->id = stringConverter->IdOfString(name.Get());
        auto read = DecodeField(sizeSig, sigBlob, a, &props->type);
        Js::VerifyCatastrophic(read == sizeSig);

        TRACE_METADATA(_u("GetFieldProperties(%d), result: %s\n"), fd, name.Get());
        
        return props; 
    }

    // Info:        Get the field list for this typedef
    // Parameter:   td - the type def
    ImmutableList<const FieldProperties*> * Assembly::Fields(__in mdTypeDef td) const
    {
        Assert((mdtTypeDef & td) == mdtTypeDef);

        auto fieldTokens = FieldTokens(td);

        auto result = fieldTokens->Select<const FieldProperties*>([&](mdToken td) {
            return GetFieldProperties(td);
        },a);

        return result;
    }

    // Info:        Get the property token list for this typedef
    // Parameter:   td - the type def
    ImmutableList<mdProperty> * Assembly::PropertyTokens(__in mdTypeDef td) const
    {
        const auto chunkSize = 4;
        mdProperty refs[chunkSize];
        auto count = 0uL;
        HCORENUM he = nullptr;
        auto result = ImmutableList<mdProperty>::Empty();
        HRESULT hr;

        while ((hr = GetImport()->EnumProperties(&he, td, refs, chunkSize, &count)) == S_OK && count > 0)
        {
            result = result->PrependArray(refs, count, a);
        }

        Js::VerifyOkCatastrophic(hr);

        GetImport()->CloseEnum(he);

        TRACE_METADATA(_u("PropertyTokens(%d), result count: %d\n"), td, result->Count());        
        
        return result;
    }

    // Info:        Get the properties of the given mdProperty
    // Parameter:   pt - the property token
    const PropertyProperties* Assembly::GetPropertyProperties2(__in mdProperty pt, __in const TypeDefProperties * type) const
    {
        Assert((mdtProperty & pt) == mdtProperty);

        auto props = Anew(a,PropertyProperties,*this); 
        ulong sizeName = 0;
        ulong sizeSig;
        PCCOR_SIGNATURE sigBlob;
        
        auto hr = GetImport()->GetPropertyProps(pt, &props->classToken, nullptr, 0, &sizeName, &props->flags, nullptr, &sizeSig, nullptr, nullptr, nullptr, &props->setter, &props->getter, nullptr, 0, nullptr);
        Js::VerifyOkCatastrophic(hr);
        props->propertyToken = pt;
        Js::VerifyCatastrophic(sizeName > 0);
        AutoHeapString name;
        name.CreateNew(sizeName);
        // name was sizeName which was provided within the range of a ULONG, therefore it is still a ULONG when retrieved.
        hr = GetImport()->GetPropertyProps(pt, &props->classToken, name.Get(), (ULONG)name.GetLength(), &sizeName, &props->flags, &sigBlob, &sizeSig, nullptr, nullptr, nullptr, &props->setter, &props->getter, nullptr, 0, nullptr);
        Js::VerifyOkCatastrophic(hr);
        props->id = stringConverter->IdOfString(name.Get());
        auto read = DecodeProperty(sizeSig, sigBlob, a, &props->hasThis, &props->paramCount, &props->type, &props->parameters);
        Js::VerifyCatastrophic(read == sizeSig);

        // Resolve method ids
        props->getterMethod = type->GetMethodByToken(props->getter);
        props->setterMethod = type->GetMethodByToken(props->setter);
        Js::VerifyCatastrophic(props->getterMethod || props->setterMethod);
        
        TRACE_METADATA(_u("GetPropertyProperties2(%d), result: %s\n"), pt, name.Get());
        
        return props; 
    }

    // Info:        Get the member ref properties
    // Parameter:   mr - the member ref
    const MemberRefProperties* Assembly::GetMemberRefProperties(__in mdMemberRef mr) const
    {
        Assert((mdtMemberRef & mr) == mdtMemberRef);

        auto props = Anew(a,MemberRefProperties,*this);
        ulong sizeName = 0;
        ulong sizeSig;
        PCCOR_SIGNATURE sigBlob;
        
        auto hr = GetImport()->GetMemberRefProps(mr, &props->classToken, nullptr, 0, &sizeName, &sigBlob, &sizeSig);
        Js::VerifyOkCatastrophic(hr);
        props->mr = mr;
        Js::VerifyCatastrophic(sizeName > 0);
        AutoHeapString name;
        name.CreateNew(sizeName);
        // name was sizeName which was provided within the range of a ULONG, therefore it is still a ULONG when retrieved.
        hr = GetImport()->GetMemberRefProps(mr, &props->classToken, name.Get(), (ULONG)name.GetLength(), &sizeName, &sigBlob, &sizeSig);
        Js::VerifyOkCatastrophic(hr);
        props->id = stringConverter->IdOfString(name.Get());
        props->pvSig = sigBlob;
        props->sizeSig = sizeSig;

        TRACE_METADATA(_u("GetMemberRefProperties(%d), result: %s\n"), mr, name.Get());

        return props; 
    }

    // Info:        Get the custom attribute tokens for the given entity
    // Parameter:   td - the thing to get attributes for
    ImmutableList<mdCustomAttribute> * Assembly::CustomAttributeTokens(__in mdToken td, __in mdToken typeOfInterest) const
    {
        const auto chunkSize = 4;
        mdCustomAttribute refs[chunkSize];
        auto count = 0uL;
        HCORENUM he = nullptr;
        auto result = ImmutableList<mdCustomAttribute>::Empty();
        HRESULT hr;

        while ((hr = GetImport()->EnumCustomAttributes(&he, td, typeOfInterest, refs, chunkSize, &count)) == S_OK && count > 0)
        {
            result = result->PrependArray(refs, count, a);
        }

        Js::VerifyOkCatastrophic(hr);

        GetImport()->CloseEnum(he);
        
        TRACE_METADATA(_u("CustomAttributeTokens(%d, %d), result count: %d\n"), td, typeOfInterest, result->Count());
        
        return result;
    }

    // Info:        Get the properties of a custom attribute
    // Parameter:   cv - The custom attribute token
    const CustomAttributeProperties* Assembly::GetCustomAttributeProperties(__in mdCustomAttribute cv) const
    {
        Assert((mdtCustomAttribute & cv) == mdtCustomAttribute);

        CustomAttributeProperties* props = Anew(a, CustomAttributeProperties, *this);
        HRESULT hr = GetImport()->GetCustomAttributeProps(cv, &props->scopeToken, &props->attributeTypeToken, &props->blob, &props->blobSize);

        Js::VerifyOkCatastrophic(hr);

        TRACE_METADATA(_u("GetCustomAttributeProperties(%d), result: %d\n"), cv, props->attributeTypeToken);
            
        props->attributeToken = cv;
        props->isMemberRef = false;

        switch(TypeFromToken(props->attributeTypeToken))
        {
        case mdtMemberRef:
            {
                const MemberRefProperties* memberRefProps = GetMemberRefProperties(props->attributeTypeToken);
                switch(TypeFromToken(memberRefProps->classToken))
                {
                    case mdtTypeRef:
                        {
                            const TypeRefProperties* typeRefProps = GetTypeRefProperties(memberRefProps->classToken);
                            props->attributeTypeId = typeRefProps->id;
                        }
                        break;
                    default:
                        Js::Throw::FatalProjectionError();
                }
                props->isMemberRef = true;
                props->pSig = memberRefProps->pvSig;
                props->sizeSig = memberRefProps->sizeSig;
            }
            break;
        case mdtMethodDef:
            {
                auto memberProps = GetMemberProperties(props->attributeTypeToken);
                switch(TypeFromToken(memberProps->classToken))
                {
                case mdtTypeDef:
                    {
                        auto typeDefProps = GetTypeDefProperties(memberProps->classToken);
                        props->attributeTypeId = typeDefProps->id;
                    }
                    break;
                default:
                    Js::Throw::FatalProjectionError();
                }
            }
            break;
        default:
            Js::Throw::FatalProjectionError();
        }

        return props; 
    }

    // Info:        Get the custom attributes for the given entity
    // Parameter:   td - the thing to get attributes for
    ImmutableList<const CustomAttributeProperties*> * Assembly::CustomAttributes(__in mdToken td, __in mdToken typeOfInterest) const
    {
        ImmutableList<mdInterfaceImpl>* customAttributeTokens = CustomAttributeTokens(td,typeOfInterest);

        ImmutableList<const CustomAttributeProperties*>* result = customAttributeTokens->Select<const CustomAttributeProperties*>([&](mdToken td) 
        {
            return GetCustomAttributeProperties(td);
        }, a);

        TRACE_METADATA(_u("CustomAttributes(%d, %d), result count: %d\n"), td, typeOfInterest, result->Count());

        return result;
    }

    // Info:        Get the properties for this type ref.
    // Parameter:   tr - the type ref
    const TypeRefProperties * Assembly::GetTypeRefProperties(__in mdTypeRef tr) const
    {
        Assert((mdtTypeRef & tr) == mdtTypeRef);

        void * cachedItem;
        if (tokenCache->TryGetValue(tr, &cachedItem)) 
        {            
            auto props = (TypeRefProperties *)cachedItem;
            Assert(props->referenceToken == tr);
            
            TRACE_METADATA(_u("Cached: GetTypeRefProperties(%d), result: %s\n"), tr, stringConverter->StringOfId(props->id));

            return props;
        }
        
        auto props = Anew(a,TypeRefProperties,*this); 
        ulong sizeName = 0;
        HRESULT hr = GetImport()->GetTypeRefProps(tr, &props->resolutionScope, nullptr, 0, &sizeName);
        Js::VerifyOkCatastrophic(hr);
        props->referenceToken = tr;
        Js::VerifyCatastrophic(sizeName > 0);
        AutoHeapString name;
        name.CreateNew(sizeName);
        // name was sizeName which was provided within the range of a ULONG, therefore it is still a ULONG when retrieved.
        hr = GetImport()->GetTypeRefProps(tr, &props->resolutionScope, name.Get(), (ULONG)name.GetLength(), &sizeName);
        Js::VerifyOkCatastrophic(hr);
        props->id = stringConverter->IdOfString(name.Get());

        tokenCache->Item(tr, props);

        TRACE_METADATA(_u("Fetched: GetTypeRefProperties(%d), result: %s\n"), tr, name.Get());

        return props; 
    }

    // Info:        Get tokens for all the interfaces implemented on this type.
    // Parameter:   td - a type def token
    // Return:      List of interface impls
    ImmutableList<mdInterfaceImpl> * Assembly::InterfacesImplemented(__in mdTypeDef td) const
    {
        Assert(mdtTypeDef == TypeFromToken(td));
        const auto chunkSize = 4;
        mdInterfaceImpl refs[chunkSize];
        auto count = 0uL;
        HCORENUM he = nullptr;
        auto result = ImmutableList<mdInterfaceImpl>::Empty();
        HRESULT hr;

        while ((hr = GetImport()->EnumInterfaceImpls(&he, td, refs, chunkSize, &count)) == S_OK && count > 0)
        {
            result = result->PrependArray(refs, count, a);
        }

        Js::VerifyOkCatastrophic(hr);

        GetImport()->CloseEnum(he);
        
        TRACE_METADATA(_u("InterfacesImplemented(%d), result count: %d\n"), td, result->Count());
        
        return result;
    }

    // Info:        Get method impls for a type
    // Parameter:   td - a type def token
    // Return:      List of method impls
    ImmutableList<const MethodImpl*> * Assembly::MethodImpls(__in mdTypeDef td) const
    {
        const auto chunkSize = 16;
        mdToken methodBodies[chunkSize];
        mdToken methodDecls[chunkSize];
        auto count = 0uL;
        HCORENUM he = nullptr;
        auto result = ImmutableList<const MethodImpl*>::Empty();
        auto tail = result;
        HRESULT hr;

        while ((hr = GetImport()->EnumMethodImpls(&he, td, methodBodies, methodDecls, chunkSize, &count)) == S_OK && count > 0)
        {
            for (size_t i = 0; i < count; ++i)
            {
                auto methodImpl = Anew(a, MethodImpl, *this);
                methodImpl->methodBody = methodBodies[i];
                methodImpl->methodDecl = methodDecls[i];
                result = result->Append(methodImpl, a, &tail);
            }
        }

        Js::VerifyOkCatastrophic(hr);

        GetImport()->CloseEnum(he);
        
        TRACE_METADATA(_u("MethodImpls(%d), result count: %d\n"), td, result->Count());
        
        return result;
    }

    // Info:        Get properties for the given interface impl
    // Parameter:   ii - interface impl token
    // Return:      Interface impl properties
    const InterfaceImplProperties * Assembly::GetInterfaceImplProperties(__in mdInterfaceImpl ii) const
    {
        auto props = Anew(a, InterfaceImplProperties,*this); 
        auto hr = GetImport()->GetInterfaceImplProps(ii, &props->classToken, &props->interfaceToken);
        Js::VerifyOkCatastrophic(hr);
        props->implToken = ii;
        
        TRACE_METADATA(_u("GetInterfaceImplProperties(%d), result: %d, %d\n"), ii, props->classToken, props->interfaceToken);
        
        return props;
    }		

    // Info:        Get the parameter tokens for this method
    // Parameter:   mb - the method def token
    // Return:      List of parameter tokens
    ImmutableList<mdParamDef> * Assembly::ParameterTokens(__in mdMethodDef mb) const
    {
        const auto chunkSize = 4;
        mdParamDef refs[chunkSize];
        auto count = 0uL;
        HCORENUM he = nullptr;
        auto result = ImmutableList<mdParamDef>::Empty();
        auto tail = result;
        HRESULT hr;

        while ((hr = GetImport()->EnumParams(&he, mb, refs, chunkSize, &count)) == S_OK && count > 0)
        {
            result = result->AppendArrayToCurrentList(refs, count, a, &tail);
        }

        Js::VerifyOkCatastrophic(hr);

        GetImport()->CloseEnum(he);
        
        TRACE_METADATA(_u("ParameterTokens(%d), result count: %d\n"), mb, result->Count());
        
        return result;
    }

    // Info:        Get the parameter properties for the given tokenn
    // Parameter:   pt - the parameter def token
    // Return:      Parameter properties
    const ParameterProperties * Assembly::GetParameterProperties(__in mdParamDef pt) const
    {
        auto props = Anew(a,ParameterProperties,*this);
        ulong sizeName = 0;
        auto hr = GetImport()->GetParamProps(pt, &props->methodToken, &props->sequence, nullptr, 0, &sizeName, &props->flags, nullptr, nullptr, nullptr);
        Js::VerifyOkCatastrophic(hr);
        props->pt = pt;
        Js::VerifyCatastrophic(sizeName > 0);
        AutoHeapString name;
        name.CreateNew(sizeName);
        // name was sizeName which was provided within the range of a ULONG, therefore it is still a ULONG when retrieved.
        hr = GetImport()->GetParamProps(pt, &props->methodToken, &props->sequence, name.Get(), (ULONG)name.GetLength(), &sizeName, &props->flags, nullptr, nullptr, nullptr);
        Js::VerifyOkCatastrophic(hr);
        props->id = stringConverter->IdOfString(name.Get());
        
        TRACE_METADATA(_u("GetParameterProperties(%d), result: %s\n"), pt, name.Get());
        
        return props; 
    }

    // Info:        Given a type spec get the corresponding type.
    // Parameter:   ts - the type spec
    // Return:      The type
    const Type * Assembly::GetTypeSpec(__in mdTypeSpec ts) const
    {
        PCCOR_SIGNATURE sigBlob;
        ULONG sizeSig;
        auto hr = GetImport()->GetTypeSpecFromToken(ts, &sigBlob, &sizeSig);
        Js::VerifyOkCatastrophic(hr);
        Type * type = nullptr;
        auto read = DecodeType(sizeSig, sigBlob, a, &type);
        Js::VerifyCatastrophic(read == sizeSig);

        TRACE_METADATA(_u("GetTypeSpec(%d)\n"), ts);
        
        return type;
    }

    // Info:        Parse a blob-style guid into a GUID
    // Parameter:   value - the source guid
    //              guid - the destination guid
    void ParseGuid(__in ULONG size, __in_bcount(size) const BYTE* value, __out GUID& guid)
    {
        Js::VerifyCatastrophic(size==18);
        guid.Data1 = *((ULONG*)value);
        value+=sizeof(ULONG);
        guid.Data2 = *((WORD*)value);
        value+=sizeof(WORD);
        guid.Data3 = *((WORD*)value);
        value+=sizeof(WORD);
        memcpy_s(&guid.Data4, sizeof(guid.Data4), value, sizeof(guid.Data4));
    }

    // Info:        Get a GUID attribute value.
    // Parameter:   token - The entity token to look at (class, interface, etc)
    //              attributeName - The name of the attribute
    //              guid - Receives the resulting GUID. IID_NULL if not present.
    void Assembly::GetGuidAttributeValue(__in mdToken token, __in LPCWSTR attributeName, __out GUID & guid) const
    {
        const void *pvData;
        ULONG cbData;

        HRESULT hr = GetImport()->GetCustomAttributeByName(token, attributeName, &pvData, &cbData);

        TRACE_METADATA(_u("GetGuidAttributeValue(%d, %s)\n"), token, attributeName);
        
        if (FAILED(hr) || (hr == S_FALSE))
        {
            guid = IID_NULL;
            return;
        }

        if (cbData < sizeof(GUID) + 2)
        {
            guid = IID_NULL;
            return;
        }

        ParseGuid(cbData-2, reinterpret_cast<const BYTE *>(pvData) + 2, guid /* out */);
    }

    // Info:        Test whether an attribute is present on the given entity.
    // Parameter:   token - The entity token to look at (class, interface, etc)
    //              attributeName - The name of the attribute
    // Returns:     true, if the attribute is present
    bool Assembly::IsAttributePresent(__in mdToken token, __in LPCWSTR attributeName) const
    {
        HRESULT hr = GetImport()->GetCustomAttributeByName(token, attributeName, nullptr, nullptr);

        TRACE_METADATA(_u("IsAttributePresent(%d, %s)\n"), token, attributeName);
        
        if (FAILED(hr) || (hr == S_FALSE))
        {
            return false;
        }

        return true;
    }

    // Info:        Gets the character count for a given utf8 string
    // Parameter:   pch - the utf8 string, must be null terminated
    // Returns:     The number of characters in the string
    size_t GetCharacterCount(LPCUTF8 pch)
    {
        size_t cch = 0;
        utf8::DecodeOptions decodeOptions = utf8::doDefault;
        while (utf8::Decode(pch, pch + 4, decodeOptions)) // WARNING: No end-of-buffer check! Assumes sequence is well-formed and null-terminated.
            cch++;
        return cch;
    }

    // Info:        Verify the next two bytes of the attribute
    // Parameter:   dataSize - the total size, in bytes, of the data provided
    //              data - the bytes to be verified
    //              expectedValue - the expected value of the next two bytes
    // Returns:     The number of bytes verified
    ULONG Assembly::VerifyNextInt16OfAttribute(ULONG dataSize, const byte* data, __int16 expectedValue) const
    {
        // Verify data has at leat 2 bytes and those bytes match the given value
        Js::VerifyCatastrophic((dataSize >= 2) && (*((__int16 *)data) == expectedValue));
        // Return number of bytes verified
        return 2;
    }

    // Info:        Get a string pointer and length from attribute data
    // Parameter:   dataSize - the total size, in bytes, of the data provided
    //              data - the bytes to be verified
    //              strLen - the length of the string
    // Returns:     The number of bytes verified
    ULONG Assembly::VerifyAttributeString(ULONG dataSize, const byte* data, ULONG * strLen) const
    {
        ULONG lengthBytes = CorSigUncompressData(data, strLen);
        Js::VerifyCatastrophic(lengthBytes != -1);
        Js::VerifyCatastrophic(dataSize >= (lengthBytes + *strLen));
        return (lengthBytes + *strLen);
    }

    // Info:        Verify the attribute content, for an attribute with no named args or content of variable size (i.e. strings)
    // Parameter:   dataSize - the total size, in bytes, of the attribute's data
    //              data - the content of the attribute
    void Assembly::VerifySimpleAttributeBytes(ULONG dataSize, const byte* data,  ULONG expectedValueSize) const
    {
        // Verify total size of blob
        Js::VerifyCatastrophic(dataSize == 2 /* Prolog */ + (expectedValueSize /* Actual Value */ + 2 /* Number of named arguments - in our case 0*/));
        // Verify prolog
        Js::VerifyCatastrophic(*((__int16 *)data) == PrologBytes);
        // Verify number of named arguments == 0
        Js::VerifyCatastrophic(*((__int16 *)(data + 2 + expectedValueSize)) == 0);
    }

    // Info:        Get a string attribute from the given entity
    // Parameter:   token - The entity token to look at (class, interface, etc)
    //              attributeName - The name of the attribute
    // Returns:     The string if present. Null otherwise.
    LPCWSTR Assembly::GetStringAttribute(__in mdToken token, __in LPCWSTR attributeName) const 
    {
        // Get the overload name for this method, if there is one.
        const byte *pbData;
        ULONG cbData;

        auto hr = GetImport()->GetCustomAttributeByName(token, attributeName, (const void**)&pbData, &cbData);
        
        TRACE_METADATA(_u("GetStringAttribute(%d, %s)\n"), token, attributeName);
        
        if(SUCCEEDED(hr) && hr != S_FALSE)
        {
            ULONG verifiedBytes = 0;
            // Verify Prolog
            verifiedBytes += VerifyNextInt16OfAttribute(cbData, pbData, PrologBytes);
            ULONG strLen;
            verifiedBytes += VerifyAttributeString(cbData + verifiedBytes, &(pbData[verifiedBytes]), &strLen);
            auto stringArg = (char *)&(pbData[verifiedBytes-strLen]);
            auto nameLength = strLen + 1; // +1 to include space for the null terminator
            auto sz = AnewArrayZ(a, char16, nameLength);
            int chName = MultiByteToWideChar(CP_UTF8, 0, stringArg, strLen, sz, nameLength);
            
            // chName should never be greater than strLen, but ensure we properly null-terminate regardless.
            // We cast to ulong so that negative values will be very large and cause us to use strLen.
            Assert(chName >= 0);
            if (static_cast<ULONG>(chName) > strLen)
            {
                chName = strLen;
            }
            sz[chName] = _u('\0');
            return sz;
        }

        return nullptr;
    }

    // Info:        Gets Int32 attribute from the given entity
    // Parameter:   token - The entity token to look at (class, interface, parameter etc)
    //              attributeName - The name of the attribute
    //              attributeValue - attribute value if present
    // Returns:     true if present. false otherwise.
    bool Assembly::GetInt32Attribute(__in mdToken token, __in LPCWSTR attributeName, INT32 &attributeValue) const 
    {
        // Get the overload name for this method, if there is one.
        const byte *pbData;
        ULONG cbData;

        auto hr = GetImport()->GetCustomAttributeByName(token, attributeName, (const void**)&pbData, &cbData);

        TRACE_METADATA(_u("GetInt32Attribute(%d, %s)\n"), token, attributeName);
        
        if (hr == S_OK)
        {
            VerifySimpleAttributeBytes(cbData, pbData, sizeof(INT32));
            attributeValue = *((INT32 *)(pbData + 2));
            return true;
        }

        return false;
    }

    // Info:        Returns whether the token has a contract version on it
    // Parameter:   token - The entity token to look at (class, interface, parameter etc)
    // Returns:     true if present. false otherwise.
    bool Assembly::HasContractAttribute(__in mdToken token) const
    {
        // Get the overload name for this method, if there is one.
        const byte *pbData;
        ULONG cbData;

        auto hr = GetImport()->GetCustomAttributeByName(token, _u("Windows.Foundation.Metadata.ContractVersionAttribute"), (const void**)&pbData, &cbData);

        if (hr == S_OK)
        {
            return true;
        }

        return false;
    }

    // Info:        Gets DWORD attribute from the given entity
    // Parameter:   token - The entity token to look at (class, interface, parameter etc)
    //              attributeName - The name of the attribute
    //              attributeValue - attribute value if present
    // Returns:     true if present. false otherwise.
    bool Assembly::GetDWORDAttribute(__in mdToken token, __in LPCWSTR attributeName, DWORD &attributeValue) const 
    {
        // Get the overload name for this method, if there is one.
        const byte *pbData;
        ULONG cbData;

        auto hr = GetImport()->GetCustomAttributeByName(token, attributeName, (const void**)&pbData, &cbData);

        TRACE_METADATA(_u("GetDWORDAttribute(%d, %s)\n"), token, attributeName);
        
        if (hr == S_OK)
        {
            VerifySimpleAttributeBytes(cbData, pbData, sizeof(DWORD));
            attributeValue = *((DWORD *)(pbData + 2));
            return true;
        }

        return false;
    }

    // Info:        Get the size of a basic type like int, string, etc.
    // Parameter:   typeCode - basic type typecode
    // Returns:     The size, in bytes
    size_t Assembly::GetBasicTypeSize(__in CorElementType typeCode)
    {
        switch(typeCode)
        {
        case ELEMENT_TYPE_VOID:
            return 0;
        case ELEMENT_TYPE_CHAR:
            return sizeof(char16);
        case ELEMENT_TYPE_BOOLEAN:
            return sizeof(bool);
        case ELEMENT_TYPE_U1:
            return sizeof(byte); 
        case ELEMENT_TYPE_I2:
        case ELEMENT_TYPE_U2:
            return sizeof(__int16);
        case ELEMENT_TYPE_I4:
        case ELEMENT_TYPE_U4:
            return sizeof(int32);
        case ELEMENT_TYPE_I8:
        case ELEMENT_TYPE_U8:
            return sizeof(int64);
        case ELEMENT_TYPE_STRING:
            return sizeof(HANDLE);
        case ELEMENT_TYPE_R4:
            return sizeof(float);
        case ELEMENT_TYPE_R8:
            return sizeof(double);
        case ELEMENT_TYPE_OBJECT:
            return sizeof(LPVOID);
        default:
            Js::Throw::FatalProjectionError();
        }
    }

    // Info:        Get the size of a basic type like int, string, etc.
    // Parameter:   typeCode - basic type typecode
    // Returns:     The size, in bytes
    size_t Assembly::GetBasicTypeAlignment(__in CorElementType typeCode)
    {
        switch(typeCode)
        {
        case ELEMENT_TYPE_VOID:
            return 0;
        case ELEMENT_TYPE_CHAR:
            return __alignof(char16);
        case ELEMENT_TYPE_BOOLEAN:
            return __alignof(bool);
        case ELEMENT_TYPE_U1:
            return __alignof(byte); 
        case ELEMENT_TYPE_I2:
        case ELEMENT_TYPE_U2:
            return __alignof(__int16);
        case ELEMENT_TYPE_I4:
        case ELEMENT_TYPE_U4:
            return __alignof(int32);
        case ELEMENT_TYPE_I8:
        case ELEMENT_TYPE_U8:
            return __alignof(int64);
        case ELEMENT_TYPE_STRING:
            return __alignof(HANDLE);
        case ELEMENT_TYPE_R4:
            return __alignof(float);
        case ELEMENT_TYPE_R8:
            return __alignof(double);
        case ELEMENT_TYPE_OBJECT:
            return __alignof(LPVOID);
        default:
            Js::Throw::FatalProjectionError();
        }
    }

    // Info:        Checks if the type could be part of this assembly, and if so, retrieves its token.
    // Parameter:   fullTypeName - the type name to get.
    // Returns:     A valid type token if found; or mdTokenNil if not.
    mdTypeDef Assembly::TryGetTypeByName(PCWSTR fullTypeName)
    {
        Assert(properties->sizeName - 1 == wcslen(properties->name));
        Assert(properties->name[properties->sizeName - 1] == 0);

        if (wcsncmp(fullTypeName, properties->name, properties->sizeName - 1) == 0)
        {
            mdTypeDef token;
            HRESULT hr = GetImport()->FindTypeDefByName(fullTypeName, mdTokenNil, &token);
            if (SUCCEEDED(hr))
            {
                TRACE_METADATA(_u("TryGetTypeDefByName(\"%s\") succeeded, result: %d\n"), fullTypeName, token);
                return token;
            }

            TRACE_METADATA(_u("TryGetTypeDefByName(\"%s\") failed\n"), fullTypeName);            
        }
        
        return mdTokenNil;
    }

#if DBG
    inline void PrintCorTypeAttrFlags(DWORD flags)
    {
        auto visibility = (CorTypeAttr)(flags&tdVisibilityMask);
        if(visibility == tdNotPublic) {printf("tdNotPublic ");}
        if(visibility == tdPublic) {printf("tdPublic ");}
        if(visibility == tdNestedPublic) {printf("tdNestedPublic ");}
        if(visibility == tdNestedPrivate) {printf("tdNestedPrivate ");}
        if(visibility == tdNestedFamily) {printf("tdNestedFamily ");}
        if(visibility == tdNestedAssembly) {printf("tdNestedAssembly ");}
        if(visibility == tdNestedFamANDAssem) {printf("tdNestedFamANDAssem ");}
        if(visibility == tdNestedFamORAssem) {printf("tdNestedFamORAssem ");}

        auto layout = (CorTypeAttr)(flags&tdLayoutMask);
        if(layout == tdAutoLayout) {printf("tdAutoLayout ");}
        if(layout == tdSequentialLayout) {printf("tdSequentialLayout ");}
        if(layout == tdExplicitLayout) {printf("tdExplicitLayout ");}

        auto semantics = (CorTypeAttr)(flags&tdClassSemanticsMask);
        if(semantics == tdClass) {printf("tdClass ");}
        if(semantics == tdInterface) {printf("tdInterface ");}

        if(flags&tdAbstract) {printf("tdAbstract ");}
        if(flags&tdSealed) {printf("tdSealed ");}
        if(flags&tdSpecialName) {printf("tdSpecialName ");}
        if(flags&tdImport) {printf("tdImport ");}
        if(flags&tdSerializable) {printf("tdSerializable ");}

        auto stringFormat = (CorTypeAttr)(flags&tdStringFormatMask);
        if(stringFormat == tdAnsiClass) {printf("tdAnsiClass ");}
        if(stringFormat == tdUnicodeClass) {printf("tdUnicodeClass ");}
        if(stringFormat == tdAutoClass) {printf("tdAutoClass ");}
        if(stringFormat == tdCustomFormatClass) {printf("tdCustomFormatClass ");}
        if(stringFormat == tdAnsiClass) {printf("tdAnsiClass ");}

        if(flags&tdBeforeFieldInit) {printf("tdBeforeFieldInit ");}
        if(flags&tdForwarder) {printf("tdForwarder ");}
    }
#endif

    ImmutableList<const MethodProperties*> * TypeDefProperties::GetMethods() const
    {
        if (methods == UNAVAILABLE)
        {
            // Cast around const-ness
            TypeDefProperties * _this = (TypeDefProperties *)this;
            _this->methods = assembly.Methods(td);
        }

        return methods;
    }

    const MethodProperties * TypeDefProperties::GetMethodByName(MetadataStringId nameId, MetadataStringId overloadNameId) const
    {
        auto methods = GetMethods();

        auto result = methods->WhereSingle([&](const Metadata::MethodProperties * method) {
            return method->id == nameId && method->overloadNameId == overloadNameId;
        });
        
#ifdef PROJECTION_METADATA_TRACE
        assembly.Trace(_u("GetMethodByName(%s, %s), result: %s\n"), 
            assembly.stringConverter->StringOfId(nameId), 
            assembly.stringConverter->StringOfId(overloadNameId), 
            assembly.stringConverter->StringOfId(result != nullptr ? result->id : MetadataStringIdNil));
#endif

        return result;
    }

    const MethodProperties * TypeDefProperties::GetMethodByToken(mdMethodDef md) const
    {
        if (md == mdMethodDefNil)
        {
            return nullptr;
        }

        auto methods = GetMethods();

        auto result = methods->WhereSingle([&](const Metadata::MethodProperties * method) {
            return method->mb == md;
        });
        
#ifdef PROJECTION_METADATA_TRACE
        assembly.Trace(_u("GetMethodByToken(%d), result: %s\n"), md, assembly.stringConverter->StringOfId(result != nullptr ? result->id : MetadataStringIdNil));
#endif

        return result;
    }
    
    ImmutableList<const PropertyProperties*> * TypeDefProperties::GetProperties() const
    {
        if (properties == UNAVAILABLE)
        {
            // Cast around const-ness
            TypeDefProperties * _this = (TypeDefProperties *)this;

            auto propertyTokens = this->assembly.PropertyTokens(td);

            _this->properties = propertyTokens->Select<const PropertyProperties*>([&](mdToken td) {
                return this->assembly.GetPropertyProperties2(td, this);
            },assembly.a);
        }

        return properties;
    }
    
    ImmutableList<const EventProperties*> * TypeDefProperties::GetEvents() const
    {
        if (events == UNAVAILABLE)
        {
            // Cast around const-ness
            TypeDefProperties * _this = (TypeDefProperties *)this;

            auto eventTokens = this->assembly.EventTokens(td);

            _this->events = eventTokens->Select<const EventProperties*>([&](mdTypeDef td) {
                return this->assembly.GetEventProperties2(td, this);
            },assembly.a);
        }

        return events;
    }

    ImmutableList<const ParameterProperties*> * MethodProperties::GetParameters() const
    {
        if (parameters == UNAVAILABLE)
        {
            // Cast around const-ness
            MethodProperties * _this = (MethodProperties *)this;

            auto parameterTokens = this->assembly.ParameterTokens(mb);

            _this->parameters = parameterTokens->Select<const ParameterProperties*>([&](mdParamDef pd) {
                return this->assembly.GetParameterProperties(pd);
            },assembly.a);
        }

        return parameters;
    }

    
}
