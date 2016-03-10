//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

namespace JsDiag
{
    // Internal error codes to hint error details. !!! Do not change any existing error code value !!!
#define ENUM_BEGIN(x)   enum class x {
#define ENUM_END()      };
#define ENUM_V(x, v)    x = v,
#define ENUM__(x)       x,
#include "DiagErrorCode.h"

    struct DiagException
    {
        HRESULT hr;
        DiagErrorCode errorCode;

        DiagException(HRESULT hr, DiagErrorCode errorCode = DiagErrorCode::NONE) :
            hr(hr),
            errorCode(errorCode)
        {
        }

        __declspec(noreturn) static void Throw(HRESULT hr, DiagErrorCode errorCode = DiagErrorCode::NONE)
        {
#if defined(ENABLE_DEBUG_CONFIG_OPTIONS)
            char16 buffer[16];
            DWORD count = GetEnvironmentVariableW(_u("DiagDebug"), buffer, _countof(buffer));
            if (count > 0)
            {
                int debugbreak = _wtoi(buffer);
                if(debugbreak > 0)
                {
                    DebugBreak();
                }
            }
#endif
            throw DiagException(hr, errorCode);
        }

        __declspec(noreturn) static void ThrowOOM(DiagErrorCode errorCode = DiagErrorCode::NONE)
        {
            Throw(E_OUTOFMEMORY, errorCode);
        }
    };

    inline void CheckHR(HRESULT hr, DiagErrorCode errorCode = DiagErrorCode::NONE)
    {
        if (!SUCCEEDED(hr))
        {
            DiagException::Throw(hr, errorCode);
        }
    }

//
// Verify a script runtime condition. If false, throw E_UNEXPECTED DiagException.
// When the script runtime is in certain corrupted state, diagnostics cannot continue.
//
// Defined this as a macro instead of a function because PREFAST has trouble to understand
// this will not return if condition fails. Neither _When_(!condition, _Analysis_noreturn_)
// nor _When_(!condition, _Raises_SEH_exception_) works.
//
#define DIAG_VERIFY_RUNTIME(condition) \
    if (!(condition)) { \
        AssertMsg(false, #condition); \
        DiagException::Throw(E_UNEXPECTED, DiagErrorCode::RUNTIME_VERIFY_FAIL); \
    } \

} // namespace JsDiag.
