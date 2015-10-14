/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
// See comment in objbase.h for an explanation of this syntax.
#include <initguid.h>

#define INITGUID

#include <guids.h> 
#include <activscp.h>
#include ".\dispex.h"

// For IDebugHelperExOld and ProcessDebugManager
#include "activdbg.h"
#include "ad1ex.h"      

extern "C"
{   
    extern const IID GUID_NULL = {};
    const IID IID_IUnknown = __uuidof(IUnknown);
    const IID IID_IClassFactory = __uuidof(IClassFactory);    
    const IID IID_IInternetSecurityManager = __uuidof(IInternetSecurityManager);
    const IID CLSID_ProcessDebugManager = __uuidof(ProcessDebugManager);            // needed by iedevtools_host_iel1.lib

    // copied from inetcore\publicshed\sdk\uuid\ieguids.c
    DEFINE_GUID(CLSID_StdComponentCategoriesMgr, 0x0002E005, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
    DEFINE_GUID(CLSID_InternetSecurityManager, 0x7b8a2d94,0x0ac9,0x11d1,0x89,0x6c,0x00,0xc0,0x4f,0xb6,0xbf,0xc4);      
};
