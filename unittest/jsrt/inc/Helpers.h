//-----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//-----------------------------------------------------------------------------

#pragma once

using namespace WEX::Common;

namespace JsrtUnitTests
{
    String GetCurrentModulePath();
    LPCWSTR LoadScriptFileWithPath(LPCWSTR filename); 
    LPCWSTR LoadScriptFile(LPCWSTR filename);
}