//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

#include "JsDocCommentsParser.h"
#include "JsDocParsingException.h"
#include "DocComments.h"

namespace Authoring
{
    JsDocCommentsParser::JsDocCommentsParser() :
        m_allocator(nullptr),
        m_classifier(nullptr),
        m_scanner(nullptr),
        m_functionDocComments(nullptr),
        m_varDocComments(nullptr),
        m_fieldDocComments(nullptr),
        m_typeDefintionSetDocComments(nullptr),
        m_seenTag(JsDocToken::none),
        m_parsingMode(JsDocCommentsParsingMode::JsDocCommentsParsingModeNone),
        m_typeDefSeen(false)
    {
    }

    AutoJsDocCommentsParsingModeResetter::AutoJsDocCommentsParsingModeResetter(JsDocCommentsParser* parent, JsDocCommentsParsingMode mode) : m_parent(parent)
    {
        parent->m_parsingMode = mode;
        parent->m_typeDefSeen = false;
    }

    AutoJsDocCommentsParsingModeResetter::~AutoJsDocCommentsParsingModeResetter()
    {
        this->m_parent->m_parsingMode = JsDocCommentsParsingMode::JsDocCommentsParsingModeNone;
        this->m_parent->m_typeDefSeen = false;
    }

    AutoFunctionDocCommentsResetter::AutoFunctionDocCommentsResetter(JsDocCommentsParser* parent, FunctionDocComments* result) : m_parent(parent)
    {
        AssertMsg(this->m_parent->m_functionDocComments == nullptr, "parent->m_functionDocComments should be null before Parse is called.");
        this->m_parent->m_functionDocComments = result;
    }

    AutoFunctionDocCommentsResetter::~AutoFunctionDocCommentsResetter()
    {
        AssertMsg(this->m_parent->m_functionDocComments != nullptr, "parent->m_functionDocComments should not be null after Parse is called.");
        this->m_parent->m_functionDocComments = nullptr;
    }

    HRESULT JsDocCommentsParser::ParseFuncDocComments(ArenaAllocator* alloc, LPCWSTR signatureText, FunctionDocComments** funcDocComments)
    {
        this->m_allocator = alloc;
        FunctionDocComments* result = Anew(this->m_allocator, FunctionDocComments, this->m_allocator);
        result->implicitSignature = Anew(this->m_allocator, FunctionDocComments::Signature, this->m_allocator);

        FunctionDocComments::Signature* initialSignature = Anew(this->m_allocator, FunctionDocComments::Signature, this->m_allocator);
        result->signatures.Add(initialSignature);
        {
            AutoFunctionDocCommentsResetter resetter(this, result);
            if (signatureText != nullptr)
            {
                AutoJsDocCommentsParsingModeResetter modeResetter(this, JsDocCommentsParsingMode::JsDocCommentsParsingModeFunctionDeclaration);
                Assert(this->m_typeDefSeen == false);
                this->Parse(alloc, signatureText);
                if (this->m_typeDefSeen)
                {
                    result = Anew(this->m_allocator, FunctionDocComments, this->m_allocator);
                    result->implicitSignature = Anew(this->m_allocator, FunctionDocComments::Signature, this->m_allocator);
                }
                else
                {
                    WCHAR* description = this->m_scanner->GetTaglessDescription();
                    for (int i = 0; i < this->m_functionDocComments->signatures.Count(); i++)
                    {
                        this->m_functionDocComments->signatures.Item(i)->description = description;
                    }
                }
            }
        }

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        this->TraceFunctionDocComments(signatureText, result);
#endif

        *funcDocComments = result;
        return S_OK;
    }

    AutoVarDocCommentsResetter::AutoVarDocCommentsResetter(JsDocCommentsParser* parent, VarDocComments* result) : m_parent(parent)
    {
        AssertMsg(this->m_parent->m_varDocComments == nullptr, "parent->m_varDocComments should be null before Parse is called.");
        this->m_parent->m_varDocComments = result;
    }

    AutoVarDocCommentsResetter::~AutoVarDocCommentsResetter()
    {
        AssertMsg(this->m_parent->m_varDocComments != nullptr, "parent->m_varDocComments should not be null after Parse is called.");
        this->m_parent->m_varDocComments = nullptr;
    }

    HRESULT JsDocCommentsParser::ParseVarDocComments(ArenaAllocator* alloc, LPCWSTR text, VarDocComments** varDocComments)
    {
        this->m_allocator = alloc;
        VarDocComments* result = Anew(this->m_allocator, VarDocComments, this->m_allocator);
        {
            AutoVarDocCommentsResetter resetter(this, result);
            AutoJsDocCommentsParsingModeResetter modeResetter(this, JsDocCommentsParsingMode::JsDocCommentsParsingModeVariableDeclaration);
            Assert(this->m_typeDefSeen == false);
            this->Parse(alloc, text);
            if (this->m_typeDefSeen)
            {
                result = Anew(this->m_allocator, VarDocComments, this->m_allocator);
            }
            else
            {
                WCHAR* description = this->m_scanner->GetTaglessDescription();
                this->m_varDocComments->description = description;
            }
        }

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        this->TraceVariableDocComments(text, result);
#endif

        *varDocComments = result;
        return S_OK;
    }

    AutoFieldDocCommentsResetter::AutoFieldDocCommentsResetter(JsDocCommentsParser* parent, const wchar* fieldName, FieldDocComments* result) : m_parent(parent)
    {
        AssertMsg(this->m_parent->m_fieldDocComments == nullptr, "parent->m_fieldDocComments should be null before Parse is called.");
        this->m_parent->m_fieldName = fieldName;
        this->m_parent->m_fieldDocComments = result;
    }

    AutoFieldDocCommentsResetter::~AutoFieldDocCommentsResetter()
    {
        AssertMsg(this->m_parent->m_fieldDocComments != nullptr, "parent->m_fieldDocComments should not be null after Parse is called.");
        this->m_parent->m_fieldName = nullptr;
        this->m_parent->m_fieldDocComments = nullptr;
    }

    HRESULT JsDocCommentsParser::ParseFieldDocComments(ArenaAllocator* alloc, LPCWSTR fieldName, LPCWSTR text, bool isGlobalVariableAsField, FieldDocComments** fieldDocComments)
    {
        this->m_allocator = alloc;
        FieldDocComments* result = Anew(this->m_allocator, FieldDocComments, this->m_allocator);
        {
            AutoFieldDocCommentsResetter resetter(this, fieldName, result);
            AutoJsDocCommentsParsingModeResetter modeResetter(this, isGlobalVariableAsField ? JsDocCommentsParsingMode::JsDocCommentsParsingModeGlobalVariableAsFieldDeclaration : JsDocCommentsParsingMode::JsDocCommentsParsingModeFieldDeclaration);
            Assert(this->m_typeDefSeen == false);
            this->Parse(alloc, text);
            if (this->m_typeDefSeen)
            {
                result = Anew(this->m_allocator, FieldDocComments, this->m_allocator);
            }
        }

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        this->TraceFieldDocComments(text, result);
#endif

        *fieldDocComments = result;
        return S_OK;
    }

