//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

class RemoteBaseDictionary
{
public:
    RemoteBaseDictionary(ExtRemoteTyped baseDictionary) : baseDictionary(baseDictionary) {}

    JDRemoteTyped GetBuckets();
    JDRemoteTyped GetEntries();

    template <typename Fn>
    bool ForEachValue(Fn fn)
    {
        return ForEachEntry([&](JDRemoteTyped& entry)
        {
            return fn(entry.Field("value"));
        });
    }

    template <typename Fn>
    bool ForEachEntry(Fn fn)
    {
        JDRemoteTyped buckets = GetBuckets();
        JDRemoteTyped entries = GetEntries();
        ULONG bucketCount = baseDictionary.Field("bucketCount").GetUlong();
        for (ULONG i = 0; i < bucketCount; i++)
        {
            LONG nextIndex = -1;
            for (LONG currentIndex = buckets[i].GetLong(); currentIndex != -1; currentIndex = nextIndex)
            {
                JDRemoteTyped entry = entries[currentIndex];
                nextIndex = entry.Field("next").GetLong();
                if (fn(entry))
                {
                    return true;
                }
            }
        }
        return false;
    }

    ExtRemoteTyped GetExtRemoteTyped() { return baseDictionary; }

private:
    JDRemoteTyped baseDictionary;
};
