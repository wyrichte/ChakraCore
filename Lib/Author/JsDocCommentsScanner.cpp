//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

#include "JsDocToken.h"
#include "JsDocCommentsScanner.h"
#include "JsDocParsingException.h"

namespace Authoring
{
    JsDocCommentsScanner::JsDocCommentsScanner(ArenaAllocator* allocator, Js::CharClassifier* classifier, const WCHAR* text) :
        m_text(text),
        m_current(text),
        m_tagDescriptionSpans(allocator),
        m_taglessDescriptionSpans(allocator),
        m_typeNameSpans(allocator),
        m_unscanStack(allocator),
        m_isParamOptional(false),
        m_hasValue(false),
        m_lineNumber(1),
        m_columnNumber(1),
        m_lastLineNumber(-1),
        m_lastColumnNumber(-1),
        m_allocator(allocator),
        m_classifier(classifier)
    {
    }

    JsDocToken JsDocCommentsScanner::ScanToken()
    {
        this->m_unscanStack.Push(UnscanItem(this->m_current, this->m_lineNumber, this->m_columnNumber));
        while (true)
        {
            WCHAR read = *(this->m_current);
            this->AdvanceCurrentPointer();
            if (this->IsWhitespace(read))
            {
                // skip to next iteration - whitespace are unimportant
            }
            else
            {
                switch (read)
                {
                case L'\0':
                    return JsDocToken::eof;
                case L'\r':
                    // TODO (andrewau) handle MacOS file which use '\r' as end line characters.
                    //                 The current code will not work if '\n' is not present
                    break;
                case L'\n':
                    return JsDocToken::endline;
                case L'{':
                    return JsDocToken::openbrace;
                case L'}':
                    return JsDocToken::closebrace;
                case L'[':
                    return JsDocToken::openbracket;
                case L']':
                    return JsDocToken::closebracket;
                case L'*':
                    return JsDocToken::star;
                case L'|':
                    return JsDocToken::pipe;
                case L'=':
                    return JsDocToken::equals;
                default:
                    this->RewindCurrentPointer(); /* So that the currently consumed character can be consumed again by other Scan() method that will be called by the error tolerant parser */
                    return JsDocToken::unknown;
                }
            }
        }
    }

    JsDocToken JsDocCommentsScanner::ScanTagOrDescription()
    {
        this->m_unscanStack.Push(UnscanItem(this->m_current, this->m_lineNumber, this->m_columnNumber));
        bool isEof;
        this->SkipWhitespaces(isEof);

        if (isEof)
        {
            return JsDocToken::eof;
        }

        if (*(this->m_current) == L'@')
        {
            
            JsDocToken token = JsDocToken::atunknown;

            JsDocCommentsScanner::Span tagSpan = this->SkipTagName(isEof);
            switch (*(tagSpan.m_start + 1))
            {
            case 'r':
                if (tagSpan.Equals(Names::jsdoc_returns_tag))
                {
                    token = JsDocToken::atreturns;
                }
                else if (tagSpan.Equals(Names::jsdoc_return_tag))
                {
                    token = JsDocToken::atreturns;
                }
                break;
            case 'p':
                if (tagSpan.Equals(Names::jsdoc_param_tag_tag))
                {
                    return JsDocToken::atparam;
                }
                else if (tagSpan.Equals(Names::jsdoc_param_tag))
                {
                    token = JsDocToken::atparam;
                }
                else if (tagSpan.Equals(Names::jsdoc_property_tag))
                {
                    token = JsDocToken::atproperty;
                }
                break;
            case 'd':
                if (tagSpan.Equals(Names::jsdoc_deprecated_tag))
                {
                    token = JsDocToken::atdeprecated;
                }
                else if (tagSpan.Equals(Names::jsdoc_description_tag))
                {
                    token = JsDocToken::atdescription;
                }
                break;
            case 't':
                if (tagSpan.Equals(Names::jsdoc_type_tag))
                {
                    token = JsDocToken::attype;
                }
                else if (tagSpan.Equals(Names::jsdoc_typedef_tag))
                {
                    token = JsDocToken::attypedef;
                }
                break;
            case 's':
                if (tagSpan.Equals(Names::jsdoc_summary_tag))
                {
                    token = JsDocToken::atsummary;
                }
                break;
            default:
                // Unknown tag - not supported yet. Let's log it and move on:
                if (PHASE_TRACE1(Js::JsDocParsingPhase))
                {
                    WCHAR* unsupportedTag = tagSpan.ToString(this->m_allocator);
                    OUTPUT_TRACE(Js::JsDocParsingPhase, L"Unsupported jsdoc tag: %s \n", unsupportedTag);
                }
            }
            return token;
        }
        else
        {
            bool isEof;
            this->SkipDescription(isEof);
            return JsDocToken::description;
        }
    }

