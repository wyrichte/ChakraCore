//  Copyright (c) Microsoft Corporation. All rights reserved.

// disable non-standard extension warning. This is necessary to use the 'override' 
// specifier in code that compiles at Warning level 4
#pragma warning(disable: 4481)

#include <windows.h>

// WinRT Related Includes
#include <wrl\implements.h>
#include <wrl\module.h>
#include <wrl\event.h>
#include <wrlwrappers.h>
#include <windowsstringp.h>
#include <roerrorapi.h>
#include "DevTests.CamelCasing.h"

#define IfFailedReturn(expr) if(FAILED(expr)) { return expr; }
#define IfFailedGoLabel(expr, label) if (FAILED(expr)) { goto label; } 
#define IfFailedGo(expr) IfFailedGoLabel(expr, LReturn)
#define IfNullReturnError(EXPR, ERROR) do { if (!(EXPR)) { return (ERROR); } } while(FALSE)