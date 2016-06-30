//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// This is supposed to exclude rarely-used .h files from windows.h and thus make build faster.
// See http://windows/winbuilddocs/Wiki%20Pages/NOT_LEAN_AND_MEAN.aspx for details.
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

#include "DiagAssertion.h"
#pragma warning(push)
#pragma warning(disable:4838) // conversion from 'int' to 'UINT' requires a narrowing conversion
#include "atlbase.h"
#pragma warning(pop)
#include <atlcoll.h>
#include <atlcom.h>
#include <ntassert.h>

#include "dbghelp.h"
#include "DbgEng.h"

#include "Core/CommonTypedefs.h"
#include "thrownew.h"
#include "BasePtr.h"
#include "AutoPtr.h"
#include "DiagException.h"
#include "DiagAutoPtr.h"
#include "Serializer.h"
#include "ScriptDump.h"
#include "ScriptDumpReader.h"

// .inl files
#include "Serializer.inl"
