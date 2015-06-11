//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js {

    TypePath* TypePath::New(Recycler* recycler, uint size)
    {
        size = max(size, InitialTypePathSize);
        size = PowerOf2Policy::GetSize(size);

        if (PHASE_OFF1(Js::TypePathDynamicSizePhase))
        {
            size = MaxPathTypeHandlerLength;
        }
        
        Assert(size <= MaxPathTypeHandlerLength);
        
        TypePath * newTypePath = RecyclerNewPlusZ(recycler, sizeof(PropertyRecord *) * size, TypePath);
        newTypePath->pathSize = (uint16)size;

        return newTypePath;
    }

    PropertyIndex TypePath::Lookup(PropertyId propId,int typePathLength)
    {
        return LookupInline(propId,typePathLength);
    }

    __inline PropertyIndex TypePath::LookupInline(PropertyId propId,int typePathLength)
    {
        if (propId == Constants::NoProperty) {
           return Constants::NoSlot;
        }
        PropertyIndex propIndex = Constants::NoSlot;
        if (map.TryGetValue(propId, &propIndex, assignments)) {
           if (propIndex<typePathLength) {
                return propIndex;
            }
        }
        return Constants::NoSlot;
    }

    TypePath * TypePath::Branch(Recycler * recycler, int pathLength, bool couldSeeProto) 
    {
        AssertMsg(pathLength < this->pathLength, "Why are we branching at the tip of the type path?");
        
        // Ensure there is at least one free entry in the new path, so we can extend it.
        // TypePath::New will take care of aligning this appropriately.
        TypePath * branchedPath = TypePath::New(recycler, pathLength + 1);

        for (PropertyIndex i = 0; i < pathLength; i++) 
        {
            branchedPath->AddInternal(assignments[i]);

#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
            if (couldSeeProto)
            {
                if (this->usedFixedFields.Test(i))
                {
                    // Review (jedmiad): This is actually too consevative.  See point 4 in PathTypeHandlerBase::ConvertToSimpleDictionaryType.
                    // We must conservatively copy all used as fixed bits if some prototype instance could also take
                    // this transition.  See comment in PathTypeHandlerBase::ConvertToSimpleDictionaryType.
                    // Yes, we could devise a more efficient way of copying bits 1 through pathLength, if performance of this
                    // code path proves important enough.
                    branchedPath->usedFixedFields.Set(i);
                }
                else if (this->fixedFields.Test(i))
                {
                    // We must clear any fixed fields that are not also used as fixed if some prototype instance could also take 
                    // this transition.  See comment in PathTypeHandlerBase::ConvertToSimpleDictionaryType.
                    this->fixedFields.Clear(i);
                }
            }
#endif

        }

#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
        // When branching, we must ensure that fixed field values on the prefix shared by the two branches are always
        // consistent.  Hence, we can't leave any of them uninitialized, because they could later get initialized to
        // different values, by two different instances (one on the old branch and one on the new branch).  If that happened
        // and the instance from the old branch later switched to the new branch, it would magically gain a different set 
        // of fixed properties!  
        if (this->maxInitializedLength < pathLength) 
        {
            this->maxInitializedLength = pathLength;
        }
        branchedPath->maxInitializedLength = pathLength;
#endif

#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
        if (PHASE_VERBOSE_TRACE1(FixMethodPropsPhase))
        {
            Output::Print(L"FixedFields: TypePath::Branch: singleton: 0x%p(0x%p)\n", this->singletonInstance, this->singletonInstance->Get());
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

        return branchedPath;
    }
    
    TypePath * TypePath::Grow(Recycler * recycler) 
    {
        AssertMsg(this->pathSize == this->pathLength, "Why are we growing the type path?");

        // Ensure there is at least one free entry in the new path, so we can extend it.
        // TypePath::New will take care of aligning this appropriately.
        TypePath * clonedPath = TypePath::New(recycler, this->pathLength + 1);

        for (PropertyIndex i = 0; i < pathLength; i++) 
        {
            clonedPath->AddInternal(assignments[i]);
        }

        // Copy fixed field info
        clonedPath->maxInitializedLength = this->maxInitializedLength;
        clonedPath->singletonInstance = this->singletonInstance;
        clonedPath->fixedFields = this->fixedFields;
        clonedPath->usedFixedFields = this->usedFixedFields;

        return clonedPath;
    }

#if DBG
    bool TypePath::HasSingletonInstanceOnlyIfNeeded()
    {
#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
        return DynamicTypeHandler::AreSingletonInstancesNeeded() || this->singletonInstance == null;
#else
        return true;
#endif
    }
#endif

    Var TypePath::GetSingletonFixedFieldAt(PropertyIndex index, int typePathLength, ScriptContext * requestContext)
    {
#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
        Assert(index < this->pathLength);
        Assert(index < typePathLength);
        Assert(typePathLength <= this->pathLength);

        if (!CanHaveFixedFields(typePathLength))
        {
            return null;
        }

        DynamicObject* localSingletonInstance = this->singletonInstance->Get();

        return localSingletonInstance != null && localSingletonInstance->GetScriptContext() == requestContext && this->fixedFields.Test(index) ? localSingletonInstance->GetSlot(index) : null;
#else
        return null;
#endif

    }

}

