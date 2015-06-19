//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

#include <initguid.h>
#define INITGUID
#include "guids.h"

JScript9Interface::ArgInfo JScript9Interface::m_argInfo = { 0 };
TestHooks JScript9Interface::m_testHooks = { 0 };
JsrtTestHooks JScript9Interface::m_jsrtTestHooks = { 0 };
MemProtectTestHooks JScript9Interface::m_memProtectTestHooks = { 0 };
boolean JScript9Interface::m_testHooksSetup = false;
boolean JScript9Interface::m_freeChakraLoaded = false;
boolean JScript9Interface::m_testHooksInitialized = false;

void JScript9Interface::SetArgInfo(ArgInfo& args)
{
    m_argInfo = args;
}

HINSTANCE JScript9Interface::LoadDll(__in_z LPCWSTR dllName, size_t /*urlLen*/, ArgInfo& argInfo)
{
    m_argInfo = argInfo;

    // Try to load the test dll. If that fails fall back to the release version.
    HINSTANCE jscriptLibrary = LoadLibraryEx(dllName, nullptr, 0);
    if (!jscriptLibrary)
    {
        // Clear out the test hooks since DLL didn't load properly and don't want to accidentally use them (eg for FaultInjection dump)
        memset((void*)&m_testHooks, 0, sizeof(m_testHooks));
        m_testHooksSetup = false;
        jscriptLibrary = LoadLibrary(L"chakra.dll");
    }
    if (!jscriptLibrary)
    {
        int ret = GetLastError();
        fwprintf(stderr, L"FATAL ERROR: Unable to load chakra.dll. GLE=0x%x\n", ret);
        return NULL;
    }

    m_freeChakraLoaded = true;

    if (!m_testHooksSetup)
    {
        m_testHooks.pfDllGetClassObject = (TestHooks::DllGetClassObjectPtr)GetProcAddress(jscriptLibrary, "DllGetClassObject");
        m_testHooks.pfJsVarToScriptDirect = (TestHooks::JsVarToScriptDirectPtr)GetProcAddress(jscriptLibrary, "JsVarToScriptDirect");
        m_testHooks.pfJsVarAddRef = (TestHooks::JsVarAddRefPtr)GetProcAddress(jscriptLibrary, "JsVarAddRef");
        m_testHooks.pfJsVarRelease = (TestHooks::JsVarReleasePtr)GetProcAddress(jscriptLibrary, "JsVarRelease");
        m_testHooks.pfJsVarToExtension = (TestHooks::JsVarToExtensionPtr)GetProcAddress(jscriptLibrary, "JsVarToExtension");
    }

    m_jsrtTestHooks.pfJsrtCreateRuntime = (JsrtTestHooks::JsrtCreateRuntimePtr)GetProcAddress(jscriptLibrary, "JsCreateRuntime");
    m_jsrtTestHooks.pfJsrtCreateContext = (JsrtTestHooks::JsrtCreateContextPtr)GetProcAddress(jscriptLibrary, "JsCreateContext");
    m_jsrtTestHooks.pfJsrtSetCurrentContext = (JsrtTestHooks::JsrtSetCurrentContextPtr)GetProcAddress(jscriptLibrary, "JsSetCurrentContext");
    m_jsrtTestHooks.pfJsrtGetCurrentContext = (JsrtTestHooks::JsrtGetCurrentContextPtr)GetProcAddress(jscriptLibrary, "JsGetCurrentContext");
    m_jsrtTestHooks.pfJsrtDisposeRuntime = (JsrtTestHooks::JsrtDisposeRuntimePtr)GetProcAddress(jscriptLibrary, "JsDisposeRuntime");
    m_jsrtTestHooks.pfJsrtCreateObject = (JsrtTestHooks::JsrtCreateObjectPtr)GetProcAddress(jscriptLibrary, "JsCreateObject");
    m_jsrtTestHooks.pfJsrtCreateExternalObject = (JsrtTestHooks::JsrtCreateExternalObjectPtr)GetProcAddress(jscriptLibrary, "JsCreateExternalObject");
    m_jsrtTestHooks.pfJsrtCreateFunction = (JsrtTestHooks::JsrtCreateFunctionPtr)GetProcAddress(jscriptLibrary, "JsCreateFunction");
    m_jsrtTestHooks.pfJsrtPointerToString = (JsrtTestHooks::JsrtPointerToStringPtr)GetProcAddress(jscriptLibrary, "JsPointerToString");
    m_jsrtTestHooks.pfJsrtSetProperty = (JsrtTestHooks::JsrtSetPropertyPtr)GetProcAddress(jscriptLibrary, "JsSetProperty");
    m_jsrtTestHooks.pfJsrtGetGlobalObject = (JsrtTestHooks::JsrtGetGlobalObjectPtr)GetProcAddress(jscriptLibrary, "JsGetGlobalObject");
    m_jsrtTestHooks.pfJsrtGetUndefinedValue = (JsrtTestHooks::JsrtGetUndefinedValuePtr)GetProcAddress(jscriptLibrary, "JsGetUndefinedValue");
    m_jsrtTestHooks.pfJsrtGetTrueValue = (JsrtTestHooks::JsrtGetUndefinedValuePtr)GetProcAddress(jscriptLibrary, "JsGetTrueValue");
    m_jsrtTestHooks.pfJsrtGetFalseValue = (JsrtTestHooks::JsrtGetUndefinedValuePtr)GetProcAddress(jscriptLibrary, "JsGetFalseValue");
    m_jsrtTestHooks.pfJsrtConvertValueToString = (JsrtTestHooks::JsrtConvertValueToStringPtr)GetProcAddress(jscriptLibrary, "JsConvertValueToString");
    m_jsrtTestHooks.pfJsrtConvertValueToNumber = (JsrtTestHooks::JsrtConvertValueToNumberPtr)GetProcAddress(jscriptLibrary, "JsConvertValueToNumber");
    m_jsrtTestHooks.pfJsrtConvertValueToBoolean = (JsrtTestHooks::JsrtConvertValueToBooleanPtr)GetProcAddress(jscriptLibrary, "JsConvertValueToBoolean");
    m_jsrtTestHooks.pfJsrtStringToPointer = (JsrtTestHooks::JsrtStringToPointerPtr)GetProcAddress(jscriptLibrary, "JsStringToPointer");
    m_jsrtTestHooks.pfJsrtBooleanToBool = (JsrtTestHooks::JsrtBooleanToBoolPtr)GetProcAddress(jscriptLibrary, "JsBooleanToBool");
    m_jsrtTestHooks.pfJsrtGetPropertyIdFromName = (JsrtTestHooks::JsrtGetPropertyIdFromNamePtr)GetProcAddress(jscriptLibrary, "JsGetPropertyIdFromName");
    m_jsrtTestHooks.pfJsrtGetProperty = (JsrtTestHooks::JsrtGetPropertyPtr)GetProcAddress(jscriptLibrary, "JsGetProperty");
    m_jsrtTestHooks.pfJsrtHasProperty = (JsrtTestHooks::JsrtHasPropertyPtr)GetProcAddress(jscriptLibrary, "JsHasProperty");
    m_jsrtTestHooks.pfJsrtRunScript = (JsrtTestHooks::JsrtRunScriptPtr)GetProcAddress(jscriptLibrary, "JsRunScript");    
    m_jsrtTestHooks.pfJsrtCallFunction = (JsrtTestHooks::JsrtCallFunctionPtr)GetProcAddress(jscriptLibrary, "JsCallFunction");
    m_jsrtTestHooks.pfJsrtNumbertoDouble = (JsrtTestHooks::JsrtNumberToDoublePtr)GetProcAddress(jscriptLibrary, "JsNumberToDouble");
    m_jsrtTestHooks.pfJsrtDoubleToNumber = (JsrtTestHooks::JsrtDoubleToNumberPtr)GetProcAddress(jscriptLibrary, "JsDoubleToNumber");
    m_jsrtTestHooks.pfJsrtGetExternalData = (JsrtTestHooks::JsrtGetExternalDataPtr)GetProcAddress(jscriptLibrary, "JsGetExternalData");
    m_jsrtTestHooks.pfJsrtCreateArray = (JsrtTestHooks::JsrtCreateArrayPtr)GetProcAddress(jscriptLibrary, "JsCreateArray");
    m_jsrtTestHooks.pfJsrtSetIndexedProperty = (JsrtTestHooks::JsrtSetIndexedPropertyPtr)GetProcAddress(jscriptLibrary, "JsGetIndexedProperty");
    m_jsrtTestHooks.pfJsrtSetException = (JsrtTestHooks::JsrtSetExceptionPtr)GetProcAddress(jscriptLibrary, "JsSetException");
    m_jsrtTestHooks.pfJsrtGetAndClearException = (JsrtTestHooks::JsrtGetAndClearExceptiopnPtr)GetProcAddress(jscriptLibrary, "JsGetAndClearException");
    m_jsrtTestHooks.pfJsrtCreateError = (JsrtTestHooks::JsrtCreateErrorPtr)GetProcAddress(jscriptLibrary, "JsCreateError");
    m_jsrtTestHooks.pfJsrtGetRuntime = (JsrtTestHooks::JsrtGetRuntimePtr)GetProcAddress(jscriptLibrary, "JsGetRuntime");
    m_jsrtTestHooks.pfJsrtCollectGarbage = (JsrtTestHooks::JsrtCollectGarbagePtr)GetProcAddress(jscriptLibrary, "JsCollectGarbage");
    m_jsrtTestHooks.pfJsrtStartDebugging = (JsrtTestHooks::JsrtStartDebuggingPtr)GetProcAddress(jscriptLibrary, "JsStartDebugging");
    m_jsrtTestHooks.pfJsrtRelease = (JsrtTestHooks::JsrtReleasePtr)GetProcAddress(jscriptLibrary, "JsRelease");
    m_jsrtTestHooks.pfJsrtAddRef = (JsrtTestHooks::JsrtAddRefPtr)GetProcAddress(jscriptLibrary, "JsAddRef");
    m_jsrtTestHooks.pfJsrtGetValueType = (JsrtTestHooks::JsrtGetValueType)GetProcAddress(jscriptLibrary, "JsGetValueType");


    m_memProtectTestHooks.pfMemProtectHeapCreate = (MemProtectTestHooks::MemProtectHeapCreatePtr)GetProcAddress(jscriptLibrary, "MemProtectHeapCreate");
    m_memProtectTestHooks.pfMemProtectHeapRootAlloc = (MemProtectTestHooks::MemProtectHeapRootAllocPtr)GetProcAddress(jscriptLibrary, "MemProtectHeapRootAlloc");
    m_memProtectTestHooks.pfMemProtectHeapRootAllocLeaf = (MemProtectTestHooks::MemProtectHeapRootAllocLeafPtr)GetProcAddress(jscriptLibrary, "MemProtectHeapRootAllocLeaf");
    m_memProtectTestHooks.pfMemProtectHeapUnrootAndZero = (MemProtectTestHooks::MemProtectHeapUnrootAndZeroPtr)GetProcAddress(jscriptLibrary, "MemProtectHeapUnrootAndZero");
    m_memProtectTestHooks.pfMemProtectHeapMemSize = (MemProtectTestHooks::MemProtectHeapMemSizePtr)GetProcAddress(jscriptLibrary, "MemProtectHeapMemSize");
    m_memProtectTestHooks.pfMemProtectHeapDestroy = (MemProtectTestHooks::MemProtectHeapDestroyPtr)GetProcAddress(jscriptLibrary, "MemProtectHeapDestroy");
    m_memProtectTestHooks.pfMemProtectHeapCollect = (MemProtectTestHooks::MemProtectHeapCollectPtr)GetProcAddress(jscriptLibrary, "MemProtectHeapCollect");
    m_memProtectTestHooks.pfMemProtectHeapProtectCurrentThread = (MemProtectTestHooks::MemProtectHeapProtectCurrentThreadPtr)GetProcAddress(jscriptLibrary, "MemProtectHeapProtectCurrentThread");
    m_memProtectTestHooks.pfMemProtectHeapUnprotectCurrentThread = (MemProtectTestHooks::MemProtectHeapUnprotectCurrentThreadPtr)GetProcAddress(jscriptLibrary, "MemProtectHeapUnprotectCurrentThread");
    m_memProtectTestHooks.pfMemProtectHeapSynchronizeWithCollector = (MemProtectTestHooks::MemProtectHeapSynchronizeWithCollectorPtr)GetProcAddress(jscriptLibrary, "MemProtectHeapSynchronizeWithCollector");
    
    return jscriptLibrary;
}

