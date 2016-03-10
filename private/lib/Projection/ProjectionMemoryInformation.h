//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// Description: Implementation for the ProjectionContext
//  This class is used to hold Projection related global fields
//  that is common in the same script engine. There should be only
//  one instance of ProjectionContext per ScriptEngine, and it's
//  freed when ScriptEngine is deleted. 

#if DBG_DUMP

#pragma once

namespace Projection
{
    class ProjectionObjectInstance;
    class CUnknownImpl;
    enum ReleaseBufferType;
    class FinalizableTypedArrayContents;

    struct ListForMemoryInfoDetails
    {
        virtual void Dump() = 0;
    };

    template<typename T>
    struct ListForMemoryInfoNode
    {
        size_t object;
        ListForMemoryInfoDetails *details;
        ULONG id;
        ListForMemoryInfoNode<T> *next;
        ListForMemoryInfoNode<T> *CreatedNext;
        ListForMemoryInfoNode<T> *DeletedNext;

        static T TaggedToUnknown(size_t taggedUnknown)
        {
            const size_t lowBitZero = (size_t) -2;
            auto pointer = reinterpret_cast<T>(taggedUnknown & lowBitZero); 
            return pointer;
        }

        static size_t UnknownToTagged(T objectToTag)
        {
            size_t result = (size_t)objectToTag;
            ++result; // Set the low bit
            return result;
        }

        ListForMemoryInfoNode(T objectToStore, ListForMemoryInfoDetails *details, ULONG id) : 
            details(details), id(id), next(nullptr), CreatedNext(nullptr), DeletedNext(nullptr)
        {
            object = UnknownToTagged(objectToStore);
        }

        ~ListForMemoryInfoNode()
        {
            if (details)
            {
                delete details;
            }
        }

        void Dump();
    };

    template<typename T>
    class ListForMemoryInfo
    {
        ListForMemoryInfoNode<T> *listHead;
        ListForMemoryInfoNode<T> *CreatedHead;
        ListForMemoryInfoNode<T> *DeletedHead;

    public:
        ListForMemoryInfo()
        {
            listHead = nullptr;
            CreatedHead = nullptr;
            DeletedHead = nullptr;
        }
        ~ListForMemoryInfo()
        {
            while (listHead)
            {
                ListForMemoryInfoNode<T> *toDelete = listHead;
                listHead = listHead->next;
                delete toDelete;
            }
        }

        ListForMemoryInfoNode<T> * Add(T objectToAdd, ListForMemoryInfoDetails *details = nullptr);
        ListForMemoryInfoNode<T> * Remove(T objectToRemove);
        ListForMemoryInfoNode<T> * Get(T objectToGet);
        void Dump(LPCWSTR headerMsg);
        void DumpDetails(LPCWSTR headerMsg);
    };

    struct ProjectionObjectDetails : ListForMemoryInfoDetails
    {
        ULONG creationUnkRef;
        ULONG disposeUnkRef;
        bool isZombied;

        ProjectionObjectDetails(ULONG creationUnkRef) : creationUnkRef(creationUnkRef), disposeUnkRef((ULONG)(-1)), isZombied(false)
        {
        }

        void Dump();
    };

    enum UnknownImplType
    {
        delegateWrapper,
        vectorWrapper,
        vectorViewWrapper,
        iterableWrapper,
        iteratorWrapper,
        propertyValueWrapper,
        referenceWrapper,
        referenceArrayWrapper
    };

    struct UnknownObjectDetails sealed : ListForMemoryInfoDetails
    {
        UnknownImplType unknownImplType;

        UnknownObjectDetails(UnknownImplType unknownImplType) : unknownImplType(unknownImplType)
        {
        }

        void Dump();
    };

    struct FinalizableArrayContentsDetails : ListForMemoryInfoDetails
    {
        ReleaseBufferType releaseBufferType;
        UINT32 numberOfElements;

        FinalizableArrayContentsDetails(ReleaseBufferType releaseBufferType, UINT32 numberOfElements) : releaseBufferType(releaseBufferType), numberOfElements(numberOfElements)
        {
        }

        void Dump();
    };

    class ProjectionMemoryInformation : public IProjectionContextMemoryInfo
    {
        ListForMemoryInfo<ProjectionObjectInstance *> projectionObjects;
        ListForMemoryInfo<CUnknownImpl *> unknownObjects;
        ListForMemoryInfo<FinalizableTypedArrayContents *> finalizableArrayContents;

    public:
        ProjectionMemoryInformation()
        {
        }

        ~ProjectionMemoryInformation()
        {
            DumpCurrentStats(_u("Stats while destroying thread"), true);
        }

        void Release()
        {
            delete this;
        }
        
        void DumpCurrentStats(LPCWSTR headerMsg, bool forceDetailed);
        void DumpDetailedCurrentStats();

        void AddProjectionObject(ProjectionObjectInstance *projectionObject, ULONG creationUnkRef);
        void DisposeProjectionObject(ProjectionObjectInstance *projectionObject, bool isDispose, ULONG disposeUnkRef = (ULONG)(-1));

        void AddUnknown(CUnknownImpl *unknownObject, UnknownImplType unknownImplType);
        void DestructUnknown(CUnknownImpl *unknownObject);

        void AddFinalizableArrayContents(FinalizableTypedArrayContents *finalizableArrayBuffer);
        void DisposeFinalizableArrayContents(FinalizableTypedArrayContents *finalizableArrayBuffer);
    };
};

#endif
