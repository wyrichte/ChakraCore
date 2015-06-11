// Copyright (C) Microsoft. All rights reserved. 
namespace UnifiedRegex
{
    namespace CaseInsensitive
    {
        static const int EquivClassSize = 3;
        bool RangeToEquivClass(uint& tblidx, uint l, uint h, uint& acth, uint equivl[EquivClassSize]);
    }
}