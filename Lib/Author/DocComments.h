//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// Description: 

#pragma once

namespace Authoring
{
    class CommentBuffer : public TextBuffer
    {
    public:
        CommentBuffer(ArenaAllocator *alloc);
        virtual ~CommentBuffer();
        static CommentBuffer *New(ArenaAllocator *alloc);
        void SetCommentType(CommentType commentType);
        CommentType GetCommentType();
    private:
        CommentType commentType;
    };

    struct ParseError
    {
        HRESULT hr;
        uint line;
        charcount_t pos;
        ParseError(HRESULT hr, uint line, charcount_t pos) : hr(hr), line(line), pos(pos) 
        {
            Assert(FAILED(hr));
        }
    };

    struct CompatibleWith
    {
        LPCWSTR platform;
        LPCWSTR minVersion;
        CompatibleWith() : platform(nullptr), minVersion(nullptr) { }
    };

    struct Deprecated
    {
        LPCWSTR type;
        LPCWSTR message;
        Deprecated() : type(nullptr), message(nullptr) { }
    };

    struct DocComment
    {
        ParseError* parseError;
        LPCWSTR type;
        LPCWSTR description;
        LPCWSTR locid;
        LPCWSTR elementType;
        LPCWSTR value;
        DocComment() : parseError(nullptr), type(nullptr), description(nullptr), locid(nullptr), elementType(nullptr), value(nullptr) { }
    };

    struct VarDocComments : DocComment
    {
        LPCWSTR externalFile;
        LPCWSTR externalid;
        LPCWSTR helpKeyword;
        Deprecated* deprecated;
        JsUtil::List<CompatibleWith*, ArenaAllocator> compatibleWith;
        VarDocComments(ArenaAllocator* alloc) : compatibleWith(alloc), externalFile(nullptr), externalid(nullptr), helpKeyword(nullptr), deprecated(nullptr) { }
    };

    struct FieldDocComments : VarDocComments
    {
        LPCWSTR name;
        bool isStatic;
        FieldDocComments(ArenaAllocator* alloc) : VarDocComments(alloc), name(nullptr), isStatic(false) { }
    };

    struct TypeDefintionDocComments
    {
        TypeDefintionDocComments(ArenaAllocator* alloc) : m_alloc(alloc), fields(alloc), name(nullptr), description(nullptr) {}
        LPCWSTR name;
        LPCWSTR description;
        JsUtil::List<FieldDocComments*, ArenaAllocator> fields;
        ArenaAllocator* m_alloc;
    };

    struct TypeDefintionSetDocComments
    {
        TypeDefintionSetDocComments(ArenaAllocator* alloc) : m_alloc(alloc), typeDefinitions(alloc) {}
        JsUtil::List<TypeDefintionDocComments*, ArenaAllocator> typeDefinitions;
        ArenaAllocator* m_alloc;
    };

    struct FunctionDocComments
    {
        struct Signature;

        struct Param : DocComment
        {
            LPCWSTR name;
            bool optional;
            Signature* signature;

            Param() : name(nullptr), optional(false), signature(nullptr) { }
        };

        struct ReturnValue : VarDocComments
        {
            ReturnValue(ArenaAllocator* alloc) : VarDocComments(alloc) { }
        };

        struct Signature
        {
            LPCWSTR description;
            LPCWSTR locid;
            ReturnValue* returnValue;
            LPCWSTR externalFile;
            LPCWSTR externalid ;
            LPCWSTR helpKeyword;
            Deprecated* deprecated;
            JsUtil::List<Param*, ArenaAllocator> params;
            JsUtil::List<CompatibleWith*, ArenaAllocator> compatibleWith;

            Signature(ArenaAllocator* alloc) : params(alloc), compatibleWith(alloc), description(nullptr), locid(nullptr), returnValue(nullptr), externalFile(nullptr), 
                externalid(nullptr), helpKeyword(nullptr), deprecated(nullptr) 
            { }

            Param* FindParam(LPCWSTR name);
        };

        ParseError* parseError;
        Signature* implicitSignature;
        JsUtil::List<Signature*, ArenaAllocator> signatures;
        JsUtil::List<FieldDocComments*, ArenaAllocator> fields;

        FunctionDocComments(ArenaAllocator* alloc) : parseError(nullptr), signatures(alloc), fields(alloc), implicitSignature(nullptr) { }

        FunctionDocComments::Signature* FirstSignature() 
        {
            auto firstSignature = (signatures.Count() > 0) ? signatures.Item(0) : nullptr;
            return firstSignature ? firstSignature : implicitSignature;
        }
    };

    HRESULT ParseFuncDocComments(__in ArenaAllocator* alloc, __in LPCWSTR docText, __in CommentType commentType, __out_ecount(1) FunctionDocComments** funcDoc);
    HRESULT ParseVarDocComments(__in ArenaAllocator* alloc, __in LPCWSTR docText, __in CommentType commentType, __out_ecount(1) VarDocComments** varDoc);
    HRESULT ParseFieldDocComments(__in ArenaAllocator* alloc, __in LPCWSTR fieldName, __in LPCWSTR docText, __in CommentType commentType, __in bool isGlobalVariableAsField, __out_ecount(1) FieldDocComments** fieldDoc);
    HRESULT ParseTypeDefinitionComments(__in ArenaAllocator* alloc, __in LPCWSTR docText, __in CommentType commentType, __out_ecount(1) TypeDefintionSetDocComments** typeDefintionSetDocComments);
}
