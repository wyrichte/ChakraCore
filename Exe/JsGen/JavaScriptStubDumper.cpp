//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2010 by Microsoft Corporation.  All rights reserved.
//---------------------------------------------------------------------------
#include "stdafx.h"
#include "JavaScriptStubDumper.h"
#include "Prolog.h"
#include "jskwd.h"

namespace ProjectionModel
{
    enum Pass
    {
        passDeclareBodies,
        passImplementBodies
    };

    struct Context
    {
        Pass pass;
        ProjectionModel::ProjectionBuilder & builder;
        FILE * file;
        ArenaAllocator * alloc;
        bool dumpExtraTypeInfo;
        Context(Pass pass, ProjectionModel::ProjectionBuilder & builder, FILE * file, ArenaAllocator * alloc, bool dumpExtraTypeInfo)
            : pass(pass), builder(builder), file(file), alloc(alloc), dumpExtraTypeInfo(dumpExtraTypeInfo)
        { }
    };

    typedef const Context & CONTEXT;

    void DumpPropertiesObject(CONTEXT, ImmutableList<LPCWSTR> * parent, LPCWSTR typeName, const Metadata::Assembly * assembly, RtPROPERTIESOBJECT propertiesObject, bool dumpHelp);
    void DumpConstructorBody(CONTEXT, ImmutableList<LPCWSTR> * parent, RtFUNCTION constructor);
    void DumpSubnamespace(CONTEXT, ImmutableList<LPCWSTR> * parent, LPCWSTR typeName, const Metadata::Assembly * assembly, RtPROPERTIESOBJECT subnamespace);
    void DumpNamespaceHelp(CONTEXT, ImmutableList<LPCWSTR> * parent, ImmutableList<LPCWSTR> *typeName, RtPROPERTY field);
    void DumpProperties(CONTEXT, ImmutableList<LPCWSTR> * parent, LPCWSTR typeName, const Metadata::Assembly * assembly, ImmutableList<RtPROPERTY> * properties, bool dumpHelp);

#if DBG
    volatile size_t counter = 0;
    volatile size_t checkfor = 0;
    inline size_t IncrementCounter() 
    {
        counter++;
        if (counter == checkfor)
        {
            printf("Counter hit %lu", (unsigned long)checkfor);
        }
        return counter;
    }
#else
    inline size_t IncrementCounter() { return 0; }

#endif

    // Info:        Dump a type name in the xml docid format
    // Parameters:  c - generation context 
    //              type - the type to dump
    void DumpIntellidocTypeName(CONTEXT c, RtTYPE type)
    {
        IncrementCounter();

        // Specs for format of docids
        // http://windows/windows8/docs/Windows%208%20Feature%20Documents/Developer%20Experience%20(DEVX)/DevDiv%20Specs/Org/VSPro/Pro%20Experience/XML%20Documentation%20Comments%20for%20Windows%20Runtime%20Components.docx
        // http://msdn.microsoft.com/en-us/library/fsbx0t7x.aspx

        if (TypeDefinitionType::Is(type))
        {
            auto tdt = TypeDefinitionType::From(type);
            auto typeName = c.builder.stringConverter->StringOfId(tdt->typeDef->id);

            if (wcscmp(typeName, L"Windows.Foundation.PropertyValue") == 0 
                || wcscmp(typeName, L"Windows.Foundation.IPropertyValue") == 0)
            {
                fwprintf(c.file,L"System.Object");
                return;
            }
            
            if (wcscmp(typeName, L"Windows.Foundation.IReference`1") == 0)
            {
                Assert(tdt->genericParameters->Count() == 1);
                DumpIntellidocTypeName(c, tdt->genericParameters->First());
                return;
            }

            if (wcscmp(typeName, L"Windows.Foundation.IReferenceArray`1") == 0)
            {
                Assert(tdt->genericParameters->Count() == 1);
                DumpIntellidocTypeName(c, tdt->genericParameters->First());
                fwprintf(c.file,L"[]");
                return;
            }

            fwprintf(c.file,L"%s",typeName); 
            return;
        }
        switch(type->typeCode)
        {
        case tcVoidType:
            fwprintf(c.file,L"Void");
            return;
        case tcByRefType:
            {
                auto br = ByRefType::From(type);
                DumpIntellidocTypeName(c, br->pointedTo);
                fwprintf(c.file,L"@");
            }
            return;
        case tcSystemGuidType:
            fwprintf(c.file,L"System.String");
            return;
        case tcWindowsFoundationDateTimeType:
            fwprintf(c.file,L"Windows.Foundation.DateTime");
            return;
        case tcWindowsFoundationTimeSpanType:
            fwprintf(c.file,L"Windows.Foundation.TimeSpan");
            return;
        case tcWindowsFoundationEventRegistrationTokenType:
            fwprintf(c.file,L"Windows.Foundation.EventRegistrationToken");
            return;
        case tcWindowsFoundationHResultType:
            fwprintf(c.file,L"Windows.Foundation.HResult");
            return;
        case tcArrayType:
            {
                auto at = ArrayType::From(type);
                DumpIntellidocTypeName(c, at->elementType);
                fwprintf(c.file,L"[]");
                return;
            }
        case tcGenericClassVarType:
            {
                auto gcv = GenericClassVarType::From(type);
                fwprintf(c.file,L"'T%d'", gcv->var->index);
            }
            return;
        case tcMissingNamedType:
            {
                auto stn = MissingNamedType::From(type);
                fwprintf(c.file,L"%s", c.builder.stringConverter->StringOfId(stn->fullTypeNameId)); 
            }
            return;
        case tcGenericParameterType:
            {
                auto gpt = GenericParameterType::From(type);
                fwprintf(c.file,L"`%d", gpt->properties->sequence); 
            }
            return;
        case tcMissingGenericInstantiationType:
            {
                auto gi = MissingGenericInstantiationType::From(type);
                fwprintf(c.file,L"%s",c.builder.stringConverter->StringOfId(gi->parent->fullTypeNameId)); 
            }
            return;
        case tcBasicType:
            {
                auto bt = BasicType::From(type);
                switch(bt->typeCor)
                {
                    case ELEMENT_TYPE_STRING:   
                        fwprintf(c.file,L"System.String");
                        return;
                    case ELEMENT_TYPE_CHAR: 
                        fwprintf(c.file,L"System.Char");
                        return;
                    case ELEMENT_TYPE_BOOLEAN:  
                        fwprintf(c.file,L"System.Boolean");
                        return;
                    case ELEMENT_TYPE_I1:  
                        fwprintf(c.file,L"System.Int8");
                        return;
                    case ELEMENT_TYPE_U1: 
                        fwprintf(c.file,L"System.Byte");
                        return;
                    case ELEMENT_TYPE_I2: 
                        fwprintf(c.file,L"System.Int16");
                        return;
                    case ELEMENT_TYPE_U2: 
                        fwprintf(c.file,L"System.UInt16");
                        return;
                    case ELEMENT_TYPE_I4: 
                        fwprintf(c.file,L"System.Int32");
                        return;
                    case ELEMENT_TYPE_U4: 
                        fwprintf(c.file,L"System.UInt32");
                        return;
                    case ELEMENT_TYPE_I8: 
                        fwprintf(c.file,L"System.Int64");
                        return;
                    case ELEMENT_TYPE_U8: 
                        fwprintf(c.file,L"System.UInt64");
                        return;
                    case ELEMENT_TYPE_R4: 
                        fwprintf(c.file,L"System.Single");
                        return;
                    case ELEMENT_TYPE_R8: 
                        fwprintf(c.file,L"System.Double");
                        return;
                    case ELEMENT_TYPE_OBJECT: 
                        fwprintf(c.file,L"System.Object");
                        return;
                    case ELEMENT_TYPE_VAR: 
                        fwprintf(c.file,L"T"); 
                        return;
                    default: Assert(0);
                }

            }

        }
        Assert(0);
    }

    // Info:        Dump a type name
    // Parameters:  c - generation context 
    //              type - the type to dump
    void DumpTypeName(CONTEXT c, RtTYPE type, bool escapeForXml = true)
    {
        auto leftChevron = escapeForXml ? L"&lt;" : L"<";
        auto rightChevron = escapeForXml ? L"&gt;" : L">";

        if (TypeDefinitionType::Is(type))
        {
            // Check if it is IpropertyValue, PropertyValue or IREference/Array
            auto tdt = TypeDefinitionType::From(type);
            auto typeName = c.builder.stringConverter->StringOfId(tdt->typeDef->id);
            
            if (wcscmp(typeName, L"Windows.Foundation.PropertyValue") == 0 
                || wcscmp(typeName, L"Windows.Foundation.IPropertyValue") == 0)
            {
                fwprintf(c.file,L"Object");
                return;
            }
            
            if (wcscmp(typeName, L"Windows.Foundation.IReference`1") == 0)
            {
                Assert(tdt->genericParameters->Count() == 1);
                DumpTypeName(c, tdt->genericParameters->First(), escapeForXml);
                return;
            }

            if (wcscmp(typeName, L"Windows.Foundation.IReferenceArray`1") == 0)
            {
                Assert(tdt->genericParameters->Count() == 1);
                fwprintf(c.file,L"Array");
                return;
            }

            if (tdt->genericParameters)
            {
                // If the type is generic, strip out the `
                auto positionOfTick = wcscspn(typeName, L"`");
                wchar_t * updatedTypeName = new wchar_t[positionOfTick+1];
                wcsncpy_s(updatedTypeName, positionOfTick+1, typeName, positionOfTick);
                fwprintf(c.file,L"%s%s",updatedTypeName,leftChevron); 
                delete[] updatedTypeName;

                auto dumpTypeName = [&](RtTYPE type) { 
                    DumpTypeName(c,type); 
                };
                tdt->genericParameters->IterateBetween(
                    dumpTypeName,
                    [&](RtTYPE, RtTYPE) {fwprintf(c.file,L",");});
                fwprintf(c.file,L"%s",rightChevron); 
            }
            else
            {
                fwprintf(c.file,L"%s",typeName); 
            }
            return;
        }
        switch(type->typeCode)
        {
        case tcMultiOutType:
            fwprintf(c.file,L"Object");
            return;
        case tcVoidType:
            fwprintf(c.file,L"Void");
            return;
        case tcByRefType:
            {
                auto br = ByRefType::From(type);
                DumpTypeName(c, br->pointedTo);
            }
            return;
        case tcSystemGuidType:
            fwprintf(c.file,L"String");
            return;
        case tcWindowsFoundationDateTimeType:
            fwprintf(c.file,L"Date");
            return;
        case tcWindowsFoundationTimeSpanType:
        case tcWindowsFoundationEventRegistrationTokenType:
        case tcWindowsFoundationHResultType:
            fwprintf(c.file,L"Number");
            return;
        case tcArrayType:
            {
                auto at = ArrayType::From(type);
                if (at->elementType->typeCode != tcBasicType)
                {
                    fwprintf(c.file,L"Array");
                    return;
                }
                auto bt = BasicType::From(at->elementType);
                switch (bt->typeCor)
                {
                case ELEMENT_TYPE_I1:  
                    fwprintf(c.file,L"Int8Array");
                    return;
                case ELEMENT_TYPE_U1:
                    fwprintf(c.file,L"Uint8Array");
                    return;
                case ELEMENT_TYPE_I2:
                    fwprintf(c.file,L"Int16Array");
                    return;
                case ELEMENT_TYPE_U2:
                    fwprintf(c.file,L"Uint16Array");
                    return;
                case ELEMENT_TYPE_I4:
                    fwprintf(c.file,L"Int32Array");
                    return;
                case ELEMENT_TYPE_U4:
                    fwprintf(c.file,L"Uint32Array");
                    return;
                case ELEMENT_TYPE_R4:
                    fwprintf(c.file,L"Float32Array");
                    return;
                case ELEMENT_TYPE_R8:
                    fwprintf(c.file,L"Float64Array");
                    return;
                default:
                    fwprintf(c.file,L"Array");
                    return;
                }
            }
            return;
        case tcGenericClassVarType:
            {
                auto gcv = GenericClassVarType::From(type);
                fwprintf(c.file,L"'T%d'", gcv->var->index);
            }
            return;
        case tcMissingNamedType:
            {
                auto stn = MissingNamedType::From(type);
                fwprintf(c.file,L"%s", c.builder.stringConverter->StringOfId(stn->fullTypeNameId)); 
            }
            return;
        case tcGenericParameterType:
            {
                auto gpt = GenericParameterType::From(type);
                fwprintf(c.file,L"%s", c.builder.stringConverter->StringOfId(gpt->properties->id)); 
            }
            return;
        case tcMissingGenericInstantiationType:
            {
                auto gi = MissingGenericInstantiationType::From(type);
                fwprintf(c.file,L"%s%s",c.builder.stringConverter->StringOfId(gi->parent->fullTypeNameId), leftChevron); 
                gi->genericParameters->IterateBetween(
                    [&](RtTYPE type) { DumpTypeName(c,type); },
                    [&](RtTYPE, RtTYPE) {fwprintf(c.file,L",");});
                fwprintf(c.file,L"%s",rightChevron); 
            }
            return;
        case tcBasicType:
            {
                auto bt = BasicType::From(type);
                switch(bt->typeCor)
                {
                    case ELEMENT_TYPE_STRING:   
                    case ELEMENT_TYPE_CHAR: 
                        fwprintf(c.file,L"String");
                        return;
                    case ELEMENT_TYPE_BOOLEAN:  
                        fwprintf(c.file,L"Boolean");
                        return;
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
                        fwprintf(c.file,L"Number");
                        return;
                    case ELEMENT_TYPE_OBJECT: 
                        fwprintf(c.file,L"Object");
                        return;
                    case ELEMENT_TYPE_VAR: 
                        fwprintf(c.file,L"T"); 
                        return;
                    default: Assert(0);
                }

            }

        }
        Assert(0);
    }

