//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include <initguid.h>
#define INITGUID
#include "guids.h"
#include <activscp.h>

extern "C"
{       
    extern const IID GUID_NULL = {};
    const IID IID_IUnknown = __uuidof(IUnknown);
};

// copied from inetcore\publicshed\sdk\uuid\ieguids.c
DEFINE_GUID(CGID_MSHTML, 0xde4ba900, 0x59ca, 0x11cf, 0x95, 0x92, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00);
DEFINE_GUID(CGID_DocHostCommandHandler, 0xf38bc242, 0xb950, 0x11d1, 0x89, 0x18, 0x00, 0xc0, 0x4f, 0xc2, 0xc8, 0x36);
