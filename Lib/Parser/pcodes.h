//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

/***************************************************************************
Pcode argument types and opcodes.
***************************************************************************/

#ifdef PCAM

#ifndef PCAS
#define PCAS(a,b,c) PCAM(a,b,sizeof(b))
#endif //PCAS

// no args
PCAM(NONE, void, 0)

// single arg - (name, type, param type for Gen_* method)
PCAS(  U1,   byte,    int)
PCAS(  U2, ushort,   long)
PCAS(  I1,  sbyte,    int)
PCAS(  UI,   uint,  ulong)
PCAS(  I4,   long,   long)
PCAS(  LC,  short,   long)    // local
PCAS(  CV, ushort,   long)    // count of arguments
PCAS(NAME,   uint,  ulong)    // name or string
PCAS(OFFS,   uint,  ulong)    // label or other type of offset
PCAS(LINE,   uint,  ulong)    // label or other type of offset
PCAS( DBL, double, double)

// multiple args - (name, last type, size)
PCAM(  LCCV,ushort, sizeof(short) + sizeof(ushort)   ) // local, count
PCAM(  LCLC, short, sizeof(short) * 2                ) // 2 locals
PCAM(  NMBL,  byte, sizeof( uint) + sizeof( byte)    ) // name, boolean
PCAM(  NMCV,ushort, sizeof( uint) + sizeof(ushort)   ) // name, count
PCAM(  NMLC, short, sizeof( uint) + sizeof(short)    ) // name, local
PCAM(  UIUI,  uint, sizeof( uint) + sizeof( uint)    ) // 2 uints
PCAM(LCLCLC, short, sizeof(short) * 3                ) // 3 locals
PCAM(NMLCLC, short, sizeof( uint) + sizeof(short) * 2) // name, 2 locals
PCAM(NMUIU1,  byte, sizeof( uint) * 2 + sizeof(byte)) // name, index, byte
PCAM(NMLCBL, sbyte, sizeof( uint) + sizeof(short) + sizeof(sbyte)) // name, index, boolean


PCAM(LCOFFS,  uint, sizeof(short) + sizeof(uint)     ) // local, offset
PCAM(  LCI1, sbyte, sizeof(short) + sizeof(byte)     ) // local, I1
PCAM(  LCI4,  long,  sizeof(short)+ sizeof(long)     ) // local, I4
PCAM( LCDBL,double, sizeof(short) + sizeof(double)   ) // local, double
PCAM(  LCNM,  uint, sizeof(short) + sizeof(uint)     ) // local, double
PCAM(LCI4OFFS,uint, sizeof(short) + sizeof(long) + sizeof(uint)) //local, I4, offset
PCAM(LCLCOFFS,uint, sizeof(short) + sizeof(short) + sizeof(uint)) //local, I4, offset

PCAM(  I1LC, short,  sizeof(byte) + sizeof(short)    ) // local, I1
PCAM(  I4LC, short, sizeof(long) + sizeof(short)     ) // local, I4

#undef PCAS
#undef PCAM
#endif //PCAM


/*****************************************************************************/
//
//    pcode enum
//                , net stack effect
//                   , stack depth used (must be >= max(0, -net))
//                      , whether last arg is a count of variants to pop
//                         , opcode args

#ifdef PCODE

PCODE(None        , 0, 0, 0, NONE)

/*****************************************************************************/
/*                           Function start/end etc.                         */
/*****************************************************************************/

PCODE(FuncEnd     , 0, 0, 0, NONE)

PCODE(Bos0        , 0, 0, 0, NONE)
PCODE(Bos1        , 0, 0, 0, U1)
PCODE(Bos2        , 0, 0, 0, U2)
PCODE(Bos4        , 0, 0, 0, I4)
PCODE(DebugBreak  , 0, 0, 0, I4)


/*****************************************************************************/
/*                                Constants                                  */
/*****************************************************************************/

PCODE(IntConst    ,+1, 0, 0, I1)
PCODE(LngConst    ,+1, 0, 0, I4)
PCODE(FltConst    ,+1, 0, 0, DBL)
PCODE(StrConst    ,+1, 0, 0, NAME)
PCODE(RegExp      ,+1, 0, 0, NAME)

PCODE(False       ,+1, 0, 0, NONE)
PCODE(True        ,+1, 0, 0, NONE)

PCODE(Null        ,+1, 0, 0, NONE)
PCODE(Empty       ,+1, 0, 0, NONE)


/*****************************************************************************/
/*                            Rvalues and lvalues                            */
/*****************************************************************************/

// New opcodes for accessing variables/properties and calling functions:


PCODE(LocalLd     ,+1, 0, 0, LC)
PCODE(LocalAdr    ,+1, 0, 0, LC)
PCODE(LocalFncAdr ,+1, 0, 0, LC)
PCODE(LocalDelAdr ,+1, 0, 0, LC)

PCODE(LclLd       ,+1, 0, 0, LC)
PCODE(LclSt       , 0, 1, 0, LC)
PCODE(LclAdr      ,+1, 0, 0, LC)
PCODE(LclDelAdr   ,+1, 0, 0, LC)

