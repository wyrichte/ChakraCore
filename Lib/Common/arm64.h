//---------------------------------------------------------------------------
//
// File: arm64.h
//
// Copyright (C) by Microsoft Corporation.  All rights reserved.
//
//----------------------------------------------------------------------------

// ARM64-specific macro definitions

#pragma once

#ifndef _M_ARM64
#error Include arm64.h in builds of ARM64 targets only.
#endif

extern "C" LPVOID arm64_GET_CURRENT_FRAME(void);
extern "C" VOID arm64_SAVE_REGISTERS(void*);

/*
 * The relevant part of the frame looks like this (high addresses at the top, low ones at the bottom):
 *
 * ----------------------
 * r3     <=== Homed input parameters
 * r2     <
 * r1     <
 * r0     <===
 * lr     <=== return address
 * r11    <=== current r11 (frame pointer)
 * ...
 */

const DWORD ReturnAddrOffsetFromFramePtr = 1;
const DWORD ArgOffsetFromFramePtr = 2;
