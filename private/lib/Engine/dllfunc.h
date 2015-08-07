//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

// DLL export related headers.
#pragma once

extern long g_cLibLocks;

//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
class CComEngineModule :
    public CComModule
{
public:
        LONG Lock();
        LONG Unlock();
        LONG GetLockCount();
};

extern CComEngineModule _Module;
void DetachProcess();
// TODO:
// original jscript.dll is built for manually loading mui files for different languages instead of 
// depending on OS to do mui loading. This might be needed if we need to release new jscript as separate
// download package. 
class GL;
extern GL * g_pgllmap;
