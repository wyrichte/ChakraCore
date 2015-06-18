//---------------------------------------------------------------------------
//
// File: common.h
//
// Copyright (C) Microsoft. All rights reserved. 
//
//----------------------------------------------------------------------------

#pragma once

#define REGEX_TRIGRAMS 1 
// #define USE_POPCNT_INSTRUCTION 1
// #define USE_BITCOUNTS 1

#ifdef LANGUAGE_SERVICE
#define ERROR_RECOVERY 1
#define PARSENODE_EXTENSIONS 1
#define LANGUAGE_SERVICE_ONLY 1
#else
#define ERROR_RECOVERY 0
#define PARSENODE_EXTENSIONS 0
#define LANGUAGE_SERVICE_ONLY 0
#endif

#include "activdbg.h"
#include "activdbg100.h"
#include "activdbg_private.h"
#include "activaut.h"
// From Common.lib
#include "Common.h"

// FORWARD
namespace Js
{
    class ScriptContext;
    class JavascriptString;
    class JavascriptRegularExpressionResult;
}

namespace UnifiedRegex {
    struct RegexPattern;
    struct Program;
}

#pragma warning(push)
#pragma warning(disable: 4995) /* 'function': name was marked as #pragma deprecated */
#include <stdio.h>
#include <mbstring.h>
#include <activscp.h>
#pragma warning(pop)

// TODO: temporary workaround for getting at COleScript::GetUserLocale().
// We need a better way to pass around config.
extern LCID GetUserLocale();

#include "ParserCommon.h"
#include "scrutil.h"
#include "alloc.h"
#include "cmperr.h"
#include "errstr.h"
#include "globals.h"
#include "idiom.h"
#include "var.h"
#include "keywords.h"
#include "ptree.h"
#include "tokens.h"
#include "hash.h"
#include "CharClassifier.h"
#include "scan.h"
#include "screrror.h"
#include "rterror.h"
#include "parse.h"

#include "RegexFlags.h"

#include "unicode.h"
#include "vers.h"

#include "Chars.h"
#include "DebugWriter.h"
#include "RegexStats.h"
#include "CaseInsensitive.h"
#include "BitCounts.h"
#include "CharSet.h"
#include "CharMap.h"
#include "CharTrie.h"
#include "StandardChars.h"
#include "OctoquadIdentifier.h"
#include "TextbookBoyerMoore.h"
#include "RegexRunTime.h"
#include "RegexCompileTime.h"
#include "RegexParser.h"
#include "RegexPattern.h"

#include "CharTrie.inl"

#include "pnodewalk.h"
#include "pnodevisit.h"
#include "pnodechange.h"

#include "BackgroundParser.h"
