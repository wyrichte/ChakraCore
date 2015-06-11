// Copyright (C) Microsoft. All rights reserved.
//

#include "stdafx.h"
#include "activprof.h"
#include "ScriptDirectUnitTests.h"

#define PROFILER_HEAP_OBJECT_FLAGS_DUMPED 0x8000

enum DumpMode
{
    DumpHETestOnly,
    FullDump,
    NoDump,
};

DumpMode dumpMode = DumpHETestOnly;
LPCWSTR defaultScript = L"var a = new Array();";
ULONG numSnapshotElements= 0; 
PROFILER_HEAP_OBJECT** pSnapshot = NULL;
UINT maxNameId = 0;
LPCWSTR* pNameIdMap = NULL;
IActiveScriptProfilerHeapEnum* pEnum = NULL;


LPCWSTR GetNameFromId(PROFILER_HEAP_OBJECT_NAME_ID nameId)
{
    return pNameIdMap[nameId];
}

bool IsGlobal(PROFILER_HEAP_OBJECT_NAME_ID nameId)
{
    static PROFILER_HEAP_OBJECT_NAME_ID globalTypeNameId = UINT_MAX;
    if (globalTypeNameId == UINT_MAX)
    {
        for (UINT i=0; i < maxNameId; i++)
        {
            if (wcscmp(GetNameFromId(i), L"GlobalObject") == 0)
            {
                globalTypeNameId = i;
            }
        }
    }
    return nameId == globalTypeNameId;
}

HRESULT VisitObject(std::stringstream& details, ULONG objIndex);

ULONG FindObjectInSnapshot(PROFILER_HEAP_OBJECT_ID objectId)
{
    for (ULONG i = 0; i < numSnapshotElements; i++)
    {
        if (pSnapshot[i]->objectId == objectId)
        {
            return i;
        }
    }
    return ULONG_MAX;
}

PROFILER_HEAP_OBJECT_ID FindHETestObjectInSnapshot()
{
    for (ULONG i = 0; i < numSnapshotElements; i++)
    {
        if (! IsGlobal(pSnapshot[i]->typeNameId))
        {
            continue;
        }
        PROFILER_HEAP_OBJECT& obj = *pSnapshot[i];
        PROFILER_HEAP_OBJECT_OPTIONAL_INFO optionalInfoArray[PROFILER_HEAP_OBJECT_OPTIONAL_INFO_MAX_VALUE];
        pEnum->GetOptionalInfo(&obj, obj.optionalInfoCount, optionalInfoArray);
        for (UINT j=0; j < obj.optionalInfoCount; j++)
        {
            PROFILER_HEAP_OBJECT_OPTIONAL_INFO* optionalInfo = &optionalInfoArray[j];
            if (optionalInfo->infoType != PROFILER_HEAP_OBJECT_OPTIONAL_INFO_NAME_PROPERTIES)
            {
                continue;
            }
            PROFILER_HEAP_OBJECT_RELATIONSHIP_LIST& properties = *optionalInfo->namePropertyList;
            for (UINT k = 0; k < properties.count; k++)
            {
                PROFILER_HEAP_OBJECT_RELATIONSHIP& elem = properties.elements[k];
                if (wcscmp(GetNameFromId(elem.relationshipId), L"HETest") == 0)
                {
                    return elem.objectId;
                }
            }
        }
    }
    return NULL;
}

HRESULT VisitSnapshotObject(std::stringstream& details, PROFILER_HEAP_OBJECT_ID objectId)
{
    ULONG objIndex = FindObjectInSnapshot(objectId);
    if (objIndex == ULONG_MAX)
    {
        return E_FAIL;
    }
    PROFILER_HEAP_OBJECT& obj = *pSnapshot[objIndex];
    if ((obj.flags & PROFILER_HEAP_OBJECT_FLAGS_DUMPED) == 0)
    {
        obj.flags |= PROFILER_HEAP_OBJECT_FLAGS_DUMPED;
        VisitObject(details, objIndex);
    }
    return S_OK;
}

