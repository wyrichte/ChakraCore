//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    template <typename Key, bool zero>
    struct SameValueComparerCommon
    {
        static bool Equals(Key, Key) { static_assert(false, "Can only use SameValueComparer with Var as the key type"); }
        static hash_t GetHashCode(Key) { static_assert(false, "Can only use SameValueComparer with Var as the key type"); }
    };

    // When MSVC++ gets the C++11 template aliasing feature we can do this instead of the struct inheritance hack:
    //template <typename Key> using SameValueComparer = SameValueComparerCommon<Key, false>;
    //template <typename Key> using SameValueZeroComparer = SameValueComparerCommon<Key, true>;
    template <typename Key> struct SameValueComparer : public SameValueComparerCommon<Key, false> { }; 
    template <typename Key> struct SameValueZeroComparer : public SameValueComparerCommon<Key, true> { };

    template <bool zero>
    struct SameValueComparerCommon<Var, zero>
    {
        static bool Equals(Var x, Var y)
        {
            if (zero)
            {
                return JavascriptConversion::SameValueZero(x, y, nullptr);
            }
            else
            {
                return JavascriptConversion::SameValue(x, y, nullptr);
            }
        }

        static hash_t GetHashCode(Var i)
        {
            switch (JavascriptOperators::GetTypeId(i))
            {
            case TypeIds_Integer:
                return TaggedInt::ToInt32(i);

            case TypeIds_Int64Number:
            case TypeIds_UInt64Number:
                {
                    __int64 v = JavascriptInt64Number::FromVar(i)->GetValue();
                    return (uint)v ^ (uint)(v >> 32);
                }

            case TypeIds_Number:
                {
                    double v = JavascriptNumber::GetValue(i);
                    if (JavascriptNumber::IsNan(v))
                    {
                        return 0;
                    }
                    
                    if (zero)
                    {
                        // SameValueZero treats -0 and +0 the same, so normalize to get same hash code
                        if (JavascriptNumber::IsNegZero(v))
                        {
                            v = 0.0;
                        }
                    }
                    
                    return (uint)(__int64)v ^ (uint)((__int64)v >> 32);
                }

            case TypeIds_String:
                {
                    JavascriptString* v = JavascriptString::FromVar(i);
                    return JsUtil::CharacterBuffer<WCHAR>::StaticGetHashCode(v->GetString(), v->GetLength());
                }

            default:
                return RecyclerPointerComparer<Var>::GetHashCode(i);
            }
        }
    };
}