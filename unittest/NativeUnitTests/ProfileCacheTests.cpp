/*
ProfileCacheTests.cpp

History:
10/28/11 - Initial version - JohniS
*/
#include "StdAfx.h"
#include "ScriptDirectUnitTests.h"
using namespace std;

HRESULT LoadScriptFromFile(LPCWSTR filename, LPWSTR* script)
{
    FILE * file;
    LPCOLESTR contents = NULL;
    BOOL fOpenFailed = FALSE;

    if(_wfopen_s(&file, filename, L"rb"))
    {
        fOpenFailed = TRUE;
    }

    if (fOpenFailed)
    {
        wchar_t wszBuff[512];
        fwprintf(stderr, L"_wfopen of %s failed", filename);
        wszBuff[0] = 0;
        if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            GetLastError(),
            0,
            wszBuff,
            _countof(wszBuff),
            NULL))
        {
            fwprintf(stderr, L": %s", wszBuff);
        }
        fwprintf(stderr, L"\n");
        return E_FAIL;
    }

    //
    // Determine the file length, in bytes.
    //
    fseek(file, 0, SEEK_END);
    UINT lengthBytes = ftell(file);
    fseek(file, 0, SEEK_SET);
    LPCOLESTR contentsRaw = (LPCOLESTR) calloc(lengthBytes + 2, 1);
    if (NULL == contentsRaw)
    {
        fwprintf(stderr, L"out of memory");
        return E_OUTOFMEMORY;
    }
    //
    // Read the entire content as a binary block.
    //
    fread((void*) contentsRaw, sizeof(char), lengthBytes, file);
    fclose(file);


    //Detecting UTF8 encoding
    byte * pRawBytes = (byte*)contentsRaw;
    bool isUtf8 = false;
    if( (0xEF == *pRawBytes && 0xBB == *(pRawBytes+1) && 0xBF == *(pRawBytes+2)))
    {
        isUtf8 = true;
    }
    else if (0xFFFE == *contentsRaw || 0x0000 == *contentsRaw && 0xFEFF == *(contentsRaw+1))
    {
        // unicode unsupported
        fwprintf(stderr, L"unsupported file encoding");
        return E_UNEXPECTED;
    }
    else if (0xFEFF == *contentsRaw)
    {
        // unicode LE
        contents = contentsRaw;
    }
    else
    {
        // Assume UTF8
        isUtf8 = true;
    }

    if (isUtf8)
    {
        UINT cAnsiChars = lengthBytes + 1;
        *script = (LPWSTR) calloc(cAnsiChars, sizeof(wchar_t));
        if (NULL == script)
        {
            fwprintf(stderr, L"out of memory");
            return E_OUTOFMEMORY;
        }

        // Convert to Unicode.
        if (0 == MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)contentsRaw, cAnsiChars,
            *script, cAnsiChars))
        {
            free((void*)*script);

            fwprintf(stderr, L"failed MultiByteToWideChar conversion");

            return HRESULT_FROM_WIN32(GetLastError());
        }

    }

    return S_OK;
}
void PrintSeparator()
{
    printf("=============================================\n");
}
/*
Implementation of ProfileCacheTest class
*/
ProfileCacheTest::ProfileCacheTest(JsHostNativeTestArguments* jsHostArgs, LPCWSTR jsFile, LPWSTR inputCacheFile, LPWSTR outputCacheFile)
{
    mptr_jsHostArgs = jsHostArgs;
    mptr_jsFile = jsFile;
    mptr_inputCacheFile = inputCacheFile;
    mptr_outputCacheFile = outputCacheFile;
    mptr_fakeMSTHML = new FakeMSHTML(inputCacheFile, outputCacheFile);
    LoadScriptFromFile(jsFile,&mScript);
}
void ProfileCacheTest::CreateNewEngine()
{
    printf("CreateNewEngine()\n");
    mptr_jsHostArgs->createNewEngine(&mptr_EzeScriptDirect, &mJsHostScriptSite, true);
    mptr_EzeScriptDirect->QueryInterface(__uuidof(IActiveScriptParse), (LPVOID*)&mptr_ActiveScriptParse);
    mptr_EzeScriptDirect->QueryInterface(__uuidof(IActiveScriptLifecycleEventSink), (LPVOID*)&mptr_LifecycleEventSink);
}
void ProfileCacheTest::ParseScriptText()
{
    printf("ParseScriptText()\n");
    mptr_ActiveScriptParse->ParseScriptText(mScript, NULL, (IUnknown*)(static_cast<IActiveScriptDataCache*>(mptr_fakeMSTHML)), NULL, 
        0xdeadbeef, 0, SCRIPTTEXT_HOSTMANAGESSOURCE, &mResult, &mExcepInfo);
}
void ProfileCacheTest::CloseEngine()
{
    printf("CloseEngine()\n");
    mptr_LifecycleEventSink->Release();
    mptr_LifecycleEventSink = NULL;
    mptr_ActiveScriptParse->Release();
    mptr_ActiveScriptParse = NULL;
    mptr_EzeScriptDirect->Release();
    mptr_EzeScriptDirect = NULL;
    mptr_jsHostArgs->shutdownScriptSite(mJsHostScriptSite);
    mJsHostScriptSite = NULL;
}
void ProfileCacheTest::VerifyDataStreamMethodCallCounter(UINT expectedGetWriteCount, UINT expectedSaveWriteCount)
{
    printf("Verify GetWriteDataStream Counter\n");
    if(mptr_fakeMSTHML->GetWriteDataStreamCounter() == expectedGetWriteCount)
    {
        printf("PASS\n");
    }
    else
    {
        printf("FAIL\n");
    }
    printf("Verify SaveWriteDataStream Counter\n");
    if(mptr_fakeMSTHML->SaveWriteDataStreamCounter() == expectedSaveWriteCount)
    {
        printf("PASS\n");
    }
    else
    {
        printf("FAIL\n");
    }
}


