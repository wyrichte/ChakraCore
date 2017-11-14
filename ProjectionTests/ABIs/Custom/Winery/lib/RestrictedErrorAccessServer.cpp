//  Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "RestrictedErrorAccessServer.h"

Winery::WinRTErrorTests::Errors Winery::WinRTErrorTests::RestrictedErrorAccessErrorCodes::m_Errors = {
    E_FAIL, 
    E_INVALIDARG, 
    E_POINTER, 
    E_NOINTERFACE, 
    E_NOTIMPL, 
    E_ABORT, 
    E_ACCESSDENIED, 
    E_HANDLE, 
    E_OUTOFMEMORY, 
    E_UNEXPECTED,
    E_BOUNDS,
    REGDB_E_CLASSNOTREG
};
