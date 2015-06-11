s//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Authoring
{
    //
    //  String
    //

    inline LPWSTR String::CopyImpl(ArenaAllocator *alloc, LPCWSTR src, charcount_t len)
    {
        Assert(alloc != nullptr);
        Assert(src != nullptr);
    
        if(len == 0)
        {
            return L"";
        }

        auto result = String::Alloc(alloc, len + 1);
        js_wmemcpy_s(result, len, src, len);
        result[len] = L'\0';

        return result;
    }

    inline LPWSTR String::CopyImpl(ArenaAllocator *alloc, LPCWSTR src1, charcount_t len1, LPCWSTR src2, charcount_t len2)
    {
        Assert(alloc != nullptr);
        Assert(src1 != nullptr && src2 != nullptr);
        Assert(len1 > 0 && len2 > 0);

        auto result = String::Alloc(alloc, len1 + len2 + 1);
        js_wmemcpy_s(result, len1, src1, len1);
        js_wmemcpy_s(result+len1, len2, src2, len2);
        result[len1 + len2] = L'\0';

        return result;
    }

    LPWSTR String::Copy(ArenaAllocator* alloc, LPCWSTR src)
    {   
        if(src == nullptr)
        {
            return nullptr;
        }

        int len = wcslen(src);

        return CopyImpl(alloc, src, len);
    }

    LPWSTR String::Copy(ArenaAllocator* alloc, LPCWSTR src, charcount_t len)
    {
        if(src == nullptr)
        {
            return nullptr;
        }

        return CopyImpl(alloc, src, len);
    }

    LPWSTR String::Copy(ArenaAllocator* alloc, LPCWSTR src1, charcount_t len1, LPCWSTR src2, charcount_t len2)
    {
        if (src1 == nullptr || len1 == 0)
        {
            return Copy(alloc, src2, len2);
        }
        if (src2 == nullptr || len2 == 0)
        {
            return Copy(alloc, src1, len1);
        }

        return CopyImpl(alloc, src1, len1, src2, len2);
    }

    LPCWSTR String::Trim(ArenaAllocator* alloc, LPCWSTR string)
    {
        Assert (alloc != nullptr);

        if(string == nullptr)
        {
            return nullptr;
        }

        int length = 0;
        LPCWSTR begin = string;
        while (*begin && IsWhitespace(*begin))
        {
            begin ++;
        }
        
        LPCWSTR current = begin;
        int i = 0;
        while (*current)
        {
            i++;
            if (!IsWhitespace(*current))
                length = i;
            current ++;
        }

        if(length == 0)
        {
            return L"";
        }

        return Js::InternalString::New(alloc, begin, length)->GetBuffer();
    }

    bool String::IsNullOrEmpty(LPCWSTR s)
    {
        return (s == nullptr) || (s[0] == L'\0');
    }

    LPWSTR String::Alloc(ArenaAllocator *alloc, charcount_t len)
    {
        Assert(alloc != nullptr);
        return AnewArray(alloc, wchar_t, len);
    }

    charcount_t String::Length(LPCWSTR s)
    {
        return s ? wcslen(s) : 0;
    }

    BSTR String::ToBSTR(LPCWSTR s, charcount_t len)
    {
        return s ? ::SysAllocStringLen(s, len) : nullptr;
    }

    //
    //  UTF8String
    //

    LPUTF8 UTF8String::Copy(ArenaAllocator* alloc, LPCUTF8 value, size_t length)
    {
        auto result = AnewArray(alloc, utf8char_t, length + 1);
        memcpy_s(result, length, value, length);
        result[length] = L'\0';
        return result;
    }

    LPUTF8 UTF8String::FromWStr(ArenaAllocator* alloc, Js::ScriptContext* scriptContext, LPCWSTR value, charcount_t length, __out size_t& outputLength, __out Js::Utf8SourceInfo** sourceInfo)
    {
        Assert(alloc);
        Assert(scriptContext);
        Assert(sourceInfo);

        // UTF8 characters can take variable length from 1 to 3 chars for each wchar. We need the buffer to be 3 times 
        // the input string length. Arena allocator will not return the unused memory. In order to save space, we will 
        // try to allocate only what we need.
        const size_t minInputBlockSize = 1200; // 1200 * 3 < 4k (page size) 
        const size_t minOutputBlockSize = minInputBlockSize * 3;

        // We will make an optimistic assumption that the input is all in ASCII (i.e. one UTF8 output will be same 
        // length as input).
        size_t allocatedSize = length + minOutputBlockSize + 1;
        if (length < minInputBlockSize)
        {
            // for small files, allocate the 3 times of the length
            allocatedSize = (length * 3) + 1; 
        }

        LPUTF8 outputBuffer = AnewArray(alloc, utf8char_t, allocatedSize);
        outputLength = 0;

        size_t currentInputLength = length;        
        do
        {
            size_t lengthToConvert = currentInputLength <= minInputBlockSize  ? currentInputLength : (currentInputLength / 3);

            if ((allocatedSize - outputLength) < (lengthToConvert * 3))
            {
                // extend the buffer, let the minimum be the blocksize to avoid calling realloc for every iteration
                size_t additionalAllocatedSize = max(minOutputBlockSize, (lengthToConvert * 3) - (allocatedSize - outputLength) + 1);
                outputBuffer = reinterpret_cast<LPUTF8>(alloc->Realloc(outputBuffer, allocatedSize, allocatedSize + additionalAllocatedSize));
                allocatedSize += additionalAllocatedSize;
            }

            // do the actual conversion
            Assert((allocatedSize - outputLength) >= (lengthToConvert * 3));
            size_t currentOutputLength = utf8::EncodeInto(outputBuffer + outputLength , value + (length - currentInputLength), lengthToConvert);

            currentInputLength -= lengthToConvert;
            outputLength += currentOutputLength;
        } while (currentInputLength > 0);

        Assert(outputLength < allocatedSize);

        // adjust to the actual size
        outputBuffer = reinterpret_cast<LPUTF8>(alloc->Realloc(outputBuffer, allocatedSize, outputLength + 1));
        if (sourceInfo != NULL)
        {
            Js::Utf8SourceInfo* si = Js::Utf8SourceInfo::NewWithNoCopy(scriptContext, outputBuffer, length, outputLength, scriptContext->GetModuleSrcInfo(kmodGlobal));
            (*sourceInfo) = si;
        }

        // add the null terminator
        outputBuffer[outputLength] = 0;

        return outputBuffer;
    }

    //
    // Js::InternalString helpers
    //

    Js::InternalString* AllocInternalString(ArenaAllocator* alloc, LPCWSTR value)
    {
        Assert(alloc != NULL);
        Assert(value != NULL);
        return Js::InternalString::New(alloc, value, ::wcslen(value));
    }

    Js::InternalString* AllocInternalString(ArenaAllocator* alloc, IdentPtr value, bool noCopy)
    {
        Assert(alloc != NULL);
        Assert(value != NULL);
        auto sz = value->Psz();
        auto len = value->Cch();
        return noCopy ? Js::InternalString::NewNoCopy(alloc, sz, len) : Js::InternalString::New(alloc, sz, len);
    }

    //
    //  BSTR helpers
    //

    BSTR AllocBSTR(Js::InternalString * s)
    {
        return s ? String::ToBSTR(s->GetBuffer(), s->GetLength()) : nullptr;
    }

    BSTR AllocBSTR(Js::PropertyRecord const * s)
    {
        return s ? String::ToBSTR(s->GetBuffer(), s->GetLength()) : nullptr;
    }

    //
    //  TextBuffer
    //

    TextBuffer::TextBuffer(ArenaAllocator *alloc): m_alloc(alloc), m_buffer(NULL), m_current(0), m_size(0) { }

    TextBuffer::~TextBuffer()
    {
        if(m_buffer)
        {
            Adelete(m_alloc, m_buffer);
            m_buffer = nullptr;
        }
    }

    void TextBuffer::Release()
    {
        Adelete(m_alloc, this);
    }

    void TextBuffer::Ensure(size_t length)
    {
        if (m_current + length >= m_size) 
            Grow(length);
    }

    TextBuffer* TextBuffer::New(ArenaAllocator *alloc)
    {
        return Anew(alloc, TextBuffer, alloc);
    }

    void TextBuffer::Add(wchar_t const *text, charcount_t length)
    {
        Ensure(length);
        memcpy_s(&m_buffer[m_current], sizeof(wchar_t) * length, text, sizeof(wchar_t) * length);
        m_current += length;
    }

    void TextBuffer::Add(LPCWSTR text)
    {
        Add(text, ::wcslen(text));
    }

    void TextBuffer::Add(wchar_t ch)
    {
        Ensure(sizeof(ch));
        m_buffer[m_current++] = ch;
    }

    void TextBuffer::Add(IdentPtr ident)
    {
        Add(ident->Psz(), ident->Cch());
    }

    void TextBuffer::Add(Js::InternalString *string)
    {
        Add(string->GetBuffer(), string->GetLength());
    }

    void TextBuffer::Add4Hex(unsigned long value)
    {
        swprintf_s(scratch, sizeof(scratch)/sizeof(wchar_t), L"%04x", value);
        Add(scratch);
    }

    void TextBuffer::AddHex(unsigned long value)
    {
        swprintf_s(scratch, sizeof(scratch)/sizeof(wchar_t), L"%x", value);
        Add(scratch);
    }

    void TextBuffer::AddInt(long value)
    {
        swprintf_s(scratch, sizeof(scratch)/sizeof(wchar_t), L"%ld", value);
        Add(scratch);
    }

    void TextBuffer::AddDouble(double value)
    {
        _snwprintf_s(scratch, sizeof(scratch)/sizeof(wchar_t), L"%g", value);
        Add(scratch);
    }

    // Translates code-points from text *NOT* the number of bytes. That is, if text contains 
    // a multi-byte sequence more than codePointCount bytes will be read from text but
    // only codePointCount words will be written to the buffer.
    void TextBuffer::AddUtf8(LPCUTF8 text, charcount_t codePointCount)
    {
        Ensure(codePointCount);
        utf8::DecodeInto(&m_buffer[m_current], text, codePointCount, utf8::doAllowThreeByteSurrogates);
        m_current += codePointCount;
    }

    void TextBuffer::Clear() { m_current = 0; }

    size_t TextBuffer::Length() { return m_current; }

    // Returns the raw buffer, not zero terminated
    LPCWSTR TextBuffer::GetBuffer() { return m_buffer;	}

    BSTR TextBuffer::ToBSTR()
    {
        return ::SysAllocStringLen(m_buffer, m_current);
    }

    Js::InternalString *TextBuffer::ToInternalString(ArenaAllocator *alloc)
    {
        return Js::InternalString::New(alloc, GetBuffer(), Length());
    }

    LPCWSTR TextBuffer::Sz(bool detach)
    {
        if(m_buffer == nullptr)
        {
            return L"";
        }
        Add(L'\0');
        
        auto sz = GetBuffer();
        
        if(detach)
        {
            m_buffer = nullptr;
        }

        return sz;
    }

    const size_t MaxMemoryIncrement = 10 * 1024 * 1024;

    void TextBuffer::Grow(size_t length)
    {
        size_t newSize = m_size ? (m_size < MaxMemoryIncrement ? m_size * 2 : m_size + MaxMemoryIncrement) : 1024;
        while (m_current + length >= newSize)
        {
            newSize = newSize * 2;
        }
        Assert(newSize > m_size);
        Buffer newBuffer = reinterpret_cast<Buffer>(m_alloc->Realloc(m_buffer, m_size * sizeof(wchar_t), newSize * sizeof(wchar_t)));
        if (!newBuffer)
        {
            Js::Throw::OutOfMemory();
        }
        m_buffer = newBuffer;
        m_size = newSize;
    }
}