    // Info:        Whether the type is IInspectable
    bool IsInspectable(RtTYPE type)
    {
        if(type->typeCode==tcBasicType)
        {
            auto bt = BasicType::From(type);
            if (bt->typeCor==ELEMENT_TYPE_OBJECT)
            {
                return true;
            }
        }
        return false;
    }

    // Info:        Copy a list of T attributes
    // Parameters:  c - generation context
    //              original - list of T attributes to copy
    //              tail - optional tail of the copied list
    // Returns:     A copy of the original list
    template<class T> ImmutableList<T>* CopyAttributeList(CONTEXT c, ImmutableList<T>* original, ImmutableList<T>** tail = nullptr)
    {
        auto out = ImmutableList<T>::Empty();
        auto tempTail = out;
        if (!original->IsEmpty())
        {
            original->Iterate([&](T attribute) {
                out = out->Append(attribute, c.alloc, &tempTail);
            });
        }

        if (tail != nullptr)
        {
            *tail = tempTail;
        }

        return out;
    }

    // Info:        Combine two lists of T attributes, such that neither list is mutated
    // Parameters:  c - generation context
    //              list1 - first list to combine
    //              list2 - the list to append to the first
    // Returns:     A list containing the elements of the first list followed by the elements of the second
    template<class T> ImmutableList<T>* CombineAttributeLists(CONTEXT c, ImmutableList<T>* list1, ImmutableList<T>* list2)
    {
        auto tail = ImmutableList<T>::Empty();

        // AppendListToCurrentList modifies the list, so get a copy of the first list to append to.
        auto out = CopyAttributeList(c, list1, &tail);
        out = out->AppendListToCurrentList(list2, tail);

        return out;
    }

    // Info:        Compare attribute versions
    // Parameters:  a1 - the first attribute
    //              a2 - the second attribute
    // Returns:     An int that indicates if the first attribute's version is greater, equal to, or less than the second
    template<class T> int CompareAttributeVersions(T* a1, T* a2)
    {
        Assert(a1 != nullptr && a2 != nullptr);
        auto version1 = a1->version;
        auto version2 = a2->version;
        if (version1 > version2)
        {
            return 1;
        }
        else if (version1 < version2)
        {
            return -1;
        }
        return 0;
    }

    // Info:        Dump intellisense comment for a supported on attribute
    // Parameters:  c - generation context
    //              supportedOn - supportedOn attribute to dump
    void DumpSupportedOnIntellisenseComment(CONTEXT c, SupportedOnAttribute supportedOn)
    {
        fwprintf(c.file,L"    /// <compatibleWith platform='");
        switch (supportedOn.platform)
        {
            case Platform::Platform_windows:
                fwprintf(c.file,L"Windows'");
                break;
            case Platform::Platform_windowsPhone:
                fwprintf(c.file,L"WindowsPhoneAppx'");
                break;
            default:
                Assert(FALSE);
                break;
        }
       
        switch (supportedOn.version)
        {
            case 0x6020000:
                fwprintf(c.file,L" minVersion='8.0' />\n");
                break;
            case 0x6030000:
                fwprintf(c.file,L" minVersion='8.1' />\n");
                break;
            default:
                Assert(FALSE);
                break;
        }
    }

    // Info:        Dump intellisense comment for supported on attributes
    // Parameters:  c - generation context
    //              attributes - list of supported on  attributes to dump
    void DumpSupportedOnAttributesIntellisenseComment(CONTEXT c, ImmutableList<SupportedOnAttribute>* attributes)
    {
        if (attributes->IsEmpty())
        {
            return;
        }

        struct CompareSupportedAttributeVersions : public regex::Comparer<SupportedOnAttribute*>
        {
            bool Equals(SupportedOnAttribute* a1, SupportedOnAttribute* a2)
            {
                return Compare(a1,a2) == 0;
            }
            int GetHashCode(SupportedOnAttribute* s)
            {
                Assert(0);
                return 0;
            }
            int Compare(SupportedOnAttribute* a1, SupportedOnAttribute* a2)
            {
                return CompareAttributeVersions(a1, a2);
            }
        };
        CompareSupportedAttributeVersions versionCompare;

        auto supportedOnAttributes =
            // ReverseSortCurrentList modifies the list, so get a copy of the list to sort.
            CopyAttributeList(c, attributes)->ReverseSortCurrentList(&versionCompare);

        // Only dump the first occurence of the supported on attribute for a given platform.
        bool dumpWindows = true;
        bool dumpWindowsPhone = true;

        supportedOnAttributes->IterateWhile([&](SupportedOnAttribute targetAttribute) {
            switch (targetAttribute.platform)
            {
                case Platform::Platform_windows:
                    if (dumpWindows)
                    {
                        DumpSupportedOnIntellisenseComment(c, targetAttribute);
                        dumpWindows = false;
                    }
                    break;
                case Platform::Platform_windowsPhone:
                    if (dumpWindowsPhone)
                    {
                        DumpSupportedOnIntellisenseComment(c, targetAttribute);
                        dumpWindowsPhone = false;
                    }
                    break;
                default:
                    Assert(FALSE);
                    break;
            }

            if (!dumpWindows && !dumpWindowsPhone)
            {
                return false;
            }

            return true;
        });
    }

    // Info:        Dump intellisense comment for a deprecated attribute
    // Parameters:  c - generation context
    //              deprecated - deprecated attribute to dump
    void DumpDeprecatedIntellisenseComment(CONTEXT c, DeprecatedAttribute deprecated)
    {
        fwprintf(c.file,L"    /// <deprecated type='");
        switch (deprecated.deprecateType)
        {
        case DeprecateType::Deprecate_deprecate:
            fwprintf(c.file,L"deprecate'>");
            break;
        case DeprecateType::Deprecate_remove:
            fwprintf(c.file,L"remove'>");
            break;
        default:
            Assert(FALSE);
            break;
        }
        if (deprecated.infoString)
        {
            fwprintf(c.file,deprecated.infoString);
        }
        fwprintf(c.file,L"</deprecated>\n");
    }

    // Info:        Dump intellisense comment for deprecated attributes
    // Parameters:  c - generation context
    //              deprecatedAttributes - list of deprecated attributes to dump
    void DumpDeprecatedAttributesIntellisenseComment(CONTEXT c, ImmutableList<DeprecatedAttribute>* deprecatedAttributes)
    {
        if (deprecatedAttributes->IsEmpty())
        {
            return;
        }

        struct CompareDeprecatedAttributeVersions : public regex::Comparer<DeprecatedAttribute*>
        {
            bool Equals(DeprecatedAttribute* v1, DeprecatedAttribute* v2)
            {
                return Compare(v1,v2) == 0;
            }
            int GetHashCode(DeprecatedAttribute* s)
            {
                Assert(0);
                return 0;
            }
            int Compare(DeprecatedAttribute* a1, DeprecatedAttribute* a2)
            {
                return CompareAttributeVersions(a1, a2);
            }
        };
        CompareDeprecatedAttributeVersions versionCompare;

        DeprecatedAttribute targetAttribute = 
            // ReverseSortCurrentList modifies the list, so get a copy of the list to sort.
            CopyAttributeList(c, deprecatedAttributes)->ReverseSortCurrentList(&versionCompare)->First();
        DumpDeprecatedIntellisenseComment(c,targetAttribute);
    }

    // Info:        Get list of deprecated attributes for property
    // Parameters:  prop - property for which to get attributes
    // Returns:     List of all deprecated attributes for the property
    ImmutableList<DeprecatedAttribute>* GetPropertyDeprecatedAttributes(CONTEXT c, RtABIPROPERTYPROPERTY prop)
    {
        // Add deprecated attributes from the setter first and then from the getter, so any getter deprecated attributes
        // from the same version, take precedence over the setter's deprecated attributes.
        auto setterList = ImmutableList<DeprecatedAttribute>::Empty();
        if (prop->setter.HasValue())
        {
            setterList = prop->setter.GetValue()->deprecatedAttributes;
        }
        auto getterList = ImmutableList<DeprecatedAttribute>::Empty();
        if (prop->getter.HasValue())
        {
            getterList = prop->getter.GetValue()->deprecatedAttributes;
        }

        return CombineAttributeLists(c, setterList, getterList);
    }

    // Info:        Get list of supported on attributes for property
    // Parameters:  prop - property for which to get attributes
    // Returns:     List of all supported on attributes for the property
    ImmutableList<SupportedOnAttribute>* GetPropertySupportedOnAttributes(CONTEXT c, RtABIPROPERTYPROPERTY prop)
    {
        // Add supported on attributes from the setter first and then from the getter, so any getter attributes
        // that are the same, take precedence over the setter's attributes.
        auto setterList = ImmutableList<SupportedOnAttribute>::Empty();
        if (prop->setter.HasValue())
        {
            setterList = prop->setter.GetValue()->supportedOnAttributes;
        }
        auto getterList = ImmutableList<SupportedOnAttribute>::Empty();
        if (prop->getter.HasValue())
        {
            getterList = prop->getter.GetValue()->supportedOnAttributes;
        }

        return CombineAttributeLists(c, setterList, getterList);
    }

    // Info:        Dump intellisense comment for the parameter
    // Parameters:  c - generation context 
    //              parameter - the parameter to dump
    void DumpParameterIntellisenseComment(CONTEXT c, RtPARAMETER parameter)
    {
        auto paramName = c.builder.stringConverter->StringOfId(parameter->id);
        if (!IsInspectable(parameter->type)) // Don't show IInspectable here because 'Object' doesn't match if it is a number
        {
            fwprintf(c.file,L"    /// <param name='%s' type='", paramName);
            DumpTypeName(c,parameter->type);
            fwprintf(c.file,L"'/>\n");
        }
        else
        {
            fwprintf(c.file,L"    /// <param name='%s'/>\n", paramName);
        }
    }

    // Info:        True if the given type is documentable in an .xml file
    bool IsDocumentableType(RtTYPE type)
    {
        if (TypeDefinitionType::Is(type))
        {
            auto tdt = TypeDefinitionType::From(type);
            auto typeName = tdt->typeDef->assembly.stringConverter->StringOfId(tdt->typeDef->id);
            if (wcscmp(typeName, L"Windows.Foundation.PropertyValue") != 0 
                && wcscmp(typeName, L"Windows.Foundation.IPropertyValue") != 0
                && wcscmp(typeName, L"Windows.Foundation.IReference`1") != 0
                && wcscmp(typeName, L"Windows.Foundation.IReferenceArray`1") != 0)
            {
                return true;
            }
        }
        return false;
    }

    // Info:        Dump intellisense comment for parameter list
    // Parameters:  c - generation context 
    //              parameters - the propertiesObject to dump
    void DumpParameterListIntellisenseComment(CONTEXT c, ImmutableList<RtPARAMETER> * parameters, RtTYPE returnType, const Metadata::Assembly * assembly)
    {
        auto inParameters = parameters->Where([&](RtPARAMETER parameter) { // Partition?
            return parameter->isIn;
        },c.alloc);

        inParameters
            ->Iterate([&](RtPARAMETER parameter) {
                DumpParameterIntellisenseComment(c,parameter);
        });

        if (returnType->typeCode!=tcVoidType)
        {
            if (ByRefType::Is(returnType))
            {
                auto brt = ByRefType::From(returnType);
                returnType = brt->pointedTo;
            }

            if (!IsInspectable(returnType))
            {
                fwprintf(c.file,L"    /// <returns type='");
                DumpTypeName(c, returnType);
                if (IsDocumentableType(returnType))
                {
                    auto tdt = TypeDefinitionType::From(returnType);
                    fwprintf(c.file,L"' externalid='T:");
                    DumpIntellidocTypeName(c, returnType);
                    fwprintf(c.file,L"' externalFile='%s.xml", tdt->typeDef->assembly.properties->name);
                }
                fwprintf(c.file,L"'/>\n");
            }
        }
    }

    // Info:        Dump a parameter list in the xml doc format
    // Parameters:  c - generation context 
    //              parameters - the parameters
    void DumpIntellidocParameterList(CONTEXT c, ImmutableList<RtABIPARAMETER> * parameters)
    {
        parameters
            ->IterateBetween(            
            [&](RtPARAMETER param) { DumpIntellidocTypeName(c, param->type); },
            [&](RtPARAMETER, RtPARAMETER) { fwprintf(c.file,L","); });
    }

