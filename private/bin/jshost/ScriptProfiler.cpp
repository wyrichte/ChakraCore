#include "stdafx.h"


ScriptProfiler::ScriptProfiler(void)
    : m_refCount(1), m_fProfileOn(false)
{
}


ScriptProfiler::~ScriptProfiler(void)
{
    std::map<PROFILER_TOKEN, ScriptInfo *>::iterator it;

    for (it = m_scriptMetaDataList.begin(); it != m_scriptMetaDataList.end(); ++it)
    {
        delete (*it).second;
    }
}

HRESULT ScriptProfiler::QueryInterface(REFIID riid, void** ppvObject) 
{
    if (riid == _uuidof(IUnknown))
    {
        *ppvObject =  static_cast<IUnknown*>(this);
    }
    else if (riid == _uuidof(IActiveScriptProfilerCallback))
    {
        *ppvObject =  static_cast<IActiveScriptProfilerCallback*>(this);
    }
    else if (riid == _uuidof(IActiveScriptProfilerCallback2))
    {
        *ppvObject =  static_cast<IActiveScriptProfilerCallback2*>(this);
    }
    else if (riid == _uuidof(IActiveScriptProfilerCallback3))
    {
        *ppvObject =  static_cast<IActiveScriptProfilerCallback3*>(this);
    }
    else
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

ULONG ScriptProfiler::AddRef()
{
    return InterlockedIncrement(&m_refCount);
}

ULONG ScriptProfiler::Release()
{
    long currentCount = InterlockedDecrement(&m_refCount);
    if (currentCount == 0)
    {
        delete this;
    }
    return currentCount;
}

STDMETHODIMP ScriptProfiler::Initialize(DWORD dwContext)
{
    dwContext;

    if (m_fProfileOn)
    {
        return E_UNEXPECTED;
    }

    m_fProfileOn = true;
    if (HostConfigFlags::flags.LogProfilerVerbose)
    {
        wprintf(_u("Script Profiler starts\n"));
    }    

    return S_OK;
}

STDMETHODIMP ScriptProfiler::Shutdown(HRESULT /* hrReason */)
{
    if (!m_fProfileOn)
    {
        return E_UNEXPECTED;
    }

    m_fProfileOn = false;

    // Validate the function call info
    ValidateEvents();

    if (HostConfigFlags::flags.LogProfilerVerbose)
    {
        wprintf(_u("Script Profiler ends\n"));
    }
    return S_OK;
}

STDMETHODIMP ScriptProfiler::ScriptCompiled(
                    PROFILER_TOKEN id,
                    PROFILER_SCRIPT_TYPE type,
                    IUnknown *pDocumentContext)
{
    if (!m_fProfileOn)
    {
        return S_OK;
    }
    if(HostConfigFlags::flags.PerformUTF8BoundaryTestIsEnabled && pDocumentContext != nullptr)
    {
        //Just need to make sure an AV doesn't happen.
        WCHAR* text = this->GetText(pDocumentContext);
        if (text != nullptr)
        {
            ::CoTaskMemFree(text);
        }
    }

    auto mapItem = m_scriptMetaDataList.find(id);
    if (mapItem != m_scriptMetaDataList.end()) 
    {
        wprintf(_u("ERROR : Script compiled already sent for the id : %d\n"), id);
        return E_FAIL;
    }

    ScriptInfo *pInfo = new ScriptInfo;
    if (pInfo == NULL)
    {
        return E_OUTOFMEMORY;
    }

    pInfo->scriptId = id;
    m_scriptMetaDataList[id] = pInfo;

    return S_OK;
}

STDMETHODIMP ScriptProfiler::FunctionCompiled(
    PROFILER_TOKEN functionId,
    PROFILER_TOKEN scriptId,
    const WCHAR *pwszFunctionName,
    const WCHAR *pwszFunctionNameHint,
    IUnknown *pDocumentContext)
{
    if (!m_fProfileOn)
    {
        return S_OK;
    }
    auto mapItem = m_scriptMetaDataList.find(scriptId);
    if (mapItem == m_scriptMetaDataList.end()) 
    {
        wprintf(_u("ERROR : Script ID (%d) not found\n"), scriptId);
        return E_FAIL;
    }

    // Few validations

    std::map<PROFILER_TOKEN, std::wstring> *functionsList = &mapItem->second->listOffunctions;
    if (functionsList->find(functionId) != functionsList->end())
    {
        wprintf(_u("ERROR : Function compiled already sent for function id %d\n"), functionId);
        return E_FAIL;
    }

    if (pwszFunctionName == NULL && pwszFunctionNameHint == NULL)
    {
        wprintf(_u("ERROR : No function name passed for function id %d\n"), functionId);
        return E_FAIL;
    }

    functionsList->insert(std::make_pair(functionId, std::wstring(pwszFunctionName)));

    // Get the line/column info.
    HRESULT hr = S_OK;
    if (pDocumentContext != nullptr)
    {
        CComPtr<IDebugDocumentContext> pDebugDocumentContext = nullptr;
        hr = pDocumentContext->QueryInterface(__uuidof(IDebugDocumentContext), reinterpret_cast<void **>(&pDebugDocumentContext));
        if (SUCCEEDED(hr))
        {
            CComPtr<IDebugDocument> pDocument = NULL;
            hr = pDebugDocumentContext->GetDocument(&pDocument);
            if (SUCCEEDED(hr))
            {
                CComPtr<IDebugDocumentText> pDocumentText = NULL;
                hr = pDocument->QueryInterface(__uuidof(IDebugDocumentText), reinterpret_cast<void **>(&pDocumentText));

                if (SUCCEEDED(hr))
                {
                    // Find the character position of this script function in the main source
                    ULONG cCharPosition = 0;
                    ULONG cNumChars = 0;
                    IfFailGo(pDocumentText->GetPositionOfContext(pDebugDocumentContext, &cCharPosition, &cNumChars));

                    // Ask PDM for the line/column offset.
                    ULONG line = 0;
                    ULONG column = 0;
                    hr = pDocumentText->GetLineOfPosition(cCharPosition, &line, &column);
                    if (cNumChars == 0 && hr == E_INVALIDARG) 
                    {
                        hr = S_OK; // handle pdm returning invalid args for empty/white space string
                    }

                    // Ask PDM for the character position.
                    ULONG characterPosition = 0;
                    IfFailGo(pDocumentText->GetPositionOfLine(line, &characterPosition));
                    Assert(characterPosition <= cCharPosition);

                    if (HostConfigFlags::flags.LogLineColumnProfileInfo)
                    {
                        WCHAR buffer[512];

                        // Line/column indexes are 0-based.
                        swprintf_s(buffer, _u("[FunctionCompiled]: Function: %s, Line: %d, Column: %d, Character Position: %d\n"), pwszFunctionName, line + 1, column + 1, characterPosition);
                        m_functionCompiledLog.push_back(buffer);
                    }
                }
            }
        }
    }

Error:
    return hr;
}

bool FunctionCallInfo::operator == (const FunctionCallInfo& other) const
{
    if (this->functionCallType == other.functionCallType)
    {
        switch (functionCallType)
        {
        case SCRIPT_FUNCTION:
            return this->scriptId == other.scriptId && this->functionId == other.functionId;

        case NATIVE_FUNCTION:
            return this->functionName == other.functionName && this->scriptType == other.scriptType;
        }
    }

    return false;
}

bool FunctionCallInfo::operator != (const FunctionCallInfo& other) const
{
    return !(*this == other);
}

STDMETHODIMP ScriptProfiler::OnFunctionEnter(PROFILER_TOKEN scriptId, PROFILER_TOKEN functionId)
{
    if (m_fProfileOn)
    {
        m_functionCallStack.push(FunctionCallInfo(scriptId, functionId));
        CheckFunctionEnter();
    }
    return S_OK;
}

STDMETHODIMP ScriptProfiler::OnFunctionExit(PROFILER_TOKEN scriptId, PROFILER_TOKEN functionId)
{
    if (m_fProfileOn)
    {
        if (m_functionCallStack.empty()
            || m_functionCallStack.top() != FunctionCallInfo(scriptId, functionId))
        {
            wprintf(_u("ERROR : Function Exit event without Enter event\n"));
            return E_FAIL;
        }
        m_functionCallStack.pop();
    }
    return S_OK;
}

STDMETHODIMP ScriptProfiler::OnFunctionEnterByName(const WCHAR *pwszFunctionName, PROFILER_SCRIPT_TYPE type)
{
    if (m_fProfileOn)
    {
        m_functionCallStack.push(FunctionCallInfo(pwszFunctionName, type));
        CheckFunctionEnter();
    }
    return S_OK;
}

STDMETHODIMP ScriptProfiler::OnFunctionExitByName(const WCHAR *pwszFunctionName, PROFILER_SCRIPT_TYPE type)
{
    if (m_fProfileOn)
    {
        if (m_functionCallStack.empty()
            || m_functionCallStack.top() != FunctionCallInfo(pwszFunctionName, type))
        {
            wprintf(_u("ERROR : Function Exit event without Enter event\n"));
            return E_FAIL;
        }
        m_functionCallStack.pop();
    }
    return S_OK;
}

const WCHAR* ScriptProfiler::GetFunctionName(PROFILER_TOKEN scriptId, PROFILER_TOKEN functionId) const
{
    auto scriptIter = m_scriptMetaDataList.find(scriptId);
    if (scriptIter == m_scriptMetaDataList.end())
    {
        wprintf(_u("ERROR : Script ID (%d) not found\n"), scriptId);
        return _u("");
    }

    const std::map<PROFILER_TOKEN, std::wstring>& functionsList = scriptIter->second->listOffunctions;
    auto functionIter = functionsList.find(functionId);
    if (functionIter == functionsList.end())
    {
        wprintf(_u("ERROR : No compile even found for function %d\n"), functionId);
        return _u("");
    }

    return functionIter->second.c_str();
}

WCHAR *ScriptProfiler::GetText(IUnknown *debugDocument)
{
    Assert(debugDocument != nullptr);

    HRESULT hr;
    CComPtr<IDebugDocumentContext> pDocumentContext1;
    hr = debugDocument->QueryInterface(__uuidof(IDebugDocumentContext), (void **) &pDocumentContext1);
    CComPtr<IDebugDocument> pDebugDocument;
    CComPtr<IDebugDocumentText> pDebugDocumentText;
    Assert(SUCCEEDED(hr));

    hr = pDocumentContext1->GetDocument(&pDebugDocument);
    Assert(SUCCEEDED(hr));

    hr = pDebugDocument->QueryInterface(__uuidof(IDebugDocumentText), (void**)&pDebugDocumentText);
    Assert(SUCCEEDED(hr));
    if (pDebugDocumentText)
    {
        ULONG cLine = 0, cNumChar = 0;
        WCHAR *pszSrcText = NULL;
        if (pDebugDocumentText->GetSize(&cLine, &cNumChar) == S_OK)
        {
            // Allocate buffer for source text and attributes
            pszSrcText = (WCHAR *)::CoTaskMemAlloc((ULONG)((cNumChar + 1) * sizeof(WCHAR)));

            if (pszSrcText != NULL)
            {
                pszSrcText[cNumChar] = '\0';
                ULONG cchSrcText = 0;
                hr = pDebugDocumentText->GetText(0, pszSrcText, NULL, &cchSrcText, cNumChar);
            }

            return pszSrcText;
        }
    }

    return nullptr;
}

void ScriptProfiler::CheckFunctionEnter()
{
    // Always try to retrieve function name, which verifies scriptId/functionId
    const WCHAR* functionName = nullptr;
    {
        const FunctionCallInfo& top = m_functionCallStack.top();
        switch (top.functionCallType)
        {
        case SCRIPT_FUNCTION:
            functionName = GetFunctionName(top.scriptId, top.functionId);
            break;

        case NATIVE_FUNCTION:
            functionName = top.functionName.c_str();
            break;
        }
    }

    if (HostConfigFlags::flags.LogProfilerCallTree)
    {
        WCHAR fmt[64], buf[256];
        StringCchPrintf(fmt, _countof(fmt), _u("%%%ds%%s\n"), 2 * static_cast<int>(m_functionCallStack.size()));
        StringCchPrintf(buf, _countof(buf), fmt, _u(""), functionName);
        m_logs.push_back(buf);
    }
}

void ScriptProfiler::ValidateEvents()
{
    if (!m_functionCallStack.empty())
    {
        wprintf(_u("ERROR : Missing %d function exit events!\n"), static_cast<int>(m_functionCallStack.size()));
    }

    if (!m_functionCompiledLog.empty())
    {
        wprintf(_u("Dumping sorted function compiled events\n"));
        m_functionCompiledLog.sort();
        for (auto iter = m_functionCompiledLog.begin(); iter != m_functionCompiledLog.end(); ++iter)
        {
            wprintf(iter->c_str());
        }
    }

    // Flush saved logs if any
    for (auto iter = m_logs.begin(); iter != m_logs.end(); ++iter)
    {
        wprintf(iter->c_str());
    }

    if (HostConfigFlags::flags.LogProfilerVerbose)
    {
        wprintf(_u("Passed: Validation\n"));
    }
}

