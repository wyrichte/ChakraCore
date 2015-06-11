//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"

using namespace Authoring;

namespace Authoring
{
    namespace Names
    {
        const wchar_t functionName[] = L"functionName";
        const wchar_t funcParamSignature[] = L"funcParamSignature";
        const wchar_t signatures[] = L"signatures";
    }
}

class FunctionHelpImpl: 
    public SimpleComObjectWithAlloc<IAuthorFunctionHelp>
{
    friend class FunctionHelpConverter;
    typedef SimpleComObjectWithAlloc<IAuthorFunctionHelp> base;
public:
    class SignatureInfo;

    class ParamInfo: public InnerComObject<IAuthorParameter>
    {
        friend class FunctionHelpConverter;
        FunctionHelpImpl* m_parent;
        ComStringField m_name;
        ComStringField m_type;
        ComStringField m_description;
        ComStringField m_locid;
        ComStringField m_elementType;
        bool m_optional;
        IAuthorSignature* m_funcParamSignature;
    public:
        ParamInfo(FunctionHelpImpl* parent, LPCWSTR name)
            :InnerComObject<IAuthorParameter>(parent), m_parent(parent)
        {
            Assert(parent != NULL);
            SetName(name);
            m_funcParamSignature = NULL;
            m_optional = false;
        }

        void ApplyDocComments(FunctionDocComments::Param* paramDoc)
        {
            SetType(paramDoc->type);
            SetDescription(paramDoc->description);
            SetLocid(paramDoc->locid);
            SetElementType(paramDoc->elementType);
            SetOptional(paramDoc->optional);
            if(paramDoc->signature != nullptr)
            {
                auto signature = m_parent->CreateSignature();
                signature->ApplyDocComments(paramDoc->signature, /* namedParams = */nullptr, /* canAdd = */true, /* shouldAddUndocumentedParam = */ false);
                SetSignature(signature);
            }
        }

        LPCWSTR Name() { return m_name.Sz(); }

        // IAuthorParameter
        STRING_PROPERTY_IMPL(Name, m_name)
        STRING_PROPERTY_IMPL(Type, m_type)
        STRING_PROPERTY_IMPL(Description, m_description)
        STRING_PROPERTY_IMPL(Locid, m_locid)
        STRING_PROPERTY_IMPL(ElementType, m_elementType)
        VARIANT_BOOL_PROPERTY_IMPL(Optional, m_optional)
        INTERFACE_PROPERTY_IMPL(FunctionParamSignature, IAuthorSignature, m_funcParamSignature)
        // End IAuthorParameter
    private:
        void SetName(LPCWSTR name) { m_name.Set(m_parent->Alloc(), name); }
        void SetType(LPCWSTR type) { m_type.Set(m_parent->Alloc(), type); }
        void SetDescription(LPCWSTR description) { m_description.Set(m_parent->Alloc(), description); }
        void SetLocid(LPCWSTR locid) { m_locid.Set(m_parent->Alloc(), locid); }
        void SetElementType(LPCWSTR elementType) { m_elementType.Set(m_parent->Alloc(), elementType); }
        void SetOptional(bool optional) { m_optional = optional; }
        void SetSignature(SignatureInfo* signatureInfo)
        {
            Assert(m_funcParamSignature == nullptr);
            m_funcParamSignature = signatureInfo;
        }
        
    }; // End ParamInfo

    class ReturnValueInfo: public InnerComObject<IAuthorReturnValue>
    {
        friend class FunctionHelpConverter;
        FunctionHelpImpl* m_parent;
        ComStringField m_type;
        ComStringField m_description;
        ComStringField m_locid;
        ComStringField m_elementType;
        ComStringField m_helpKeyword;
        ComStringField m_externalid;
        ComStringField m_externalFile;

    public:
        ReturnValueInfo(FunctionHelpImpl* parent)
            :InnerComObject<IAuthorReturnValue>(parent), m_parent(parent)
        {
            Assert(parent != NULL);
        }

        void ApplyDocComments(FunctionDocComments::ReturnValue* returnDoc)
        {
            SetType(returnDoc->type);
            SetDescription(returnDoc->description);
            SetLocid(returnDoc->locid);
            SetElementType(returnDoc->elementType);
            SetHelpKeyword(returnDoc->helpKeyword);
            SetExternalid(returnDoc->externalid);
            SetExternalFile(returnDoc->externalFile);
        }

        // IAuthorReturnValue
        STRING_PROPERTY_IMPL(Type, m_type)
        STRING_PROPERTY_IMPL(Description, m_description)
        STRING_PROPERTY_IMPL(Locid, m_locid)
        STRING_PROPERTY_IMPL(ElementType, m_elementType)
        STRING_PROPERTY_IMPL(HelpKeyword, m_helpKeyword)
        STRING_PROPERTY_IMPL(Externalid, m_externalid)
        STRING_PROPERTY_IMPL(ExternalFile, m_externalFile)
        // End IAuthorReturnValue
    private:
        void SetType(LPCWSTR type) { m_type.Set(m_parent->Alloc(), type); }
        void SetDescription(LPCWSTR description) { m_description.Set(m_parent->Alloc(), description); }
        void SetLocid(LPCWSTR locid) { m_locid.Set(m_parent->Alloc(), locid); }
        void SetElementType(LPCWSTR elementType) { m_elementType.Set(m_parent->Alloc(), elementType); }
        void SetHelpKeyword(LPCWSTR helpKeyword) { m_helpKeyword.Set(m_parent->Alloc(), helpKeyword); }
        void SetExternalid(LPCWSTR externalid) { m_externalid.Set(m_parent->Alloc(), externalid); }
        void SetExternalFile(LPCWSTR externalFile) { m_externalFile.Set(m_parent->Alloc(), externalFile); }
    }; // End ReturnValueInfo

    class SignatureInfo: 
        public InnerComObject<IAuthorSignature>
    {
        friend class FunctionHelpConverter;
        typedef Collection<IAuthorParameterSet, IAuthorParameter> ParameterSet;
        FunctionHelpImpl* m_parent;
        ComStringField m_description;
        ComStringField m_locid;
        ComStringField m_externalFile;
        ComStringField m_externalid;
        IAuthorReturnValue* m_returnValue;
        ComStringField m_helpKeyword;
        IAuthorDeprecatedInfo* m_deprecated;
        ParameterSet m_params;
        CompatibleWithSet m_compatibleWith;
    public:
        SignatureInfo(FunctionHelpImpl* parent, JsUtil::List<LPCWSTR, ArenaAllocator>* namedParams)
            :InnerComObject<IAuthorSignature>(parent), m_parent(parent), m_params(parent, parent->Alloc()), m_compatibleWith(parent, parent->Alloc()), m_returnValue(nullptr), m_deprecated(nullptr)
        {
            Assert(parent != NULL);

            if(namedParams != nullptr)
            {
                for(int i = 0; i < namedParams->Count(); i++)
                {
                    auto paramName = String::Copy(m_parent->Alloc(), namedParams->Item(i));
                    AddParam(m_parent->CreateParam(paramName));
                }
            }
        }

        void ApplyDocComments(FunctionDocComments::Signature* signatureDoc, JsUtil::List<LPCWSTR, ArenaAllocator>* namedParams, bool canAdd, bool shouldAddUndocumentedParam)
        {
            Assert(signatureDoc != nullptr);

            // Allowed configurations are:
            // shouldAddUndocumentedParam | namedParams == nullptr
            // ---------------------------+-----------------------
            // T                          | F
            // F                          | T
            // F                          | F
            // 
            // So T T is not allowed
            Assert(!(shouldAddUndocumentedParam && namedParams == nullptr));

            SetDescription(signatureDoc->description);
            SetLocid(signatureDoc->locid);
            SetExternalFile(signatureDoc->externalFile);
            SetExternalid(signatureDoc->externalid);
            SetHelpKeyword(signatureDoc->helpKeyword);

            if (signatureDoc->returnValue)
            {
                auto returnValue = m_parent->CreateReturnValue();
                returnValue->ApplyDocComments(signatureDoc->returnValue);
                SetReturnValue(returnValue);
            }

            if (signatureDoc->deprecated)
            {
                auto deprecated = m_parent->CreateDeprecated();
                deprecated->ApplyDocComments(signatureDoc->deprecated);
                SetDeprecated(deprecated);
            }

            for (int i = 0; i < signatureDoc->compatibleWith.Count(); i++)
            {
                auto compatibleWithDoc = signatureDoc->compatibleWith.Item(i);
                auto compatInfo = m_parent->CreateCompatibleWith();

                compatInfo->ApplyDocComments(compatibleWithDoc);
                AddCompatibleWith(compatInfo);
            }

            // Merging the information from DocComments and the function named parameters
            bool docCommentHasParamsNotInParseTree = false;
            int numNamedParams = -1;
            FunctionDocComments::Param** reorder = nullptr;

            if (shouldAddUndocumentedParam)
            {
                numNamedParams = namedParams->Count();
                reorder = AnewArray(m_parent->Alloc(), FunctionDocComments::Param*, numNamedParams);
                for (int j = 0; j < numNamedParams; j++)
                {
                    reorder[j] = nullptr;
                }

                for (int i = 0; i < signatureDoc->params.Count(); i++)
                {
                    bool docCommentInParseTree = false;
                    for (int j = 0; j < numNamedParams; j++)
                    {
                        if (wcscmp(signatureDoc->params.Item(i)->name, namedParams->Item(j)) == 0)
                        {
                            reorder[j] = signatureDoc->params.Item(i);
                            docCommentInParseTree = true;
                            break;
                        }
                    }
                    if (!docCommentInParseTree)
                    {
                        docCommentHasParamsNotInParseTree = true;
                        break;
                    }
                }
            }

            if (shouldAddUndocumentedParam && !docCommentHasParamsNotInParseTree)
            {
                for (int j = 0; j < numNamedParams; j++)
                {
                    ParamInfo* param = m_parent->CreateParam(String::Trim(m_parent->Alloc(), namedParams->Item(j)));
                    AddParam(param);
                    if (reorder[j] != nullptr)
                    {
                        param->ApplyDocComments(reorder[j]);
                    }
                }
            }
            else
            {
                for (int i = 0; i < signatureDoc->params.Count(); i++)
                {
                    auto paramDoc = signatureDoc->params.Item(i);

                    if (paramDoc->name == nullptr)
                    {
                        // <param> tag missing name element, ignore it.
                        continue;
                    }

                    auto param = FindParamByName(paramDoc->name);
                    if (param == nullptr)
                    {
                        if (canAdd)
                        {
                            param = m_parent->CreateParam(String::Trim(m_parent->Alloc(), paramDoc->name));
                            AddParam(param);
                        }
                        else
                        {
                            // The parameter doesn't exist in the signature, skip it.
                            continue;
                        }
                    }

                    param->ApplyDocComments(paramDoc);
                }
            }
        }

        void ApplyFieldDocComments(JsValueDoc* fieldDoc)
        {
            Assert(fieldDoc);
            // Field doc comments should only be used when no function doc comments are available.
            Assert(String::IsNullOrEmpty(m_description.Sz()) && 
                String::IsNullOrEmpty(m_locid.Sz()) && 
                String::IsNullOrEmpty(m_externalFile.Sz()) && 
                String::IsNullOrEmpty(m_externalid.Sz()) &&
                String::IsNullOrEmpty(m_helpKeyword.Sz()));

            SetDescription(fieldDoc->description);
            SetLocid(fieldDoc->locid);
            SetExternalFile(fieldDoc->externalFile);
            SetExternalid(fieldDoc->externalid);
            SetHelpKeyword(fieldDoc->helpKeyword);

            if (fieldDoc->deprecated)
            {
                auto deprecated = m_parent->CreateDeprecated();
                deprecated->ApplyDocComments(fieldDoc->deprecated);
                SetDeprecated(deprecated);
            }

            for (int i = 0; i < fieldDoc->compatibleWith.Count(); i++)
            {
                auto compatibleWithDoc = fieldDoc->compatibleWith.Item(i);
                auto compatInfo = m_parent->CreateCompatibleWith();

                compatInfo->ApplyDocComments(compatibleWithDoc);
                AddCompatibleWith(compatInfo);
            }
        }

        // ISignatureInfo
        STRING_PROPERTY_IMPL(Description, m_description)
        STRING_PROPERTY_IMPL(Locid, m_locid)
        STRING_PROPERTY_IMPL(ExternalFile, m_externalFile)
        STRING_PROPERTY_IMPL(Externalid, m_externalid)
        STRING_PROPERTY_IMPL(HelpKeyword, m_helpKeyword)
        INTERFACE_PROPERTY_IMPL(ReturnValue, IAuthorReturnValue, m_returnValue)
        INTERFACE_PROPERTY_IMPL(Deprecated, IAuthorDeprecatedInfo, m_deprecated)
        COLLECTION_PROPERTY_IMPL(Parameters, IAuthorParameterSet, &m_params)
        COLLECTION_PROPERTY_IMPL(CompatibleWith, IAuthorCompatibleWithSet, &m_compatibleWith);
    private:
        void SetDescription(LPCWSTR description) { m_description.Set(m_parent->Alloc(), description); }
        void SetLocid(LPCWSTR locid) { m_locid.Set(m_parent->Alloc(), locid); }
        void SetExternalFile(LPCWSTR externalFile) { m_externalFile.Set(m_parent->Alloc(), externalFile); }
        void SetExternalid(LPCWSTR externalid) { m_externalid.Set(m_parent->Alloc(), externalid); }
        void SetHelpKeyword(LPCWSTR helpKeyword) { m_helpKeyword.Set(m_parent->Alloc(), helpKeyword); }
        void SetReturnValue(IAuthorReturnValue* returnValue) { m_returnValue = returnValue; }
        void SetDeprecated(IAuthorDeprecatedInfo* deprecated) { m_deprecated = deprecated; }

        void AddCompatibleWith(IAuthorCompatibleWithInfo* compatInfo)
        {
            m_compatibleWith.Add(static_cast<IAuthorCompatibleWithInfo *>(compatInfo));
        }

        void AddParam(ParamInfo* paramInfo)
        {
            m_params.Add(static_cast<IAuthorParameter *>(paramInfo));
        }

        ParamInfo* FindParamByName(LPCWSTR paramName)
        {
            LPCWSTR trimmedParamName = String::Trim(m_parent->Alloc(), paramName);

            if (trimmedParamName == nullptr)
                return nullptr;

            for(int i=0; i<m_params.Count(); i++)
            {
                auto param = static_cast<ParamInfo *>(m_params.Item(i));
                if(wcscmp(param->Name(), trimmedParamName) == 0)
                {
                    return param;
                }
            }

            return nullptr;
        }
    }; // End SignatureInfo

private:
    typedef Collection<IAuthorSignatureSet, IAuthorSignature> SignatureSet;
    ComStringField m_functionName;
    SignatureSet m_signatures;
    AuthoringFileHandle* m_sourceFileHandle;
public:
    FunctionHelpImpl(PageAllocator* pageAlloc)
        : base(pageAlloc, L"ls:FunctionHelp"), m_signatures(this, Alloc()), m_sourceFileHandle(nullptr)
    {
    }

    HRESULT Initialize(LPCWSTR functionDisplayName, JsUtil::List<LPCWSTR, ArenaAllocator>& namedParams, FunctionDocComments* funcDoc, JsValueDoc* fieldDoc, AuthoringFileHandle* sourceFileHandle)
    {
        Assert(functionDisplayName != NULL);
        m_functionName.Set(Alloc(), functionDisplayName);

        SignatureInfo* declaration = nullptr;
        if(funcDoc == nullptr || funcDoc->signatures.Count() == 0)
        {
            declaration = CreateSignature(&namedParams);
            AddSignature(declaration);
        }

        if(funcDoc)
        {
            ApplyDocComments(declaration, &namedParams, funcDoc);
        }
        else if(fieldDoc)
        {
            declaration->ApplyFieldDocComments(fieldDoc);
        }

        if (sourceFileHandle && !sourceFileHandle->IsTransient())
            // Only remember references to non-transient files. Transient files when a function is created because of eval or calling 
            // Function() directly. We still need to return help information for this function but there is no file associated with it.
            // Transient source files are discarded and keeping a reference count doesn't ensure they will be kept.
            m_sourceFileHandle = sourceFileHandle;

        return S_OK;
    }

    SignatureInfo* CreateSignature(JsUtil::List<LPCWSTR, ArenaAllocator>* namedParams = nullptr)
    {
        return Anew(Alloc(), SignatureInfo, this, namedParams);
    }

    ParamInfo* CreateParam(LPCWSTR name = nullptr)
    {
        return Anew(Alloc(), ParamInfo, this, name);
    }

    ReturnValueInfo* CreateReturnValue()
    {
        return Anew(Alloc(), ReturnValueInfo, this);
    }

    DeprecatedInfo<FunctionHelpImpl>* CreateDeprecated()
    {
        return Anew(Alloc(), DeprecatedInfo<FunctionHelpImpl>, this);
    }

    CompatibleWithInfo<FunctionHelpImpl>* CreateCompatibleWith()
    {
        return Anew(Alloc(), CompatibleWithInfo<FunctionHelpImpl>, this);
    }

    // IFunctionHelpInfo implementation
    STRING_PROPERTY_IMPL(FunctionName, m_functionName)
    COLLECTION_PROPERTY_IMPL(Signatures, IAuthorSignatureSet, &m_signatures)
    INTERFACE_PROPERTY_IMPL(SourceFileHandle, IAuthorFileHandle, m_sourceFileHandle)
    // End IFunctionHelpInfo
    
private:
    void ApplyDocComments(SignatureInfo* declaration, JsUtil::List<LPCWSTR, ArenaAllocator>* namedParams, FunctionDocComments* funcDoc)
    {
        bool shouldAddUndocumentedParam = false;
        Assert(funcDoc != null);
        if(funcDoc->signatures.Count() > 0)
        {
            // Create signatures from doc comments
            for(int i = 0; i < funcDoc->signatures.Count(); i++)
            {
                auto signatureDoc = funcDoc->signatures.Item(i);

                auto signature = CreateSignature();
                // In case we have more than one signature - the author is clear about the parameter list, so let's not add undocumented parameters automatically in this case
                shouldAddUndocumentedParam = funcDoc->signatures.Count() == 1; 
                signature->ApplyDocComments(signatureDoc, namedParams, /* canAdd = */true, shouldAddUndocumentedParam); 
                AddSignature(signature);
            }
        }
        else
        {
            // Merge the declaration and the root comments
            Assert(declaration != null);
            // In case we are using the implicit signature - we are not creating parameter when it is not found (canAdd = false),
            // so let's not add undocumented parameters automatically in this case also
            // shouldAddUndocumentedParam = false (this is already initialized at the beginning of the function)
            declaration->ApplyDocComments(funcDoc->implicitSignature, namedParams, /* canAdd = */false, shouldAddUndocumentedParam);
        }
    }

    void AddSignature(SignatureInfo* signature)
    {
        m_signatures.Add(signature);
    }
};

