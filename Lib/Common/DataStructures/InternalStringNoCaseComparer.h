//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace JsUtil
{
    template <typename T>
    class NoCaseComparer;

    template <>
    class NoCaseComparer<JsUtil::CharacterBuffer<WCHAR>>
    {
    public:
        static bool Equals(JsUtil::CharacterBuffer<WCHAR> const& x, JsUtil::CharacterBuffer<WCHAR> const& y);
        static uint GetHashCode(JsUtil::CharacterBuffer<WCHAR> const& i);
    private:
        static int Compare(JsUtil::CharacterBuffer<WCHAR> const& x, JsUtil::CharacterBuffer<WCHAR> const& y);
    };
      
}
