//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

namespace Authoring 
{
    class AuthorParseNodeCursor;
    class AuthoringFileText;

    typedef JsUtil::List<AuthorSymbolReference, ArenaAllocator> ReferenceList;

    class ParseNodeTree 
    {
    public:
        enum MutationKind { mutNone = 0, mutDocComments = 1, mutDirectExecution = 2, mutForceRegisterLoad = 4, mutLoopGuards = 8, mutLevelTruncation = 16};
    private:
        ArenaAllocator* m_alloc;
        ArenaAllocator* m_parserAlloc;
        Parser* m_parser;
        ParseNodePtr m_nodeTree;
        LanguageServiceExtension* m_parserLSExtension;
        AuthoringFileText* m_text;
        bool m_treeWasGenerated;
        JsUtil::List<AuthorParseNodeCursor*, ArenaAllocator> m_activeCursors;
        MutationKind m_mutations;
        int m_lastRememberedComment;

        void UpdateVersion() { ReleaseActiveCursors(); }
        void ReleaseActiveCursors();

    public:

        ParseNodeTree(ArenaAllocator *alloc): m_nodeTree(nullptr), m_parserLSExtension(nullptr), m_treeWasGenerated(false), m_text(nullptr),
            m_activeCursors(alloc), m_alloc(alloc), m_parser(nullptr), m_mutations(mutNone), m_lastRememberedComment(-1)  {  }
        ~ParseNodeTree();

        void Initialize(Parser* parser, ParseNodePtr tree, AuthoringFileText* text, ArenaAllocator *alloc);

        bool IsMutated() const { return m_mutations != mutNone; }
        bool IsMutated(MutationKind mutation) const { return (m_mutations & mutation) != 0; }
        bool IsByteCodeGenerated() const { return m_treeWasGenerated; }
        bool IsValidFor(Js::ScriptContext* scriptContext) const
        { 
            return (m_nodeTree && !IsMutated() && !IsByteCodeGenerated()) && m_parser && scriptContext == m_parser->GetScriptContext();  
        }
        bool IsValidFor(AuthoringFileText* text) const
        {
            return m_text == text;
        }
        bool HasActiveCursors() const { return (m_activeCursors.Count() > 0); }
        ParseNodePtr TreeRoot () const { return m_nodeTree; }
        LanguageServiceExtension*  LanguageServiceExtension() const { return m_parserLSExtension; }
        Parser* GetParser() const { return m_parser; } 
        MutationKind Mutations() const { return m_mutations; }

        void ReleaseParser();
        void ForgetTree();
        void SetMutated(MutationKind mutation);
        void SetByteCodeGenerated();
        void SetCleaned();

        void AddCursor(AuthorParseNodeCursor* pCursor);
        void RemoveCursor (AuthorParseNodeCursor* pCursor);

        ParseNode* FindParseNode(ArenaAllocator* alloc, charcount_t min, OpCode nop);

        bool CollectReferences(charcount_t offset, ReferenceList *list, IdentPtr &symbol);

        bool ContainsName(Js::InternalString *name);
    private:
        template<typename TAccept>
        ParseNode* FindParseNode(ArenaAllocator* alloc, charcount_t pos, TAccept accept)
        {
            Assert(alloc);
            if (!m_nodeTree) 
                return nullptr;
            ParseNodeCursor cursor(alloc, this);
            auto node = cursor.SeekToOffset(pos);
            if(!node)
                return nullptr;
            return cursor.Up(accept);
        }
    };
}