HRESULT JScript9Interface::GetThreadService(IActiveScriptGarbageCollector** threadService)
{
    HRESULT hr;
    if (m_testHooks.pfnGetThreadService)
    {
        hr = m_testHooks.pfnGetThreadService(threadService);
        if (SUCCEEDED(hr))
        {
            return hr;
        }
    }
   
    if (!JScript9Interface::SupportsDllGetClassObjectCallback())
    {
        hr = E_NOINTERFACE;
        goto LReturn;
    }

    IClassFactory * jscriptClassFactory = NULL;
    hr = JScript9Interface::DllGetClassObject(CLSID_ChakraThreadService, IID_IClassFactory, (LPVOID*)&jscriptClassFactory);
    IfFailedGo(hr);

    hr = jscriptClassFactory->CreateInstance(NULL, IID_IActiveScriptGarbageCollector, (LPVOID*)threadService);    
    jscriptClassFactory->Release();
    IfFailedGo(hr);

LReturn:
    return hr;
}

HRESULT JScript9Interface::FinalGC()
{    
    if (m_testHooks.pfFinalGC)
    {
        m_testHooks.pfFinalGC();
        return S_OK;
    }

    HRESULT hr = S_OK;
    IActiveScript* activeScript = NULL;
   
    if (!JScript9Interface::SupportsDllGetClassObjectCallback())
    {
        hr = E_NOINTERFACE;
        goto LReturn;
    }

    IClassFactory * jscriptClassFactory = NULL;
    hr = JScript9Interface::DllGetClassObject(CLSID_Chakra, IID_IClassFactory, (LPVOID*)&jscriptClassFactory);
    IfFailedGo(hr);

    hr = jscriptClassFactory->CreateInstance(NULL, _uuidof(IActiveScript), (LPVOID*)&activeScript);    
    jscriptClassFactory->Release();
    IfFailedGo(hr);

    IActiveScriptGarbageCollector * gc;
    activeScript->QueryInterface(IID_IActiveScriptGarbageCollector, (LPVOID*)&gc);
    activeScript->Release();

    IfFailedGo(hr);
    gc->CollectGarbage((SCRIPTGCTYPE)SCRIPTGCTYPE_FORCEINTHREAD);
    gc->Release();    

LReturn:
    return hr;
}

