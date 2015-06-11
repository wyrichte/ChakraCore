//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

static const uint primes[] = {
    3, 7, 11, 17, 23, 29, 37, 47, 59, 71, 89, 107, 131, 163, 197, 239, 293, 353, 431, 521, 631, 761, 919,
    1103, 1327, 1597, 1931, 2333, 2801, 3371, 4049, 4861, 5839, 7013, 8419, 10103, 12143, 14591,
    17519, 21023, 25229, 30293, 36353, 43627, 52361, 62851, 75431, 90523, 108631, 130363, 156437,
    187751, 225307, 270371, 324449, 389357, 467237, 560689, 672827, 807403, 968897, 1162687, 1395263,
    1674319, 2009191, 2411033, 2893249, 3471899, 4166287, 4999559, 5999471, 7199369
};

struct PrimePolicy
{
    __inline static uint GetBucket(hash_t hashCode, int size)
    {            
        uint targetBucket = hashCode % size;
        return targetBucket;
    }

    __inline static uint GetSize(uint capacity)
    {
        return GetPrime(capacity);
    }

private:
    static bool IsPrime(uint candidate) 
    {
        if ((candidate & 1) != 0) 
        {
            int limit = (uint)sqrt((FLOAT)candidate);
            for (int divisor = 3; divisor <= limit; divisor+=2)
            {
                if ((candidate % divisor) == 0)
                    return false;
            }
            return true;
        }
        return (candidate == 2);
    }

    static uint GetPrime(uint min) 
    {
        if (min <= 0)
            return 17;

        for (int i = 0; i < sizeof(primes)/sizeof(uint); i++) 
        {
            uint prime = primes[i];
            if (prime >= min) return prime;
        }

        //outside of our predefined table. 
        //compute the hard way. 
        for (uint i = (min | 1); i < 0x7FFFFFFF; i += 2) 
        {
            if (IsPrime(i))
            {
                return i;
            }
        }
        return min;
    }
};

struct PowerOf2Policy
{
    __inline static uint GetBucket(hash_t hashCode, int size)
    {            
        AssertMsg(Math::IsPow2(size), "Size is not a power of 2.");
        uint targetBucket = hashCode & (size-1);
        return targetBucket;
    }

    /// Returns a size that is power of 2 and
    /// greater than specified capacity.
    __inline static uint GetSize(uint minCapacity)
    {
        if(minCapacity <= 0)
        {
            return 4; 
        }

        if (Math::IsPow2(minCapacity))
        {
            return minCapacity;
        }
        else
        {
            return 1 << (Math::Log2(minCapacity) + 1);
        }
    }
};

#ifndef JD_PRIVATE
template <class SizePolicy, uint averageChainLength = 2, uint growthRateNumerator = 2, uint growthRateDenominator = 1, uint minBucket = 4>
struct DictionarySizePolicy
{
    CompileAssert(growthRateNumerator > growthRateDenominator);
    CompileAssert(growthRateDenominator != 0);
    __inline static uint GetBucket(hash_t hashCode, uint bucketCount)
    {            
        return SizePolicy::GetBucket(hashCode, bucketCount);
    }
    __inline static uint GetNextSize(uint minCapacity)
    {
        uint nextSize = minCapacity * growthRateNumerator / growthRateDenominator;
        return (growthRateDenominator != 1 && nextSize <= minCapacity)? minCapacity + 1 : nextSize;        
    }
    __inline static uint GetBucketSize(uint size)
    {
        if (minBucket * averageChainLength >= size)
        {
            return SizePolicy::GetSize(minBucket);
        }
        return SizePolicy::GetSize((size + (averageChainLength - 1)) / averageChainLength);
    }
};

typedef DictionarySizePolicy<PrimePolicy> PrimeSizePolicy;
typedef DictionarySizePolicy<PowerOf2Policy> PowerOf2SizePolicy;
#endif
