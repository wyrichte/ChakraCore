//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once
#define USED_IN_STATIC_LIB

// This is used in implementation not header, to make sure we don't have 
// size/format dependency on runtime.
#define SCRIPT_DIRECT_TYPE
#include "runtime.h"
#undef SCRIPT_DIRECT_TYPE

#include "base\finalizableObject.h"
#include "edgejsstatic.h"
#include "base\MockexternalObject.h"
#include "base\scriptContextbase.h"
#include "base\scriptenginebase.h"
#include "base\NumberUtilitiesBase.h"
#include <edgescriptdirect.h>
