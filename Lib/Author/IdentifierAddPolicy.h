//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

namespace Authoring
{
    class Completions;

    struct HintInfo
    {
        AuthorScope scope;
        AuthorType  type;
        int         fileId;
        charcount_t funcSourceMin;
        LPCWSTR     typeName;
        LPCWSTR     description;

        HintInfo() : scope(ascopeUnknown), typeName(nullptr), description(nullptr), type(atUnknown), fileId(-1), funcSourceMin(0) { }
        HintInfo(AuthorScope scope, AuthorType type, int fileId, charcount_t funcSourceMin) 
            : scope(scope), type(type), typeName(nullptr), description(nullptr), fileId(fileId), funcSourceMin(funcSourceMin) { }
    };

    struct IdentInfo
    {
    public:
        IdentPtr ident;
        AuthorCompletionKind kind;
        AuthorCompletionFlags group;
        HintInfo* hintInfo;
    
        IdentInfo() { }
        IdentInfo(IdentPtr ident, AuthorCompletionKind kind, AuthorCompletionFlags group, HintInfo* hintInfo)
            : ident(ident), kind(kind), group(group), hintInfo(hintInfo) { }
    };

    class FileAuthoring;

    class IdentifierContext
    {
    private:
        Js::ScriptContext   *context;
        Completions         *completions;
        ArenaAllocator      *alloc;
        int                 fileId;
        FileAuthoring*      fileAuthoring;
    public:

        IdentifierContext(Js::ScriptContext *context, Completions *completions, ArenaAllocator* alloc, int fileId, FileAuthoring* fileAuthoring)
            : context(context), completions(completions), alloc(alloc), fileId(fileId), fileAuthoring(fileAuthoring)
        { 
            Assert(alloc != nullptr);
        }

        void Add(IdentInfo info);
        void AddPid(IdentPtr pid, AuthorCompletionKind kind, AuthorCompletionFlags flags, HintInfo* hintInfo);

        ArenaAllocator* Alloc() { return alloc; }
        int FileId() { return fileId; }
        bool Abort();
    };

    class IdentifierPreorderAddPolicy : public VisitorPolicyBase<IdentifierContext *>
    {
    private:
        IdentPtr RightNameOf(ParseNode *pnode);
        void AddStrs(ParseNode *pnode, AuthorCompletionKind kind, IdentifierContext* context);

    protected:
        bool Preorder(ParseNode *pnode, IdentifierContext* context);
    };
}