#ifdef DEBUG
template <> AuthorInfoParentType CompatibleWithInfo<FunctionHelpImpl>::TypeOfParent()
{
    return ptFunctionHelpImpl;
}

template <> AuthorInfoParentType DeprecatedInfo<FunctionHelpImpl>::TypeOfParent()
{
    return ptFunctionHelpImpl;
}
#endif


HRESULT Authoring::FunctionHelpInfo::CreateInstance(
    PageAllocator* pageAlloc, 
    LPCWSTR functionDisplayName, 
    JsUtil::List<LPCWSTR, ArenaAllocator>& namedParams, 
    CommentBuffer* functionComments,
    JsValueDoc* fieldDoc,
    IAuthorFunctionHelp** funcInfo, 
    FunctionHelpInfoParseStatus* parseStatus, 
    AuthoringFileHandle* sourceFileHandle)
{
    IfNullReturnError(functionDisplayName, E_POINTER);
    IfNullReturnError(funcInfo, E_POINTER);
    HRESULT hr = S_OK;

    if (parseStatus != nullptr) *parseStatus = FunctionHelpInfoParseStatus::fhipsOK;
    
    FunctionHelpImpl* funcInfoImpl = new FunctionHelpImpl(pageAlloc);

    FunctionDocComments* funcDoc = nullptr;
    if (!String::IsNullOrEmpty(functionComments->Sz()))
    {
        IfFailedReturn(Authoring::ParseFuncDocComments(funcInfoImpl->Alloc(), functionComments->GetBuffer(), functionComments->GetCommentType(), &funcDoc));
        if (funcDoc && funcDoc->parseError)
        {
            if (parseStatus != nullptr) *parseStatus = FunctionHelpInfoParseStatus::fhipsInvalidXML;
        }
    }
    
    if(!functionDisplayName || wcscmp(functionDisplayName, Js::Constants::AnonymousFunction) == 0)
    {
        functionDisplayName = L"";
    }

    hr = funcInfoImpl->Initialize(functionDisplayName, namedParams, funcDoc, fieldDoc, sourceFileHandle);
    IfFailedReturn(hr);

    *funcInfo = funcInfoImpl;
    return S_OK;
}

