//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once
#define WIN32_LEAN_AND_MEAN 1

#include <atlbase.h>
#include "edgescriptDirect.h"
#include "Runtime.h"

#include <objsafe.h>
#include <guids.h>

#include "ChakraInternalInterface.h"

#include "DynamicSourceHolder.h"
#include "jscriptdllcommon.h"

#include "hostvariant.h"
#include "refcountedHostVariant.h"
#include "hostdispatch.h"
#include "scrutil.h"
#include "NamedItemList.h"
#include "dispmemberproxy.h"

#include "JavascriptExternalOperators.h"
#include "DefaultScriptOperations.h"
#include "HostObject.h"
#include "DispatchHelper.h"
#include "NamedEventHandler.h"
#include "activdbg.h"
#include "activscp_private.h"
#include "ad1ex.h"
#include "codectx.h"
#include "breakpointProbe.h"
#include "ScriptDebugDocument.h"

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

#include "ComObjectBase.h"

#include "..\..\lib\staticlib\base\scriptenginebase.h"
#include "ScriptEngine.h"
#include "JsrtContext.h"

#include "scrpting.h"
#include "caller.h"
#include "activescripterror.h"
#include "scriptsite.h"

#include "ActiveScriptProfilerHeapEnum.h"
