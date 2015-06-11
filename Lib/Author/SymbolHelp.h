//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

namespace Authoring
{
    class SymbolHelp : public SimpleComObjectWithAlloc<IAuthorSymbolHelp>
    {
        typedef Collection<IAuthorCompatibleWithSet, IAuthorCompatibleWithInfo> CompatibleWithSet;

        AuthorType           m_type;
        AuthorScope          m_scope;
        ComStringField       m_name;
        ComStringField       m_typeName;
        ComStringField       m_description;
        ComStringField       m_locid;
        ComStringField       m_elementType;
        ComStringField       m_helpKeyword;
        AuthoringFileHandle* m_sourceFileHandle;
        ComStringField       m_externalFile;
        ComStringField       m_externalid;

        RefCountedPtr<IAuthorDeprecatedInfo>  m_deprecated;
        RefCountedPtr<IAuthorCompatibleWithSet> m_compatibleWith;
        RefCountedPtr<IAuthorFunctionHelp> m_funcHelp;

        typedef SimpleComObjectWithAlloc<IAuthorSymbolHelp> base;

    public:

        SymbolHelp(PageAllocator* pageAlloc);

        void Initialize(
            AuthorType type, 
            AuthorScope scope, 
            LPCWSTR typeName, 
            LPCWSTR name, 
            LPCWSTR description, 
            LPCWSTR locid, 
            LPCWSTR elementType, 
            LPCWSTR helpKeyword, 
            LPCWSTR externalFile, 
            LPCWSTR externalid, 
            IAuthorDeprecatedInfo* deprecated, 
            IAuthorCompatibleWithSet* compatibleWith,
            IAuthorFunctionHelp* funcHelp, 
            AuthoringFileHandle* sourceFileHandle);

        SIMPLE_PROPERTY_IMPL(Type, AuthorType, m_type)
        SIMPLE_PROPERTY_IMPL(Scope, AuthorScope, m_scope)
        STRING_PROPERTY_IMPL(Name, m_name)
        STRING_PROPERTY_IMPL(TypeName, m_typeName)
        INTERFACE_PROPERTY_IMPL(FunctionHelp, IAuthorFunctionHelp, m_funcHelp)
        STRING_PROPERTY_IMPL(Description, m_description)
        STRING_PROPERTY_IMPL(Locid, m_locid)
        STRING_PROPERTY_IMPL(ElementType, m_elementType)
        STRING_PROPERTY_IMPL(HelpKeyword, m_helpKeyword)
        INTERFACE_PROPERTY_IMPL(SourceFileHandle, IAuthorFileHandle, m_sourceFileHandle)
        STRING_PROPERTY_IMPL(ExternalFile, m_externalFile)
        STRING_PROPERTY_IMPL(Externalid, m_externalid)
        INTERFACE_PROPERTY_IMPL(Deprecated, IAuthorDeprecatedInfo, m_deprecated)
        INTERFACE_PROPERTY_IMPL(CompatibleWith, IAuthorCompatibleWithSet, m_compatibleWith)

        static Js::RecyclableObject* ToRecyclableObject(IAuthorSymbolHelp* symbolHelp, Js::ScriptContext* scriptContext);
        static IAuthorSymbolHelp* FromRecyclableObject(Js::RecyclableObject* obj, PageAllocator* pageAlloc, Js::ScriptContext* scriptContext, FileAuthoring* fileAuthoring);

    protected:
        virtual void OnDelete() override;
    };
}