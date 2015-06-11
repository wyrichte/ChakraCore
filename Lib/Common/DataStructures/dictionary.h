//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

#if PROFILE_DICTIONARY
#include "DictionaryStats.h"
#endif

class ThreadContext;

namespace JsUtil
{
    static const int primes[] = {
        3, 7, 11, 17, 23, 29, 37, 47, 59, 71, 89, 107, 131, 163, 197, 239, 293, 353, 431, 521, 631, 761, 919,
        1103, 1327, 1597, 1931, 2333, 2801, 3371, 4049, 4861, 5839, 7013, 8419, 10103, 12143, 14591,
        17519, 21023, 25229, 30293, 36353, 43627, 52361, 62851, 75431, 90523, 108631, 130363, 156437,
        187751, 225307, 270371, 324449, 389357, 467237, 560689, 672827, 807403, 968897, 1162687, 1395263,
        1674319, 2009191, 2411033, 2893249, 3471899, 4166287, 4999559, 5999471, 7199369
    };       

    // TODO (post-checkin): convert to BaseDictionary- easier now to have custom dictionary since this does compacting
    // and weak reference resolution
    template <class TKey, class TValue, class KeyComparer = DefaultComparer<const TKey*>, bool cleanOnInsert = true> class WeaklyReferencedKeyDictionary
    {
    public:
        typedef WeakRefDictionaryEntry<TKey, TValue> EntryType;
        typedef TKey KeyType;
        typedef TValue ValueType;
        typedef void (*EntryRemovalCallbackMethodType)(const EntryType& e, void* cookie);

        struct EntryRemovalCallback
        {
            EntryRemovalCallbackMethodType fnCallback;
            void* cookie;
        };


    private:
        int size;
        int* buckets;
        EntryType * entries;
        int count;
        int version;
        int freeList;
        int freeCount;
        Recycler* recycler;
        EntryRemovalCallback entryRemovalCallback;
        uint lastWeakReferenceCleanupId;
        bool disableCleanup;

    public:
        // Allow WeaklyReferencedKeyDictionary field to be inlined in classes with DEFINE_VTABLE_CTOR_MEMBER_INIT
        WeaklyReferencedKeyDictionary(VirtualTableInfoCtorEnum) { }

        WeaklyReferencedKeyDictionary(Recycler* recycler, int capacity = 0, EntryRemovalCallback* pEntryRemovalCallback = NULL):
            buckets(NULL),
            size(0),
            entries(NULL),
            count(0),
            version(0),
            freeList(0),
            freeCount(0),
            recycler(recycler),
            lastWeakReferenceCleanupId(recycler->GetWeakReferenceCleanupId()),
            disableCleanup(false)
        {
            if (pEntryRemovalCallback != NULL)
            {
                this->entryRemovalCallback.fnCallback = pEntryRemovalCallback->fnCallback;
                this->entryRemovalCallback.cookie = pEntryRemovalCallback->cookie;
            }
            else
            {
                this->entryRemovalCallback.fnCallback = NULL;
            }

            if (capacity > 0) { Initialize(capacity); }
        }

        ~WeaklyReferencedKeyDictionary()
        {
        }

        inline int Count()
        {
            return count - freeCount;
        }

        TValue Item(TKey* key)
        {
            int i = FindEntry(key);
            if (i >= 0) return entries[i].value;
            Js::Throw::FatalInternalError();
        }

        void Item(TKey* key, const TValue value)
        {
            Insert(key, value, false);
        }

        const TValue& GetValueAt(const int& index) const
        {
            if (index >= 0 && index < count)
            {
                return entries[index].value;
            }
            Js::Throw::FatalInternalError();
        }

        bool TryGetValue(const TKey* key, TValue* value)
        {
            int i = FindEntry<TKey>(key);
            if (i >= 0)
            {
                *value = entries[i].value;
                return true;
            }
            return false;
        }

        bool TryGetValueAndRemove(const TKey* key, TValue* value)
        {
            if (buckets == NULL) return false;

            hash_t hash = GetHashCode(key);
            uint targetBucket = hash % size;
            int last = -1;
            int i = 0;

            if ((i = FindEntry<TKey>(key, hash, targetBucket, last)) != -1)
            {
                *value = entries[i].value;
                RemoveEntry(i, last, targetBucket);
                return true;
            }

            return false;
        }

        template <typename TLookup>
        inline TValue Lookup(const TLookup* key, TValue defaultValue, __out TKey const** pKeyOut)
        {
            int i = FindEntry(key);
            if (i >= 0)
            {
                (*pKeyOut) = entries[i].key->Get();
                return entries[i].value;
            }
            (*pKeyOut) = null;
            return defaultValue;
        }
        
        inline TValue Lookup(const TKey* key, TValue defaultValue)
        {
            int i = FindEntry(key);
            if (i >= 0)
            {
                return entries[i].value;
            }
            return defaultValue;
        }

        const RecyclerWeakReference<TKey>* Add(TKey* key, TValue value)
        {
            return Insert(key, value, true);
        }

        const RecyclerWeakReference<TKey>* UncheckedAdd(TKey* key, TValue value)
        {
            return Insert(key, value, true, false);
        }

        const RecyclerWeakReference<TKey>* UncheckedAdd(const RecyclerWeakReference<TKey>* weakRef, TValue value)
        {
            return UncheckedInsert(weakRef, value);
        }

        template<class Fn>
        void Map(Fn fn)
        {
            for(int i = 0; i < size; i++) 
            {
                if(buckets[i] != -1)
                {
                    for(int previousIndex = -1, currentIndex = buckets[i]; currentIndex != -1;)
                    {
                        EntryType &currentEntry = entries[currentIndex];
                        TKey * key = currentEntry.key->Get();
                        if(key != null)
                        {
                            fn(key, currentEntry.value, currentEntry.key);

                            // Keep the entry
                            previousIndex = currentIndex;
                            currentIndex = currentEntry.next;
                        }
                        else
                        {
                            // Remove the entry
                            const int nextIndex = currentEntry.next;
                            RemoveEntry(currentIndex, previousIndex, i);
                            currentIndex = nextIndex;
                        }
                    }
                }
            }
        }

        void SetDisableCleanup(bool disableCleanup)
        {
            this->disableCleanup = disableCleanup;
        }

        bool GetDisableCleanup()
        {
            return this->disableCleanup;
        }

        bool Clean()
        {
            if (!disableCleanup && recycler->GetWeakReferenceCleanupId() != this->lastWeakReferenceCleanupId)
            {
                Map([](TKey * key, TValue value, const RecyclerWeakReference<TKey>* weakRef) {});
                this->lastWeakReferenceCleanupId = recycler->GetWeakReferenceCleanupId();
            }

            return freeCount > 0;
        }

        void Clear()
        {
            if (count > 0)
            {
                for (int i = 0; i < size; i++) buckets[i] = -1;
                memset(entries, 0, sizeof(EntryType) * size);
                freeList = -1;
                count = 0;
                freeCount = 0;
            }
        }

        void EnsureCapacity()
        {
            if (freeCount == 0 && count == size)
            {
                if (cleanOnInsert && Clean())
                {
                    Assert(freeCount > 0);
                }
                else
                {
                    Resize();
                }
            }
        }

    private:
        const RecyclerWeakReference<TKey>* UncheckedInsert(const RecyclerWeakReference<TKey>* weakRef, TValue value)
        {
            if (buckets == NULL) Initialize(0);

            int hash = GetHashCode(weakRef->FastGet());
            uint bucket = (uint)hash % size;

            Assert(FindEntry(weakRef->FastGet()) == -1);
            return Insert(weakRef, value, hash, bucket);
        }

        const RecyclerWeakReference<TKey>* Insert(TKey* key, TValue value, bool add, bool checkForExisting = true)
        {
            if (buckets == NULL) Initialize(0);

            hash_t hash = GetHashCode(key);
            uint bucket = hash % size;

            if (checkForExisting)
            {
                int previous = -1;
                int i = FindEntry(key, hash, bucket, previous);

                if (i != -1)
                {
                    if (add)
                    {
                        Js::Throw::FatalInternalError();
                    }

                    entries[i].value = value;
                    version++;
                    return entries[i].key;
                }
            }

            // We know we need to insert- so first try creating the weak reference, before adding it to
            // the dictionary. If we OOM here, we still leave the dictionary as we found it.
            const RecyclerWeakReference<TKey>* weakRef = recycler->CreateWeakReferenceHandle<TKey>(key);

            return Insert(weakRef, value, hash, bucket);
        }

        const RecyclerWeakReference<TKey>* Insert(const RecyclerWeakReference<TKey>* weakRef, TValue value, int hash, uint bucket)
        {
            int index;
            if (freeCount > 0)
            {
                index = freeList;
                freeList = entries[index].next;
                freeCount--;
            }
            else
            {
                if (count == size)
                {
                    if (cleanOnInsert && Clean())
                    {
                        index = freeList;
                        freeList = entries[index].next;
                        freeCount--;
                    }
                    else
                    {
                        Resize();
                        bucket = (uint)hash % size;
                        index = count;
                        count++;
                    }
                }
                else
                {
                    index = count;
                    count++;
                }
            }

            entries[index].next = buckets[bucket];
            entries[index].key = weakRef;
            entries[index].hash = hash;
            entries[index].value = value;
            buckets[bucket] = index;
            version++;

            return entries[index].key;
        }
        
        void Resize()
        {
            int newSize = GetPrime(count * 2);

            // Ideally, this should be a real overflow check but this is causing a sunspider regression on two tests, so using
            // an assert for now. Also, OACR thinks that GetPrime can return a value less than the current size even when there
            // is no overflow.
            AssertMsg(newSize > count, "Failed to compute a larger size than the current size");
            __analysis_assume(newSize > count);
            
            int* newBuckets = RecyclerNewArrayLeaf(recycler, int, newSize);
            for (int i = 0; i < newSize; i++) newBuckets[i] = -1;
            EntryType* newEntries = RecyclerNewArray(recycler, EntryType, newSize);
            js_memcpy_s(newEntries, sizeof(EntryType) * newSize, entries, sizeof(EntryType) * count);
            for (int i = 0; i < count; i++)
            {
                uint bucket = (uint)newEntries[i].hash % newSize;
                newEntries[i].next = newBuckets[bucket];
                newBuckets[bucket] = i;
            }
            buckets = newBuckets;
            size = newSize;
            entries = newEntries;
        }

        template <typename TLookup>
        __inline hash_t GetHashCode(const TLookup* key)
        {
            return TAGHASH(KeyComparer::GetHashCode(key));
        }

        template <typename TLookup>
        inline int FindEntry(const TLookup* key)
        {
            if (buckets != null)
            {
                hash_t hash = GetHashCode(key);
                uint bucket = (uint)hash % size;
                int previous = -1;
                return FindEntry(key, hash, bucket, previous);
            }

            return -1;
        }

        template <typename TLookup>
        inline int FindEntry(const TLookup* key, hash_t const hash, uint& bucket, int& previous)
        {
            if (buckets != null)
            {
                BOOL inSweep = this->recycler->IsSweeping();
                previous = -1;
                for (int i = buckets[bucket]; i >= 0; )
                {
                    if (entries[i].hash == hash)
                    {
                        TKey* strongRef = null;

                        if (!inSweep)
                        {
                            // Quickly check for null if we're not in sweep- if it's null, it's definitely been collected
                            // so remove
                            strongRef = entries[i].key->FastGet();
                        }
                        else
                        {
                            // If we're in sweep, use the slower Get which checks if the object is getting collected
                            // This could return null too but we won't clean it up now, we'll clean it up later
                            strongRef = entries[i].key->Get();
                        }

                        if (strongRef == null)
                        {
                            i = RemoveEntry(i, previous, bucket);
                            continue;
                        }
                        else
                        {
                            // if we get here, strongRef is not null
                            if (KeyComparer::Equals(strongRef, key)) 
                                return i;
                        }
                    }

                    previous = i;
                    i = entries[i].next;
                }
            }
            return -1;
        }
        
        void Initialize(int capacity)
        {
            if (capacity <= 0) { capacity = 17; }
            int size = GetPrime(capacity);           

            int* buckets = RecyclerNewArrayLeaf(recycler, int, size);
            EntryType * entries = RecyclerNewArray(recycler, EntryType, size);

            // No need for auto pointers here since these are both recycler
            // allocated objects
            if (buckets != null && entries != null)
            {
                this->size = size;
                this->buckets = buckets;
                for (int i = 0; i < size; i++) buckets[i] = -1;
                this->entries = entries;
                this->freeList = -1;
            }
        }

        int RemoveEntry(int i, int previous, uint bucket)
        {
            int next = entries[i].next;

            if (previous < 0) // Previous < 0 => first node
            {
                buckets[bucket] = entries[i].next;
            }
            else
            {
                entries[previous].next = entries[i].next;
            }

            if (this->entryRemovalCallback.fnCallback != NULL)
            {
                this->entryRemovalCallback.fnCallback(entries[i], this->entryRemovalCallback.cookie);
            }

            entries[i].next = freeList;
            entries[i].key = NULL;
            entries[i].hash = EntryType::INVALID_HASH_VALUE;
            // Hold onto the pid here so that we can reuse it
            // entries[i].value = NULL;
            freeList = i;
            freeCount++;
            version++;

            return next;
        }

        static bool IsPrime(int candidate) 
        {
            if ((candidate & 1) != 0) 
            {
                int limit = (int)sqrt((FLOAT)candidate);
                for (int divisor = 3; divisor <= limit; divisor+=2)
                {
                    if ((candidate % divisor) == 0)
                        return false;
                }
                return true;
            }
            return (candidate == 2);
        }

        static int GetPrime(int min) 
        {
            if (min < 0)
                return min;

            for (int i = 0; i < sizeof(JsUtil::primes)/sizeof(int); i++) 
            {
                int prime = JsUtil::primes[i];
                if (prime >= min) return prime;
            }

            //outside of our predefined table. 
            //compute the hard way. 
            for (int i = (min | 1); i < 0x7FFFFFFF; i += 2) 
            {
                if (IsPrime(i))
                {
                    return i;
                }
            }
            return min;
        }
    };      
}
