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

HINSTANCE JScript9Interface::LoadDll(bool useRetailDllName, LPCWSTR alternateDllName, ArgInfo& argInfo)
{
    m_argInfo = argInfo;

    // If using an alternate dll name do not try default nor fallback.
    // If using retail dll name, only try chakra.dll and use normal LoadLibrary search.
    // Otherwise first try chakratest.dll then chakra.dll and only look for them in the same directory as jshost.exe.

    bool useDefault = alternateDllName == nullptr && !useRetailDllName;
    LPCWSTR dllName;

    wchar_t filename[_MAX_PATH];
    wchar_t drive[_MAX_DRIVE];
    wchar_t dir[_MAX_DIR];

    if (!useDefault)
    {
        dllName = alternateDllName != nullptr ? alternateDllName : L"chakra.dll";
    }
    else
    {
        wchar_t modulename[_MAX_PATH];
        GetModuleFileName(NULL, modulename, _MAX_PATH);
        _wsplitpath_s(modulename, drive, _MAX_DRIVE, dir, _MAX_DIR, nullptr, 0, nullptr, 0);
        _wmakepath_s(filename, drive, dir, L"chakratest.dll", nullptr);
        dllName = filename;
    }

    HINSTANCE chakraLibrary = LoadLibraryEx(dllName, nullptr, 0);

    if (!chakraLibrary)
    {
        // Clear out the test hooks since DLL didn't load properly but may have loaded the hooks and we don't want to accidentally use them (eg for FaultInjection dump)
        memset((void*)&m_testHooks, 0, sizeof(m_testHooks));
        m_testHooksSetup = false;

        if (!useDefault)
        {
            int ret = GetLastError();
            fwprintf(stderr, L"FATAL ERROR: Unable to load %s. GLE=0x%x\n", dllName, ret);
            fflush(stderr);
            return nullptr;
        }
        else
        {
            _wmakepath_s(filename, drive, dir, L"chakra.dll", nullptr);
            chakraLibrary = LoadLibraryEx(filename, nullptr, 0);

            if (!chakraLibrary)
            {
                int ret = GetLastError();
                fwprintf(stderr, L"FATAL ERROR: chakratest.dll nor chakra.dll found next to jshost.exe. GLE=0x%x\n", ret);
                fflush(stderr);
                return nullptr;
            }
        }
    }

    Assert(chakraLibrary);

    if (!m_testHooksSetup)
    {
        m_freeChakraLoaded = true;

        m_testHooks.pfDllGetClassObject = (TestHooks::DllGetClassObjectPtr)GetProcAddress(chakraLibrary, "DllGetClassObject");
        m_testHooks.pfJsVarToScriptDirect = (TestHooks::JsVarToScriptDirectPtr)GetProcAddress(chakraLibrary, "JsVarToScriptDirect");
        m_testHooks.pfJsVarAddRef = (TestHooks::JsVarAddRefPtr)GetProcAddress(chakraLibrary, "JsVarAddRef");
        m_testHooks.pfJsVarRelease = (TestHooks::JsVarReleasePtr)GetProcAddress(chakraLibrary, "JsVarRelease");
        m_testHooks.pfJsVarToExtension = (TestHooks::JsVarToExtensionPtr)GetProcAddress(chakraLibrary, "JsVarToExtension");
    }

    m_jsrtTestHooks.pfJsrtCreateRuntime = (JsrtTestHooks::JsrtCreateRuntimePtr)GetProcAddress(chakraLibrary, "JsCreateRuntime");
    m_jsrtTestHooks.pfJsrtCreateContext = (JsrtTestHooks::JsrtCreateContextPtr)GetProcAddress(chakraLibrary, "JsCreateContext");
    m_jsrtTestHooks.pfJsrtSetCurrentContext = (JsrtTestHooks::JsrtSetCurrentContextPtr)GetProcAddress(chakraLibrary, "JsSetCurrentContext");
    m_jsrtTestHooks.pfJsrtGetCurrentContext = (JsrtTestHooks::JsrtGetCurrentContextPtr)GetProcAddress(chakraLibrary, "JsGetCurrentContext");
    m_jsrtTestHooks.pfJsrtDisposeRuntime = (JsrtTestHooks::JsrtDisposeRuntimePtr)GetProcAddress(chakraLibrary, "JsDisposeRuntime");
    m_jsrtTestHooks.pfJsrtCreateObject = (JsrtTestHooks::JsrtCreateObjectPtr)GetProcAddress(chakraLibrary, "JsCreateObject");
    m_jsrtTestHooks.pfJsrtCreateExternalObject = (JsrtTestHooks::JsrtCreateExternalObjectPtr)GetProcAddress(chakraLibrary, "JsCreateExternalObject");
    m_jsrtTestHooks.pfJsrtCreateFunction = (JsrtTestHooks::JsrtCreateFunctionPtr)GetProcAddress(chakraLibrary, "JsCreateFunction");
    m_jsrtTestHooks.pfJsrtPointerToString = (JsrtTestHooks::JsrtPointerToStringPtr)GetProcAddress(chakraLibrary, "JsPointerToString");
    m_jsrtTestHooks.pfJsrtSetProperty = (JsrtTestHooks::JsrtSetPropertyPtr)GetProcAddress(chakraLibrary, "JsSetProperty");
    m_jsrtTestHooks.pfJsrtGetGlobalObject = (JsrtTestHooks::JsrtGetGlobalObjectPtr)GetProcAddress(chakraLibrary, "JsGetGlobalObject");
    m_jsrtTestHooks.pfJsrtGetUndefinedValue = (JsrtTestHooks::JsrtGetUndefinedValuePtr)GetProcAddress(chakraLibrary, "JsGetUndefinedValue");
    m_jsrtTestHooks.pfJsrtGetTrueValue = (JsrtTestHooks::JsrtGetUndefinedValuePtr)GetProcAddress(chakraLibrary, "JsGetTrueValue");
    m_jsrtTestHooks.pfJsrtGetFalseValue = (JsrtTestHooks::JsrtGetUndefinedValuePtr)GetProcAddress(chakraLibrary, "JsGetFalseValue");
    m_jsrtTestHooks.pfJsrtConvertValueToString = (JsrtTestHooks::JsrtConvertValueToStringPtr)GetProcAddress(chakraLibrary, "JsConvertValueToString");
    m_jsrtTestHooks.pfJsrtConvertValueToNumber = (JsrtTestHooks::JsrtConvertValueToNumberPtr)GetProcAddress(chakraLibrary, "JsConvertValueToNumber");
    m_jsrtTestHooks.pfJsrtConvertValueToBoolean = (JsrtTestHooks::JsrtConvertValueToBooleanPtr)GetProcAddress(chakraLibrary, "JsConvertValueToBoolean");
    m_jsrtTestHooks.pfJsrtStringToPointer = (JsrtTestHooks::JsrtStringToPointerPtr)GetProcAddress(chakraLibrary, "JsStringToPointer");
    m_jsrtTestHooks.pfJsrtBooleanToBool = (JsrtTestHooks::JsrtBooleanToBoolPtr)GetProcAddress(chakraLibrary, "JsBooleanToBool");
    m_jsrtTestHooks.pfJsrtGetPropertyIdFromName = (JsrtTestHooks::JsrtGetPropertyIdFromNamePtr)GetProcAddress(chakraLibrary, "JsGetPropertyIdFromName");
    m_jsrtTestHooks.pfJsrtGetProperty = (JsrtTestHooks::JsrtGetPropertyPtr)GetProcAddress(chakraLibrary, "JsGetProperty");
    m_jsrtTestHooks.pfJsrtHasProperty = (JsrtTestHooks::JsrtHasPropertyPtr)GetProcAddress(chakraLibrary, "JsHasProperty");
    m_jsrtTestHooks.pfJsrtRunScript = (JsrtTestHooks::JsrtRunScriptPtr)GetProcAddress(chakraLibrary, "JsRunScript");    
    m_jsrtTestHooks.pfJsrtCallFunction = (JsrtTestHooks::JsrtCallFunctionPtr)GetProcAddress(chakraLibrary, "JsCallFunction");
    m_jsrtTestHooks.pfJsrtNumbertoDouble = (JsrtTestHooks::JsrtNumberToDoublePtr)GetProcAddress(chakraLibrary, "JsNumberToDouble");
    m_jsrtTestHooks.pfJsrtDoubleToNumber = (JsrtTestHooks::JsrtDoubleToNumberPtr)GetProcAddress(chakraLibrary, "JsDoubleToNumber");
    m_jsrtTestHooks.pfJsrtGetExternalData = (JsrtTestHooks::JsrtGetExternalDataPtr)GetProcAddress(chakraLibrary, "JsGetExternalData");
    m_jsrtTestHooks.pfJsrtCreateArray = (JsrtTestHooks::JsrtCreateArrayPtr)GetProcAddress(chakraLibrary, "JsCreateArray");
    m_jsrtTestHooks.pfJsrtSetIndexedProperty = (JsrtTestHooks::JsrtSetIndexedPropertyPtr)GetProcAddress(chakraLibrary, "JsGetIndexedProperty");
    m_jsrtTestHooks.pfJsrtSetException = (JsrtTestHooks::JsrtSetExceptionPtr)GetProcAddress(chakraLibrary, "JsSetException");
    m_jsrtTestHooks.pfJsrtGetAndClearException = (JsrtTestHooks::JsrtGetAndClearExceptiopnPtr)GetProcAddress(chakraLibrary, "JsGetAndClearException");
    m_jsrtTestHooks.pfJsrtCreateError = (JsrtTestHooks::JsrtCreateErrorPtr)GetProcAddress(chakraLibrary, "JsCreateError");
    m_jsrtTestHooks.pfJsrtGetRuntime = (JsrtTestHooks::JsrtGetRuntimePtr)GetProcAddress(chakraLibrary, "JsGetRuntime");
    m_jsrtTestHooks.pfJsrtCollectGarbage = (JsrtTestHooks::JsrtCollectGarbagePtr)GetProcAddress(chakraLibrary, "JsCollectGarbage");
    m_jsrtTestHooks.pfJsrtStartDebugging = (JsrtTestHooks::JsrtStartDebuggingPtr)GetProcAddress(chakraLibrary, "JsStartDebugging");
    m_jsrtTestHooks.pfJsrtRelease = (JsrtTestHooks::JsrtReleasePtr)GetProcAddress(chakraLibrary, "JsRelease");
    m_jsrtTestHooks.pfJsrtAddRef = (JsrtTestHooks::JsrtAddRefPtr)GetProcAddress(chakraLibrary, "JsAddRef");
    m_jsrtTestHooks.pfJsrtGetValueType = (JsrtTestHooks::JsrtGetValueType)GetProcAddress(chakraLibrary, "JsGetValueType");


    m_memProtectTestHooks.pfMemProtectHeapCreate = (MemProtectTestHooks::MemProtectHeapCreatePtr)GetProcAddress(chakraLibrary, "MemProtectHeapCreate");
    m_memProtectTestHooks.pfMemProtectHeapRootAlloc = (MemProtectTestHooks::MemProtectHeapRootAllocPtr)GetProcAddress(chakraLibrary, "MemProtectHeapRootAlloc");
    m_memProtectTestHooks.pfMemProtectHeapRootAllocLeaf = (MemProtectTestHooks::MemProtectHeapRootAllocLeafPtr)GetProcAddress(chakraLibrary, "MemProtectHeapRootAllocLeaf");
    m_memProtectTestHooks.pfMemProtectHeapUnrootAndZero = (MemProtectTestHooks::MemProtectHeapUnrootAndZeroPtr)GetProcAddress(chakraLibrary, "MemProtectHeapUnrootAndZero");
    m_memProtectTestHooks.pfMemProtectHeapMemSize = (MemProtectTestHooks::MemProtectHeapMemSizePtr)GetProcAddress(chakraLibrary, "MemProtectHeapMemSize");
    m_memProtectTestHooks.pfMemProtectHeapDestroy = (MemProtectTestHooks::MemProtectHeapDestroyPtr)GetProcAddress(chakraLibrary, "MemProtectHeapDestroy");
    m_memProtectTestHooks.pfMemProtectHeapCollect = (MemProtectTestHooks::MemProtectHeapCollectPtr)GetProcAddress(chakraLibrary, "MemProtectHeapCollect");
    m_memProtectTestHooks.pfMemProtectHeapProtectCurrentThread = (MemProtectTestHooks::MemProtectHeapProtectCurrentThreadPtr)GetProcAddress(chakraLibrary, "MemProtectHeapProtectCurrentThread");
    m_memProtectTestHooks.pfMemProtectHeapUnprotectCurrentThread = (MemProtectTestHooks::MemProtectHeapUnprotectCurrentThreadPtr)GetProcAddress(chakraLibrary, "MemProtectHeapUnprotectCurrentThread");
    m_memProtectTestHooks.pfMemProtectHeapSynchronizeWithCollector = (MemProtectTestHooks::MemProtectHeapSynchronizeWithCollectorPtr)GetProcAddress(chakraLibrary, "MemProtectHeapSynchronizeWithCollector");
    
    return chakraLibrary;
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