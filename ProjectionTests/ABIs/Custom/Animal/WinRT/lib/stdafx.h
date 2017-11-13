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
#include "animals.h"
#include "Fabrikam.h"
#include <windowscollectionsp.h>
#include "InterfaceWithEventServer.h"

#define IfFailedReturn(expr) if(FAILED(expr)) { return expr; }
#define IfFailedGoLabel(expr, label) if (FAILED(expr)) { goto label; } 
#define IfFailedGo(expr) IfFailedGoLabel(expr, LReturn)
#define IfNullReturnError(expr, errorToReturn) if ((expr) == NULL) { return (errorToReturn); }

#define AddRefPtr(ptr) if(ptr != NULL) { ptr->AddRef(); }
#define ReleasePtr(ptr) if(ptr != NULL) { ptr->Release(); }