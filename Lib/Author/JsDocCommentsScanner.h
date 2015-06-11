//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#pragma once
#include "JsDocToken.h"

namespace Authoring
{
    //
    // JsDocCommentsScanner is a class used to scan a string of JsDocComments 
    // (with the Javascript comment delimiters removed by the JavaScript Parser)
    //
    // Unlike a traditional scanner, lexeme in the jsdoc language is not context free
    // Sometimes whitespace is needed as it is a part of the string literal, sometimes it 
    // isn't. To accomodate that, the scanner has multiple Scan* methods, each methods 
    // correspond to a different type of token that the parser would expect. It is 
    // expected that the parser call the right variant when it need a certain type of token
    //
    class JsDocCommentsScanner
    {
    public:
        JsDocCommentsScanner(ArenaAllocator* allocator, Js::CharClassifier* classifier, const WCHAR* text);
        JsDocToken ScanToken();
        JsDocToken ScanTagOrDescription();
        JsDocToken ScanStarTagOrDescription();
        JsDocToken ScanHyphenOrDescription();
        JsDocToken ScanOpenBraceOrDescription();
        JsDocToken ScanName();
        JsDocToken ScanOpenBracketOrName();
        JsDocToken ScanOpenBraceOpenBracketOrName();
        JsDocToken ScanTypeName(bool allowSpace);
        JsDocToken ScanDescription();
        JsDocToken ScanValue(bool allowSpace);

        int GetLineNumber() const;
        int GetColumnNumber() const;

        WCHAR* GetName() const;
        const WCHAR* GetTypeName() const;
        int GetTypeNameCount() const;
        const WCHAR* GetTypeName(int i) const;
        WCHAR* GetDescription() const;
        WCHAR* GetTagDescription() const;
        WCHAR* GetTaglessDescription() const;
        bool HasValue() const;
        WCHAR* GetValue() const;
        bool GetIsParamOptional() const;

        void AddTaglessDescriptionLine();
        void AddTagDescriptionLine();
        void ClearTypes();
        void ClearTagDescriptions();
        void ClearTaglessDescriptions();
        void SetParamIsOptional();

        void Unscan();
        void RemoveLastTypeName(); // TODO (andrewau) consider moving the storages instead
    private:
        //
        // start pointers are inclusive, end pointers are exclusive
        // For example, for the type name { Hello }
        // We have                          s    e
        // where s is the start pointer, and e is the end pointer
        //
        struct Span
        {
            // A default constructor is required by being an entry in List
            Span();
            Span(const WCHAR* start, const WCHAR* end);

            const WCHAR* m_start;
            const WCHAR* m_end;

            WCHAR* ToString(ArenaAllocator* alloc)
            {
                return String::Copy(alloc, m_start, m_end - m_start);
            }

            template<int N>
            bool Equals(_Null_terminated_ const WCHAR (&str)[N])
            {
                ptrdiff_t length = m_end - m_start;
                return wcsncmp(m_start, str, m_end - m_start) == 0 && (length == (N - 1));
            }
        };

        struct UnscanItem
        {
            // A default constructor is required by being an entry in List
            UnscanItem();
            UnscanItem(const WCHAR* current, int lineNumber, int columnNumber);

            const WCHAR* m_current;
            int m_lineNumber;
            int m_columnNumber;
        };

        typedef JsUtil::List<struct Span, ArenaAllocator> SpanList;
        typedef JsUtil::Stack<struct UnscanItem, ArenaAllocator> UnscanItemStack;

        inline void AdvanceCurrentPointer();
        inline void RewindCurrentPointer();
        void SkipDescription(bool& isEof);
        void SkipName(bool& isEof);
        JsDocCommentsScanner::Span SkipTypeName(bool allowSpace, bool& isEof);
        JsDocCommentsScanner::Span SkipTagName(bool& isEof);
        void SkipValue(bool allowSpace, bool& isEof);
        void SkipWhitespaces(bool& isEof);

        template<class T>
        void SkipUntil(bool& isEof, T shouldStop);

        WCHAR* GetMultipleLineDescription(const SpanList& descriptionSpans) const;
        WCHAR* Trim(_Inout_updates_(bufferSize) WCHAR* start, int bufferSize) const;
        bool IsWhitespace(WCHAR c);

        const WCHAR* const m_text;
        const WCHAR* m_current;

        const WCHAR* m_nameStart;
        const WCHAR* m_nameEnd;
        const WCHAR* m_descriptionStart;
        const WCHAR* m_descriptionEnd;
        const WCHAR* m_valueStart;
        const WCHAR* m_valueEnd;

        SpanList m_typeNameSpans;
        SpanList m_tagDescriptionSpans;
        SpanList m_taglessDescriptionSpans;

        UnscanItemStack m_unscanStack;

        bool m_isParamOptional;
        bool m_hasValue;

        int m_lineNumber;
        int m_columnNumber;
        int m_lastLineNumber;
        int m_lastColumnNumber;
        
        // resources for operation
        ArenaAllocator* m_allocator;
        Js::CharClassifier* m_classifier;
    };
}