HRESULT VisitElement(std::stringstream& details, PROFILER_HEAP_OBJECT_RELATIONSHIP& elem)
{
    if (LOWORD(elem.relationshipInfo) == PROFILER_PROPERTY_TYPE_HEAP_OBJECT)
    {
        return VisitSnapshotObject(details, elem.objectId);
    }
    if (LOWORD(elem.relationshipInfo) == PROFILER_PROPERTY_TYPE_EXTERNAL_OBJECT)
    {
        return VisitSnapshotObject(details, elem.objectId);
    }
    return S_OK;
}


HRESULT VisitObject(std::stringstream& details, ULONG objIndex)
{
    PROFILER_HEAP_OBJECT& obj = *pSnapshot[objIndex];
    HRESULT hr = S_OK;
    PROFILER_HEAP_OBJECT_OPTIONAL_INFO optionalInfoArray[PROFILER_HEAP_OBJECT_OPTIONAL_INFO_MAX_VALUE];
    pEnum->GetOptionalInfo(&obj, obj.optionalInfoCount, optionalInfoArray);
    for (UINT j=0; j < obj.optionalInfoCount; j++)
    {
        PROFILER_HEAP_OBJECT_OPTIONAL_INFO* optionalInfo = &optionalInfoArray[j];
        switch(optionalInfo->infoType)
        {
            case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_INTERNAL_PROPERTY:
            {
                VisitElement(details, *optionalInfo->internalProperty);
                break;
            }
            case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_PROTOTYPE:
            {
                IfFailGo(VisitSnapshotObject(details, optionalInfo->prototype));
                break;
            }
            case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_FUNCTION_NAME:
            {
                break;
            }
            case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_SCOPE_LIST:
            {
                PROFILER_HEAP_OBJECT_SCOPE_LIST& scopeList = *optionalInfo->scopeList;
                for (UINT k = 0; k < scopeList.count; k++)
                {
                    IfFailGo(VisitSnapshotObject(details, scopeList.scopes[k]));
                } 
                break;
            }
            case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_NAME_PROPERTIES:
            {
                PROFILER_HEAP_OBJECT_RELATIONSHIP_LIST& properties = *optionalInfo->namePropertyList;
                for (UINT k = 0; k < properties.count; k++)
                {
                    PROFILER_HEAP_OBJECT_RELATIONSHIP& elem = properties.elements[k];
                    VisitElement(details, elem);
                }
                break;
            }
            case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_INDEX_PROPERTIES:
            {
                PROFILER_HEAP_OBJECT_RELATIONSHIP_LIST& indexPropertyList = *optionalInfo->indexPropertyList;
                for (UINT k = 0; k < indexPropertyList.count; k++)
                {
                    PROFILER_HEAP_OBJECT_RELATIONSHIP& elem = indexPropertyList.elements[k];
                    VisitElement(details, elem);
                }
                break;
            }
            default:
            {
                details << "*** Error: Unexpected optional info type ***" << endl;
                return E_FAIL;
            }
        }
    }
Error:
    return hr;
}

typedef  HRESULT (*EnumeratorCallback)(std::stringstream&, PROFILER_HEAP_OBJECT&, uint);

