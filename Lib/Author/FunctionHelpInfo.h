//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Authoring
{
    class JsValueDoc;

    enum FunctionHelpInfoParseStatus
    {
        fhipsOK,
        fhipsInvalidXML
    };
    
    enum AuthorInfoParentType
    {
        ptSymbolHelp,
        ptFunctionHelpImpl
    };

    template <typename TAuthorInfo>
    class AuthorInfoBase : public InnerComObject<TAuthorInfo>
    {
    public:
        AuthorInfoBase(IUnknown* parent) :
            InnerComObject<TAuthorInfo>(parent)
        {}

#ifdef DEBUG
        AuthorInfoParentType GetParentType() { return m_parentType; }
        void SetParentType(AuthorInfoParentType value) { m_parentType = value; }

    private:
        AuthorInfoParentType m_parentType;
#endif
    };

    template <typename TParent>
    class DeprecatedInfo : public AuthorInfoBase<IAuthorDeprecatedInfo>
    {
        TParent* m_parent;
        ComStringField m_type;
        ComStringField m_message;

    public:
        DeprecatedInfo(TParent* parent)
            :AuthorInfoBase<IAuthorDeprecatedInfo>(parent), m_parent(parent)
        {
            Assert(parent != NULL);
#ifdef DEBUG
            SetParentType(TypeOfParent());
#endif
        }

        void ApplyDocComments(Deprecated* deprecatedDoc)
        {
            SetType(deprecatedDoc->type);
            SetMessage(deprecatedDoc->message);
        }

        static Js::RecyclableObject* ToRecyclableObject(IAuthorDeprecatedInfo* deprecated, Js::ScriptContext* scriptContext)
        {
            Assert(deprecated);
            Assert(scriptContext);

#ifdef DEBUG
            auto depInfoBase = static_cast<AuthorInfoBase<IAuthorDeprecatedInfo>*>(deprecated);
            Assert(depInfoBase->GetParentType() == TypeOfParent());
#endif
            auto depInfo = static_cast<DeprecatedInfo<TParent>*>(deprecated);

            auto jsDeprecated = JsHelpers::CreateObject(scriptContext);
            JsHelpers::SetField(jsDeprecated, Names::type, depInfo->m_type.Sz(), scriptContext);
            JsHelpers::SetField(jsDeprecated, Names::message, depInfo->m_message.Sz(), scriptContext);

            return jsDeprecated;
        }

        static IAuthorDeprecatedInfo* FromRecyclableObject(TParent* parentObj, Js::RecyclableObject* jsDeprecated, ArenaAllocator* alloc, Js::ScriptContext* scriptContext)
        {
            Assert(parentObj);
            Assert(jsDeprecated);
            Assert(alloc);
            Assert(scriptContext);

            DeprecatedInfo<TParent>* deprecated = Anew(parentObj->Alloc(), DeprecatedInfo<TParent>, parentObj);
            deprecated->m_type.Set(alloc, JsHelpers::GetProperty<LPCWSTR>(jsDeprecated, Names::type, alloc, scriptContext));
            deprecated->m_message.Set(alloc, JsHelpers::GetProperty<LPCWSTR>(jsDeprecated, Names::message, alloc, scriptContext));
            
            return deprecated;
        }

        // IAuthorDeprecatedInfo
        STRING_PROPERTY_IMPL(Type, m_type)
        STRING_PROPERTY_IMPL(Message, m_message)
        // End IAuthorDeprecatedInfo
    private:
#ifdef DEBUG
        static AuthorInfoParentType TypeOfParent();
#endif
        void SetType(LPCWSTR type) { m_type.Set(m_parent->Alloc(), type); }
        void SetMessage(LPCWSTR message) { m_message.Set(m_parent->Alloc(), message); }
    }; // End DeprecatedInfo

    template <typename TParent>
    class CompatibleWithInfo : public AuthorInfoBase<IAuthorCompatibleWithInfo>
    {
        TParent* m_parent;
        ComStringField m_platform;
        ComStringField m_minVersion;

    public:
        CompatibleWithInfo(TParent* parent)
            :AuthorInfoBase<IAuthorCompatibleWithInfo>(parent), m_parent(parent)
        {
            Assert(parent != NULL);
#ifdef DEBUG
            SetParentType(TypeOfParent());
#endif
        }

        void ApplyDocComments(CompatibleWith* compatibleWithDoc)
        {
            SetPlatform(compatibleWithDoc->platform);
            SetMinVersion(compatibleWithDoc->minVersion);
        }

        static Js::RecyclableObject* ToRecyclableObject(IAuthorCompatibleWithInfo* compatibleWith, Js::ScriptContext* scriptContext)
        {
            Assert(compatibleWith);
            Assert(scriptContext);

#ifdef DEBUG
            auto compatInfoBase = static_cast<AuthorInfoBase<IAuthorCompatibleWithInfo>*>(compatibleWith);
            Assert(compatInfoBase->GetParentType() == TypeOfParent());
#endif
            auto compatInfo = static_cast<CompatibleWithInfo<TParent>*>(compatibleWith);

            auto jsCompatibleWith = JsHelpers::CreateObject(scriptContext);
            JsHelpers::SetField(jsCompatibleWith, Names::platform, compatInfo->m_platform.Sz(), scriptContext);
            JsHelpers::SetField(jsCompatibleWith, Names::minVersion, compatInfo->m_minVersion.Sz(), scriptContext);

            return jsCompatibleWith;
        }

        static IAuthorCompatibleWithInfo* FromRecyclableObject(TParent* parentObj, Js::RecyclableObject* jsCompatibleWith, ArenaAllocator* alloc, Js::ScriptContext* scriptContext)
        {
            Assert(parentObj);
            Assert(jsCompatibleWith);
            Assert(alloc);
            Assert(scriptContext);

            CompatibleWithInfo<TParent>* compatibleWith = Anew(parentObj->Alloc(), CompatibleWithInfo<TParent>, parentObj);
            compatibleWith->m_platform.Set(alloc, JsHelpers::GetProperty<LPCWSTR>(jsCompatibleWith, Names::platform, alloc, scriptContext));
            compatibleWith->m_minVersion.Set(alloc, JsHelpers::GetProperty<LPCWSTR>(jsCompatibleWith, Names::minVersion, alloc, scriptContext));
            
            return compatibleWith;
        }

        // IAuthorCompatibleWithInfo
        STRING_PROPERTY_IMPL(Platform, m_platform)
        STRING_PROPERTY_IMPL(MinVersion, m_minVersion)
        // End IAuthorCompatibleWithInfo
    private:
#ifdef DEBUG
        static AuthorInfoParentType TypeOfParent();
#endif
        void SetPlatform(LPCWSTR platform) { m_platform.Set(m_parent->Alloc(), platform); }
        void SetMinVersion(LPCWSTR minVersion) { m_minVersion.Set(m_parent->Alloc(), minVersion); }
    }; // End CompatibleWithInfo

    typedef Collection<IAuthorCompatibleWithSet, IAuthorCompatibleWithInfo> CompatibleWithSet;
    TYPE_STATS(CompatibleWithSet, L"CompatibleWithSet");

    class FunctionHelpInfo
    {
    public:
        static HRESULT CreateInstance(
            __in PageAllocator* pageAlloc, 
            __in LPCWSTR functionDisplayName, 
            __in JsUtil::List<LPCWSTR, ArenaAllocator>& namedParams, 
            __in CommentBuffer* functionComments,
            __in JsValueDoc* fieldDoc,
            __out IAuthorFunctionHelp** funcInfo, 
            __out FunctionHelpInfoParseStatus* parseStatus, 
            __in AuthoringFileHandle* sourceFileHandle);

        static Js::RecyclableObject* ToRecyclableObject(IAuthorFunctionHelp* funcInfo, Js::ScriptContext* scriptContext);
        static IAuthorFunctionHelp* FromRecyclableObject(Js::RecyclableObject* obj, PageAllocator* pageAlloc, Js::ScriptContext* scriptContext, FileAuthoring* fileAuthoring);
    };
}
