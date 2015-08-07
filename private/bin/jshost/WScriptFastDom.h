/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once

#include "edgescriptdirect.h"
#include "wscript.h"

class WScriptFastDom
{
public:
    class CallbackMessage : public MessageBase
    {
    private:
        Var m_function;
        
    public:
        CallbackMessage(unsigned int time, Var function);
        ~CallbackMessage();

        virtual HRESULT Call() override;

        HRESULT CallJavascriptFunction();
        typedef Var CustomArgType;

        template <class Func>
        static CallbackMessage* Create(Var function, const Func& func, unsigned int time = 0)
        {
            return new CustomMessage<Func, CallbackMessage>(time, function, func);
        }
    };

    class RunInfo
    {
    public:
        enum ContextType
        {
            self,
            sameThread,
            crossThread
        };

        LPCWSTR source;
        ContextType context;
        bool isDiagnosticHost;
        bool isPrimary;
        HRESULT hr;
        LPWSTR errorMessage;
        WORD domainId;

        RunInfo()
            : source(nullptr), context(ContextType::self), isDiagnosticHost(false), isPrimary(false), hr(S_OK), errorMessage(L""), domainId(0)
        {
        }

        void SetContext(BSTR context)
        {
            if (wcscmp(context, L"samethread") == 0)
            {
                this->context = ContextType::sameThread;
            }
            else if (wcscmp(context, L"crossthread") == 0)
            {
                this->context = ContextType::crossThread;
            }
            else
            {
                this->context = ContextType::self;
            }
        }
    };

    static Var Echo(Var function, CallInfo callInfo, Var* args);
    static Var Quit(Var function, CallInfo callInfo, Var* args);
    static Var StdErrWriteLine(Var function, CallInfo callInfo, Var* args);
    static Var StdErrWrite(Var function, CallInfo callInfo, Var* args);
    static Var StdOutWriteLine(Var function, CallInfo callInfo, Var* args);
    static Var StdOutWrite(Var function, CallInfo callInfo, Var* args);
    static Var StdInReadLine(Var function, CallInfo callInfo, Var* args);
    static Var StdInEOF(Var function, CallInfo callInfo, Var* args);
    static Var LoadScriptFile(Var function, CallInfo callInfo, Var* args);
    static Var LoadScript(Var function, CallInfo callInfo, Var* args);
    static bool ParseRunInfoFromArgs(CComPtr<IActiveScriptDirect> activeScriptDirect, CallInfo callInfo, Var* args, RunInfo& scriptArgs, bool isSourceRaw = false);
    static Var InitializeProjection(Var function, CallInfo callInfo, Var* args);
    static Var RegisterCrossThreadInterfacePS(Var function, CallInfo callInfo, Var* args);
    static Var GetWorkingSet(Var function, CallInfo callInfo, Var* args);
    static Var CopyOnWrite(Var function, CallInfo callInfo, Var* args);
    static Var CreateCanvasPixelArray(Var function, CallInfo callInfo, Var* args);
    static Var Shutdown(Var function, CallInfo callInfo, Var* args);
    static Var PerformSourceRundown(Var function, CallInfo callInfo, Var* args);
    static Var DebugDynamicAttach(Var function, CallInfo callInfo, Var* args);
    static Var DebugDynamicDetach(Var function, CallInfo callInfo, Var* args);
    static Var Edit(Var function, CallInfo callInfo, Var* args);
    static Var StartScriptProfiler(Var function, CallInfo callInfo, Var* args);
    static Var StopScriptProfiler(Var function, CallInfo callInfo, Var* args);
    static Var SetTimeout(Var function, CallInfo callInfo, Var* args);
    static Var ClearTimeout(Var function, CallInfo callInfo, Var* args);
    static Var EmitStackTraceEvent(Var function, CallInfo callInfo, Var* args);
    static Var CallFunction(Var function, CallInfo callInfo, Var* args);
    static Var SetEvalEnabled(Var function, CallInfo callInfo, Var* args); 
    static Var SetRestrictedMode(Var function, CallInfo callInfo, Var* args);
    static Var TestConstructor(Var function, CallInfo callInfo, Var* args);

    static HRESULT Initialize(IActiveScript * activeScript);
    static HRESULT InitializeStreams(IActiveScriptDirect *activeScriptDirect, Var wscript);
    static HRESULT InitializeProperty(IActiveScriptDirect *activeScriptDirect, __in LPCWSTR propName, __out Var * obj, __out PropertyId *propId);

    static void EnableEditTests();
    static void AddMessageQueue(MessageQueue *messageQueue);
    static void PushMessage(MessageBase *message) { s_messageQueue->Push(message); }
    static void SetMainScriptSite(JsHostActiveScriptSite* activeScriptSite);
    static void ClearMainScriptSite();
    static HRESULT __cdecl TestConstructorInitMethod(Var instance);
    static void ShutdownAll();

private:
    static HRESULT CreateArgsObject(IActiveScriptDirect *const activeScriptDirect, __out Var *argsObject);
    static Var EchoToStream(FILE * stream, bool newLine, Var function, unsigned int count, Var * args);
    static HRESULT AddMethodToObject(__in LPWSTR propertyName, __in IActiveScriptDirect* scriptDirect, __inout Var wscript, __in ScriptMethod signature);
    static HRESULT AddToScriptEngineMapNoThrow(Var globalObject, IJsHostScriptSite* jsHostScriptSite);
    static HRESULT RemoveFromScriptEngineMapNoThrow(Var globalObject, IJsHostScriptSite** jsHostScriptSite);

    static MessageQueue *s_messageQueue;
    __declspec(thread) static JsHostActiveScriptSite* s_mainScriptSite;

    static bool s_enableEditTest;
    static bool s_stdInAtEOF;
    static const int StdInMaxLineLength = 1024 * 1024;
};

