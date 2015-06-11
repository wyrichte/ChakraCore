//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------
#pragma once

namespace UnifiedRegex
{
    enum RegexFlags : uint8
    {
        NoRegexFlags        = 0,
        IgnoreCaseRegexFlag = 1 << 0,
        GlobalRegexFlag     = 1 << 1,
        MultilineRegexFlag  = 1 << 2,
        UnicodeRegexFlag    = 1 << 3,
        StickyRegexFlag     = 1 << 4,
        AllRegexFlags       = (1 << 5) - 1
    };
}