    JsDocToken JsDocCommentsScanner::ScanStarTagOrDescription()
    {
        this->m_unscanStack.Push(UnscanItem(this->m_current, this->m_lineNumber, this->m_columnNumber));
        bool isEof;
        this->SkipWhitespaces(isEof);

        if (isEof)
        {
            return JsDocToken::eof;
        }

        if (*(this->m_current) == L'*')
        {
            this->AdvanceCurrentPointer();
            return JsDocToken::star;
        }
        else
        {
            return this->ScanTagOrDescription();
        }
    }

    JsDocToken JsDocCommentsScanner::ScanHyphenOrDescription()
    {
        this->m_unscanStack.Push(UnscanItem(this->m_current, this->m_lineNumber, this->m_columnNumber));
        bool isEof;
        this->SkipWhitespaces(isEof);

        if (isEof)
        {
            return JsDocToken::eof;
        }

        if (*(this->m_current) == L'-')
        {
            this->AdvanceCurrentPointer();
            return JsDocToken::hyphen;
        }
        else
        {
            bool isEof;
            this->SkipDescription(isEof);
            return JsDocToken::description;
        }
    }

    JsDocToken JsDocCommentsScanner::ScanOpenBraceOrDescription()
    {
        this->m_unscanStack.Push(UnscanItem(this->m_current, this->m_lineNumber, this->m_columnNumber));
        bool isEof;
        this->SkipWhitespaces(isEof);

        if (isEof)
        {
            return JsDocToken::eof;
        }

        if (*(this->m_current) == L'{')
        {
            this->AdvanceCurrentPointer();
            return JsDocToken::openbrace;
        }
        else
        {
            bool isEof;
            this->SkipDescription(isEof);
            return JsDocToken::description;
        }
    }

    JsDocToken JsDocCommentsScanner::ScanName()
    {
        this->m_unscanStack.Push(UnscanItem(this->m_current, this->m_lineNumber, this->m_columnNumber));
        bool isEof;
        this->SkipWhitespaces(isEof);

        if (isEof)
        {
            return JsDocToken::eof;
        }

        this->SkipName(isEof);
        return JsDocToken::name;
    }

    JsDocToken JsDocCommentsScanner::ScanOpenBracketOrName()
    {
        this->m_unscanStack.Push(UnscanItem(this->m_current, this->m_lineNumber, this->m_columnNumber));
        bool isEof;
        this->SkipWhitespaces(isEof);

        if (isEof)
        {
            return JsDocToken::eof;
        }

        if (*(this->m_current) == L'[')
        {
            this->AdvanceCurrentPointer();
            return JsDocToken::openbracket;
        }
        else
        {
            bool isEof;
            this->SkipName(isEof);
            return JsDocToken::name;
        }
    }

    JsDocToken JsDocCommentsScanner::ScanOpenBraceOpenBracketOrName()
    {
        this->m_unscanStack.Push(UnscanItem(this->m_current, this->m_lineNumber, this->m_columnNumber));
        bool isEof;
        this->SkipWhitespaces(isEof);

        if (isEof)
        {
            return JsDocToken::eof;
        }

        if (*(this->m_current) == L'{')
        {
            this->AdvanceCurrentPointer();
            return JsDocToken::openbrace;
        }
        else if (*(this->m_current) == L'[')
        {
            this->AdvanceCurrentPointer();
            return JsDocToken::openbracket;
        }
        else
        {
            bool isEof;
            this->SkipName(isEof);
            return JsDocToken::name;
        }
    }

    JsDocToken JsDocCommentsScanner::ScanTypeName(bool allowSpace)
    {
        this->m_unscanStack.Push(UnscanItem(this->m_current, this->m_lineNumber, this->m_columnNumber));
        bool isEof;
        this->SkipWhitespaces(isEof);

        if (isEof)
        {
            return JsDocToken::eof;
        }

        Span typeNameSpan = this->SkipTypeName(allowSpace, isEof);
        this->m_typeNameSpans.Add(typeNameSpan);
        return JsDocToken::type_name;
    }

