//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

namespace Authoring
{
    class MessageSet : public SimpleComObjectWithAlloc<IAuthorMessageSet>
    {
    private:
        struct InternalMessage
        {
            long position;
            long length;
            HRESULT errorCode;

            InternalMessage(long position, long length, HRESULT errorCode) : position(position), length(length), errorCode(errorCode) { }
        };

        typedef JsUtil::List<InternalMessage *, ArenaAllocator> Messages;

        Messages *messages;
        HRESULT hr;

    public:
        MessageSet(PageAllocator* pageAlloc) : SimpleComObjectWithAlloc<IAuthorMessageSet>(pageAlloc, L"ls:MessageSet")
        { 
            messages = Messages::New(Alloc());
        }

        int Count() { return messages->Count(); }

        void RecordError(long position, long length, HRESULT errorCode);
        STDMETHOD(get_Count)(int *result);

        STDMETHOD(GetItems)(int startIndex, int count, AuthorFileMessage *messages);
    };

    void authorErrorHandler(void *data, charcount_t position, charcount_t length, HRESULT errorCode);
}