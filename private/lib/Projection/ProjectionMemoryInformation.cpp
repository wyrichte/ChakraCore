//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// *******************************************************
// General projection related context information for the script engine
// *******************************************************

#include "ProjectionPch.h"

#if DBG_DUMP
namespace Projection
{
    template<typename T>
    void ListForMemoryInfoNode<T>::Dump()
    {
        Output::Print(_u("%lu\t\t"), id);
        if (object == 0)
        {
            Output::Print(_u("Destroyed\t"));
        }
        else 
        {
            Output::Print(_u("Alive\t\t"));
        }
                
        if (details != nullptr)
        {
            details->Dump();
        }

        Output::Print(_u("\n"));
    }

#define SortAddToListForMemoryInfo(typeOfList, nodeToAdd)                       \
    if (typeOfList##Head == nullptr || typeOfList##Head->id > nodeToAdd->id)    \
    {                                                                           \
        nodeToAdd->typeOfList##Next = typeOfList##Head;                         \
        typeOfList##Head = nodeToAdd;                                           \
    }                                                                           \
    else                                                                        \
    {                                                                           \
        ListForMemoryInfoNode<T> *current = typeOfList##Head;                   \
        while (current->typeOfList##Next != nullptr)                            \
        {                                                                       \
            if (current->typeOfList##Next->id > nodeToAdd->id)                  \
            {                                                                   \
                break;                                                          \
            }                                                                   \
                                                                                \
            current = current->typeOfList##Next;                                \
        }                                                                       \
                                                                                \
        nodeToAdd->typeOfList##Next = current->typeOfList##Next;                \
        current->typeOfList##Next = nodeToAdd;                                  \
    }


#define DumpSortedListForMemoryInfo(typeOfList, headerMsg)                      \
    if (Js::Configuration::Global.flags.TraceWin8Allocations)                   \
    {                                                                           \
        Output::Print(_u("%s %s : "), headerMsg, _u(#typeOfList));                    \
    }                                                                           \
                                                                                \
    while (typeOfList##Head != nullptr)                                         \
    {                                                                           \
        ListForMemoryInfoNode<T> * objectToDelete = typeOfList##Head;           \
        if (Js::Configuration::Global.flags.TraceWin8Allocations)               \
        {                                                                       \
            Output::Print(_u("%lu  "), objectToDelete->id);                        \
        }                                                                       \
        typeOfList##Head = typeOfList##Head->typeOfList##Next;                  \
        objectToDelete->typeOfList##Next = nullptr;                             \
    }                                                                           \
                                                                                \
    if (Js::Configuration::Global.flags.TraceWin8Allocations)                   \
    {                                                                           \
        Output::Print(_u("\n"));                                                   \
        Output::Flush();                                                        \
    }

    template<typename T>
    ListForMemoryInfoNode<T> * ListForMemoryInfo<T>::Add(T objectToAdd, ListForMemoryInfoDetails *details)
    {
        ULONG id = (listHead == nullptr) ? 0 : (listHead->id + 1);
        ListForMemoryInfoNode<T> *nodeToAdd = new ListForMemoryInfoNode<T>(objectToAdd, details, id);

        // Add it to the All Object list
        nodeToAdd->next = listHead;
        listHead = nodeToAdd;

        // Add to the creation List
        SortAddToListForMemoryInfo(Created, nodeToAdd);

        return nodeToAdd;
    }

    template<typename T>
    ListForMemoryInfoNode<T> * ListForMemoryInfo<T>::Remove(T objectToRemove)
    {
        ListForMemoryInfoNode<T> *nodeOfRemoveObject = listHead;
        size_t objectToRemoveTagged = ListForMemoryInfoNode<T>::UnknownToTagged(objectToRemove);
        while (nodeOfRemoveObject != nullptr)
        {
            if (nodeOfRemoveObject->object == objectToRemoveTagged)
            {
                nodeOfRemoveObject->object = 0;
                break;
            }
            nodeOfRemoveObject = nodeOfRemoveObject->next;
        }

        Assert(nodeOfRemoveObject != nullptr);

        // Add to the deletion List
        SortAddToListForMemoryInfo(Deleted, nodeOfRemoveObject);

        return nodeOfRemoveObject;
    }

    template<typename T>
    ListForMemoryInfoNode<T> * ListForMemoryInfo<T>::Get(T objectToGet)
    {
        ListForMemoryInfoNode<T> *nodeOfObject = listHead;
        size_t objectToGetTagged = ListForMemoryInfoNode<T>::UnknownToTagged(objectToGet);
        while (nodeOfObject != nullptr)
        {
            if (nodeOfObject->object == objectToGetTagged)
            {
                return nodeOfObject;
            }
            nodeOfObject = nodeOfObject->next;
        }

        return nullptr;
    }

    template<typename T>
    void ListForMemoryInfo<T>::Dump(LPCWSTR headerMsg)
    {
        // Add to the deletion List
        DumpSortedListForMemoryInfo(Created, headerMsg);
        DumpSortedListForMemoryInfo(Deleted, headerMsg);
    }

    template<typename T>
    void ListForMemoryInfo<T>::DumpDetails(LPCWSTR headerMsg)
    {
        if (Js::Configuration::Global.flags.TraceWin8Allocations)
        {
            ULONG createdObjects = 0;
            ULONG deletedObjects = 0;

            Output::Print(headerMsg);
            ListForMemoryInfoNode<T> *current = listHead;
            while(current != nullptr)
            {
                current->Dump();
                if (current->object == 0)
                {
                    deletedObjects++;
                }
                else
                {
                    createdObjects++;
                }
                current = current->next;
            }

            Output::Print(_u("\nAlive: %lu\t\tDead: %lu\n"), createdObjects, deletedObjects);
            Output::Flush();
        }
    }

    void ProjectionObjectDetails::Dump()
    {
        Output::Print(_u("%lu\t\t\t\t"), creationUnkRef);

        //if (disposeUnkRef == (ULONG)(-1))
        //{
        //    Output::Print(_u("-"), creationUnkRef);
        //}
        //else
        //{
        //    Output::Print(_u("%lu"), disposeUnkRef);
        //}
    }

    void UnknownObjectDetails::Dump()
    {
        switch(unknownImplType)
        {
        case delegateWrapper:
            Output::Print(_u("Delegate"));
            break;

        case vectorWrapper:
            Output::Print(_u("IVector"));
            break;

        case vectorViewWrapper:
            Output::Print(_u("IVectorView"));
            break;

        case iteratorWrapper:
            Output::Print(_u("IIterator"));
            break;

        case iterableWrapper:
            Output::Print(_u("IIterable"));
            break;

        case propertyValueWrapper:
            Output::Print(_u("IPropertyValue"));
            break;

        case referenceWrapper:
            Output::Print(_u("IReference"));
            break;

        case referenceArrayWrapper:
            Output::Print(_u("IReferenceArray"));
            break;
        }
    }

    void FinalizableArrayContentsDetails::Dump()
    {
        Output::Print(_u("%u\t\t\t\t%s"), numberOfElements, (releaseBufferType == releaseBufferUsingCoTaskMemFree) ? _u("CoTaskMemAlloc") : _u("new[]"));
    }

    void ProjectionMemoryInformation::DumpCurrentStats(LPCWSTR headerMsg, bool forceDetailed)
    {
        if (Js::Configuration::Global.flags.TraceWin8Allocations)
        {
            Output::Print(_u("\nMemoryTrace: %s\n"), headerMsg);
        }

        projectionObjects.Dump(_u("ProjectionObjects "));
        unknownObjects.Dump(_u("Unknowns "));
        finalizableArrayContents.Dump(_u("FinalizableArrayBuffer"));

        if (Js::Configuration::Global.flags.PrintWin8StatsDetailed || forceDetailed)
        {
            DumpDetailedCurrentStats();
        }
    }

    void ProjectionMemoryInformation::DumpDetailedCurrentStats()
    {
        if (Js::Configuration::Global.flags.TraceWin8Allocations)
        {
            //projectionObjects.DumpDetails(_u("\n\nProjectionObjects status:\n\nId\t\tStatus\t\tCreationUnkRef\t\tDisposeUnkRef\n"));
            projectionObjects.DumpDetails(_u("\n\nProjectionObjects status:\n\nId\t\tStatus\t\tCreationUnkRef\n"));
            unknownObjects.DumpDetails(_u("\n\n\nUnknowns status:\n\nId\t\tStatus\t\tType\n"));
            finalizableArrayContents.DumpDetails(_u("\n\n\nFinalizableArrayBuffer status:\n\nId\t\tStatus\t\tNumberOfElements\t\tBufferAllocatedUsing\n"));
        }
    }

    void ProjectionMemoryInformation::AddProjectionObject(ProjectionObjectInstance *projectionObject, ULONG creationUnkRef)
    {
        ProjectionObjectDetails *details = new ProjectionObjectDetails(creationUnkRef);
        ListForMemoryInfoNode<ProjectionObjectInstance *> *addedObject = projectionObjects.Add(projectionObject, details);
        if (Js::Configuration::Global.flags.TraceWin8Allocations)
        {
            Output::Print(_u("MemoryTrace: Created Projection Object: %lu    UnknownRefCount = %lu\n"), addedObject->id, creationUnkRef);
            Output::Flush();
        }
    }

    void ProjectionMemoryInformation::DisposeProjectionObject(ProjectionObjectInstance *projectionObject, bool isDispose, ULONG disposeUnkRef)
    {
        ListForMemoryInfoNode<ProjectionObjectInstance *> *removedObject;
        if (isDispose) {
            removedObject = projectionObjects.Remove(projectionObject);
        }
        else
        {
            removedObject = projectionObjects.Get(projectionObject);
        }
        Assert(removedObject->details);
        ProjectionObjectDetails * details = ((ProjectionObjectDetails *)(removedObject->details));
        if (!isDispose)
        {
            details->disposeUnkRef = disposeUnkRef;
            Assert(!details->isZombied);
            details->isZombied = true;
        }
        else if (!details->isZombied)
        {
            details->disposeUnkRef = disposeUnkRef;
        }

        if (Js::Configuration::Global.flags.TraceWin8DeallocationsImmediate)
        {
            Output::Print(_u("MemoryTrace: %s Projection Object: %lu    UnknownRefCount = %lu    isZombied = %s\n"), isDispose ? _u("Collected") : _u("Zombied"), removedObject->id, details->disposeUnkRef, (details->isZombied)? _u("true") : _u("false"));
            Output::Flush();
        }
    }

    void ProjectionMemoryInformation::AddUnknown(CUnknownImpl *unknownObject, UnknownImplType unknownImplType)
    {
        UnknownObjectDetails *details = new UnknownObjectDetails(unknownImplType);
        ListForMemoryInfoNode<CUnknownImpl *> *addedObject = unknownObjects.Add(unknownObject, details);
        if (Js::Configuration::Global.flags.TraceWin8Allocations)
        {
            Output::Print(_u("MemoryTrace: Created Unknown Object: %lu    Type = "), addedObject->id);
            details->Dump();
            Output::Print(_u("\n"));
            Output::Flush();
        }
    }

    void ProjectionMemoryInformation::DestructUnknown(CUnknownImpl *unknownObject)
    {
        ListForMemoryInfoNode<CUnknownImpl *> *removedObject = unknownObjects.Remove(unknownObject);
        if (Js::Configuration::Global.flags.TraceWin8DeallocationsImmediate)
        {
            Output::Print(_u("MemoryTrace: Destroying Unknown Object: %lu    Type = "), removedObject->id);
            removedObject->details->Dump();
            Output::Print(_u("\n"));
            Output::Flush();
        }
    }

    void ProjectionMemoryInformation::AddFinalizableArrayContents(FinalizableTypedArrayContents *finalizableArrayBuffer)
    {
        FinalizableArrayContentsDetails *details = new FinalizableArrayContentsDetails(finalizableArrayBuffer->releaseBufferType, finalizableArrayBuffer->numberOfElements);
        ListForMemoryInfoNode<FinalizableTypedArrayContents *> *addedObject = finalizableArrayContents.Add(finalizableArrayBuffer, details);
        if (Js::Configuration::Global.flags.TraceWin8Allocations)
        {
            Output::Print(_u("MemoryTrace: Created FinalizableArrayBuffer Object: %lu    Number of Elements = %u    Buffer Created Using = %s\n"), 
                addedObject->id, details->numberOfElements, (details->releaseBufferType == releaseBufferUsingCoTaskMemFree) ? _u("CoTaskMemAlloc") : _u("new[]"));
            Output::Flush();
        }
    }

    void ProjectionMemoryInformation::DisposeFinalizableArrayContents(FinalizableTypedArrayContents *finalizableArrayBuffer)
    {
        ListForMemoryInfoNode<FinalizableTypedArrayContents *> *removedObject = finalizableArrayContents.Remove(finalizableArrayBuffer);
        FinalizableArrayContentsDetails *details = (FinalizableArrayContentsDetails *)removedObject->details;
        if (Js::Configuration::Global.flags.TraceWin8DeallocationsImmediate)
        {
            Output::Print(_u("MemoryTrace: Disposing FinalizableArrayBuffer Object: %lu    Number of Elements = %u    Buffer Created Using = %s\n"), 
                removedObject->id, details->numberOfElements, (details->releaseBufferType == releaseBufferUsingCoTaskMemFree) ? _u("CoTaskMemAlloc") : _u("new[]"));
            Output::Flush();
        }
    }
}
#endif