    JsDocToken JsDocCommentsScanner::ScanDescription()
    {
        this->m_unscanStack.Push(UnscanItem(this->m_current, this->m_lineNumber, this->m_columnNumber));
        bool isEof;
        this->SkipWhitespaces(isEof);

        if (isEof)
        {
            return JsDocToken::eof;
        }

        this->SkipDescription(isEof);
        return JsDocToken::description;
    }

    JsDocToken JsDocCommentsScanner::ScanValue(bool allowSpace)
    {
        this->m_unscanStack.Push(UnscanItem(this->m_current, this->m_lineNumber, this->m_columnNumber));
        bool isEof;
        this->SkipWhitespaces(isEof);

        if (isEof)
        {
            return JsDocToken::eof;
        }

        this->SkipValue(allowSpace, isEof);

        this->m_hasValue = true;
        return JsDocToken::value;
    }

    void JsDocCommentsScanner::Unscan()
    {
        this->m_current = this->m_unscanStack.Peek().m_current;
        this->m_lineNumber = this->m_unscanStack.Peek().m_lineNumber;
        this->m_columnNumber = this->m_unscanStack.Peek().m_columnNumber;
        this->m_unscanStack.Pop();
    }

    void JsDocCommentsScanner::RemoveLastTypeName()
    {
        this->m_typeNameSpans.RemoveAtEnd();
    }

    int JsDocCommentsScanner::GetLineNumber() const
    {
        return this->m_lineNumber;
    }

    int JsDocCommentsScanner::GetColumnNumber() const
    {
        return this->m_columnNumber;
    }

    void JsDocCommentsScanner::AdvanceCurrentPointer()
    {
        this->m_lastLineNumber = this->m_lineNumber;
        this->m_lastColumnNumber = this->m_columnNumber;
        if (*this->m_current == '\n')
        {
            this->m_columnNumber = 1;
            this->m_lineNumber++;
        }
        else
        {
            this->m_columnNumber++;
        }

        this->m_current++;
    }

    void JsDocCommentsScanner::RewindCurrentPointer()
    {
        this->m_lineNumber = this->m_lastLineNumber;
        this->m_columnNumber = this->m_lastColumnNumber;
        this->m_current--;
    }

    template<class T>
    void JsDocCommentsScanner::SkipUntil(bool& isEof, T shouldStop)
    {
        isEof = false;
        while (true)
        {
            WCHAR read = *(this->m_current);
            this->AdvanceCurrentPointer();
            if (read == L'\0')
            {
                isEof = true;
                break;
            }
            if (shouldStop(this, read))
            {
                break;
            }
        }

        // After this loop, current points to the next character after one character after the character should stop, so rewind by 1
        this->RewindCurrentPointer();
    }

    void JsDocCommentsScanner::SkipDescription(bool& isEof)
    {
        this->m_descriptionStart = this->m_current;
        this->SkipUntil(isEof, [](JsDocCommentsScanner* thisPtr, WCHAR c) { return c == L'\r' || c == L'\n'; });
        this->m_descriptionEnd = this->m_current;
    }

    void JsDocCommentsScanner::SkipValue(bool allowSpace, bool& isEof)
    {
        this->m_valueStart = this->m_current;
        this->SkipUntil(isEof, [allowSpace](JsDocCommentsScanner* thisPtr, WCHAR c) { return (!allowSpace && thisPtr->IsWhitespace(c)) || c == L'\r' || c == L'\n' || c == L']'; });
        this->m_valueEnd = this->m_current;
    }

    void JsDocCommentsScanner::SkipName(bool& isEof)
    {
        this->m_nameStart = this->m_current;
        this->SkipUntil(isEof, [](JsDocCommentsScanner* thisPtr, WCHAR c) { return thisPtr->IsWhitespace(c) || c == L'\r' || c == L'\n' || c == L']' || c == '='; });
        this->m_nameEnd = this->m_current;
    }

