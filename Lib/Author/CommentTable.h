//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

namespace Authoring
{
    enum CommentType : unsigned char 
    { 
        commenttypeNone         = 0,
        commenttypeBlock        = 1, 
        commenttypeSingleLine   = 2, 
        commenttypeVSDoc        = 4, 
        commenttypeJSDoc        = 8,
        commenttypeAnyDoc       = 12,
        commenttypeAny          = 0xF 
    };
    
    struct CommentTableEntryFlags 
    { 
        CommentType commentType : 4;
        bool        adjacent    : 1;  // adjacent = true means it is adjacent with the previous entry.
    };

    inline bool CommentTypeMatches(CommentType commentType, CommentType filter) 
    { 
        return (filter == commenttypeAny) || ((commentType & filter) != 0);
    }

    const int BEFORE_ENTRIES = 2;

    struct CommentTableEntry
    {
        CommentTableEntryFlags  flags;
        charcount_t             ichMin;
        charcount_t             ichLim;

        CommentTableEntry() : ichMin(0), ichLim(0) 
        {
            flags.adjacent    = 0;
            flags.commentType = commenttypeSingleLine;
        }

        CommentTableEntry(charcount_t ichMin, charcount_t ichLim, CommentType commentType, bool adjacent)
            : ichMin(ichMin), ichLim(ichLim) 
        {
            flags.adjacent    = adjacent;
            flags.commentType = commentType;
        }

        bool MatchesType(CommentType commentType) const
        { 
            return CommentTypeMatches(flags.commentType, commentType);
        }

        bool InRange(charcount_t min, charcount_t lim)
        {
            return ichMin >= min && ichLim <= lim;
        }
    };

    class AuthoringFileText;
    class CommentBuffer;
    class CommentTable;

    class CommentTableIterator
    {
    public:
        CommentTableIterator(ArenaAllocator* allocator, const CommentTable* commentTable, bool groupAdjacentComments, bool typeDefOnly);
        CommentBuffer* GetCurrentComment(AuthoringFileText* text, /* out */ int* pMin, /* out */ int* pLim);
        void GetCurrentCommentSpan(/* out */ int* pMin, /* out */ int* pLim);
        bool HasNext();
        void MoveNext();
    private:
        ArenaAllocator* m_allocator;
        const CommentTable* m_commentTable;
        const bool m_groupAdjacentComments;
        const bool m_typeDefOnly;
        int m_index;
        int m_nextIndex;
    };

    __interface CommentTableAssociator
    {
    public:
        virtual void run(CommentTable* thisPtr, int commentTableIndex) = 0;
    };

    template<typename THandler>
    class CommentTableAssociatorImpl : public CommentTableAssociator
    {
    public:
        CommentTableAssociatorImpl(THandler handler) : m_handler(handler)
        {
        }

        virtual void run(CommentTable* thisPtr, int commentTableIndex)
        {
            this->m_handler(thisPtr, commentTableIndex);
        }
    private:
        THandler m_handler;
    };

    class CommentTable : public RefCounted<DeletePolicy::OperatorDelete> 
    {
        enum GetAdjacentCommentsOperation
        {
            GetAdjacentCommentsOperation_GetBuffer,
            GetAdjacentCommentsOperation_GetSpan,
        };

        friend class CommentTableIterator;

        static const size_t initialSize = 16;
        static const size_t growFactor = 4;
    private:
        typedef JsUtil::List<CommentTableEntry, ArenaAllocator> Entries;
        typedef JsUtil::List<int, ArenaAllocator> TypeDefIndices;

        typedef JsUtil::BaseDictionary<int, int, ArenaAllocator> NumberMap;

        ArenaAllocator  alloc;
        Entries         entries;

        //
        // CommentTable is responsible for storing the association between particular location in the source file and the 'associated' comment buffers.
        // External callers associates location with comment buffers, and we are responsible for handling two types of queries.
        // 
        // 1.) Given a location - find the comment buffer with max index, and
        // 2.) Given a comment buffer, find the minimum associated location.
        //
        // The first query is used to lookup the DocComment, scanning upwards for all adjacent comments, and start parsing
        // The second query is used to lookup the location, and count the number of endlines between that comment location and the min associated location
        // Suppose there is more than one endline, the comment is not considered a describing comment and therefore we disregard the association
        //
        NumberMap locationToMaxAssociatedCommentEntryMap;
        NumberMap commentEntryToMinAssociatedLocationMap;

