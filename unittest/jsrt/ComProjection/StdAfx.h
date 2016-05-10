//-----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//-----------------------------------------------------------------------------

#pragma once

#define WIN32_LEAN_AND_MEAN 1

#include <windows.h>
#include <malloc.h>
#include <stdlib.h>
#include "wexstring.h"
#include "wextestclass.h"
#include "Helpers.h"
#include "oaidl.h"
#include "dispex.h"
#include "propvarutil.h"


#if defined(_DEBUG)
#define JS_ATL_DEBUG
#undef _DEBUG
#endif
#pragma warning(push)
#pragma warning(disable:4456) // declaration of '' hides previous local declaration
#include "atlbase.h"
#pragma warning(pop)
#ifdef JS_ATL_DEBUG
#define _DEBUG
#undef JS_ATL_DEBUG
#endif

#include "jsrt.h"
#include "TestComponent.h"
