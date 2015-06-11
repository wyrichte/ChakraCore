//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

#define MD_GROW_LOCALS_AREA_UP

// Don't encode large user constants, because they won't emit them as 32-bit numbers
// in the instruction stream.
#undef MD_ENCODE_LG_CONSTS
#define MD_ENCODE_LG_CONSTS false

//
// Machine dependent constants.
//
const int MachChar = 1;
const int MachShort = 2;
const int MachInt = 4;
const int MachRegInt = 4;
const int MachPtr = 4;
const int MachDouble = 8;
const int MachRegDouble = 8;
const int MachArgsSlotOffset = MachPtr;
const int MachStackAlignment = MachDouble;

const int PAGESIZE = 0x1000;

const IRType TyMachReg = TyInt32;
const IRType TyMachPtr = TyUint32;
const IRType TyMachDouble = TyFloat64;

const DWORD EMIT_BUFFER_ALIGNMENT = 16;
const DWORD INSTR_ALIGNMENT = 2;

#ifdef INSERT_NOPS
const int CountNops = 10;
const int MachMaxInstrSize = (2 * CountNops + 1)*4;
#else
const int MachMaxInstrSize = 4;
#endif

#define SOFTWARE_FIXFOR_HARDWARE_BUGWIN8_502326