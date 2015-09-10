//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once
#define WIN32_LEAN_AND_MEAN 1

#include <atlbase.h>

//====================================
// Runtime includes
//====================================
#include "Runtime.h"
#include "Debug\DiagProbe.h"

//====================================
// Projection includes
//====================================
#ifdef ENABLE_PROJECTION
#include "..\WinRT\WinRTLib.h"

#ifdef _M_ARM
#include "arm\CallingConvention.h"
#endif

#ifdef _M_ARM64
#include "arm64\CallingConvention.h"
#endif

#include "ProjectionMemoryInformation.h"
#include "ProjectionContext.h"
#endif

//====================================
// Engine includes
//====================================
#include <objsafe.h>
#include "activscp_private.h"
#include "edgescriptDirect.h"
#include "ChakraInternalInterface.h"
#include "scrutil.h"
#include "NamedItemList.h"
#include "..\..\lib\staticlib\base\scriptenginebase.h"
#include "IDebugBitCorrectApplicationThread.h"
#include "ScriptEngine.h"

#include "jscriptdllcommon.h"

//====================================
// JsrtChakra includes
//====================================
#include "JsrtContext.h"
