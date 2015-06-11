//---------------------------------------------------------------------------
// Copyright (C) 2011 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"

using namespace Authoring;

TYPE_STATS(AuthoringFileText, L"AsyncRequestList")

AsyncRequestList::AsyncRequestList(PageAllocator* pageAlloc) 
    : _alloc(L"ls:AsyncRequests", pageAlloc, Js::Throw::OutOfMemory)
{
    m_asyncRequests = JsUtil::List<AsyncRequest, ArenaAllocator>::New(&_alloc);
}

AsyncRequestList* AsyncRequestList::New(PageAllocator* pageAlloc)
{
  Assert(pageAlloc);

  return new AsyncRequestList(pageAlloc);
}

void AsyncRequestList::Add(AsyncRequest* asyncRequest)
{
  Assert(asyncRequest);
  Assert(m_asyncRequests);

  AsyncRequest request;
  request.requestSource = String::Copy(&_alloc, asyncRequest->requestSource);
  request.requestType = String::Copy(&_alloc, asyncRequest->requestType);
  request.requestCharSet = String::Copy(&_alloc, asyncRequest->requestCharSet);
  m_asyncRequests->Add(request);
}

JsUtil::List<AsyncRequest, ArenaAllocator> * AsyncRequestList::Items() 
{ 
    return m_asyncRequests; 
}