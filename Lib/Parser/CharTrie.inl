//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace UnifiedRegex
{
    __inline bool RuntimeCharTrie::Match
        ( const Char* const input
        , const CharCount inputLength
        , CharCount& inputOffset
#if ENABLE_REGEX_CONFIG_OPTIONS
        , RegexStats* stats
#endif
        ) const
    {
        const RuntimeCharTrie* curr = this;
        while (true)
        {
            if (curr->count == 0)
                return true;
            if (inputOffset >= inputLength)
                return false;
#if ENABLE_REGEX_CONFIG_OPTIONS
            if (stats != 0)
                stats->numCompares++;
#endif

#if 0
            int l = 0;
            int h = curr->count - 1;
            while (true)
            {
                if (l > h)
                    return false;
                int m = (l + h) / 2;
                if (curr->children[m].c == input[inputOffset])
                {
                    inputOffset++;
                    curr = &curr->children[m].node;
                    break;
                }
                else if (CTU(curr->children[m].c) < CTU(input[inputOffset]))
                    l = m + 1;
                else
                    h = m - 1;
            }
#else
            int i = 0;
            while (true)
            {
                if (curr->children[i].c == input[inputOffset])
                {
                    inputOffset++;
                    curr = &curr->children[i].node;
                    break;
                }
                else if (curr->children[i].c > input[inputOffset])
                    return false;
                else if (++i >= curr->count)
                    return false;
            }
#endif
        }
    }
}