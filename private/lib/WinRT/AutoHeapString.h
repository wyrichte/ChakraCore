//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#pragma once

class AutoHeapString
{
private:
    size_t stringLength;
    LPWSTR heapString;

public:
    AutoHeapString() throw() : stringLength(0), heapString(nullptr)
    {
    }

    ~AutoHeapString() throw()
    {
        if (heapString)
        {
            HeapDeleteArray(stringLength, heapString);
        }
    }

    // Initialize this string from an HSTRING.
    void CreateNew(size_t stringLength)
    {
        if (this->heapString)
        {
            HeapDeleteArray(this->stringLength, this->heapString);
        }
        this->heapString = HeapNewArray(wchar_t, stringLength);
        this->stringLength = stringLength;
    }

    LPWSTR Get() const throw()
    {
        return heapString;
    }

    size_t GetLength() const throw()
    {
        return stringLength;
    }
};
