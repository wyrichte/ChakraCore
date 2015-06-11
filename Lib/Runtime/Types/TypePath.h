//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

#define MAX_SIZE_PATH_LENGTH (128)

namespace Js
{
    class TinyDictionary
    {
        static const int PowerOf2_BUCKETS = 8;
        static const byte NIL = 0xff;
        static const int NEXTPTRCOUNT = MAX_SIZE_PATH_LENGTH;

        byte buckets[PowerOf2_BUCKETS];
        byte next[NEXTPTRCOUNT];

public:
        TinyDictionary()
        {
            DWORD* init = (DWORD*)buckets;
            init[0] = init[1] = 0xffffffff;
        }

        void Add(PropertyId key, byte value)
        {
            Assert(value < NEXTPTRCOUNT);
            __analysis_assume(value < NEXTPTRCOUNT);

            uint32 bucketIndex = key&(PowerOf2_BUCKETS-1);

            byte i = buckets[bucketIndex];
            buckets[bucketIndex] = value;
            next[value] = i;
        }

        // Template shared with diagnostics
        template <class Data>
        __inline bool TryGetValue(PropertyId key, PropertyIndex* index, const Data& data)
        {
            uint32 bucketIndex = key&(PowerOf2_BUCKETS-1);

            for (byte i = buckets[bucketIndex] ; i != NIL ; i = next[i])
            {
                Assert(i < NEXTPTRCOUNT);
                __analysis_assume(i < NEXTPTRCOUNT);

                if (data[i]->GetPropertyId()== key)
                {
                    *index = i;
                    return true;
                }
                Assert(i != next[i]);
            }
            return false;
        }
    };

    class TypePath 
    {
        friend class PathTypeHandlerBase;
        friend class DynamicObject;
        friend class SimplePathTypeHandler;
        friend class PathTypeHandler;

    public:
        static const uint MaxPathTypeHandlerLength = MAX_SIZE_PATH_LENGTH;
        static const uint InitialTypePathSize = 16;

    private:
        TinyDictionary map;
        uint16 pathLength;      // Entries in use
        uint16 pathSize;        // Allocated entries

#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
        // We sometimes set up PathTypeHandlers and associate TypePaths before we create any instances
        // that populate the corresponding slots, e.g. for object literals or constructors with only
        // this statements.  This field keeps track of the longest instance associated with the given
        // TypePath.
        int maxInitializedLength;
        RecyclerWeakReference<DynamicObject>* singletonInstance;
        BVStatic<MAX_SIZE_PATH_LENGTH> fixedFields;
        BVStatic<MAX_SIZE_PATH_LENGTH> usedFixedFields;
#endif

        // PropertyRecord assignments are allocated off the end of the structure
        const PropertyRecord * assignments[0];

#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
        TypePath()
            : pathLength(0), maxInitializedLength(0), singletonInstance(null)
        {
        }
#else
        TypePath()
            : pathLength(0)
        {
        }
#endif

    public:
        static TypePath* New(Recycler* recycler, uint size = InitialTypePathSize);

        TypePath * Branch(Recycler * alloc, int pathLength, bool couldSeeProto);
        
        TypePath * Grow(Recycler * alloc);

        const PropertyRecord* GetPropertyIdUnchecked(int index)
        {
            Assert(((uint)index) < ((uint)pathLength));
            return assignments[index];
        }

        const PropertyRecord* GetPropertyId(int index)
        {
            if (((uint)index) < ((uint)pathLength))
                return GetPropertyIdUnchecked(index);
            else
                return NULL;
        }

        const PropertyRecord ** GetPropertyAssignments() 
        {
            return assignments;
        }

        int Add(const PropertyRecord * propertyRecord)
        {
#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
            Assert(this->pathLength == this->maxInitializedLength);
            this->maxInitializedLength++;
#endif
            return AddInternal(propertyRecord);
        }

        uint16 GetPathLength() { return this->pathLength; }
        uint16 GetPathSize() const { return this->pathSize; }
        
        PropertyIndex Lookup(PropertyId propId,int typePathLength);
        PropertyIndex LookupInline(PropertyId propId,int typePathLength);

