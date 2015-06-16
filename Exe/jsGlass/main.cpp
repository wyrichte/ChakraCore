/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"
#include "ieisos.h"

// The javascript portion of glass is 'imported' by escaping the quote and backslashes
// and wrapping each line as a string then #including it here
WCHAR defaultScript[] =
#include "jsglass.js.encoded"
;


typedef HRESULT (STDAPICALLTYPE* FN_DllGetClassObject)(REFCLSID,REFIID, LPVOID*);

HRESULT PrivateCoCreate(
    LPCWSTR strModule,
    REFCLSID rclsid,
    LPUNKNOWN pUnkOuter,
    DWORD dwClsContext,
    REFIID iid,
    LPVOID* ppunk
    )
{
    HRESULT hr = NOERROR;
    HINSTANCE hInstance = NULL;
    CComPtr <IClassFactory> pClassFactory;
    FN_DllGetClassObject pProc = NULL;

    hInstance = LoadLibraryEx(strModule, NULL, 0);

    IfNullGo(hInstance, E_FAIL);
    IfNullGo(pProc = (FN_DllGetClassObject) GetProcAddress(hInstance, "DllGetClassObject"), E_FAIL);
    IfFailGo(pProc(rclsid, __uuidof(IClassFactory), (LPVOID*)&pClassFactory));
    IfFailGo(pClassFactory->CreateInstance(pUnkOuter, iid, ppunk));
Error:
    return hr;
}

//copied from scriptcontext loadfile ...
LPWSTR FileSystem::GetContents(FILE* file)
{
    LPWSTR contents;

    //
    // Determine the file length, in bytes.
    //

    fseek(file, 0, SEEK_END);
    unsigned int lengthBytes = ftell(file);
    fseek(file, 0, SEEK_SET);
    LPWSTR contentsRaw = (LPWSTR) calloc(lengthBytes + 2, 1);
    if (NULL == contentsRaw)
    {
        return NULL;
    }

    //
    // Read the entire content as a binary block.
    //

    fread((void*) contentsRaw, sizeof(char), lengthBytes, file);

    //
    // Read encoding, handling any conversion to Unicode.
    //
    // Warning: The UNICODE buffer for parsing is supossed to be provided by the host.
    // this is temporary code to read from Unicode and ANSI files.
    // It is not a complete read of the encoding. Some encodings like UTF7, UTF1, EBCDIC, SCSU, BOCU could be
    // wrongly classified as ANSI
    //

    byte * pRawBytes = (byte*)contentsRaw;
    if( (0xEF == *pRawBytes && 0xBB == *(pRawBytes+1) && 0xBF == *(pRawBytes+2)) ||
        0xFFFE == *contentsRaw ||
        0x0000 == *contentsRaw && 0xFEFF == *(contentsRaw+1) )
    {
        free((void*)contentsRaw);
        return NULL;
    }
    else if (0xFEFF == *contentsRaw)
    {
        // unicode LE
        contents = contentsRaw;
    }
    else
    {
        //assume ANSI
        // note: some other encodings might fall wrongly in this category

        unsigned int cAnsiChars = lengthBytes + 1;
        contents = (LPWSTR) calloc(cAnsiChars, sizeof(wchar_t));
        if (NULL == contents)
            wprintf(L"out of memory");

        // Covert to Unicode.
        if (0 == MultiByteToWideChar(CP_ACP, 0, (LPCSTR)contentsRaw, cAnsiChars,
            (LPWSTR)contents, cAnsiChars))
        {
            free((void*)contents);
            free((void*)contentsRaw);
            wprintf(L"failed MultiByteToWideChar conversion");
            return NULL;
        }
        free((void*)contentsRaw);
    }
    return contents;
}

HRESULT JsGlass::ParseFile(LPCWSTR filename, Var* topFunc)
{
    FILE * file;

    if(_wfopen_s(&file, filename, L"rb"))
    {
        return E_FAIL;
    }

    LPWSTR contents = FileSystem::GetContents(file);
    fclose(file);
    if (!contents)
    {
        return E_FAIL;
    }

    HRESULT hr = _scriptDirect->Parse(contents, topFunc);
    free((void*) contents);
    return hr;
}

Var JsGlass::ScriptReadLine(Var method, CallInfo callInfo, Var _this,...)
{
    JS_ARGS

    WCHAR buf[4096];
    fgetws(buf,4096,stdin);
    size_t len = wcsnlen(buf,4096);
    //strip out the return char
    buf[len-1] = L'\0';

    return that->CreateString(buf);
}

static FILE* GetStream(LPCWSTR streamName)
{
    if (0 == wcscmp(streamName,L"stdOut"))
    {
        return stdout;
    }
    else if (0== wcscmp(streamName,L"stdErr"))
    {
        return stderr;
    }

    return stdout;
}

Var JsGlass::ScriptWrite(Var method, CallInfo callInfo, Var _this,...)
{
    HRESULT hr = S_OK;
    JS_ARGS

    CComVariant varOut;
    CComVariant varStream;
    FILE* stream;

    IfFailGo( that->GetVariantFromVar(args[0], &varOut, NULL) );
    that->GetVariantFromVar(args[1], &varStream, NULL);

    OpenedFile* file = that->_fileSys.GetFile(args[1]);
    if (file)
    {
        stream = file->_file;
    }
    else
    {
        stream = GetStream(varStream.bstrVal);
    }
    fprintf(stream,"%S",varOut.bstrVal);
    fflush(stream);

Error:

    return that->GetUndefined();
}

Var JsGlass::ScriptArgumentItem(Var method, CallInfo callInfo, Var _this,...)
{
    JS_ARGS

    CComVariant variant;
    ScriptType type;

    if (S_OK != that->GetVariantFromVar(args[0],&variant,&type))
    {
        // TODO Error handling?
        return NULL;
    }
    if (type != ScriptType_Int)
    {
        return NULL;
    }
    int index = variant.intVal+1; // +1 to account for skipping past jsGlass.exe

    return that->CreateString(that->_argv[index]);
}

Var JsGlass::ScriptOpenTarget(Var method, CallInfo callInfo, Var _this,...)
{
    JS_ARGS

    CComVariant variantScript;
    if (S_OK == that->GetVariantFromVar(args[0], &variantScript, NULL))
    {
        CComVariant variantFileName;
        LPCWSTR filename = NULL;
        if (callInfo.Count == 3)
        {
            if (S_OK == that->GetVariantFromVar(args[1], &variantFileName, NULL))
            {
                filename = variantFileName.bstrVal;
            }
        }
        that->_targetHost->AddScript(variantScript.bstrVal,filename);
    }
    return that->GetUndefined();
}

