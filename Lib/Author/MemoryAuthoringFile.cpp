//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

namespace Authoring
{
    TYPE_STATS(StringFileReader, L"StringFileReader")
    TYPE_STATS(MemoryAuthoringFile, L"MemoryAuthoringFile")

    STDMETHODIMP StringFileReader::Read(long length, __out_ecount(length) wchar_t *buffer, __out_ecount(1) long *read)
    {
        STDMETHOD_PREFIX;
        ValidateArg((length >= 0));
        ValidateArg(buffer);
        ValidateArg(read);

        long copyLength = length;
        if (length + this->offset > stringLength)
            copyLength = stringLength - this->offset;
        if (copyLength > 0) 
        {
            auto copyBytes = sizeof(wchar_t) * copyLength;
            memcpy_s(buffer, copyBytes, &(((LPWSTR)string)[this->offset]), copyBytes);
            this->offset += copyLength;
            *read = copyLength;
        }
        else
            *read = 0;

        STDMETHOD_POSTFIX;
    }

    STDMETHODIMP StringFileReader::Seek(long offset)
    {
        STDMETHOD_PREFIX;
        ValidateArg(offset >= 0 && static_cast<charcount_t>(offset) < stringLength);
        this->offset = static_cast<charcount_t>(offset);
        STDMETHOD_POSTFIX;
    }

    STDMETHODIMP StringFileReader::Close()
    {
        return S_OK;
    }

    STDMETHODIMP MemoryAuthoringFile::GetDisplayName(BSTR *name)
    {
#if DEBUG
        Assert(name && (this->name || this->nameLen == 0));
        *name = this->name ? String::ToBSTR(this->name, this->nameLen) : nullptr;
#else
        *name = NULL;
#endif
        return S_OK;
    }
        
    STDMETHODIMP MemoryAuthoringFile::GetLength(long *length)
    {
        if (length)
            *length = this->length;

        return S_OK;
    }
        
    STDMETHODIMP MemoryAuthoringFile::GetReader(IAuthorFileReader **result)
    {
        HRESULT hr = S_OK;
        ValidateArg(result);

        *result = new StringFileReader(text, length);

Error:
        return hr;
    }
        
    STDMETHODIMP MemoryAuthoringFile::StatusChanged(AuthorFileStatus newStatus)
    {
        status = newStatus;
        return S_OK;
    }

}