//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

//
//         Uppercase     Type    Type     Type     Type     Type     Type  
//            Name      Size 1  Size 2   Size 4   Size 8   Size 16  Size 32
//         ---------    ------  -------  -------  ------   ------   -------

IRBASETYPE(Illegal,   Illegal, Illegal, Illegal, Illegal, Illegal, Illegal)
IRBASETYPE(Int,       Int8,    Int16,   Int32,   Int64,   Illegal, Illegal)
IRBASETYPE(Uint,      Uint8,   Uint16,  Uint32,  Uint64,  Illegal, Illegal)
IRBASETYPE(Float,     Illegal, Illegal, Float32, Float64, Illegal, Illegal)
#ifdef SIMD_JS_ENABLED
IRBASETYPE(Simd,      Illegal, Illegal, Illegal, Illegal, Illegal, Illegal) // Is this correct ?
#endif
IRBASETYPE(Var,       Illegal, Illegal, Illegal, Illegal, Illegal, Illegal)
IRBASETYPE(Condcode,  Illegal, Illegal, Illegal, Illegal, Illegal, Illegal)
IRBASETYPE(Misc,      Illegal, Illegal, Illegal, Illegal, Illegal, Illegal)