#ifdef DIRECT_AUTHOR_TEST
Var JsGlass::GetTokenRanges(Var method, CallInfo callInfo, Var _this, ...)
{
    JS_ARGS

    CComVariant variantScript;
    if (S_OK == that->GetVariantFromVar(args[0], &variantScript, NULL))
    {
        that->_authoringHost->GetTokenRanges(that->_authorServices, variantScript.bstrVal);
    }
    return that->GetUndefined();
}

Var JsGlass::GetRegions(Var method, CallInfo callInfo, Var _this, ...)
{
    JS_ARGS

    CComVariant variantScript;
    if (S_OK == that->GetVariantFromVar(args[0], &variantScript, NULL))
    {
        that->_authoringHost->GetRegions(that->_authorServices, variantScript.bstrVal);
    }
    return that->GetUndefined();
}

Var JsGlass::GetCompletions(Var method, CallInfo callInfo, Var _this, ...)
{
    JS_ARGS

    LPCWSTR strs[20];
    int count = callInfo.Count - 2;
    if (count > sizeof(strs)/sizeof(strs[0]))
        count = sizeof(strs)/sizeof(strs[0]);

    CComVariant variantScript;
    that->GetVariantFromVar(args[0], &variantScript, NULL);

    if (variantScript.vt == VT_I4)
    {
        count = count < variantScript.lVal ? count : variantScript.lVal;
        for (int i = 0; i < count; i++)
        {
            if ((S_OK == that->GetVariantFromVar(args[i + 1], &variantScript, NULL)) && (variantScript.vt == VT_BSTR))
                strs[i] = variantScript.bstrVal;
            else
                strs[i] = NULL;
        }
    }

    that->_authoringHost->GetCompletions(that->_authorServices, count, strs);
    return that->GetUndefined();
}

Var JsGlass::GetErrors(Var method, CallInfo callInfo, Var _this, ...)
{
    JS_ARGS

    CComVariant variantScript;
    if (S_OK == that->GetVariantFromVar(args[0], &variantScript, NULL))
    {
        that->_authoringHost->GetErrors(that->_authorServices, variantScript.bstrVal);
    }
    return that->GetUndefined();
}

Var JsGlass::GetAst(Var method, CallInfo callInfo, Var _this, ...)
{
    JS_ARGS

    CComVariant variantScript;
    if (S_OK == that->GetVariantFromVar(args[0], &variantScript, NULL))
    {
        that->_authoringHost->GetAst(that->_authorServices, variantScript.bstrVal);
    }
    return that->GetUndefined();
}

Var JsGlass::GetFunctionHelp(Var method, CallInfo callInfo, Var _this, ...)
{
    JS_ARGS

    LPCWSTR strs[20];
    int count = callInfo.Count - 2;
    if (count > sizeof(strs)/sizeof(strs[0]))
        count = sizeof(strs)/sizeof(strs[0]);

    CComVariant variantScript;
    that->GetVariantFromVar(args[0], &variantScript, NULL);

    if (variantScript.vt == VT_I4)
    {
        count = count < variantScript.lVal ? count : variantScript.lVal;
        for (int i = 0; i < count; i++)
        {
            if ((S_OK == that->GetVariantFromVar(args[i + 1], &variantScript, NULL)) && (variantScript.vt == VT_BSTR))
                strs[i] = variantScript.bstrVal;
            else
                strs[i] = NULL;
        }
    }

    that->_authoringHost->GetFunctionHelp(that->_authorServices, count, strs);

    return that->GetUndefined();
}

Var JsGlass::GetQuickInfo(Var method, CallInfo callInfo, Var _this, ...)
{
    JS_ARGS

    CComVariant variantScript;
    if (S_OK == that->GetVariantFromVar(args[0], &variantScript, NULL))
    {
        that->_authoringHost->GetQuickInfo(that->_authorServices, variantScript.bstrVal);
    }
    return that->GetUndefined();
}

Var JsGlass::ProcessCompletionsSession(Var method, CallInfo callInfo, Var _this, ...)
{
    JS_ARGS

    CComVariant variantScript;
    CComVariant defaultPath;
    if (S_OK == that->GetVariantFromVar(args[0], &variantScript, NULL))
    {
        if (S_OK == that->GetVariantFromVar(args[1], &defaultPath, NULL))
        {
            that->_authoringHost->ProcessCompletionsSession(that->_authorServices, variantScript.bstrVal, defaultPath.bstrVal);
        }
    }
    return that->GetUndefined();
}

Var JsGlass::SplatterSession(Var method, CallInfo callInfo, Var _this, ...)
{
    JS_ARGS

    CComVariant variantScript;
    if (S_OK == that->GetVariantFromVar(args[0], &variantScript, NULL))
    {
        that->_authoringHost->SplatterSession(that->_authorServices, variantScript.bstrVal);
    }
    return that->GetUndefined();
}

Var JsGlass::MultipleHostTypeCompletion(Var method, CallInfo callInfo, Var _this, ...)
{
     JS_ARGS

    LPCWSTR strs[20];
    int count = callInfo.Count - 2;
    if (count > sizeof(strs)/sizeof(strs[0]))
        count = sizeof(strs)/sizeof(strs[0]);

    CComVariant variantScript;
    that->GetVariantFromVar(args[0], &variantScript, NULL);

    if (variantScript.vt == VT_I4)
    {
        count = count < variantScript.lVal ? count : variantScript.lVal;
        for (int i = 0; i < count; i++)
        {
            if ((S_OK == that->GetVariantFromVar(args[i + 1], &variantScript, NULL)) && (variantScript.vt == VT_BSTR))
                strs[i] = variantScript.bstrVal;
            else
                strs[i] = NULL;
        }
    }

    that->_authoringHost->MultipleHostTypeCompletion(that->_authorServices, count, strs);
    return that->GetUndefined();
}
#endif

Var JsGlass::ScriptRunTarget(Var method, CallInfo callInfo, Var _this,...)
{
    JS_ARGS

    that->_targetHost->RunPendingScripts();

    return that->GetUndefined();
}