class FunctionHelpConverter
{
    // Prevent instantiation
    FunctionHelpConverter() { }
public:
    static Js::RecyclableObject* ToRecyclableObject(FunctionHelpImpl* funcInfoImpl, Js::ScriptContext* scriptContext)
    {
        Assert(scriptContext);

        auto root = JsHelpers::CreateObject(scriptContext);
        if(funcInfoImpl)
        {
            JsHelpers::SetField(root, Names::functionName, funcInfoImpl->m_functionName.Sz(), scriptContext);
            JsHelpers::SetField(root, Names::_fileId, funcInfoImpl->m_sourceFileHandle ? funcInfoImpl->m_sourceFileHandle->FileId() : 0 , scriptContext);
            JsHelpers::SetField(root, Names::signatures, 
                JsHelpers::CreateArray(funcInfoImpl->m_signatures.Count(), [&] (uint index) -> Js::RecyclableObject* 
                { 
                    auto signature = static_cast<FunctionHelpImpl::SignatureInfo*>(funcInfoImpl->m_signatures.Item(index));
                    return ToRecyclableObject(signature, scriptContext);
                }, scriptContext), scriptContext);
        }
        return root;
    }

    static FunctionHelpImpl* FromRecyclableObject(Js::RecyclableObject* obj, PageAllocator* pageAlloc, Js::ScriptContext* scriptContext, FileAuthoring* fileAuthoring)
    {
        Assert(obj);
        Assert(pageAlloc);
        Assert(scriptContext);
        Assert(fileAuthoring);

        FunctionHelpImpl* funcInfoImpl = new FunctionHelpImpl(pageAlloc);

        auto alloc = funcInfoImpl->Alloc();

        funcInfoImpl->m_functionName.Set(alloc, JsHelpers::GetProperty<LPCWSTR>(obj, Names::functionName, alloc, scriptContext));
        auto fileId = JsHelpers::GetProperty<int>(obj, Names::_fileId, alloc, scriptContext);
        if (fileId > 0)
            funcInfoImpl->m_sourceFileHandle = fileAuthoring->GetAuthoringFileById(fileId);
        auto signatures = JsHelpers::GetProperty<Js::JavascriptArray*>(obj, Names::signatures, alloc, scriptContext);
        if (!signatures || signatures->GetLength() == 0)
        {
            funcInfoImpl->Release();
            return nullptr;
        }

        JsHelpers::ForEach<Js::RecyclableObject*>(signatures, alloc, [&] (uint index, Js::RecyclableObject* jsSignature) 
        {

            FunctionHelpImpl::SignatureInfo* signature = FromRecyclableObject(funcInfoImpl, jsSignature, alloc, scriptContext);
            funcInfoImpl->AddSignature(signature);
        });

        return funcInfoImpl;
    }

private:
    static Js::RecyclableObject* ToRecyclableObject(FunctionHelpImpl::SignatureInfo* signature, Js::ScriptContext* scriptContext)
    {
        Assert(signature);
        Assert(scriptContext);
        auto jsSignature = JsHelpers::CreateObject(scriptContext);
        JsHelpers::SetField(jsSignature, Names::description, signature->m_description.Sz(), scriptContext);
        JsHelpers::SetField(jsSignature, Names::locid, signature->m_locid.Sz(), scriptContext);
        JsHelpers::SetField(jsSignature, Names::externalFile, signature->m_externalFile.Sz(), scriptContext);
        JsHelpers::SetField(jsSignature, Names::externalid, signature->m_externalid.Sz(), scriptContext);
        JsHelpers::SetField(jsSignature, Names::helpKeyword, signature->m_helpKeyword.Sz(), scriptContext);
        JsHelpers::SetField(jsSignature, Names::params, 
            JsHelpers::CreateArray(signature->m_params.Count(), [&] (uint index) -> Js::RecyclableObject* 
        { 
            auto param = static_cast<FunctionHelpImpl::ParamInfo*>(signature->m_params.Item(index));
            auto jsParams = JsHelpers::CreateObject(scriptContext);
            JsHelpers::SetField(jsParams, Names::name, param->m_name.Sz(), scriptContext);
            JsHelpers::SetField(jsParams, Names::type, param->m_type.Sz(), scriptContext);
            JsHelpers::SetField(jsParams, Names::description, param->m_description.Sz(), scriptContext);
            JsHelpers::SetField(jsParams, Names::locid, param->m_locid.Sz(), scriptContext);
            JsHelpers::SetField(jsParams, Names::elementType, param->m_elementType.Sz(), scriptContext);
            JsHelpers::SetField(jsParams, Names::optional, param->m_optional, scriptContext);
            if (param->m_funcParamSignature)
            {
                JsHelpers::SetField(jsParams, Names::funcParamSignature, 
                    ToRecyclableObject(static_cast<FunctionHelpImpl::SignatureInfo*>(param->m_funcParamSignature), scriptContext), scriptContext);
            }
            JsHelpers::CreateObject(scriptContext);
            return jsParams;
        }, scriptContext), scriptContext);

        auto returnValue = static_cast<FunctionHelpImpl::ReturnValueInfo*>(signature->m_returnValue);
        if (returnValue)
        {
            auto jsReturnValue = JsHelpers::CreateObject(scriptContext);
            JsHelpers::SetField(jsReturnValue, Names::type, returnValue->m_type.Sz(), scriptContext);
            JsHelpers::SetField(jsReturnValue, Names::description, returnValue->m_description.Sz(), scriptContext);
            JsHelpers::SetField(jsReturnValue, Names::locid, returnValue->m_locid.Sz(), scriptContext);
            JsHelpers::SetField(jsReturnValue, Names::helpKeyword, returnValue->m_helpKeyword.Sz(), scriptContext);
            JsHelpers::SetField(jsReturnValue, Names::elementType, returnValue->m_elementType.Sz(), scriptContext);
            JsHelpers::SetField(jsReturnValue, Names::externalid, returnValue->m_externalid.Sz(), scriptContext);
            JsHelpers::SetField(jsReturnValue, Names::externalFile, returnValue->m_externalFile.Sz(), scriptContext);

            JsHelpers::SetField(jsSignature, Names::returnValue, jsReturnValue, scriptContext);
        }

        if (signature->m_deprecated)
        {
            JsHelpers::SetField(jsSignature, Names::deprecated, DeprecatedInfo<FunctionHelpImpl>::ToRecyclableObject(signature->m_deprecated, scriptContext), scriptContext);
        }

        JsHelpers::SetField(jsSignature, Names::compatibleWith,
            JsHelpers::CreateArray(signature->m_compatibleWith.Count(), [&] (uint index) -> Js::RecyclableObject*
        {
            return CompatibleWithInfo<FunctionHelpImpl>::ToRecyclableObject(signature->m_compatibleWith.Item(index), scriptContext);
        }, scriptContext), scriptContext);

        return jsSignature;
    }

