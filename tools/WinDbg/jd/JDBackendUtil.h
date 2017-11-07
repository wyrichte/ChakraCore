//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

class JDBackendUtil
{
public:
    static ExtRemoteTyped GetFunctionBodyFromFunc(ExtRemoteTyped func);
    static EXT_CLASS_BASE::PropertyNameReader GetPropertyNameReaderFromFunc(ExtRemoteTyped func);
};