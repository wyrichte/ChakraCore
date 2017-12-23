//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

enum class VerifyAllByteCodeReturnCode
{
    Success = 0,
    FileOpenError = 1,
    BadFileContent = 2,
    GUIDMismatch = 3,
    RequireGUIDUpdate = 4,
};

VerifyAllByteCodeReturnCode VerifyAllByteCode(LPWSTR verificationFile);