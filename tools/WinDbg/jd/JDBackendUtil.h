//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

class JDBackendUtil
{
public:
    static JDRemoteTyped GetFunctionBodyFromFunc(JDRemoteTyped func);
    static EXT_CLASS_BASE::PropertyNameReader GetPropertyNameReaderFromFunc(JDRemoteTyped func);
};