    private:
        int AddInternal(const PropertyRecord* propId)
        {
            Assert(pathLength < this->pathSize);
            if (pathLength >= this->pathSize)
            {
                Throw::InternalError();
            }


            // The previous dictionary did not replace on dupes.
            // I believe a dupe here would be a bug, but to be conservative
            // replicate the exact previous behavior.
#if DBG
            PropertyIndex temp;
            if (map.TryGetValue(propId->GetPropertyId(), &temp, assignments))
            {
                AssertMsg(false, "Adding a duplicate to the type path");
            }
#endif 
            map.Add((unsigned int)propId->GetPropertyId(), (byte)pathLength);

#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
            if (PHASE_VERBOSE_TRACE1(FixMethodPropsPhase))
            {
                Output::Print(L"FixedFields: TypePath::AddInternal: singleton = 0x%p(0x%p)\n", 
                    this->singletonInstance, this->singletonInstance != null ? this->singletonInstance->Get() : null);
                Output::Print(L"   fixed fields:");

                for (PropertyIndex i = 0; i < GetPathLength(); i++)
                {
                    Output::Print(L" %s %d%d%d,", GetPropertyId(i)->GetBuffer(),
                        i < GetMaxInitializedLength() ? 1 : 0,
                        GetIsFixedFieldAt(i, GetPathLength()) ? 1 : 0,
                        GetIsUsedFixedFieldAt(i, GetPathLength()) ? 1 : 0);
                }
                
                Output::Print(L"\n");
            }
#endif

            assignments[pathLength]=propId;
            pathLength++;
            return (pathLength-1);
        }


#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
        int GetMaxInitializedLength() { return this->maxInitializedLength; }
        void SetMaxInitializedLength(int newMaxInitializedLength)
        {
            Assert(this->maxInitializedLength <= newMaxInitializedLength);
            this->maxInitializedLength = newMaxInitializedLength;
        }

        Var GetSingletonFixedFieldAt(PropertyIndex index, int typePathLength, ScriptContext * requestContext);

        bool HasSingletonInstance() const
        {
            return this->singletonInstance != null;
        }

        RecyclerWeakReference<DynamicObject>* GetSingletonInstance() const
        {
            return this->singletonInstance;
        }

        void SetSingletonInstance(RecyclerWeakReference<DynamicObject>* instance, int typePathLength)
        {
            Assert(this->singletonInstance == null && instance != null);
            Assert(typePathLength >= this->maxInitializedLength);
            this->singletonInstance = instance;
        }

        void ClearSingletonInstance()
        {
            this->singletonInstance = null;
        }

        void ClearSingletonInstanceIfSame(DynamicObject* instance)
        {
            if (this->singletonInstance != null && this->singletonInstance->Get() == instance)
            {
                ClearSingletonInstance();
            }
        }

        void ClearSingletonInstanceIfDifferent(DynamicObject* instance)
        {
            if (this->singletonInstance != null && this->singletonInstance->Get() != instance)
            {
                ClearSingletonInstance();
            }
        }

        bool GetIsFixedFieldAt(PropertyIndex index, int typePathLength)
        {
            Assert(index < this->pathLength);
            Assert(index < typePathLength);
            Assert(typePathLength <= this->pathLength);

            return this->fixedFields.Test(index) != 0;
        }

        bool GetIsUsedFixedFieldAt(PropertyIndex index, int typePathLength)
        {
            Assert(index < this->pathLength);
            Assert(index < typePathLength);
            Assert(typePathLength <= this->pathLength);

            return this->usedFixedFields.Test(index) != 0;
        }

        void SetIsUsedFixedFieldAt(PropertyIndex index, int typePathLength)
        {
            Assert(index < this->maxInitializedLength);
            Assert(CanHaveFixedFields(typePathLength));
            this->usedFixedFields.Set(index);
        }

        void ClearIsFixedFieldAt(PropertyIndex index, int typePathLength)
        {
            Assert(index < this->maxInitializedLength);
            Assert(index < typePathLength);
            Assert(typePathLength <= this->pathLength);

            this->fixedFields.Clear(index);
            this->usedFixedFields.Clear(index);
        }

        bool CanHaveFixedFields(int typePathLength)
        {
            // We only support fixed fields on singleton instances.
            // If the instance in question is a singleton, it must be the tip of the type path.
            return this->singletonInstance != null && typePathLength >= this->maxInitializedLength;
        }

        void AddBlankFieldAt(PropertyIndex index, int typePathLength)
        {
            Assert(index >= this->maxInitializedLength);
            this->maxInitializedLength = index + 1;

#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
            if (PHASE_VERBOSE_TRACE1(FixMethodPropsPhase))
            {
                Output::Print(L"FixedFields: TypePath::AddBlankFieldAt: singleton = 0x%p(0x%p)\n", 
                    this->singletonInstance, this->singletonInstance != null ? this->singletonInstance->Get() : null);
                Output::Print(L"   fixed fields:");

                for (PropertyIndex i = 0; i < GetPathLength(); i++)
                {
                    Output::Print(L" %s %d%d%d,", GetPropertyId(i)->GetBuffer(),
                        i < GetMaxInitializedLength() ? 1 : 0,
                        GetIsFixedFieldAt(i, GetPathLength()) ? 1 : 0,
                        GetIsUsedFixedFieldAt(i, GetPathLength()) ? 1 : 0);
                }
                
                Output::Print(L"\n");
            }
#endif
        }

