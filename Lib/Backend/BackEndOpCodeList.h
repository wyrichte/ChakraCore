// Copyright (C) Microsoft. All rights reserved. 

//
// NOTE: This file is intended to be "#include" multiple times.  The call site must define the macro
// "MACRO" to be executed for each entry.
//
#if !defined(DEF_OP)
#error DEF_OP must be defined before including this file
#endif      

// -----------------------------------------------------------------------------------------------
// Additional machine independent opcode used byte backend
#define MACRO_BACKEND_ONLY(opcode, layout, attr) \
    DEF_OP(opcode, layout, OpBackEndOnly|attr)

#include "ByteCode\OpCodes.h"

DEF_OP(MDStart, Empty, None)

#define MACRO DEF_OP

// TODO (louisl): can we move the definition below into Runtim instead?
// TODO (louisl): how do we deal with different backends?
#ifdef _M_AMD64
    #include "..\..\BackEnd\amd64\MdOpcodes.h"
#elif defined(_M_IX86)
    #include "..\..\BackEnd\i386\MdOpcodes.h"
#elif defined(_M_ARM)
    #include "..\..\BackEnd\arm\MdOpcodes.h"
#elif defined(_M_ARM64)
    #include "..\..\BackEnd\arm64\MdOpcodes.h"
#endif

#undef MACRO

