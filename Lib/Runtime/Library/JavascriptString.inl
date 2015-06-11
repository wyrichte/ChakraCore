//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    // static
    inline bool IsValidCharCount(size_t charCount)
    {
        return charCount <= JavascriptString::MaxCharLength;
    }

    inline JavascriptString::JavascriptString(StaticType * type)
        : RecyclableObject(type), m_charLength(0), m_pszValue(0)
    {
        Assert(type->GetTypeId() == TypeIds_String);
    }

    inline JavascriptString::JavascriptString(StaticType * type, charcount_t charLength, const wchar_t* szValue)
        : RecyclableObject(type), m_charLength(charLength), m_pszValue(szValue)
    {
        Assert(type->GetTypeId() == TypeIds_String);
        AssertMsg( IsValidCharCount(charLength), "String length is out of range" );
    }

    inline charcount_t JavascriptString::GetLength() const
    {
        return m_charLength;
    }

    inline int JavascriptString::GetLengthAsSignedInt() const
    {
        Assert( IsValidCharCount(m_charLength) );
        return static_cast<int>(m_charLength);
    }

    inline const wchar_t* JavascriptString::UnsafeGetBuffer() const
    {
        return m_pszValue;
    }

    inline void JavascriptString::SetLength(charcount_t newLength)
    {
        if( !IsValidCharCount(newLength) )
        {
            JavascriptExceptionOperators::ThrowOutOfMemory(this->GetScriptContext());
        }
        m_charLength = newLength;
    }

    inline void JavascriptString::SetBuffer(const wchar_t* buffer)
    {
        m_pszValue = buffer;
    }

    inline bool JavascriptString::IsValidIndexValue(charcount_t idx) const
    {
        return IsValidCharCount(idx) && idx < GetLength();
    }

    inline bool JavascriptString::Is(Var aValue)
    {
        return JavascriptOperators::GetTypeId(aValue) == TypeIds_String;
    }

    inline JavascriptString* JavascriptString::FromVar(Var aValue)
    {
        AssertMsg(Is(aValue), "Ensure var is actually a 'JavascriptString'");
        
        return static_cast<JavascriptString *>(RecyclableObject::FromVar(aValue));
    }

    inline charcount_t
    JavascriptString::GetBufferLength(const wchar_t * content)
    {
        size_t cchActual = wcslen(content);
        
#if defined(_M_X64_OR_ARM64)
        if (!IsValidCharCount(cchActual))
        {
            // Limit javascript string to 31-bit length
            Js::Throw::OutOfMemory();
        }
#else
        // There shouldn't be enought mamory to have UINT_MAX character.
        // INT_MAX is the upper bound for 32-bit;
        Assert( IsValidCharCount(cchActual) );
#endif
        return static_cast<charcount_t>(cchActual);
    }

    inline charcount_t
    JavascriptString::GetBufferLength(
        const wchar_t * content,                     // Value to examine
        int charLengthOrMinusOne)                    // Optional length, in characters
    {
        //
        // Determine the actual length, in characters, not including a terminating '\0':
        // - If a length was not specified (charLength < 0), search for a terminating '\0'.
        //

        charcount_t cchActual;
        if (charLengthOrMinusOne < 0)
        {
            AssertMsg( charLengthOrMinusOne == -1, "The only negative value allowed is -1" );
            cchActual = GetBufferLength(content);
        }
        else
        {
            cchActual = static_cast<charcount_t>(charLengthOrMinusOne);
        }
#ifdef CHECK_STRING
// removed this to accommodate much larger string constant in regex-dna.js
        if (cchActual > 64 * 1024)
        {
            //
            // String was probably not '\0' terminated:
            // - We need to validate that the string's contents always fit within 1 GB to avoid
            //   overflow checking on 32-bit when using 'int' for 'byte *' pointer operations.
            //

            Throw::OutOfMemory();  // TODO: determine argument error
        }
#endif
        return cchActual;
    }

    template< size_t N >
    inline Var JavascriptString::StringBracketHelper(Arguments args, ScriptContext *scriptContext, const wchar_t (&tag)[N])
    {
        CompileAssert( 0 < N && N <= JavascriptString::MaxCharLength );
        return StringBracketHelper(args, scriptContext, tag, static_cast<charcount_t>(N-1), null, 0);
    }

    template< size_t N1, size_t N2 >
    inline Var JavascriptString::StringBracketHelper(Arguments args, ScriptContext *scriptContext, const wchar_t (&tag)[N1], const wchar_t (&prop)[N2])
    {
        CompileAssert( 0 < N1 && N1 <= JavascriptString::MaxCharLength );
        CompileAssert( 0 < N2 && N2 <= JavascriptString::MaxCharLength );
        return StringBracketHelper(args, scriptContext, tag, static_cast<charcount_t>(N1-1), prop, static_cast<charcount_t>(N2-1));
    }
    
    inline BOOL JavascriptString::BufferEquals(LPCWSTR otherBuffer, charcount_t otherLength)
    {
        return otherLength == this->GetLength() &&
            JsUtil::CharacterBuffer<WCHAR>::StaticEquals(this->GetString(), otherBuffer, otherLength);
    }

    template <typename StringType>
    inline void JavascriptString::Copy(__out_ecount(bufLen) wchar_t *const buffer, const charcount_t bufLen)
    {
        Assert(buffer);

        charcount_t stringLen = this->GetLength();
        if (bufLen < stringLen)
        {
            Throw::InternalError();
        }

        if (IsFinalized())
        {
            CopyHelper(buffer, GetString(), stringLen);
            return;
        }

        // Copy everything except nested string trees into the buffer. Collect nested string trees and the buffer locations
        // where they need to be copied, and copy them at the end. This is done to avoid excessive recursion during the copy.
        StringCopyInfoStack nestedStringTreeCopyInfos(GetScriptContext());
        ((StringType *)this)->StringType::CopyVirtual(buffer, nestedStringTreeCopyInfos, 0);
        FinishCopy(buffer, nestedStringTreeCopyInfos);        
    }
} // namespace Js
