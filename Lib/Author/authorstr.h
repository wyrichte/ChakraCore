//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// Description: String helpers

#pragma once

namespace Authoring
{
    inline bool IsWhitespace(wchar_t c) 
    {
        return (c == L' ' || c == L'\t');
    }

    Js::InternalString* AllocInternalString(ArenaAllocator* alloc, LPCWSTR value);
    Js::InternalString* AllocInternalString(ArenaAllocator* alloc, IdentPtr value, bool noCopy = false);
    BSTR AllocBSTR(Js::PropertyRecord const* s);
    BSTR AllocBSTR(Js::InternalString * s);
        
    template< size_t N >
    BOOL StringEqualsLiteral(Js::InternalString *s, const wchar_t (&w)[N])
    {
        return s->GetLength() == LengthOfLiteral(w) && 
            JsUtil::CharacterBuffer<WCHAR>::StaticEquals(s->GetBuffer(), w, (charcount_t)LengthOfLiteral(w));
    }

    
    //
    // Summary: Contains static helpers to manipulate zero terminated unicode strings (wchar_t*).
    // Notes:   Treats nullptr as a valid value which is considered an empty string. 
    // 
    class String
    {
    public:
        static LPWSTR Alloc(ArenaAllocator* alloc, charcount_t len);
        static LPWSTR Copy(ArenaAllocator* alloc, LPCWSTR src);
        static LPWSTR Copy(ArenaAllocator* alloc, LPCWSTR src, charcount_t len);
        static LPWSTR Copy(ArenaAllocator* alloc, LPCWSTR src1, charcount_t len1, LPCWSTR src2, charcount_t len2);
        static bool IsNullOrEmpty(LPCWSTR s);
        static LPCWSTR Trim(ArenaAllocator* alloc, LPCWSTR string);
        static charcount_t Length(LPCWSTR s);
        static BSTR ToBSTR(LPCWSTR s, charcount_t len);

    private:
        static inline LPWSTR CopyImpl(ArenaAllocator *alloc, LPCWSTR src, charcount_t len);
        static inline LPWSTR CopyImpl(ArenaAllocator *alloc, LPCWSTR src1, charcount_t len1, LPCWSTR src2, charcount_t len2);
    };

    //
    // Summary: Contains static helpers to manipulate utf8 encoded strings (utf8char_t*).
    // Notes:   Does not assume null terminator on input strings but always adds one to the returned strings.
    //
    class UTF8String
    {
    public:
        static LPUTF8 Copy(ArenaAllocator* alloc, LPCUTF8 value, size_t length);

        static LPUTF8 FromWStr(ArenaAllocator* alloc, Js::ScriptContext* scriptContext, LPCWSTR value, charcount_t length, __out size_t& outputLength, __out Js::Utf8SourceInfo** sourceInfo);
    };
    
    //
    // Summary: Represents a span of zero terminated unicode string.
    // Notes:  - The class doesn't own the string buffer
    //         - All the logic is delegated to String class
    class StringSpan
    {
        LPCWSTR     _s;
        charcount_t _len;
    public:
        StringSpan() 
        { 
            Set(nullptr, 0);
        }
        StringSpan(LPCWSTR s, charcount_t len)
        {
            Set(s, len);
        }
        void Set(LPCWSTR s)
        {
            Set(s, String::Length(s));
        }
        void Set(LPCWSTR s, charcount_t len)
        {
            if(!s)
                Assert(len == 0);
            _s   = s;
            _len = len;
        }
        LPCWSTR Buffer() { return _s; }
        charcount_t Length() { return _len; }
        BSTR ToBSTR() { return String::ToBSTR(_s, _len); }
    };

    class TextBuffer
    {
    private:
        typedef wchar_t *Buffer;
        Buffer m_buffer;
        size_t m_current;
        size_t m_size;
        ArenaAllocator *m_alloc;
        HRESULT m_hr;
        wchar_t scratch[100];
 
    private:
        void Ensure(size_t length);
        void Grow(size_t length);

    public:
        TextBuffer(ArenaAllocator *alloc);
        virtual ~TextBuffer();
        void Release();

        static TextBuffer *New(ArenaAllocator *alloc);
        void Add(wchar_t const *text, charcount_t length);
        void Add(LPCWSTR text);
        void Add(wchar_t ch);
        void Add(IdentPtr ident);
        void Add(Js::InternalString *string);
        void Add4Hex(unsigned long value);
        void AddHex(unsigned long value);
        void AddInt(long value);
        void AddDouble(double value);
        void AddUtf8(LPCUTF8 text, charcount_t codePointCount);
        void Clear();
        size_t Length();
        LPCWSTR GetBuffer();
        LPCWSTR Sz(bool detach = false);
        BSTR ToBSTR();
        Js::InternalString *ToInternalString(ArenaAllocator *alloc);
    };
}