HRESULT gRetVal = S_OK;
//Global JsHostNativeTestArguments object passed to this test from JsHost
JsHostNativeTestArguments* gArgs = NULL;
IActiveScript *gActiveScript = NULL;
IActiveScriptParse *gActiveScriptParse = NULL;
IActiveScriptLifecycleEventSink *gActiveScriptLifecycleEventSink = NULL;

//Collection of all testcases for API testing
HRESULT RunTestcase1()
{
    /*
    Basic testcase:
    - Parse a javascript file, then invoke OnEvent method on IActiveScriptLifecycleEventSink
    - Chakra should call the GetWriteDataStream and then SaveWriteDataStream methods
    - If we call the OnEvent again, GetWriteDataStream and SaveWriteDataStream should not be called again
    */
    HRESULT hr = NOERROR;

    printf("ProfileCacheTests: Basic Test\n");
    ProfileCacheTest* test = new ProfileCacheTest(gArgs, L"profilecachetest1.js", NULL, L"profilecachetest1.out");
    test->CreateNewEngine();
    test->ParseScriptText();
    printf("IActiveScriptLifecycleEventSink->OnEvent()\n");
    hr = test->mptr_LifecycleEventSink->OnEvent(EventId_StartupComplete, NULL);
    IfFailedReturn(hr);
    test->VerifyDataStreamMethodCallCounter(1,1);
    printf("IActiveScriptLifecycleEventSink->OnEvent()\n");
    hr = test->mptr_LifecycleEventSink->OnEvent(EventId_StartupComplete, NULL);
    IfFailedReturn(hr);
    test->VerifyDataStreamMethodCallCounter(1,1);
    test->CloseEngine();
    PrintSeparator();
    return hr;
}

HRESULT RunTestcase2()
{
    /*
    When we have a javascript with less than 5 functions, GetWrite and SaveWrite should not be called
    */

    HRESULT hr = NOERROR;

    printf("ProfileCacheTests: JS with less than 5 functions\n");
    ProfileCacheTest* test = new ProfileCacheTest(gArgs, L"profilecachetest2.js", NULL, L"profilecachetest2.out");
    test->CreateNewEngine();
    test->ParseScriptText();
    printf("IActiveScriptLifecycleEventSink->OnEvent()\n");
    hr = test->mptr_LifecycleEventSink->OnEvent(EventId_StartupComplete, NULL);
    IfFailedReturn(hr);
    test->VerifyDataStreamMethodCallCounter(0,0);
    test->CloseEngine();
    PrintSeparator();
    return hr;
}

