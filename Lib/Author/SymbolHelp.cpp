//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

namespace Authoring
{
    namespace Names
    {
        const wchar_t functionHelp[] = L"functionHelp";
        const wchar_t symbolDisplayType[] = L"symbolDisplayType";
        const wchar_t symbolType[] = L"symbolType";
    }

    TYPE_STATS(SymbolHelp, L"SymbolHelp")
    TYPE_STATS(DeprecatedInfo<SymbolHelp>, L"DeprecatedInfo<SymbolHelp>")
    TYPE_STATS(CompatibleWithInfo<SymbolHelp>, L"CompatibleWithInfo<SymbolHelp>")

    SymbolHelp::SymbolHelp(PageAllocator* pageAlloc)
        : base(pageAlloc, L"ls: SymbolHelp"), m_type(atUnknown), m_scope(ascopeUnknown), m_funcHelp(nullptr), m_sourceFileHandle(nullptr), m_deprecated(nullptr)
    {
    }

    void SymbolHelp::Initialize(AuthorType type, AuthorScope scope, LPCWSTR typeName, LPCWSTR name, LPCWSTR description, LPCWSTR locid, LPCWSTR elementType, LPCWSTR helpKeyword, LPCWSTR externalFile, LPCWSTR externalid, IAuthorDeprecatedInfo* deprecated, IAuthorCompatibleWithSet* compatibleWith, IAuthorFunctionHelp* funcHelp, AuthoringFileHandle* sourceFileHandle)
    {
        m_type = type;
        m_scope = scope;
        m_name.Set(Alloc(), name);
        m_typeName.Set(Alloc(), typeName);
        m_description.Set(Alloc(), description);
        m_locid.Set(Alloc(), locid);
        m_elementType.Set(Alloc(), elementType);
        m_helpKeyword.Set(Alloc(), helpKeyword);
        m_funcHelp.Assign(funcHelp);
        m_sourceFileHandle = sourceFileHandle;
        m_externalFile.Set(Alloc(), externalFile);
        m_externalid.Set(Alloc(), externalid);
        m_deprecated.Assign(deprecated);
        m_compatibleWith.Assign(compatibleWith);
    }

    void SymbolHelp::OnDelete()
    {
        m_funcHelp.ReleaseAndNull();
        m_deprecated.ReleaseAndNull();
        m_compatibleWith.ReleaseAndNull();
        base::OnDelete();
    }

    struct AuthorTypeConverter
    {
        static LPCWSTR ToString(AuthorType authorType)
        {
            switch(authorType)
            {
            case atBoolean:
                return L"Boolean";
            case atNumber:
                return L"Number";
            case atString:
                return L"String";
            case atObject:
                return L"Object";
            case atFunction:
                return L"Function";
            case atArray:
                return L"Array";
            case atDate:
                return L"Date";
            case atRegEx:
                return L"RegEx";
            case atSymbol:
                return L"Symbol";
            default:
                return L"unknown";
            }
        }

        static AuthorType FromString(LPCWSTR s)
        {
             if(!String::IsNullOrEmpty(s))
            {
                switch (s[0])
                {
                case 'A':
                    if(!wcscmp(s, L"Array"))
                        return atArray;
                    break;
                case 'B':
                    if(!wcscmp(s, L"Boolean"))
                        return atBoolean;
                    break;
                case 'D':
                    if(!wcscmp(s, L"Date"))
                        return atDate;
                    break;
                case 'F':
                    if(!wcscmp(s, L"Function"))
                        return atFunction;
                    break;
                case 'N':
                    if(!wcscmp(s, L"Number"))
                        return atNumber;
                    break;
                case 'O':
                    if(!wcscmp(s, L"Object"))
                        return atObject;
                    break;
                case 'R':
                    if(!wcscmp(s, L"RegEx"))
                        return atRegEx;
                    break;
                case 'S':
                    if (!wcscmp(s, L"String"))
                    {
                        return atString;
                    }
                    else if (!wcscmp(s, L"Symbol"))
                    {
                        return atSymbol;
                    }
                    break;

                }
             }
             return atUnknown;
        }
    };

