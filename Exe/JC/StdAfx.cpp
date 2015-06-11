//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

// stdafx.cpp : source file that includes just the standard includes
// JnConsole.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

// Stub external APIs
LPWSTR JsUtil::ExternalApi::GetFeatureKeyName()
{
    return L"";
}

bool ConfigParserAPI::FillConsoleTitle(__ecount(cchBufferSize) LPWSTR buffer, size_t cchBufferSize, __in LPWSTR moduleName)
{
    return false;
}

void ConfigParserAPI::DisplayInitialOutput(__in LPWSTR moduleName)
{
}
