//
// TODO JenH: Move this to real unit test when add native test support to mshtmlhost
//

#include "stdafx.h"
#include "activprof.h"
#include "edgescriptdirect.h"

#ifndef IFFAILRET
#define IFFAILRET(expr) do {if (FAILED(hr = (expr))) return hr; } while (0)
#endif


HRESULT __stdcall GetHeapObjectInfo(Var instance, HostProfilerHeapObject** heapObjOut, HeapObjectInfoReturnResult& returnResult)
{        
    HostProfilerHeapObject** externalObjects = 0;

    CComPtr<IActiveScriptDirect> scriptDirectRef; 
    HRESULT hr;
    IFFAILRET(JScript9Interface::JsVarToScriptDirect(instance, &scriptDirectRef));
    Var globalObject;
    IFFAILRET(scriptDirectRef->GetGlobalObject(&globalObject));
    CComPtr<IJavascriptOperations> jsOperations;
    PropertyId dummyFastDomVarPid;
    IFFAILRET(scriptDirectRef->GetOrAddPropertyId(_u("dummyFastDomVar"), &dummyFastDomVarPid));
    IFFAILRET(scriptDirectRef->GetJavascriptOperations(&jsOperations));
    Var dummyFastDomVar;
    IFFAILRET(jsOperations->GetProperty(scriptDirectRef, globalObject, dummyFastDomVarPid, &dummyFastDomVar));
    PropertyId DomHeapObjectType;
    IFFAILRET(scriptDirectRef->GetOrAddPropertyId(_u("DOMObject"), &DomHeapObjectType));

    USHORT optionalInfoCount = 3;
    UINT headerAllocSize =  offsetof(HostProfilerHeapObject, optionalInfo);
    UINT elementAttributesAllocSize = offsetof(ProfilerHeapObjectOptionalInfo, elementAttributesSize) + sizeof(((ProfilerHeapObjectOptionalInfo*)0)->elementAttributesSize);
    UINT elementTextChildrenAllocSize = offsetof(ProfilerHeapObjectOptionalInfo, elementTextChildrenSize) + sizeof(((ProfilerHeapObjectOptionalInfo*)0)->elementTextChildrenSize);
    USHORT relationshipCount = 2;
    UINT relationshipAllocSize = offsetof(ProfilerHeapObjectOptionalInfo, relationshipList.elements) + sizeof(PROFILER_HEAP_OBJECT_RELATIONSHIP)*(relationshipCount);

    SIZE_T allocSize = headerAllocSize + elementAttributesAllocSize + elementTextChildrenAllocSize + relationshipAllocSize;
    HostProfilerHeapObject* heapObj = (HostProfilerHeapObject*)CoTaskMemAlloc(allocSize);
    if (! heapObj)
    {
        goto Error;
    }
    
    memset(heapObj, 0, allocSize);
    heapObj->objectId = (PROFILER_HEAP_OBJECT_ID)instance;
    heapObj->size = 222;
    heapObj->typeNameId = DomHeapObjectType;
    heapObj->optionalInfoCount = optionalInfoCount;
    ProfilerHeapObjectOptionalInfo* optionalInfoNext = (ProfilerHeapObjectOptionalInfo*)(&heapObj->optionalInfo);

    optionalInfoNext->infoType = PROFILER_HEAP_OBJECT_OPTIONAL_INFO_ELEMENT_ATTRIBUTES_SIZE;
    optionalInfoNext->elementAttributesSize = 555;
    optionalInfoNext = (ProfilerHeapObjectOptionalInfo*)((char *)optionalInfoNext + elementAttributesAllocSize);

    optionalInfoNext->infoType = PROFILER_HEAP_OBJECT_OPTIONAL_INFO_ELEMENT_TEXT_CHILDREN_SIZE;
    optionalInfoNext->elementTextChildrenSize = 666;
    optionalInfoNext = (ProfilerHeapObjectOptionalInfo*)((char *)optionalInfoNext + elementTextChildrenAllocSize);

    optionalInfoNext->infoType = PROFILER_HEAP_OBJECT_OPTIONAL_INFO_RELATIONSHIPS;
    optionalInfoNext->relationshipList.count = relationshipCount;
    optionalInfoNext->relationshipList.elements[0].relationshipId = dummyFastDomVarPid;
    optionalInfoNext->relationshipList.elements[0].relationshipInfo = PROFILER_PROPERTY_TYPE_HEAP_OBJECT;
    optionalInfoNext->relationshipList.elements[0].objectId = (PROFILER_HEAP_OBJECT_ID)dummyFastDomVar;

    static UINT callCount = 0;
    optionalInfoNext->relationshipList.elements[1].relationshipId = dummyFastDomVarPid;
    optionalInfoNext->relationshipList.elements[1].relationshipInfo = PROFILER_PROPERTY_TYPE_EXTERNAL_OBJECT;
    optionalInfoNext->relationshipList.elements[1].externalObjectAddress = (PROFILER_EXTERNAL_OBJECT_ADDRESS)(0xdeadbeef + callCount);

    USHORT const externalObjectCount = 1;
    allocSize = sizeof(HostProfilerHeapObject*) * externalObjectCount;
    externalObjects = (HostProfilerHeapObject**)CoTaskMemAlloc(allocSize);
    if (!externalObjects) goto Error;
    memset(externalObjects, 0, allocSize);
    allocSize = sizeof(HostProfilerHeapObject);
    externalObjects[0] = (HostProfilerHeapObject*)CoTaskMemAlloc(allocSize);
    if (! externalObjects[0]) goto Error;
    memset(externalObjects[0], 0, allocSize);
    externalObjects[0]->flags = PROFILER_HEAP_OBJECT_FLAGS_EXTERNAL | PROFILER_HEAP_OBJECT_FLAGS_NEW_STATE_UNAVAILABLE;
    externalObjects[0]->flags |= (callCount==0 ? PROFILER_HEAP_OBJECT_FLAGS_SIZE_UNAVAILABLE: PROFILER_HEAP_OBJECT_FLAGS_SIZE_APPROXIMATE);
    externalObjects[0]->externalAddress = (PROFILER_EXTERNAL_OBJECT_ADDRESS)(0xdeadbeef + callCount);
    externalObjects[0]->typeNameId = PROFILER_HEAP_OBJECT_NAME_ID_UNAVAILABLE;
    externalObjects[0]->size = callCount++;
    heapObj->externalObjectCount = externalObjectCount;
    heapObj->externalObjects = externalObjects;
    *heapObjOut = heapObj;
    returnResult = HeapObjectInfoReturnResult_Success;
    return S_OK;

Error:
    if (externalObjects)
    {
        for (UINT i=0; i < externalObjectCount; i++)
        {
            if (externalObjects[i]) CoTaskMemFree(externalObjects[i]);
        }
        CoTaskMemFree(externalObjects);
    }
    if (heapObj)
    {
        CoTaskMemFree(heapObj);
    }

    returnResult = HeapObjectInfoReturnResult_NoResult;
    return E_OUTOFMEMORY;
}