HRESULT IterateEnumerator(std::stringstream& testResult, EnumeratorCallback callback=NULL, uint callbackFlags=0)
{
    ULONG totalSize = 0;
    ULONG largest = 0;
    ULONG smallest = 0xFFFFFF;
    std::stringstream details;
    HRESULT hr = S_OK;

    IfFailedReturn(pEnum->GetNameIdMap(&pNameIdMap, &maxNameId));

    const size_t snapshotChunkSize = 1000;
    PROFILER_HEAP_OBJECT* snapshotChunk[snapshotChunkSize];
    
    numSnapshotElements = 0;
    pSnapshot = (PROFILER_HEAP_OBJECT**)malloc(sizeof(PROFILER_HEAP_OBJECT*));
    if (! pSnapshot)
    {
        goto Error;
    }

    do {
        ULONG numFetched = 0;
        hr = pEnum->Next(snapshotChunkSize, snapshotChunk, &numFetched);
        if (numFetched == 0 || FAILED(hr)) break;
        void* newSnapshot = realloc(pSnapshot, sizeof(PROFILER_HEAP_OBJECT*) * (numSnapshotElements + numFetched));
        if (! pSnapshot)
        {
            goto Error;
        }
        pSnapshot = (PROFILER_HEAP_OBJECT**)newSnapshot;
        UINT copySize = sizeof(PROFILER_HEAP_OBJECT*) * numFetched;
        memcpy(pSnapshot + numSnapshotElements, snapshotChunk, copySize);
        numSnapshotElements += numFetched;
    } while (TRUE);

    if (FAILED(hr))
    {
        goto Error;
    }

    if (dumpMode == DumpHETestOnly)
    {
        PROFILER_HEAP_OBJECT_ID HETestObj = FindHETestObjectInSnapshot();
        if (! HETestObj)
        {
            testResult << "*** Error: HETest object missing from snapshot ***";
            hr = E_FAIL;
            goto Error;
        }
        IfFailGo(VisitSnapshotObject(testResult, HETestObj));
    }
    // Regardless of the dump mode, iterate through the entire snapshot and dump everything to catch
    // any issues in the snapshot. We'll not actally dump it to testResults unless FullDump is set. 
    for (ULONG i=0; i < numSnapshotElements; i++)
    {
        PROFILER_HEAP_OBJECT& obj = *pSnapshot[i];
        totalSize += obj.size;
        if (obj.size > largest)
        {
            largest = obj.size;
        }
        if (obj.size < smallest)
        {
            smallest = obj.size;
        }
        details << endl << setfill(' ') << setw(4) << dec << i << " "; 
        IfFailGo(VisitSnapshotObject(details, pSnapshot[i]->objectId));
        if (callback)
        {
            IfFailGo(callback(details, *pSnapshot[i], callbackFlags));
        }
    }
Error:
    if (FAILED(hr) || dumpMode == FullDump)
    {
        testResult << details.str();
    }
    return hr;
}

void ClearSnapshot()
{
    if (pSnapshot)
    {
        pEnum->FreeObjectAndOptionalInfo(numSnapshotElements, pSnapshot);
        free(pSnapshot);
        pEnum->Release();
        pSnapshot = NULL;
    }
    if (pNameIdMap)
    {
        CoTaskMemFree(pNameIdMap);
        pNameIdMap = NULL;
    }
}

HRESULT PopulateSnapshot(MyScriptDirectTests* myTests, bool newEngine, LPCWSTR srcFileName, std::stringstream& testResult)
{
    void* jsHostScriptSite = NULL;
    HRESULT hr = S_OK;
    {
        CComPtr<IActiveScriptDirect> scriptDirect = NULL;
        CComPtr<IActiveScriptParse> scriptParse = NULL;
        CComPtr<IActiveScriptProfilerControl3> profilerControl = NULL;
        if (newEngine == false)
        {
            scriptDirect = myTests->GetScriptDirectNoRef();
        }
        else
        {
            IActiveScriptDirect * scriptDirectTemp = myTests->CreateNewEngine(&jsHostScriptSite, true);
            if (scriptDirectTemp == NULL) { hr = E_FAIL; goto Error; }            
            scriptDirect = scriptDirectTemp;
            scriptDirectTemp->Release();
            IfFailGo(scriptDirect->QueryInterface(__uuidof(IActiveScriptParse), (LPVOID*)&scriptParse));            
        }
        IfFailGo(scriptDirect->QueryInterface(__uuidof(IActiveScriptProfilerControl3), (void**)&profilerControl));

        if (srcFileName != NULL)
        {
            if (dumpMode == FullDump) 
            {
                testResult << endl << "Loading from file: " << WStringToString(srcFileName) << endl << endl;
            }
            IfFailGo(myTests->LoadScriptFromFile(srcFileName, jsHostScriptSite));
        }
        else if (scriptParse)
        {
            EXCEPINFO excepInfo;
            VARIANT result;
            IfFailGo(scriptParse->ParseScriptText(defaultScript, NULL, NULL, NULL, 0xdeadbeef, 0, SCRIPTTEXT_HOSTMANAGESSOURCE, &result, &excepInfo));
        }
        else
        {
            myTests->ParseAndExecute(defaultScript);
        }

        myTests->CollectGarbage(SCRIPTGCTYPE_NORMAL);
                
        if (SUCCEEDED(hr = profilerControl->EnumHeap(&pEnum)))
        {
            hr = IterateEnumerator(testResult);            
        }
    }
Error:
    // ClearSnapshot needs to be called before ShutdownScriptSite because ShutdownScriptSite will delete the arena used by ClearSnapshot.
    ClearSnapshot();
    if (jsHostScriptSite) myTests->ShutdownScriptSite(jsHostScriptSite);
    return hr;
}

