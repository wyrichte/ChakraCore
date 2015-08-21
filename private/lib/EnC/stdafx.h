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
#include "scrpting.h"

#include "..\Engine\IDebugBitCorrectApplicationThread.h"
#include "..\Engine\ScriptEngine.h"


#ifdef EDIT_AND_CONTINUE
#include "pnodediff.h"
#include "ParseTreeComparer.h"
#include "ScriptDebugDocument.h"
#include "SemanticChange.h"
#include "ScriptEdit.h"
#include "EditTest.h"
#endif