    AutoTypeDefintionSetDocCommentsResetter::AutoTypeDefintionSetDocCommentsResetter(JsDocCommentsParser* parent, TypeDefintionSetDocComments* result) : m_parent(parent)
    {
        AssertMsg(this->m_parent->m_typeDefintionSetDocComments == nullptr, "parent->m_typeDefintionSetDocComments should be null before Parse is called.");
        this->m_parent->m_typeDefintionSetDocComments = result;
    }

    AutoTypeDefintionSetDocCommentsResetter::~AutoTypeDefintionSetDocCommentsResetter()
    {
        AssertMsg(this->m_parent->m_typeDefintionSetDocComments != nullptr, "parent->m_typeDefintionSetDocComments should not be null after Parse is called.");
        this->m_parent->m_typeDefintionSetDocComments = nullptr;
    }

    HRESULT JsDocCommentsParser::ParseTypeDefinitionComments(ArenaAllocator* alloc, LPCWSTR text, TypeDefintionSetDocComments** typeDefintionSetDocComments)
    {
        Assert(alloc != null);
        Assert(typeDefintionSetDocComments != null);
        this->m_allocator = alloc;
        TypeDefintionSetDocComments* result = Anew(this->m_allocator, TypeDefintionSetDocComments, this->m_allocator);
        {
            AutoTypeDefintionSetDocCommentsResetter resetter(this, result);
            AutoJsDocCommentsParsingModeResetter modeResetter(this, JsDocCommentsParsingMode::JsDocCommentsParsingModeTypeDefinitions);
            this->Parse(alloc, text);
        }

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        this->TraceTypeDefinitionDocComments(text, result);
#endif

        *typeDefintionSetDocComments = result;
        return S_OK;
    }

    void JsDocCommentsParser::OnReturnsTagSeen()
    {
        this->m_seenTag = JsDocToken::atreturns;
    }

    void JsDocCommentsParser::OnParamTagSeen()
    {
        this->m_seenTag = JsDocToken::atparam;
    }

    void JsDocCommentsParser::OnPropertyTagSeen()
    {
        this->m_seenTag = JsDocToken::atproperty;
    }

    void JsDocCommentsParser::OnDeprecatedTagSeen()
    {
        this->m_seenTag = JsDocToken::atdeprecated;
    }

    void JsDocCommentsParser::OnDescriptionTagSeen()
    {
        this->m_scanner->AddTaglessDescriptionLine();
        this->m_seenTag = JsDocToken::atdescription;
    }

    void JsDocCommentsParser::OnSummaryTagSeen()
    {
        this->m_scanner->AddTaglessDescriptionLine();
        this->m_seenTag = JsDocToken::atsummary;
    }

    void JsDocCommentsParser::OnEndTag()
    {
        switch (this->m_seenTag)
        {
        case JsDocToken::atreturns:
            this->OnReturnsTagParsed();
            break;
        case JsDocToken::atparam:
            this->OnParamTagParsed();
            break;
        case JsDocToken::atproperty:
            this->OnPropertyTagParsed();
            break;
        case JsDocToken::atdeprecated:
            this->OnDeprecatedTagParsed();
            break;
        case JsDocToken::atdescription:
            this->OnDescriptionTagParsed();
            break;
        case JsDocToken::atsummary:
            this->OnSummaryTagParsed();
            break;
        }

        this->m_seenTag = JsDocToken::none;
    }

    void JsDocCommentsParser::OnReturnsTagParsed()
    {
        if (this->m_functionDocComments != nullptr)
        {
            int numSignatures = this->m_functionDocComments->signatures.Count();
            int specifiedNumTypes = this->m_scanner->GetTypeNameCount();
            int numTypes = specifiedNumTypes == 0 ? 1 : specifiedNumTypes;
            this->RepeatSignatureSet(numTypes - 1);
            int k = 0;
            WCHAR* description = this->m_scanner->GetTagDescription();
            for (int i = 0; i < numTypes; i++)
            {
                WCHAR* typeName = (specifiedNumTypes == 0) ? L"Object" : this->m_scanner->GetTypeName(i);
                wchar* mappedTypeName = nullptr;
                wchar* mappedElementType = nullptr;
                this->CanonicalizeType(typeName, &mappedTypeName, &mappedElementType);

                for (int j = 0; j < numSignatures; j++)
                {
                    this->m_functionDocComments->signatures.Item(k)->returnValue = Anew(this->m_allocator, FunctionDocComments::ReturnValue, this->m_allocator);
                    this->m_functionDocComments->signatures.Item(k)->returnValue->type = mappedTypeName;
                    this->m_functionDocComments->signatures.Item(k)->returnValue->elementType = mappedElementType;
                    this->m_functionDocComments->signatures.Item(k)->returnValue->description = description;
                    k++;
                }
            }
        }

        this->m_scanner->ClearTagDescriptions();
        this->m_scanner->ClearTypes();
    }

    void JsDocCommentsParser::OnParamTagParsed()
    {
        if (this->m_functionDocComments != nullptr)
        {
            int numSignatures = this->m_functionDocComments->signatures.Count();
            int specifiedNumTypes = this->m_scanner->GetTypeNameCount();
            int numTypes = specifiedNumTypes == 0 ? 1 : specifiedNumTypes;
            this->RepeatSignatureSet(numTypes - 1);
            int k = 0;
            WCHAR* description = this->m_scanner->GetTagDescription();
            for (int i = 0; i < numTypes; i++)
            {
                WCHAR* name = this->m_scanner->GetName();
                WCHAR* typeName = (specifiedNumTypes == 0) ? L"Object" : this->m_scanner->GetTypeName(i);
                WCHAR* mappedTypeName = nullptr;
                WCHAR* elementType = nullptr;

                this->CanonicalizeType(typeName, &mappedTypeName, &elementType);

                for (int j = 0; j < numSignatures; j++)
                {
                    FunctionDocComments::Param* param = Anew(this->m_allocator, FunctionDocComments::Param);
                    param->type = mappedTypeName;
                    param->elementType = elementType;
                    param->name = name;
                    param->description = description;
                    param->optional = this->m_scanner->GetIsParamOptional();
                    if (this->m_scanner->HasValue())
                    {
                        param->value = this->m_scanner->GetValue();
                    }
                    this->m_functionDocComments->signatures.Item(k)->params.Add(param);
                    k++;
                }
            }
        }

        this->m_scanner->ClearTagDescriptions();
        this->m_scanner->ClearTypes();
    }

