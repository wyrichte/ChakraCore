//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once
#define WIN32_LEAN_AND_MEAN 1

#include <atlbase.h>
#include "edgescriptDirect.h"

#include "Runtime.h"

#include <objsafe.h>

#include "ChakraInternalInterface.h"

#include "classfac.h"
#include "jscriptdllcommon.h"
#include "ComObjectBase.h"
#include "..\StaticLib\base\ScriptEngineBase.h"
#include "activscp_private.h"
#include "scrutil.h"
#include "NamedItemList.h"
#include "breakpointProbe.h"
#include "scrpting.h"

#if _WIN64
typedef IDebugApplicationThread64 IDebugBitCorrectApplicationThread;
#else
typedef IDebugApplicationThread IDebugBitCorrectApplicationThread;
#endif

#include "..\Engine\ScriptEngine.h"


#ifdef EDIT_AND_CONTINUE
#include "pnodediff.h"
#include "ParseTreeComparer.h"
#include "SemanticChange.h"
#include "ScriptEdit.h"
#include "EditTest.h"
#endif
