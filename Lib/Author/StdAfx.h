//----------------------------------------------------------------------------
//
// File: StdAfx.h
//
// Copyright (C) Microsoft. All rights reserved. 
//
//----------------------------------------------------------------------------

#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>        // added for ntassert.h
#include <ntassert.h>
#include <atlbase.h>
#include <xmllite.h>

#include "..\Parser\Parser.h"
#include "..\Runtime\runtime.h"
#include "..\Common\Common.h"
#include "..\Common\Exceptions\reporterror.h"
#include "Authoring.h"
