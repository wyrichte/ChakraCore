//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#if _WIN64
typedef IDebugApplicationThread64 IDebugBitCorrectApplicationThread;
#else
typedef IDebugApplicationThread IDebugBitCorrectApplicationThread;
#endif
