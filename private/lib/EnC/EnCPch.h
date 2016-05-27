//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once
#define WIN32_LEAN_AND_MEAN 1

#pragma warning(push)
#pragma warning(disable:4456) // declaration of '' hides previous local declaration
#include <atlbase.h>
#pragma warning(pop)
// atlbase.h 3.0 leak a warning(push) under _DEBUG
#ifdef _DEBUG   
#pragma warning(pop)
#endif


#include "Parser.h"

// =================
// Runtime Includes
// =================
#include "Runtime.h"
#include "Debug\DiagProbe.h"
#include "Debug\BreakpointProbe.h"
#include "Debug\DebugDocument.h"
// =================

//====================================
// Engine includes
//====================================
#include <objsafe.h>
#include "activscp_private.h"
#include "edgescriptDirect.h"
#include "ChakraInternalInterface.h"
#include "scrutil.h"
#include "NamedItemList.h"
#include "..\StaticLib\base\ScriptEngineBase.h"
#include "IDebugBitCorrectApplicationThread.h"
#include "ScriptEngine.h"

#include "jscriptdllcommon.h"
#include "dllfunc.h"
#include "ComObjectBase.h"
#include "scrpting.h"

//====================================
// EnC includes
//====================================
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