    Js::RecyclableObject* SymbolHelp::ToRecyclableObject(IAuthorSymbolHelp* symbolHelp, Js::ScriptContext* scriptContext)
    {
        Assert(symbolHelp);
        Assert(scriptContext);
        auto symbolHelpImpl = static_cast<SymbolHelp*>(symbolHelp);
        auto jsSymbolHelp = JsHelpers::CreateObject(scriptContext);
        if(symbolHelp)
        {
            JsHelpers::SetField(jsSymbolHelp, Names::symbolType, AuthorTypeConverter::ToString(symbolHelpImpl->m_type), scriptContext);
            JsHelpers::SetField(jsSymbolHelp, Names::symbolDisplayType, symbolHelpImpl->m_typeName.Sz(), scriptContext);
            JsHelpers::SetField(jsSymbolHelp, Names::scope, AuthorScopeConverter::ToString(symbolHelpImpl->m_scope), scriptContext);
            JsHelpers::SetField(jsSymbolHelp, Names::name, symbolHelpImpl->m_name.Sz(), scriptContext);
            JsHelpers::SetField(jsSymbolHelp, Names::description, symbolHelpImpl->m_description.Sz(), scriptContext);
            JsHelpers::SetField(jsSymbolHelp, Names::locid, symbolHelpImpl->m_locid.Sz(), scriptContext);
            JsHelpers::SetField(jsSymbolHelp, Names::helpKeyword, symbolHelpImpl->m_helpKeyword.Sz(), scriptContext);
            JsHelpers::SetField(jsSymbolHelp, Names::elementType, symbolHelpImpl->m_elementType.Sz(), scriptContext);
            JsHelpers::SetField(jsSymbolHelp, Names::externalFile, symbolHelpImpl->m_externalFile.Sz(), scriptContext);
            JsHelpers::SetField(jsSymbolHelp, Names::externalid, symbolHelpImpl->m_externalid.Sz(), scriptContext);
            JsHelpers::SetField(jsSymbolHelp, Names::_fileId, symbolHelpImpl->m_sourceFileHandle ? symbolHelpImpl->m_sourceFileHandle->FileId() : 0 , scriptContext);

            if (symbolHelpImpl->m_deprecated)
            {
                JsHelpers::SetField(jsSymbolHelp, Names::deprecated, DeprecatedInfo<SymbolHelp>::ToRecyclableObject(symbolHelpImpl->m_deprecated, scriptContext), scriptContext);
            }

            if (symbolHelpImpl->m_compatibleWith)
            {
                auto compatibleWithCollection = static_cast<CompatibleWithSet*>(static_cast<IAuthorCompatibleWithSet*>(symbolHelpImpl->m_compatibleWith));
                JsHelpers::SetField(jsSymbolHelp, Names::compatibleWith,
                    JsHelpers::CreateArray(compatibleWithCollection->Count(), [&] (uint index) -> Js::RecyclableObject*
                {
                    return CompatibleWithInfo<SymbolHelp>::ToRecyclableObject(compatibleWithCollection->Item(index), scriptContext);
                }, scriptContext), scriptContext);
            }

            if(symbolHelpImpl->m_funcHelp)
            {
                JsHelpers::SetField(jsSymbolHelp, Names::functionHelp, FunctionHelpInfo::ToRecyclableObject(symbolHelpImpl->m_funcHelp, scriptContext), scriptContext);
            }
        }
        return jsSymbolHelp;
    }

