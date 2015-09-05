//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once
#define WIN32_LEAN_AND_MEAN 1

#include <atlbase.h>
#include "edgescriptDirect.h"

#include "Parser.h"

// =================
// Runtime Includes
// =================
#include "Runtime.h"
#include "Debug\DiagProbe.h"
#include "Debug\BreakpointProbe.h"
#include "Debug\DebugDocument.h"
// =================

#include <objsafe.h>

#include "ChakraInternalInterface.h"

#include "classfac.h"
#include "jscriptdllcommon.h"
#include "ComObjectBase.h"
#include "..\StaticLib\base\ScriptEngineBase.h"
#include "activscp_private.h"
#include "scrutil.h"
#include "NamedItemList.h"
#include "scrpting.h"

#include "..\Engine\IDebugBitCorrectApplicationThread.h"
#include "..\Engine\ScriptEngine.h"


#ifdef EDIT_AND_CONTINUE
#include "pnodewalk.h"
#include "pnodevisit.h"
#include "pnodechange.h"
#include "pnodediff.h"
#include "ParseTreeComparer.h"
#include "ScriptDebugDocument.h"
#include "SemanticChange.h"
#include "ScriptEdit.h"
#include "EditTest.h"

#include "EditAndContinue.h"
#endif