    void JsDocCommentsParser::OnPropertyTagParsed()
    {
        if (this->m_varDocComments != nullptr)
        {
            AssertMsg(false, "@property is not supported for describing variables.");
        }
        else if (this->m_fieldDocComments != nullptr)
        {
            if (this->m_fieldName == nullptr || wcscmp(this->m_fieldName, this->m_scanner->GetName()) == 0)
            {
                WCHAR* typeName = this->m_scanner->GetTypeNameCount() == 0 ? L"Object" : this->m_scanner->GetTypeName();
                wchar* mappedTypeName = nullptr;
                wchar* mappedElementType = nullptr;
                this->CanonicalizeType(typeName, &mappedTypeName, &mappedElementType);
                this->m_fieldDocComments->type = mappedTypeName;
                this->m_fieldDocComments->elementType = mappedElementType;
                this->m_fieldDocComments->name = this->m_scanner->GetName();
                this->m_fieldDocComments->description = this->m_scanner->GetTagDescription();
            }
        }
        else if (this->m_functionDocComments != nullptr)
        {
            FieldDocComments* fieldDocComments = Anew(this->m_allocator, FieldDocComments, this->m_allocator);

            WCHAR* typeName = this->m_scanner->GetTypeNameCount() == 0 ? L"Object" : this->m_scanner->GetTypeName();
            wchar* mappedTypeName = nullptr;
            wchar* mappedElementType = nullptr;
            this->CanonicalizeType(typeName, &mappedTypeName, &mappedElementType);
            fieldDocComments->type = mappedTypeName;
            fieldDocComments->elementType = mappedElementType;
            fieldDocComments->name = this->m_scanner->GetName();
            fieldDocComments->description = this->m_scanner->GetTagDescription();
            this->m_functionDocComments->fields.Add(fieldDocComments);
        }
        else if (this->m_typeDefintionSetDocComments != nullptr && this->m_typeDefintionSetDocComments->typeDefinitions.Count() > 0)
        {
            FieldDocComments* fieldDocComments = Anew(this->m_allocator, FieldDocComments, this->m_allocator);
            WCHAR* typeName = this->m_scanner->GetTypeNameCount() == 0 ? L"Object" : this->m_scanner->GetTypeName();
            wchar* mappedTypeName = nullptr;
            wchar* mappedElementType = nullptr;
            this->CanonicalizeType(typeName, &mappedTypeName, &mappedElementType);
            fieldDocComments->type = mappedTypeName;
            fieldDocComments->elementType = mappedElementType;
            fieldDocComments->name = this->m_scanner->GetName();
            fieldDocComments->description = this->m_scanner->GetTagDescription();
            this->m_typeDefintionSetDocComments->typeDefinitions.Last()->fields.Add(fieldDocComments);
        }

        this->m_scanner->ClearTagDescriptions();
        this->m_scanner->ClearTypes();
    }

    void JsDocCommentsParser::OnDeprecatedTagParsed()
    {
        if (this->m_functionDocComments != nullptr)
        {
            WCHAR* description = this->m_scanner->GetTagDescription();
            for (int i = 0; i < this->m_functionDocComments->signatures.Count(); i++)
            {
                this->m_functionDocComments->signatures.Item(i)->deprecated = Anew(this->m_allocator, Deprecated);
                this->m_functionDocComments->signatures.Item(i)->deprecated->message = description;
            }
        }

        this->m_scanner->ClearTagDescriptions();
    }

    void JsDocCommentsParser::OnDescriptionTagParsed()
    {
        this->m_scanner->ClearTagDescriptions();
    }

    void JsDocCommentsParser::OnSummaryTagParsed()
    {        
        this->m_scanner->ClearTagDescriptions();
    }

    void JsDocCommentsParser::OnTypeTagParsed()
    {
        if (this->m_varDocComments != nullptr && this->m_scanner->GetTypeNameCount() > 0)
        {
            WCHAR* typeName = this->m_scanner->GetTypeNameCount() == 0 ? L"Object" : this->m_scanner->GetTypeName();
            wchar* mappedTypeName = nullptr;
            wchar* mappedElementType = nullptr;
            this->CanonicalizeType(typeName, &mappedTypeName, &mappedElementType);
            this->m_varDocComments->type = mappedTypeName;
            this->m_varDocComments->elementType = mappedElementType;
            this->m_scanner->ClearTypes();
        }
        
        this->m_scanner->ClearTypes();
    }

    void JsDocCommentsParser::OnTypeDefTagParsed()
    {
        if (this->m_typeDefintionSetDocComments != nullptr)
        {
            TypeDefintionDocComments* typeDefintionDocComments = Anew(this->m_allocator, TypeDefintionDocComments, this->m_allocator);
            typeDefintionDocComments->name = this->m_scanner->GetName();
            typeDefintionDocComments->description = this->m_scanner->GetTaglessDescription();
            this->m_typeDefintionSetDocComments->typeDefinitions.Add(typeDefintionDocComments);
            this->m_scanner->ClearTypes();
            this->m_scanner->ClearTaglessDescriptions();
        }
    }

    void JsDocCommentsParser::OnTagLessDescriptionParsed()
    {
        if (this->m_seenTag == JsDocToken::none)
        {
            this->m_scanner->AddTaglessDescriptionLine();
        }
        else
        {
            this->m_scanner->AddTagDescriptionLine();
        }
    }

    /*
     * +-------------+----------------------+----------------------+-------------------+-----------------+-----------------------------------+
     * |             | Function Declaration | Variable Declaration | Field Declaration | Type Definition |Global Variable As FieldDeclaration|
     * +-------------+----------------------+----------------------+-------------------+-----------------+-----------------------------------+
     * |@param       | Yes                  | No                   | No                | No              | No                                |
     * |@returns     |                      |                      |                   |                 |                                   |
     * +-------------+----------------------+----------------------+-------------------+-----------------+-----------------------------------+
     * |@property    | No                   | No                   | Yes               | Yes             | No                                |
     * +-------------+----------------------+----------------------+-------------------+-----------------+-----------------------------------+
     * |@type        | No                   | Yes                  | Yes               | No              | Yes                               |
     * +-------------+----------------------+----------------------+-------------------+-----------------+-----------------------------------+
     * |@typedef     | No                   | No                   | No                | Yes             | No                                |
     * +-------------+----------------------+----------------------+-------------------+-----------------+-----------------------------------+
     * |@deprecated  | Yes                  | Yes                  | Yes               | Yes             | Yes                               |
     * |@description |                      |                      |                   |                 |                                   |
     * |@summary     |                      |                      |                   |                 |                                   |
     * +-------------+----------------------+----------------------+-------------------+-----------------+-----------------------------------+
     */

    bool JsDocCommentsParser::ShouldParseAtParamTag()
    {
        switch (this->m_parsingMode)
        {
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeNone:                             AssertMsg(false, "We should never parse without a context"); return false;
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeFieldDeclaration:                 return false;
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeGlobalVariableAsFieldDeclaration: return false;
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeFunctionDeclaration:              return true;
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeTypeDefinitions:                  return false;
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeVariableDeclaration:              return false;
        }
        AssertMsg(false, "This is just impossible, we have gone through all possibilities");
        return false;
    }

