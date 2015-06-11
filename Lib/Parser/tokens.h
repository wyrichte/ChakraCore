//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

/*****************************************************************************
*
*  Define the token kind enum. Note that all entries from the keyword
*  table are defined first, such that the first entry has the value 0.
*/

enum tokens
{
    tkNone,
#define KEYWORD(tk,...) tk,
#define TOK_DCL(tk,...) tk,
#include "keywords.h"

    tkLimKwd,
    tkLastKwd = tkLimKwd - 1,


    tkEOF,        // end of source code
    tkIntCon,    // integer literal
    tkFltCon,    // floating literal
    tkStrCon,    // string literal
    tkRegExp,    // regular expression literal

#ifdef LANGUAGE_SERVICE
    tkExternalSourceStart,
    tkExternalSourceEnd,
#endif
    tkLim
};
