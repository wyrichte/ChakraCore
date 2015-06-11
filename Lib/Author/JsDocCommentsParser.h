//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#include "JsDocToken.h"
#include "JsDocCommentsScanner.h"


namespace Authoring
{
    class AutoFunctionDocCommentsResetter;
    class AutoVarDocCommentsResetter;
    class AutoFieldDocCommentsResetter;
    class AutoTypeDefintionSetDocCommentsResetter;

    namespace Names
    {
        // JSDoc tags
        const wchar_t jsdoc_return_tag[] = L"@return";
        const wchar_t jsdoc_returns_tag[] = L"@returns";
        const wchar_t jsdoc_property_tag[] = L"@property";
        const wchar_t jsdoc_param_tag[] = L"@param";
        const wchar_t jsdoc_param_tag_tag[] = L"@paramTag";
        const wchar_t jsdoc_deprecated_tag[] = L"@deprecated";
        const wchar_t jsdoc_type_tag[] = L"@type";
        const wchar_t jsdoc_description_tag[] = L"@description";
        const wchar_t jsdoc_summary_tag[] = L"@summary";
        const wchar_t jsdoc_todo_tag[] = L"@todo";
        const wchar_t jsdoc_typedef_tag[] = L"@typedef";
    }

    enum JsDocCommentsParsingMode
    {
        JsDocCommentsParsingModeNone,
        JsDocCommentsParsingModeVariableDeclaration,
        JsDocCommentsParsingModeFieldDeclaration,
        JsDocCommentsParsingModeGlobalVariableAsFieldDeclaration,
        JsDocCommentsParsingModeFunctionDeclaration,
        JsDocCommentsParsingModeTypeDefinitions
    };

    //
    // JsDocCommentsParser is a class used to parse a string of JsDocComments 
    // (with the Javascript comment delimiters removed by the JavaScript Parser)
    //
    // This is a top down, predictive, recursive parser, the grammar of the jsdoc 
    // comments supported is, in BNF form, as follow:
    //
    // GOAL         = LINES
    //              
    // LINES        = LINE endline LINES              
    //              = LINE eof 
    //              
    // STARORNOT    = star
    //              = EPSILON
    //              
    // LINE         = STARORNOT atreturns  TYPELIST description
    //              = STARORNOT attype     TYPELIST name
    //              = STARORNOT attypedef  TYPELIST name
    //              = STARORNOT atproperty TYPELIST name description
    //              = STARORNOT atproperty TYPELIST name hyphen description
    //              = STARORNOT atparam    TYPELIST PARAMNAME description
    //              = STARORNOT atparam    TYPELIST PARAMNAME hyphen description
    //              = STARORNOT atdeprecated description
    //              = STARORNOT atdescription description
    //              = STARORNOT atsummary description
    //              = STARORNOT description
    //              = description
    //
    // PARAMNAME    = name
    //              = openbracket name closebracket
    //              = openbracket name equals value closebracket
    //              
    // TYPELIST     = openbrace TYPENAMELIST closebrace 
    //              = EPSILON
    //
    // TYPENAMELIST = typename
    //              = typename pipe TYPENAMELIST
    //
    class JsDocCommentsParser
    {
    public:
        JsDocCommentsParser();
        HRESULT ParseFuncDocComments(ArenaAllocator* alloc, LPCWSTR signatureText, FunctionDocComments** funcDocComments);
        HRESULT ParseVarDocComments(ArenaAllocator* alloc, LPCWSTR text, VarDocComments** varDocComments);
        HRESULT ParseFieldDocComments(ArenaAllocator* alloc, LPCWSTR fieldName, LPCWSTR text, bool isGlobalVariableAsField, FieldDocComments** fieldDocComments);
        HRESULT ParseTypeDefinitionComments(ArenaAllocator* alloc, LPCWSTR text, TypeDefintionSetDocComments** typeDefintionSetDocComments);
        void Parse(ArenaAllocator* alloc, const TCHAR* text);

    private:
        // Control recognition of tags under different parsing contexts
        bool ShouldParseAtParamTag();
        bool ShouldParseAtReturnsTag();
        bool ShouldParseAtPropertyTag();
        bool ShouldParseAtTypeTag();
        bool ShouldParseAtTypeDefTag();
        bool ShouldParseDescriptionTags();

        // Top down recursive parsing methods
        void ParseLines();
        void ParseLine();
        void ParseTypeList(bool endsWithName);
        void ParseTypeNameList();
        void ParseParamName();

