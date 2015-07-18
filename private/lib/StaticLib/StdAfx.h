//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once
#define USED_IN_STATIC_LIB

// This is used in implementation not header, to make sure we don't have 
// size/format dependency on runtime.
// Due to open source refactoring, the required files are moved out, but
// we will keep the explicit include path to ensure the knowledge of dependency.
#define SCRIPT_DIRECT_TYPE
#include "runtime.h"
#undef SCRIPT_DIRECT_TYPE

#include "..\..\..\core\lib\common\core\finalizableObject.h"
#include "edgejsstatic.h"
#include "base\MockexternalObject.h"
#include "..\..\..\core\lib\runtime\language\scriptContextbase.h"
#include "base\scriptenginebase.h"
#include "..\..\..\core\lib\common\common\NumberUtilitiesBase.h"
#include <edgescriptdirect.h>