        void AddSingletonInstanceFieldAt(DynamicObject* instance, PropertyIndex index, bool isFixed, int typePathLength)
        {
            Assert(index < this->pathLength);
            Assert(typePathLength >= this->maxInitializedLength);
            Assert(index >= this->maxInitializedLength);
            // This invariant is predicated on the properties getting initialized in the order of indexes in the type handler.
            Assert(instance != null);
            Assert(this->singletonInstance == null || this->singletonInstance->Get() == instance);
            Assert(!fixedFields.Test(index) && !usedFixedFields.Test(index));

            if (this->singletonInstance == null)
            {
                this->singletonInstance = instance->CreateWeakReferenceToSelf();
            }

            this->maxInitializedLength = index + 1;

            if (isFixed)
            {
                this->fixedFields.Set(index);
            }

#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
            if (PHASE_VERBOSE_TRACE1(FixMethodPropsPhase))
            {
                Output::Print(L"FixedFields: TypePath::AddSingletonInstanceFieldAt: singleton = 0x%p(0x%p)\n", 
                    this->singletonInstance, this->singletonInstance != null ? this->singletonInstance->Get() : null);
                Output::Print(L"   fixed fields:");

                for (PropertyIndex i = 0; i < GetPathLength(); i++)
                {
                    Output::Print(L" %s %d%d%d,", GetPropertyId(i)->GetBuffer(),
                        i < GetMaxInitializedLength() ? 1 : 0,
                        GetIsFixedFieldAt(i, GetPathLength()) ? 1 : 0,
                        GetIsUsedFixedFieldAt(i, GetPathLength()) ? 1 : 0);
                }
                
                Output::Print(L"\n");
            }
#endif
        }

        void AddSingletonInstanceFieldAt(PropertyIndex index, int typePathLength)
        {
            Assert(index < this->pathLength);
            Assert(typePathLength >= this->maxInitializedLength);
            Assert(index >= this->maxInitializedLength);
            Assert(!fixedFields.Test(index) && !usedFixedFields.Test(index));

            this->maxInitializedLength = index + 1;

#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
            if (PHASE_VERBOSE_TRACE1(FixMethodPropsPhase))
            {
                Output::Print(L"FixedFields: TypePath::AddSingletonInstanceFieldAt: singleton = 0x%p(0x%p)\n", 
                    this->singletonInstance, this->singletonInstance != null ? this->singletonInstance->Get() : null);
                Output::Print(L"   fixed fields:");

                for (PropertyIndex i = 0; i < GetPathLength(); i++)
                {
                    Output::Print(L" %s %d%d%d,", GetPropertyId(i)->GetBuffer(),
                        i < GetMaxInitializedLength() ? 1 : 0,
                        GetIsFixedFieldAt(i, GetPathLength()) ? 1 : 0,
                        GetIsUsedFixedFieldAt(i, GetPathLength()) ? 1 : 0);
                }
                
                Output::Print(L"\n");
            }
#endif
        }

#if DBG
        bool HasSingletonInstanceOnlyIfNeeded();
#endif

#else
        int GetMaxInitializedLength() { Assert(false); return this->pathLength; }

        Var GetSingletonFixedFieldAt(PropertyIndex index, int typePathLength, ScriptContext * requestContext);

        bool HasSingletonInstance() const { Assert(false); return false; }
        RecyclerWeakReference<DynamicObject>* GetSingletonInstance() const { Assert(false); return null; }
        void SetSingletonInstance(RecyclerWeakReference<DynamicObject>* instance, int typePathLength) { Assert(false); }
        void ClearSingletonInstance() { Assert(false); }
        void ClearSingletonInstanceIfSame(RecyclerWeakReference<DynamicObject>* instance) { Assert(false); }
        void ClearSingletonInstanceIfDifferent(RecyclerWeakReference<DynamicObject>* instance) { Assert(false); }

        bool GetIsFixedFieldAt(PropertyIndex index, int typePathLength) { Assert(false); return false; }
        bool GetIsUsedFixedFieldAt(PropertyIndex index, int typePathLength) { Assert(false); return false; }
        void SetIsUsedFixedFieldAt(PropertyIndex index, int typePathLength) { Assert(false); }
        void ClearIsFixedFieldAt(PropertyIndex index, int typePathLength) { Assert(false); }
        bool CanHaveFixedFields(int typePathLength) { Assert(false); return false; }
        void AddBlankFieldAt(PropertyIndex index, int typePathLength) { Assert(false); }
        void AddSingletonInstanceFieldAt(DynamicObject* instance, PropertyIndex index, bool isFixed, int typePathLength) { Assert(false); }
        void AddSingletonInstanceFieldAt(PropertyIndex index, int typePathLength) { Assert(false); }
#if DBG
        bool HasSingletonInstanceOnlyIfNeeded();
#endif
#endif
    };
}

