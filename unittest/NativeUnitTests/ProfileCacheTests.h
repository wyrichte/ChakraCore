#include "StdAfx.h"

/*
ProfileCacheTest represents a single testcase for the API testing
*/
class ProfileCacheTest
{
private:
    JsHostNativeTestArguments* mptr_jsHostArgs;
    IActiveScriptDirect* mptr_EzeScriptDirect;

    LPCWSTR mptr_jsFile;
    LPCWSTR mptr_inputCacheFile;
    LPCWSTR mptr_outputCacheFile;
    LPWSTR mScript;
    void* mJsHostScriptSite;

    EXCEPINFO mExcepInfo;
    VARIANT mResult;

public:
    IActiveScriptParse* mptr_ActiveScriptParse;
    IActiveScriptLifecycleEventSink* mptr_LifecycleEventSink;
    FakeMSHTML* mptr_fakeMSTHML;
    ProfileCacheTest(__in JsHostNativeTestArguments* jsHostArgs, LPCWSTR jsFile, __in LPWSTR inputCacheFile, __in LPWSTR outputCacheFile);

    //Call the actual ParseScriptText method from IActiveScriptParse interface
    void ParseScriptText();

    //Creating new engine and get the IActiveScriptParse and IActiveScriptLifecycleEventSink reference
    void CreateNewEngine();

    //Shutdown script site and releasing all reference
    void CloseEngine();

    //Method to verify whether GetWriteDataStream and SaveWriteDataStream are called or not
    void VerifyDataStreamMethodCallCounter(UINT expectedGetWriteCount, UINT expectedSaveWriteCount);
};