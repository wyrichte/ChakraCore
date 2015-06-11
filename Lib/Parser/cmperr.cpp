//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

#if DEBUG
#include <stdarg.h>
#endif //DEBUG

void ErrHandler::Throw(HRESULT hr)
{
    Assert(fInited);
    Assert(FAILED(hr));
    m_hr = hr;
#if ERROR_RECOVERY
    if (hr != ERRnoMemory && hr != E_FAIL && m_callback) m_callback(m_data, hr);
    else 
#endif
    throw ParseExceptionObject(hr);
}