    IAuthorSymbolHelp* SymbolHelp::FromRecyclableObject(Js::RecyclableObject* obj, PageAllocator* pageAlloc, Js::ScriptContext* scriptContext, FileAuthoring* fileAuthoring)
    {
        Assert(obj);
        Assert(scriptContext);
        Assert(fileAuthoring);
        auto symbolHelp = new SymbolHelp(pageAlloc);
        auto alloc = symbolHelp->Alloc();

        AuthoringFileHandle* sourceFileHandle = nullptr;
        auto fileId = JsHelpers::GetProperty<int>(obj, Names::_fileId, alloc, scriptContext);
        if (fileId > 0)
            sourceFileHandle = fileAuthoring->GetAuthoringFileById(fileId);

        RefCountedPtr<IAuthorFunctionHelp> funcHelp;
        auto jsFuncHelp = JsHelpers::GetProperty<Js::RecyclableObject*>(obj, Names::functionHelp, alloc, scriptContext);
        if(jsFuncHelp)
        {
            funcHelp.TakeOwnership(FunctionHelpInfo::FromRecyclableObject(jsFuncHelp, pageAlloc, scriptContext, fileAuthoring));
        }

        RefCountedPtr<IAuthorDeprecatedInfo> deprecated;
        auto jsDeprecated = JsHelpers::GetProperty<Js::RecyclableObject*>(obj, Names::deprecated, alloc, scriptContext);
        if (jsDeprecated && !Js::JavascriptOperators::IsUndefinedOrNullType(jsDeprecated->GetTypeId()))
        {
            deprecated.TakeOwnership(DeprecatedInfo<SymbolHelp>::FromRecyclableObject(symbolHelp, jsDeprecated, alloc, scriptContext));
        }

        RefCountedPtr<IAuthorCompatibleWithSet> compatibleWith;
        auto jsCompatibleWith = JsHelpers::GetProperty<Js::JavascriptArray*>(obj, Names::compatibleWith, alloc, scriptContext);
        if (jsCompatibleWith)
        {
            auto compatInfoSet = Anew(alloc, CompatibleWithSet, symbolHelp, symbolHelp->Alloc());
            JsHelpers::ForEach<Js::RecyclableObject*>(jsCompatibleWith, alloc, [&] (uint index, Js::RecyclableObject* jsCompat) 
            {
                compatInfoSet->Add(CompatibleWithInfo<SymbolHelp>::FromRecyclableObject(symbolHelp, jsCompat, alloc, scriptContext));
            });
            compatibleWith.TakeOwnership(compatInfoSet);
        }

        symbolHelp->Initialize(
            AuthorTypeConverter::FromString(JsHelpers::GetProperty<LPCWSTR>(obj, Names::symbolType, alloc, scriptContext)), 
            AuthorScopeConverter::FromString(JsHelpers::GetProperty<LPCWSTR>(obj, Names::scope, alloc, scriptContext)),
            JsHelpers::GetProperty<LPCWSTR>(obj, Names::symbolDisplayType, alloc, scriptContext),
            JsHelpers::GetProperty<LPCWSTR>(obj, Names::name, alloc, scriptContext),
            JsHelpers::GetProperty<LPCWSTR>(obj, Names::description, alloc, scriptContext),
            JsHelpers::GetProperty<LPCWSTR>(obj, Names::locid, alloc, scriptContext),
            JsHelpers::GetProperty<LPCWSTR>(obj, Names::elementType, alloc, scriptContext),
            JsHelpers::GetProperty<LPCWSTR>(obj, Names::helpKeyword, alloc, scriptContext),
            JsHelpers::GetProperty<LPCWSTR>(obj, Names::externalFile, alloc, scriptContext),
            JsHelpers::GetProperty<LPCWSTR>(obj, Names::externalid, alloc, scriptContext),
            deprecated, compatibleWith, funcHelp, sourceFileHandle);

        return symbolHelp;
    }

#ifdef DEBUG
    template <> AuthorInfoParentType DeprecatedInfo<SymbolHelp>::TypeOfParent()
    {
        return ptSymbolHelp;
    }

    template <> AuthorInfoParentType CompatibleWithInfo<SymbolHelp>::TypeOfParent()
    {
        return ptSymbolHelp;
    }
#endif
}