void StartTest(MyScriptDirectTests* myTests, std::stringstream& testResult, LPCSTR testName)
{
    testResult << "Running test: " << testName << endl;
    myTests->Start();
}

void CompleteTest(MyScriptDirectTests* myTests, std::stringstream& testResult, LPCSTR testName, HRESULT hr)
{
    ClearSnapshot();
    if (FAILED(hr))
    {
        testResult << endl << testName << " FAILED with HR " << hex << hr << endl;
    }
    Print(testResult.str(), SUCCEEDED(hr));
    myTests->End();
}

void RunOneTest(MyScriptDirectTests* myTests, LPCWSTR srcFileName)
{
    std::stringstream testResult;
    StartTest(myTests, testResult, "RunOneTest");
    HRESULT hr = PopulateSnapshot(myTests, false, srcFileName, testResult);
    CompleteTest(myTests, testResult, "RunOneTest", hr);
}

void DoPrivateDumpHeap(MyScriptDirectTests* myTests)
{
    std::stringstream testResult;
    StartTest(myTests, testResult, "DoPrivateDumpHeap");

    HRESULT hr = S_OK;
    CComPtr<IActiveScriptParse> scriptParse = NULL;
    IActiveScriptDirect* scriptDirect = myTests->GetScriptDirectNoRef();

    myTests->ParseAndExecute(defaultScript);
    myTests->CollectGarbage(SCRIPTGCTYPE_NORMAL);
                
    CComPtr<IHeapDumper> heapDumper = NULL;
    hr = scriptDirect->QueryInterface(__uuidof(IHeapDumper), (void**)&heapDumper);
    if (SUCCEEDED(hr))
    {
        WCHAR tempPath[MAX_PATH];
        WCHAR outputFile[MAX_PATH];
        
        DWORD retVal = GetTempPath(MAX_PATH, tempPath);

        hr = E_FAIL;
        if (retVal <= MAX_PATH && retVal > 0)
        {
            retVal = GetTempFileName(tempPath, L"PDH", 0, outputFile);
            if (retVal != 0)
            {
                hr = heapDumper->DumpHeap(outputFile, HeapDumperDumpOld, TRUE, FALSE);
                retVal = DeleteFile(outputFile);
                if (retVal == 0 && GetLastError() == ERROR_FILE_NOT_FOUND)
                {
                    hr = E_FAIL;
                }
            }
        }
    }
    CompleteTest(myTests, testResult, "DoPrivateDumpHeap", hr);
}

void DoOneEnumWithNewEngine(MyScriptDirectTests* myTests)
{
    if (dumpMode != FullDump)
    {
        dumpMode = NoDump;
    }
    std::stringstream testResult;
    StartTest(myTests, testResult, "DoOneEnumWithNewEngine");
    HRESULT hr = PopulateSnapshot(myTests, true, NULL, testResult);
    CompleteTest(myTests, testResult, "DoOneEnumWithNewEngine", hr);
}

enum DoMultipleEnumsWithinOneEngineFlags
{
    VerifyAllNewObjects,
    VerifyAllOldObjects
};

