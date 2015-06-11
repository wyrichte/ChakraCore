// Copyright (C) Microsoft. All rights reserved. 
#if ENABLE_REGEX_CONFIG_OPTIONS

#pragma once

namespace UnifiedRegex
{
    class DebugWriter : private Chars<wchar_t>
    {
    private:
        static const Char* const hex;
        static const int bufLen = 2048;
        Char buf[bufLen];
        int indent;
        bool nlPending;

    public:
        DebugWriter();
        void __cdecl Print(const Char *form, ...);
        void __cdecl PrintEOL(const Char *form, ...);
        void PrintEscapedString(const Char *str, CharCount len);
        void PrintQuotedString(const Char *str, CharCount len);
        void PrintEscapedChar(Char c);
        void PrintQuotedChar(Char c);
        void EOL();
        void Indent();
        void Unindent();
        void Flush();

    private:
        inline void CheckForNewline()
        {
            if (nlPending)
            {
                BeginLine();
                nlPending = false;
            }
        }

        void BeginLine();
    };
}
#endif