    static FunctionHelpImpl::SignatureInfo* FromRecyclableObject(FunctionHelpImpl* parentFunctionHelpInfoObj, Js::RecyclableObject* jsSignature, ArenaAllocator* alloc, Js::ScriptContext* scriptContext)
    {
        Assert(parentFunctionHelpInfoObj);
        Assert(jsSignature);
        Assert(alloc);
        Assert(scriptContext);

        FunctionHelpImpl::SignatureInfo* signature = parentFunctionHelpInfoObj->CreateSignature();
        signature->m_description.Set(alloc, JsHelpers::GetProperty<LPCWSTR>(jsSignature, Names::description, alloc, scriptContext));
        signature->m_locid.Set(alloc, JsHelpers::GetProperty<LPCWSTR>(jsSignature, Names::locid, alloc, scriptContext));
        signature->m_helpKeyword.Set(alloc, JsHelpers::GetProperty<LPCWSTR>(jsSignature, Names::helpKeyword, alloc, scriptContext));
        signature->m_externalFile.Set(alloc, JsHelpers::GetProperty<LPCWSTR>(jsSignature, Names::externalFile, alloc, scriptContext));
        signature->m_externalid.Set(alloc, JsHelpers::GetProperty<LPCWSTR>(jsSignature, Names::externalid, alloc, scriptContext));

        // returnvalue
        auto jsReturnValue = JsHelpers::GetProperty<Js::RecyclableObject*>(jsSignature, Names::returnValue, alloc, scriptContext, true);
        if (jsReturnValue)
        {
            auto returnValue = parentFunctionHelpInfoObj->CreateReturnValue();
            returnValue->m_type.Set(alloc, JsHelpers::GetProperty<LPCWSTR>(jsReturnValue, Names::type, alloc, scriptContext));
            returnValue->m_description.Set(alloc, JsHelpers::GetProperty<LPCWSTR>(jsReturnValue, Names::description, alloc, scriptContext));
            returnValue->m_locid.Set(alloc, JsHelpers::GetProperty<LPCWSTR>(jsReturnValue, Names::locid, alloc, scriptContext));
            returnValue->m_elementType.Set(alloc, JsHelpers::GetProperty<LPCWSTR>(jsReturnValue, Names::elementType, alloc, scriptContext));
            returnValue->m_helpKeyword.Set(alloc, JsHelpers::GetProperty<LPCWSTR>(jsReturnValue, Names::helpKeyword, alloc, scriptContext));
            returnValue->m_externalid.Set(alloc, JsHelpers::GetProperty<LPCWSTR>(jsReturnValue, Names::externalid, alloc, scriptContext));
            returnValue->m_externalFile.Set(alloc, JsHelpers::GetProperty<LPCWSTR>(jsReturnValue, Names::externalFile, alloc, scriptContext));
            signature->m_returnValue = returnValue;
        }

        auto jsDeprecated = JsHelpers::GetProperty<Js::RecyclableObject*>(jsSignature, Names::deprecated, alloc, scriptContext, true);
        if (jsDeprecated && !Js::JavascriptOperators::IsUndefinedOrNullType(jsDeprecated->GetTypeId()))
        {
            signature->m_deprecated = DeprecatedInfo<FunctionHelpImpl>::FromRecyclableObject(parentFunctionHelpInfoObj, jsDeprecated, alloc, scriptContext);
        }

        auto jsCompatibleWith = JsHelpers::GetProperty<Js::JavascriptArray*>(jsSignature, Names::compatibleWith, alloc, scriptContext, true);
        if (jsCompatibleWith)
        {
            JsHelpers::ForEach<Js::RecyclableObject*>(jsCompatibleWith, alloc, [&] (uint index, Js::RecyclableObject* jsCompat) 
            {
                signature->AddCompatibleWith(CompatibleWithInfo<FunctionHelpImpl>::FromRecyclableObject(parentFunctionHelpInfoObj, jsCompat, alloc, scriptContext));

            });
        }

        auto params = JsHelpers::GetProperty<Js::JavascriptArray*>(jsSignature, Names::params, alloc, scriptContext);
        if (params)
        {
            JsHelpers::ForEach<Js::RecyclableObject*>(params, alloc, [&] (uint index, Js::RecyclableObject* jsParam) 
            {
                auto param = parentFunctionHelpInfoObj->CreateParam();
                param->m_name.Set(alloc, JsHelpers::GetProperty<LPCWSTR>(jsParam, Names::name, alloc, scriptContext));
                param->m_type.Set(alloc, JsHelpers::GetProperty<LPCWSTR>(jsParam, Names::type, alloc, scriptContext));
                param->m_description.Set(alloc, JsHelpers::GetProperty<LPCWSTR>(jsParam, Names::description, alloc, scriptContext));
                param->m_locid.Set(alloc, JsHelpers::GetProperty<LPCWSTR>(jsParam, Names::locid, alloc, scriptContext));
                param->m_elementType.Set(alloc, JsHelpers::GetProperty<LPCWSTR>(jsParam, Names::elementType, alloc, scriptContext));
                param->m_optional = JsHelpers::GetProperty<bool>(jsParam, Names::optional, alloc, scriptContext);
                auto jsFuncParamSignature = JsHelpers::GetProperty<Js::RecyclableObject*>(jsParam, Names::funcParamSignature, alloc, scriptContext, true);
                if (jsFuncParamSignature)
                {
                    param->m_funcParamSignature = FromRecyclableObject(parentFunctionHelpInfoObj, jsFuncParamSignature, alloc, scriptContext);
                }
                signature->AddParam(param);
            });
        }

        return signature;
    }
};

