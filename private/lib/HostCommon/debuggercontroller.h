#pragma once

#include "dbgeng.h"
#include <vector>

#define JsrtCheckError(x) \
    if((x) != JsNoError) { \
        hr = E_FAIL;      \
        goto Error;       \
    } \

#define EXTERNAL_DATA_PROP _u("#callback#")

enum LocationToStringFlags : ULONG
{
    LTSF_None = 0,
    LTSF_IncludeDocId = 0x1,
    LTSF_IncludeDebugPropertyFlag = 0x2,
    LTSF_IncludeLineCol = 0x4,
    LTSF_All = 0xFFFFFFFF,
};

enum DebugPropertyFlags : ULONG
{
    // NOTE: Keep these flags the same with dbgcontroller.js
    LOCALS_DEFAULT = 0x0,
    // LOCALS_RADIX reserves lower byte 0xFF
    LOCALS_FULLNAME = 0x0100, // Use FullName instead of default short name
    LOCALS_TYPE = 0x0200,
    LOCALS_ATTRIBUTES = 0x0400
};

class Location
{
    std::wstring stringRep;
    std::wstring encodedText;

public:
    ULONG startChar;
    ULONG length;
    UINT64 docId;
    UINT64 srcId;

    // line and column number
    ULONG lineNumber;
    ULONG columnNumber;

    std::wstring text;
    std::wstring frameDescription;
    DWORD debugPropertyAttributes;

    Location() 
        : startChar(0),
        length(0),
        docId(0),
        srcId(0),
        lineNumber(0),
        columnNumber(0),
        text(_u("")),
        frameDescription(_u("")),
        debugPropertyAttributes(0),
        stringRep(_u("")),
        encodedText(_u(""))
    {
    }

    LPCWSTR ToString(LocationToStringFlags flags = LTSF_None);
};

class SourceMap
{
    std::vector<int> m_lineOffsets;
public:
    void Initialize(LPCWSTR text);
    int GetOffset(int line);
    int GetLine(int offset);
    int GetNumLines();
};

struct ControllerConfig
{
    ControllerConfig()
        : setBpEveryNthLine(5), 
        setBpEveryScope(false), 
        globalBpHitCount(0), 
        maxHitCountForABreakpoint(5),
        maxStringLengthToDump(16),
        maxStringLengthToDumpForType(100),
        inspectionNestingLevel(1)
    {
    }

    bool setBpEveryScope;
    ULONG setBpEveryNthLine;
    ULONG maxHitCountForABreakpoint;
    ULONG maxStringLengthToDump;
    ULONG maxStringLengthToDumpForType;
    ULONG inspectionNestingLevel;

    bool ShouldStepInto()
    {
        return (++globalBpHitCount) % 10 == 0;
    }

    BREAKRESUMEACTION ResumeActionOnBreak()
    {
        return ShouldStepInto() ? BREAKRESUMEACTION_STEP_INTO : BREAKRESUMEACTION_CONTINUE;
    }

    ERRORRESUMEACTION GetErrorResumeAction()
    {
        // TODO : make it configurable.
        return ERRORRESUMEACTION_AbortCallAndReturnErrorToCaller;
    }

private:
    int  globalBpHitCount;
};

class ScriptEngineWrapper
{
public:
    ScriptEngineWrapper();
    ~ScriptEngineWrapper();

    // CallGlobalFunction: calls a global script function
    HRESULT CallGlobalFunction(LPCWSTR function, JsValueRef *result, LPCWSTR arg1 = nullptr, LPCWSTR arg2 = nullptr, LPCWSTR arg3 = nullptr);

    // Installs a callback function onto the WScript object
    HRESULT InstallHostCallback(LPCWSTR propName, JsNativeFunction function, void *data);

    static HRESULT GetExternalData(JsValueRef func, void **data);
    
private:
    JsRuntimeHandle m_runtime;
    JsContextRef m_context;
    static JsValueRef __stdcall EchoCallback(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
};

class JsrtValueConverter
{
public:
    JsrtValueConverter(JsValueRef *arguments, unsigned short argCount) : m_args(arguments), m_argCount(argCount), m_currArg(1)
    {
        // m_currArg = 1 due to the this pointer
    };

