//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#pragma once

class AutoHSTRING
{
private:
    HSTRING _hstring;
    Js::DelayLoadWinRtString * winrtStringDelayLoad;

    //
    // helper function, always returns the passed in HRESULT
    //
    // if the HRESULT indicates success, frees any previous *target string,
    // and over-writes it with newValue
    //
    // if the HRESULT indicates failure, does nothing
    //
    HRESULT FreeAndAssignOnSuccess(HRESULT hr, HSTRING newValue, __out HSTRING *target)
    {
        if (SUCCEEDED(hr))
        {
            // InterlockedExchangePointer wouldn't have much value, unless we also modified
            //  all readers of *target to insert a ReadBarrier.
            HSTRING oldValue = *target;
            *target = newValue;
            winrtStringDelayLoad->WindowsDeleteString(oldValue);
        }
        return hr;
    }

public:

    AutoHSTRING(Js::DelayLoadWinRtString * winrtStringDelayLoad) throw() : _hstring(nullptr), winrtStringDelayLoad(winrtStringDelayLoad)
    {
    }

    ~AutoHSTRING() throw()
    {
        if (_hstring)
        {
            winrtStringDelayLoad->WindowsDeleteString(_hstring);
        }
    }

    // Initialize this string from an HSTRING.
    HRESULT Initialize(const HSTRING& other) throw()
    {
        return FreeAndAssignOnSuccess(S_OK, other, &_hstring);
    }

    // Explicit conversion to HSTRING
    HSTRING Get() const throw()
    {
        return _hstring;
    }
};