HRESULT DoMultipleEnumsWithinOneEngineCallback(std::stringstream& details, PROFILER_HEAP_OBJECT& obj, uint flags)
{
    if (flags == VerifyAllNewObjects && (obj.flags & PROFILER_HEAP_OBJECT_FLAGS_NEW_OBJECT) == 0)
    {
        details << "*** Error: object should be new and isn't" << endl;
        return E_FAIL;
    }
    else if (flags == VerifyAllOldObjects && (obj.flags & PROFILER_HEAP_OBJECT_FLAGS_NEW_OBJECT) != 0)
    {
        details << "*** Error: object should be old and isn't" << endl;
        return E_FAIL;
    }
    return S_OK;
}

void DoMultipleEnumsWithinOneEngine(MyScriptDirectTests* myTests)
{
    if (dumpMode != FullDump)
    {
        dumpMode = NoDump;
    }
    std::stringstream testResult;
    StartTest(myTests, testResult, "DoMultipleEnumsWithinOneEngine");
    myTests->ParseAndExecute(defaultScript);
    IActiveScriptProfilerControl3* profilerControl = NULL;
    IActiveScriptDirect* scriptDirect = myTests->GetScriptDirectNoRef();
	HRESULT hr = E_FAIL;
    if (SUCCEEDED(hr=scriptDirect->QueryInterface(__uuidof(IActiveScriptProfilerControl3), (void**)&profilerControl)))
    {
        ULONG previousEnumCount = 0;
        uint callbackFlag = VerifyAllNewObjects;
        for (int i = 0; i < 3; i++)
        {
            if (FAILED(hr = profilerControl->EnumHeap(&pEnum)))
            {
                break;
            }
            if (FAILED(hr = IterateEnumerator(testResult, DoMultipleEnumsWithinOneEngineCallback, callbackFlag)))
            {
                break;
            }
            callbackFlag = VerifyAllOldObjects;
            if (previousEnumCount > 0 && numSnapshotElements != previousEnumCount)
            {
                testResult << "numSnapshotElements( " << numSnapshotElements << ") != previousEnumCount (" << previousEnumCount << ")" << endl;
                hr = E_FAIL;
                break;
            }
            previousEnumCount = numSnapshotElements;
            ClearSnapshot();
        }
        profilerControl->Release();
    }
    CompleteTest(myTests, testResult, "DoMultipleEnumsWithinOneEngine", hr);
}

void DoMultipleEnumsAcrossEngines(MyScriptDirectTests* myTests)
{
    void* jsHostScriptSite = NULL;
    IActiveScriptDirect* scriptDirect = NULL;
    IActiveScriptParse* scriptParse = NULL;
    HRESULT hr = E_FAIL;

    if (dumpMode != FullDump)
    {
        dumpMode = NoDump;
    }
    std::stringstream testResult;
    StartTest(myTests, testResult, "DoMultipleEnumsAcrossEngines");
    IActiveScriptProfilerControl3* profilerControl = NULL;
    int i = 0;
    for (i = 0; i < 3; i++)
    {
        if ((scriptDirect = myTests->CreateNewEngine(&jsHostScriptSite, true)) == NULL) { hr = E_FAIL; break; }
        if (FAILED(scriptDirect->QueryInterface(__uuidof(IActiveScriptParse), (LPVOID*)&scriptParse))) { break; }

        EXCEPINFO excepInfo;
        VARIANT result;
        if (FAILED(scriptParse->ParseScriptText(defaultScript, NULL, NULL, NULL, 0xdeadbeef, 0, SCRIPTTEXT_HOSTMANAGESSOURCE, &result, &excepInfo))) { break; }        
        if (FAILED(hr= scriptDirect->QueryInterface(__uuidof(IActiveScriptProfilerControl3), (void**)&profilerControl))) { break; }
        if (FAILED(hr = profilerControl->EnumHeap(&pEnum))) { break; }
        if (FAILED(hr = IterateEnumerator(testResult))) { break; }
        ClearSnapshot();
        profilerControl->Release();
        profilerControl = NULL;
        scriptParse->Release();
        scriptParse = NULL;
        scriptDirect->Release();
        scriptDirect = NULL;
        myTests->ShutdownScriptSite(jsHostScriptSite);
        jsHostScriptSite = NULL;
    }
    if (scriptParse) { scriptParse->Release(); }
    if (profilerControl) { profilerControl->Release(); }
    if (scriptDirect) { scriptDirect->Release(); }
    if (jsHostScriptSite) { myTests->ShutdownScriptSite(jsHostScriptSite); }
    CompleteTest(myTests, testResult, "DoMultipleEnumsAcrossEngines", hr);
}

