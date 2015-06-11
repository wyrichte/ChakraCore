//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

class UInt16Math
{
public:
    template< class Func >
    static uint16 Add(uint16 lhs, uint16 rhs, __inout Func& overflowFn)
    {
        uint16 result = lhs + rhs;

        // If the result is smaller than the LHS, then we overflowed
        if( result < lhs )
        {
            overflowFn();
        }

        return result;
    }

    template< class Func >
    static void Inc(uint16& lhs, __inout Func& overflowFn)
    {
        ++lhs;

        // If lhs becomes 0, then we overflowed
        if(!lhs)
        {
            overflowFn();
        }
    }

    // Convenience function which uses DefaultOverflowPolicy (throws OOM when overflow)
    static uint16 Add(uint16 lhs, uint16 rhs)
    {
        return Add(lhs, rhs, ::Math::DefaultOverflowPolicy);
    }

    // Convenience functions which return a bool indicating overflow
    static bool Add(uint16 lhs, uint16 rhs, __out uint16* result)
    {
        ::Math::RecordOverflowPolicy overflowGuard;
        *result = Add(lhs, rhs, overflowGuard);
        return overflowGuard.HasOverflowed();
    }

    // Convenience function which uses DefaultOverflowPolicy (throws OOM when overflow)
    static void Inc(uint16& lhs)
    {
        Inc(lhs, ::Math::DefaultOverflowPolicy);
    }
};
