// Copyright (C) Microsoft. All rights reserved. 
#pragma once

namespace UnifiedRegex
{
    template <typename C>
    class StandardChars {};

    class ASCIIChars : public Chars<char>
    {
    private:
        enum CharClass : uint8
        {
            Word       = 1 << 0,
            Newline    = 1 << 1,
            Whitespace = 1 << 2,
            Letter     = 1 << 3,
            Digit      = 1 << 4,
            Octal      = 1 << 5,
            Hex        = 1 << 6
        };
        static const uint8 classes[NumChars];
        static const uint8 values[NumChars];

    public:
        __inline static bool IsWord(Char c)
        {
            return (classes[CTU(c)] & Word) != 0;
        }

        __inline static bool IsNewline(Char c)
        {
            return (classes[CTU(c)] & Newline) != 0;
        }

        __inline static bool IsWhitespace(Char c)
        {
            return (classes[CTU(c)] & Whitespace) != 0;
        }

        __inline static bool IsLetter(Char c)
        {
            return (classes[CTU(c)] & Letter) != 0;
        }

        __inline static bool IsDigit(Char c)
        {
            return (classes[CTU(c)] & Digit) != 0;
        }

        __inline static bool IsOctal(Char c)
        {
            return (classes[CTU(c)] & Octal) != 0;
        }

        __inline static bool IsHex(Char c)
        {
            return (classes[CTU(c)] & Hex) != 0;
        }

        __inline static uint DigitValue(Char c)
        {
            return values[CTU(c)];
        }
    };

    template <>
    class StandardChars<char> : ASCIIChars
    {
        static const int numDigitPairs;
        static const Char* const digitStr;
        static const int numWhitespacePairs;
        static const Char* const whitespaceStr;
        static const int numWordPairs;
        static const Char* const wordStr;
        static const int numNewlinePairs;
        static const Char* const newlineStr;
        ArenaAllocator* allocator;
        Char toEquivs[NumChars][CaseInsensitive::EquivClassSize];
        CharSet<Char>* fullSet;
        CharSet<Char>* emptySet;
        CharSet<Char>* wordSet;
        CharSet<Char>* nonWordSet;
        CharSet<Char>* newlineSet;
        CharSet<Char>* whitespaceSet;

    public:
        StandardChars(ArenaAllocator* allocator);

        void SetDigits(ArenaAllocator* setAllocator, CharSet<Char> &set);
        void SetNonDigits(ArenaAllocator* setAllocator, CharSet<Char> &set);
        void SetWhitespace(ArenaAllocator* setAllocator, CharSet<Char> &set);
        void SetNonWhitespace(ArenaAllocator* setAllocator, CharSet<Char> &set);
        void SetWordChars(ArenaAllocator* setAllocator, CharSet<Char> &set);
        void SetNonWordChars(ArenaAllocator* setAllocator, CharSet<Char> &set);
        void SetNewline(ArenaAllocator* setAllocator, CharSet<Char> &set);
        void SetNonNewline(ArenaAllocator* setAllocator, CharSet<Char> &set);

        CharSet<Char>* GetFullSet();
        CharSet<Char>* GetEmptySet();
        CharSet<Char>* GetWordSet();
        CharSet<Char>* GetNonWordSet();
        CharSet<Char>* GetNewlineSet();
        CharSet<Char>* GetWhitespaceSet();
        CharSet<Char>* GetSurrogateUpperRange() { AssertMsg(false, "Not implemented"); return nullptr; }

        inline Char ToCanonical(Char c) const
        {
            return toEquivs[CTU(c)][0];
        }

        inline bool ToEquivs(Char c, __out_ecount(3) Char* equivs) const
        {
            bool nonTrivial = false;
            for (int i = 0; i < CaseInsensitive::EquivClassSize; i++)
            {
                equivs[i] = toEquivs[CTU(c)][i];
                if (equivs[i] != c)
                    nonTrivial = true;
            }
            return nonTrivial;
        }

        inline bool IsTrivialString(const Char* str, CharCount strLen) const
        {
            for (CharCount i = 0; i < strLen; i++)
            {
                Char c = str[i];
                for (int j = 0; j < CaseInsensitive::EquivClassSize; j++)
                {
                    if (toEquivs[CTU(c)][j] != c)
                        return false;
                }
            }
            return true;
        }
    };

    template <>
    class StandardChars<uint8> : Chars<uint8>
    {
    public:
        inline StandardChars(ArenaAllocator* allocator) {}

        __inline bool IsWord(Char c) const
        {
            return ASCIIChars::IsWord(ASCIIChars::UTC(CTU(c)));
        }

        __inline bool IsNewline(Char c) const
        {
            return ASCIIChars::IsNewline(ASCIIChars::UTC(CTU(c)));
        }

        __inline bool IsWhitespaceOrNewline(Char c) const
        {
            return ASCIIChars::IsWhitespace(ASCIIChars::UTC(CTU(c)));
        }

        __inline bool IsLetter(Char c) const
        {
            return ASCIIChars::IsLetter(ASCIIChars::UTC(CTU(c)));
        }

        __inline bool IsDigit(Char c) const
        {
            return ASCIIChars::IsDigit(ASCIIChars::UTC(CTU(c)));
        }

        __inline bool IsOctal(Char c) const
        {
            return ASCIIChars::IsOctal(ASCIIChars::UTC(CTU(c)));
        }

        __inline bool IsHex(Char c) const
        {
            return ASCIIChars::IsHex(ASCIIChars::UTC(CTU(c)));
        }

