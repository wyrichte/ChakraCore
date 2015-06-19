//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#pragma once

#undef __PLACEMENT_NEW_INLINE
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

#include "DiagAssertion.h"
#include <atlbase.h>
#include <atlcoll.h>
#include <atlcom.h>
#include <ntassert.h>

#include "dbghelp.h"
#include "dbgeng.h"

#define null NULL
#include "thrownew.h"
#include "DiagException.h"
#include "AutoPtr.h"
#include "DiagAutoPtr.h"

#include "jsdiag.h"
#include "werdump.h"
#include "dllfunc.h"
#include "Serializer.h"
#include "Serializer.inl"
#include "ScriptDump.h"
#include "ScriptDumpReader.h"
