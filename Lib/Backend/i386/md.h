//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

//
// Machine dependent constants.
//

const int MachInt = 4;
const int MachRegInt = 4;
const int MachPtr = 4;
const int MachDouble = 8;
const int MachRegDouble = 8;
const int MachMaxInstrSize = 11;
const int MachArgsSlotOffset = MachPtr;
const int MachStackAlignment = MachDouble;
const unsigned int MachSignBit = 0x80000000;
const int MachSimd128 = 16;

const int PAGESIZE = 0x1000;

const IRType TyMachReg = TyInt32;
const IRType TyMachPtr = TyUint32;
const IRType TyMachDouble = TyFloat64;
const IRType TyMachSimd128 = TySimd128;

const DWORD EMIT_BUFFER_ALIGNMENT = 16;
const DWORD INSTR_ALIGNMENT = 1;