        // Callbacks when parsing (of an interesting non-terminal) is done
        void OnReturnsTagParsed();
        void OnPropertyTagParsed();
        void OnParamTagParsed();
        void OnDeprecatedTagParsed();
        void OnDescriptionTagParsed();
        void OnSummaryTagParsed();
        void OnTypeTagParsed();
        void OnTypeDefTagParsed();
        void OnTagLessDescriptionParsed();
        
        // Callback when parsing (of an interesting tag) of the LINE is done 
        void OnReturnsTagSeen();
        void OnParamTagSeen();
        void OnPropertyTagSeen();
        void OnDeprecatedTagSeen();
        void OnDescriptionTagSeen();
        void OnSummaryTagSeen();

        void OnEndTag();

        // Utilities
        FunctionDocComments::Signature* CloneSignature(FunctionDocComments::Signature* originalSignature) const;
        void RepeatSignatureSet(int times) const;
        void CanonicalizeType(_In_z_ wchar* rawTypeName, _Outptr_result_z_ wchar** mappedTypeName, _Outptr_result_z_ wchar** mappedElementType) const;

        // Tracing
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        void TraceFunctionDocComments(LPCWSTR signatureText, FunctionDocComments* result);
        void TraceVariableDocComments(LPCWSTR text, VarDocComments* result);
        void TraceFieldDocComments(LPCWSTR text, FieldDocComments* result);
        void TraceTypeDefinitionDocComments(LPCWSTR text, TypeDefintionSetDocComments* result);
#endif

        // The current token - all Parse* methods are expected m_token is the first token of the current non-terminal.
        //                   - and all Parse* methods are expected to grab the next token of the next non-terminal before it returns.
        JsDocToken m_token;

        // Various needed resources for parsing purposes
        ArenaAllocator* m_allocator;
        Js::CharClassifier* m_classifier;
        JsDocCommentsScanner* m_scanner;

        // This is non-null only when ParseFuncDocComments() is called.
        FunctionDocComments* m_functionDocComments;

        // This is non-null only when ParseVarDocComments() is called.
        VarDocComments* m_varDocComments;

        // This is non-null only when ParseFieldDocComments() is called.
        FieldDocComments* m_fieldDocComments;

        // This is non-null only when ParseFieldDocComments() is called with a non-null fieldName
        const wchar* m_fieldName;

        // This is non-null only when ParseTypeDefinitionComments() is called.
        TypeDefintionSetDocComments* m_typeDefintionSetDocComments;

        // This is set when a seen method is called
        JsDocToken m_seenTag;

        // This is the context where parsing is called
        JsDocCommentsParsingMode m_parsingMode;

        // This is used to invalidate comment parsing result if the comment buffer is meant for parsing type defs
        bool m_typeDefSeen;

        friend class AutoFunctionDocCommentsResetter;
        friend class AutoVarDocCommentsResetter;
        friend class AutoFieldDocCommentsResetter;
        friend class AutoTypeDefintionSetDocCommentsResetter;
        friend class AutoJsDocCommentsParsingModeResetter;
    };

    class AutoFunctionDocCommentsResetter
    {
    public:
        AutoFunctionDocCommentsResetter(JsDocCommentsParser* parent, FunctionDocComments* result);
        ~AutoFunctionDocCommentsResetter();
    private:
        JsDocCommentsParser* m_parent;
    };

    class AutoVarDocCommentsResetter
    {
    public:
        AutoVarDocCommentsResetter(JsDocCommentsParser* parent, VarDocComments* result);
        ~AutoVarDocCommentsResetter();
    private:
        JsDocCommentsParser* m_parent;
    };

    class AutoFieldDocCommentsResetter
    {
    public:
        AutoFieldDocCommentsResetter(JsDocCommentsParser* parent, const wchar* fieldName, FieldDocComments* result);
        ~AutoFieldDocCommentsResetter();
    private:
        JsDocCommentsParser* m_parent;
    };

    class AutoTypeDefintionSetDocCommentsResetter
    {
    public:
        AutoTypeDefintionSetDocCommentsResetter(JsDocCommentsParser* parent, TypeDefintionSetDocComments* result);
        ~AutoTypeDefintionSetDocCommentsResetter();
    private:
        JsDocCommentsParser* m_parent;
    };

    class AutoJsDocCommentsParsingModeResetter
    {
    public:
        AutoJsDocCommentsParsingModeResetter(JsDocCommentsParser* parent, JsDocCommentsParsingMode mode);
        ~AutoJsDocCommentsParsingModeResetter();
    private:
        JsDocCommentsParser* m_parent;
    };
}