    bool JsDocCommentsParser::ShouldParseAtReturnsTag()
    {
        switch (this->m_parsingMode)
        {
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeNone:                             AssertMsg(false, "We should never parse without a context"); return false;
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeFieldDeclaration:                 return false;
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeGlobalVariableAsFieldDeclaration: return false;
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeFunctionDeclaration:              return true;
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeTypeDefinitions:                  return false;
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeVariableDeclaration:              return false;
        }
        AssertMsg(false, "This is just impossible, we have gone through all possibilities");
        return false;
    }

    bool JsDocCommentsParser::ShouldParseAtPropertyTag()
    {
        switch (this->m_parsingMode)
        {
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeNone:                             AssertMsg(false, "We should never parse without a context"); return false;
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeFieldDeclaration:                 return true;
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeGlobalVariableAsFieldDeclaration: return true;
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeFunctionDeclaration:              return true;
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeTypeDefinitions:                  return true;
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeVariableDeclaration:              return false;
        }
        AssertMsg(false, "This is just impossible, we have gone through all possibilities");
        return false;
    }

    bool JsDocCommentsParser::ShouldParseAtTypeTag()
    {
        switch (this->m_parsingMode)
        {
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeNone:                             AssertMsg(false, "We should never parse without a context"); return false;
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeFieldDeclaration:                 return true;
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeGlobalVariableAsFieldDeclaration: return true;
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeFunctionDeclaration:              return false;
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeTypeDefinitions:                  return false;
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeVariableDeclaration:              return true;
        }
        AssertMsg(false, "This is just impossible, we have gone through all possibilities");
        return false;
    }

    bool JsDocCommentsParser::ShouldParseAtTypeDefTag()
    {
        switch (this->m_parsingMode)
        {
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeNone:                             AssertMsg(false, "We should never parse without a context"); return false;
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeFieldDeclaration:                 return false;
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeGlobalVariableAsFieldDeclaration: return false;
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeFunctionDeclaration:              return false;
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeTypeDefinitions:                  return true;
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeVariableDeclaration:              return false;
        }
        AssertMsg(false, "This is just impossible, we have gone through all possibilities");
        return false;
    }

    bool JsDocCommentsParser::ShouldParseDescriptionTags()
    {
        switch (this->m_parsingMode)
        {
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeNone:                             (false, "We should never parse without a context"); return false;
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeFieldDeclaration:                 return true;
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeGlobalVariableAsFieldDeclaration: return true;
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeFunctionDeclaration:              return true;
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeTypeDefinitions:                  return true;
        case JsDocCommentsParsingMode::JsDocCommentsParsingModeVariableDeclaration:              return true;
        }
        AssertMsg(false, "This is just impossible, we have gone through all possibilities");
        return false;
    }

    void JsDocCommentsParser::Parse(ArenaAllocator* alloc, const WCHAR* text)
    {
        try
        {
            this->m_classifier = Anew(this->m_allocator, Js::CharClassifier, Js::CharClassifierModes::ES6, true);
            this->m_scanner = Anew(this->m_allocator, JsDocCommentsScanner, this->m_allocator, this->m_classifier, text);
            this->m_token = this->m_scanner->ScanStarTagOrDescription();
            this->ParseLines();
        }
        catch (JsDocParsingException&)
        {
            if (PHASE_TRACE1(Js::JsDocParsingPhase))
            {
                OUTPUT_TRACE(Js::JsDocParsingPhase, L"Ooops - JsDocParsingException occurred!\n");
            }
        }
    }

    // LINES = LINE endline LINES              
    //       = LINE eof 
    void JsDocCommentsParser::ParseLines()
    {
        this->ParseLine();
        switch (this->m_token)
        {
        case JsDocToken::eof:
            this->OnEndTag();
            return;
        case JsDocToken::endline:
            this->m_token = this->m_scanner->ScanStarTagOrDescription();
            this->ParseLines();
            return;
        default:
            throw JsDocParsingException();
        }
    }