    JsDocCommentsScanner::Span JsDocCommentsScanner::SkipTagName(bool& isEof)
    {
        const WCHAR* start = this->m_current;
        this->SkipUntil(isEof, [](JsDocCommentsScanner* thisPtr, WCHAR c) { return thisPtr->IsWhitespace(c) || c == L'\r' || c == L'\n'; });
        const WCHAR* end = this->m_current;
        return Span(start, end);
    }

    JsDocCommentsScanner::Span JsDocCommentsScanner::SkipTypeName(bool allowSpace, bool& isEof)
    {
        const WCHAR* typeNameStart = this->m_current;

        //
        // allowSpace, isWhiteSpace, stop search
        // -------------------------------------
        // T           T             F
        // T           F             F
        // F           T             T
        // F           F             F
        //
        // stopSearch = (!allowSpace && isWhiteSpace)
        //

        this->SkipUntil(isEof, [allowSpace](JsDocCommentsScanner* thisPtr, WCHAR c) { return (!allowSpace && thisPtr->IsWhitespace(c)) || c == L'}' || c == L'|' || c == L'\r' || c == L'\n'; });
        const WCHAR* typeNameEnd = this->m_current;

        // Ignore any special character prefix/suffix
        auto isSpecialCharacter = [](WCHAR c) { return c == '!' || c == '=' || c == '.' || c == ' ' || c == '?'; };

        const WCHAR* lastCharacter = m_current - 1;
        while (lastCharacter > typeNameStart && this->m_classifier->IsWhiteSpace(*lastCharacter))
        {
            lastCharacter--;
        }

        // At the end of this loop, lastCharacter points to the last non-whitespace character.
        this->m_isParamOptional = *m_current == L'}' && *lastCharacter == L'=';

        while (typeNameStart < typeNameEnd && isSpecialCharacter(*typeNameStart))
        {
            typeNameStart++;
        }
        while (typeNameEnd > typeNameStart && isSpecialCharacter(*(typeNameEnd-1)))
        {
            typeNameEnd--;
        }

        return Span(typeNameStart, typeNameEnd);
    }

    void JsDocCommentsScanner::SkipWhitespaces(bool& isEof)
    {
        this->SkipUntil(isEof, [](JsDocCommentsScanner* thisPtr, WCHAR c) { return !thisPtr->IsWhitespace(c); });
    }

    WCHAR* JsDocCommentsScanner::GetName() const
    {
        size_t nameLength = (this->m_nameEnd - this->m_nameStart);
        return String::Copy(this->m_allocator, this->m_nameStart, nameLength);
    }

    const WCHAR* JsDocCommentsScanner::GetTypeName() const
    {
        if (this->GetTypeNameCount() > 0)
        {
            return this->GetTypeName(0);
        }
        return nullptr;
    }

    int JsDocCommentsScanner::GetTypeNameCount() const
    {
        return this->m_typeNameSpans.Count();
    }

    const WCHAR* JsDocCommentsScanner::GetTypeName(int i) const
    {
        size_t typeNameLength = (this->m_typeNameSpans.Item(i).m_end - this->m_typeNameSpans.Item(i).m_start);
        WCHAR* rawTypeStart = String::Copy(this->m_allocator, this->m_typeNameSpans.Item(i).m_start, typeNameLength);
        if (typeNameLength == 0)
        {
            return L"Object";
        }
        else
        {
            return this->Trim(rawTypeStart, typeNameLength + 1 /* bufferLength = string length + 1*/);
        }
    }

    bool JsDocCommentsScanner::HasValue() const
    {
        return this->m_hasValue;
    }

    WCHAR* JsDocCommentsScanner::GetValue() const
    {
        size_t valueLength = (this->m_valueEnd - this->m_valueStart);
        return String::Copy(this->m_allocator, this->m_valueStart, valueLength);
    }

    WCHAR* JsDocCommentsScanner::GetDescription() const
    {
        size_t descriptionLength = (this->m_descriptionEnd - this->m_descriptionStart);
        return String::Copy(this->m_allocator, this->m_descriptionStart, descriptionLength);
    }

    bool JsDocCommentsScanner::IsWhitespace(WCHAR c)
    {
        CharTypes charType = this->m_classifier->GetCharType(c);
        return charType == CharTypes::_C_WSP;
    }

    void JsDocCommentsScanner::AddTaglessDescriptionLine()
    {
        Span descriptionSpan(this->m_descriptionStart, this->m_descriptionEnd);
        this->m_taglessDescriptionSpans.Add(descriptionSpan);
    }

