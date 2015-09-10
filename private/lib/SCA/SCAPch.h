//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once
#define WIN32_LEAN_AND_MEAN 1

#include <atlbase.h>

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