HRESULT JScript9Interface::DisplayRecyclerStats()
{    
    if (m_testHooks.pfDisplayMemStats)
    {
        m_testHooks.pfDisplayMemStats();
        return S_OK;
    }

    return S_FALSE;
}

void JScript9Interface::UnloadDll(HINSTANCE jscriptLibrary)
{
    Assert(jscriptLibrary);
    FARPROC pDllCanUnloadNow = GetProcAddress(jscriptLibrary, "DllCanUnloadNow");
    if (pDllCanUnloadNow != NULL)
    {
        pDllCanUnloadNow();
    }
    FreeLibrary(jscriptLibrary);
    m_testHooksSetup = false;
    m_testHooksInitialized = false;
}

HRESULT JScript9Interface::ParseConfigFlags()
{
    HRESULT hr = S_OK;
    // Pass the command line arguments to the dll
    if (m_testHooks.pfSetAssertToConsoleFlag)
    {
        IfFailedReturn(SetAssertToConsoleFlag(true));
    }
    if (m_testHooks.pfSetConfigFlags)
    {
        IfFailedReturn(SetConfigFlags(m_argInfo.argc, m_argInfo.argv, &HostConfigFlags::flags));
    }

    if (! m_argInfo.filename)
    {
        return S_OK;
    }
    *(m_argInfo.filename) = NULL;
    if (m_testHooks.pfGetFilenameFlag)
    {
        return GetFileNameFlag(m_argInfo.filename);
    }

    *(m_argInfo.filename) = SysAllocString(m_argInfo.argv[m_argInfo.argc-1]);

    return *(m_argInfo.filename) ? S_OK : E_OUTOFMEMORY;
}

HRESULT JScript9Interface::OnJScript9Loaded(TestHooks& testHooks)
{ 
    if (!m_testHooksInitialized)
    {
        m_testHooks = testHooks;
        m_testHooksSetup = true;
        m_testHooksInitialized = true;
        return ParseConfigFlags();
    }
    else
    {
        return S_OK;
    }
}