    template<class T1> HRESULT Convert(T1* arg1) { 
        HRESULT hr = S_OK;
        IfFailedReturn(InternalConvert(arg1)); 
        return hr;
    }
    template<class T1, class T2> HRESULT Convert(T1 *arg1, T2 *arg2) {
        HRESULT hr = S_OK;
        IfFailedReturn(InternalConvert(arg1));
        IfFailedReturn(InternalConvert(arg2));
        return hr;
    }
    template<class T1, class T2, class T3> HRESULT Convert(T1 *arg1, T2 *arg2, T3 *arg3) {
        HRESULT hr = S_OK;
        IfFailedReturn(InternalConvert(arg1));
        IfFailedReturn(InternalConvert(arg2));
        IfFailedReturn(InternalConvert(arg3));
        return hr;
    }

private:
    HRESULT InternalConvert(ULONG *val);
    HRESULT InternalConvert(LPCWSTR *val);
    HRESULT InternalConvert(bool *val);

    JsValueRef *m_args;
    unsigned short m_argCount;
    unsigned short m_currArg;
};

class DebuggerController
{
public:
    DebuggerController(LPCWSTR baselineFilename, LPCWSTR baselinePath);
    ~DebuggerController();

    HRESULT SetBaseline(std::wstring const& fullPath);
    HRESULT LogBreakpoint(__in __nullterminated WCHAR const* reason);
    HRESULT LogLocals(__in __nullterminated WCHAR const* locals);
    HRESULT LogCallstack(__in __nullterminated WCHAR const* callstack);
    HRESULT LogSetNextStatement(__in __nullterminated WCHAR const* description);
    HRESULT DumpLog();
    HRESULT LogEvaluateExpression(__in __nullterminated WCHAR const* expression);
    HRESULT LogMessage(__in __nullterminated WCHAR const* message);
    HRESULT LogJson(__in __nullterminated WCHAR const* logString);

    HRESULT AddSourceFile(LPCWSTR text, ULONG srcId);
    HRESULT HandleBreakpoint(LONG bpId);
    HRESULT HandleException();
    HRESULT ResetBpMap();
    HRESULT HandleMutationBreakpoint();
    HRESULT HandleDOMMutationBreakpoint();

    HRESULT VerifyAndWriteNewBaselineFile(std::wstring const& filename);

    static JsValueRef GetJavascriptUndefined();
    static JsValueRef ConvertDoubleToNumber(double val);

    static void Log(__in __nullterminated char16 *msg, ...);
    static void LogError(__in __nullterminated char16 *msg, ...);
    static LPCWSTR EncodeString(_In_reads_z_(len) LPCWSTR str, _In_ size_t len, std::wstring& encodedStr);
    static LPCWSTR EncodeString(const std::wstring& str, std::wstring& encodedStr);
    static LPCWSTR EncodeString(BSTR str, std::wstring& encodedStr);
    static UINT GetRadix(DebugPropertyFlags flags);

    static LPCWSTR GetBreakpointReason(BREAKREASON reason);
    static LPCWSTR GetBreakResumeAction(BREAKRESUMEACTION resumeAction);
    static LPCWSTR GetErrorResumeAction(ERRORRESUMEACTION errorAction);
    static LPCWSTR GetBreakpointState(BREAKPOINT_STATE state);
    static LPCWSTR s_largeString;

    HRESULT InstallHostCallback(LPCWSTR propName, JsNativeFunction function, void *data);

    static UINT64 GetDocumentIdStartOffset();

    static void AppendDebugPropertyAttributesToString(std::wstring& stringRep, DWORD debugPropertyAttributes, bool prefixSeparator = true);