    // Info:        Dump a xml docid for a method
    // Parameters:  c - generation context 
    //              signature - the signature
    void DumpIntellidocMethodSignature(CONTEXT c, LPCWSTR typeName, RtABIMETHODSIGNATURE signature)
    {
        Assert(typeName);
        fwprintf(c.file,L"M:");
        fwprintf(c.file,L"%s", typeName);
        fwprintf(c.file,L".");
        fwprintf(c.file,L"%s", c.builder.stringConverter->StringOfId(signature->metadataNameId));
        auto parameters = signature->GetParameters()->allParameters
            ->Where([&](RtPARAMETER param) { return param->isIn; }, c.alloc);
        if (parameters)
        {
            fwprintf(c.file,L"(");
            DumpIntellidocParameterList(c, parameters->Cast<RtABIPARAMETER>());
            fwprintf(c.file,L")");
        }
    }

    // Info:        Dump a help keyword for a method
    // Parameters:  c - generation context 
    //              signature - the signature
    void DumpMethodHelpKeyword(CONTEXT c, LPCWSTR typeName, RtABIMETHODSIGNATURE signature)
    {
        Assert(typeName);
        fwprintf(c.file,L"%s", typeName);

        auto methodName = c.builder.stringConverter->StringOfId(signature->metadataNameId);
        // If this is a constructor function, drop the #ctor
        auto startOfctor = wcsstr(methodName, L"#ctor");
        if (NULL == startOfctor)
        {
            fwprintf(c.file,L".");
            fwprintf(c.file,L"%s", methodName);
        }
    }

    // Info:        Dump a xml docid for a type constructor
    // Parameters:  c - generation context 
    //              signature - the signature
    void DumpIntellidocTypeConstructorSignature(CONTEXT c, RtTYPECONSTRUCTORMETHODSIGNATURE signature)
    {
        Assert(IsDocumentableType(signature->parameters->returnType));
        fwprintf(c.file,L"T:");
        DumpIntellidocTypeName(c, signature->parameters->returnType);
    }

    // Info:        Dump a help keyword for a type constructor
    // Parameters:  c - generation context 
    //              signature - the signature
    void DumpTypeConstructorHelpKeyword(CONTEXT c,RtTYPECONSTRUCTORMETHODSIGNATURE signature)
    {
        DumpIntellidocTypeName(c, signature->parameters->returnType);
    }

    // Info:        Dump a property in the xml docid format
    // Parameters:  c - generation context 
    //              typeName - the name of the enclosing type
    //              prop - the property
    void DumpIntellidocPropertySignature(CONTEXT c, LPCWSTR typeName, RtABIPROPERTYPROPERTY prop)
    {
        Assert(typeName);
        fwprintf(c.file,L"P:");
        fwprintf(c.file,L"%s", typeName);
        fwprintf(c.file,L".");
        fwprintf(c.file,L"%s", c.builder.stringConverter->StringOfId(prop->metadataNameId));
    }

    // Info:        Dump a property in the help keyword format
    // Parameters:  c - generation context 
    //              typeName - the name of the enclosing type
    //              prop - the property
    void DumpPropertyHelpKeyword(CONTEXT c, LPCWSTR typeName, RtABIPROPERTYPROPERTY prop)
    {
        Assert(typeName);
        fwprintf(c.file,L"%s", typeName);
        fwprintf(c.file,L".");
        fwprintf(c.file,L"%s", c.builder.stringConverter->StringOfId(prop->metadataNameId));
    }

    // Info:        Dump a field in the xml docid format
    // Parameters:  c - generation context 
    //              typeName - the name of the enclosing type
    //              prop - the field
    void DumpIntellidocFieldSignature(CONTEXT c, LPCWSTR typeName, RtABIFIELDPROPERTY prop)
    {
        Assert(typeName);
        fwprintf(c.file,L"F:");
        fwprintf(c.file,L"%s", typeName);
        fwprintf(c.file,L".");
        fwprintf(c.file,L"%s", c.builder.stringConverter->StringOfId(prop->fieldProperties->id));
    }

    // Info:        Dump an event in the xml docid format
    // Parameters:  c - generation context 
    //              typeName - the name of the enclosing type
    //              prop - the field
    void DumpIntellidocEventSignature(CONTEXT c, LPCWSTR typeName, RtABIEVENTHANDLERPROPERTY prop)
    {
        Assert(typeName);
        fwprintf(c.file,L"E:");
        fwprintf(c.file,L"%s", typeName);
        fwprintf(c.file,L".");
        fwprintf(c.file,L"%s", c.builder.stringConverter->StringOfId(prop->abiEvent->metadataNameId));
    }

    // Info:        Dump an event in the help keyword format
    // Parameters:  c - generation context 
    //              typeName - the name of the enclosing type
    //              prop - the field
    void DumpEventHelpKeyword(CONTEXT c, LPCWSTR typeName, RtABIEVENTHANDLERPROPERTY prop)
    {
        Assert(typeName);
        fwprintf(c.file,L"%s", typeName);
        fwprintf(c.file,L".");
        fwprintf(c.file,L"%s", c.builder.stringConverter->StringOfId(prop->abiEvent->metadataNameId));
    }

    // Info:        Dump intellisense comment for function
    // Parameters:  c - generation context 
    //              dumpHelp - true if we want to dump the help keywords
    void DumpSignatureComment(CONTEXT c, LPCWSTR typeName, const Metadata::Assembly * assembly, RtABIMETHODSIGNATURE signature, bool dumpHelp)
    {
        if (typeName)
        {
            
            fwprintf(c.file,L"    /// <signature ");
            fwprintf(c.file,L"externalid='");
            DumpIntellidocMethodSignature(c, typeName, signature);
            fwprintf(c.file,L"' externalFile='%s.xml' ", assembly->properties->name);

            if (dumpHelp)
            {
                fwprintf(c.file,L"helpKeyword='", assembly->properties->name);
                DumpMethodHelpKeyword(c, typeName, signature);
                fwprintf(c.file,L"'");
            }
            fwprintf(c.file,L">\n");
        }
        else
        {
            fwprintf(c.file,L"    /// <signature>\n");
        }
        auto inParameters = signature->GetParameters()->allParameters->Where([&](RtPARAMETER param) {
            return param->isIn;
        }, c.alloc)->Cast<RtPARAMETER>();
        DumpParameterListIntellisenseComment(c, inParameters, signature->GetParameters()->returnType, assembly);
        DumpDeprecatedAttributesIntellisenseComment(c, signature->deprecatedAttributes);
        DumpSupportedOnAttributesIntellisenseComment(c, signature->supportedOnAttributes);

        fwprintf(c.file,L"    /// </signature>\n");
    }

    // Info:        Dump a struct field in the xml docid format
    // Parameters:  c - generation context 
    //              structType - The struct type
    //              field - the field
    void DumpIntellidocStructField(CONTEXT c, RtSTRUCTTYPE structType, RtABIFIELDPROPERTY field)
    {
        fwprintf(c.file,L"F:");
        DumpIntellidocTypeName(c, structType);
        fwprintf(c.file,L".");
        fwprintf(c.file,L"%s", c.builder.stringConverter->StringOfId(field->fieldProperties->id));
    }  

    // Info:        Dump intellisense comment for function overload group
    // Parameters:  c - generation context 
    //              overloadGroup - the OverloadGroup to dump
    //              dumpHelp - true if we want to dump the help keywords
    void DumpOverloadGroupIntellisenseComment(CONTEXT c, LPCWSTR typeName, const Metadata::Assembly * assembly, RtOVERLOADGROUP overloadGroup, bool dumpHelp)
    {
        // Output overload comment according to this spec:
        // http://venuswiki/(X(1)S(5ildcf55svtqw2uvyqavihnp))/Default.aspx?Page=dev11-jscript&AspxAutoDetectCookieSupport=1

        overloadGroup->overloads->Iterate([&](RtABIMETHODSIGNATURE arityGroup) {
            DumpSignatureComment(c, typeName, assembly, arityGroup, dumpHelp);
        });            
    }

    // Info:        Dump a long name with second and subsequent segments quoted
    // Parameters:  c - generation context 
    //              name - name segments in reverse order
    void DumpSegmentListAsQuotedName(CONTEXT c, ImmutableList<LPCWSTR> * name)
    {
        auto current = name->Reverse(c.alloc);
        fwprintf(c.file,L"%s", current->First());
        current = current->GetTail();

        while(current)
        {
            fwprintf(c.file,L"['%s']",current->First());
            current=current->GetTail();
        }
    }

    // Info:        Dump a long name with second and subsequent segments dotted
    // Parameters:  c - generation context 
    //              name - name segments in reverse order
    void DumpSegmentListAsDottedName(CONTEXT c, ImmutableList<LPCWSTR> * name)
    {
        auto current = name->Reverse(c.alloc);

        // Skip the "rootnamespace" string
        current = current->GetTail();

        // Print the root namespace without a dot trailing
        if (current)
        {
            fwprintf(c.file,L"%s", current->First());
            current = current->GetTail();
        }

        // Now for each sub namespace, print a dot and then the subnamespace name
        while(current)
        {
            fwprintf(c.file,L".%s",current->First());
            current=current->GetTail();
        }
    }

    // Info:        Dump a string as a quoted name.
    // Parameters:  c - generation context 
    //              name - the name to dump
    void DumpStringAsQuotedName(CONTEXT c, LPCWSTR name)
    {
        fwprintf(c.file,L"rootNamespace['");
        while(name[0]!=0x00)
        {
            if (name[0]=='.')
            {
                fwprintf(c.file,L"']['");
            }
            else
            {
                fwprintf(c.file,L"%c",name[0]);
            }
            ++name;
        }
        fwprintf(c.file,L"']");
    }

    // Info:        Dump the generic parameter list as a function definition
    // Parameters:  c - generation context 
    //              genericParameters - generic parameters to dump as function parameters
    void DumpGenericParametersAsFunctionDefinition(CONTEXT c, ImmutableList<RtTYPE> * genericParameters)
    {
        fwprintf(c.file,L"function(");
        genericParameters->IterateBetween(
            [&](RtTYPE type) { 
                Assert(type->typeCode == tcGenericParameterType);
                auto gpt = GenericParameterType::From(type);
                fwprintf(c.file,L"%s", c.builder.stringConverter->StringOfId(gpt->properties->id)); },
                [&](RtTYPE, RtTYPE) {fwprintf(c.file,L","); });
        fwprintf(c.file,L")\n"); 
    }

    // Info:        Dump the method signature as a function definition
    // Parameters:  c - generation context 
    //              signature - the method signature
    void DumpMethodSignatureAsFunctionDefinition(CONTEXT c, RtMETHODSIGNATURE signature)
    {
        fwprintf(c.file,L"function(");
        auto inParameters = 
            signature->GetParameters()->allParameters
                ->Where([&](RtPARAMETER param) { return param->isIn; }, c.alloc);
        inParameters
            ->IterateBetween(
            [&](RtPARAMETER param) {
                auto paramName = c.builder.stringConverter->StringOfId(param->id);
                if(!JsKwd::Is(paramName))
                {
                    fwprintf(c.file,L"%s", paramName); 
                }
                else
                {
                    // The parameter is a JavaScript reserved word. 
                    // Dump it as: <param name>Arg.
                    // There's a slim possibility of a collision if there's another arg with a name <paramName>Arg,
                    // but it doesn't matter when the generated file is not in strict mode. 
                    // We may need to prevent collisions if we ever want to use strict mode.
                    fwprintf(c.file,L"%sArg", paramName); 
                }
            },
            [&](RtPARAMETER, RtPARAMETER) { fwprintf(c.file,L","); });

        fwprintf(c.file,L")\n");
    }

    // Info:        Do something for each signature of the function. There are multiple signatures in the overload case.
    // Parameters:  function - The function to operate on
    //              operation - The function to call
    template<typename TOperation>
    void ForEachSignature(RtFUNCTION function, TOperation operation) 
    {
        switch(function->signature->signatureType)
        {
        case mstOverloadedMethodSignature:
            {
                auto overloadSignature = OverloadedMethodSignature::From(function->signature);
                overloadSignature->overloads->overloads->Iterate([&](RtABIMETHODSIGNATURE arityGroup) {
                    operation(arityGroup);
                });
                return;
            }
        default:
            {
                operation(function->signature);
                return;
            }
        }
    }

