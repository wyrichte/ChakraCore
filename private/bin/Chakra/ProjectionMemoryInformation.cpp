//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// *******************************************************
// General projection related context information for the script engine
// *******************************************************

#include "stdafx.h"

#if DBG_DUMP
namespace Projection
{
    template<typename T>
    void ListForMemoryInfoNode<T>::Dump()
    {
        Output::Print(L"%lu\t\t", id);
        if (object == 0)
        {
            Output::Print(L"Destroyed\t");
        }
        else 
        {
            Output::Print(L"Alive\t\t");
        }
                
        if (details != nullptr)
        {
            details->Dump();
        }

        Output::Print(L"\n");
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
        Output::Print(L"%s %s : ", headerMsg, L#typeOfList);                    \
    }                                                                           \
                                                                                \
    while (typeOfList##Head != nullptr)                                         \
    {                                                                           \
        ListForMemoryInfoNode<T> * objectToDelete = typeOfList##Head;           \
        if (Js::Configuration::Global.flags.TraceWin8Allocations)               \
        {                                                                       \
            Output::Print(L"%lu  ", objectToDelete->id);                        \
        }                                                                       \
        typeOfList##Head = typeOfList##Head->typeOfList##Next;                  \
        objectToDelete->typeOfList##Next = nullptr;                             \
    }                                                                           \
                                                                                \
    if (Js::Configuration::Global.flags.TraceWin8Allocations)                   \
    {                                                                           \
        Output::Print(L"\n");                                                   \
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

            Output::Print(L"\nAlive: %lu\t\tDead: %lu\n", createdObjects, deletedObjects);
            Output::Flush();
        }
    }

    void ProjectionObjectDetails::Dump()
    {
        Output::Print(L"%lu\t\t\t\t", creationUnkRef);

        //if (disposeUnkRef == (ULONG)(-1))
        //{
        //    Output::Print(L"-", creationUnkRef);
        //}
        //else
        //{
        //    Output::Print(L"%lu", disposeUnkRef);
        //}
    }

    void UnknownObjectDetails::Dump()
    {
        switch(unknownImplType)
        {
        case delegateWrapper:
            Output::Print(L"Delegate");
            break;

        case vectorWrapper:
            Output::Print(L"IVector");
            break;

        case vectorViewWrapper:
            Output::Print(L"IVectorView");
            break;

        case iteratorWrapper:
            Output::Print(L"IIterator");
            break;

        case iterableWrapper:
            Output::Print(L"IIterable");
            break;

        case propertyValueWrapper:
            Output::Print(L"IPropertyValue");
            break;

        case referenceWrapper:
            Output::Print(L"IReference");
            break;

        case referenceArrayWrapper:
            Output::Print(L"IReferenceArray");
            break;
        }
    }

    void FinalizableArrayContentsDetails::Dump()
    {
        Output::Print(L"%u\t\t\t\t%s", numberOfElements, (releaseBufferType == releaseBufferUsingCoTaskMemFree) ? L"CoTaskMemAlloc" : L"new[]");
    }

    void ProjectionMemoryInformation::DumpCurrentStats(LPCWSTR headerMsg, bool forceDetailed)
    {
        if (Js::Configuration::Global.flags.TraceWin8Allocations)
        {
            Output::Print(L"\nMemoryTrace: %s\n", headerMsg);
        }

        projectionObjects.Dump(L"ProjectionObjects ");
        unknownObjects.Dump(L"Unknowns ");
        finalizableArrayContents.Dump(L"FinalizableArrayBuffer");

        if (Js::Configuration::Global.flags.PrintWin8StatsDetailed || forceDetailed)
        {
            DumpDetailedCurrentStats();
        }
    }

    void ProjectionMemoryInformation::DumpDetailedCurrentStats()
    {
        if (Js::Configuration::Global.flags.TraceWin8Allocations)
        {
            //projectionObjects.DumpDetails(L"\n\nProjectionObjects status:\n\nId\t\tStatus\t\tCreationUnkRef\t\tDisposeUnkRef\n");
            projectionObjects.DumpDetails(L"\n\nProjectionObjects status:\n\nId\t\tStatus\t\tCreationUnkRef\n");
            unknownObjects.DumpDetails(L"\n\n\nUnknowns status:\n\nId\t\tStatus\t\tType\n");
            finalizableArrayContents.DumpDetails(L"\n\n\nFinalizableArrayBuffer status:\n\nId\t\tStatus\t\tNumberOfElements\t\tBufferAllocatedUsing\n");
        }
    }

    void ProjectionMemoryInformation::AddProjectionObject(ProjectionObjectInstance *projectionObject, ULONG creationUnkRef)
    {
        ProjectionObjectDetails *details = new ProjectionObjectDetails(creationUnkRef);
        ListForMemoryInfoNode<ProjectionObjectInstance *> *addedObject = projectionObjects.Add(projectionObject, details);
        if (Js::Configuration::Global.flags.TraceWin8Allocations)
        {
            Output::Print(L"MemoryTrace: Created Projection Object: %lu    UnknownRefCount = %lu\n", addedObject->id, creationUnkRef);
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
            Output::Print(L"MemoryTrace: %s Projection Object: %lu    UnknownRefCount = %lu    isZombied = %s\n", isDispose ? L"Collected" : L"Zombied", removedObject->id, details->disposeUnkRef, (details->isZombied)? L"true" : L"false");
            Output::Flush();
        }
    }

    void ProjectionMemoryInformation::AddUnknown(CUnknownImpl *unknownObject, UnknownImplType unknownImplType)
    {
        UnknownObjectDetails *details = new UnknownObjectDetails(unknownImplType);
        ListForMemoryInfoNode<CUnknownImpl *> *addedObject = unknownObjects.Add(unknownObject, details);
        if (Js::Configuration::Global.flags.TraceWin8Allocations)
        {
            Output::Print(L"MemoryTrace: Created Unknown Object: %lu    Type = ", addedObject->id);
            details->Dump();
            Output::Print(L"\n");
            Output::Flush();
        }
    }

    void ProjectionMemoryInformation::DestructUnknown(CUnknownImpl *unknownObject)
    {
        ListForMemoryInfoNode<CUnknownImpl *> *removedObject = unknownObjects.Remove(unknownObject);
        if (Js::Configuration::Global.flags.TraceWin8DeallocationsImmediate)
        {
            Output::Print(L"MemoryTrace: Destroying Unknown Object: %lu    Type = ", removedObject->id);
            removedObject->details->Dump();
            Output::Print(L"\n");
            Output::Flush();
        }
    }

    void ProjectionMemoryInformation::AddFinalizableArrayContents(FinalizableTypedArrayContents *finalizableArrayBuffer)
    {
        FinalizableArrayContentsDetails *details = new FinalizableArrayContentsDetails(finalizableArrayBuffer->releaseBufferType, finalizableArrayBuffer->numberOfElements);
        ListForMemoryInfoNode<FinalizableTypedArrayContents *> *addedObject = finalizableArrayContents.Add(finalizableArrayBuffer, details);
        if (Js::Configuration::Global.flags.TraceWin8Allocations)
        {
            Output::Print(L"MemoryTrace: Created FinalizableArrayBuffer Object: %lu    Number of Elements = %u    Buffer Created Using = %s\n", 
                addedObject->id, details->numberOfElements, (details->releaseBufferType == releaseBufferUsingCoTaskMemFree) ? L"CoTaskMemAlloc" : L"new[]");
            Output::Flush();
        }
    }

    void ProjectionMemoryInformation::DisposeFinalizableArrayContents(FinalizableTypedArrayContents *finalizableArrayBuffer)
    {
        ListForMemoryInfoNode<FinalizableTypedArrayContents *> *removedObject = finalizableArrayContents.Remove(finalizableArrayBuffer);
        FinalizableArrayContentsDetails *details = (FinalizableArrayContentsDetails *)removedObject->details;
        if (Js::Configuration::Global.flags.TraceWin8DeallocationsImmediate)
        {
            Output::Print(L"MemoryTrace: Disposing FinalizableArrayBuffer Object: %lu    Number of Elements = %u    Buffer Created Using = %s\n", 
                removedObject->id, details->numberOfElements, (details->releaseBufferType == releaseBufferUsingCoTaskMemFree) ? L"CoTaskMemAlloc" : L"new[]");
            Output::Flush();
        }
    }
}
#endif