    void JsDocCommentsScanner::AddTagDescriptionLine()
    {
        Span descriptionSpan(this->m_descriptionStart, this->m_descriptionEnd);
        this->m_tagDescriptionSpans.Add(descriptionSpan);
    }

    void JsDocCommentsScanner::ClearTypes()
    {
        this->m_typeNameSpans.Clear();
        this->m_isParamOptional = false;
        this->m_hasValue = false;
    }

    void JsDocCommentsScanner::ClearTagDescriptions()
    {
        this->m_tagDescriptionSpans.Clear();
    }

    void JsDocCommentsScanner::ClearTaglessDescriptions()
    {
        this->m_taglessDescriptionSpans.Clear();
    }

    WCHAR* JsDocCommentsScanner::GetTagDescription() const
    {
        return this->GetMultipleLineDescription(this->m_tagDescriptionSpans);
    }

    WCHAR* JsDocCommentsScanner::GetTaglessDescription() const
    {
        return this->GetMultipleLineDescription(this->m_taglessDescriptionSpans);
    }

    bool JsDocCommentsScanner::GetIsParamOptional() const
    {
        return this->m_isParamOptional;
    }

    void JsDocCommentsScanner::SetParamIsOptional()
    {
        this->m_isParamOptional = true;
    }

    WCHAR* JsDocCommentsScanner::GetMultipleLineDescription(const SpanList& descriptionSpans) const
    {
        unsigned int numberOfLines = 0;
        size_t length = 0;

        descriptionSpans.Map([&](int unusedIndex, Span element)
        {
            // Intentionally ignore overflow - a string that long that overflow my integer?
            numberOfLines++;
            length += (element.m_end - element.m_start);
        });

        if (numberOfLines == 0)
        {
            return L"";
        }
        
        WCHAR endLine[] = L"\r\n";
        int endLineLength = _countof(endLine) - 1; /* be careful with the null terminator! */

        // Between every line, I will insert '\r\n', so the count is total length + (numberOfLines - 1) * 2 + 1 (for the '\0')
        unsigned int bufferLength = length + (numberOfLines - 1) * endLineLength + 1;
        WCHAR* result = AnewArray(this->m_allocator, WCHAR, bufferLength);
        WCHAR* cursor = result;
        int copiedLines = 0;

        descriptionSpans.Map([&](int unusedIndex, Span element)
        {
            size_t elementLength = element.m_end - element.m_start;
            js_wmemcpy_s(cursor, elementLength, element.m_start, elementLength);
            copiedLines++;
            cursor += elementLength;
            if (copiedLines != numberOfLines) /* The line I just copied is not the last line */
            {
                js_wmemcpy_s(cursor, endLineLength, endLine, endLineLength);
                cursor += endLineLength;
            }
        });

        AssertMsg(cursor == result + bufferLength - 1, "At the end of the copying, cursor should point to one character after the last written character");

        return this->Trim(result, bufferLength);
    }

    WCHAR* JsDocCommentsScanner::Trim(WCHAR* start, int bufferSize) const
    {
        Assert(bufferSize > 0);
        WCHAR* end = start + bufferSize - 1;

        // Trim beginning white spaces
        while (start < end && this->m_classifier->IsWhiteSpace(*start))
        {
            start++;
        }

        // Trim ending white spaces
        end--;
        while (end > start && this->m_classifier->IsWhiteSpace(*end))
        {
            end--;
        }
        // At the end of this loop, cursor points to the last non-whitespace character.

        // Null terminating the string
        end++;
        *end = L'\0';

        return start;
    }

    JsDocCommentsScanner::Span::Span() : m_start(nullptr), m_end(nullptr)
    {
        // no-op
    }

    JsDocCommentsScanner::Span::Span(const WCHAR* start, const WCHAR* end) : m_start(start), m_end(end)
    {
        // no-op
    }

    JsDocCommentsScanner::UnscanItem::UnscanItem() : m_current(nullptr), m_lineNumber(-1), m_columnNumber(-1)
    {
        // no-op
    }

    JsDocCommentsScanner::UnscanItem::UnscanItem(const WCHAR* current, int lineNumber, int columnNumber) : m_current(current), m_lineNumber(lineNumber), m_columnNumber(columnNumber)
    {
        // no-op
    }
}
