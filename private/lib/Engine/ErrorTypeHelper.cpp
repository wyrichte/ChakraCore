//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "EnginePch.h"

// The following is explicilty not localized for WER error reporting
const ErrorTypeHelper::MapArrayElem ErrorTypeHelper::mapArray[] =
{
    { kjstError, JavascriptError, _u("Generic error") },
    { kjstEvalError, JavascriptEvalError, _u("Eval error") },
    { kjstRangeError, JavascriptRangeError, _u("Range error") },
    { kjstReferenceError, JavascriptReferenceError, _u("Reference error") },
    { kjstSyntaxError, JavascriptSyntaxError, _u("Syntax error") },
    { kjstTypeError, JavascriptTypeError, _u("Type error") },
    { kjstURIError, JavascriptURIError, _u("URI error") },
    { kjstCustomError, CustomError, _u("Custom error") },
    { kjstWinRTError, WinRTError, _u("WinRT error") },
};


#ifdef _DEBUG
void ErrorTypeHelper::ValidateMapArray()
{
    Assert(mapArray[JavascriptError].internalType == kjstError);
    Assert(mapArray[JavascriptEvalError].internalType == kjstEvalError);
    Assert(mapArray[JavascriptRangeError].internalType == kjstRangeError);
    Assert(mapArray[JavascriptReferenceError].internalType == kjstReferenceError);
    Assert(mapArray[JavascriptSyntaxError].internalType == kjstSyntaxError);
    Assert(mapArray[JavascriptTypeError].internalType == kjstTypeError);
    Assert(mapArray[JavascriptURIError].internalType == kjstURIError);
    Assert(mapArray[CustomError].internalType == kjstCustomError);
    Assert(mapArray[WinRTError].internalType == kjstWinRTError);
    Assert(_countof(mapArray) == LastError + 1);
}
#endif


HRESULT ErrorTypeHelper::MapToInternal(JsErrorType externalErrorType, ErrorTypeEnum &internalErrorType)
{
#ifdef _DEBUG
    ValidateMapArray();
#endif
    if ((uint)externalErrorType >= _countof(mapArray))
    {
        return E_INVALIDARG;
    }
    internalErrorType = mapArray[externalErrorType].internalType;
    return S_OK;
};

JsErrorType ErrorTypeHelper::MapToExternal(ErrorTypeEnum internalErrorType)
{
#ifdef _DEBUG
    ValidateMapArray();
#endif
    if ((uint)internalErrorType >= _countof(mapArray))
    {
        return CustomError;
    }
    return mapArray[internalErrorType].externalType;
}

LPCWSTR ErrorTypeHelper::MapExternalToErrorText(JsErrorType externalErrorType)
{
#ifdef _DEBUG
    ValidateMapArray();
#endif
    if ((uint)externalErrorType >= _countof(mapArray))
    {
        return NULL;
    }
    return mapArray[externalErrorType].messageInEnglish;
}