    // Info:        Dump the method signature as an addType definition. This is Maru support.
    // Parameters:  c - generation context 
    //              parent - parent name
    //              typeName - name of enclosing type
    //              assembly - assembly of the enclosing type
    //              propertyName - name of the function
    //              kind - the kind of thing to add: 'function', 'event', 'constructor', etc
    //              function - the function to dump
    void DumpFunctionAddType(CONTEXT c, ImmutableList<LPCWSTR> * parent, 
        LPCWSTR typeName, const Metadata::Assembly * assembly, 
        LPCWSTR propertyName, LPCWSTR kind, RtFUNCTION function) 
    {
        fwprintf(c.file,L"addType(");
        DumpSegmentListAsQuotedName(c, parent);
        fwprintf(c.file,L", '%s', '%s', {\n", propertyName, kind);
        fwprintf(c.file,L"    signatures: [\n");

        ForEachSignature(function, [&](RtMETHODSIGNATURE signature) {
            ImmutableList<RtPARAMETER> * parameters = signature->GetParameters()->allParameters->Cast<RtPARAMETER>();
            auto returnType = signature->GetParameters()->returnType;
            if (ByRefType::Is(returnType))
            {
                returnType = ByRefType::From(returnType)->pointedTo;
            }
            auto inParameters = parameters->Where([&](RtPARAMETER parameter) { // Partition?
                return parameter->isIn;
            }, c.alloc);

            fwprintf(c.file,L"        {\n");
            fwprintf(c.file,L"            parameters: [\n");

            inParameters->Iterate([&](RtPARAMETER parameter) {
                fwprintf(c.file,L"                {name: '%s', type:'", c.builder.stringConverter->StringOfId(parameter->id));
                DumpTypeName(c,parameter->type);
                fwprintf(c.file,L"'},\n");
            });

            fwprintf(c.file,L"            ]\n");

            if (returnType->typeCode!=tcVoidType)
            {


                fwprintf(c.file,L"            ,returnType: '");
                DumpTypeName(c, returnType);
                fwprintf(c.file,L"'\n");
                if (IsDocumentableType(returnType))
                {
                    auto tdt = TypeDefinitionType::From(returnType);
                    fwprintf(c.file,L"            ,returnTypeExternalId: 'T:");
                    DumpIntellidocTypeName(c, returnType);
                    fwprintf(c.file,L"'\n");
                    fwprintf(c.file,L"            ,returnTypeExternalFile: '%s.xml'\n", tdt->typeDef->assembly.properties->name);
                }
            }

            switch(signature->signatureType)
            {
                case mstAbiMethodSignature:
                    {
                        auto abiSignature = AbiMethodSignature::From(signature);
                        fwprintf(c.file,L"            ,externalid: '");
                        DumpIntellidocMethodSignature(c, typeName, abiSignature);
                        fwprintf(c.file,L"'\n");
                        fwprintf(c.file,L"            ,externalFile: '%s.xml'\n", assembly->properties->name);
                        fwprintf(c.file,L"            , helpKeyword: '");
                        DumpMethodHelpKeyword(c, typeName, abiSignature);
                        fwprintf(c.file,L"'\n");
                        break;
                    }
                case mstTypeConstructorMethodSignature:
                    {
                        auto typeSignature = TypeConstructorMethodSignature::From(signature);
                        if(IsDocumentableType(typeSignature->parameters->returnType))
                        {
                            fwprintf(c.file,L"            ,externalid: '");
                            DumpIntellidocTypeConstructorSignature(c, typeSignature);
                            fwprintf(c.file,L"'\n");
                            fwprintf(c.file,L"            ,externalFile: '%s.xml'\n", assembly->properties->name);
                            fwprintf(c.file,L"            , helpKeyword: '");
                            DumpTypeConstructorHelpKeyword(c, typeSignature);
                            fwprintf(c.file,L"'\n");
                        }
                        break;
                    }
            }
             
            fwprintf(c.file,L"        },\n");
        });
 
        fwprintf(c.file,L"    ]\n");

        if(function->functionType == functionRuntimeClassConstructor) {
            auto constructor = RuntimeClassConstructor::From(function);
            fwprintf(c.file,L"    ,interfaces: [\n");
            constructor->allInterfaces->Iterate(
                [&](RtINTERFACECONSTRUCTOR iface) {
                    fwprintf(c.file,L"        '");
                    auto returnType = iface->signature->GetParameters()->returnType;
                    DumpTypeName(c, returnType);
                    fwprintf(c.file,L"' ,\n");
                }
            );

            fwprintf(c.file,L"    ]\n");
        }

        fwprintf(c.file,L"});\n");
    }

    // Info:        Dump the property as an addType definition. This is Maru support.
    // Parameters:  c - generation context 
    //              parent - parent name
    //              typeName - name of enclosing type
    //              assembly - assembly of the enclosing type
    //              prop - the property
    void DumpPropertyAddType(CONTEXT c, ImmutableList<LPCWSTR> * parent, LPCWSTR typeName, const Metadata::Assembly * assembly, RtABIPROPERTYPROPERTY prop) 
    {
        fwprintf(c.file,L"addType(");
        DumpSegmentListAsQuotedName(c, parent);
        fwprintf(c.file,L", '%s', '%s', {\n", c.builder.stringConverter->StringOfId(prop->identifier), L"property");
        fwprintf(c.file,L"    returnType: '%s'\n", c.builder.stringConverter->StringOfId(prop->GetPropertyType()->fullTypeNameId));
        fwprintf(c.file,L"    ,externalid: '");
        DumpIntellidocPropertySignature(c, typeName, prop);
        fwprintf(c.file,L"'\n");
        fwprintf(c.file,L"    ,externalFile: '%s.xml'\n", assembly->properties->name);
        fwprintf(c.file,L"    ,helpKeyword: '");
        DumpPropertyHelpKeyword(c, typeName, prop);
        fwprintf(c.file,L"'\n");
        fwprintf(c.file,L"});\n");
    }

    // Info:        Dump the event as an addType definition. This is Maru support.
    // Parameters:  c - generation context 
    //              parent - parent name
    //              typeName - name of enclosing type
    //              assembly - assembly of the enclosing type
    //              eventHandler - the event
    void DumpEventAddType(CONTEXT c, ImmutableList<LPCWSTR> * parent, LPCWSTR typeName, const Metadata::Assembly * assembly, RtABIEVENTHANDLERPROPERTY eventHandler) 
    {
        fwprintf(c.file,L"addType(");
        DumpSegmentListAsQuotedName(c, parent);
        fwprintf(c.file,L", '%s', 'event', {\n", c.builder.stringConverter->StringOfId(eventHandler->identifier));
        fwprintf(c.file,L"    externalid: '");
        DumpIntellidocEventSignature(c, typeName, eventHandler);
        fwprintf(c.file,L"'\n");
        fwprintf(c.file,L"     ,externalFile: '%s.xml'\n", assembly->properties->name);
        fwprintf(c.file,L"    ,helpKeyword: '");
        DumpEventHelpKeyword(c, typeName, eventHandler);
        fwprintf(c.file,L"'\n");        
        fwprintf(c.file,L"});\n");
    }

    // Info:        Dump a type constructor
    // Parameters:  c - generation context 
    //              type - the type to dump
    void DumpTypeConstructor(CONTEXT c, RtTYPE type)
    {
        if (TypeDefinitionType::Is(type))
        {
            // Check if it is IpropertyValue, PropertyValue or IReference/Array
            auto tdt = TypeDefinitionType::From(type);
            auto typeName = c.builder.stringConverter->StringOfId(tdt->typeDef->id);
            
            if (wcscmp(typeName, L"Windows.Foundation.PropertyValue") == 0 
                || wcscmp(typeName, L"Windows.Foundation.IPropertyValue") == 0)
            {
                fwprintf(c.file,L"Object");
                return;
            }
            
            if (wcscmp(typeName, L"Windows.Foundation.IReference`1") == 0)
            {
                Assert(tdt->genericParameters->Count() == 1);
                DumpTypeConstructor(c, tdt->genericParameters->First());
                return;
            }

            if (wcscmp(typeName, L"Windows.Foundation.IReferenceArray`1") == 0)
            {
                Assert(tdt->genericParameters->Count() == 1);
                fwprintf(c.file,L"Array");
                return;
            }

            DumpStringAsQuotedName(c, typeName);
            if (tdt->genericParameters)
            {
                fwprintf(c.file,L"(");

                auto dumpTypeConstructor = [&](RtTYPE type) { 
                    DumpTypeConstructor(c,type); 
                };
                tdt->genericParameters->IterateBetween(
                    dumpTypeConstructor,
                    [&](RtTYPE, RtTYPE) {fwprintf(c.file,L",");});
                fwprintf(c.file,L")"); 
            }
            return;
        }
        switch(type->typeCode)
        {
        case tcMultiOutType:
            fwprintf(c.file,L"Object");
            return;
        case tcVoidType: return;
        case tcByRefType:
            {
                auto br = ByRefType::From(type);
                DumpTypeConstructor(c, br->pointedTo);
            }
            return;
        case tcSystemGuidType:
            fwprintf(c.file,L"String");
            return;
        case tcWindowsFoundationDateTimeType:
            fwprintf(c.file,L"Date");
            return;
        case tcWindowsFoundationTimeSpanType:
        case tcWindowsFoundationEventRegistrationTokenType:
        case tcWindowsFoundationHResultType:
            fwprintf(c.file,L"Number");
            return;
        case tcArrayType:
            {
                auto at = ArrayType::From(type);
                if (at->elementType->typeCode != tcBasicType)
                {
                    fwprintf(c.file,L"Array");
                    return;
                }
                auto bt = BasicType::From(at->elementType);
                switch (bt->typeCor)
                {
                case ELEMENT_TYPE_I1:  
                    fwprintf(c.file,L"Int8Array");
                    return;
                case ELEMENT_TYPE_U1:
                    fwprintf(c.file,L"Uint8Array");
                    return;
                case ELEMENT_TYPE_I2:
                    fwprintf(c.file,L"Int16Array");
                    return;
                case ELEMENT_TYPE_U2:
                    fwprintf(c.file,L"Uint16Array");
                    return;
                case ELEMENT_TYPE_I4:
                    fwprintf(c.file,L"Int32Array");
                    return;
                case ELEMENT_TYPE_U4:
                    fwprintf(c.file,L"Uint32Array");
                    return;
                case ELEMENT_TYPE_R4:
                    fwprintf(c.file,L"Float32Array");
                    return;
                case ELEMENT_TYPE_R8:
                    fwprintf(c.file,L"Float64Array");
                    return;
                default:
                    fwprintf(c.file,L"Array");
                    return;
                }
            }
            return;
        case tcGenericClassVarType:
            {
                // No type constructor available for this type, use Object constructor.
                auto gcv = GenericClassVarType::From(type);
                fwprintf(c.file,L"/*T%d*/Object", gcv->var->index);
            }
            return;
        case tcMissingNamedType:
            {
                auto stn = MissingNamedType::From(type);
                fwprintf(c.file,L"/* missing named type, assumed external */ "); 
                DumpStringAsQuotedName(c, c.builder.stringConverter->StringOfId(stn->fullTypeNameId));
            }
            return;
        case tcGenericParameterType:
            {
                auto gpt = GenericParameterType::From(type);
                switch (c.pass) 
                {
                case passDeclareBodies:
                    fwprintf(c.file,L"%s", c.builder.stringConverter->StringOfId(gpt->properties->id)); 
                    break;
                case passImplementBodies:
                    // For pass 2, we are generating the stub for ['Type']['prototype']. 
                    // We do not have type constructors for the generic parameters at this stage, 
                    // so we return the Object constructor.
                    fwprintf(c.file,L"Object");
                    break;
                }
            }
            return;
        case tcMissingGenericInstantiationType:
            {
                auto gi = MissingGenericInstantiationType::From(type);
                fwprintf(c.file,L"/* missing generic instantiation type, assumed external */ ");
                DumpStringAsQuotedName(c, c.builder.stringConverter->StringOfId(gi->parent->fullTypeNameId));
                fwprintf(c.file,L"("); 
                gi->genericParameters->IterateBetween(
                    [&](RtTYPE type) { DumpTypeConstructor(c,type); },
                    [&](RtTYPE, RtTYPE) {fwprintf(c.file,L",");});
                fwprintf(c.file,L")"); 
            }
            return;
        case tcBasicType:
            {
                auto bt = BasicType::From(type);
                switch(bt->typeCor)
                {
                    case ELEMENT_TYPE_STRING:   
                    case ELEMENT_TYPE_CHAR: 
                        fwprintf(c.file,L"String");
                        return;
                    case ELEMENT_TYPE_BOOLEAN:  
                        fwprintf(c.file,L"Boolean");
                        return;
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
                        fwprintf(c.file,L"Number");
                        return;
                    case ELEMENT_TYPE_OBJECT: 
                    case ELEMENT_TYPE_VAR: 
                        fwprintf(c.file,L"Object");
                        return;
                    default: Assert(0);
                }

            }

        }
        Assert(0);
    }

