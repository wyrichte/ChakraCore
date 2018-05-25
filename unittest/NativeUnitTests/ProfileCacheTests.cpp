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

    if(_wfopen_s(&file, filename, _u("rb")))
    {
        fOpenFailed = TRUE;
    }

    if (fOpenFailed)
    {
        char16 wszBuff[512];
        fwprintf(stderr, _u("_wfopen of %s failed"), filename);
        wszBuff[0] = 0;
        if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            GetLastError(),
            0,
            wszBuff,
            _countof(wszBuff),
            NULL))
        {
            fwprintf(stderr, _u(": %s"), wszBuff);
        }
        fwprintf(stderr, _u("\n"));
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
        fwprintf(stderr, _u("out of memory"));
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
        fwprintf(stderr, _u("unsupported file encoding"));
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
        *script = (LPWSTR) calloc(cAnsiChars, sizeof(char16));
        if (NULL == script)
        {
            fwprintf(stderr, _u("out of memory"));
            return E_OUTOFMEMORY;
        }

        // Convert to Unicode.
        if (0 == MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)contentsRaw, cAnsiChars,
            *script, cAnsiChars))
        {
            free((void*)*script);

            fwprintf(stderr, _u("failed MultiByteToWideChar conversion"));

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

const uint CACHE_FILE_LENGTH = 128;
wchar_t inCacheFile[CACHE_FILE_LENGTH];
wchar_t outCacheFile[CACHE_FILE_LENGTH];

HRESULT SetProfileCacheFilenames(uint testId, const wchar_t* suffix)
{
    swprintf_s(inCacheFile, CACHE_FILE_LENGTH, _u("profilecachetest%u.%s.in"), testId, suffix);
    swprintf_s(outCacheFile, CACHE_FILE_LENGTH, _u("profilecachetest%u.%s.out"), testId, suffix);

    return NOERROR;
}

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

    IfFailedReturn(SetProfileCacheFilenames(1, gArgs->flagCount > 1 ? gArgs->flags[1] : _u("")));
    printf("ProfileCacheTests: Basic Test\n");
    ProfileCacheTest* test = new ProfileCacheTest(gArgs, _u("profilecachetest1.js"), nullptr, outCacheFile);
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
    When we have a javascript with less than 5 functions, SaveWrite should not be called
    */

    HRESULT hr = NOERROR;

    IfFailedReturn(SetProfileCacheFilenames(2, gArgs->flagCount > 1 ? gArgs->flags[1] : _u("")));
    printf("ProfileCacheTests: JS with less than 5 functions\n");
    ProfileCacheTest* test = new ProfileCacheTest(gArgs, _u("profilecachetest2.js"), nullptr, outCacheFile);
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

HRESULT RunTestcase3()
{
    /*
    When we have a javascript with 5 functions, GetWrite and SaveWrite should be called
    */

    HRESULT hr = NOERROR;

    IfFailedReturn(SetProfileCacheFilenames(3, gArgs->flagCount > 1 ? gArgs->flags[1] : _u("")));
    printf("ProfileCacheTests: Execute js file with 5 functions\n");
    ProfileCacheTest* test = new ProfileCacheTest(gArgs, _u("profilecachetest3.js"), nullptr, outCacheFile);
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

    IfFailedReturn(SetProfileCacheFilenames(4, gArgs->flagCount > 1 ? gArgs->flags[1] : _u("")));
    printf("ProfileCacheTests: Call OnEvent with VARIANT\n");
    ProfileCacheTest* test = new ProfileCacheTest(gArgs, _u("profilecachetest4.js"), nullptr, outCacheFile);
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

    IfFailedReturn(SetProfileCacheFilenames(5, gArgs->flagCount > 1 ? gArgs->flags[1] : _u("")));
    printf("ProfileCacheTests: Invalid Profile Data\n");
    ProfileCacheTest* test = new ProfileCacheTest(gArgs, _u("profilecachetest5.js"), nullptr, outCacheFile);
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

    IfFailedReturn(SetProfileCacheFilenames(5, gArgs->flagCount > 1 ? gArgs->flags[1] : _u("")));
    printf("ProfileCacheTests: Profile Data Less than 15 percents change\n");
    ProfileCacheTest* test = new ProfileCacheTest(gArgs, _u("profilecachetest5.js"), outCacheFile, outCacheFile);
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

HRESULT RunTestcase7()
{
    /*
    Use a profile data which has different version number. Chakra should not use it and save a new one.
    */

    HRESULT hr = NOERROR;

    printf("ProfileCacheTests: Profile Data Mismatched Version\n");

    IfFailedReturn(SetProfileCacheFilenames(7, gArgs->flagCount > 1 ? gArgs->flags[1] : _u("")));
    ProfileCacheTest* test = new ProfileCacheTest(gArgs, _u("profilecachetest7.js"), inCacheFile, outCacheFile);
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

    IfFailedReturn(SetProfileCacheFilenames(8, gArgs->flagCount > 1 ? gArgs->flags[1] : _u("")));
    ProfileCacheTest* test = new ProfileCacheTest(gArgs, _u("profilecachetest8.js"), nullptr, outCacheFile);
    test->CreateNewEngine();
    test->mptr_fakeMSTHML->SetFailGetWriteDataStream(TRUE);
    test->ParseScriptText();
    printf("IActiveScriptLifecycleEventSink->OnEvent()\n");
    hr = test->mptr_LifecycleEventSink->OnEvent(EventId_StartupComplete, NULL);
    IfFailedReturn(hr);
    // We will have two attempts to get a write stream now because the first one returns E_FAIL so the stream pointer
    // in the wrapper class will stay nullptr. This means next write will try again to open another write stream.
    test->VerifyDataStreamMethodCallCounter(2,0);
    test->CloseEngine();

    return hr;
}

HRESULT RunTestcase9()
{
    HRESULT hr = NOERROR;

    printf("ProfileCacheTests: Fail HRESULT from SaveWriteDataStream\n");

    IfFailedReturn(SetProfileCacheFilenames(9, gArgs->flagCount > 1 ? gArgs->flags[1] : _u("")));
    ProfileCacheTest* test = new ProfileCacheTest(gArgs, _u("profilecachetest9.js"), nullptr, outCacheFile);
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

    IfFailedReturn(SetProfileCacheFilenames(10, gArgs->flagCount > 1 ? gArgs->flags[1] : _u("")));
    ProfileCacheTest* test = new ProfileCacheTest(gArgs, _u("profilecachetest10.js"), nullptr, outCacheFile);
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

    IfFailedReturn(SetProfileCacheFilenames(11, gArgs->flagCount > 1 ? gArgs->flags[1] : _u("")));
    ProfileCacheTest* test = new ProfileCacheTest(gArgs, _u("profilecachetest11.js"), nullptr, outCacheFile);
    test->mptr_fakeMSTHML->SetNullGetWriteDataStream(TRUE);
    test->CreateNewEngine();
    test->ParseScriptText();

    printf("IActiveScriptLifecycleEventSink->OnEvent()\n");
    hr = test->mptr_LifecycleEventSink->OnEvent(EventId_StartupComplete, NULL);
    IfFailedReturn(hr);

    // Same as RunTestcase8, both of the write activities will attempt to open a new write stream.
    test->VerifyDataStreamMethodCallCounter(2,0);
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