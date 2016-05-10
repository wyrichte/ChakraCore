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

// =================
// Runtime Includes
// =================
#include "Runtime.h"
#include "Library\JavascriptRegularExpression.h"
#include "Library\JavascriptProxy.h"
#include "Library\SameValueComparer.h"
#include "Library\MapOrSetDataList.h"
#include "Library\JavascriptMap.h"
#include "Library\JavascriptSet.h"
#include "Library\JavascriptWeakMap.h"
#include "Library\JavascriptWeakSet.h"
// =================

#include "jscriptdllcommon.h"
#include "edgescriptDirect.h"

#include "SCAEngine.h"
#include "StreamHelper.h"
#include "StreamReader.h"
#include "StreamWriter.h"
#include "SCASerialization.h"
#include "SCADeserialization.h"
#include "SCAPropBag.h"
