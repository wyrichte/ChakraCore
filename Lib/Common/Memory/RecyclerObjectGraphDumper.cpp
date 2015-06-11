//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include <StdAfx.h>

#ifdef RECYCLER_DUMP_OBJECT_GRAPH

RecyclerObjectGraphDumper::RecyclerObjectGraphDumper(Recycler * recycler, RecyclerObjectGraphDumper::Param * param) : 
    recycler(recycler),
    param(param),        
    dumpObjectName(null),
    dumpObject(null),    
    isOutOfMemory(false)
#ifdef PROFILE_RECYCLER_ALLOC
    , dumpObjectTypeInfo(null)
#endif
{
    recycler->objectGraphDumper = this;
}

RecyclerObjectGraphDumper::~RecyclerObjectGraphDumper()
{
    recycler->objectGraphDumper = null;
}

void RecyclerObjectGraphDumper::BeginDumpObject(wchar_t const * name)
{
    Assert(dumpObjectName == null);
    Assert(dumpObject == null);
    dumpObjectName = name;
}

void RecyclerObjectGraphDumper::BeginDumpObject(wchar_t const * name, void * address)
{
    Assert(dumpObjectName == null);
    Assert(dumpObject == null);
    swprintf_s(tempObjectName, _countof(tempObjectName), L"%s %p", name, address);
    dumpObjectName = tempObjectName;
}

void RecyclerObjectGraphDumper::BeginDumpObject(void * objectAddress)
{
    Assert(dumpObjectName == null);
    Assert(dumpObject == null);
    this->dumpObject = objectAddress;
#ifdef PROFILE_RECYCLER_ALLOC
    if (recycler->trackerDictionary)
    {
        Recycler::TrackerData * trackerData = recycler->GetTrackerData(objectAddress);
               
        if (trackerData != null)
        {
            this->dumpObjectTypeInfo = trackerData->typeinfo;
            this->dumpObjectIsArray = trackerData->isArray;
        }
        else
        {
            Assert(false);
            this->dumpObjectTypeInfo = null;
            this->dumpObjectIsArray = null;
        }
    }
#endif
}

void RecyclerObjectGraphDumper::EndDumpObject()
{
    Assert(this->dumpObjectName != null || this->dumpObject != null);
    this->dumpObjectName = null;
    this->dumpObject = null;
}
void RecyclerObjectGraphDumper::DumpObjectReference(void * objectAddress, bool remark)
{
    if (this->param == null || !this->param->dumpRootOnly || recycler->collectionState == CollectionStateFindRoots)
    {
        if (this->param != null && this->param->dumpReferenceFunc)
        {
            if (!this->param->dumpReferenceFunc(this->dumpObjectName, this->dumpObject, objectAddress))
                return;
        }
        Output::Print(L"\"");
        if (this->dumpObjectName)
        {
            Output::Print(L"%s", this->dumpObjectName);
        }
        else
        {
            Assert(this->dumpObject != null);
#ifdef PROFILE_RECYCLER_ALLOC
            RecyclerObjectDumper::DumpObject(this->dumpObjectTypeInfo, this->dumpObjectIsArray, this->dumpObject);
#else
            Output::Print(L"Address %p", objectAddress);
#endif
        }

        Output::Print(remark? L"\" => \"" : L"\" -> \"");
        recycler->DumpObjectDescription(objectAddress);

        Output::Print(L"\"\n");
    }
}
#endif