        __inline uint DigitValue(Char c) const
        {
            return ASCIIChars::DigitValue(ASCIIChars::UTC(CTU(c)));
        }
    };

    template <>
    class StandardChars<wchar_t> : public Chars<wchar_t>
    {
    private:
        static const int numDigitPairs;
        static const Char* const digitStr;
        static const int numWhitespacePairs;
        static const Char* const whitespaceStr;
        static const int numWordPairs;
        static const Char* const wordStr;
        static const int numNewlinePairs;
        static const Char* const newlineStr;

        ArenaAllocator* allocator;

        // Map character to:
        //  - -1 if trivial equivalence class
        //  - otherwise to four 16-bit fields: <0><equiv 3><equiv 2><equiv 1>
        CharMap<Char, uint64> toEquivs;

        CharSet<Char>* fullSet;
        CharSet<Char>* emptySet;
        CharSet<Char>* wordSet;
        CharSet<Char>* nonWordSet;
        CharSet<Char>* newlineSet;
        CharSet<Char>* whitespaceSet;
        CharSet<Char>* surrogateUpperRange;

    public:
        StandardChars(ArenaAllocator* allocator);

        __inline bool IsWord(Char c) const
        {
            return CTU(c) < ASCIIChars::NumChars && ASCIIChars::IsWord(ASCIIChars::UTC(CTU(c)));
        }

        __inline bool IsNewline(Char c) const
        {
            return CTU(c) < ASCIIChars::NumChars ? ASCIIChars::IsNewline(ASCIIChars::UTC(CTU(c))) : (CTU(c) & 0xfffe) == 0x2028;
        }

        __inline bool IsWhitespaceOrNewline(Char c) const
        {
            if (CTU(c) < ASCIIChars::NumChars)
                return ASCIIChars::IsWhitespace(ASCIIChars::UTC(CTU(c)));
            else
                return CTU(c) == 0x1680 || CTU(c) == 0x180e || (CTU(c) >= 0x2000 && CTU(c) <= 0x200a) ||
                       CTU(c) == 0x2028 || CTU(c) == 0x2029 || CTU(c) == 0x202f || CTU(c) == 0x205f ||
                       CTU(c) == 0x3000 || CTU(c) == 0xfeff;
        }

        __inline bool IsLetter(Char c) const
        {
            return CTU(c) < ASCIIChars::NumChars && ASCIIChars::IsLetter(ASCIIChars::UTC(CTU(c)));
        }

        __inline bool IsDigit(Char c) const
        {
            return CTU(c) < ASCIIChars::NumChars && ASCIIChars::IsDigit(ASCIIChars::UTC(CTU(c)));
        }

        __inline bool IsOctal(Char c) const
        {
            return CTU(c) < ASCIIChars::NumChars && ASCIIChars::IsOctal(ASCIIChars::UTC(CTU(c)));
        }

        __inline bool IsHex(Char c) const
        {
            return CTU(c) < ASCIIChars::NumChars && ASCIIChars::IsHex(ASCIIChars::UTC(CTU(c)));
        }

        __inline uint DigitValue(Char c) const
        {
            return CTU(c) < ASCIIChars::NumChars ? ASCIIChars::DigitValue(ASCIIChars::UTC(CTU(c))) : 0;
        }

        void SetDigits(ArenaAllocator* setAllocator, CharSet<Char> &set);
        void SetNonDigits(ArenaAllocator* setAllocator, CharSet<Char> &set);
        void SetWhitespace(ArenaAllocator* setAllocator, CharSet<Char> &set);
        void SetNonWhitespace(ArenaAllocator* setAllocator, CharSet<Char> &set);
        void SetWordChars(ArenaAllocator* setAllocator, CharSet<Char> &set);
        void SetNonWordChars(ArenaAllocator* setAllocator, CharSet<Char> &set);
        void SetNewline(ArenaAllocator* setAllocator, CharSet<Char> &set);
        void SetNonNewline(ArenaAllocator* setAllocator, CharSet<Char> &set);

        CharSet<Char>* GetFullSet();
        CharSet<Char>* GetEmptySet();
        CharSet<Char>* GetWordSet();
        CharSet<Char>* GetNonWordSet();
        CharSet<Char>* GetNewlineSet();
        CharSet<Char>* GetWhitespaceSet();
        CharSet<Char>* GetSurrogateUpperRange();

        inline Char ToCanonical(Char c) const
        {
            uint64 r = toEquivs.Get(c);
            return r == (uint64)-1 ? c : UTC(r & 0xffff);
        }

        inline bool ToEquivs(Char c, __out_ecount(3) Char* equivs) const
        {
            uint64 r = toEquivs.Get(c);
            if (r == (uint64)-1)
            {
                for (int i = 0; i < CaseInsensitive::EquivClassSize; i++)
                    equivs[i] = c;
                return false;
            }
            else
            {
                for (int i = 0; i < CaseInsensitive::EquivClassSize; i++)
                {
                    equivs[i] = UTC(r & 0xffff);
                    r >>= 16;
                }
                return true;
            }
        }

        inline bool IsTrivialString(const Char* str, CharCount strLen) const
        {
            for (CharCount i = 0; i < strLen; i++)
            {
                if (toEquivs.Get(str[i]) != (uint64)-1)
                    return false;
            }
            return true;
        }
    };

    typedef UnifiedRegex::StandardChars<char> ASCIIStandardChars;
    typedef UnifiedRegex::StandardChars<uint8> UTF8StandardChars;
    typedef UnifiedRegex::StandardChars<wchar_t> UnicodeStandardChars;
}