TYPE_STATS(FunctionHelpImpl, L"FunctionHelpImpl")
TYPE_STATS(FunctionHelpImpl::ParamInfo, L"ParamInfo")
TYPE_STATS(FunctionHelpImpl::ReturnValueInfo, L"ReturnValueInfo")
TYPE_STATS(DeprecatedInfo<FunctionHelpImpl>, L"DeprecatedInfo<FunctionHelpImpl>")
TYPE_STATS(CompatibleWithInfo<FunctionHelpImpl>, L"CompatibleWithInfo<FunctionHelpImpl>")
TYPE_STATS(FunctionHelpImpl::SignatureInfo, L"SignatureInfo")
TYPE_STATS(FunctionHelpImpl::SignatureInfo::ParameterSet, L"ParameterSet")
TYPE_STATS(FunctionHelpImpl::SignatureSet, L"SignatureSet")

Js::RecyclableObject* Authoring::FunctionHelpInfo::ToRecyclableObject(IAuthorFunctionHelp* funcInfo, Js::ScriptContext* scriptContext)
{
    Assert(scriptContext);

    return FunctionHelpConverter::ToRecyclableObject(static_cast<FunctionHelpImpl*>(funcInfo), scriptContext);
}

IAuthorFunctionHelp* Authoring::FunctionHelpInfo::FromRecyclableObject(
    Js::RecyclableObject* obj, 
    PageAllocator* pageAlloc, 
    Js::ScriptContext* scriptContext,
    FileAuthoring* fileAuthoring)
{
    Assert(obj);
    Assert(pageAlloc);
    Assert(scriptContext);
    Assert(fileAuthoring);

    return FunctionHelpConverter::FromRecyclableObject(obj, pageAlloc, scriptContext, fileAuthoring);
}