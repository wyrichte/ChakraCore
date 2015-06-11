// Copyright (C) Microsoft. All rights reserved. 

#pragma once

BEGIN_ENUM_BYTE(RejitReason)
    #define REJIT_REASON(n) n,
    #include "RejitReasons.h"
    #undef REJIT_REASON
END_ENUM_BYTE();

extern const char *const RejitReasonNames[];
extern const uint NumRejitReasons;