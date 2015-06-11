//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

//
// Contains constants and tables used by the encoder.
//
// #include "AssemblyStep.h"

//THUMB2 specific decl
typedef unsigned int ENCODE_32;

//Add more, if required
#define IS_CONST_01FFFFFF(x) (((x) & ~0x01ffffff) == 0)

//Add more, if required
#define IS_CONST_NEG_26(x)   (((x) & ~0x01ffffff) == ~0x01ffffff)

//Add more, if required
#define IS_CONST_INT26(x)    (IS_CONST_01FFFFFF(x) || IS_CONST_NEG_26(x))
