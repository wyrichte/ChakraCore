/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once

class DebugTargetHost;
class JsGlass;

struct Args
{
    Var* args;
public:
    Args(int count)
    {
        args = new Var[count];
    }
    ~Args()
    {
        delete [] args;
    }
    Var& operator[] (int i)
    {
        return args[i];
    }
};

#define JS_ARGS \
    va_list vl;\
    va_start(vl,_this);\
    Args args(callInfo.Count-1);\
    for(int i =0; i <(int)callInfo.Count-1; i++)\
    {\
        args[i] = va_arg(vl,Var);\
    }\
    va_end(vl);\

class OpenedFile
{
public:
    LPCWSTR _filename;
    FILE* _file;
    Var _fileStream;
    Var _fileContents;
    Var _fileReadAllFunction;

public:
    OpenedFile();
    ~OpenedFile();

    Var ReadAll(JsGlass* jsGlass);
};

class FileSystem
{
    // HACK glass doesn't need to open an arbitrary number of files
    OpenedFile _files[100];
    int _fileCount;

public:
    static LPWSTR GetContents(FILE* file);

public:
    FileSystem();

    OpenedFile* GetFile(Var fileStreamReadAllFunction);
    Var OpenTextFile(LPCWSTR filename, LPCWSTR openFlags, JsGlass* jsGlass);
    Var ReadAll(JsGlass* jsGlass, Var filestream);
};

struct JsGlassArgs
{
    LPCWSTR _jscriptDllPath;
    LPCWSTR _jsGlassPath;
#ifdef DIRECT_AUTHOR_TEST
    LPCWSTR _authorDllPath;
#endif

    JsGlassArgs();
    ~JsGlassArgs();
    HRESULT ParseCmdLine(int &argc, __inout_ecount(argc) const wchar_t** argv);
};

class JsGlass
{
private:
    bool _coInitialized;

    // Smart host for glass
    CComPtr<IActiveScriptDirect> _scriptDirect;
    CComPtr<IActiveScript> _activeScript;
#ifdef DIRECT_AUTHOR_TEST
    CComPtr<IAuthorServices> _authorServices;
#endif
    CComPtr<IActiveScriptSite> _fakeSite;

    // Debugging for Target Smart host
    CComPtr<IProcessDebugManager> _processDebugManager;


    // Root object for additional functionality exposed to Javascript
    Var _Eze;

    // Help deal with cross thread message passing
    DWORD _threadId;
    Message<JsGlass> _message;

    // Work around for getting back to the relevant scriptDirect
    static JsGlass* that;

public:
    CComPtr<IApplicationDebugger> _inlineDebugger;
    DebugTargetHost* _targetHost;
    Debugger* _debugger;
    CAuthoringHost * _authoringHost;
    FileSystem _fileSys;
    int _argc;
    const wchar_t** _argv;

public:

    static Var ScriptReadLine(Var method, CallInfo callInfo, Var _this, ...);
    static Var ScriptWrite(Var method, CallInfo callInfo, Var _this, ...);
    static Var ScriptArgumentItem(Var method, CallInfo callInfo, Var _this, ...);
    static Var ScriptOpenTarget(Var method, CallInfo callInfo, Var _this, ...);
    static Var ScriptOpenTextFile(Var method, CallInfo callInfo, Var _this, ...);
    static Var ScriptReadAll(Var method, CallInfo callInfo, Var _this, ...);
    static Var ScriptRunTarget(Var method, CallInfo callInfo, Var _this, ...);
    static Var ScriptResumeTarget(Var method, CallInfo callInfo, Var _this, ...);
    static Var ScriptPumpMessages(Var method, CallInfo callInfo, Var _this, ...);
    static Var ScriptSetBreakpoint(Var method, CallInfo callInfo, Var _this, ...);
    static Var ScriptEnableFirstChance(Var method, CallInfo callInfo, Var _this, ...);
    static Var ScriptGetEnvironmentVariable(Var method, CallInfo callInfo, Var _this, ...);
    static Var ScriptGetLocalFolderPath(Var method, CallInfo callInfo, Var _this, ...);
    static Var ScriptStartTargetHost(Var method, CallInfo callInfo, Var _this, ...);
    static Var ScriptFileExists(Var method, CallInfo callInfo, Var _this, ...);
    static Var ScriptGetLocation(Var method, CallInfo callInfo, Var _this, ...);
    static Var ScriptGetCallstack(Var method, CallInfo callInfo, Var _this, ...);
    static Var ScriptQuit(Var method, CallInfo callInfo, Var _this, ...);
    static Var ScriptCreateAuthoringEngine(Var method, CallInfo callInfo, Var _this, ...);
    static Var ScriptSetVersion(Var method, CallInfo callInfo, Var _this, ...);
    static Var ScriptGetLocals(Var method, CallInfo callInfo, Var _this, ...);
    static Var ScriptEvaluateExpr(Var method, CallInfo callInfo, Var _this, ...);
    static Var ScriptEditLocalValue(Var method, CallInfo callInfo, Var _this, ...);
#ifdef DIRECT_AUTHOR_TEST
    static Var GetTokenRanges(Var method, CallInfo callInfo, Var _this, ...);
    static Var GetRegions(Var method, CallInfo callInfo, Var _this, ...);
    static Var GetCompletions(Var method, CallInfo callInfo, Var _this, ...);
    static Var GetErrors(Var method, CallInfo callInfo, Var _this, ...);
    static Var GetAst(Var method, CallInfo callInfo, Var _this, ...);
    static Var GetQuickInfo(Var method, CallInfo callInfo, Var _this, ...);
    static Var ProcessCompletionsSession(Var method, CallInfo callInfo, Var _this, ...);
    static Var GetFunctionHelp(Var method, CallInfo callInfo, Var _this, ...);
    static Var SplatterSession(Var method, CallInfo callInfo, Var _this, ...);
    static Var MultipleHostTypeCompletion(Var method, CallInfo callInfo, Var _this, ...);
#endif

public:
    void PumpMessages();
    HRESULT ParseFile(LPCWSTR filename, Var* topFunc);
    HRESULT CreateFunctionOnObject(LPCWSTR funcName, ScriptMethod method, Var obj, Var* funcObj);
    HRESULT CreateFunctionObject(LPCWSTR funcName, ScriptMethod method, Var* funcObj);
    HRESULT SetPropertyOnObject(LPCWSTR propertyName, Var parentObj, Var propertyObj);

public:

    // methods to Create or Get Javascript Objects
    Var CreateNumber(int i);
    Var CreateString(LPCWSTR string);
    Var CreateObject();
    Var GetUndefined();
    void ThrowError(HRESULT hr, LPCWSTR message);

public:

    // Get native data from Javascript Objects
    HRESULT GetVariantFromVar(Var var, VARIANT* variant, ScriptType* type);

public:
    JsGlass();
    ~JsGlass();

    HRESULT Initialize(JsGlassArgs& args);
    HRESULT Run(LPCWSTR rootJSFile);
    HRESULT StartTargetHost(LPCWSTR hostPath, LPCWSTR pdmPath);
    HRESULT EventCallback(LPCWSTR eventJSON);
    HRESULT CreateAuthoringEngine(LPCWSTR hostPath);
};

HRESULT PrivateCoCreate(LPCWSTR strModule, REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID iid, LPVOID* ppunk);