    // STARORNOT    = star
    //              = EPSILON
    //              
    // LINE         = STARORNOT atreturns  TYPELIST description
    //              = STARORNOT atproperty TYPELIST name
    //              = STARORNOT attype     TYPELIST name
    //              = STARORNOT atparam    TYPELIST PARAMNAME description
    //              = STARORNOT atparam    TYPELIST PARAMNAME hyphen description
    //              = STARORNOT atdeprecated description
    //              = STARORNOT atdescription description
    //              = STARORNOT atsummary description
    //              = STARORNOT description
    //              = description
    void JsDocCommentsParser::ParseLine()
    {
        try
        {
            JsDocToken tag = JsDocToken::none;

            if (this->m_token == JsDocToken::star)
            {
                this->m_token = this->m_scanner->ScanTagOrDescription();
            }

            switch (this->m_token)
            {
            case JsDocToken::atreturns:
            case JsDocToken::atproperty:
            case JsDocToken::atparam:
            case JsDocToken::atdeprecated:
            case JsDocToken::atdescription:
            case JsDocToken::atsummary:
            case JsDocToken::attypedef:
                this->OnEndTag();
            }

            switch (this->m_token)
            {
            case JsDocToken::atreturns:
                if (this->ShouldParseAtReturnsTag())
                {
                    this->m_token = this->m_scanner->ScanOpenBraceOrDescription();
                    this->ParseTypeList(/* endsWithName = */ false);
                    if (this->m_token == JsDocToken::description)
                    {
                        this->m_scanner->AddTagDescriptionLine();
                        this->OnReturnsTagSeen();
                        this->m_token = this->m_scanner->ScanToken();
                    }
                    else
                    {
                        throw JsDocParsingException();
                    }
                }
                else
                {
                    this->m_token = this->m_scanner->ScanDescription(); // Consume all unknown symbol until end line
                    this->m_token = this->m_scanner->ScanToken();
                }

                break;

            case JsDocToken::atparam:
                if (this->ShouldParseAtParamTag())
                {
                    tag = this->m_token;
                    this->m_token = this->m_scanner->ScanOpenBraceOpenBracketOrName();
                    this->ParseTypeList(/* endsWithName = */ true);
                    this->ParseParamName();
                    if (this->m_token == JsDocToken::description)
                    {
                        this->m_scanner->AddTagDescriptionLine();
                        this->m_token = this->m_scanner->ScanToken();
                        this->OnParamTagSeen();
                    }
                    else if (this->m_token == JsDocToken::hyphen)
                    {
                        this->m_token = m_scanner->ScanDescription();
                        if (this->m_token == JsDocToken::description)
                        {
                            this->m_scanner->AddTagDescriptionLine();
                            this->m_token = this->m_scanner->ScanToken();
                            this->OnParamTagSeen();
                        }
                        else
                        {
                            throw JsDocParsingException();
                        }
                    }
                    else
                    {
                        throw JsDocParsingException();
                    }
                }
                else
                {
                    this->m_token = this->m_scanner->ScanDescription(); // Consume all unknown symbol until end line
                    this->m_token = this->m_scanner->ScanToken();
                }

                break;

            case JsDocToken::atproperty:
                if (this->ShouldParseAtPropertyTag())
                {
                    tag = this->m_token;
                    this->m_token = this->m_scanner->ScanOpenBraceOpenBracketOrName();
                    this->ParseTypeList(/* endsWithName = */ true);

                    if (this->m_token == JsDocToken::name)
                    {
                        this->m_token = this->m_scanner->ScanHyphenOrDescription();
                        if (this->m_token == JsDocToken::description)
                        {
                            this->m_scanner->AddTagDescriptionLine();
                            this->m_token = this->m_scanner->ScanToken();
                            this->OnPropertyTagSeen();
                        }
                        else if (this->m_token == JsDocToken::hyphen)
                        {
                            this->m_token = m_scanner->ScanDescription();
                            if (this->m_token == JsDocToken::description)
                            {
                                this->m_scanner->AddTagDescriptionLine();
                                this->m_token = this->m_scanner->ScanToken();
                                this->OnPropertyTagSeen();
                            }
                            else
                            {
                                throw JsDocParsingException();
                            }
                        }
                        else
                        {
                            throw JsDocParsingException();
                        }
                    }
                    else
                    {
                        throw JsDocParsingException();
                    }
                }
                else
                {
                    this->m_token = this->m_scanner->ScanDescription(); // Consume all unknown symbol until end line
                    this->m_token = this->m_scanner->ScanToken();
                }

                break;
            case JsDocToken::attype:
                if (this->ShouldParseAtTypeTag())
                {
                    tag = this->m_token;
                    this->m_token = this->m_scanner->ScanOpenBraceOpenBracketOrName();
                    this->ParseTypeList(/* endsWithName = */ true);

                    if (this->m_token == JsDocToken::name)
                    {
                        this->m_token = this->m_scanner->ScanToken();
                        this->OnTypeTagParsed();
                    }
                    else
                    {
                        throw JsDocParsingException();
                    }
                }
                else
                {
                    this->m_token = this->m_scanner->ScanDescription(); // Consume all unknown symbol until end line
                    this->m_token = this->m_scanner->ScanToken();
                }

                break;
            case JsDocToken::attypedef:
                if (this->ShouldParseAtTypeDefTag())
                {
                    tag = this->m_token;
                    this->m_token = this->m_scanner->ScanOpenBraceOpenBracketOrName();
                    this->ParseTypeList(/* endsWithName = */ true);

                    if (this->m_token == JsDocToken::name)
                    {
                        this->m_token = this->m_scanner->ScanToken();
                        this->OnTypeDefTagParsed();
                    }
                    else
                    {
                        throw JsDocParsingException();
                    }
                }
                else
                {
                    this->m_token = this->m_scanner->ScanDescription(); // Consume all unknown symbol until end line
                    this->m_token = this->m_scanner->ScanToken();
                    this->m_typeDefSeen = true;
                }

                break;
            case JsDocToken::atdeprecated:
                if (this->ShouldParseDescriptionTags())
                {
                    {
                        this->m_token = m_scanner->ScanDescription();
                        if (this->m_token == JsDocToken::description)
                        {
                            this->m_scanner->AddTagDescriptionLine();
                            this->m_token = this->m_scanner->ScanToken();
                            this->OnDeprecatedTagSeen();
                        }
                    }
                }
                else
                {
                    this->m_token = this->m_scanner->ScanDescription(); // Consume all unknown symbol until end line
                    this->m_token = this->m_scanner->ScanToken();
                }

                break;
            case JsDocToken::atdescription:
                if (this->ShouldParseDescriptionTags())
                {
                    this->m_token = m_scanner->ScanDescription();
                    if (this->m_token == JsDocToken::description)
                    {
                        this->m_scanner->AddTagDescriptionLine();
                        this->m_token = this->m_scanner->ScanToken();
                        this->OnDescriptionTagSeen();
                    }
                }
                else
                {
                    this->m_token = this->m_scanner->ScanDescription(); // Consume all unknown symbol until end line
                    this->m_token = this->m_scanner->ScanToken();
                }

                break;
            case JsDocToken::atsummary:
                if (this->ShouldParseDescriptionTags())
                {
                    this->m_token = m_scanner->ScanDescription();
                    if (this->m_token == JsDocToken::description)
                    {
                        this->m_scanner->AddTagDescriptionLine();
                        this->m_token = this->m_scanner->ScanToken();
                        this->OnSummaryTagSeen();
                    }
                }
                else
                {
                    this->m_token = this->m_scanner->ScanDescription(); // Consume all unknown symbol until end line
                    this->m_token = this->m_scanner->ScanToken();
                }

                break;
            case JsDocToken::description:
                if (this->ShouldParseDescriptionTags())
                {
                    this->m_token = this->m_scanner->ScanToken();
                    this->OnTagLessDescriptionParsed();
                }
                else
                {
                    this->m_token = this->m_scanner->ScanDescription(); // Consume all unknown symbol until end line
                    this->m_token = this->m_scanner->ScanToken();
                }

                break;
            case JsDocToken::atunknown:
                this->m_token = this->m_scanner->ScanDescription(); // Consume all unknown symbol until end line
                this->m_token = this->m_scanner->ScanToken();
                break;
            };
        }
        catch (JsDocParsingException&)
        {
            // TODO (andrewau) consider reporting the tolerated error
            this->m_token = this->m_scanner->ScanDescription(); // Consume all unknown symbol until end line
            this->m_token = this->m_scanner->ScanToken();

            if (PHASE_TRACE1(Js::JsDocParsingPhase))
            {
                OUTPUT_TRACE(Js::JsDocParsingPhase, L"Ooops - JsDocParsingException occurred - tolerated.\n");
            }
        }
    }

    // TYPELIST = openbrace TYPENAMELIST closebrace 
    //          = EPSILON
    void JsDocCommentsParser::ParseTypeList(bool endsWithName)
    {
        if (this->m_token == JsDocToken::openbrace)
        {
            this->m_token = this->m_scanner->ScanTypeName(/* allowSpace = */true);
        }
        else
        {
            return;
        }
        this->ParseTypeNameList();
        if (this->m_token != JsDocToken::closebrace)
        {
            this->m_scanner->Unscan(); /* undo the wrong token */
            this->m_scanner->Unscan(); /* undo the type name  */
            this->m_scanner->RemoveLastTypeName();
            this->m_token = this->m_scanner->ScanTypeName(/* allowSpace = */false);
            OUTPUT_TRACE(Js::JsDocParsingErrorsPhase, L"Missing '}' after specifying a type at (%d, %d))\n", this->m_scanner->GetLineNumber(), this->m_scanner->GetColumnNumber());
        }
        if (endsWithName)
        {
            this->m_token = this->m_scanner->ScanOpenBracketOrName();
        }
        else
        {
            this->m_token = this->m_scanner->ScanDescription();
        }
    }