void DoHeapSummary(MyScriptDirectTests* myTests)
{
    std::stringstream testResult;
    StartTest(myTests, testResult, "DoHeapSummary");
    myTests->ParseAndExecute(defaultScript);
    IActiveScriptProfilerControl4* profilerControl = NULL;
    IActiveScriptDirect* scriptDirect = myTests->GetScriptDirectNoRef();
	HRESULT hr = E_FAIL;
    if (SUCCEEDED(hr=scriptDirect->QueryInterface(__uuidof(IActiveScriptProfilerControl4), (void**)&profilerControl)))
    {
        PROFILER_HEAP_SUMMARY heapSummary;
        heapSummary.version = PROFILER_HEAP_SUMMARY_VERSION_1;
        if (! FAILED(hr = profilerControl->SummarizeHeap(&heapSummary)))
        {
            if (heapSummary.totalHeapSize == 0 || heapSummary.totalHeapSize > 200000)
            {
                testResult << "totalHeapSize( " << heapSummary.totalHeapSize << " is 0 or greater than 200,000" << endl;
                hr = E_FAIL;
            }
        }
        profilerControl->Release();
    }
    CompleteTest(myTests, testResult, "DoHeapSummary", hr);
}

void RunHeapEnumTest(MyScriptDirectTests* myTests)
{
    int flagCount = 0;
    LPWSTR* flags = NULL;
    myTests->GetFlags(&flagCount, &flags);
    LPCWSTR srcFileNameFullPath = NULL;
    if (flagCount > 1)
    {
        int curFlag = 1;
        if (_wcsicmp(flags[1], L"-dump") == 0)
        {
            dumpMode = FullDump;
            ++curFlag;
        }
        // the last unmatched flag will be the source name
        if (curFlag < flagCount)
        {
            srcFileNameFullPath = flags[flagCount-1];
        }
    }

    WCHAR srcFileDrive[_MAX_DRIVE];
    WCHAR srcFileDir[_MAX_DIR];
    WCHAR srcFileName[_MAX_FNAME];
    if (srcFileNameFullPath)
    {
        _wsplitpath_s(srcFileNameFullPath, srcFileDrive, _countof(srcFileDrive), srcFileDir, _countof(srcFileDir), srcFileName, _countof(srcFileName), NULL, 0);
    }
    if (srcFileNameFullPath && _wcsicmp(srcFileName, L"DoOneEnumWithNewEngine") == 0)
    {
        DoOneEnumWithNewEngine(myTests);
    }
    else if (srcFileNameFullPath && _wcsicmp(srcFileName, L"DoMultipleEnumsWithinOneEngine") == 0)
    {
        DoMultipleEnumsWithinOneEngine(myTests);
    }
    else if (srcFileNameFullPath && _wcsicmp(srcFileName, L"DoMultipleEnumsAcrossEngines") == 0)
    {
        DoMultipleEnumsAcrossEngines(myTests);
    }
    else if (srcFileNameFullPath && _wcsicmp(srcFileName, L"DoPrivateDumpHeap") == 0)
    {
        DoPrivateDumpHeap(myTests);
    }
    else if (srcFileNameFullPath && _wcsicmp(srcFileName, L"DoHeapSummary") == 0)
    {
        DoHeapSummary(myTests);
    }
    else 
    {    
        RunOneTest(myTests, srcFileNameFullPath);
    }
 }