PCODE(ArgLd       ,+1, 0, 0, LC)
PCODE(ArgSt       , 0, 1, 0, LC)
PCODE(ArgAdr      ,+1, 0, 0, LC)
PCODE(ArgDelAdr   ,+1, 0, 0, LC)

PCODE(NamedLd     ,+1, 0, 0, NAME)
PCODE(NamedAdr    ,+1, 0, 0, NAME)
PCODE(NamedFncAdr ,+1, 0, 0, NAME)
PCODE(NamedDelAdr ,+1, 0, 0, NAME)

PCODE(ThisLd      ,+1, 0, 0, NONE)

PCODE(MemLd       , 0, 1, 0, NAME)
PCODE(MemAdr      , 0, 1, 0, NAME)

PCODE(CallLd      , 0, 1, 1, CV)
PCODE(CallVoid    ,-1, 1, 1, CV)
PCODE(CallSt      ,-1, 2, 1, CV)
PCODE(CallStRev   ,-2, 2, 1, CV)

PCODE(NewLd       , 0, 1, 1, CV)

PCODE(IndxLd      ,-1, 2, 0, NONE)
PCODE(IndxSt      ,-2, 3, 0, NONE)
PCODE(IndxAdr     ,-1, 2, 0, NONE)
PCODE(IndxClean   , 0, 1, 0, NONE)

PCODE(AdrLd       , 0, 1, 0, NONE)
PCODE(AdrSt       ,-1, 2, 0, NONE)
PCODE(AdrStRev    ,-2, 2, 0, NONE)

PCODE(Dup         , 1, 1, 0, NONE)
PCODE(DupN        , 0, 0, 0, CV)    // CV values are pushed, not popped
PCODE(Stash       , 0, 0, 0, CV)

PCODE(NewArray    , 1, 0, 1, CV)
PCODE(NewObject   ,+1, 0, 0, NONE)
#if ECMA_COMPAT
PCODE(SetMemberWithAttr ,-1, 2, 0, NMLC)
#else
PCODE(SetMember   ,-1, 1, 0, NAME)
#endif


/*****************************************************************************/
/*                                Assignments                                */
/*****************************************************************************/


PCODE(Inc         , 0, 1, 0, NONE)
PCODE(Dec         , 0, 1, 0, NONE)

/*****************************************************************************/
/*                                  Jumps                                    */
/*****************************************************************************/

PCODE(Jmp         , 0, 0, 0, OFFS)

PCODE(JccTrue     ,-1, 1, 0, OFFS)
PCODE(JccFalse    ,-1, 1, 0, OFFS)


/*****************************************************************************/
/*                         Arithmetic/logical operators                      */
/*****************************************************************************/

PCODE(Neg         , 0, 1, 0, NONE)

PCODE(BitOr       ,-1, 2, 0, NONE)
PCODE(BitXor      ,-1, 2, 0, NONE)
PCODE(BitAnd      ,-1, 2, 0, NONE)
PCODE(BitNot      , 0, 1, 0, NONE)


PCODE(LogNot      , 0, 1, 0, NONE)
PCODE(TypeOf      , 0, 1, 0, NONE)
PCODE(Delete      , 0, 1, 0, NONE)
PCODE(Void        , 0, 1, 0, NONE)
PCODE(Pos         , 0, 1, 0, NONE)

PCODE(EQ          ,-1, 2, 0, NONE)
PCODE(NE          ,-1, 2, 0, NONE)
PCODE(LT          ,-1, 2, 0, NONE)
PCODE(LE          ,-1, 2, 0, NONE)
PCODE(GT          ,-1, 2, 0, NONE)
PCODE(GE          ,-1, 2, 0, NONE)

PCODE(Eqv         ,-1, 2, 0, NONE)
PCODE(NEqv        ,-1, 2, 0, NONE)
PCODE(Lsh         ,-1, 2, 0, NONE)
PCODE(Rsh         ,-1, 2, 0, NONE)
PCODE(Rs2         ,-1, 2, 0, NONE)

PCODE(Add         ,-1, 2, 0, NONE)
PCODE(Sub         ,-1, 2, 0, NONE)
PCODE(Mul         ,-1, 2, 0, NONE)
PCODE(Div         ,-1, 2, 0, NONE)
PCODE(Mod         ,-1, 2, 0, NONE)



/*****************************************************************************/
/*                           Function call/return                            */
/*****************************************************************************/

PCODE(FnEnsure    ,+1, 0, 0, UIUI)
PCODE(FnBindLcl   , 0, 1, 0, LC)
PCODE(FnBindNmd   , 0, 1, 0, NAME)
PCODE(EvtBind     ,-1, 2, 0, NAME)
PCODE(VarBind     , 0, 0, 0, NAME)

PCODE(FnReturn    ,-1, 1, 0, NONE)
PCODE(FnRetVoid   , 0, 0, 0, NONE)


/*****************************************************************************/
/*                               Miscellaneous                               */
/*****************************************************************************/

PCODE(Pop         ,-1, 1, 0, NONE)

PCODE(PopSave     ,-1, 1, 0, NONE)