    // Info:        Dump code that will instantiate an instance of the given type
    // Parameters:  c - generation context 
    //              type - the type
    void DumpTypeInstance(CONTEXT c, RtTYPE type)
    {
        if (TypeDefinitionType::Is(type))
        {
            auto tdt = TypeDefinitionType::From(type);
            auto typeName = c.builder.stringConverter->StringOfId(tdt->typeDef->id);

            if (wcscmp(typeName, L"Windows.Foundation.PropertyValue") == 0 
                || wcscmp(typeName, L"Windows.Foundation.IPropertyValue") == 0)
            {
                fwprintf(c.file,L"new Object()");
                return;
            }
            
            if (wcscmp(typeName, L"Windows.Foundation.IReference`1") == 0)
            {
                Assert(tdt->genericParameters->Count() == 1);
                DumpTypeInstance(c, tdt->genericParameters->First());
                return;
            }

            if (wcscmp(typeName, L"Windows.Foundation.IReferenceArray`1") == 0)
            {
                Assert(tdt->genericParameters->Count() == 1);
                fwprintf(c.file,L"[");
                DumpTypeInstance(c, tdt->genericParameters->First());
                fwprintf(c.file,L"]");
                return;
            }



            if (EnumType::Is(type))
            {
                fwprintf(c.file,L"/*enum - %s*/ 0", typeName);
                return;
            }

            fwprintf(c.file,L"(new ");
            DumpStringAsQuotedName(c, typeName);
            if (tdt->genericParameters)
            {
                // If there are generic parameters, pass the type constructors for those types.
                fwprintf(c.file,L"("); 
                tdt->genericParameters->IterateBetween(
                    [&](RtTYPE type) { DumpTypeConstructor(c, type); },
                    [&](RtTYPE, RtTYPE) {fwprintf(c.file,L",");});
                fwprintf(c.file,L")");
            }
            fwprintf(c.file,L"())");
            if (DelegateType::Is(type))
            {
                fwprintf(c.file,L".invoke");
                return;
            }
            return;
        }
        switch(type->typeCode)
        {
        case tcByRefType:
            {
                auto br = ByRefType::From(type);
                DumpTypeInstance(c, br->pointedTo);
            }
            return;
        case tcVoidType:return;
        case tcSystemGuidType:
            fwprintf(c.file,L"'{18E327A7-7290-431c-BF3D-9C2567114CAB}'");
            return;
        case tcWindowsFoundationDateTimeType:
            fwprintf(c.file,L"new Date()");
            return;
        case tcWindowsFoundationTimeSpanType:
        case tcWindowsFoundationEventRegistrationTokenType:
        case tcWindowsFoundationHResultType:
            fwprintf(c.file,L"0"); 
            return;
        case tcArrayType:
            {
                auto arr = ArrayType::From(type);
                if (arr->elementType->typeCode != tcBasicType)
                {
                    fwprintf(c.file,L"[");
                    DumpTypeInstance(c, arr->elementType);
                    fwprintf(c.file,L"]");
                    return;
                }
                auto bt = BasicType::From(arr->elementType);
                switch (bt->typeCor)
                {
                case ELEMENT_TYPE_I1:  
                    fwprintf(c.file,L"new Int8Array(1)");
                    return;
                case ELEMENT_TYPE_U1:
                    fwprintf(c.file,L"new Uint8Array(1)");
                    return;
                case ELEMENT_TYPE_I2:
                    fwprintf(c.file,L"new Int16Array(1)");
                    return;
                case ELEMENT_TYPE_U2:
                    fwprintf(c.file,L"new Uint16Array(1)");
                    return;
                case ELEMENT_TYPE_I4:
                    fwprintf(c.file,L"new Int32Array(1)");
                    return;
                case ELEMENT_TYPE_U4:
                    fwprintf(c.file,L"new Uint32Array(1)");
                    return;
                case ELEMENT_TYPE_R4:
                    fwprintf(c.file,L"new Float32Array(1)");
                    return;
                case ELEMENT_TYPE_R8:
                    fwprintf(c.file,L"new Float64Array(1)");
                    return;
                default:
                    fwprintf(c.file,L"[");
                    DumpTypeInstance(c, arr->elementType);
                    fwprintf(c.file,L"]");
                    return;
                }
            }
            return;
        case tcGenericClassVarType:
            {
                auto gcv = GenericClassVarType::From(type);
                fwprintf(c.file,L"'T%d'", gcv->var->index);
            }
            return;
        case tcGenericParameterType:
            {
                auto gpt = GenericParameterType::From(type);
                switch (c.pass)
                {
                case passDeclareBodies:
                    // This is being generated for the type constructor prototype (make.prototype) and should use the constructor argument passed to the generic type.
                    fwprintf(c.file,L"(new %s()).valueOf()", c.builder.stringConverter->StringOfId(gpt->properties->id)); 
                    break;
                case passImplementBodies:
                    // For pass 2, we are generating the stub for ['Type']['prototype'] so we can provide the correct intellisense.
                    // We do not have type constructors for the generic parameters at this stage, so we return object.
                    fwprintf(c.file,L"/*%s*/{}", c.builder.stringConverter->StringOfId(gpt->properties->id)); 
                    break;
                }
            }
            return;
        case tcMissingNamedType:
            {
                auto stn = MissingNamedType::From(type);
                fwprintf(c.file,L"/* missing named type, assumed external */ new "); 
                DumpStringAsQuotedName(c, c.builder.stringConverter->StringOfId(stn->fullTypeNameId));
                fwprintf(c.file,L"()"); 
            }
            return;
        case tcMissingGenericInstantiationType:
            {
                auto gi = MissingGenericInstantiationType::From(type);
                fwprintf(c.file,L"/* missing generic instantiation type, assumed external */ new ");
                DumpStringAsQuotedName(c, c.builder.stringConverter->StringOfId(gi->parent->fullTypeNameId));
                if (gi->genericParameters)
                {
                    fwprintf(c.file,L"("); 
                    gi->genericParameters->IterateBetween(
                        [&](RtTYPE type) { DumpTypeConstructor(c, type); },
                        [&](RtTYPE, RtTYPE) {fwprintf(c.file,L",");});
                    fwprintf(c.file,L")"); 
                }
                fwprintf(c.file,L"()");
            }
            return;
        case tcBasicType:
            {
                auto bt = BasicType::From(type);
                switch(bt->typeCor)
                {
                    case ELEMENT_TYPE_CHAR: fwprintf(c.file,L"'_'"); return;
                    case ELEMENT_TYPE_STRING:   fwprintf(c.file,L"''"); return;
                    case ELEMENT_TYPE_BOOLEAN:  fwprintf(c.file,L"true"); return;
                    case ELEMENT_TYPE_I1:  
                    case ELEMENT_TYPE_U1: 
                    case ELEMENT_TYPE_I2: 
                    case ELEMENT_TYPE_U2: 
                    case ELEMENT_TYPE_I4: 
                    case ELEMENT_TYPE_U4: 
                    case ELEMENT_TYPE_I8: 
                    case ELEMENT_TYPE_U8: 
                        fwprintf(c.file,L"0"); 
                        return;
                    case ELEMENT_TYPE_R4: 
                    case ELEMENT_TYPE_R8: 
                        fwprintf(c.file,L"1.1"); 
                        return;
                    case ELEMENT_TYPE_OBJECT: 
                        fwprintf(c.file,L"{}"); 
                        return;
                    case ELEMENT_TYPE_VAR: 
                        fwprintf(c.file,L"{}"); 
                        return;
                    default: Assert(0);
                }

            }

        }
        Assert(0);
    }

    // Info:        Dump code that will instantiate an instance of the given type
    // Parameters:  c - generation context 
    //              type - the type
    void DumpReturnTypeInstance(CONTEXT c, RtTYPE type, ImmutableList<RtPARAMETER> * parameters)
    {
        if (MultiOutType::Is(type))
        {
            fwprintf(c.file,L"{"); 
            parameters->Where([](RtPARAMETER param) {
                return !param->isIn;
            },c.alloc)            ->IterateBetween(            
            [&](RtPARAMETER param) { 
                fwprintf(c.file,L"%s:", c.builder.stringConverter->StringOfId(param->id));
                DumpTypeInstance(c, param->type); 
            },
            [&](RtPARAMETER, RtPARAMETER) { fwprintf(c.file,L","); });

            fwprintf(c.file,L"}"); 
            return;
        }
        return DumpTypeInstance(c, type);
    }

    // Info:        Dump a specialized function body if the method is one we override
    // Parameters:  c - generation context 
    //              method - the method to check
    // Returns:     A boolean value indicating whether a specialized function body was 
    //              emitted for this signature.
    bool DumpSpecialFunctionBody(CONTEXT c, RtABIMETHODSIGNATURE method)
    {
        if (method && method->iid->piid == IID_IIterator1)
        {
            if (wcscmp(c.builder.stringConverter->StringOfId(method->metadataNameId), L"MoveNext") == 0)
            {
                // Override the "moveNext" method for all iterators to set an internal property
                // marking the end of the collection.
                fwprintf(c.file,L"this._$endOfCollection = true; return this.hasCurrent");
                return true;
            }
            else if (wcscmp(c.builder.stringConverter->StringOfId(method->metadataNameId), L"get_HasCurrent") == 0)
            {
                // Override the "hasCurrent" property getter for all iterators to refer to the internal property
                // set by the "moveNext" method.
                fwprintf(c.file,L"return !this._$endOfCollection");
                return true;
            }
        }
        
        // There is no special function body for this method signature
        return false;
    }

    // Info:        Dump a type instance of the given arity group
    // Parameters:  c - generation context 
    //              arityGroup - the type
    void DumpArityGroupInstance(CONTEXT c, RtABIMETHODSIGNATURE arityGroup)
    {
        fwprintf(c.file,L"if(arguments.length>=%d) {", (int)arityGroup->inParameterCount); // Explicit cast to int is because size_t will be 64 bits on amd64 and OACR will warn
        // Check for specialized function body
        if (!DumpSpecialFunctionBody(c, arityGroup))
        {
            // Otherwise, default to returning an instance of the return type
            fwprintf(c.file,L"return ");
            DumpReturnTypeInstance(c, arityGroup->GetParameters()->returnType, arityGroup->GetParameters()->allParameters);
        }
        fwprintf(c.file,L";}\n");
    }

    // Info:        Dump a type instance of the given overload group
    // Parameters:  c - generation context 
    //              overloadGroup - the type
    void DumpOverloadInstance(CONTEXT c, RtOVERLOADGROUP overloadGroup)
    {
        fwprintf(c.file,L"\n");
        overloadGroup->overloads->Iterate([&](RtABIMETHODSIGNATURE arityGroup) {
            DumpArityGroupInstance(c, arityGroup);
        }); 
    }

    // Info:        Dump a function as an instance
    // Parameters:  c - generation context 
    //              typeName - name of the enclosing type
    //              assembly - assembly of the enclosing type
    //              function - the function
    //              dumpHelp - true if we want to dump the help keywords
    void DumpFunctionInstance(CONTEXT c, LPCWSTR typeName, const Metadata::Assembly * assembly, RtFUNCTION function, bool dumpHelp)
    {
        switch(function->functionType)
        {
        case functionAbiMethod:
            {
                auto abiMethod = AbiMethod::From(function);
                DumpMethodSignatureAsFunctionDefinition(c, abiMethod->signature);
                fwprintf(c.file,L" {\n");
                DumpSignatureComment(c, typeName, assembly, abiMethod->signature, dumpHelp);
                // Check for a specialized function body
                if (!DumpSpecialFunctionBody(c, abiMethod->signature))
                {
                    // Otherwise, default to returning an instance of the return type
                    fwprintf(c.file,L" return ");
                    DumpReturnTypeInstance(c, abiMethod->signature->GetParameters()->returnType, abiMethod->signature->GetParameters()->allParameters);
                }
                fwprintf(c.file,L";}");
            }
            return;
        case functionOverloadGroupConstructor:
            {
                auto overloadConstructor = OverloadGroupConstructor::From(function);
                DumpMethodSignatureAsFunctionDefinition(c, overloadConstructor->signature);
                fwprintf(c.file,L" {\n");
                DumpOverloadGroupIntellisenseComment(c, typeName, assembly,  overloadConstructor->signature->overloads, dumpHelp);
                DumpOverloadInstance(c, overloadConstructor->signature->overloads);
                fwprintf(c.file,L";}");
            }
            return;
        }
        Assert(0);
    }

    // Info:        Dump a member body as an instance
    // Parameters:  c - generation context 
    //              typeName - name of the enclosing type
    //              assembly - assembly of the enclosing type
    //              expr - the function body
    //              dumpHelp - true if we want to dump the help keywords
    void DumpMemberBody(CONTEXT c, LPCWSTR typeName, const Metadata::Assembly * assembly, RtEXPR expr, bool dumpHelp)
    {
        switch(expr->type)
        {
        case exprUInt32Literal:
            fwprintf(c.file,L"%u", UInt32Literal::From(expr)->value);
            return;
        case exprInt32Literal:
            fwprintf(c.file,L"%d", Int32Literal::From(expr)->value);
            return;
        case exprNullLiteral:
            fwprintf(c.file,L"null");
            return;
        case exprFunction:
            DumpFunctionInstance(c, typeName, assembly, Function::From(expr), dumpHelp);
            return;
        }
        Assert(0);
    }

    // Info:        Dump a member body as an instance
    // Parameters:  c - generation context 
    //              abiProp - the ABI property for which to dump a getter function
    void DumpPropertyGetter(CONTEXT c, RtABIPROPERTYPROPERTY abiProp)
    {
        // Check for a specialized function body
        if (!abiProp->getter.HasValue() || !DumpSpecialFunctionBody(c, abiProp->getter.GetValue()))
        {
            // Otherwise, default to returning an instance of the property type
            fwprintf(c.file,L"return ");
            DumpTypeInstance(c, abiProp->GetPropertyType());
        }
    }

    // Info:        Define a property
    // Parameters:  c - generation context 
    //              fobject - called to dump the name of the object
    //              propertyName - the property to add to the object
    //              fpropertyValue - called to dump the property value
    template<typename FObject, typename FPropertyValue>
    void DumpDefineProperty(CONTEXT c, FObject fobject, LPCWSTR propertyName, FPropertyValue fpropertyValue)
    {
        fwprintf(c.file,L"dp(");
        fobject();
        fwprintf(c.file,L",'%s', ", propertyName);
        fpropertyValue();
        fwprintf(c.file,L");\n");
    }