    //
    // Dump one DebugProperty to JSON
    //
    template <class Debugger, class DebugProperty>
    HRESULT DumpProperty(Debugger& debugger, const DebugProperty& debugProperty, int expandLevel, DebugPropertyFlags flags, std::wstring& json,
        _Inout_opt_ int* pScopeId = nullptr, _Inout_opt_ std::map<std::wstring, int>* mapPropertyName = nullptr, bool first = true)
    {
        return debugger.MapPropertyInfo(debugProperty, flags, [&](const Debugger::AutoDebugPropertyInfo& info)
        {
            HRESULT hr = S_OK;

            std::wstring encodedName;
            EncodeString(info.Name(!!(flags & DebugPropertyFlags::LOCALS_FULLNAME)), encodedName);

            // Since there can be multiple scopes at the same level, while building JSON out of all scopes value will be trimmed since they have same name
            // Appending some ID, so that they will remain and tested.
            {
                std::wstring nameSuffix;
                int id = -1;
                if (pScopeId && encodedName == _u("[Scope]"))
                {
                    id = (*pScopeId)++;
                }
                else if (mapPropertyName)
                {
                    auto pair = mapPropertyName->insert(std::pair<std::wstring, int>(encodedName, 1));
                    if (!pair.second)
                    {
                        id = pair.first->second++; // Found it in map take the count and append 1 to it
                        nameSuffix += _u("#");
                    }
                }

                if (id >= 0)
                {
                    char16 buf[10];
                    _itow_s(id, buf, 10);
                    nameSuffix += buf;
                    encodedName += nameSuffix; // After this point encodedName includes suffix!
                }
            }

            if (!first)
            {
                json += _u(",");
            }

            if ((flags & DebugPropertyFlags::LOCALS_TYPE) && ::SysStringLen(info.Type()) > 0) // Skip if no Type (e.g. [Methods], [Scope]...) to reduce noise
            {
                std::wstring encodedType;
                EncodeString(info.Type(), encodedType);
                json += _u("\"") + encodedName + _u(" - [Type]\": \"") + encodedType + _u("\", ");
            }

            json += _u("\"") + encodedName + _u("\": ");
            if (expandLevel > 0 && info.IsExpandable())
            {
                json += _u("{");
                IfFailGo(DumpAllProperties(debugger, debugProperty, expandLevel - 1, flags, json));
                json += _u("}");
            }
            else
            {
                UINT len = ::SysStringLen(info.Value());
                if (len < debugger.GetControllerConfig().maxStringLengthToDump)
                {
                    UINT begin = 0;
                    if (_wcsicmp(info.Type(), _u("String")) == 0)
                    {
                        // We have to escape the contents of this string.  This requires
                        // removing the initial and terminating double-quote first.
                        begin++;
                        len -= 2;
                    }

                    std::wstring encodedValue;
                    EncodeString(info.Value() + begin, len, encodedValue);
                    json += _u("\"") + encodedValue + _u("\"");
                }
                else
                {                        
                    json += s_largeString; // The string was too large, put in a placeholder.
                }
            }

            if (flags & DebugPropertyFlags::LOCALS_ATTRIBUTES)
            {
                json += _u(", \"") + encodedName + _u(" - [Attributes]\": {");
                AppendDebugPropertyAttributesToString(json, info.Attr(), /*prefixSeparator*/false);
                json += _u("}");
            }

        Error:
            return hr;
        });
    }

    //
    // Dump the members of one DebugProperty to JSON
    //
    template <class Debugger, class DebugProperty>
    HRESULT DumpAllProperties(Debugger& debugger, const DebugProperty& debugProperty, int expandLevel, DebugPropertyFlags flags, std::wstring& json)
    {
        int scopeId = 1;
        std::map<std::wstring, int> mapPropertyName;
        bool first = true;

        return debugger.EnumDebugProperties(debugProperty, flags, [&](const DebugProperty& member)
        {
            HRESULT hr = DumpProperty(debugger, member, expandLevel, flags, json, &scopeId, &mapPropertyName, first);
            first = false;
            return hr;
        });
    }

    template <class Debugger, class DebugProperty, class Func>
    HRESULT DumpLocals(Debugger& debugger, const DebugProperty& root, int expandLevel, DebugPropertyFlags flags, const Func& preDump)
    {
        HRESULT hr = S_OK;

        std::wstring json = _u("{\"locals\" : {");
        IfFailGo(preDump(json));
        IfFailGo(DumpAllProperties(debugger, root, expandLevel, flags, json));
        json += _u("}}");

        IfFailGo(LogLocals(json.c_str()));

    Error:
        return hr;
    }

    template <class Debugger, class DebugProperty>
    HRESULT DumpLocals(Debugger& debugger, const DebugProperty& root, int expandLevel, DebugPropertyFlags flags)
    {
        return DumpLocals(debugger, root, expandLevel, flags, [](std::wstring&) { return S_OK; });
    }

private:
    std::wstring            m_baselineFilename;
    std::wstring            m_baselinePath;
    ScriptEngineWrapper*    m_scriptWrapper;
};