//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#pragma once

#define WIN32_LEAN_AND_MEAN 1
#define INC_OLE2

#include "windows.h"
#include "ntassert.h"
#include "intsafe.h"
#pragma push_macro("_DEBUG")
#undef _DEBUG
#pragma warning(push)
#pragma warning(disable:4838) // conversion from 'int' to 'UINT' requires a narrowing conversion
#pragma warning(disable:4456) // declaration of '' hides previous local declaration
#include "atlbase.h"
#pragma warning(pop)
#pragma pop_macro("_DEBUG")

#include "atlsafe.h"

#include "scaformat.h"
#include "scalookup.h"
#include "Core/CommonTypedefs.h"

#define IfFalseReturnError(expr, hr) do { if (!(expr)) { return (hr); } } while(FALSE)
#define IfNullReturnError(expr, hr) IfFalseReturnError(expr, hr)
#define IfFailedGoLabel(expr, label) if (FAILED(expr)) { goto label; }
#define IfFailGo(expr) IfFailedGoLabel(hr = (expr), Error)

#ifndef ASSERT
#define ASSERT NT_ASSERT
#endif

#include "StreamReader.h"
#include "SCAStringWalk.h"
#include "SCAWalk.h"
#include "PropertyReader.h"