    // Info:        Define a property getter
    // Parameters:  c - generation context 
    //              fobject - called to dump the name of the object
    //              propertyName - the property to add to the object
    //              fpropertyValue - called to dump the property value
    template<typename FObject, typename FPropertyValue>
    void DumpDefinePropertyGetter(CONTEXT c, FObject fobject, LPCWSTR propertyName, FPropertyValue fpropertyValue)
    {
        fwprintf(c.file,L"dpg(");
        fobject();
        fwprintf(c.file,L",'%s', function() { ", propertyName);
        fpropertyValue();
        fwprintf(c.file,L";});\n");
    }

    // Info:        Define a property getter and setter
    // Parameters:  c - generation context 
    //              fobject - called to dump the name of the object
    //              propertyName - the property to add to the object
    //              fpropertyValue - called to dump the property value
    template<typename FObject, typename FPropertyValue, typename FPropertySetter>
    void DumpDefinePropertyGetterSetter(CONTEXT c, FObject fobject, LPCWSTR propertyName, FPropertyValue fpropertyValue, FPropertySetter fpropertySetter)
    {
        fwprintf(c.file,L"dpg(");
        fobject();
        fwprintf(c.file,L",'%s', function() { return ", propertyName);
        fpropertyValue();
        fwprintf(c.file,L";},");
        fpropertySetter();
        fwprintf(c.file,L");\n");
    }

    // Info:        Dump the definition of an enum
    // Parameters:  c - generation context 
    //              fobject - called to dump the name of the object
    //              propertyName - the property to add to the object
    //              fpropertyValue - called to dump the property value
    void DumpEnumBody(CONTEXT c, ImmutableList<LPCWSTR> * parent, RtENUM _enum)
    {
        fwprintf(c.file,L"(function () { return /* enum */ {\n");
        _enum->properties->fields->IterateBetween(
            [&](RtPROPERTY prop) {
                auto field = AbiFieldProperty::From(prop);
                auto enumName = c.builder.stringConverter->StringOfId(_enum->typeDef->id);

                fwprintf(c.file,L"    /// <field name='%s' type='Number'", c.builder.stringConverter->StringOfId(field->identifier));
                fwprintf(c.file,L" externalid='");
                DumpIntellidocFieldSignature(c, enumName, field);

                // For enums, the help keyword is the enum type name, not the enum field name
                fwprintf(c.file, L"' externalFile='%s.xml' helpKeyword='%s'", _enum->typeDef->assembly.properties->name, enumName);

                auto supportedOnAttributes = _enum->supportedOnAttributes;
                if (!supportedOnAttributes->IsEmpty())
                {
                    fwprintf(c.file, L">\n");
                    DumpSupportedOnAttributesIntellisenseComment(c, supportedOnAttributes);
                    fwprintf(c.file, L"    /// </field>\n");
                }
                else
                {
                    fwprintf(c.file, L"/>\n");			
                }

                fwprintf(c.file, L"    '%s':", c.builder.stringConverter->StringOfId(field->identifier));
                DumpMemberBody(c, enumName, &_enum->typeDef->assembly, field->expr, true);
            },
            [&](RtPROPERTY, RtPROPERTY) {fwprintf(c.file,L",\n");}        
        );
        fwprintf(c.file,L"\n};})()\n");
    }

    // Info:        Dump the help keyword for the enum type.
    // Parameters:  c - generation context 
    //              parent - the parent object
    //              enumPropertyName - the property to add to the object
    //              _enum - the enum object
    void DumpEnumHelp(CONTEXT c, ImmutableList<LPCWSTR> * parent, LPCWSTR enumPropertyName, RtENUM _enum)
    {
        // We need to use the language service helper intellisense to annotate the keyword on the enum type,
        // since the enum itself is not a function but just an object
        fwprintf(c.file,L"if(rootNamespace.intellisense) {rootNamespace.intellisense.annotate(");
        DumpSegmentListAsQuotedName(c, parent);
        fwprintf(c.file,L", {\n");

        auto enumName = c.builder.stringConverter->StringOfId(_enum->typeDef->id);
        fwprintf(c.file,L"    /// <field type='%s'", enumName);
        fwprintf(c.file,L" externalid='T:%s", enumName);
        fwprintf(c.file,L"' externalFile='%s.xml' helpKeyword='%s'/>\n", _enum->typeDef->assembly.properties->name, enumName);
        fwprintf(c.file,L"    '%s':undefined\n});}\n", enumPropertyName);
    }

    // Info:        Dump an event parameter 
    // Parameters:  c - generation context 
    //              evnt - the event
    void DumpEventParameter(CONTEXT c, RtEVENT evnt)
    {
        auto addOn = AbiMethodSignature::From(evnt->addOn);
        auto handlerparameter = AbiParameter::From(addOn->GetParameters()->allParameters->First());
        if (DelegateType::Is(handlerparameter->type))
        {
            auto handlertype = DelegateType::From(handlerparameter->type);
            auto handlerexpr = c.builder.ExprOfToken(handlertype->typeId, handlertype->typeDef->td, handlertype->typeDef->assembly, handlertype->genericParameters); 
            auto handlerfunction = Function::From(handlerexpr);
            if (DelegateConstructor::Is(handlerfunction))
            {
                auto handlerdelegate = DelegateConstructor::From(handlerfunction);
                if (handlerdelegate->invokeInterface)
                {
                    auto invokeinterface = RuntimeInterfaceConstructor::From(handlerdelegate->invokeInterface);
                    auto invokemethod = AbiMethodProperty::From(invokeinterface->ownProperties->First());
                    auto invokemethodbody = AbiMethod::From(invokemethod->body);
                    auto invokemethodparameters = invokemethodbody->signature->GetParameters();
                    auto inparameters = invokemethodparameters->allParameters->Where([&](RtPARAMETER param) { return param->isIn; }, c.alloc);
                    auto index = 0;
                    while(inparameters)
                    {
                        if (index>0)
                        {
                            fwprintf(c.file,L",");
                        }
                        // Double the second parameter so that we can have an unmodified version for 'detail'
                        if (index==1)
                        {
                            fwprintf(c.file,L"[");
                            DumpTypeInstance(c, inparameters->First()->type);
                            fwprintf(c.file,L",");
                            DumpTypeInstance(c, inparameters->First()->type);
                            fwprintf(c.file,L"]");
                        }
                        else
                        {
                            DumpTypeInstance(c, inparameters->First()->type);
                        }
                        ++index;
                        inparameters = inparameters->GetTail();
                    }
                }
            }
        }
    }

    // Info:        Dump a property
    // Parameters:  c - generation context 
    //              parent - name of parent
    //              typeName - the enclosing type name
    //              assembly - assembly of the enclosing type
    //              field - the property
    //              dumpHelp - true if we want to dump the help keywords
    void DumpProperty(CONTEXT c, ImmutableList<LPCWSTR> * parent, LPCWSTR typeName, const Metadata::Assembly * assembly, RtPROPERTY field, bool dumpHelp)
    {
        switch(field->propertyType)
        {
        case ptAbiPropertyProperty:
            {
                auto prop = AbiPropertyProperty::From(field);
                DumpDefinePropertyGetter(
                    c,
                    [&](){ DumpSegmentListAsQuotedName(c, parent); }, 
                    c.builder.stringConverter->StringOfId(field->identifier),
                    [&](){ DumpPropertyGetter(c, prop); });

                if(c.dumpExtraTypeInfo) {
                    DumpPropertyAddType(c, parent, typeName, assembly, prop);
                }

                if(c.dumpExtraTypeInfo && field->expr->type == exprFunction) {
                    DumpFunctionAddType(
                        c,
                        parent,
                        typeName,
                        assembly,
                        c.builder.stringConverter->StringOfId(field->identifier),
                        L"function",
                        Function::From(field->expr));
                }
            }
            break;
        case ptAbiAddEventListenerProperty:
            {
                auto add = AbiAddEventListenerProperty::From(field);

                auto value = [&](){
                    fwprintf(c.file,L"function(event,handler) {\n");
                    fwprintf(c.file,L"/// <param name='event' type='String'/>\n");
                    fwprintf(c.file,L"/// <param name='handler' type='Function'/>\n");
                    add->events->Iterate([&](RtEVENT evnt) {
                        fwprintf(c.file,L"if(event=='%s') { handler(eventParamOf(", c.builder.stringConverter->StringOfId(evnt->nameId));
                        DumpEventParameter(c, evnt);
                        fwprintf(c.file,L")); return; }\n", c.builder.stringConverter->StringOfId(evnt->nameId));
                    });

                    fwprintf(c.file,L"}");
                };

                DumpDefineProperty(
                    c,
                    [&](){ DumpSegmentListAsQuotedName(c, parent); }, 
                    c.builder.stringConverter->StringOfId(field->identifier), value);
            }
            break;
        case ptAbiRemoveEventListenerProperty:
            {
                auto value = [&](){
                    fwprintf(c.file,L"function(event,handler) {\n");
                    fwprintf(c.file,L"/// <param name='event' type='String'/>\n");
                    fwprintf(c.file,L"/// <param name='handler' type='Function'/>\n");
                    fwprintf(c.file,L"}");
                };

                DumpDefineProperty(
                    c,
                    [&](){ DumpSegmentListAsQuotedName(c, parent); }, 
                    c.builder.stringConverter->StringOfId(field->identifier), value);
            }
            break;
        case ptAbiEventHandlerProperty:
            {
                auto eventHandler = AbiEventHandlerProperty::From(field);
                
                auto getterValue = [&](){
                    fwprintf(c.file,L"function(ev) {\n");
                    fwprintf(c.file,L"    /// <signature ");
                    fwprintf(c.file,L"externalid='");
                    DumpIntellidocEventSignature(c, typeName, eventHandler);
                    fwprintf(c.file,L"' externalFile='%s.xml' ", assembly->properties->name);
                    if (dumpHelp)
                    {
                        fwprintf(c.file,L"helpKeyword='", assembly->properties->name);
                        DumpEventHelpKeyword(c, typeName, eventHandler);
                        fwprintf(c.file,L"'");
                    }
                    fwprintf(c.file,L">\n");
                    fwprintf(c.file,L"    /// <param name='ev' type='Object' />\n");
                    DumpDeprecatedAttributesIntellisenseComment(c, eventHandler->abiEvent->addOn->deprecatedAttributes);
                    DumpSupportedOnAttributesIntellisenseComment(c, eventHandler->abiEvent->addOn->supportedOnAttributes);
                    fwprintf(c.file,L"    /// </signature>\n");
                    fwprintf(c.file,L"}\n");
                };
                auto setter= [&](){
                    fwprintf(c.file,L"function(handler) {");
                    fwprintf(c.file,L"handler(eventParamOf(");
                    DumpEventParameter(c, eventHandler->abiEvent);
                    fwprintf(c.file,L"))}");
                };

                DumpDefinePropertyGetterSetter(
                    c,
                    [&](){ DumpSegmentListAsQuotedName(c, parent); }, 
                    c.builder.stringConverter->StringOfId(field->identifier),
                    getterValue, setter);

                if(c.dumpExtraTypeInfo) {
                    DumpEventAddType(c, parent, typeName, assembly, eventHandler);
                }

            }
            break;
        case ptAbiFieldProperty:
            {
                auto fieldProperty = AbiFieldProperty::From(field);
                if (fieldProperty->fieldProperties->IsLiteral())
                {
                    // Static (add to parent)
                    DumpDefineProperty(
                        c,
                        [&](){ DumpSegmentListAsQuotedName(c, parent->GetTail()); }, 
                        c.builder.stringConverter->StringOfId(field->identifier), 
                        [&](){ DumpMemberBody(c, typeName, assembly, field->expr, dumpHelp); });

                    if(c.dumpExtraTypeInfo && field->expr->type == exprFunction) {
                        DumpFunctionAddType(
                            c,
                            parent->GetTail(),
                            typeName,
                            assembly,
                            c.builder.stringConverter->StringOfId(field->identifier),
                            L"function",
                            Function::From(field->expr));
                    }
                }
                else
                {
                    DumpDefineProperty(
                        c,
                        [&](){ DumpSegmentListAsQuotedName(c, parent); }, 
                        c.builder.stringConverter->StringOfId(field->identifier), 
                        [&](){ DumpMemberBody(c, typeName, assembly, field->expr, dumpHelp); });

                    if(c.dumpExtraTypeInfo && field->expr->type == exprFunction) {
                        DumpFunctionAddType(
                            c,
                            parent,
                            typeName,
                            assembly,
                            c.builder.stringConverter->StringOfId(field->identifier),
                            L"function",
                            Function::From(field->expr));
                    }
                }
            }
            break;
        case ptAbiMethodProperty:
        case ptOverloadParentProperty:
            {
                DumpDefineProperty(
                    c,
                    [&](){ DumpSegmentListAsQuotedName(c, parent); }, 
                    c.builder.stringConverter->StringOfId(field->identifier), 
                    [&](){ DumpMemberBody(c, typeName, assembly, field->expr, dumpHelp); });

                if(c.dumpExtraTypeInfo && field->expr->type == exprFunction) {
                    DumpFunctionAddType(
                        c,
                        parent,
                        typeName,
                        assembly,
                        c.builder.stringConverter->StringOfId(field->identifier),
                        L"function",
                        Function::From(field->expr));
                }
            }
            break;
        case ptAbiTypeProperty:
            {
                auto thisName = parent->Prepend(c.builder.stringConverter->StringOfId(field->identifier), c.alloc);

                switch(c.pass)
                {
                case passDeclareBodies:
                    {
                        switch(field->expr->type)
                        {
                        case exprFunction:
                            {
                                auto constructor = Function::From(field->expr);
                                bool isHidden = false;
                                switch(constructor->functionType)
                                {
                                    case functionRuntimeClassConstructor:
                                        {
                                            auto rc = RuntimeClassConstructor::From(constructor);
                                            typeName = c.builder.stringConverter->StringOfId(rc->typeDef->id);
                                            assembly = &(rc->typeDef->assembly);
                                            break;
                                        }
                                    case functionInterfaceConstructor:
                                        {
                                            if (RuntimeInterfaceConstructor::Is(constructor))
                                            {
                                                auto rc = RuntimeInterfaceConstructor::From(constructor);
                                                typeName = c.builder.stringConverter->StringOfId(rc->typeDef->id);
                                                assembly = &(rc->typeDef->assembly);
                                                isHidden = true;
                                                break;
                                            }
                                            else
                                            {
                                                // This is a missing interface case
                                                return;
                                            }
                                        }
                                    case functionStructConstructor:
                                        {
                                            auto rc = StructConstructor::From(constructor);
                                            typeName = c.builder.stringConverter->StringOfId(rc->structType->typeDef->id);
                                            assembly = &(rc->structType->typeDef->assembly);
                                            isHidden = true;
                                            break;
                                        }
                                    case functionDelegateConstructor:
                                        {
                                            auto rc = RuntimeInterfaceConstructor::From(DelegateConstructor::From(constructor)->invokeInterface);                                            
                                            typeName = c.builder.stringConverter->StringOfId(rc->typeDef->id);
                                            assembly = &(rc->typeDef->assembly);
                                            isHidden = true;
                                            break;
                                        }
                                    case functionMissingTypeConstructor:
                                        {
                                            return;
                                        }
                                    default:
                                        Assert(0);
                                }
                                DumpDefineProperty(
                                    c,
                                    [&](){ DumpSegmentListAsQuotedName(c, parent); }, 
                                    c.builder.stringConverter->StringOfId(field->identifier), 
                                    [&](){ DumpConstructorBody(c, thisName, constructor);});
                                
                                if (isHidden)
                                {
                                    DumpDefineProperty(
                                        c,
                                        [&](){ DumpSegmentListAsQuotedName(c, thisName); },
                                        L"_$hidden",
                                        [&](){ fwprintf(c.file, L"true"); });
                                }

                                if(c.dumpExtraTypeInfo) {
                                    DumpFunctionAddType(
                                        c,
                                        parent,
                                        typeName,
                                        assembly,
                                        c.builder.stringConverter->StringOfId(field->identifier),
                                        L"constructor",
                                        constructor);
                                }
                                break;
                            }
                        case exprEnum:
                            {
                                auto enumName = c.builder.stringConverter->StringOfId(field->identifier);
                                auto enumExpr = Enum::From(field->expr);
                                DumpDefineProperty(
                                    c,
                                    [&](){ DumpSegmentListAsQuotedName(c, parent); }, 
                                    enumName, 
                                    [&](){ DumpEnumBody(c, parent, enumExpr);});
                                    DumpEnumHelp(c, parent, enumName, enumExpr);
                                    break;
                            }
                        default:
                            Js::Throw::FatalInternalError();
                        }
                    }
                    return;
                case passImplementBodies:
                    {
                        switch(field->expr->type)
                        {
                        case exprFunction:
                            {
                                auto constructor = Function::From(field->expr);
                                auto instance = parent->Prepend(c.builder.stringConverter->StringOfId(field->identifier), c.alloc);
                                DumpConstructorBody(c, instance, constructor);
                                break;
                            }
                        case exprEnum:
                            break;
                        default:
                            Js::Throw::FatalInternalError();
                        }
                    }
                    return;
                }
            }
            return;
        case ptAbiNamespaceProperty:
            {
                auto thisName = parent->Prepend(c.builder.stringConverter->StringOfId(field->identifier), c.alloc);
                fwprintf(c.file,L"/* namespace */ if(");
                DumpSegmentListAsQuotedName(c, thisName);
                fwprintf(c.file,L"==undefined) {");
                DumpSegmentListAsQuotedName(c, thisName);
                fwprintf(c.file,L"= {}; }\n");
                DumpNamespaceHelp(c, parent, thisName, field);

                auto subnamespace = PropertiesObject::From(field->expr);
                DumpSubnamespace(c, thisName, typeName, assembly, subnamespace);
                
            }
            return;
        case ptAbiArrayLengthProperty:
        case ptUnresolvableNameConflictProperty:
        case ptFunctionLengthProperty:
            // Show nothing for this case.
            return;
        default:
            Assert(0);
        }         
    }