Var JsGlass::ScriptResumeTarget(Var method, CallInfo callInfo, Var _this,...)
{
    HRESULT hr = S_OK;
    JS_ARGS

    CComVariant variantString;

    BOOL fWait = FALSE;

    IfFailGo( that->GetVariantFromVar(args[0], &variantString, NULL) );
    if (0 == _wcsicmp(variantString.bstrVal, L"continue"))
    {
        if (callInfo.Count > 2)
        {
            CComVariant variantWaitStr;
            if (that->GetVariantFromVar(args[1], &variantWaitStr, NULL) == S_OK)
            {
                fWait = (0 == _wcsicmp(variantWaitStr.bstrVal, L"wait")) ? TRUE : FALSE;
            }
        }
        that->_debugger->Resume(BREAKRESUMEACTION_CONTINUE);
    }
    else if (0 == _wcsicmp(variantString.bstrVal, L"in"))
    {
        that->_debugger->Resume(BREAKRESUMEACTION_STEP_INTO);
    }
    else if (0 == _wcsicmp(variantString.bstrVal, L"over"))
    {
        that->_debugger->Resume(BREAKRESUMEACTION_STEP_OVER);
    }
    else if (0 == _wcsicmp(variantString.bstrVal, L"out"))
    {
        that->_debugger->Resume(BREAKRESUMEACTION_STEP_OUT);
    }
    else if (0 == _wcsicmp(variantString.bstrVal, L"document"))
    {
        AssertMsg(false, "document stepping not supported");
        goto Error;
    }

    if (fWait)
    {
        that->_targetHost->ADummyBlockCall();
    }

Error:

    return that->GetUndefined();
}

Var JsGlass::ScriptOpenTextFile(Var method, CallInfo callInfo, Var _this,...)
{
    HRESULT hr = S_OK;
    JS_ARGS

    CComVariant variantFile;
    CComVariant variantFlags;
    ScriptType type;
    Var result = that->GetUndefined();

    IfFailGo( that->GetVariantFromVar(args[0], &variantFile, &type) );
    AssertMsg(type == ScriptType_String);

    IfFailGo( that->GetVariantFromVar(args[1], &variantFlags, &type) );
    AssertMsg(type == ScriptType_String);

    result = that->_fileSys.OpenTextFile(variantFile.bstrVal, variantFlags.bstrVal, that);

Error:
    // should probably throw in error case instead of returning undefined...
    return result;
}

Var JsGlass::ScriptSetBreakpoint(Var method, CallInfo callInfo, Var _this,...)
{
    HRESULT hr = S_OK;
    JS_ARGS
    CComVariant variantString;
    CComVariant variantInt;
    ScriptType type;

    IfFailGo( that->GetVariantFromVar(args[0], &variantString, &type) );
    AssertMsg(type == ScriptType_String, "Bad 1st arg to SetBreakpoint");

    IfFailGo( that->GetVariantFromVar(args[1], &variantInt, &type) );
    if (type == ScriptType_String)
    {
        int i = _wtoi(variantInt.bstrVal);
        variantInt = i;
    }
    else
    {
        AssertMsg(type == ScriptType_Int, "Bad 2nd arg to SetBreakpoint");
    }

    IfFailGo( that->_targetHost->SetBreakpoint(variantString.bstrVal, variantInt.intVal) );

Error:

    return that->GetUndefined();
}

Var JsGlass::ScriptEnableFirstChance(Var method, CallInfo callInfo, Var _this,...)
{
    HRESULT hr = S_OK;
    JS_ARGS
    CComVariant variantBool;
    ScriptType type;
    BOOL fEnable = FALSE;

    IfFailGo( that->GetVariantFromVar(args[0], &variantBool, &type) );
    if (type == ScriptType_Bool)
    {
        fEnable = (variantBool.boolVal == VARIANT_TRUE) ? TRUE : FALSE;
    }

    IfFailGo( that->_targetHost->EnableFirstChanceException(fEnable) );

Error:

    return that->GetUndefined();
}

Var JsGlass::ScriptGetLocation(Var method, CallInfo callInfo, Var _this,...)
{
    HRESULT hr = S_OK;
    JS_ARGS

    IfFailGo( that->_debugger->GetLocation() );

Error:

    return that->GetUndefined();
}

Var JsGlass::ScriptGetCallstack(Var method, CallInfo callInfo, Var _this,...)
{
    HRESULT hr = S_OK;
    JS_ARGS

    IfFailGo( that->_debugger->GetCallstack() );

Error:

    return that->GetUndefined();
}