PCODE(WithBeg     , 0, 1, 0, NONE)
PCODE(WithPop      ,-1, 1, 0, NONE)
PCODE(WithEnd     ,-1, 1, 0, NONE)



PCODE(ForInBeg    ,+3, 1, 0, NONE)
PCODE(ForInEnd    ,+2, 2, 0, NONE)
PCODE(ForInPop    ,-2, 2, 0, NONE)

PCODE(SwBeg          , 0, 0, 0, NONE)
PCODE(SwEnd          , 0, 0, 0, NONE)
PCODE(SwTest      ,-1, 2, 0, OFFS)


/*****************************************************************************/
/*                               Exception Handling                          */
/*****************************************************************************/

PCODE(EValuePush  , 1, 0, 0, NONE)
PCODE(InstOf      ,-1, 2, 0, NONE)
PCODE(In          ,-1, 2, 0, NONE)
PCODE(Throw       ,-1, 1, 0, NONE)
PCODE(TryEnd      ,-1, 1, 0, OFFS)
PCODE(TryPop      ,-1, 1, 0, NONE)
PCODE(TryBeg      , 1, 0, 0, OFFS)
PCODE(FinEnd      ,-2, 2, 0, NONE)
PCODE(Finally     ,-2, 2, 0, NONE)
PCODE(FinBeg      , 2, 0, 0, OFFS)
PCODE(SetupFnReturnAfterFinally    , -1, 1, 0, NONE)
PCODE(FnReturnAfterFinally        ,  0, 0, 0, NONE)
PCODE(SetupRethrowAfterFinally    ,  0, 0, 0, NONE)
PCODE(RethrowAfterFinally        ,  0, 0, 0, NONE)



/*****************************************************************************/
/*   Folding optimization - aggregates of highly used opcode sequences       */
/*****************************************************************************/
PCODE(IncLclJmp,    0,  0,  0, LCOFFS)
PCODE(LclStTrue,    0,  0,  0, LC)
PCODE(LclStFalse,   0,  0,  0, LC)
PCODE(LclStNull,    0,  0,  0, LC)
PCODE(LclStIntConst,0,  0,  0, LCI1)
PCODE(LclStLngConst,0,  0,  0, LCI4)
PCODE(LclStFltConst,0,  0,  0, LCDBL)
PCODE(LclStStrConst,0,  0,  0, LCNM)

PCODE(ArgStTrue,    0,  0,  0, LC)
PCODE(ArgStFalse,   0,  0,  0, LC)
PCODE(ArgStNull,    0,  0,  0, LC)
PCODE(ArgStIntConst,0,  0,  0, LCI1)
PCODE(ArgStLngConst,0,  0,  0, LCI4)
PCODE(ArgStFltConst,0,  0,  0, LCDBL)
PCODE(ArgStStrConst,0,  0,  0, LCNM)

PCODE(JccFalseLclLtLngConst,    0,  0,  0,  LCI4OFFS )
PCODE(LclStLclAddLcl,           0,  0,  0,  LCLCLC )
PCODE(JccFalseLclLtLcl,         0,  0,  0,  LCLCOFFS )
PCODE(LclIncPost,              +1,  0,  0,  LC)    
PCODE(LclDecPost,              +1,  0,  0,  LC)  
PCODE(LclInc,                   0,  0,  0,  LC)   
PCODE(LclDec,                   0,  0,  0,  LC)   
PCODE(LclAsgAddIntConst,        0,  0,  0, I1LC)
PCODE(LclAsgAddLngConst,        0,  0,  0, I4LC)
PCODE(LclAsgSubIntConst,        0,  0,  0, I1LC)
PCODE(LclAsgSubLngConst,        0,  0,  0, I4LC)
PCODE(LclAsgMulIntConst,        0,  0,  0, I1LC)
PCODE(LclAsgMulLngConst,        0,  0,  0, I4LC)
PCODE(LclAsgDivIntConst,        0,  0,  0, I1LC)
PCODE(LclAsgDivLngConst,        0,  0,  0, I4LC)
PCODE(LclAsgModIntConst,        0,  0,  0, I1LC)
PCODE(LclAsgModLngConst,        0,  0,  0, I4LC)
PCODE(LclAsgAndIntConst,        0,  0,  0, I1LC)
PCODE(LclAsgAndLngConst,        0,  0,  0, I4LC)
PCODE(LclAsgXorIntConst,        0,  0,  0, I1LC)
PCODE(LclAsgXorLngConst,        0,  0,  0, I4LC)
PCODE(LclAsgOrIntConst,         0,  0,  0, I1LC)
PCODE(LclAsgOrLngConst,         0,  0,  0, I4LC)
PCODE(LclAsgLshIntConst,        0,  0,  0, I1LC)
PCODE(LclAsgLshLngConst,        0,  0,  0, I4LC)
PCODE(LclAsgRshIntConst,        0,  0,  0, I1LC)
PCODE(LclAsgRshLngConst,        0,  0,  0, I4LC)
PCODE(LclAsgRs2IntConst,        0,  0,  0, I1LC)
PCODE(LclAsgRs2LngConst,        0,  0,  0, I4LC)


#undef PCODE
#endif //PCODE