    // Info:        Dump the fields of a struct as a constructor
    // Parameters:  c - generation context 
    //              parent - the name of the parent
    //              fields - the struct fields
    void DumpStructConstructor(CONTEXT c, ImmutableList<LPCWSTR> * parent, RtSTRUCTCONSTRUCTOR _struct)
    {
        switch(c.pass)
        {
            case passDeclareBodies:
                {
                    auto assemblyName = _struct->structType->typeDef->assembly.properties->name;
                    auto typeConstructor = TypeConstructorMethodSignature::From(_struct->signature);

                    fwprintf(c.file,L"/* struct constructor */function() {\n");

                    if (IsDocumentableType(typeConstructor->parameters->returnType))
                    {
                        fwprintf(c.file,L"    /// <signature externalid='");
                        DumpIntellidocTypeConstructorSignature(c, typeConstructor);
                        fwprintf(c.file,L"' externalFile='%s.xml' helpKeyword='", assemblyName);
                        DumpTypeName(c, _struct->structType);
                        fwprintf(c.file,L"'>\n");
                    }
                    else
                    {
                        fwprintf(c.file,L"    /// <signature>\n");
                    }

                    fwprintf(c.file,L"    /// <returns type='");
                    DumpTypeName(c, _struct->structType);

                    if (IsDocumentableType(typeConstructor->parameters->returnType))
                    {
                        fwprintf(c.file,L"' externalid='");
                        DumpIntellidocTypeConstructorSignature(c, typeConstructor);
                        fwprintf(c.file,L"' externalFile='%s.xml' helpKeyword='", assemblyName);
                        DumpTypeName(c, _struct->structType);
                    }
                    fwprintf(c.file,L"'/>\n");
                    fwprintf(c.file,L"    /// </signature>\n");
                    _struct->structType->fields->Iterate([&](RtPROPERTY prop) {
                        auto field = AbiFieldProperty::From(prop);
                        fwprintf(c.file,L"    /// <field name='%s' type='", c.builder.stringConverter->StringOfId(field->identifier));
                        DumpTypeName(c, field->type, true);
                        fwprintf(c.file,L"' externalid='");
                        DumpIntellidocStructField(c, _struct->structType, field);

                        // For fields, the help keyword is the struct type name, not the struct field name
                        fwprintf(c.file, L"' externalFile='%s.xml' helpKeyword='", assemblyName);
                        DumpTypeName(c, _struct->structType);

                        auto supportedOnAttributes = _struct->supportedOnAttributes;
                        if (!supportedOnAttributes->IsEmpty())
                        {
                            fwprintf(c.file, L"'>\n");
                            DumpSupportedOnAttributesIntellisenseComment(c, supportedOnAttributes);
                            fwprintf(c.file, L"\n    /// </field>\n");
                        }
                        else
                        {
                            fwprintf(c.file, L"'/>\n");
                        }
                    });
                    fwprintf(c.file,L"    var result={};\n");
                    _struct->structType->fields->Iterate([&](RtPROPERTY prop) {
                        auto field = AbiFieldProperty::From(prop);
                        DumpDefineProperty(c,
                            [&](){ fwprintf(c.file,L"result"); }, 
                            c.builder.stringConverter->StringOfId(field->identifier), 
                            [&](){ DumpTypeInstance(c, field->type); });
                    });
                    fwprintf(c.file,L"return result; }"); 
                }
                return;
            case passImplementBodies:
                return;
        }

        Assert(0);
    }

    // Info:        Given a specialization, assume it is vectorlike and return the element type
    // Parameters:  specialization - the specialization
    RtTYPE ElementTypeOfVectorSpecialization(RtSPECIALIZATION specialization)
    {
        switch(specialization->specializationType)
        {
        case specVectorSpecialization:
            return VectorSpecialization::From(specialization)->getAt->GetParameters()->returnType;
        case specVectorViewSpecialization:
            return VectorViewSpecialization::From(specialization)->getAt->GetParameters()->returnType;
        }
        Assert(0);
        return nullptr;
    }

    // Info:        Dump a constructor for a runtime class or interface
    // Parameters:  c - generation context 
    //              typeName - the type name of the enclosing type
    //              assembly - assembly of the enclosing type
    //              methodSignature - signature of the constructor
    //              properties - properties (statics) of the construct
    //              prototype - prototype fields
    //              specialization - specialization if any
    //              dumpHelp - true if we want to dump the help keywords
    void DumpRuntimeConstructor(CONTEXT c, ImmutableList<LPCWSTR> * parent, 
        LPCWSTR typeName, const Metadata::Assembly * assembly,
        RtMETHODSIGNATURE methodSignature, RtPROPERTIESOBJECT properties, RtEXPR prototype, RtSPECIALIZATION specialization, bool dumpHelp)
    {
        bool isArray = false;
        if(specialization)
        {
            switch(specialization->specializationType)
            {
            case specVectorSpecialization:
            case specVectorViewSpecialization:
                isArray = true;
                break;
            }
        }
        switch(c.pass)
        {
            case passDeclareBodies:
                {
                    DumpMethodSignatureAsFunctionDefinition(c, methodSignature);
                    fwprintf(c.file,L" {\n");

                    switch(methodSignature->signatureType)
                    {
                    case mstOverloadedMethodSignature:
                        { 
                            auto overloadSignature = OverloadedMethodSignature::From(methodSignature);
                            overloadSignature->overloads->overloads->Iterate([&](RtABIMETHODSIGNATURE arityGroup) {
                                DumpSignatureComment(c, typeName, assembly, arityGroup, dumpHelp);
                            });
                            break;
                        }
                    case mstAbiMethodSignature:
                        {
                            auto signature = AbiMethodSignature::From(methodSignature);
                            DumpSignatureComment(c, typeName, assembly, signature, dumpHelp);
                            break;
                        }
                    case mstUncallableMethodSignature:
                        {
                            // Even if a method is uncallable (like an unconstructable class), we still need to supply the F1 keyword
                            fwprintf(c.file,L"    /// <signature helpKeyword='%s' />", typeName);
                            fwprintf(c.file,L"\n");
                            break;
                        }
                    }
                    auto prototypeBody = PropertiesObject::From(prototype);


                    // Note, the doc comments for the object's properties (ie fields) needs to be dumped in the make() function
                    fwprintf(c.file,L"    function make() {\n");


                    ///////////////////////////////
                    prototypeBody->fields->Iterate([&](RtPROPERTY prop) {
                        if(AbiPropertyProperty::Is(prop))
                        {
                            auto field = AbiPropertyProperty::From(prop);
                            fwprintf(c.file,L"    /// <field name='%s' type='", c.builder.stringConverter->StringOfId(field->identifier));
                            DumpTypeName(c, field->GetPropertyType(), true);
                            fwprintf(c.file,L"' externalid='");
                            DumpIntellidocPropertySignature(c, typeName, field);
                            fwprintf(c.file,L"' externalFile='%s.xml' ", assembly->properties->name);
                            if (dumpHelp)
                            {
                                fwprintf(c.file,L"helpKeyword='", assembly->properties->name);
                                DumpPropertyHelpKeyword(c, typeName, field);
                                fwprintf(c.file,L"'");
                            }
                            auto deprecatedAttributes = GetPropertyDeprecatedAttributes(c, field);
                            auto supportedOnAttributes = GetPropertySupportedOnAttributes(c, field);
                            if (!deprecatedAttributes->IsEmpty() || !supportedOnAttributes->IsEmpty())
                            {
                                fwprintf(c.file,L">\n");
                                DumpDeprecatedAttributesIntellisenseComment(c, deprecatedAttributes);
                                DumpSupportedOnAttributesIntellisenseComment(c, supportedOnAttributes);
                                fwprintf(c.file,L"    /// </field>\n");
                            }
                            else
                            {
                                fwprintf(c.file,L"/>\n");
                            }
                        }
                    });
                    ///////////////////////////////

                    fwprintf(c.file,L"    return this; };\n");

                    if(isArray)
                    {
                        fwprintf(c.file,L"    make.prototype = new Array();\n");
                    }
                    auto prototypeName = ToImmutableList(L"make", c.alloc);
                    prototypeName = prototypeName->Prepend(L"prototype", c.alloc);
                    DumpPropertiesObject(c, prototypeName, typeName, assembly, prototypeBody, dumpHelp);
                    DumpDefineProperty(
                        c,
                        [&](){ DumpSegmentListAsQuotedName(c, prototypeName); }, 
                        L"constructor",
                        [&](){ fwprintf(c.file,L"make"); });
                    fwprintf(c.file,L"    var result = new make();\n");
                    if(isArray)
                    {
                        auto elementType = ElementTypeOfVectorSpecialization(specialization);
                        fwprintf(c.file,L"    result.push(");
                        DumpTypeInstance(c, elementType);
                        fwprintf(c.file,L");\n");
                    }
                    if (specialization==nullptr || specialization->specializationType != specPromiseSpecialization)
                    {
                        fwprintf(c.file,L"    return result;\n");
                    }
                    else
                    {
                        fwprintf(c.file,L"    return new AsyncOpWrapper(result);\n");
                    }
                    fwprintf(c.file,L"}\n");
                }
                return;
            case passImplementBodies:
                {
                    auto protoype = parent->Prepend(L"prototype", c.alloc);
                    auto prototypeBody = PropertiesObject::From(prototype);
                    DumpPropertiesObject(c, protoype, typeName, assembly, prototypeBody, dumpHelp);

                    auto statics = PropertiesObject::From(properties);
                    DumpPropertiesObject(c, parent, typeName, assembly, statics, dumpHelp);
                    
                    // Use annotate to dump the doc comments for the properties
                    bool hasStaticProperties = false;

                    statics->fields->Iterate([&](RtPROPERTY prop) {
                        if(AbiPropertyProperty::Is(prop))
                        {
                            auto field = AbiPropertyProperty::From(prop);
                            
                            if (!hasStaticProperties)
                            {
                                fwprintf(c.file,L"(function (rootNamespace) { if (rootNamespace.intellisense) { rootNamespace.intellisense.annotate(");
                                DumpSegmentListAsQuotedName(c, parent);
                                fwprintf(c.file,L", {\n");
                                hasStaticProperties = true;
                            }
                            fwprintf(c.file,L"    /// <field type='");
                            DumpTypeName(c, field->GetPropertyType(), true);
                            fwprintf(c.file,L"' externalid='");
                            DumpIntellidocPropertySignature(c, typeName, field);
                            fwprintf(c.file,L"' externalFile='%s.xml' ", assembly->properties->name);
                            if (dumpHelp)
                            {
                                fwprintf(c.file,L"helpKeyword='", assembly->properties->name);
                                DumpPropertyHelpKeyword(c, typeName, field);
                                fwprintf(c.file, L"'");
                            }
                            auto deprecatedAttributes = GetPropertyDeprecatedAttributes(c, field);
                            auto supportedOnAttributes = GetPropertySupportedOnAttributes(c, field);
                            if (!deprecatedAttributes->IsEmpty() || !supportedOnAttributes->IsEmpty())
                            {
                                fwprintf(c.file,L">\n");
                                DumpDeprecatedAttributesIntellisenseComment(c, deprecatedAttributes);
                                DumpSupportedOnAttributesIntellisenseComment(c, supportedOnAttributes);
                                fwprintf(c.file,L"    /// </field>\n");
                            }
                            else
                            {
                                fwprintf(c.file,L"/>\n");
                            }
                            fwprintf(c.file,L"    '%s':undefined,\n", c.builder.stringConverter->StringOfId(field->identifier));
                        }
                    });

                    if (hasStaticProperties)
                    {
                        fwprintf(c.file,L"});}})(rootNamespace);\n");
                    }
                }
                return;
        }

        Assert(0);
    }