    // TYPENAMELIST = typename
    //              = typename pipe TYPENAMELIST
    void JsDocCommentsParser::ParseTypeNameList()
    {
        if (this->m_token == JsDocToken::type_name)
        {
            this->m_token = this->m_scanner->ScanToken();
        }
        else
        {
            throw JsDocParsingException();
        }
        if (this->m_token == JsDocToken::pipe)
        {
            this->m_token = this->m_scanner->ScanTypeName(/* allowSpace = */true);
            this->ParseTypeNameList();
        }
    }

    // PARAMNAME    = name
    //              = openbracket name closebracket
    //              = openbracket name equals value closebracket
    void JsDocCommentsParser::ParseParamName()
    {
        if (this->m_token == JsDocToken::name)
        {
            // done
        }
        else
        {
            this->m_scanner->SetParamIsOptional();
            this->m_token = this->m_scanner->ScanName();
            if (this->m_token == JsDocToken::name)
            {
                this->m_token = this->m_scanner->ScanToken();
                if (this->m_token == JsDocToken::closebracket)
                {
                    // done
                } 
                else if (this->m_token == JsDocToken::equals)
                {
                    this->m_token = this->m_scanner->ScanValue(/* allowSpace = */true);
                    if (this->m_token == JsDocToken::value)
                    {
                        this->m_token = this->m_scanner->ScanToken();
                        if (this->m_token == JsDocToken::closebracket)
                        {
                            // done
                        }
                        else
                        {
                            this->m_scanner->Unscan(); /* undo the wrong token */
                            this->m_scanner->Unscan(); /* undo the wrong value */
                            this->m_token = this->m_scanner->ScanValue(/* allowSpace = */false);
                            OUTPUT_TRACE(Js::JsDocParsingErrorsPhase, L"Missing ']' after specifying the parameter as optional at (%d, %d))\n", this->m_scanner->GetLineNumber(), this->m_scanner->GetColumnNumber());
                        }
                    } 
                    else
                    {
                        throw JsDocParsingException();
                    }
                }
                else 
                {
                    this->m_scanner->Unscan(); /* undo the wrong token */
                    OUTPUT_TRACE(Js::JsDocParsingErrorsPhase, L"Missing ']' after specifying the parameter as optional at (%d, %d))\n", this->m_scanner->GetLineNumber(), this->m_scanner->GetColumnNumber());
                }
            }
            else
            {
                throw JsDocParsingException();
            }
        }
        this->m_token = m_scanner->ScanHyphenOrDescription();
    }

    // WARNING: The code does not clone all fields - it clones only fields that are set in this class.
    // That is why this method exists as a private method in this class - it is NOT intended for reuse outside this class.
    FunctionDocComments::Signature* JsDocCommentsParser::CloneSignature(FunctionDocComments::Signature* originalSignature) const
    {
        FunctionDocComments::Signature* cloneSignature = Anew(this->m_allocator, FunctionDocComments::Signature, this->m_allocator);

        FunctionDocComments::ReturnValue* originalReturnValue = originalSignature->returnValue;
        if (originalReturnValue != nullptr)
        {
            FunctionDocComments::ReturnValue* cloneReturnValue = Anew(this->m_allocator, FunctionDocComments::ReturnValue, this->m_allocator);
            cloneReturnValue->type = originalReturnValue->type;
            cloneReturnValue->description = originalReturnValue->description;
        }

        Deprecated* originalDeprecated = originalSignature->deprecated;
        if (originalDeprecated != nullptr)
        {
            Deprecated* cloneDeprecated = Anew(this->m_allocator, Deprecated);
            cloneDeprecated->message = originalDeprecated->message;
            cloneSignature->deprecated = cloneDeprecated;
        }

        int originalParamCount = originalSignature->params.Count();
        for (int k = 0; k < originalParamCount; k++)
        {
            FunctionDocComments::Param* originalParam = originalSignature->params.Item(k);
            FunctionDocComments::Param* cloneParam = Anew(this->m_allocator, FunctionDocComments::Param);

            cloneParam->type = originalParam->type;
            cloneParam->name = originalParam->name;
            cloneParam->description = originalParam->description;
            cloneSignature->params.Add(cloneParam);
        }

        return cloneSignature;
    }

    void JsDocCommentsParser::RepeatSignatureSet(int times) const
    {
        int numSignatures = this->m_functionDocComments->signatures.Count();
        for (int i = 0; i < times; i++)
        {
            for (int j = 0; j < numSignatures; j++)
            {
                FunctionDocComments::Signature* originalSignature = this->m_functionDocComments->signatures.Item(j);
                FunctionDocComments::Signature* cloneSignature = this->CloneSignature(originalSignature);
                this->m_functionDocComments->signatures.Add(cloneSignature);
            }
        }
    }

