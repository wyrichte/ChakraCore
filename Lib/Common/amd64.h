//---------------------------------------------------------------------------
//
// File: arm.h
//
// Copyright (C) by Microsoft Corporation.  All rights reserved.
//
//----------------------------------------------------------------------------

// ARM-specific macro definitions

#pragma once

#ifndef _M_AMD64
#error Include amd64.h in builds of AMD64 targets only.
#endif

extern "C" VOID amd64_SAVE_REGISTERS(void*);
