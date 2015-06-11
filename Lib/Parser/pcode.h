//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

// define the kcb* values for the pcode argument types
enum
{
#define PCAM(a,t,cb) kcb##a = cb,
#include "pcodes.h"
    MWUSED_kcb
};

// define the pcode argument types
#define PCAM(a,t,cb) typedef t TYP_##a;
#include "pcodes.h"

// define the pcode values
enum pcodes
{
#define PCODE(n,s,d,v,a) OP_##n,
#include "pcodes.h"
    OP_Count
};

// define the kcbOP_* values for the opcodes
enum
{
#define PCODE(n,s,d,v,a) kcbOP_##n = kcb##a + 1,
#include "pcodes.h"
    MWUNUSED_kcbOP
};

// define the last argument type for the opcode
#define PCODE(n,s,d,v,a) typedef TYP_##a TYP_##n;
#include "pcodes.h"


/*****************************************************************************/

// Beginning of statement descriptor
struct StatementSpan
{
    long ich;
    long cch;
};

struct VarDsc
{
    TYP_OFFS ibName; // variable name offset
};

struct ScopeDsc
{
    TYP_LC cvarArg;   // Number of arguments.
    TYP_LC cvarLcl;   // Number of locals.
    TYP_LC cvarTmp;   // Number of temps.
    INT cvarFnc;      // Number of functions.
    VarDsc rgvd[];  // Variable descriptors follow.
};


enum
{
    ffncGlobalCode = 0x0001,
    ffncNestedFunction = 0x0002,
    ffncNonLocalRefs = 0x0004
};


struct FncDsc
{
    long ibHint;       // offset to function name hint
    long ibName;       // offset to function name
    long cvarMaxStack; // max stack depth
    long codeOffset;       // offset to code
    long cbCode;       // size of code
    long ibosBase;     // index to first StatementSpan structure for this function
    long cbos;         // number of StatementSpan structures for this function
    uint grffnc;       // flags

#if SCRIPT_DEBUGGER
    long sourceOffset;        // offset to the source
    long cchSrc;       // length of the source in number of OLECHARs
#endif //SCRIPT_DEBUGGER
    ScopeDsc scope;    // scope descriptor - only ever one of these
};