    void JsDocCommentsParser::CanonicalizeType(_In_z_ wchar* rawTypeName, _Outptr_result_z_ wchar** mappedTypeName, _Outptr_result_z_ wchar** mappedElementType) const
    {
        Assert(rawTypeName != nullptr);
        Assert(mappedTypeName != nullptr);
        Assert(mappedElementType != nullptr);

        *mappedTypeName = rawTypeName;

        // Special mapping string for community frequently used type string that need translation
        const wchar_t arrayPrefix[] = L"Array";
        const wchar_t functionPrefix[] = L"function";
        const unsigned int arrayPrefixLength = _countof(arrayPrefix) - 1;
        const unsigned int functionPrefixLength = _countof(functionPrefix) - 1;

        if (wcscmp(rawTypeName, L"*") == 0)
        {
            *mappedTypeName = L"Object";
        }
        else if (_wcsicmp(rawTypeName, L"String") == 0)
        {
            *mappedTypeName = L"String";
        }
        else if (_wcsicmp(rawTypeName, L"Number") == 0)
        {
            *mappedTypeName = L"Number";
        }
        else if (_wcsicmp(rawTypeName, L"Object") == 0)
        {
            *mappedTypeName = L"Object";
        }
        else if (_wcsicmp(rawTypeName, L"Boolean") == 0)
        {
            *mappedTypeName = L"Boolean";
        }
        else if (wcsncmp(rawTypeName, functionPrefix, functionPrefixLength) == 0) /** NOTE: Prefix match! **/
        {
            int typeNameLength = wcslen(rawTypeName);
            if (typeNameLength <= functionPrefixLength || rawTypeName[functionPrefixLength] == '(' || this->m_classifier->IsWhiteSpace(rawTypeName[functionPrefixLength]))
            {
                *mappedTypeName = L"Function";
            }
        }
        else if (wcsncmp(rawTypeName, arrayPrefix, arrayPrefixLength) == 0)
        {
            int typeNameLength = wcslen(rawTypeName);
            if (typeNameLength > arrayPrefixLength + 1)
            {
                WCHAR lastChar = rawTypeName[typeNameLength - 1];
                WCHAR* elementTypeStart = nullptr;
                unsigned int elementTypeLength = 0;

                if (rawTypeName[arrayPrefixLength] == L'.' && rawTypeName[arrayPrefixLength + 1] == L'<')
                {
                    if (lastChar == '>')
                    {
                        elementTypeStart = rawTypeName + arrayPrefixLength + 2;        /* .<  has 2 characters */
                        elementTypeLength = typeNameLength - arrayPrefixLength - 3; /* .<> has 3 characters */
                    }
                    else
                    {
                        elementTypeStart = rawTypeName + arrayPrefixLength + 2;        /* .<  has 2 characters */
                        elementTypeLength = typeNameLength - arrayPrefixLength - 2; /* .<  has 2 characters */
                        OUTPUT_TRACE(Js::JsDocParsingErrorsPhase, L"Missing '>' after specifying a array element type at (%d, %d))\n", this->m_scanner->GetLineNumber(), this->m_scanner->GetColumnNumber());
                    }
                }
                else if (rawTypeName[arrayPrefixLength] == L'[')
                {
                    if (lastChar == ']')
                    {
                        elementTypeStart = rawTypeName + arrayPrefixLength + 1;        /* [  has 1 character  */
                        elementTypeLength = typeNameLength - arrayPrefixLength - 2; /* [] has 2 characters */
                    }
                    else
                    {
                        elementTypeStart = rawTypeName + arrayPrefixLength + 1;        /* [  has 1 character  */
                        elementTypeLength = typeNameLength - arrayPrefixLength - 1; /* [  has 1 characters */
                    }
                    OUTPUT_TRACE(Js::JsDocParsingErrorsPhase, L"Missing ']' after specifying a array element type at (%d, %d))\n", this->m_scanner->GetLineNumber(), this->m_scanner->GetColumnNumber());
                }

                if (elementTypeStart != nullptr)
                {
                    *mappedTypeName = L"Array";
                    *mappedElementType = AnewArray(this->m_allocator, WCHAR, elementTypeLength + 1); /* so it has null terminating 0 */
                    js_wmemcpy_s(*mappedElementType, elementTypeLength, elementTypeStart, elementTypeLength);
                    (*mappedElementType)[elementTypeLength] = L'\0';
                }
            }
        }

    }

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    void JsDocCommentsParser::TraceFunctionDocComments(LPCWSTR signatureText, FunctionDocComments* result)
    {
        if (PHASE_TRACE1(Js::JsDocParsingPhase))
        {
            Output::Print(L"JsDocCommentsParser::ParseFuncDocComments:\n");
            Output::Print(L"{\n  input :\n  {");
            if (signatureText != nullptr)
            {
                Output::Print(L"\n    signatureText : %ls", Js::JSONString::EscapeNonEmptyString(this->m_allocator, signatureText));
            }

            Output::Print(L"\n  },\n  output :\n  {\n    signatures :\n    [");
            for (int i = 0; i < result->signatures.Count(); i++)
            {
                if (i != 0)
                {
                    Output::Print(L",\n");
                }
                else
                {
                    Output::Print(L"\n");
                }

                bool signatureProperty = false;
                Output::Print(L"      {");
                if (result->signatures.Item(i)->description != nullptr)
                {
                    Output::Print(L"\n        description : %ls", Js::JSONString::EscapeNonEmptyString(this->m_allocator, result->signatures.Item(i)->description));
                    signatureProperty = true;
                }
                if (result->signatures.Item(i)->deprecated != nullptr)
                {
                    if (signatureProperty)
                    {
                        Output::Print(L",\n");
                    }
                    else
                    {
                        Output::Print(L"\n");
                    }

                    Output::Print(L"        deprecated : %ls", Js::JSONString::EscapeNonEmptyString(this->m_allocator, result->signatures.Item(i)->deprecated->message));
                    signatureProperty = true;
                }

                if (signatureProperty)
                {
                    Output::Print(L",\n");
                }
                else
                {
                    Output::Print(L"\n");
                }

                Output::Print(L"        parameters :\n        [");
                for (int j = 0; j < result->signatures.Item(i)->params.Count(); j++)
                {
                    if (j != 0)
                    {
                        Output::Print(L",\n");
                    }
                    else
                    {
                        Output::Print(L"\n");
                    }

                    Output::Print(L"          {");
                    bool parameterProperty = false;
                    if (result->signatures.Item(i)->params.Item(j)->type != nullptr)
                    {
                        Output::Print(L"\n            type : %ls", Js::JSONString::EscapeNonEmptyString(this->m_allocator, result->signatures.Item(i)->params.Item(j)->type));
                        parameterProperty = true;
                    }

                    if (result->signatures.Item(i)->params.Item(j)->name != nullptr)
                    {
                        if (parameterProperty)
                        {
                            Output::Print(L",\n");
                        }
                        else
                        {
                            Output::Print(L"\n");
                        }

                        Output::Print(L"            name : %ls", Js::JSONString::EscapeNonEmptyString(this->m_allocator, result->signatures.Item(i)->params.Item(j)->name));
                        parameterProperty = true;
                    }
                    if (result->signatures.Item(i)->params.Item(j)->description != nullptr)
                    {
                        if (parameterProperty)
                        {
                            Output::Print(L",\n");
                        }
                        else
                        {
                            Output::Print(L"\n");
                        }

                        Output::Print(L"            description : %ls", Js::JSONString::EscapeNonEmptyString(this->m_allocator, result->signatures.Item(i)->params.Item(j)->description));
                        parameterProperty = true;
                    }
                    if (parameterProperty)
                    {
                        Output::Print(L",\n");
                    }
                    else
                    {
                        Output::Print(L"\n");
                    }

                    Output::Print(L"            optional : %ls", Js::JSONString::EscapeNonEmptyString(this->m_allocator, (result->signatures.Item(i)->params.Item(j)->optional ? L"true" : L"false")));
                    parameterProperty = true;

                    if (result->signatures.Item(i)->params.Item(j)->value != nullptr)
                    {
                        if (parameterProperty)
                        {
                            Output::Print(L",\n");
                        }
                        else
                        {
                            Output::Print(L"\n");
                        }

                        Output::Print(L"            value : %ls", Js::JSONString::EscapeNonEmptyString(this->m_allocator, result->signatures.Item(i)->params.Item(j)->value));
                        parameterProperty = true;
                    }

                    Output::Print(L"\n          }");
                }

                Output::Print(L"\n        ]");

                if (result->signatures.Item(i)->returnValue != nullptr)
                {
                    Output::Print(L",\n        returns :\n        {");
                    bool returnProperty = false;
                    if (result->signatures.Item(i)->returnValue->type != nullptr)
                    {
                        Output::Print(L"\n          type : %ls", Js::JSONString::EscapeNonEmptyString(this->m_allocator, result->signatures.Item(i)->returnValue->type));
                        returnProperty = true;
                    }

                    if (result->signatures.Item(i)->returnValue->description != nullptr)
                    {
                        if (returnProperty)
                        {
                            Output::Print(L",\n");
                        }
                        else
                        {
                            Output::Print(L"\n");
                        }

                        Output::Print(L"          description : %ls", Js::JSONString::EscapeNonEmptyString(this->m_allocator, result->signatures.Item(i)->returnValue->description));
                        returnProperty = true;
                    }
                    Output::Print(L"\n        }");
                }

                Output::Print(L"\n      }");
            }

            Output::Print(L"\n    ]");
            Output::Print(L"\n  }");
            Output::Print(L"\n}\n");
        }
    }