    // Info:        Dump an interface constructor 
    // Parameters:  c - generation context 
    //              parent - parent name
    //              constructor - the constructor
    void DumpRuntimeInterfaceConstructorBody(CONTEXT c, ImmutableList<LPCWSTR> * parent, RtRUNTIMEINTERFACECONSTRUCTOR constructor)
    {
        // Dump a generic type wrapper function if there are generic parameters and this is pass 1.
        bool dumpGenericWrapper = (constructor->genericParameters->Count() > 0) && (c.pass == passDeclareBodies);
        if (dumpGenericWrapper)
        {
            DumpGenericParametersAsFunctionDefinition(c, constructor->genericParameters);
            fwprintf(c.file,L" {\n");
            fwprintf(c.file,L" return ");
        }

        DumpRuntimeConstructor(c, parent, c.builder.stringConverter->StringOfId(constructor->typeDef->id), &constructor->typeDef->assembly, constructor->signature, constructor->properties, constructor->prototype, constructor->specialization, false);
    
        if (dumpGenericWrapper)
        {
            fwprintf(c.file,L"}\n");
        }


    }
    
    // Info:        Dump a class constructor 
    // Parameters:  c - generation context 
    //              parent - parent name
    //              constructor - the constructor
    void DumpRuntimeClassConstructorBody(CONTEXT c, ImmutableList<LPCWSTR> * parent, RtRUNTIMECLASSCONSTRUCTOR constructor)
    {
        DumpRuntimeConstructor(c, parent, c.builder.stringConverter->StringOfId(constructor->typeDef->id), &constructor->typeDef->assembly, constructor->signature, constructor->properties, constructor->prototype, constructor->specialization, true);
    }

    // Info:        Dump a function as aconstructor 
    // Parameters:  c - generation context 
    //              parent - parent name
    //              constructor - the constructor
    void DumpConstructorBody(CONTEXT c, ImmutableList<LPCWSTR> * parent, RtFUNCTION constructor)
    {
        switch(constructor->functionType)
        {
        case functionStructConstructor:
            DumpStructConstructor(c, parent,StructConstructor::From(constructor));
            return;
        case functionInterfaceConstructor:
            if (RuntimeInterfaceConstructor::Is(constructor))
            {
                DumpRuntimeInterfaceConstructorBody(c, parent, RuntimeInterfaceConstructor::From(constructor));
            }
            return;
        case functionRuntimeClassConstructor:
            DumpRuntimeClassConstructorBody(c, parent, RuntimeClassConstructor::From(constructor));
            return;
        case functionDelegateConstructor:
            DumpConstructorBody(c, parent, DelegateConstructor::From(constructor)->invokeInterface);
            return;
        case functionMissingTypeConstructor:
            // Missing type constructor
            return;
        }
        Assert(0);
    }
    
    // Info:        Dump a subnamespace
    // Parameters:  c - generation context 
    //              parent - parent name
    //              typeName - enclosing type name
    //              assembly - enclosing assembly
    //              subnamespace - sub namespace elements
    void DumpSubnamespace(CONTEXT c, ImmutableList<LPCWSTR> * parent, LPCWSTR typeName, const Metadata::Assembly * assembly, RtPROPERTIESOBJECT subnamespace)
    {
        subnamespace->fields->Iterate([&](RtPROPERTY prop) {
            DumpProperty(c, parent, typeName, assembly, prop, true);
        });
    }

    // Info:        Dump the help keyword for a namespace
    // Parameters:  c - generation context 
    //              parent - parent name
    //              typeName - enclosing type name
    //              field - the property field
    void DumpNamespaceHelp(CONTEXT c, ImmutableList<LPCWSTR> * parent, ImmutableList<LPCWSTR> * typeName, RtPROPERTY field)
    {
        // We need to use the language service helper intellisense to annotate the keyword on the namespace,
        // since the namespace itself is not a function but just an object

        fwprintf(c.file,L"if (rootNamespace.intellisense) { rootNamespace.intellisense.annotate(");
        DumpSegmentListAsQuotedName(c, parent);
        fwprintf(c.file,L", {\n");

        fwprintf(c.file,L"    /// <field type='");
        DumpSegmentListAsDottedName(c, typeName);
        fwprintf(c.file,L"' externalid='T:");
        DumpSegmentListAsDottedName(c, typeName);
        // Namespaces are not associated with any individual metadata assembly, since multiple assemblies can contribute to the same namespace.
        // For now, all of WinRT is mapped to Windows.xml, so using the root namespace as the name of the xml file.
        fwprintf(c.file,L"' externalFile='");
        DumpSegmentListAsDottedName(c, parent);
        fwprintf(c.file,L".xml' helpKeyword='");
        DumpSegmentListAsDottedName(c, typeName);
        fwprintf(c.file, L"' />\n");

        fwprintf(c.file,L"    '%s':undefined\n});}\n", c.builder.stringConverter->StringOfId(field->identifier));
    }

    // Info:        Dump a properties list
    // Parameters:  c - generation context 
    //              parent - parent name
    //              typeName - enclosing type name
    //              assembly - enclosing assembly
    //              properties - the properties
    //              dumpHelp - true if we want to dump the help keywords
    void DumpProperties(CONTEXT c, ImmutableList<LPCWSTR> * parent, LPCWSTR typeName, const Metadata::Assembly * assembly, ImmutableList<RtPROPERTY> * properties, bool dumpHelp)
    {
        properties->Iterate([&](RtPROPERTY prop) {
            DumpProperty(c, parent, typeName, assembly, prop, dumpHelp);
        });
    }

    // Info:        Dump a properties object
    // Parameters:  c - generation context 
    //              parent - parent name
    //              typeName - enclosing type name
    //              assembly - enclosing assembly
    //              propertiesObject - the properties
    //              dumpHelp - true if we want to dump the help keywords
    void DumpPropertiesObject(CONTEXT c, ImmutableList<LPCWSTR> * parent, LPCWSTR typeName, const Metadata::Assembly * assembly, RtPROPERTIESOBJECT propertiesObject, bool dumpHelp)
    {
        DumpProperties(c, parent, typeName, assembly, propertiesObject->fields, dumpHelp);
    }

    // Info:        Dump a namespace assignment expression
    // Parameters:  c - generation context 
    //              parent - parent name
    //              assignmentExpr - the assignment expression
    void DumpNamespaceAssignmentExpr(CONTEXT c, ImmutableList<LPCWSTR> * parent, RtEXPR assignmentExpr)
    {
        switch(assignmentExpr->type)
        {
        case exprPropertiesObject:
            DumpPropertiesObject(c, parent, nullptr, nullptr, PropertiesObject::From(assignmentExpr), true);
            return;
        }
        Assert(0);
    }

    // Info:        Dump a namespace assignment
    // Parameters:  c - generation context 
    //              var - parent name
    void DumpNamespaceAssignment(CONTEXT c, RtASSIGNMENT var)
    {
        if (c.pass == passDeclareBodies)
        {
            fwprintf(c.file,L"if(this.%s==undefined) {this.%s = {};}\n", var->identifier, var->identifier);
        }
        auto parent = ToImmutableList(L"rootNamespace", c.alloc)->Prepend(var->identifier, c.alloc);
        DumpNamespaceAssignmentExpr(c, parent, var->expr);
    }

    // Info:        Dump the prolog
    // Parameters:  rootNamespace - "this" or custom-defined root namespace
    //              file - the file
    void DumpProlog(LPCWSTR rootNamespace, FILE * file)
    {
        if(wcscmp(rootNamespace, L"this") != 0) {
            fwprintf(file, L"var %s = {};\n", rootNamespace);

            fwprintf(file,L"function addType(base, name, kind, values) {\n");
            fwprintf(file,L"    if(base._type === undefined) base._type = {};\n");
            fwprintf(file,L"    base._type[name] = {};\n");
            fwprintf(file,L"    base._type[name].kind = kind;\n");
            fwprintf(file,L"    for(var key in values) base._type[name][key] = values[key];\n");
            fwprintf(file,L"}\n");
        }

        fwprintf(file, JSGEN_PROLOG_BODY); 
    }

    // Info:        Dump the epilog
    // Parameters:  rootNamespace - "this" or custom-defined root namespace
    //              file - the file
    void DumpEpilog(LPCWSTR rootNamespace, FILE * file)
    {
        // Define Object.defineProperties if one doesn't exist.
        fwprintf(file,L"}).call(%s);\n", rootNamespace);
    }

    // Info:        Dump the full assignment space
    // Parameters:  varSpace - the assignment space
    //              rootNamespace - "this" or user defined root namespace
    //              file - the file
    void JavaScriptStubDumper::Dump(ProjectionModel::ProjectionBuilder & builder, RtASSIGNMENTSPACE varSpace, LPCWSTR rootNamespace, FILE * file, ArenaAllocator * a)
    {
        bool dumpExtraTypeInfo = wcscmp(rootNamespace, L"this") != 0;

        Context context(passDeclareBodies, builder, file, a, dumpExtraTypeInfo); 

        DumpProlog(rootNamespace, file);
        // Pass1 -- declare all types. No bodies yet.
        fwprintf(file,L"// Begin Pass 1 ----------------------------------------------------------------------\n");
        varSpace->vars->Iterate([&](RtASSIGNMENT var) {DumpNamespaceAssignment(context, var);});
        // Pass2 -- declare all types. No bodies yet.
        fwprintf(file,L"// Begin Pass 2 ----------------------------------------------------------------------\n");
        context.pass = passImplementBodies;
        varSpace->vars->Iterate([&](RtASSIGNMENT var) {DumpNamespaceAssignment(context, var);});
        DumpEpilog(rootNamespace, file);
    }
}