void JsGlass::PumpMessages()
{
    MSG msg;
    while (PeekMessage( &msg, (HWND)-1 /*only thread msgs*/, 0,0,PM_NOREMOVE))
    {
        GetMessage(&msg, NULL, 0, 0);

        if (Message<JsGlass>::M_HANDLED == Message<JsGlass>::TryDispatch(msg))
        {
            continue;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

Var JsGlass::ScriptPumpMessages(Var method, CallInfo callInfo, Var _this,...)
{
    that->PumpMessages();
    return that->GetUndefined();
}

Var JsGlass::ScriptGetLocalFolderPath(Var method, CallInfo callInfo, Var _this, ...)
{
    WCHAR pathBuffer[_MAX_PATH];
    Var result = that->GetUndefined();

    GetModuleFileName(NULL, pathBuffer, _MAX_PATH);
    size_t pathLen = wcslen(pathBuffer);
    size_t localLen = wcslen(L".local");

    if (pathLen+localLen+1 < _MAX_PATH)
    {
        wcsncpy_s(pathBuffer+pathLen, _MAX_PATH-pathLen, L".local", localLen);
        if (_waccess_s(pathBuffer, 0) == 0)
        {
            result = that->CreateString(pathBuffer);
        }
    }

    return result;
}

Var JsGlass::ScriptGetEnvironmentVariable(Var method, CallInfo callInfo, Var _this,...)
{
    HRESULT hr = S_OK;
    WCHAR* buf = NULL;
    Var result = that->GetUndefined();
    CComVariant variantString;
    ScriptType type;
    JS_ARGS

    IfFailGo( that->GetVariantFromVar(args[0], &variantString, &type) );
    AssertMsg(type == ScriptType_String, "Bad 1st arg to GetEnvironmentVariable");
    DWORD bufSize = GetEnvironmentVariable(variantString.bstrVal, NULL, 0) + 1;
    buf = new WCHAR[bufSize];
    if (GetEnvironmentVariable(variantString.bstrVal,buf,bufSize))
    {
        result = that->CreateString(buf);
    }

Error:
    if (buf)
    {
        delete [] buf;
    }
    return result;
}

Var JsGlass::ScriptCreateAuthoringEngine(Var method, CallInfo callInfo, Var _this,...)
{
    HRESULT hr = S_OK;
    CComVariant variantStringEngine;
    ScriptType type;
    JS_ARGS

    IfFailGo( that->GetVariantFromVar(args[0], &variantStringEngine, &type) );
    IfFailGo( that->CreateAuthoringEngine(variantStringEngine.bstrVal) );
    return that->GetUndefined();

Error:
    that->ThrowError(hr, L"ScriptCreateAuthoringEngine failed");
    return that->GetUndefined();
}

Var JsGlass::ScriptGetLocals(Var method, CallInfo callInfo, Var _this, ...)
{
    HRESULT hr = S_OK;
    CComVariant variantInt;
    ScriptType type;
    JS_ARGS

    IfFailGo( that->GetVariantFromVar(args[0], &variantInt, &type) );
    if (type == ScriptType_String)
    {
        int i = _wtoi(variantInt.bstrVal);
        variantInt = i;
    }
    else
    {
        AssertMsg(type == ScriptType_Int, "Bad 2nd arg to SetBreakpoint");
    }

    IfFailGo( that->_debugger->GetLocals(variantInt.intVal) );

Error:

    return that->GetUndefined();
}

Var JsGlass::ScriptEvaluateExpr(Var method, CallInfo callInfo, Var _this, ...)
{
    HRESULT hr = S_OK;
    CComVariant variantInt;
    ScriptType type;
    JS_ARGS

    CComVariant variantString;
    CComVariant variantBool;

    IfFailGo( that->GetVariantFromVar(args[0], &variantString, NULL) );
    IfFailGo( that->GetVariantFromVar(args[1], &variantInt, &type) );
    if (type == ScriptType_String)
    {
        int i = _wtoi(variantInt.bstrVal);
        variantInt = i;
    }
    else
    {
        AssertMsg(type == ScriptType_Int, "Bad 2nd arg to SetBreakpoint");
    }

    IfFailGo( that->GetVariantFromVar(args[2], &variantBool, &type) );

    VARIANT_BOOL checkOrder = VARIANT_FALSE;
    if (type == ScriptType_Bool)
    {
        checkOrder = variantBool.boolVal;
    }

    IfFailGo( that->_debugger->EvaluateExpr(variantString.bstrVal, variantInt.intVal, checkOrder) );

Error:

    return that->GetUndefined();
}

Var JsGlass::ScriptEditLocalValue(Var method, CallInfo callInfo, Var _this, ...)
{
    HRESULT hr = S_OK;
    CComVariant variantStringLocalRoot;
    CComVariant variantStringLocalChild;
    CComVariant variantStringValue;
    ScriptType type;
    JS_ARGS

    CComVariant variantString;

    IfFailGo( that->GetVariantFromVar(args[0], &variantStringLocalRoot, &type) );
    AssertMsg(type == ScriptType_String, "Bad 1st arg to EditLocalValue");

    IfFailGo( that->GetVariantFromVar(args[1], &variantStringLocalChild, &type) );
    AssertMsg(type == ScriptType_String, "Bad 1st arg to EditLocalValue");

    IfFailGo( that->GetVariantFromVar(args[2], &variantStringValue, &type) );
    AssertMsg(type == ScriptType_String, "Bad 2nd arg to EditLocalValue");

    IfFailGo( that->_debugger->EditLocalValue(variantStringLocalRoot.bstrVal, variantStringLocalChild.bstrVal, variantStringValue.bstrVal) );

Error:

    return that->GetUndefined();
}

Var JsGlass::ScriptSetVersion(Var method, CallInfo callInfo, Var _this, ...)
{
    HRESULT hr = S_OK;
    CComVariant variantNumberEngine;
    ScriptType type;
    JS_ARGS

    IfFailGo( that->GetVariantFromVar(args[0], &variantNumberEngine, &type) );
    if (type == ScriptType_Int)
    {

    }

Error:
    return that->GetUndefined();
}

Var JsGlass::ScriptStartTargetHost(Var method, CallInfo callInfo, Var _this,...)
{
    HRESULT hr = S_OK;
    CComVariant variantStringEngine;
    CComVariant variantStringPDM;
    ScriptType type;
    JS_ARGS

    IfFailGo( that->GetVariantFromVar(args[0], &variantStringEngine, &type) );
    IfFailGo( that->GetVariantFromVar(args[1], &variantStringPDM, &type) );
    IfFailGo( that->StartTargetHost(variantStringEngine.bstrVal, variantStringPDM.bstrVal) );
    return that->GetUndefined();

Error:
    that->ThrowError(hr, L"ScriptStartTargetHost failed");
    return that->GetUndefined();
}

Var JsGlass::ScriptReadAll(Var method, CallInfo callInfo, Var _this,...)
{
    JS_ARGS

    return that->_fileSys.ReadAll(that, _this);
}

Var JsGlass::ScriptFileExists(Var method, CallInfo callInfo, Var _this,...)
{
    HRESULT hr = S_OK;
    CComVariant variantStringEngine;
    ScriptType type;
    FILE* file = NULL;
    Var result = that->GetUndefined();
    JS_ARGS

    IfFailGo( that->GetVariantFromVar(args[0], &variantStringEngine, &type) );
    if (0 == _wfopen_s(&file, variantStringEngine.bstrVal, L"rb"))
    {
        // Use 'truthy' string name as True return value
        result = args[0];
    }

Error:

    if (file)
    {
        fclose(file);
    }

    return result;
}

Var JsGlass::ScriptQuit(Var method, CallInfo callInfo, Var _this,...)
{
    if (that->_debugger)
    {
        that->_debugger->Resume(BREAKRESUMEACTION_ABORT);
    }
    if (that->_targetHost)
    {
        that->_targetHost->Quit();
    }
    return that->GetUndefined();
}

Var FileSystem::ReadAll(JsGlass* jsGlass, Var filestream)
{
    OpenedFile* file = GetFile(filestream);
    if (!file)
    {
        return jsGlass->GetUndefined();
    }
    return file->ReadAll(jsGlass);
}


//////////////////////////
//START EGREGIOUS FILESYS HACK-A-MA-THON

OpenedFile::OpenedFile()
    :_filename(NULL),
     _file(NULL),
     _fileStream(NULL),
     _fileContents(NULL)
{
}

OpenedFile::~OpenedFile()
{
    if (_filename)
    {
        delete _filename;
    }
    if (_file)
    {
        fclose(_file);
    }
}

Var OpenedFile::ReadAll(JsGlass* jsGlass)
{
    // Read all of File into a varString
    if (_fileContents)
    {
        return _fileContents;
    }
    LPWSTR contents = FileSystem::GetContents(_file);
    _fileContents = jsGlass->CreateString(contents);
    return _fileContents;
}

FileSystem::FileSystem()
    :_fileCount(0)
{
}

OpenedFile* FileSystem::GetFile(Var fileStream)
{
    for(int i = 0; i < _fileCount; i++)
    {
        // I'm identifying the files by the ReadAllFunction object I placed on them in OpenTextFile
        // That is why this is called "GetFileHack" rather than GetFile
        if (_files[i]._fileStream == fileStream)
        {
            return _files+i;
        }
    }
    return NULL;
}

Var FileSystem::OpenTextFile(LPCWSTR filename, LPCWSTR openFlags, JsGlass* jsGlass)
{
    HRESULT hr = S_OK;
    OpenedFile* file = &_files[_fileCount];

    if(_wfopen_s(&(file->_file), filename, openFlags))
    {
        return jsGlass->GetUndefined();
    }

    file->_fileStream = jsGlass->CreateObject();
    IfFailGo( jsGlass->CreateFunctionOnObject(L"ReadAll", JsGlass::ScriptReadAll, file->_fileStream, &(file->_fileReadAllFunction)) );

    _fileCount++;

Error:

    return file->_fileStream;
}

// End Egregious FILESYS hack
//////////////////////////////

HRESULT JsGlass::GetVariantFromVar(Var var, VARIANT* variant, ScriptType* type)
{
    ScriptType scriptType;
    if (S_OK != _scriptDirect->GetScriptType(var, &scriptType))
    {
        return S_FALSE;
    }
    if (type)
    {
        *type = scriptType;
    }
    VARTYPE vtType;

    switch (scriptType)
    {
    case ScriptType_String:
    case ScriptType_RegularExpression:
        vtType = VT_BSTR;
        break;
    case ScriptType_Number:
        vtType = VT_R8;
        break;
    case ScriptType_Int:
        vtType = VT_I4;
        break;
    case ScriptType_Bool:
        vtType = VT_BOOL;
        break;
    default:
        return S_FALSE;
    }

    if (S_OK != _scriptDirect->ChangeTypeFromVar(var, vtType, variant))
    {
        return S_FALSE;
    }
    return S_OK;
}

Var JsGlass::CreateNumber(int i)
{
    CComVariant variantNum = i;
    Var varNumber;
    if (S_OK == _scriptDirect->ChangeTypeToVar(&variantNum, &varNumber))
    {
        return varNumber;
    }
    return GetUndefined();
}

Var JsGlass::CreateString(LPCWSTR string)
{
    CComVariant variantString = string;
    Var varString;
    if (S_OK == _scriptDirect->ChangeTypeToVar(&variantString, &varString))
    {
        return varString;
    }
    return GetUndefined();
}

Var JsGlass::GetUndefined()
{
    Var undefined;
    if (S_OK == _scriptDirect->GetUndefined(&undefined))
    {
        return undefined;
    }
    return NULL;
}


void JsGlass::ThrowError(HRESULT hr, LPCWSTR message)
{
    Var errorObject;
    if (S_OK == _scriptDirect->CreateErrorObject(CustomError, hr, message, &errorObject))
    {
        CComPtr<IJavascriptOperations> jscriptOperations;
        IfFailGo( _scriptDirect->GetJavascriptOperations(&jscriptOperations));
        IfFailGo(jscriptOperations->ThrowException(_scriptDirect, errorObject, false));
    }
Error:
    // Should never be here
    return;
}

Var JsGlass::CreateObject()
{
    Var newObj;
    if (S_OK == _scriptDirect->CreateObject(&newObj))
    {
        return newObj;
    }
    return NULL;
}

HRESULT JsGlass::CreateFunctionObject(LPCWSTR funcName, ScriptMethod method, Var* funcObj)
{
    HRESULT hr = S_OK;
    Var externalVar;
    HTYPE typeRef;

    IfFailGo( _scriptDirect->CreateType(TypeId_Unspecified, NULL,method,NULL, FALSE, NULL, false, &typeRef) );
    IfFailGo( _scriptDirect->CreateTypedObject(typeRef,12, FALSE,&externalVar) );

    *funcObj = externalVar;

Error:
    return hr;
}

HRESULT JsGlass::SetPropertyOnObject(LPCWSTR propertyName, Var parentObj, Var propertyObj)
{
    HRESULT hr = S_OK;
    CComPtr<ITypeOperations> defaultScriptOperations;

    PropertyId sessionPropId;
    IfFailGo( _scriptDirect->GetOrAddPropertyId((LPWSTR)propertyName, &sessionPropId) );

    IfFailGo( _scriptDirect->GetDefaultTypeOperations(&defaultScriptOperations) );
    BOOL wasPropertySet = FALSE;
    IfFailGo( defaultScriptOperations->SetProperty(_scriptDirect, parentObj, sessionPropId, propertyObj, &wasPropertySet) );
    if (!wasPropertySet)
    {
        hr = E_FAIL;
    }

Error:
    return hr;
}

HRESULT JsGlass::CreateFunctionOnObject(LPCWSTR funcName, ScriptMethod method, Var obj, Var* funcObj)
{
    HRESULT hr = S_OK;
    Var newObj;
    IfFailGo( CreateFunctionObject(funcName, method, &newObj) );
    IfFailGo( SetPropertyOnObject(funcName, obj, newObj) );
    if (funcObj)
    {
        *funcObj = newObj;
    }

Error:
    return hr;
}

HRESULT JsGlass::CreateAuthoringEngine(LPCWSTR hostPath)
{
    // The debuggee needs to run on it's own thread
    CAuthoringHost::HostLoopArgs args;
    args.jsGlass = this;
    args.jscriptPath = hostPath;

    unsigned int threadId = 0;
    HANDLE hThread = (HANDLE) _beginthreadex(NULL,0, CAuthoringHost::HostLoop, &args, 0, &threadId );
    while (!PostThreadMessage(threadId, WM_USER, 0, 0))
    {
        DWORD code;
        if (!GetExitCodeThread(hThread, &code) || code != STILL_ACTIVE)
        {
            CloseHandle(hThread);
            return E_FAIL;
        }
        // the PostMessage will fail until the target thread starts
        // processing messages.
        Sleep(100);
    }

    CloseHandle(hThread);
    // HostLoop must be done with CAuthoringHost::HostLoopArgs by this point!!!
    // This is ok because it won't start processing messages until it is done with the args.

    return S_OK;
}

HRESULT JsGlass::StartTargetHost(LPCWSTR hostPath, LPCWSTR pdmPath)
{
    // The debuggee needs to run on it's own thread
    DebugTargetHost::HostLoopArgs args;
    args.jsGlass = this;
    args.jscriptPath = hostPath;
    args.pdmPath = pdmPath;

    unsigned int threadId = 0;
    HANDLE hThread = (HANDLE)_beginthreadex(NULL,0, DebugTargetHost::HostLoop, &args, 0, &threadId );
    
    while(!PostThreadMessage(threadId,WM_USER,0,0))
    {
        DWORD code;
        if (!GetExitCodeThread(hThread, &code) || code != STILL_ACTIVE)
        {
            CloseHandle(hThread);
            return E_FAIL;
        }
        // the PostMessage will fail until the target thread starts
        // processing messages.
        Sleep(100);
    }
    CloseHandle(hThread);

    // HostLoop must be done with DebugTargetHost::HostLoopArgs by this point!!!
    // This is ok because it won't start processing messages until it is done with the args.

    return S_OK;
}

HRESULT JsGlass::EventCallback(LPCWSTR eventJSON)
{
    CComBSTR eventJSONBstr = eventJSON;
    if (S_OK == this->_message.AsyncCall(&JsGlass::EventCallback,eventJSONBstr,NULL))
    {
        // The CComBSTR is captured by the Async Call.
        // The message it creates will have a CComBstr that
        // handles the lifetime responsibility for the eventname string.

        return S_OK;
    }

    HRESULT hr = S_OK;
    CComPtr<IJavascriptOperations> scriptOperations;
    PropertyId callbackPropertyId;
    Var callback;
    Var eventName[2];
    Var result;
    CallInfo callinfo = {2, CallFlags_None};

    IfFailGo( _scriptDirect->GetJavascriptOperations(&scriptOperations) );
    IfFailGo( _scriptDirect->GetOrAddPropertyId(L"callback", &callbackPropertyId) );
    IfFailGo( scriptOperations->GetProperty(_scriptDirect, _Eze, callbackPropertyId, &callback) );

    //
    // TODO construct an Event Object rather than an Event string..
    //
    eventName[0] = GetUndefined(); // undefined this
    eventName[1] = CreateString(eventJSON);
    IfFailGo( _scriptDirect->Execute(callback, callinfo, eventName, /*serviceProvider*/ NULL, &result) );

Error:
    return hr;
}

HRESULT JsGlass::Run(LPCWSTR rootJSFile)
{
    HRESULT hr = S_OK;
    CallInfo callInfo = {0, CallFlags_None};
    CComVariant variantArgc = _argc - 1; //strip off jsglass.exe
    Var globalVar;
    IfFailGo( _scriptDirect->GetGlobalObject(&globalVar) );

    Var varEze;
    IfFailGo( CreateFunctionOnObject(L"Eze", JsGlass::ScriptReadLine, globalVar, &varEze) );
    IfFailGo( CreateFunctionOnObject(L"ReadLine", JsGlass::ScriptReadLine, varEze, NULL) );
    IfFailGo( CreateFunctionOnObject(L"Write", JsGlass::ScriptWrite, varEze, NULL) );
    IfFailGo( CreateFunctionOnObject(L"ArgumentItem", JsGlass::ScriptArgumentItem, varEze, NULL) );
    IfFailGo( CreateFunctionOnObject(L"OpenScript", JsGlass::ScriptOpenTarget, varEze, NULL) );
    IfFailGo( CreateFunctionOnObject(L"RunScript", JsGlass::ScriptRunTarget, varEze, NULL) );
    IfFailGo( CreateFunctionOnObject(L"ResumeScript", JsGlass::ScriptResumeTarget, varEze, NULL) );
    IfFailGo( CreateFunctionOnObject(L"OpenTextFile", JsGlass::ScriptOpenTextFile, varEze, NULL) );
    IfFailGo( CreateFunctionOnObject(L"PumpMessages", JsGlass::ScriptPumpMessages, varEze, NULL) );
    IfFailGo( CreateFunctionOnObject(L"SetBreakpoint", JsGlass::ScriptSetBreakpoint, varEze, NULL) );
    IfFailGo( CreateFunctionOnObject(L"EnableFirstChanceException", JsGlass::ScriptEnableFirstChance, varEze, NULL) );
    IfFailGo( CreateFunctionOnObject(L"GetEnvironmentVariable", JsGlass::ScriptGetEnvironmentVariable, varEze, NULL) );
    IfFailGo( CreateFunctionOnObject(L"GetLocalFolderPath", JsGlass::ScriptGetLocalFolderPath, varEze, NULL) );
    IfFailGo( CreateFunctionOnObject(L"StartTargetHost", JsGlass::ScriptStartTargetHost, varEze, NULL) );
    IfFailGo( CreateFunctionOnObject(L"FileExists", JsGlass::ScriptFileExists, varEze, NULL) );
    IfFailGo( CreateFunctionOnObject(L"GetLocation", JsGlass::ScriptGetLocation, varEze, NULL) );
    IfFailGo( CreateFunctionOnObject(L"GetCallstack", JsGlass::ScriptGetCallstack, varEze, NULL) );
    IfFailGo( CreateFunctionOnObject(L"GetLocals", JsGlass::ScriptGetLocals, varEze, NULL) );
    IfFailGo( CreateFunctionOnObject(L"EvaluateExpr", JsGlass::ScriptEvaluateExpr, varEze, NULL) );
    IfFailGo( CreateFunctionOnObject(L"EditLocalValue", JsGlass::ScriptEditLocalValue, varEze, NULL) );
    IfFailGo( CreateFunctionOnObject(L"Quit", JsGlass::ScriptQuit, varEze, NULL) );
    IfFailGo( CreateFunctionOnObject(L"CreateAuthoringEngine", JsGlass::ScriptCreateAuthoringEngine, varEze, NULL) );
#ifdef DIRECT_AUTHOR_TEST
    IfFailGo( CreateFunctionOnObject(L"GetTokenRanges", JsGlass::GetTokenRanges, varEze, NULL) );
    IfFailGo( CreateFunctionOnObject(L"GetRegions", JsGlass::GetRegions, varEze, NULL) );
    IfFailGo( CreateFunctionOnObject(L"GetCompletions", JsGlass::GetCompletions, varEze, NULL) );
    IfFailGo( CreateFunctionOnObject(L"GetErrors", JsGlass::GetErrors, varEze, NULL) );
    IfFailGo( CreateFunctionOnObject(L"GetAst", JsGlass::GetAst, varEze, NULL) );
    IfFailGo( CreateFunctionOnObject(L"GetQuickInfo", JsGlass::GetQuickInfo, varEze, NULL) );
    IfFailGo( CreateFunctionOnObject(L"ProcessCompletionsSession", JsGlass::ProcessCompletionsSession, varEze, NULL) );
    IfFailGo( CreateFunctionOnObject(L"GetFunctionHelp", JsGlass::GetFunctionHelp, varEze, NULL) );
    IfFailGo( CreateFunctionOnObject(L"SplatterSession", JsGlass::SplatterSession, varEze, NULL) );
    IfFailGo( CreateFunctionOnObject(L"MultipleHostTypeCompletion", JsGlass::MultipleHostTypeCompletion, varEze, NULL) );
#endif

    Var varArgc = CreateNumber(_argc-1); //strip off jsglass.exe
    IfFailGo( SetPropertyOnObject(L"ArgumentLength", varEze, varArgc) );

    // Call the function
    Var topFunc;
    IfFailGo( _scriptDirect->Parse(defaultScript, &topFunc) );
    //IfFailGo( ParseFile(rootJSFile, &topFunc) );

    _Eze = varEze;

    Var resultVar;
    IfFailGo( _scriptDirect->Execute(topFunc, callInfo, NULL, /*serviceProvider*/ NULL, &resultVar) );
    return S_OK;

Error:
    wprintf(L"ERROR: Script execution failed");
    return hr;

}

JsGlassArgs::JsGlassArgs()
    : _jscriptDllPath(NULL),
      _jsGlassPath(NULL)
#ifdef DIRECT_AUTHOR_TEST
      , _authorDllPath(NULL)
#endif
{
}
JsGlassArgs::~JsGlassArgs()
{
    if (_jscriptDllPath)
    {
        delete _jscriptDllPath;
    }
    if (_jsGlassPath)
    {
        delete _jsGlassPath;
    }
}

static HRESULT SetPathOrDefault(LPCWSTR* member, LPCWSTR environmenttVar, LPCWSTR relativePath)
{
    WCHAR* buf;
    size_t bufSize;
    size_t strLen;
    size_t relativePathLen = wcslen(relativePath);

    // Environment variable will get the preference first

    DWORD envVarLen = GetEnvironmentVariable(environmenttVar, NULL, 0);
    if (envVarLen)
    {
        bufSize = envVarLen + relativePathLen + 2;
        buf = new WCHAR[bufSize];
        GetEnvironmentVariable(environmenttVar,buf,envVarLen);
        strLen = wcslen(buf) + 1;
        buf[strLen - 1] = '\\';
        wcsncpy_s(buf+strLen, bufSize-strLen, relativePath, relativePathLen);
        if (_waccess_s(buf, 0) == 0)
        {
            *member = buf;
            return S_OK;
        }
    }

    // if failed above try .local (for testing on the local boxes)

    WCHAR pathBuffer[_MAX_PATH];
    GetModuleFileName(NULL, pathBuffer, _MAX_PATH);
    strLen = wcslen(pathBuffer);
    size_t localLen = wcslen(L".local\\");

    wchar_t drive[_MAX_DRIVE];
    wchar_t dir[_MAX_DIR];
    _wsplitpath_s(pathBuffer, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0);

    if (strLen+localLen+relativePathLen+1 < _MAX_PATH)
    {
        wcsncpy_s(pathBuffer+strLen, _MAX_PATH-strLen, L".local\\", localLen);
        wcsncpy_s(pathBuffer+strLen+localLen, _MAX_PATH-strLen-localLen, relativePath, relativePathLen);

        if (_waccess_s(pathBuffer, 0) == 0)
        {
            *member = _wcsdup(pathBuffer);
            return S_OK;
        }
    }

    if (!_wmakepath_s(pathBuffer, drive, dir, relativePath, 0))
    {
        if (_waccess_s(pathBuffer, 0) == 0)
        {
            *member = _wcsdup(pathBuffer);
            return S_OK;
        }
    }

    // Failed to get the path
    return E_FAIL;
}

HRESULT GetFileTime(LPCWSTR filename, FILETIME *time)
{
    HRESULT hr = S_OK;
    HANDLE hFind;
    WIN32_FIND_DATA findData;

    hFind = FindFirstFile(filename, &findData);
    IfFalseGo(hFind != INVALID_HANDLE_VALUE, E_FAIL);
    IfFalseGo(FindClose(hFind), E_FAIL);

    *time = findData.ftLastWriteTime;

Error:
    return hr;
}

template< size_t N >
bool EqualsNoCase(const wchar_t *text, const wchar_t (&w)[N] )
{
    return _wcsnicmp(text, w, N - 1) == 0;
}

template< size_t N >
bool StartsWith(const wchar_t *text, const wchar_t (&w)[N] )
{
    return wcsncmp(text, w, N - 1) == 0;
}

template< size_t N >
const wchar_t *Skip(const wchar_t *text, const wchar_t (&w)[N] )
{
    return text + N - 1;
}

HRESULT JsGlassArgs::ParseCmdLine(int &argc, __inout_ecount(argc) const wchar_t** argv)
{
    HRESULT hr = S_OK;
    const wchar_t* dllName = L"chakra.dll";
#ifdef DIRECT_AUTHOR_TEST
    const wchar_t* authorDllName = NULL;
#endif
    // Parse options
    for (int j = 1; j < argc;)
    {
        bool isSwitch = false;

#ifdef DIRECT_AUTHOR_TEST
        if (StartsWith(argv[j], L"-AuthorBinary:"))
        {
            authorDllName = Skip(argv[j], L"-AuthorBinary:");
            isSwitch = true;
        }
#endif
        if (isSwitch)
        {
            argc--;
            for (int i = j; i < argc; i++)
                argv[i] = argv[i + 1];
            continue; // argv[j] updated, parse again
        }
        else
        {
            j++; // Parse next arg
        }
    }

    // This should only have the args that impact loading Eze as the host for jsGlass
    // Most argument parsing should be delegated to Jscript code

    // I'm using the _NTTREE environment variable to determine the right Jscript engine
    // to load as the host for jsglass.exe
    
    hr = SetPathOrDefault(&_jscriptDllPath,
                     L"_NTTREE",
                     dllName);
    IfFailGo(hr);

#ifdef DIRECT_AUTHOR_TEST    
    if (authorDllName && *authorDllName)
    {
         hr = SetPathOrDefault(&_authorDllPath,
                  L"_NTTREE",
                  authorDllName);
    }
#endif
Error:
    if (FAILED(hr))
    {
        fwprintf(stdout, L"Failed to load/locate file : %s under _NTTREE or .local\n", dllName, _jscriptDllPath);
        fflush(stdout);
    }
    return hr;
}

JsGlass* JsGlass::that = NULL;

JsGlass::JsGlass()
    : _coInitialized(false),
     _targetHost(NULL),
     _authoringHost(NULL),
#ifdef DIRECT_AUTHOR_TEST
     _authorServices(NULL),
#endif
     _message(this,GetCurrentThreadId())
{
    AssertMsg(that == NULL);
    that = this;
}

JsGlass::~JsGlass()
{
    if (_activeScript)
    {
        _activeScript->SetScriptState(SCRIPTSTATE_CLOSED);
    }
    if (_coInitialized)
    {
        CoUninitialize();
    }
    if (_targetHost)
    {
        _targetHost->Release();
    }
    if (_authoringHost)
    {
        _authoringHost->Release();
    }
}

HRESULT JsGlass::Initialize(JsGlassArgs& args)
{
    HRESULT hr = S_OK;
    //const CLSID CLSID_JScript  = { 0xf414c260, 0x6ac0, 0x11cf, 0xb6, 0xd1, 0x00, 0xaa, 0x00, 0xbb, 0xbb, 0x58 };
    const CLSID CLSID_Chakra = { 0x1b7cd997, 0xe5ff, 0x4932, 0xa7, 0xa6, 0x2a, 0x9e, 0x63, 0x6d, 0xa3, 0x85 };
#ifdef DIRECT_AUTHOR_TEST
    const CLSID CLSID_JScript9Ls = { 0xf13098a9, 0xcec8, 0x471e, 0x8e, 0x43, 0xd0, 0xbd, 0x93, 0x12, 0x62, 0x3};
#endif

    IfFailGo( CoInitializeEx(NULL, IsOs_OneCoreUAP() ? COINIT_MULTITHREADED : COINIT_APARTMENTTHREADED) );
    _coInitialized = true;

    hr = PrivateCoCreate(args._jscriptDllPath, CLSID_Chakra, NULL, CLSCTX_INPROC_SERVER, _uuidof(IActiveScript), (LPVOID*)&_activeScript);

    if (FAILED(hr))
    {
        fwprintf(stdout, L"Failed to PrivateCoCreate :%s\n", args._jscriptDllPath);
        fflush(stdout);
        goto Error;
    }

    IfFailGo( _activeScript->QueryInterface(&_scriptDirect) );

#if DBG
    {
        CComPtr<IActiveScriptProperty> pActiveScriptProperty;
        CComVariant VarVersion;
        IfFailGo(_scriptDirect->QueryInterface(&pActiveScriptProperty));
        hr = pActiveScriptProperty->GetProperty(SCRIPTPROP_INVOKEVERSIONING, nullptr, &VarVersion);
        AssertMsg(SUCCEEDED(hr) && VarVersion.vt == VT_I4 && VarVersion.iVal == SCRIPTLANGUAGEVERSION_5_12, L"Incorrect runtime version");
    }
#endif   

    ActiveScriptController* pActiveScriptController = new ActiveScriptController();
    IfFalseGo( pActiveScriptController, E_OUTOFMEMORY );
    _fakeSite.Attach( pActiveScriptController);

    IfFailGo( pActiveScriptController->Initialize(NULL, 7) );
    IfFailGo( _activeScript->SetScriptSite(_fakeSite) );

    _debugger = new Debugger(this);
    _inlineDebugger = _debugger;

    _threadId = GetCurrentThreadId();

#ifdef DIRECT_AUTHOR_TEST
    if (args._authorDllPath && *(args._authorDllPath))
    {
        hr =  PrivateCoCreate(args._authorDllPath, CLSID_JScript9Ls, NULL, CLSCTX_INPROC_SERVER, _uuidof(IAuthorServices), (LPVOID*)&_authorServices);
        if (FAILED(hr))
        {
            fwprintf(stdout, L"Failed to PrivateCoCreate author dll at :%s\n", args._authorDllPath);
            fflush(stdout);
            goto Error;
        }
    }
#endif
Error:

    return hr;
}

int _cdecl wmain(int argc, const wchar_t* argv[])
{
    HRESULT hr = S_OK;
    JsGlassArgs args;
    JsGlass jsGlass;

    if (argc < 2 || (wcsncmp(argv[1], L"/?", 2) == 0) || (wcsncmp(argv[1], L"-?", 2) == 0))
    {
        // Print usage
        fwprintf(stdout, L"jsglass.exe [-AuthorBinary:chakrals.dll] example.json\n");
        fflush(stdout);

        return (int)E_FAIL;
    }

    if (FAILED(args.ParseCmdLine(argc,argv)))
    {
        fwprintf(stdout, L"Failed to complete ParseCmdLine\n");
        fflush(stdout);
        goto Error;
    }

    if (FAILED(jsGlass.Initialize(args)))
    {
        fwprintf(stdout, L"Failed to complete Initialize\n");
        fflush(stdout);
        goto Error;
    }

    jsGlass._argc = argc;
    jsGlass._argv = argv;

    if (FAILED(jsGlass.Run(args._jsGlassPath)))
    {
        fwprintf(stdout, L"Failed to complete Run\n");
        fflush(stdout);
        goto Error;
    }

Error:
    // Flush all I/O buffers
    _flushall();

    return (int)hr;
}