    void JsDocCommentsParser::TraceVariableDocComments(LPCWSTR text, VarDocComments* result)
    {
        if (PHASE_TRACE1(Js::JsDocParsingPhase))
        {
            Output::Print(L"JsDocCommentsParser::ParseVarDocComments:\n");
            Output::Print(L"{\n  input :\n  {");
            if (text != nullptr)
            {
                Output::Print(L"\n    text : %ls", Js::JSONString::EscapeNonEmptyString(this->m_allocator, text));
            }

            Output::Print(L"\n  },\n  output :\n  {\n    VarDocComments :\n    {");
            bool varDocProperties = false;
            if (result->type != nullptr)
            {
                Output::Print(L"\n        type : %ls", Js::JSONString::EscapeNonEmptyString(this->m_allocator, result->type));
                varDocProperties = true;
            }

            if (varDocProperties)
            {
                Output::Print(L",\n");
            }
            else
            {
                Output::Print(L"\n");
            }

            if (result->description != nullptr)
            {
                Output::Print(L"\n        description : %ls", Js::JSONString::EscapeNonEmptyString(this->m_allocator, result->description));
                varDocProperties = true;
            }

            Output::Print(L"\n    }");
            Output::Print(L"\n  }");
            Output::Print(L"\n}");
            Output::Print(L"\n");
        }
    }

    void JsDocCommentsParser::TraceFieldDocComments(LPCWSTR text, FieldDocComments* result)
    {
        if (PHASE_TRACE1(Js::JsDocParsingPhase))
        {
            Output::Print(L"JsDocCommentsParser::ParseFieldDocComments:\n");
            Output::Print(L"{\n  input :\n  {");
            if (text != nullptr)
            {
                Output::Print(L"\n    text : %ls", Js::JSONString::EscapeNonEmptyString(this->m_allocator, text));
            }

            Output::Print(L"\n  },\n  output :\n  {\n    FieldDocComments :\n    {");
            bool fieldDocProperties = false;
            if (result->type != nullptr)
            {
                Output::Print(L"\n        type : %ls", Js::JSONString::EscapeNonEmptyString(this->m_allocator, result->type));
                fieldDocProperties = true;
            }

            if (fieldDocProperties)
            {
                Output::Print(L",\n");
            }
            else
            {
                Output::Print(L"\n");
            }

            if (result->description != nullptr)
            {
                Output::Print(L"\n        description : %ls", Js::JSONString::EscapeNonEmptyString(this->m_allocator, result->description));
                fieldDocProperties = true;
            }

            Output::Print(L"\n    }");
            Output::Print(L"\n  }");
            Output::Print(L"\n}");
            Output::Print(L"\n");
        }
    }

    void JsDocCommentsParser::TraceTypeDefinitionDocComments(LPCWSTR text, TypeDefintionSetDocComments* result)
    {
        if (PHASE_TRACE1(Js::JsDocParsingPhase))
        {
            Output::Print(L"JsDocCommentsParser::ParseTypeDefinitionDocComments:\n");
            Output::Print(L"{\n  input :\n  {");
            if (text != nullptr)
            {
                Output::Print(L"\n    text : %ls", Js::JSONString::EscapeNonEmptyString(this->m_allocator, text));
            }

            Output::Print(L"\n  },\n  output :\n  {\n    TypeDefinitions :\n    [");
            for (int i = 0; i < result->typeDefinitions.Count(); i++)
            {
                if (i != 0)
                {
                    Output::Print(L",\n");
                }
                else
                {
                    Output::Print(L"\n");
                }

                bool typeDefinitionProperties = false;
                Output::Print(L"      {");
                if (result->typeDefinitions.Item(i)->name != nullptr)
                {
                    Output::Print(L"\n        name : %ls", Js::JSONString::EscapeNonEmptyString(this->m_allocator, result->typeDefinitions.Item(i)->name));
                    typeDefinitionProperties = true;
                }

                if (typeDefinitionProperties)
                {
                    Output::Print(L",\n");
                }
                else
                {
                    Output::Print(L"\n");
                }

                if (result->typeDefinitions.Item(i)->description != nullptr)
                {
                    Output::Print(L"        description : %ls", Js::JSONString::EscapeNonEmptyString(this->m_allocator, result->typeDefinitions.Item(i)->description));
                    typeDefinitionProperties = true;
                }

                if (typeDefinitionProperties)
                {
                    Output::Print(L",\n");
                }
                else
                {
                    Output::Print(L"\n");
                }

                Output::Print(L"        properties :\n        [");
                for (int j = 0; j < result->typeDefinitions.Item(i)->fields.Count(); j++)
                {
                    if (j != 0)
                    {
                        Output::Print(L",\n");
                    }
                    else
                    {
                        Output::Print(L"\n");
                    }
                    Output::Print(L"          {");
                    bool fieldProperty = false;
                    if (result->typeDefinitions.Item(i)->fields.Item(j)->type != nullptr)
                    {
                        Output::Print(L"\n            type : %ls", Js::JSONString::EscapeNonEmptyString(this->m_allocator, result->typeDefinitions.Item(i)->fields.Item(j)->type));
                        fieldProperty = true;
                    }

                    if (result->typeDefinitions.Item(i)->fields.Item(j)->name != nullptr)
                    {
                        if (fieldProperty)
                        {
                            Output::Print(L",\n");
                        }
                        else
                        {
                            Output::Print(L"\n");
                        }

                        Output::Print(L"            name : %ls", Js::JSONString::EscapeNonEmptyString(this->m_allocator, result->typeDefinitions.Item(i)->fields.Item(j)->name));
                        fieldProperty = true;
                    }
                    if (result->typeDefinitions.Item(i)->fields.Item(j)->description != nullptr)
                    {
                        if (fieldProperty)
                        {
                            Output::Print(L",\n");
                        }
                        else
                        {
                            Output::Print(L"\n");
                        }

                        Output::Print(L"            description : %ls", Js::JSONString::EscapeNonEmptyString(this->m_allocator, result->typeDefinitions.Item(i)->fields.Item(j)->description));
                        fieldProperty = true;
                    }
                    Output::Print(L"\n          }");
                }

                Output::Print(L"\n        ]");
                Output::Print(L"\n      }");
            }
            Output::Print(L"\n    ]");
            Output::Print(L"\n  }");
            Output::Print(L"\n}");
            Output::Print(L"\n");
        }
    }
#endif 
}
