//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

namespace Authoring
{
    TYPE_STATS(MessageSet, L"MessageSet")

    void MessageSet::RecordError(long position, long length, HRESULT errorCode)
    {
        // Ignore the error if an error was already reported for this position.
        int count = messages->Count();
        if (count > 0)
        {
            InternalMessage *lastMessage =  messages->Item(count - 1);
            if (lastMessage->position == position)
                return;
        }
        length = length == 0 ? 1 : length;
        InternalMessage *message = Anew(Alloc(), InternalMessage, position, length, errorCode);

        messages->Add(message);
    }

    
    STDMETHODIMP MessageSet::get_Count(int *result) 
    {
        STDMETHOD_PREFIX;

        ValidatePtr(result, E_POINTER);

        *result = messages->Count();

        STDMETHOD_POSTFIX;
    }

    STDMETHODIMP MessageSet::GetItems(int startIndex, int count, AuthorFileMessage *messages)
    {
        STDMETHOD_PREFIX;

        ValidatePtr(messages, E_POINTER);
        ValidateArg(startIndex + count <= this->messages->Count());
        LCID lcid = GetUserLocale();

        for (int i = 0; i < count; i++)
        {
            int sourceIndex = startIndex + i; 
            InternalMessage *src = this->messages->Item(sourceIndex);
            AuthorFileMessage *dest = &messages[i];

            dest->position = src->position;
            dest->length = src->length;
            dest->message = BstrGetResourceString(HRESULT_CODE(src->errorCode), lcid);
            dest->messageID = HRESULT_CODE(src->errorCode);
        }

        STDMETHOD_POSTFIX;
    }

    void authorErrorHandler(void *data, charcount_t position, charcount_t length, HRESULT errorCode)
    {
        MessageSet *messageSet = static_cast<MessageSet *>(data);
        Assert(messageSet);
        messageSet->RecordError(position, length, errorCode);
    }

}