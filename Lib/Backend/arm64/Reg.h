//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

extern char const * const RegNames[];

#define RA_DONTALLOCATE     0x1
#define RA_CALLEESAVE       0x2

enum RegNum : BYTE {
#define REGDAT(Name, Listing,    Encode,    Type,    BitVec)  Reg ## Name,
#include "RegList.h"
#undef REGDAT

    RegNumCount,  // Number of operations
};

#define FIRST_FLOAT_REG RegD0

#define FOREACH_REG(reg) \
        for (RegNum reg = (RegNum)(RegNOREG+1); reg != RegNumCount; reg = (RegNum)(reg+1)) 
#define NEXT_REG

#define FOREACH_INT_REG(reg) \
        for (RegNum reg = (RegNum)(RegNOREG+1); reg != FIRST_FLOAT_REG; reg = (RegNum)(reg+1)) 
#define NEXT_INT_REG

#define FOREACH_FLOAT_REG(reg) \
        for (RegNum reg = FIRST_FLOAT_REG; reg != RegNumCount; reg = (RegNum)(reg+1)) 
#define NEXT_FLOAT_REG

