//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

BOOL FGetResourceString(long isz, __out_ecount(cchMax) OLECHAR *psz, int cchMax, LCID lcid,
                        BOOL fResourceDllOnly = FALSE);
BSTR BstrGetResourceString(long isz, LCID lcid);

struct LMAP
{
    LCID lcid;
    HANDLE hlib;
    BOOL fUnload;
    //#ifdef SCRIPT_MUI
    BOOL fMui;  // must use FreeMUILibrary
    //#ednif // SCRIPT_MUI
};

extern GL * g_pgllmap;
