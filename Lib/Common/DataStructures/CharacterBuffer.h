// Copyright (C) Microsoft. All rights reserved. 
#pragma once

namespace JsUtil 
{
    static const charcount_t MAX_FAST_HASH_LENGTH = 256;
    // A buffer of characters, may have embedded null.
    template <typename T>
    class CharacterBuffer
    {
    public:
        CharacterBuffer() : string(null), len((charcount_t)-1) {}
        CharacterBuffer(T const * string, charcount_t len) : string(string), len(len) {}

        bool operator==(CharacterBuffer const& other) const 
        {
            Assert(string != null);
            if (this->len != other.len)
            {
                return false;
            }        
            return this->string == other.string || StaticEquals(string, other.string, this->len);
        }

        operator hash_t() const 
        {
            Assert(string != null);
            return StaticGetHashCode(string, len);
        }

        int FastHash() const 
        {
            Assert(string != null);
            return InternalGetHashCode<true>(string, len);
        }

        CharacterBuffer& operator=(T const * s)
        {
            Assert(s == null);
            string = null;
            len = (charcount_t)-1;
            return *this;
        }

        static bool StaticEquals(__in_z T const * s1, __in_z T const* s2, __in charcount_t length);

        static int StaticGetHashCode(__in_z T const * s, __in charcount_t length)
        {
            return InternalGetHashCode<false>(s, length);
        }

        // If this function gets modified; please update the getHash function in the following location:
        // \inetcore\mshtml\types\fastDOMCompiler.pl
        // The hash generated there must be identical to this function.
        template <bool fastHash>
        static int InternalGetHashCode(__in_z T const * s, __in charcount_t length)
        {
            // TODO: This hash performs poorly on small strings, particularly in SunSpider's string-unpack-code test.
            // Consider finding a better hash function now that some type handlers hash by string instead of PropertyId.
            int hash = 0;
            charcount_t hashLength = length;
            if (fastHash)
            {
                hashLength = min(length, MAX_FAST_HASH_LENGTH);
            }
            for (charcount_t i = 0; i < hashLength; i++)
            {
                hash = _rotl(hash, 7);
                hash ^= s[i];
            }
            return hash;
        }

        T const * GetBuffer() const { return string; }
        charcount_t GetLength() const { return len; }
    private:
        T const * string;
        charcount_t len;
    };

    template<>
    inline bool 
    CharacterBuffer<WCHAR>::StaticEquals(__in_z WCHAR const * s1, __in_z WCHAR const * s2, __in charcount_t length) 
    { 
        return wmemcmp(s1, s2, length) == 0;
    }

    template<>
    inline bool 
    CharacterBuffer<unsigned char>::StaticEquals(__in_z unsigned char const * s1, __in_z unsigned char const *s2, __in charcount_t length)
    {
        return memcmp(s1, s2, length) == 0;
    }

  
};