        //
        // typeDefIndices is an ordered list of all the entries indice for comment containing type definitions.
        //
        // For example, if the 0th, 3th and 6th entries are comments containing type definitions, 
        // The typeDefIndices will be (0, 3, 6)
        //
        // JSDoc type def comments do NOT merge adjacent entries, so if 4 is adjancent with 3, 4 is NOT parsed together with 3
        //
        // However VSDoc type def comments DO merge adjacent entries, so if 7th and 8th entries is adjacent to 6th as VSDoc
        // 6th, 7th, 8th VSDoc will be merged together and parsed.
        // 
        // For VSDoc, the typeDefIndice entry will still be (0, 3, 6), even if the line that actually contains the tag <typedef> is on 7th entry.
        //
        TypeDefIndices  typeDefIndices;
        int             lastRememberedComment;
        CommentType     lastRememberedCommentType;
        charcount_t     prevCommentLine;
        CommentBuffer*  GetAdjacentComments(ArenaAllocator* alloc, AuthoringFileText *text, int currentIndex, int* pNextIndex) const;
        CommentBuffer*  GetAdjacentComments(ArenaAllocator* alloc, AuthoringFileText *text, int currentIndex, GetAdjacentCommentsOperation op, bool vsDocCorrection, 
            bool typeDefOnly, bool groupAdjacentComments, int lastIndex, /* out */ int* pMin, /* out */ int* pLim, /* out */ int* pNextIndex) const;
        int AnyCommentInRange(charcount_t min, charcount_t lim);
        template <typename TFindSpecific>
        int FindCommentInRange(charcount_t min, charcount_t lim, CommentType commentType, TFindSpecific FindSpecific);

        CommentTable(PageAllocator* pageAllocator) : alloc(L"ls:CommentTable", pageAllocator, Js::Throw::OutOfMemory), entries(&alloc), typeDefIndices(&alloc), locationToMaxAssociatedCommentEntryMap(&alloc), commentEntryToMinAssociatedLocationMap(&alloc), lastRememberedComment(-1), lastRememberedCommentType((CommentType)0), prevCommentLine(0) { }
        bool AssociateCommentBefore(charcount_t ichMin, charcount_t ichLim, CommentTableAssociator& handler);
    public:    

        CommentTableIterator* GetIterator(bool groupAdjacentComments);
        CommentTableIterator* GetTypeDefIterator();
    
        static CommentTable* Create(PageAllocator* pageAllocator) { return new CommentTable(pageAllocator); }

        int Count() const { return entries.Count(); }
        bool Empty() const { return entries.Count() == 0; }

        CommentTableEntry GetEntry(int index) const { return entries.Item(index); }

        void Clear();
        void AddComment(OLECHAR firstChar, OLECHAR secondChar, bool containTypeDef, charcount_t min, charcount_t lim, bool adjacent, bool multiline, charcount_t startLine, charcount_t endLine);
        int FirstCommentInRange(charcount_t min, charcount_t lim, CommentType commentType);
        int LastCommentInRange(charcount_t min, charcount_t lim, CommentType commentType);
        bool OffsetInComment(charcount_t offset);
        void Associate(int location, int commentTableEntryIndex);

        template<class THandler>
        bool AssociateCommentBefore(charcount_t ichMin, charcount_t ichLim, THandler handler)
        {
            CommentTableAssociatorImpl<THandler> wrapper(handler);
            return this->AssociateCommentBefore(ichMin, ichLim, (CommentTableAssociator&)wrapper);
        }

        CommentBuffer* GetLastAssociatedComment(ArenaAllocator* alloc, AuthoringFileText *text, CommentType commentType);
        CommentBuffer* GetCommentsBefore(ArenaAllocator* alloc, AuthoringFileText *text, charcount_t ichLocation, CommentType commentType);
        CommentBuffer* GetFirstCommentBetween(ArenaAllocator* alloc, AuthoringFileText *text, charcount_t ichLocation, charcount_t lim, CommentType commentType);

        static HRESULT Add(void *data, OLECHAR firstChar, OLECHAR secondChar, bool containTypeDef, charcount_t min, charcount_t lim, bool adjacent, bool multiline, charcount_t startLine, charcount_t endLine);
#if ENABLE_DEBUG_CONFIG_OPTIONS
        void DumpCommentTable(AuthoringFileText* text);
#endif
    };
}