HRESULT RunTestcase3()
{
    /*
    When we have a javascript with 5 functions, GetWrite and SaveWrite should be called
    */

    HRESULT hr = NOERROR;

    printf("ProfileCacheTests: Execute js file with 5 functions\n");
    ProfileCacheTest* test = new ProfileCacheTest(gArgs, L"profilecachetest3.js", NULL, L"profilecachetest3.out");
    test->CreateNewEngine();
    test->ParseScriptText();
    printf("IActiveScriptLifecycleEventSink->OnEvent()\n");
    hr = test->mptr_LifecycleEventSink->OnEvent(EventId_StartupComplete, NULL);
    IfFailedReturn(hr);
    test->VerifyDataStreamMethodCallCounter(1,1);
    test->CloseEngine();
    PrintSeparator();
    return hr;
}

HRESULT RunTestcase4()
{
    /*
    Eventhough the VARIANT parameter on OnEvent method is not useful currently, it should not break when we pass non null value
    */

    HRESULT hr = NOERROR;

    printf("ProfileCacheTests: Call OnEvent with VARIANT\n");
    ProfileCacheTest* test = new ProfileCacheTest(gArgs, L"profilecachetest4.js", NULL, L"profilecachetest4.out");
    test->CreateNewEngine();
    test->ParseScriptText();
    printf("IActiveScriptLifecycleEventSink->OnEvent()\n");
    hr = test->mptr_LifecycleEventSink->OnEvent(EventId_StartupComplete, new VARIANT());
    IfFailedReturn(hr);
    test->VerifyDataStreamMethodCallCounter(1,1);
    test->CloseEngine();
    PrintSeparator();
    return hr;
}

HRESULT RunTestcase5()
{
    /*
    Eventhough the VARIANT parameter on OnEvent method is not useful currently, it should not break when we pass non null value
    */

    HRESULT hr = NOERROR;

    printf("ProfileCacheTests: Invalid Profile Data\n");
    ProfileCacheTest* test = new ProfileCacheTest(gArgs, L"profilecachetest5.js", NULL, L"profilecachetest5.out");
    test->CreateNewEngine();
    test->ParseScriptText();
    printf("IActiveScriptLifecycleEventSink->OnEvent()\n");
    hr = test->mptr_LifecycleEventSink->OnEvent(EventId_StartupComplete, NULL);
    IfFailedReturn(hr);
    test->VerifyDataStreamMethodCallCounter(1,1);
    test->CloseEngine();
    PrintSeparator();
    return hr;
}

HRESULT RunTestcase6()
{
    /*
    When the profile data has less than 15% change, the GetWrite and SaveWrite should not be called
    */

    HRESULT hr = NOERROR;

    printf("ProfileCacheTests: Profile Data Less than 15 percents change\n");
    ProfileCacheTest* test = new ProfileCacheTest(gArgs, L"profilecachetest5.js", L"profilecachetest5.out", L"profilecachetest5.out");
    test->CreateNewEngine();
    test->ParseScriptText();
    printf("IActiveScriptLifecycleEventSink->OnEvent()\n");
    hr = test->mptr_LifecycleEventSink->OnEvent(EventId_StartupComplete, NULL);
    IfFailedReturn(hr);
    test->VerifyDataStreamMethodCallCounter(0,0);
    test->CloseEngine();
    PrintSeparator();
    return hr;
}

HRESULT RunTestcase7()
{
    /*
    Use a profile data which has different version number. Chakra should not use it and save a new one.
    */

    HRESULT hr = NOERROR;

    printf("ProfileCacheTests: Profile Data Mismatched Version\n");

    ProfileCacheTest* test = new ProfileCacheTest(gArgs, L"profilecachetest7.js", L"profilecachetest7.in", L"profilecachetest7.out");
    test->CreateNewEngine();
    test->ParseScriptText();
    printf("IActiveScriptLifecycleEventSink->OnEvent()\n");
    hr = test->mptr_LifecycleEventSink->OnEvent(EventId_StartupComplete, NULL);
    IfFailedReturn(hr);
    test->VerifyDataStreamMethodCallCounter(1,1);
    test->CloseEngine();
    PrintSeparator();
    return hr;
}

