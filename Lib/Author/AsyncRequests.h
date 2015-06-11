#pragma once

namespace Authoring
{
  struct AsyncRequest
  {
      LPCWSTR requestSource;
      LPCWSTR requestCharSet;
      LPCWSTR requestType;

      AsyncRequest() : requestSource(nullptr), requestCharSet(nullptr), requestType(nullptr) { }
      AsyncRequest(LPCWSTR requestSource, LPCWSTR requestCharSet, LPCWSTR requestType)
        : requestSource(requestSource), requestCharSet(requestCharSet), requestType(requestType) { }
  };

  class AsyncRequestList : public RefCounted<DeletePolicy::OperatorDelete>
  {
      ArenaAllocator _alloc;
      JsUtil::List<AsyncRequest, ArenaAllocator> *m_asyncRequests;

      AsyncRequestList(PageAllocator* pageAlloc);

  public:
      static AsyncRequestList* New(PageAllocator* pageAlloc);
      virtual ~AsyncRequestList() { }
      void Add(AsyncRequest* asyncRequest);
      JsUtil::List<AsyncRequest, ArenaAllocator> * Items();
  };
}
