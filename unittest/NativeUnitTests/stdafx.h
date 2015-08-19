// Copyright (C) Microsoft. All rights reserved. 
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define IfFailGo(expr) IfFailGoto(expr, Error)

#define IfNullGo(expr, result) \
 do {                          \
    if ((expr) == NULL) {      \
        IfFailGo ((result));   \
    }                          \
 } while (0)
 
#define IfFailGoto(expr, label) \
do {                           \
hr = (expr);                  \
if (FAILED (hr)) {            \
goto label;                  \
}                             \
} while (0)
 
#define IfFailedReturn(EXPR) do { hr = (EXPR); if (FAILED(hr)) { return hr; }} while(FALSE)
#define MYCOUNTOF(X) (sizeof(X) / sizeof(X[0]))
typedef unsigned int uint;
typedef unsigned __int64 uint64;
typedef unsigned __int16 uint16;
#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <fstream>
#include <iomanip>
#include <windows.h>
#include <string>
#include <map>
#include <assert.h>
#include "time.h"
#include <float.h>


class AutoCriticalSection 
{
public:
    AutoCriticalSection(CRITICAL_SECTION* cs) {m_cs = cs; EnterCriticalSection(cs); };
    ~AutoCriticalSection() { assert(m_cs!= NULL); LeaveCriticalSection(m_cs); }
private:
    CRITICAL_SECTION* m_cs;
};

#if DBG
#define ChakraAssert(x) assert(x)
#else
#define ChakraAssert(x)
#endif

#define IfFalseAssertReturn(expr)  \
do { \
    if (!(expr)) { \
        ChakraAssert(expr); \
        return ; \
    } \
} while (FALSE)

#define IfFalsePrintAssertReturn(expr, msg, ...)  \
do { \
    if (!(expr)) { \
        printf(msg, __VA_ARGS__); \
        ChakraAssert(expr); \
        return ; \
    } \
} while (FALSE)

#define UnconditionallyFail(str) \
do { \
    printf("UNCONDITIONALLY failed: %s", str); \
    ChakraAssert(false); \
    return; \
} while (FALSE)

// TODO: reference additional headers your program requires here