HRESULT RunTestcase8()
{
    HRESULT hr = NOERROR;

    printf("ProfileCacheTests: Fail HRESULT from GetWriteDataStream\n");

    ProfileCacheTest* test = new ProfileCacheTest(gArgs, L"profilecachetest8.js", NULL, L"profilecachetest8.out");
    test->CreateNewEngine();
    test->ParseScriptText();
    test->mptr_fakeMSTHML->SetFailGetWriteDataStream(TRUE);
    printf("IActiveScriptLifecycleEventSink->OnEvent()\n");
    hr = test->mptr_LifecycleEventSink->OnEvent(EventId_StartupComplete, NULL);
    IfFailedReturn(hr);
    test->VerifyDataStreamMethodCallCounter(1,0);
    test->CloseEngine();

    return hr;
}

HRESULT RunTestcase9()
{
    HRESULT hr = NOERROR;

    printf("ProfileCacheTests: Fail HRESULT from SaveWriteDataStream\n");

    ProfileCacheTest* test = new ProfileCacheTest(gArgs, L"profilecachetest9.js", NULL, L"profilecachetest9.out");
    test->CreateNewEngine();
    test->ParseScriptText();
    test->mptr_fakeMSTHML->SetFailSaveWriteDataStream(TRUE);
    printf("IActiveScriptLifecycleEventSink->OnEvent()\n");
    hr = test->mptr_LifecycleEventSink->OnEvent(EventId_StartupComplete, NULL);
    IfFailedReturn(hr);
    test->VerifyDataStreamMethodCallCounter(1,1);
    test->CloseEngine();
    PrintSeparator();
    return hr;
}

HRESULT RunTestcase10()
{
    HRESULT hr = NOERROR;

    printf("ProfileCacheTests: NULL IStream for GetReadDataStream\n");

    ProfileCacheTest* test = new ProfileCacheTest(gArgs, L"profilecachetest10.js", NULL, L"profilecachetest10.out");
    test->mptr_fakeMSTHML->SetNullGetReadDataStream(TRUE);
    test->CreateNewEngine();
    test->ParseScriptText();
    printf("IActiveScriptLifecycleEventSink->OnEvent()\n");
    hr = test->mptr_LifecycleEventSink->OnEvent(EventId_StartupComplete, NULL);
    IfFailedReturn(hr);
    test->VerifyDataStreamMethodCallCounter(1,1);
    test->CloseEngine();
    PrintSeparator();
    return hr;
}

HRESULT RunTestcase11()
{
    HRESULT hr = NOERROR;

    printf("ProfileCacheTests: NULL IStream for GetWriteDataStream\n");

    ProfileCacheTest* test = new ProfileCacheTest(gArgs, L"profilecachetest11.js", NULL, L"profilecachetest11.out");
    test->mptr_fakeMSTHML->SetNullGetWriteDataStream(TRUE);
    test->CreateNewEngine();
    test->ParseScriptText();

    printf("IActiveScriptLifecycleEventSink->OnEvent()\n");
    hr = test->mptr_LifecycleEventSink->OnEvent(EventId_StartupComplete, NULL);
    IfFailedReturn(hr);

    test->VerifyDataStreamMethodCallCounter(1,0);
    test->CloseEngine();
    PrintSeparator();
    return hr;
}

void RunProfileCacheTests(JsHostNativeTestArguments* args)
{
    gArgs = args;
    gActiveScript = args->activeScript;

    try
    {
        RunTestcase1();
        RunTestcase2();
        RunTestcase3();
        RunTestcase4();
        RunTestcase5();
        RunTestcase6();
        RunTestcase7();
        RunTestcase8();
        RunTestcase9();
        RunTestcase10();
        RunTestcase11();
    }
    catch(std::string message)
    {
        Print(message, false);
    }
    catch(exception ex)
    {
        Print(ex.what(), false);
    }
}