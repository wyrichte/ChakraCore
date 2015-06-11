/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once

#ifndef USE_EDGEMODE_JSRT
#define USE_EDGEMODE_JSRT
#endif // USE_EDGEMODE_JSRT
#include "jsrt.h"

class WScriptJsrt
{
public:
    typedef void (*OnAttachCallback)(JsValueRef function);
    static bool Initialize(OnAttachCallback onAttach);

    class CallbackMessage : public MessageBase
    {
        JsValueRef m_function;
        
        CallbackMessage(CallbackMessage const&);

    public:
        CallbackMessage(unsigned int time, JsValueRef function);
        ~CallbackMessage();

        HRESULT Call();
    };

    static void AddMessageQueue(MessageQueue *messageQueue);

    static LPCWSTR ConvertErrorCodeToMessage(JsErrorCode errorCode)
    {
        switch (errorCode)
        {
        case (JsErrorCode::JsErrorInvalidArgument) :
            return L"TypeError: InvalidArgument";
        case (JsErrorCode::JsErrorNullArgument) :
            return L"TypeError: NullArgument";
        case (JsErrorCode::JsErrorArgumentNotObject) :
            return L"TypeError: ArgumentNotAnObject";
        case (JsErrorCode::JsErrorOutOfMemory) :
            return L"OutOfMemory";
        case (JsErrorCode::JsErrorScriptException) :
            return L"ScriptError";
        case (JsErrorCode::JsErrorScriptCompile) :
            return L"SyntaxError";
        case (JsErrorCode::JsErrorFatal) :
            return L"FatalError";
        default:
            AssertMsg(false, "Unexpected JsErrorCode");
            return NULL;
        }
    }

private:
    static OnAttachCallback onAttach;

    static JsValueRef __stdcall EchoCallback(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static bool CreateArgsObject(JsValueRef *argsObject);
    static JsValueRef __stdcall QuitCallback(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef __stdcall LoadScriptFileCallback(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef __stdcall LoadScriptCallback(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef __stdcall GetWorkingSetCallback(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef __stdcall AttachCallback(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef __stdcall SetTimeoutCallback(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef __stdcall ClearTimeoutCallback(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);

    static MessageQueue *s_messageQueue;
};

