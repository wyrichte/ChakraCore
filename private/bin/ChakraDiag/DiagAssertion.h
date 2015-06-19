//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//---------------------------------------------------------------------------

// As we disabled Runtime assertions due to dependency on quite a lot of other Runtime functionality that we don't need here,
// we still need the Assert, so re-define it in a simple way.
#pragma once
#include <stdio.h>

#ifdef Assert
#undef Assert
#endif

#ifdef AssertMsg
#undef AssertMsg
#endif

#if defined(DBG)
//#define AssertMsg(exp, comment) NT_ASSERTMSG(comment, exp);
#define AssertMsg(exp, comment)   \
    do { \
        if (!(exp)) \
        { \
            fprintf(stderr, "ASSERTION (%s, line %d) %s %s\n", __FILE__, __LINE__, _CRT_STRINGIZE(exp), comment); \
            fflush(stderr); \
            DebugBreak(); \
        } \
    } while(0)
#else
#define AssertMsg(exp, comment) ((void)0)
#endif //defined(DBG)

#define Assert(exp)             AssertMsg(exp, #exp)

#ifndef CompileAssert
#define CompileAssert(e) \
    typedef char __C_ASSERT__[(e) ? 1 : -1];
#endif

//
// Customize ATLASSERT. Otherwise default ATLASSERT delegates to _ASSERTE in C runtime,
// which does not hit unless linking to the right debug version of lib/runtime.
//
#ifdef ATLASSERT
#error Include this file before ATL headers
#endif
#define ATLASSERT Assert

#ifdef ASSERT
#undef ASSERT
#endif
#define ASSERT Assert
