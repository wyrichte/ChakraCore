//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "EnginePch.h"
#include "JsrtExternalObject.h"
#include "Library\JavascriptStringObject.h"
#include "Library\BoundFunction.h"
#include "Library\JavascriptSymbol.h"
#include "Library\SameValueComparer.h"
#include "Library\MapOrSetDataList.h"
#include "Library\JavascriptMap.h"
#include "Library\JavascriptSet.h"
#include "Library\JavascriptWeakMap.h"
#include "Library\JavascriptWeakSet.h"
#include "Library\JavascriptArrayIterator.h"
#include "Library\JavascriptMapIterator.h"
#include "Library\JavascriptSetIterator.h"
#include "Library\JavascriptStringIterator.h"
#include "Library\ArgumentsObject.h"
#include "Library\ES5Array.h"
#include "ActiveScriptProfilerHeapEnum.h"

#ifdef ENABLE_PROJECTION
#include "ProjectionWriter.h"
#include "namespaceProjection.h"
#endif

//#define VALIDATE_OPTIONAL_INFO_ALLOCATIONS

// ToDo: Assert that these don't collide with PROFILER_HEAP_OBJECT_FLAGS.
const ULONG ActiveScriptProfilerHeapEnum::PROFILER_HEAP_OBJECT_INTERNAL_FLAGS_SCOPE_SLOT_ARRAY = 0x80000000;
const ULONG ActiveScriptProfilerHeapEnum::PROFILER_HEAP_OBJECT_INTERNAL_FLAGS_BOUND_FUNCTION_ARGUMENT_LIST = 0x40000000;
const ULONG ActiveScriptProfilerHeapEnum::INTERNAL_PROFILER_HEAP_OBJECT_INTERNAL_FLAGS_MASK =
    PROFILER_HEAP_OBJECT_INTERNAL_FLAGS_SCOPE_SLOT_ARRAY |
    PROFILER_HEAP_OBJECT_INTERNAL_FLAGS_BOUND_FUNCTION_ARGUMENT_LIST;

const ULONG ActiveScriptProfilerHeapEnum::PROFILER_RELATIONSHIP_INFO_MASK     = 0x0000ffff;
const ULONG ActiveScriptProfilerHeapEnum::PROFILER_RELATIONSHIP_FLAGS_MASK    = 0xffff0000;

const LPCWSTR c_functionRelationshipNames[] = { _u("sourceUrl"), _u("sourceRow"), _u("sourceCol") };
const LPCWSTR c_dataViewRelationshipNames[] = { _u("buffer"), _u("byteOffset"), _u("byteLength") };

#ifdef ENABLE_TEST_HOOKS
ActiveScriptProfilerHeapEnum::GetHeapObjectInfoPtr ActiveScriptProfilerHeapEnum::pfGetHeapObjectInfo = NULL;
#endif

#ifdef HEAP_ENUMERATION_VALIDATION
const ULONG ActiveScriptProfilerHeapEnum::PROFILER_HEAP_OBJECT_INTERNAL_FLAGS_UNREPORTED_LIBRARY_OBJECT = 0x20000000; // *** If you change this, change the value in DebugObject.cpp ***
const ULONG ActiveScriptProfilerHeapEnum::PROFILER_HEAP_OBJECT_INTERNAL_FLAGS_UNREPORTED_USER_OBJECT = 0x10000000; // *** If you change this, change the value in DebugObject.cpp ***
int ActiveScriptProfilerHeapEnum::libObjectCount = 0;
int ActiveScriptProfilerHeapEnum::userObjectCount = 0;
ActiveScriptProfilerHeapEnum* ActiveScriptProfilerHeapEnum::currentEnumerator = NULL;
#endif

const ActiveScriptProfilerHeapEnum::InternalTypeIdMap ActiveScriptProfilerHeapEnum::internalTypeIdMap[] = {
    { Js::TypeIds_Undefined, HeapObjectType_Undefined},
    { Js::TypeIds_Null, HeapObjectType_Null},
    { Js::TypeIds_Boolean, HeapObjectType_Boolean},
    { Js::TypeIds_Integer, HeapObjectType_Number},
    { Js::TypeIds_Number, HeapObjectType_Number},
    { Js::TypeIds_Int64Number, HeapObjectType_Number},
    { Js::TypeIds_UInt64Number, HeapObjectType_Number},
    { Js::TypeIds_String, HeapObjectType_String},
    { Js::TypeIds_Symbol, HeapObjectType_Symbol },
    { Js::TypeIds_Enumerator, HeapObjectType_Invalid},
    { Js::TypeIds_VariantDate, HeapObjectType_GetVarDateFunctionObject },
    { Js::TypeIds_SIMDFloat32x4, HeapObjectType_SIMD },
    { Js::TypeIds_SIMDFloat64x2, HeapObjectType_SIMD },
    { Js::TypeIds_SIMDInt32x4, HeapObjectType_SIMD },
    { Js::TypeIds_SIMDInt16x8, HeapObjectType_SIMD },
    { Js::TypeIds_SIMDInt8x16, HeapObjectType_SIMD },
    { Js::TypeIds_SIMDUint32x4, HeapObjectType_SIMD },
    { Js::TypeIds_SIMDUint16x8, HeapObjectType_SIMD },
    { Js::TypeIds_SIMDUint8x16, HeapObjectType_SIMD },
    { Js::TypeIds_SIMDBool32x4, HeapObjectType_SIMD },
    { Js::TypeIds_SIMDBool16x8, HeapObjectType_SIMD },
    { Js::TypeIds_SIMDBool8x16, HeapObjectType_SIMD },
    { Js::TypeIds_HostDispatch, HeapObjectType_HostObject },
    { Js::TypeIds_WithScopeObject, HeapObjectType_Scope },
    { Js::TypeIds_UndeclBlockVar, HeapObjectType_Invalid},
    { Js::TypeIds_Proxy, HeapObjectType_ProxyObject},
    { Js::TypeIds_Function, HeapObjectType_FunctionObject},
    { Js::TypeIds_Object, HeapObjectType_ObjectObject},
    { Js::TypeIds_Array, HeapObjectType_ArrayObject},
    { Js::TypeIds_NativeIntArray, HeapObjectType_ArrayObject},
    { Js::TypeIds_CopyOnAccessNativeIntArray, HeapObjectType_ArrayObject },
    { Js::TypeIds_NativeFloatArray, HeapObjectType_ArrayObject},
    { Js::TypeIds_ES5Array, HeapObjectType_ArrayObject},
    { Js::TypeIds_Date, HeapObjectType_DateObject},
    { Js::TypeIds_RegEx, HeapObjectType_RegexObject},
    { Js::TypeIds_Error, HeapObjectType_ErrorObject},
    { Js::TypeIds_BooleanObject, HeapObjectType_BooleanObject},
    { Js::TypeIds_NumberObject, HeapObjectType_NumberObject},
    { Js::TypeIds_StringObject, HeapObjectType_StringObject},
    { Js::TypeIds_SIMDObject, HeapObjectType_SIMDObject },
    { Js::TypeIds_Arguments, HeapObjectType_ArgumentObject},
    { Js::TypeIds_ArrayBuffer, HeapObjectType_ArrayBuffer},
    { Js::TypeIds_Int8Array, HeapObjectType_TypedArrayObject},
    { Js::TypeIds_Uint8Array, HeapObjectType_TypedArrayObject},
    { Js::TypeIds_Uint8ClampedArray, HeapObjectType_TypedArrayObject},
    { Js::TypeIds_Int16Array, HeapObjectType_TypedArrayObject},
    { Js::TypeIds_Uint16Array, HeapObjectType_TypedArrayObject},
    { Js::TypeIds_Int32Array, HeapObjectType_TypedArrayObject},
    { Js::TypeIds_Uint32Array, HeapObjectType_TypedArrayObject},
    { Js::TypeIds_Float32Array, HeapObjectType_TypedArrayObject},
    { Js::TypeIds_Float64Array, HeapObjectType_TypedArrayObject},
    { Js::TypeIds_Int64Array, HeapObjectType_TypedArrayObject},
    { Js::TypeIds_Uint64Array, HeapObjectType_TypedArrayObject},
    { Js::TypeIds_CharArray, HeapObjectType_TypedArrayObject},
    { Js::TypeIds_BoolArray, HeapObjectType_TypedArrayObject},
    { Js::TypeIds_EngineInterfaceObject, HeapObjectType_ObjectObject},
    { Js::TypeIds_DataView, HeapObjectType_DataView},
    { Js::TypeIds_WinRTDate, HeapObjectType_DateObject },
    { Js::TypeIds_Map, HeapObjectType_MapObject},
    { Js::TypeIds_Set, HeapObjectType_SetObject},
    { Js::TypeIds_WeakMap, HeapObjectType_WeakMapObject},
    { Js::TypeIds_WeakSet, HeapObjectType_WeakSetObject},
    { Js::TypeIds_SymbolObject, HeapObjectType_SymbolObject },
    { Js::TypeIds_ArrayIterator, HeapObjectType_ArrayIterator },
    { Js::TypeIds_MapIterator, HeapObjectType_MapIterator },
    { Js::TypeIds_SetIterator, HeapObjectType_SetIterator },
    { Js::TypeIds_StringIterator, HeapObjectType_StringIterator },
    { Js::TypeIds_JavascriptEnumeratorIterator, HeapObjectType_EnumeratorIterator },
    { Js::TypeIds_Generator, HeapObjectType_Generator },
    { Js::TypeIds_Promise, HeapObjectType_Promise },
    { Js::TypeIds_SharedArrayBuffer, HeapObjectType_SharedArrayBuffer },
    { Js::TypeIds_WebAssemblyModule, HeapObjectType_WebAssemblyModule },
    { Js::TypeIds_WebAssemblyInstance, HeapObjectType_WebAssemblyInstance },
    { Js::TypeIds_WebAssemblyMemory, HeapObjectType_WebAssemblyMemory },
    { Js::TypeIds_WebAssemblyTable, HeapObjectType_WebAssemblyTable },
    { Js::TypeIds_GlobalObject, HeapObjectType_GlobalObject},
    { Js::TypeIds_ModuleRoot, HeapObjectType_FormObject},
    { Js::TypeIds_HostObject, HeapObjectType_HostObject},
    { Js::TypeIds_ActivationObject, HeapObjectType_Scope},
    { Js::TypeIds_SpreadArgument, HeapObjectType_ObjectObject },
    { Js::TypeIds_ModuleNamespace, HeapObjectType_ModuleNamespace },
    { Js::TypeIds_ListIterator, HeapObjectType_ListIterator },
    { Js::TypeIds_ExternalIterator, HeapObjectType_ExternalIterator },
};

#if DBG
static bool validationDone = false;
void ActiveScriptProfilerHeapEnum::ValidateTypeMap()
{
    if (validationDone)
    {
        return;
    }
    validationDone = true;
    Assert(_countof(internalTypeIdMap) == Js::TypeIds_Limit);
    for (int i=0; i < _countof(internalTypeIdMap); i++)
    {
        Assert(internalTypeIdMap[i].typeId == i);
    }
};
#endif

PROFILER_HEAP_OBJECT_NAME_ID ActiveScriptProfilerHeapEnum::GetPropertyId(LPCWSTR propertyName)
{
    return (PROFILER_HEAP_OBJECT_NAME_ID)m_scriptEngine.GetScriptContext()->GetOrAddPropertyIdTracked(propertyName, wcslen(propertyName));
}

// Property Ids is unique to a script engine, so can't use a static table
void ActiveScriptProfilerHeapEnum::CreateTypeNameIds()
{
    typeNameIdMap[HeapObjectType_Undefined].typeNameId = GetPropertyId(_u("Undefined"));
    typeNameIdMap[HeapObjectType_Null].typeNameId = GetPropertyId(_u("Null"));
    typeNameIdMap[HeapObjectType_Boolean].typeNameId = GetPropertyId(_u("Boolean"));
    typeNameIdMap[HeapObjectType_Number].typeNameId = GetPropertyId(_u("Number"));
    typeNameIdMap[HeapObjectType_String].typeNameId = GetPropertyId(_u("String"));
    typeNameIdMap[HeapObjectType_ArgumentObject].typeNameId = GetPropertyId(_u("ArgumentObject"));
    typeNameIdMap[HeapObjectType_ArrayObject].typeNameId = GetPropertyId(_u("ArrayObject"));
    typeNameIdMap[HeapObjectType_ArrayBuffer].typeNameId = GetPropertyId(_u("ArrayBuffer"));
    typeNameIdMap[HeapObjectType_BooleanObject].typeNameId = GetPropertyId(_u("BooleanObject"));
    typeNameIdMap[HeapObjectType_DataView].typeNameId = GetPropertyId(_u("DataView"));
    typeNameIdMap[HeapObjectType_DateObject].typeNameId = GetPropertyId(_u("DateObject"));
    typeNameIdMap[HeapObjectType_ErrorObject].typeNameId = GetPropertyId(_u("ErrorObject"));
    typeNameIdMap[HeapObjectType_GetVarDateFunctionObject].typeNameId = GetPropertyId(_u("GetVarDateFunctionObject"));
    typeNameIdMap[HeapObjectType_FunctionObject].typeNameId = GetPropertyId(_u("FunctionObject"));
    typeNameIdMap[HeapObjectType_NumberObject].typeNameId = GetPropertyId(_u("NumberObject"));
    typeNameIdMap[HeapObjectType_ObjectObject].typeNameId = GetPropertyId(_u("ObjectObject"));
    typeNameIdMap[HeapObjectType_RegexObject].typeNameId = GetPropertyId(_u("RegexObject"));
    typeNameIdMap[HeapObjectType_StringObject].typeNameId = GetPropertyId(_u("StringObject"));
    typeNameIdMap[HeapObjectType_TypedArrayObject].typeNameId = GetPropertyId(_u("TypedArrayObject"));
    typeNameIdMap[HeapObjectType_GlobalObject].typeNameId = GetPropertyId(_u("GlobalObject"));
    typeNameIdMap[HeapObjectType_FormObject].typeNameId = GetPropertyId(_u("FormObject"));
    typeNameIdMap[HeapObjectType_Scope].typeNameId = GetPropertyId(_u("Scope"));
    typeNameIdMap[HeapObjectType_HostObject].typeNameId = GetPropertyId(_u("HostObject"));
    typeNameIdMap[HeapObjectType_DOM].typeNameId = GetPropertyId(_u("DOMObject"));
    typeNameIdMap[HeapObjectType_WinRT].typeNameId = GetPropertyId(WinRTObjectType);
    typeNameIdMap[HeapObjectType_MapObject].typeNameId = GetPropertyId(_u("MapObject"));
    typeNameIdMap[HeapObjectType_SetObject].typeNameId = GetPropertyId(_u("SetObject"));
    typeNameIdMap[HeapObjectType_WeakMapObject].typeNameId = GetPropertyId(_u("WeakMapObject"));
    typeNameIdMap[HeapObjectType_WeakSetObject].typeNameId = GetPropertyId(_u("WeakSetObject"));
    typeNameIdMap[HeapObjectType_JsrtExternalObject].typeNameId = GetPropertyId(_u("ExternalObject"));
    typeNameIdMap[HeapObjectType_Symbol].typeNameId = GetPropertyId(_u("Symbol"));
    typeNameIdMap[HeapObjectType_SymbolObject].typeNameId = GetPropertyId(_u("SymbolObject"));
    typeNameIdMap[HeapObjectType_ProxyObject].typeNameId = GetPropertyId(_u("Proxy"));
    typeNameIdMap[HeapObjectType_ArrayIterator].typeNameId = GetPropertyId(_u("ArrayIterator"));
    typeNameIdMap[HeapObjectType_MapIterator].typeNameId = GetPropertyId(_u("MapIterator"));
    typeNameIdMap[HeapObjectType_SetIterator].typeNameId = GetPropertyId(_u("SetIterator"));
    typeNameIdMap[HeapObjectType_StringIterator].typeNameId = GetPropertyId(_u("StringIterator"));
    typeNameIdMap[HeapObjectType_ListIterator].typeNameId = GetPropertyId(_u("ListIterator"));
    typeNameIdMap[HeapObjectType_Generator].typeNameId = GetPropertyId(_u("Generator"));
    typeNameIdMap[HeapObjectType_Promise].typeNameId = GetPropertyId(_u("Promise"));
    typeNameIdMap[HeapObjectType_SharedArrayBuffer].typeNameId = GetPropertyId(_u("SharedArrayBuffer"));
    typeNameIdMap[HeapObjectType_WebAssemblyModule].typeNameId = GetPropertyId(_u("WebAssemblyModule"));
    typeNameIdMap[HeapObjectType_WebAssemblyInstance].typeNameId = GetPropertyId(_u("WebAssemblyInstance"));
    typeNameIdMap[HeapObjectType_WebAssemblyMemory].typeNameId = GetPropertyId(_u("WebAssemblyMemory"));
    typeNameIdMap[HeapObjectType_WebAssemblyTable].typeNameId = GetPropertyId(_u("WebAssemblyTable"));
    typeNameIdMap[HeapObjectType_EnumeratorIterator].typeNameId = GetPropertyId(_u("ReflectIterator"));
    typeNameIdMap[HeapObjectType_SIMD].typeNameId = GetPropertyId(_u("SIMD"));
    typeNameIdMap[HeapObjectType_SIMDObject].typeNameId = GetPropertyId(_u("SIMDObject"));
    typeNameIdMap[HeapObjectType_ModuleNamespace].typeNameId = GetPropertyId(_u("ModuleNamespace"));
    typeNameIdMap[HeapObjectType_ExternalIterator].typeNameId = GetPropertyId(_u("ExternalIterator"));
}

PROFILER_HEAP_OBJECT_NAME_ID ActiveScriptProfilerHeapEnum::GetTypeNameId(ProfilerHeapObjectType objectType)
{
    Assert(objectType <= HeapObjectType_Last);
    __analysis_assume(objectType <= HeapObjectType_Last);
    return typeNameIdMap[objectType].typeNameId;
}

PROFILER_HEAP_OBJECT_NAME_ID ActiveScriptProfilerHeapEnum::GetTypeNameId(Js::RecyclableObject* obj)
{
#if DBG
    ValidateTypeMap();
#endif
    if (JsrtExternalObject::Is(obj))
    {
        return GetTypeNameId(HeapObjectType_JsrtExternalObject);
    }
    if (! Js::ExternalObject::Is(obj))
    {
        if (Projection::IsWinRTConstructorFunction(obj))
        {
            return GetTypeNameId(HeapObjectType_WinRT);
        }
        Assert(obj->GetTypeId() <= _countof(internalTypeIdMap));
        ProfilerHeapObjectType externalTypeId = internalTypeIdMap[obj->GetTypeId()].profilerType;

        Assert(externalTypeId != HeapObjectType_Invalid);
        return GetTypeNameId(externalTypeId);
    }
    if (! Js::CustomExternalObject::Is(obj))
    {
        return GetTypeNameId(HeapObjectType_HostObject);
    }
    else if (IsWinRTType(Js::CustomExternalObject::FromVar(obj)))
    {
        return GetTypeNameId(HeapObjectType_WinRT);
    }
    return GetTypeNameId(HeapObjectType_DOM);
}

DEFINE_IUNKNOWN(ActiveScriptProfilerHeapEnum, m_cref);

STDMETHODIMP ActiveScriptProfilerHeapEnum::QueryInterface(REFIID riid, void **ppv)
{
    CHECK_POINTER(ppv);
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IActiveScriptProfilerHeapEnum)))
    {
        *ppv = (IActiveScriptProfilerHeapEnum *)this;
    }
    else
    {
        *ppv = NULL;
        return HR(E_NOINTERFACE);
    }
    AddRef();
    return NOERROR;
}

ActiveScriptProfilerHeapEnum::ActiveScriptProfilerHeapEnum(Recycler& recycler, ScriptEngine& scriptEngine, ArenaAllocator* allocator, bool preEnumHeap2Behavior, PROFILER_HEAP_ENUM_FLAGS enumFlags) :
    m_cref(1),
    m_scriptEngine(scriptEngine),
    m_recycler(recycler),
    m_scanQueue(allocator),
    isClosed(FALSE),
    isInitialized(FALSE),
    m_preEnumHeap2Behavior(preEnumHeap2Behavior),
    m_enumFlags(enumFlags),
    m_autoSetupRecyclerForNonCollectingMark(NULL)
#ifdef HEAP_ENUMERATION_VALIDATION
    ,enumerationCount(0)
#endif
{
    m_scriptEngine.GetScriptSite(__uuidof(IHeapEnumHost), (void**)&m_heapEnumHost); // OK not to support IHeapEnumHost
#ifdef HEAP_ENUMERATION_VALIDATION
    if (Js::Configuration::Global.flags.ValidateHeapEnum)
    {
        m_vtableMap.Initialize();
    }
#endif
}

ActiveScriptProfilerHeapEnum::~ActiveScriptProfilerHeapEnum()
{
    Assert(m_cref == 0);
    if  (m_autoSetupRecyclerForNonCollectingMark)
    {
        delete m_autoSetupRecyclerForNonCollectingMark;
        m_autoSetupRecyclerForNonCollectingMark = NULL;
    }

    while (!m_scanQueue.Empty())
    {
        HeapScanQueueElement e = m_scanQueue.Head();
        m_scanQueue.RemoveHead();
        Assert(e.m_profHeapObject);
        FreeObjectAndOptionalInfo(e.m_profHeapObject);
    }
    if (isInitialized && !isClosed)
    {
        m_scriptEngine.GetScriptContext()->ClearHeapEnum();
    }
}

STDMETHODIMP ActiveScriptProfilerHeapEnum::Summarize(PROFILER_HEAP_SUMMARY* pHeapSummary)
{
    if (pHeapSummary == NULL)
    {
        return E_POINTER;
    }
    if (pHeapSummary->version != PROFILER_HEAP_SUMMARY_VERSION_1)
    {
        return E_INVALIDARG;
    }
    pHeapSummary->totalHeapSize = 0;

    HRESULT hr = S_OK;
    BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT
    {
        EnumerateHeap([](ProfilerHeapObject* snapshotObj) -> bool {
            AssertMsg(FALSE, "Function should never be called");
            return false;
        }, pHeapSummary);
    }
    END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);
    if (FAILED(hr))
    {
        return hr;
    }
    return S_OK;
}

STDMETHODIMP ActiveScriptProfilerHeapEnum::Init()
{
    if (m_heapEnumHost)
    {
        HRESULT hr = m_heapEnumHost->BeginEnumeration();
        IFFAILRET(hr);
    }
    HRESULT hr = S_OK;

    m_scriptEngine.GetScriptContext()->SetHeapEnum(this);
    BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT
    {
        CreateTypeNameIds();
        m_autoSetupRecyclerForNonCollectingMark = new Recycler::AutoSetupRecyclerForNonCollectingMark(m_recycler, true);
        if (! m_autoSetupRecyclerForNonCollectingMark)
        {
            Js::Throw::OutOfMemory();
        }
        m_autoSetupRecyclerForNonCollectingMark->SetupForHeapEnumeration();
        // We only enumerate the global object for the current script engine. Trident is responsible for reporting
        // iframe relationships etc from that global
        Js::GlobalObject* globalObject = m_scriptEngine.GetScriptContext()->GetGlobalObject();
        Visit(globalObject, PROFILER_HEAP_OBJECT_FLAGS_IS_ROOT);

        // iterate through the JavascriptDispatch objects that we've handed out to Trident
        // eg. as event handlers on an ActiveX object, as these won't necessarily show
        // up anywhere else and they could have circular references
        m_scriptEngine.GetScriptSiteHolder()->ReportJavascriptDispatchObjects([this](JavascriptDispatch* javascriptDispatch) {
            VisitRoot(javascriptDispatch);
        });

        // Iterate through projection wrappers that were given to WinRT and holding onto heap objects/external objects
        auto projectionContext = m_scriptEngine.GetProjectionContext();
        if (projectionContext != nullptr && projectionContext->GetExistingProjectionWriter() != nullptr)
        {
            projectionContext->GetExistingProjectionWriter()->ReportUnknownImpls(this);
        }
    }
    END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);
    if (SUCCEEDED(hr))
    {
        //m_scriptEngine.GetScriptContext()->SetHeapEnum(this);
        isInitialized = TRUE;
#ifdef HEAP_ENUMERATION_VALIDATION
        ++enumerationCount;
#endif
    }
    else
    {
        m_scriptEngine.GetScriptContext()->SetHeapEnum(nullptr);
    }
    return hr;
}

STDMETHODIMP ActiveScriptProfilerHeapEnum::Next(__in ULONG celt,
    __out_ecount_part(celt, *pceltFetched) PROFILER_HEAP_OBJECT* elements[],
    __out ULONG *pceltFetched)
{
    if (celt == 0)
    {
        if (pceltFetched != NULL)
        {
            *pceltFetched = 0;
        }

        return S_OK;
    }

    IfNullReturnError(elements, E_POINTER);
    *elements = nullptr;

    //  If user only requests a single element, we do not require non-NULL
    //  pceltFetched as return values of S_FALSE vs. S_OK are sufficient
    //  to indicate the count of items fetched (either 0 or 1, respectively)
    if ((pceltFetched == NULL) && (celt > 1))
    {
        return E_POINTER;
    }
    HRESULT hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }

    ULONG i = 0;
    Js::ScriptContext *scriptContext = m_scriptEngine.GetScriptContext();

    // If the heap enumerator is not yet set, ensure it is there for external objects to use.
    if (scriptContext->GetHeapEnum() == nullptr)
    {
        scriptContext->SetHeapEnum(this);
    }

    BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT
    {
        EnumerateHeap([this, &elements, &i, celt](ProfilerHeapObject* snapshotObj) -> bool {
            elements[i++] = snapshotObj->AsPublicFacing();
            return (i < celt) ? true : false; //whether should keep going or not
        });
    }
    END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);
    if (FAILED(hr))
    {
        return hr;
    }
    if (pceltFetched != NULL)
    {
        *pceltFetched = i;
    }

    return (i < celt) ? S_FALSE : S_OK;
}

template <typename T>
void ActiveScriptProfilerHeapEnum::GetOptionalInfo(T* obj, ULONG celt, PROFILER_HEAP_OBJECT_OPTIONAL_INFO elements[])
{
    ProfilerHeapObjectOptionalInfo* optionalInfo = (ProfilerHeapObjectOptionalInfo*)&obj->optionalInfo;
    for (ULONG i=0; i < celt; i++)
    {
        elements[i].infoType = optionalInfo->infoType;
        switch(optionalInfo->infoType)
        {
        case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_FUNCTION_NAME:
            {
                elements[i].functionName = optionalInfo->functionName;
                break;
            }
        case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_PROTOTYPE:
            {
                elements[i].prototype = optionalInfo->prototype;
                break;
            }
        case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_ELEMENT_ATTRIBUTES_SIZE:
            {
                elements[i].elementAttributesSize = optionalInfo->elementAttributesSize;
                break;
            }
        case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_ELEMENT_TEXT_CHILDREN_SIZE:
            {
                elements[i].elementTextChildrenSize = optionalInfo->elementTextChildrenSize;
                break;
            }
        default:
            {
                // Everything else is a pointer, so just copy address in
                elements[i].scopeList = &(optionalInfo->scopeList);
                break;
            }
        }
        optionalInfo = GetNextOptionalInfo(optionalInfo);
    }
}

STDMETHODIMP ActiveScriptProfilerHeapEnum::GetOptionalInfo(__in PROFILER_HEAP_OBJECT* objectIn,
    __in ULONG celt,
    __out_ecount(celt) PROFILER_HEAP_OBJECT_OPTIONAL_INFO elements[])
{
    if (celt == 0)
    {
        return S_OK;
    }

    IfNullReturnError(elements, ERROR_INVALID_PARAMETER);
    ZeroMemory(elements, celt * sizeof(PROFILER_HEAP_OBJECT_OPTIONAL_INFO));

    if (objectIn == nullptr)
    {
        return ERROR_INVALID_PARAMETER;
    }
    ProfilerHeapObject* obj = ProfilerHeapObject::AsInternal(objectIn);
    if (celt > obj->OptionalInfoCountJsAndHost())
    {
        return ERROR_INVALID_PARAMETER;
    }

    HRESULT hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }

    GetOptionalInfo(&obj->jsInfo, obj->OptionalInfoCountJSOnly(), elements);
    if (obj->hostInfo)
    {
        GetOptionalInfo(obj->hostInfo, obj->hostInfo->optionalInfoCount, elements+obj->OptionalInfoCountJSOnly());
    }
    return S_OK;
}

STDMETHODIMP ActiveScriptProfilerHeapEnum::FreeObjectAndOptionalInfo(__in ULONG celt,
    __in_ecount(celt) PROFILER_HEAP_OBJECT* elements[])
{
    if (celt == 0)
    {
        return S_OK;
    }
    if (elements == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }
    for (ULONG i=0; i < celt; i++)
    {
        FreeObjectAndOptionalInfo(ProfilerHeapObject::AsInternal(elements[i]));
        elements[i] = NULL;
    }
    return S_OK;
}

void ActiveScriptProfilerHeapEnum::FreeBSTR(HostProfilerHeapObject *externalObject)
{
    for (UINT i = 0; i < externalObject->optionalInfoCount; i ++)
    {
        ProfilerHeapObjectOptionalInfo* optionalInfo = (ProfilerHeapObjectOptionalInfo*)&externalObject->optionalInfo;
        if (optionalInfo->infoType == PROFILER_HEAP_OBJECT_OPTIONAL_INFO_RELATIONSHIPS)
        {
            for (UINT j = 0; j < optionalInfo->relationshipList.count; j ++)
            {
                PROFILER_HEAP_OBJECT_RELATIONSHIP &relationship = optionalInfo->relationshipList.elements[j];
                if (IsRelationshipInfoType(relationship, PROFILER_RELATIONSHIP_INFO::PROFILER_PROPERTY_TYPE_BSTR))
                {
                    SysFreeString((BSTR)relationship.objectId);
                }
            }
        }
        optionalInfo = GetNextOptionalInfo(optionalInfo);
    }
}

void ActiveScriptProfilerHeapEnum::FreeSubString(ProfilerHeapObject* obj)
{
    USHORT optionalInfoCountJSOnly = obj->OptionalInfoCountJSOnly();
    if (optionalInfoCountJSOnly > 0)
    {
        ProfilerHeapObjectOptionalInfo* optionalInfo = (ProfilerHeapObjectOptionalInfo*)&obj->jsInfo.optionalInfo;
        while ((optionalInfo != nullptr) && (optionalInfoCountJSOnly-- > 0))
        {
            if (optionalInfo->infoType == PROFILER_HEAP_OBJECT_OPTIONAL_INFO_INTERNAL_PROPERTY)
            {
                PROFILER_HEAP_OBJECT_RELATIONSHIP internalProp = optionalInfo->internalProperty;
                if (IsRelationshipInfoType(internalProp, PROFILER_RELATIONSHIP_INFO::PROFILER_PROPERTY_TYPE_SUBSTRING))
                {
                    CoTaskMemFree(internalProp.subString);
                    internalProp.subString = NULL;
                }
            }
            optionalInfo = GetNextOptionalInfo(optionalInfo);
        }
    }
}


void ActiveScriptProfilerHeapEnum::FreeHostObjectExternalObjectList(HostProfilerHeapObject& hostInfo)
{
    FreeBSTR(&hostInfo);
    if (hostInfo.externalObjectCount == 0)
    {
        return;
    }

    for (UINT i=0; i < hostInfo.externalObjectCount; i++)
    {
        FreeBSTR(hostInfo.externalObjects[i]);
        CoTaskMemFree(hostInfo.externalObjects[i]);
    }
    CoTaskMemFree(hostInfo.externalObjects);
    hostInfo.externalObjects = NULL;
    hostInfo.externalObjectCount = 0;
}

void ActiveScriptProfilerHeapEnum::FreeObjectAndOptionalInfo(ProfilerHeapObject* obj)
{
    if (this->m_enumFlags & PROFILER_HEAP_ENUM_FLAGS_SUBSTRINGS)
    {
        FreeSubString(obj);
    }
    if (obj->hostInfo)
    {
        FreeHostObjectExternalObjectList(*(obj->hostInfo));
        CoTaskMemFree(obj->hostInfo);
        obj->hostInfo = NULL;
    }
    CoTaskMemFree(obj);
}

ActiveScriptProfilerHeapEnum::ProfilerHeapObject* ActiveScriptProfilerHeapEnum::CreateElement(void* obj, size_t size, ULONG flags, UINT numberOfElements)
{
    ProfilerHeapObject* element = NULL;
    if (IsScopeSlotArray(flags))
    {
        element = CreateScopeSlotArrayElement((void**)obj, size);
    }
    else if (IsBoundFunctionArgs(flags))
    {
        element = CreateBoundFunctionArgsElement((Var*)obj, size, numberOfElements);
    }
    else if (! IsSiteClosed(flags))
    {
        element = CreateObjectElement(Js::RecyclableObject::FromVar(obj)->GetThisObjectOrUnWrap(), size);
    }
    else
    {
        element = AllocateElement(sizeof(ProfilerHeapObject), GetTypeNameId(HeapObjectType_ObjectObject));
        element->jsInfo.size = size;
    }
    element->jsInfo.objectId = (PROFILER_HEAP_OBJECT_ID)obj;
    element->jsInfo.flags = element->jsInfo.flags | flags;
    return element;
}

ActiveScriptProfilerHeapEnum::ProfilerHeapObject* ActiveScriptProfilerHeapEnum::CreateBoundFunctionArgsElement(Var* args, UINT allocatedSizeInGC, UINT numberOfElements)
{
    Assert(numberOfElements > 0);
    UINT propertiesAllocSize = offsetof(ProfilerHeapObjectOptionalInfo, indexPropertyList.elements) + sizeof(PROFILER_HEAP_OBJECT_RELATIONSHIP)*(numberOfElements);
    UINT headerAllocSize =  ProfilerHeapObject::AllocHeaderSize();

    UINT allocSize = headerAllocSize + propertiesAllocSize;
    ProfilerHeapObject* element = AllocateElement(allocSize, GetTypeNameId(HeapObjectType_ArgumentObject));

    element->jsInfo.size = allocatedSizeInGC;
    element->jsInfo.optionalInfoCount = 1;
    ProfilerHeapObjectOptionalInfo* optionalInfoNext = (ProfilerHeapObjectOptionalInfo*)(&element->jsInfo.optionalInfo);

    optionalInfoNext->infoType = PROFILER_HEAP_OBJECT_OPTIONAL_INFO_INDEX_PROPERTIES;
    PROFILER_HEAP_OBJECT_RELATIONSHIP_LIST& properties = optionalInfoNext->indexPropertyList;

    properties.count = numberOfElements;
    for (UINT i = 0; i < numberOfElements; i++)
    {
        properties.elements[i].relationshipId = i;
        FillProperty(m_scriptEngine.GetScriptContext(), args[i], properties.elements[i]);
    }

    return element;
}

ActiveScriptProfilerHeapEnum::ProfilerHeapObject* ActiveScriptProfilerHeapEnum::CreateScopeSlotArrayElement(void** scopeSlotArray, UINT allocatedSizeInGC)
{
    UINT propertiesAllocSize;
    LPCWSTR functionName = NULL;
    UINT functionNameAllocSize = 0;
    USHORT optionalInfoCount = 1;
    PropertyId * propertyIds = NULL;
    Js::DebuggerScope::DebuggerScopePropertyList* propertyList = NULL;

    Js::ScopeSlots slotArray(scopeSlotArray);

    UINT propertyCount = 0;
    if (!slotArray.IsDebuggerScopeSlotArray())
    {
        Js::ParseableFunctionInfo* pfi = slotArray.GetFunctionInfo()->GetParseableFunctionInfo();
        Assert(pfi);

        propertyCount = slotArray.GetCount();
        propertiesAllocSize = offsetof(ProfilerHeapObjectOptionalInfo, namePropertyList.elements) + sizeof(PROFILER_HEAP_OBJECT_RELATIONSHIP)*(propertyCount);

        functionName = pfi->GetExternalDisplayName();
        propertyIds = pfi->GetPropertyIdsForScopeSlotArray();
    }
    else
    {
        Js::DebuggerScope* debuggerScope = slotArray.GetDebuggerScope();
        Assert(debuggerScope);

        propertyList = debuggerScope->scopeProperties;
        AssertMsg(propertyList, "Should never get a null property list since empty scopes are removed during byte code generation.");
        propertyCount = propertyList ? propertyList->Count() : 0;

        propertiesAllocSize = offsetof(ProfilerHeapObjectOptionalInfo, namePropertyList.elements) + sizeof(PROFILER_HEAP_OBJECT_RELATIONSHIP)*(propertyCount);
    }

    Assert(propertyCount <= Js::ScopeSlots::MaxEncodedSlotCount);
    if (propertyCount == Js::ScopeSlots::MaxEncodedSlotCount)
    {
        UINT maxPropertyCount = (allocatedSizeInGC / sizeof(scopeSlotArray[0])) - Js::ScopeSlots::FirstSlotIndex;
        // When the slot array is smaller than the minimum heap size, we can't tell the length. It's initialized to zero, so use
        // that to find the end of the slot array.

        for (propertyCount = 0; propertyCount < maxPropertyCount && scopeSlotArray[propertyCount + Js::ScopeSlots::FirstSlotIndex] != 0; propertyCount++);
        Assert(propertyCount != 0);
    }

    UINT headerAllocSize =  ProfilerHeapObject::AllocHeaderSize();

    if (functionName)
    {
        optionalInfoCount ++;
        functionNameAllocSize = offsetof(ProfilerHeapObjectOptionalInfo, functionName) + sizeof(((ProfilerHeapObjectOptionalInfo*)0)->functionName);
    }

    UINT allocSize = headerAllocSize + propertiesAllocSize + functionNameAllocSize;
    ProfilerHeapObject* element = AllocateElement(allocSize, GetTypeNameId(HeapObjectType_Scope));

    element->jsInfo.size = allocatedSizeInGC;
    element->jsInfo.optionalInfoCount = optionalInfoCount;
    ProfilerHeapObjectOptionalInfo* optionalInfoNext = (ProfilerHeapObjectOptionalInfo*)(&element->jsInfo.optionalInfo);

    if (functionNameAllocSize > 0)
    {
        optionalInfoNext->infoType = PROFILER_HEAP_OBJECT_OPTIONAL_INFO_FUNCTION_NAME;
        optionalInfoNext->functionName = functionName;
        optionalInfoNext = (ProfilerHeapObjectOptionalInfo*)((char *)optionalInfoNext + functionNameAllocSize);
    }

    optionalInfoNext->infoType = PROFILER_HEAP_OBJECT_OPTIONAL_INFO_NAME_PROPERTIES;
    PROFILER_HEAP_OBJECT_RELATIONSHIP_LIST& properties = optionalInfoNext->namePropertyList;

    properties.count = propertyCount;

    UINT elementIndex = 0;

    for (UINT j = 0; j < propertyCount; j++)
    {
        UINT slotIndex;
        PropertyId propertyId;

        if (!slotArray.IsDebuggerScopeSlotArray())
        {
            AssertMsg(propertyIds, "Property ID array was not found for the function slot array.");
            Assert(propertyIds[j] != Js::Constants::NoProperty);
            propertyId = propertyIds[j];
            slotIndex = j;
        }
        else
        {
            slotIndex = propertyList->Item(j).location;
            Assert(slotIndex != Js::Constants::NoRegister);
            AssertMsg(propertyList, "Property list was not found for the debugger scope slot array.");

            propertyId = propertyList->Item(j).propId;
        }

        void * slotValue = scopeSlotArray[slotIndex + Js::ScopeSlots::FirstSlotIndex];
        if (this->m_scriptEngine.GetScriptContext()->IsUndeclBlockVar(slotValue))
        {
            // Don't report undeclared lexical variables
            properties.count--;
            continue;
        }

        properties.elements[elementIndex].relationshipId = propertyId;

        FillProperty(m_scriptEngine.GetScriptContext(), slotValue, properties.elements[elementIndex]);
        elementIndex++;
    }

    Assert(elementIndex == properties.count);
    return element;
}

ActiveScriptProfilerHeapEnum::ProfilerHeapObject* ActiveScriptProfilerHeapEnum::AllocateElement(UINT allocSize, PROFILER_HEAP_OBJECT_NAME_ID typeNameId)
{
    ProfilerHeapObject* element = (ProfilerHeapObject*)CoTaskMemAlloc(allocSize);
    if (!element)
    {
        Js::Throw::OutOfMemory();
    }
    memset(element, 0, allocSize);
    element->jsInfo.typeNameId = typeNameId;
    return element;
}

Js::RecyclableObject* ActiveScriptProfilerHeapEnum::GetPrototype(Js::RecyclableObject* obj)
{
    Js::TypeId typeId = Js::JavascriptOperators::GetTypeId(obj);
    if (typeId == Js::TypeIds_Null || typeId == Js::TypeIds_HostDispatch || typeId == Js::TypeIds_HostObject)
    {
        return NULL;
    }
    return obj->GetPrototype();
}

ActiveScriptProfilerHeapEnum::ProfilerHeapObject* ActiveScriptProfilerHeapEnum::CreateObjectElement(Js::RecyclableObject* obj, size_t size)
{
    PROFILER_HEAP_OBJECT_NAME_ID typeNameId = GetTypeNameId(obj);
    HostProfilerHeapObject* extObj = NULL;
    if (typeNameId == GetTypeNameId(HeapObjectType_DOM))
    {
        extObj = CreateExternalObjectElement(typeNameId, obj);
    }
    else if (typeNameId == GetTypeNameId(HeapObjectType_WinRT))
    {
        // Constructor function
        if (obj->GetTypeId() == Js::TypeIds_Function)
        {
            Assert(Projection::IsWinRTConstructorFunction(obj));
            extObj = Projection::CreateWinrtConstructorObjectElement(this, obj);
        }
        else
        {
            extObj = CreateExternalObjectElement(typeNameId, obj);
            if (extObj)
            {
                if (Projection::NamespaceProjection::Is(obj))
                {
                    auto namespaceProjection = (Projection::NamespaceProjection *)(Js::CustomExternalObject::FromVar(obj)->GetTypeOperations());
                    extObj->typeNameId = GetPropertyId(namespaceProjection->GetFullName());
                    extObj->flags = PROFILER_HEAP_OBJECT_FLAGS_WINRT_NAMESPACE;
                }
                else
                {
                    extObj->typeNameId = Js::CustomExternalObject::FromVar(obj)->GetTypeNameId();
                }
            }
        }
    }
    ProfilerHeapObject* profObj = CreateJavascriptElement(obj, typeNameId, extObj);
    profObj->jsInfo.size = GetObjectSize(obj, size);
    if (extObj)
    {
        if (extObj->size != 0)
        {
            profObj->jsInfo.size = extObj->size;
        }
        profObj->jsInfo.flags = profObj->jsInfo.flags | extObj->flags;
        profObj->jsInfo.optionalInfoCount += extObj->optionalInfoCount;
        if (typeNameId == GetTypeNameId(HeapObjectType_DOM))
        {
            profObj->jsInfo.typeNameId = Js::CustomExternalObject::FromVar((Js::Var)obj)->GetTypeNameId();
        }
        else if (extObj->typeNameId != 0 && extObj->typeNameId != PROFILER_HEAP_OBJECT_NAME_ID_UNAVAILABLE)
        {
            profObj->jsInfo.typeNameId = extObj->typeNameId;
        }
    }
    return profObj;
}


ActiveScriptProfilerHeapEnum::ProfilerHeapObject* ActiveScriptProfilerHeapEnum::CreateJavascriptElement(Js::RecyclableObject *obj, PROFILER_HEAP_OBJECT_NAME_ID typeNameId, HostProfilerHeapObject* hostInfo)
{
    UINT namePropertyCount = 0;
    UINT namePropertiesAllocSize = 0;

    UINT indexPropertyCount = 0;
    UINT indexPropertiesAllocSize = 0;

    UINT collectionCount = 0;
    UINT collectionAllocSize = 0;

    UINT relationshipCount = 0;
    UINT relationshipAllocSize = 0;

    if (!obj->HasDeferredTypeHandler() && !obj->GetProxiedObjectForHeapEnum())
    {
        // If type handler is deferred, don't want to instantiate it by querying on it. Enumerator should not disturb system state.
        namePropertyCount = GetNamePropertyCount(obj);
        namePropertiesAllocSize = namePropertyCount == 0 ? 0 :
            offsetof(ProfilerHeapObjectOptionalInfo, namePropertyList.elements) + sizeof(PROFILER_HEAP_OBJECT_RELATIONSHIP)*(namePropertyCount);

        indexPropertyCount = GetIndexPropertyCount(obj);
        indexPropertiesAllocSize = indexPropertyCount == 0 ? 0 :
            offsetof(ProfilerHeapObjectOptionalInfo, indexPropertyList.elements) + sizeof(PROFILER_HEAP_OBJECT_RELATIONSHIP)*(indexPropertyCount);
    }

    if (Js::JavascriptMap::Is(obj))
    {
        collectionCount = GetMapCollectionCount(Js::JavascriptMap::FromVar(obj));
        collectionAllocSize = collectionCount == 0 ? 0 :
            offsetof(ProfilerHeapObjectOptionalInfo, mapCollectionList.elements) + sizeof(PROFILER_HEAP_OBJECT_RELATIONSHIP)*(collectionCount);
    }
    else if (Js::JavascriptSet::Is(obj))
    {
        collectionCount = GetSetCollectionCount(Js::JavascriptSet::FromVar(obj));
        collectionAllocSize = collectionCount == 0 ? 0 :
            offsetof(ProfilerHeapObjectOptionalInfo, setCollectionList.elements) + sizeof(PROFILER_HEAP_OBJECT_RELATIONSHIP)*(collectionCount);
    }
    else if (Js::JavascriptWeakMap::Is(obj))
    {
        collectionCount = GetWeakMapCollectionCount(Js::JavascriptWeakMap::FromVar(obj));
        collectionAllocSize = collectionCount == 0 ? 0 :
            offsetof(ProfilerHeapObjectOptionalInfo, weakMapCollectionList.elements) + sizeof(PROFILER_HEAP_OBJECT_RELATIONSHIP)*(collectionCount);
    }
    else if (Js::JavascriptWeakSet::Is(obj))
    {
        collectionCount = GetWeakSetCollectionCount(Js::JavascriptWeakSet::FromVar(obj));
        collectionAllocSize = collectionCount == 0 ? 0 :
            offsetof(ProfilerHeapObjectOptionalInfo, setCollectionList.elements) + sizeof(PROFILER_HEAP_OBJECT_RELATIONSHIP)*(collectionCount);
    }
    else if (CONFIG_FLAG(EnableFunctionSourceReportForHeapEnum) && Js::JavascriptFunction::Is(obj))
    {
        Js::FunctionInfo* funcInfo = Js::JavascriptFunction::FromVar(obj)->GetFunctionInfo();
        Assert(funcInfo);
        if (funcInfo->HasParseableInfo())
        {
            // Note: for dynamic scripts (Function Code, Eval Code) do no provide source info (there's no url, etc),
            // per agreement with F12 Memory Profiler team.
            Js::ParseableFunctionInfo* parseableInfo = funcInfo->GetParseableFunctionInfo();
            Assert(parseableInfo);
            if (!parseableInfo->IsDynamicScript())
            {
                relationshipCount = _countof(c_functionRelationshipNames);
            }
        }
        else
        {
            if (funcInfo->IsDeferredDeserializeFunction())
            {
                relationshipCount = _countof(c_functionRelationshipNames);
            }
        }
    }
    else if (Js::DataView::Is(obj))
    {
        relationshipCount = _countof(c_dataViewRelationshipNames);
    }

    relationshipAllocSize = relationshipCount == 0 ? 0 :
        offsetof(ProfilerHeapObjectOptionalInfo, relationshipList.elements) + sizeof(PROFILER_HEAP_OBJECT_RELATIONSHIP)*(relationshipCount);

    USHORT internalPropertyCount = GetInternalPropertyCount(obj);
    UINT internalPropertyAllocSize = (! internalPropertyCount) ? 0 :
        (offsetof(ProfilerHeapObjectOptionalInfo, internalProperty) + sizeof(((ProfilerHeapObjectOptionalInfo*)0)->internalProperty)) * internalPropertyCount;

    Js::RecyclableObject* prototype =  GetPrototype(obj);
    UINT prototypeAllocSize = (! prototype) ? 0 :
        offsetof(ProfilerHeapObjectOptionalInfo, prototype) + sizeof(((ProfilerHeapObjectOptionalInfo*)0)->prototype);

    LPCWSTR functionName = GetFunctionName(obj);
    UINT functionNameAllocSize =  (! functionName) ? 0 :
        offsetof(ProfilerHeapObjectOptionalInfo, functionName) + sizeof(((ProfilerHeapObjectOptionalInfo*)0)->functionName);

    uint16 scopeCount = GetScopeCount(obj);
    UINT scopeAllocSize = (! scopeCount) ? 0 :
        offsetof(ProfilerHeapObjectOptionalInfo, scopeList.scopes) + sizeof(PROFILER_HEAP_OBJECT_ID)*(scopeCount);

    USHORT optionalInfoCount =
        internalPropertyCount +
        (prototype != NULL) +
        (functionName != NULL) +
        (scopeCount > 0) +
        (namePropertyCount > 0) +
        (indexPropertyCount > 0) +
        (collectionCount > 0) +
        (relationshipCount > 0);

    UINT headerAllocSize =  ProfilerHeapObject::AllocHeaderSize();
    UINT allocSize =
        headerAllocSize +
        internalPropertyAllocSize +
        prototypeAllocSize +
        functionNameAllocSize +
        scopeAllocSize +
        namePropertiesAllocSize +
        indexPropertiesAllocSize +
        collectionAllocSize +
        relationshipAllocSize;

    ProfilerHeapObject* element = AllocateElement(allocSize, typeNameId);

    element->jsInfo.optionalInfoCount = optionalInfoCount;
    ProfilerHeapObjectOptionalInfo* optionalInfoNext = (ProfilerHeapObjectOptionalInfo*)&element->jsInfo.optionalInfo;

    if (internalPropertyAllocSize > 0)
    {
        FillInternalProperty(obj, optionalInfoNext);
        optionalInfoNext = (ProfilerHeapObjectOptionalInfo*)((char *)optionalInfoNext + internalPropertyAllocSize);
    }
    if (prototypeAllocSize > 0)
    {
        optionalInfoNext->infoType = PROFILER_HEAP_OBJECT_OPTIONAL_INFO_PROTOTYPE;
        optionalInfoNext->prototype = (DWORD_PTR)prototype;
        optionalInfoNext = (ProfilerHeapObjectOptionalInfo*)((char *)optionalInfoNext + prototypeAllocSize);
    }
    if (functionNameAllocSize > 0)
    {
        optionalInfoNext->infoType = PROFILER_HEAP_OBJECT_OPTIONAL_INFO_FUNCTION_NAME;
        optionalInfoNext->functionName = functionName;
        optionalInfoNext = (ProfilerHeapObjectOptionalInfo*)((char *)optionalInfoNext + functionNameAllocSize);
    }
    if (scopeAllocSize > 0)
    {
        FillScopes(obj, optionalInfoNext);
        optionalInfoNext = (ProfilerHeapObjectOptionalInfo*)((char *)optionalInfoNext + scopeAllocSize);
    }
    if (namePropertiesAllocSize > 0)
    {
        FillNameProperties(obj, optionalInfoNext);
        Assert(optionalInfoNext->namePropertyList.count == namePropertyCount);
        optionalInfoNext = (ProfilerHeapObjectOptionalInfo*)((char *)optionalInfoNext + namePropertiesAllocSize);
    }
    if (indexPropertiesAllocSize > 0)
    {
        FillIndexProperties(obj, optionalInfoNext);
        Assert(optionalInfoNext->indexPropertyList.count == indexPropertyCount);
        optionalInfoNext = (ProfilerHeapObjectOptionalInfo*)((char *)optionalInfoNext + indexPropertiesAllocSize);
    }

    if (collectionAllocSize > 0)
    {
        if (Js::JavascriptMap::Is(obj))
        {
            FillMapCollectionList(Js::JavascriptMap::FromVar(obj), optionalInfoNext);
            Assert(optionalInfoNext->mapCollectionList.count == collectionCount);
        }
        else if (Js::JavascriptSet::Is(obj))
        {
            FillSetCollectionList(Js::JavascriptSet::FromVar(obj), optionalInfoNext);
            Assert(optionalInfoNext->setCollectionList.count == collectionCount);
        }
        else if (Js::JavascriptWeakMap::Is(obj))
        {
            FillWeakMapCollectionList(Js::JavascriptWeakMap::FromVar(obj), optionalInfoNext);
            Assert(optionalInfoNext->weakMapCollectionList.count == collectionCount);
        }
        else if (Js::JavascriptWeakSet::Is(obj))
        {
            FillWeakSetCollectionList(Js::JavascriptWeakSet::FromVar(obj), optionalInfoNext);
            Assert(optionalInfoNext->setCollectionList.count == collectionCount);
        }

        optionalInfoNext = (ProfilerHeapObjectOptionalInfo*) ((char *) optionalInfoNext + collectionAllocSize);
    }
    else if (relationshipAllocSize > 0)
    {
        FillRelationships(obj, optionalInfoNext);
        Assert(optionalInfoNext->relationshipList.count == relationshipCount);
        optionalInfoNext = (ProfilerHeapObjectOptionalInfo*)((char *)optionalInfoNext + relationshipAllocSize);
    }

    Assert((byte *)optionalInfoNext == ((byte *)element + allocSize));

    element->hostInfo = hostInfo;
    return element;
}

HostProfilerHeapObject* ActiveScriptProfilerHeapEnum::CreateExternalObjectElement(PROFILER_HEAP_OBJECT_NAME_ID typeNameId, Js::RecyclableObject* obj)
{
    Assert(typeNameId == GetTypeNameId(HeapObjectType_DOM) || typeNameId == GetTypeNameId(HeapObjectType_WinRT));
    Assert(!Projection::IsWinRTConstructorFunction(obj));
    HostProfilerHeapObject* externalHeapObjInfo;
    HeapObjectInfoReturnResult returnResult = HeapObjectInfoReturnResult_NoResult;
    HRESULT hr = GetHeapObjectInfo(obj, &externalHeapObjInfo, returnResult);
    AssertMsg(hr != S_FALSE,  "Only S_OK or failure error codes should be returned.");
    if (SUCCEEDED(hr) && returnResult == HeapObjectInfoReturnResult_Success)
    {
        return externalHeapObjInfo;
    }
    else if (FAILED(hr) && hr != E_NOTIMPL)
    {
        AssertMsg(hr == E_OUTOFMEMORY || hr == HRESULT_FROM_WIN32(ERROR_STACK_OVERFLOW), "Unexpected HRESULT encountered.");
        return nullptr;
    }

    AssertMsg(returnResult == HeapObjectInfoReturnResult_NoResult, "GetHeapObjectInfo() succeeded and returned results so we should have returned the external object.");
    HostProfilerHeapObject* element = (HostProfilerHeapObject*)CoTaskMemAlloc(sizeof(HostProfilerHeapObject));
    if (!element)
    {
        Js::Throw::OutOfMemory();
    }
    memset(element, 0, sizeof(HostProfilerHeapObject));
    element->typeNameId = typeNameId;
    return element;
}


HRESULT ActiveScriptProfilerHeapEnum::GetHeapObjectInfo(Var instance, HostProfilerHeapObject** heapObjOut, HeapObjectInfoReturnResult& returnResult)
{
    Js::CustomExternalObject* obj = Js::CustomExternalObject::FromVar(instance);
    Js::ScriptContext* scriptContext = obj->GetScriptContext();
    HRESULT hr = E_NOTIMPL;

    BEGIN_NO_EXCEPTION
#ifdef ENABLE_TEST_HOOKS
    if (!IsWinRTType(obj) && pfGetHeapObjectInfo)
    {
        hr = pfGetHeapObjectInfo(instance, heapObjOut, returnResult);
    }
    else
#endif
    if (m_heapEnumHost)
    {
        hr = obj->GetTypeOperations()->GetHeapObjectInfo(scriptContext->GetActiveScriptDirect(), obj, ProfilerHeapObjectInfoFull, heapObjOut, &returnResult);
    }
    END_NO_EXCEPTION
    return hr;
}

UINT ActiveScriptProfilerHeapEnum::GetObjectSize(void* obj, size_t size)
{
    try
    {
        if (IsJavascriptString(obj))
        {
            return UInt32Math::Add(size, Recycler::GetAlignedSize(Js::JavascriptString::FromVar(obj)->GetAllocatedByteCount()));
        }
        if (Js::ArrayBuffer::Is(obj))
        {
            return UInt32Math::Add(size, Js::ArrayBuffer::FromVar(obj)->GetByteLength(), Js::Throw::OutOfMemory);
        }
        if (Js::JavascriptArray::Is(obj))
        {
            // TODO: Calculate more accurate size information for arrays based on number of elements and overhead of each element.
            //       Small arrays (<= 16 elements) already have this overhead figured into their heap-allocated size.
            //       For now, we always ignore overhead internal to the array as per Win8 #973732.
            return HeapInfo::GetAlignedSize(sizeof(Js::JavascriptArray));
        }
        return size;
    }
    catch (Js::OutOfMemoryException)
    {
        return UINT_MAX;
    }
}

UINT ActiveScriptProfilerHeapEnum::GetNamePropertySlotCount(Js::RecyclableObject* obj, Js::PropertyId relationshipId)
{
    UINT count = 0;
    Var getter=NULL, setter=NULL;
    if (relationshipId != Js::Constants::NoProperty &&
        relationshipId != Js::PropertyIds::_this &&
        ! Js::IsInternalPropertyId(relationshipId) &&
        Js::JavascriptOperators::HasOwnPropertyNoHostObjectForHeapEnum(obj, relationshipId, obj->GetScriptContext(), getter, setter))
    {
        // Will take up two slots if have a getter and a setter, otherwise one if either data property or have only one of getter/setter
        count = (getter && setter) ? 2 : 1;
    }
#if DBG_EXTRAFIELD
    if (count == 0 && Js::Configuration::Global.flags.ValidateHeapEnum)
    {
        int reason = 0;
        if (relationshipId == Js::Constants::NoProperty) reason = 1;
        else if (Js::IsInternalPropertyId(relationshipId)) reason = 2;
        else if (! Js::JavascriptOperators::HasOwnPropertyNoHostObjectForHeapEnum(obj, relationshipId, obj->GetScriptContext(), getter, setter)) reason = 3;
        Output::Print(_u("*** Skipping name property %p for reason %d***\n"), obj, reason);
    }
#endif
    return count;
}

Js::PropertyId
ActiveScriptProfilerHeapEnum::GetPropertyIdByIndex(Js::RecyclableObject * obj, Js::PropertyIndex i)
{
#if DBG
    // GetPropertyId may create property record if it is a string based type handler
    // Need to allow allocation here
    Recycler::AutoAllowAllocationDuringHeapEnum autoAllowAllocationDuringHeapEnum(&m_recycler);
#endif
    return obj->GetPropertyId(i);
}

UINT ActiveScriptProfilerHeapEnum::GetNamePropertyCount(Js::RecyclableObject* obj)
{
    UINT namePropertyCount = 0;
    auto scriptContext = obj->GetScriptContext();
    for (int i=0; i < obj->GetPropertyCount(); i++)
    {
        Js::PropertyId relationshipId = GetPropertyIdByIndex(obj, (Js::PropertyIndex)i);
        namePropertyCount += GetNamePropertySlotCount(obj, relationshipId);
    }

    if (!LegacyBehavior() && Js::RootObjectBase::Is(obj))
    {
        Js::RootObjectBase::FromVar(obj)->MapLetConstGlobals([scriptContext,&namePropertyCount](const Js::PropertyRecord*, Var value, bool) {
            if (!scriptContext->IsUndeclBlockVar(value))
            {
                namePropertyCount += 1;
            }
        });
    }

    return namePropertyCount;
}

Js::ArrayObject* ActiveScriptProfilerHeapEnum::GetIndexPropertyArray(Js::RecyclableObject* obj)
{
    if (Js::DynamicObject::IsAnyArray(obj))
    {
        return Js::JavascriptArray::FromAnyArray(obj);
    }
    if (Js::DynamicType::Is(obj->GetTypeId()))
    {
        Js::DynamicObject* dynamicObj = Js::DynamicObject::FromVar(obj);
        return dynamicObj->GetObjectArray();
    }
    return NULL;
}

UINT ActiveScriptProfilerHeapEnum::GetIndexPropertyCount(Js::RecyclableObject* obj)
{
    Js::ArrayObject* arr = GetIndexPropertyArray(obj);
    UINT elementCount = 0;
    if (arr)
    {
        switch(Js::JavascriptOperators::GetTypeId(arr))
        {
            case Js::TypeIds_Array:
            case Js::TypeIds_ES5Array:
                IterateArray(Js::JavascriptArray::FromAnyArray(arr), [&elementCount](Js::Var var, uint32 index) {
                    ++elementCount;
                });
                break;

            case Js::TypeIds_NativeIntArray:
                IterateNativeArray(Js::JavascriptNativeIntArray::FromVar(arr), [&elementCount](int32 element, uint32 index) {
                    ++elementCount;
                });
                break;

            case Js::TypeIds_CopyOnAccessNativeIntArray:
                IterateNativeArray(Js::JavascriptCopyOnAccessNativeIntArray::FromVar(arr), [&elementCount](int32 element, uint32 index) {
                    ++elementCount;
                });
                break;

            case Js::TypeIds_NativeFloatArray:
                IterateNativeArray(Js::JavascriptNativeFloatArray::FromVar(arr), [&elementCount](double element, uint32 index) {
                    ++elementCount;
                });
                break;

            //TODO: TypedArray

            default:
                Assert(false);
                __assume(false);
        }
    }
    return elementCount;
}

LPCWSTR ActiveScriptProfilerHeapEnum::GetFunctionName(Js::RecyclableObject* obj)
{
    if (Js::JavascriptFunction::Is(obj))
    {
        Js::JavascriptFunction* javascriptFunction = Js::JavascriptFunction::FromVar(obj);
        if (javascriptFunction->GetFunctionInfo()->IsDeferredDeserializeFunction())
        {
            Js::DeferDeserializeFunctionInfo* deferDeserializeFunctionInfo = javascriptFunction->GetDeferDeserializeFunctionInfo();
            return deferDeserializeFunctionInfo ? deferDeserializeFunctionInfo->GetDisplayName() : NULL;
        }
        else
        {
            Js::ParseableFunctionInfo* parseableFunctionInfo = javascriptFunction->GetParseableFunctionInfo();
            return parseableFunctionInfo ? parseableFunctionInfo->GetExternalDisplayName() : NULL;
        }
    }

    return NULL;
}

uint16 ActiveScriptProfilerHeapEnum::GetScopeCount(Js::RecyclableObject* obj)
{
    Js::HeapArgumentsObject* args = Js::HeapArgumentsObject::As(obj);
    if (args)
    {
        return args->GetFrameObject() ? 1 : 0;
    }

    if (! Js::ScriptFunction::Is(obj))
    {
        return 0;
    }

    if (Js::AsmJsScriptFunction::Is(obj))
    {
        // AsmJs function doesn't have ScopeSlots in framedisplay scopes instead it have custom AsmJsModuleMemory pointer
        return 0;
    }

    Js::ScriptFunction* fcn = Js::ScriptFunction::FromVar(obj);
    Js::FrameDisplay* environment = fcn->GetEnvironment();
    return (! environment) ? 0 : environment->GetLength();
}

void ActiveScriptProfilerHeapEnum::FillNameProperties(Js::RecyclableObject* obj, ProfilerHeapObjectOptionalInfo* optionalInfo)
{
    optionalInfo->infoType = PROFILER_HEAP_OBJECT_OPTIONAL_INFO_NAME_PROPERTIES;
    PROFILER_HEAP_OBJECT_RELATIONSHIP_LIST& properties = optionalInfo->namePropertyList;
    UINT namePropertyCount = 0;
    auto scriptContext = obj->GetScriptContext();
    for (int i=0; i < obj->GetPropertyCount(); i++)
    {
        Js::PropertyId relationshipId = GetPropertyIdByIndex(obj, (Js::PropertyIndex)i);
        if (! GetNamePropertySlotCount(obj, relationshipId))
        {
            continue;
        }
        Var getter = NULL, setter = NULL;
        Assert(Js::JavascriptOperators::HasOwnPropertyNoHostObjectForHeapEnum(obj, relationshipId, scriptContext, getter, setter) || getter || setter);
        Js::Var propertyValue = Js::JavascriptOperators::GetOwnPropertyNoHostObjectForHeapEnum(obj, relationshipId, scriptContext, getter, setter);
        if (! (getter || setter))
        {
            properties.elements[namePropertyCount].relationshipId = relationshipId;
            FillProperty(scriptContext, propertyValue, properties.elements[namePropertyCount++]);
        }
        else
        {
            if (getter)
            {
                PROFILER_HEAP_OBJECT_RELATIONSHIP& relationship = properties.elements[namePropertyCount];
                relationship.relationshipId = relationshipId;
                FillProperty(scriptContext, getter, properties.elements[namePropertyCount]);
                ++namePropertyCount;

                SetRelationshipFlags(relationship, PROFILER_HEAP_OBJECT_RELATIONSHIP_FLAGS_IS_GET_ACCESSOR);
            }
            if (setter)
            {
                PROFILER_HEAP_OBJECT_RELATIONSHIP& relationship = properties.elements[namePropertyCount];
                relationship.relationshipId = relationshipId;
                FillProperty(scriptContext, setter, properties.elements[namePropertyCount]);
                ++namePropertyCount;

                SetRelationshipFlags(relationship, PROFILER_HEAP_OBJECT_RELATIONSHIP_FLAGS_IS_SET_ACCESSOR);
            }
        }
    }

    if (!LegacyBehavior() && Js::RootObjectBase::Is(obj))
    {
        Js::RootObjectBase::FromVar(obj)->MapLetConstGlobals([&](const Js::PropertyRecord* propertyRecord, Var value, bool isConst)
        {
            if (!scriptContext->IsUndeclBlockVar(value))
            {
                PROFILER_HEAP_OBJECT_RELATIONSHIP& relationship = properties.elements[namePropertyCount];
                relationship.relationshipId = propertyRecord->GetPropertyId();
                FillProperty(scriptContext, value, relationship);
                ++namePropertyCount;

                SetRelationshipFlags(relationship, isConst ? PROFILER_HEAP_OBJECT_RELATIONSHIP_FLAGS_CONST_VARIABLE : PROFILER_HEAP_OBJECT_RELATIONSHIP_FLAGS_LET_VARIABLE);
            }
        });
    }
    properties.count = namePropertyCount;
}

void ActiveScriptProfilerHeapEnum::FillIndexProperties(Js::RecyclableObject* obj, ProfilerHeapObjectOptionalInfo* optionalInfo)
{
    Assert(Js::DynamicObject::IsAnyArray(obj) || Js::DynamicType::Is(obj->GetTypeId()));
    Js::ArrayObject* arr = GetIndexPropertyArray(obj);
    Assert(arr);
    optionalInfo->infoType = PROFILER_HEAP_OBJECT_OPTIONAL_INFO_INDEX_PROPERTIES;
    PROFILER_HEAP_OBJECT_RELATIONSHIP_LIST& indexPropertyList = optionalInfo->indexPropertyList;

    auto scriptContext = obj->GetScriptContext();
    UINT elementCount = 0;
    switch(Js::JavascriptOperators::GetTypeId(arr))
    {
        case Js::TypeIds_Array:
        case Js::TypeIds_ES5Array:
            IterateArray(Js::JavascriptArray::FromAnyArray(arr), [this, &scriptContext, &elementCount, &indexPropertyList](Js::Var var, uint32 index) {
                indexPropertyList.elements[elementCount].relationshipId = (PROFILER_HEAP_OBJECT_NAME_ID)index;
                FillProperty(scriptContext, var, indexPropertyList.elements[elementCount++]);
            });
            break;

        case Js::TypeIds_NativeIntArray:
            IterateNativeArray(Js::JavascriptNativeIntArray::FromVar(arr), [this, &elementCount, &indexPropertyList](int32 elementValue, uint32 index) {
                indexPropertyList.elements[elementCount].relationshipId = (PROFILER_HEAP_OBJECT_NAME_ID)index;
                FillNumberProperty(elementValue, indexPropertyList.elements[elementCount++]);
            });
            break;

        case Js::TypeIds_CopyOnAccessNativeIntArray:
            IterateNativeArray(Js::JavascriptCopyOnAccessNativeIntArray::FromVar(arr), [this, &elementCount, &indexPropertyList](int32 elementValue, uint32 index) {
                indexPropertyList.elements[elementCount].relationshipId = (PROFILER_HEAP_OBJECT_NAME_ID)index;
                FillNumberProperty(elementValue, indexPropertyList.elements[elementCount++]);
            });
            break;

        case Js::TypeIds_NativeFloatArray:
            IterateNativeArray(Js::JavascriptNativeFloatArray::FromVar(arr), [this, &elementCount, &indexPropertyList](double elementValue, uint32 index) {
                indexPropertyList.elements[elementCount].relationshipId = (PROFILER_HEAP_OBJECT_NAME_ID)index;
                FillNumberProperty(elementValue, indexPropertyList.elements[elementCount++]);
            });
            break;

        //TODO: TypedArray

        default:
            Assert(false);
            __assume(false);
    }
    indexPropertyList.count = elementCount;
}

void ActiveScriptProfilerHeapEnum::FillProperty(Js::ScriptContext *scriptContext, Js::Var property, PROFILER_HEAP_OBJECT_RELATIONSHIP& element)
{
    if (property == NULL)
    {
        AssertMsg(FALSE, "Property value should never be NULL");
        Js::Throw::InternalError();
    }
    if (scriptContext->GetRecycler()->IsValidObject(property) && Js::RecyclableObject::Is(property))
    {
        SetRelationshipInfo(element, PROFILER_PROPERTY_TYPE_HEAP_OBJECT);
        element.objectId = (DWORD_PTR)property;
        return;
    }
    if (Js::JavascriptString::Is(property))
    {
        FillStringProperty(Js::JavascriptString::FromVar(property)->GetString(), element);
        return;
    }
    if(Js::TaggedInt::Is(property))
    {
        FillNumberProperty(Js::TaggedInt::ToInt32(property), element);
        return;
    }
    Assert(Js::JavascriptNumber::Is(property));
    FillNumberProperty(Js::JavascriptNumber::GetValue(property), element);
}

void ActiveScriptProfilerHeapEnum::FillNumberProperty(int32 property, PROFILER_HEAP_OBJECT_RELATIONSHIP& element)
{
    FillNumberProperty(static_cast<double>(property), element);
}

void ActiveScriptProfilerHeapEnum::FillNumberProperty(double property, PROFILER_HEAP_OBJECT_RELATIONSHIP& element)
{
    SetRelationshipInfo(element, PROFILER_PROPERTY_TYPE_NUMBER);
    element.numberValue = property;
}

void ActiveScriptProfilerHeapEnum::FillStringProperty(LPCWSTR property, PROFILER_HEAP_OBJECT_RELATIONSHIP& element)
{
    SetRelationshipInfo(element, PROFILER_PROPERTY_TYPE_STRING);
    element.stringValue = property;
}

void ActiveScriptProfilerHeapEnum::FillObjectProperty(Js::RecyclableObject* property, PROFILER_HEAP_OBJECT_RELATIONSHIP& element)
{
    SetRelationshipInfo(element, PROFILER_PROPERTY_TYPE_HEAP_OBJECT);
    element.objectId = (PROFILER_HEAP_OBJECT_ID)property;
}

void ActiveScriptProfilerHeapEnum::FillNumberRelationship(PROFILER_HEAP_OBJECT_RELATIONSHIP& relationship, PROFILER_HEAP_OBJECT_NAME_ID relationshipId, int32 value)
{
    relationship.relationshipId = relationshipId;
    this->FillNumberProperty(value, relationship);
}

void ActiveScriptProfilerHeapEnum::FillStringRelationship(PROFILER_HEAP_OBJECT_RELATIONSHIP& relationship, PROFILER_HEAP_OBJECT_NAME_ID relationshipId, LPCWSTR value)
{
    relationship.relationshipId = relationshipId;
    this->FillStringProperty(value, relationship);
}

void ActiveScriptProfilerHeapEnum::FillObjectRelationship(PROFILER_HEAP_OBJECT_RELATIONSHIP& relationship, PROFILER_HEAP_OBJECT_NAME_ID relationshipId, Js::RecyclableObject* value)
{
    relationship.relationshipId = relationshipId;
    this->FillObjectProperty(value, relationship);
}

void ActiveScriptProfilerHeapEnum::FillScopes(Js::RecyclableObject* obj, ProfilerHeapObjectOptionalInfo* optionalInfo)
{
    optionalInfo->infoType = PROFILER_HEAP_OBJECT_OPTIONAL_INFO_SCOPE_LIST;
    PROFILER_HEAP_OBJECT_SCOPE_LIST& scopeList = optionalInfo->scopeList;

    Js::HeapArgumentsObject* args = Js::HeapArgumentsObject::As(obj);
    if (args)
    {
        scopeList.count = 1;
        scopeList.scopes[0] = (DWORD_PTR)args->GetFrameObject();
        return;
    }

    Assert(Js::ScriptFunction::Is(obj));
    Js::ScriptFunction* fcn = Js::ScriptFunction::FromVar(obj);
    Js::FrameDisplay* environment = fcn->GetEnvironment();
    Assert(environment);
    uint16 scopeCount = environment->GetLength();
    Assert(scopeCount);
    void **scopes = environment->GetDataAddress();
    Assert(scopes);

    scopeList.count = scopeCount;
    js_memcpy_s(scopeList.scopes, sizeof(scopes[0])*scopeCount, scopes, sizeof(scopes[0])*scopeCount);
}

STDMETHODIMP ActiveScriptProfilerHeapEnum::GetNameIdMap(__out_ecount(*pcelt) LPCWSTR* pPropertyMap[], __out UINT* pcelt)
{
    IfNullReturnError(pPropertyMap, E_POINTER);
    *pPropertyMap = nullptr;
    IfNullReturnError(pcelt, E_POINTER);
    *pcelt = 0;

    HRESULT hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }

    ThreadContext* threadContext = m_scriptEngine.GetScriptContext()->GetThreadContext();
    uint enginePropertyCount = GetEnginePropertyCount();
    uint propertyNameListCount = enginePropertyCount + _countof(c_functionRelationshipNames) + _countof(c_dataViewRelationshipNames);

    LPCWSTR* propertyMap = (LPCWSTR*)CoTaskMemAlloc(sizeof(LPCWSTR) * propertyNameListCount);
    IfNullReturnError(propertyMap, E_OUTOFMEMORY);

    for (uint i = 0; i < enginePropertyCount; i++)
    {
        propertyMap[i] = threadContext->IsActivePropertyId(i) ? threadContext->GetPropertyName(i)->GetBuffer() : NULL;
    }

    // Fill special properties used in relationshipList for JavascriptFunction objects.
    for (uint i = 0; i < _countof(c_functionRelationshipNames); ++i)
    {
        propertyMap[enginePropertyCount + i] = c_functionRelationshipNames[i];
    }

    // Fill special properties used in relationshipList for DataView objects.
    for (uint i = 0; i < _countof(c_dataViewRelationshipNames); ++i)
    {
        propertyMap[enginePropertyCount + _countof(c_functionRelationshipNames) + i] = c_dataViewRelationshipNames[i];
    }

    *pPropertyMap = propertyMap;
    *pcelt = (ULONG)propertyNameListCount;
    return S_OK;
}

// The number of properties in JS engine.
uint ActiveScriptProfilerHeapEnum::GetEnginePropertyCount()
{
    ThreadContext* threadContext = m_scriptEngine.GetScriptContext()->GetThreadContext();
    return threadContext->GetHighestPropertyNameIndex();
}

void ActiveScriptProfilerHeapEnum::Visit(void* obj, ULONG flags, UINT numberOfElements)
{
    RecyclerHeapObjectInfo heapObject;
    if (! obj || ! m_recycler.FindHeapObjectWithClearedAllocators(obj, heapObject) || heapObject.IsObjectMarked())
    {
        return;
    }
    flags |= (! heapObject.SetMemoryProfilerHasEnumerated() ? PROFILER_HEAP_OBJECT_FLAGS_NEW_OBJECT : 0);
    if (IsRecyclableObject(flags))
    {
        Js::RecyclableObject * object= Js::RecyclableObject::FromVar(heapObject.GetObjectAddress());
        flags |= object->GetScriptContext()->IsClosed() ? PROFILER_HEAP_OBJECT_FLAGS_SITE_CLOSED : 0;
        if (IsJavascriptString(object))
        {
            flags |= PROFILER_HEAP_OBJECT_FLAGS_SIZE_APPROXIMATE;
        }
#ifdef HEAP_ENUMERATION_VALIDATION
        if ((Js::DynamicType::Is(object->GetTypeId())))
        {
            Js::DynamicObject::FromVar(object)->SetHeapEnumValidationCookie(this->enumerationCount);
        }
#endif
    }
    heapObject.SetObjectMarked();
    ProfilerHeapObject* heapObjElem = CreateElement(heapObject.GetObjectAddress(), heapObject.GetSize(), flags, numberOfElements);

    HeapScanQueueElement e(heapObject, heapObjElem);
    if (!m_scanQueue.Append(e))
    {
        Js::Throw::OutOfMemory();
    }
}

// If the host generates any relationships with external object addresses in them, it will provide a
// corresponding HostProfilerHeapObject* in the heapObjElem->hostInfo->externalObjects array.
// Enumerator will just enqueue those to be reported later.
void ActiveScriptProfilerHeapEnum::EnqueueExternalObjects(HostProfilerHeapObject* hostInfo)
{
    if (hostInfo->externalObjectCount == 0)
    {
        return;
    }
    HostProfilerHeapObject **externalObjects = hostInfo->externalObjects;
    for (UINT i=0; i < hostInfo->externalObjectCount; i++)
    {
        size_t allocSize = sizeof(ProfilerHeapObject);
        ProfilerHeapObject* heapObjElem = (ProfilerHeapObject*)CoTaskMemAlloc(sizeof(ProfilerHeapObject));
        if (! heapObjElem)
        {
            Js::Throw::OutOfMemory();
        }
        memset(heapObjElem, 0, allocSize);
        heapObjElem->jsInfo = *externalObjects[i];
        heapObjElem->hostInfo = externalObjects[i];

        HeapScanQueueElement e(heapObjElem);
        if (!m_scanQueue.Append(e))
        {
            Js::Throw::OutOfMemory();
        }
    }

    // Remove only array buffer
    if (hostInfo->externalObjectCount > 0)
    {
        CoTaskMemFree(hostInfo->externalObjects);
        hostInfo->externalObjects = NULL;
        hostInfo->externalObjectCount = 0;
    }
}

void ActiveScriptProfilerHeapEnum::VisitRoot(JavascriptDispatch* javascriptDispatch)
{
    UINT internalPropertyAllocSize = offsetof(ProfilerHeapObjectOptionalInfo, internalProperty) + sizeof(PROFILER_HEAP_OBJECT_RELATIONSHIP);;
    UINT headerAllocSize =  ProfilerHeapObject::AllocHeaderSize();
    UINT allocSize = headerAllocSize + internalPropertyAllocSize;
    ProfilerHeapObject* heapObjElem = AllocateElement(allocSize, PROFILER_HEAP_OBJECT_NAME_ID_UNAVAILABLE);
    heapObjElem->jsInfo.flags = PROFILER_HEAP_OBJECT_FLAGS_IS_ROOT | PROFILER_HEAP_OBJECT_FLAGS_NEW_STATE_UNAVAILABLE | PROFILER_HEAP_OBJECT_FLAGS_EXTERNAL_DISPATCH | PROFILER_HEAP_OBJECT_FLAGS_SIZE_APPROXIMATE;
    heapObjElem->jsInfo.externalAddress = javascriptDispatch;
    heapObjElem->jsInfo.optionalInfoCount = 1;
    ProfilerHeapObjectOptionalInfo* internalProperty = (ProfilerHeapObjectOptionalInfo*)(&heapObjElem->jsInfo.optionalInfo);
    internalProperty->infoType = PROFILER_HEAP_OBJECT_OPTIONAL_INFO_INTERNAL_PROPERTY;
    internalProperty->internalProperty.relationshipId = PROFILER_HEAP_OBJECT_NAME_ID_UNAVAILABLE;
    SetRelationshipInfo(internalProperty->internalProperty, PROFILER_PROPERTY_TYPE_HEAP_OBJECT);
    internalProperty->internalProperty.objectId = (PROFILER_HEAP_OBJECT_ID)javascriptDispatch->GetObject();

    HeapScanQueueElement e(heapObjElem);
    if (!m_scanQueue.Append(e))
    {
        Js::Throw::OutOfMemory();
    }
}

void ActiveScriptProfilerHeapEnum::VisitRoot(CUnknownImpl *unknownImpl)
{
    UINT optionalPropertyInfoSize = unknownImpl->GetHeapObjectRelationshipInfoSize();
    if (optionalPropertyInfoSize > 0)
    {
        UINT headerAllocSize =  ProfilerHeapObject::AllocHeaderSize();
        UINT allocSize = headerAllocSize + optionalPropertyInfoSize;
        ProfilerHeapObject* heapObjElem = AllocateElement(allocSize, GetPropertyId(unknownImpl->GetFullTypeName()));

        heapObjElem->jsInfo.flags = PROFILER_HEAP_OBJECT_FLAGS_IS_ROOT | PROFILER_HEAP_OBJECT_FLAGS_NEW_STATE_UNAVAILABLE | PROFILER_HEAP_OBJECT_FLAGS_EXTERNAL_UNKNOWN | PROFILER_HEAP_OBJECT_FLAGS_SIZE_UNAVAILABLE | unknownImpl->GetWinrtTypeFlags();
        heapObjElem->jsInfo.externalAddress = unknownImpl->GetUnknown();
        heapObjElem->jsInfo.optionalInfoCount = 1;
        ProfilerHeapObjectOptionalInfo* optionalInfo = (ProfilerHeapObjectOptionalInfo*)(&heapObjElem->jsInfo.optionalInfo);
        unknownImpl->FillHeapObjectRelationshipInfo(optionalInfo);

        HeapScanQueueElement e(heapObjElem);
        if (!m_scanQueue.Append(e))
        {
            Js::Throw::OutOfMemory();
        }
    }
}

UINT ActiveScriptProfilerHeapEnum::GetMapCollectionCount(Js::JavascriptMap* map)
{
    // Multiply by 2 because key/value pairs are flattened into the same list
    return map->Size() * 2;
}

UINT ActiveScriptProfilerHeapEnum::GetSetCollectionCount(Js::JavascriptSet* set)
{
    return set->Size();
}

UINT ActiveScriptProfilerHeapEnum::GetWeakMapCollectionCount(Js::JavascriptWeakMap* weakMap)
{
    // Multiply by 2 because key/value pairs are flattened into the same list
    return weakMap->Size() * 2;
}

UINT ActiveScriptProfilerHeapEnum::GetWeakSetCollectionCount(Js::JavascriptWeakSet* weakSet)
{
    return weakSet->Size();
}

void ActiveScriptProfilerHeapEnum::FillMapCollectionList(Js::JavascriptMap* map, ProfilerHeapObjectOptionalInfo* optionalInfo)
{
    Js::ScriptContext* scriptContext = map->GetScriptContext();
    UINT elementCount = 0;

    optionalInfo->infoType = PROFILER_HEAP_OBJECT_OPTIONAL_INFO_MAP_COLLECTION_LIST;

    auto iterator = map->GetIterator();
    while (iterator.Next())
    {
        Var key = iterator.Current().Key();
        Var value = iterator.Current().Value();

        optionalInfo->mapCollectionList.elements[elementCount].relationshipId = PROFILER_HEAP_OBJECT_NAME_ID_UNAVAILABLE;
        FillProperty(scriptContext, key, optionalInfo->mapCollectionList.elements[elementCount++]);

        optionalInfo->mapCollectionList.elements[elementCount].relationshipId = PROFILER_HEAP_OBJECT_NAME_ID_UNAVAILABLE;
        FillProperty(scriptContext, value, optionalInfo->mapCollectionList.elements[elementCount++]);
    }

    optionalInfo->mapCollectionList.count = elementCount;
}

void ActiveScriptProfilerHeapEnum::FillSetCollectionList(Js::JavascriptSet* set, ProfilerHeapObjectOptionalInfo* optionalInfo)
{
    Js::ScriptContext* scriptContext = set->GetScriptContext();
    UINT elementCount = 0;

    optionalInfo->infoType = PROFILER_HEAP_OBJECT_OPTIONAL_INFO_SET_COLLECTION_LIST;

    auto iterator = set->GetIterator();
    while (iterator.Next())
    {
        Var value = iterator.Current();

        optionalInfo->setCollectionList.elements[elementCount].relationshipId = PROFILER_HEAP_OBJECT_NAME_ID_UNAVAILABLE;
        FillProperty(scriptContext, value, optionalInfo->setCollectionList.elements[elementCount++]);
    }

    optionalInfo->setCollectionList.count = elementCount;
}

void ActiveScriptProfilerHeapEnum::FillWeakMapCollectionList(Js::JavascriptWeakMap* weakMap, ProfilerHeapObjectOptionalInfo* optionalInfo)
{
    Js::ScriptContext* scriptContext = weakMap->GetScriptContext();
    UINT elementCount = 0;

    optionalInfo->infoType = PROFILER_HEAP_OBJECT_OPTIONAL_INFO_WEAKMAP_COLLECTION_LIST;

    weakMap->Map([&](Var key, Var value) {
        optionalInfo->weakMapCollectionList.elements[elementCount].relationshipId = PROFILER_HEAP_OBJECT_NAME_ID_UNAVAILABLE;
        FillProperty(scriptContext, key, optionalInfo->weakMapCollectionList.elements[elementCount++]);

        optionalInfo->weakMapCollectionList.elements[elementCount].relationshipId = PROFILER_HEAP_OBJECT_NAME_ID_UNAVAILABLE;
        FillProperty(scriptContext, value, optionalInfo->weakMapCollectionList.elements[elementCount++]);
    });

    optionalInfo->weakMapCollectionList.count = elementCount;
}

void ActiveScriptProfilerHeapEnum::FillWeakSetCollectionList(Js::JavascriptWeakSet* weakSet, ProfilerHeapObjectOptionalInfo* optionalInfo)
{
    Js::ScriptContext* scriptContext = weakSet->GetScriptContext();
    UINT elementCount = 0;

    optionalInfo->infoType = PROFILER_HEAP_OBJECT_OPTIONAL_INFO_SET_COLLECTION_LIST;

    weakSet->Map([&](Var key) {
        optionalInfo->setCollectionList.elements[elementCount].relationshipId = PROFILER_HEAP_OBJECT_NAME_ID_UNAVAILABLE;
        FillProperty(scriptContext, key, optionalInfo->setCollectionList.elements[elementCount++]);
    });

    optionalInfo->setCollectionList.count = elementCount;
}

void ActiveScriptProfilerHeapEnum::FillRelationships(Js::RecyclableObject* obj, ProfilerHeapObjectOptionalInfo* optionalInfo)
{
    AssertMsg(
        (Js::JavascriptFunction::Is(obj) && Js::JavascriptFunction::FromVar(obj)) ||
        (Js::DataView::Is(obj) && Js::DataView::FromVar(obj)),
        "Currently we are only doing this for functions and DataViews.");

    optionalInfo->infoType = PROFILER_HEAP_OBJECT_OPTIONAL_INFO_RELATIONSHIPS;
    PROFILER_HEAP_OBJECT_RELATIONSHIP_LIST& relationships = optionalInfo->relationshipList;

    UINT relationshipCount = 0;

    if (Js::JavascriptFunction::Is(obj))
    {
        Js::JavascriptFunction* function = Js::JavascriptFunction::FromVar(obj);
        Assert(function);

        Js::FunctionInfo* functionInfo = Js::JavascriptFunction::FromVar(obj)->GetFunctionInfo();
        Assert(functionInfo);

        AssertMsg(functionInfo->HasParseableInfo() || functionInfo->IsDeferredDeserializeFunction(),
            "We are only doing this for functions with ParseableFunctionInfo or DeferDeserialize functions");

        uint propId = GetEnginePropertyCount();

        int lineNumber, columnNumber;
        LPCWSTR sourceName;

        if (functionInfo->HasParseableInfo())
        {
            Js::ParseableFunctionInfo* parseableInfo = function->GetParseableFunctionInfo();
            AssertMsg(parseableInfo, "HasParseableInfo() doesn't match with GetParseableFunctionInfo() != null.");
            sourceName = parseableInfo->GetSourceName();
            lineNumber = static_cast<int32>(parseableInfo->GetLineNumber());
            columnNumber = static_cast<int32>(parseableInfo->GetColumnNumber());
        }
        else
        {
            Assert(functionInfo->IsDeferredDeserializeFunction());
            Js::DeferDeserializeFunctionInfo* deferDeserializeFunctionInfo = function->GetDeferDeserializeFunctionInfo();
            AssertMsg(deferDeserializeFunctionInfo, "IsDeferredDeserializeFunction() doesn't match with GetDeferDeserializeFunctionInfo() != null.");
            sourceName = deferDeserializeFunctionInfo->GetSourceInfo(lineNumber, columnNumber);
        }

        // Notes:
        // - the order here must match one in c_functionRelationshipNames.
        // - row and col are for beginning of the function.
        FillStringRelationship(relationships.elements[relationshipCount++], propId++, sourceName);
        FillNumberRelationship(relationships.elements[relationshipCount++], propId++, lineNumber);
        FillNumberRelationship(relationships.elements[relationshipCount++], propId++, columnNumber);
        Assert(propId == GetEnginePropertyCount() + _countof(c_functionRelationshipNames));
    }
    else if (Js::DataView::Is(obj))
    {
        Js::DataView* dataView = Js::DataView::FromVar(obj);
        uint propId = GetEnginePropertyCount() + _countof(c_functionRelationshipNames);

        // Order of relationships needs to match c_dataViewRelationshipNames
        FillObjectRelationship(relationships.elements[relationshipCount++], propId++, dataView->GetArrayBuffer());
        FillNumberRelationship(relationships.elements[relationshipCount++], propId++, static_cast<int32>(dataView->GetByteOffset()));
        FillNumberRelationship(relationships.elements[relationshipCount++], propId++, static_cast<int32>(dataView->GetLength()));

        Assert(propId == GetEnginePropertyCount() + _countof(c_functionRelationshipNames) + _countof(c_dataViewRelationshipNames));
    }

    relationships.count = relationshipCount;
}

PROFILER_RELATIONSHIP_INFO ActiveScriptProfilerHeapEnum::GetRelationshipInfo(const PROFILER_HEAP_OBJECT_RELATIONSHIP& relationship)
{
    return static_cast<PROFILER_RELATIONSHIP_INFO>(relationship.relationshipInfo & PROFILER_RELATIONSHIP_INFO_MASK);
}

bool ActiveScriptProfilerHeapEnum::IsRelationshipInfoType(const PROFILER_HEAP_OBJECT_RELATIONSHIP& relationship, PROFILER_RELATIONSHIP_INFO value)
{
    return GetRelationshipInfo(relationship) == value;
}

bool ActiveScriptProfilerHeapEnum::IsRelationshipFlagSet(const PROFILER_HEAP_OBJECT_RELATIONSHIP& relationship, PROFILER_HEAP_OBJECT_RELATIONSHIP_FLAGS flag)
{
    return !!(relationship.relationshipInfo & flag);
}

bool ActiveScriptProfilerHeapEnum::AreAnyRelationshipFlagsSet(const PROFILER_HEAP_OBJECT_RELATIONSHIP& relationship)
{
    return IsRelationshipFlagSet(
        relationship,
        static_cast<PROFILER_HEAP_OBJECT_RELATIONSHIP_FLAGS>(PROFILER_RELATIONSHIP_FLAGS_MASK));
}

void ActiveScriptProfilerHeapEnum::SetRelationshipInfo(PROFILER_HEAP_OBJECT_RELATIONSHIP& relationship, PROFILER_RELATIONSHIP_INFO value)
{
    if (ShouldStoreRelationshipFlags())
    {
        // If we're storing flags, don't overwrite the top 2 bytes in case they're set with flags.
        ULONG setValue = static_cast<ULONG>(relationship.relationshipInfo);
        setValue = (setValue & PROFILER_RELATIONSHIP_FLAGS_MASK) | value;
        relationship.relationshipInfo = static_cast<PROFILER_RELATIONSHIP_INFO>(setValue);
    }
    else
    {
        // If not storing flags, just clear the top 2 bytes to maintain old functionality.
        relationship.relationshipInfo = value;
    }
}

void ActiveScriptProfilerHeapEnum::SetRelationshipFlags(PROFILER_HEAP_OBJECT_RELATIONSHIP& relationship, PROFILER_HEAP_OBJECT_RELATIONSHIP_FLAGS flag)
{
    if (ShouldStoreRelationshipFlags())
    {
        Assert(!(flag & (PROFILER_HEAP_OBJECT_RELATIONSHIP_FLAGS_IS_GET_ACCESSOR | PROFILER_HEAP_OBJECT_RELATIONSHIP_FLAGS_IS_SET_ACCESSOR))
            || GetRelationshipInfo(relationship) == PROFILER_PROPERTY_TYPE_HEAP_OBJECT
            || GetRelationshipInfo(relationship) == PROFILER_PROPERTY_TYPE_EXTERNAL_OBJECT);

        // Set the flag into the top 2 bytes of the relationshipInfo field.
        ULONG setValue = static_cast<ULONG>(relationship.relationshipInfo);
        setValue = (setValue & PROFILER_RELATIONSHIP_INFO_MASK) | flag;
        relationship.relationshipInfo = static_cast<PROFILER_RELATIONSHIP_INFO>(setValue);
    }
}

USHORT ActiveScriptProfilerHeapEnum::GetInternalPropertyCount(Js::RecyclableObject* obj)
{
    USHORT count = 0;

    if (obj->GetProxiedObjectForHeapEnum() ||
        Js::JavascriptStringObject::Is(obj) ||
        IsReportableJavascriptString(obj) ||
        Js::TypedArrayBase::Is(obj) ||
        Js::JavascriptSymbol::Is(obj))
    {
        count += 1;
    }
    else if (
        (Js::JavascriptArrayIterator::Is(obj) && Js::JavascriptArrayIterator::FromVar(obj)->GetIteratorObjectForHeapEnum() != nullptr) ||
        (Js::JavascriptMapIterator::Is(obj) && Js::JavascriptMapIterator::FromVar(obj)->GetMapForHeapEnum() != nullptr) ||
        (Js::JavascriptSetIterator::Is(obj) && Js::JavascriptSetIterator::FromVar(obj)->GetSetForHeapEnum() != nullptr) ||
        (Js::JavascriptStringIterator::Is(obj) && Js::JavascriptStringIterator::FromVar(obj)->GetStringForHeapEnum() != nullptr))
    {
        count += 1;
    }
    else if (Js::GlobalObject::Is(obj))
    {
        Js::GlobalObject *globalObject = Js::GlobalObject::FromVar(obj);
        count += 1;  // regexconstructor; it can hold the last match regexp.
        if(globalObject->GetSecureDirectHostObject() != NULL)
        {
            count += 1;
        }
    }
    else if (IsBoundFunction(obj))
    {
        Js::BoundFunction* boundFunction = (Js::BoundFunction*)Js::JavascriptFunction::FromVar(obj);
        count += 1;
        if (boundFunction->GetBoundThis() != NULL)
        {
            count += 1;
        }
        if (boundFunction->GetArgsCountForHeapEnum() > 0)
        {
            count += 1;
        }
    }
    else if (Js::JavascriptProxy::Is(obj))
    {
        count += 2; // target & handler
    }

    return count;
}

void ActiveScriptProfilerHeapEnum::FillInternalProperty(Js::RecyclableObject* obj, ProfilerHeapObjectOptionalInfo* optionalInfo)
{
    optionalInfo->infoType = PROFILER_HEAP_OBJECT_OPTIONAL_INFO_INTERNAL_PROPERTY;

    Js::RecyclableObject* proxiedObject = obj->GetProxiedObjectForHeapEnum();
    if (proxiedObject)
    {
        SetRelationshipInfo(optionalInfo->internalProperty, PROFILER_PROPERTY_TYPE_HEAP_OBJECT);
        optionalInfo->internalProperty.objectId = (PROFILER_HEAP_OBJECT_ID)proxiedObject;
        return;
    }

    if (Js::JavascriptStringObject::Is(obj))
    {
        // JavascriptStringObjects simply wrap JavascriptString objects so we report inner JavascriptString as an internal property.
        // It's possible we could just return the underlying string buffer (wrapped by JavascriptString) but we would not then be able to
        // tell when two JavascriptStringObjects wrap the same JavascriptString since we don't have the heap address for the JavascriptString - we only get the string value.
        // Also, concat strings which alias other JavascriptStrings have their buffer set to NULL so we would need to do some special-casing.

        Js::JavascriptString* jsString = Js::JavascriptStringObject::FromVar(obj)->Unwrap();
        RecyclerHeapObjectInfo heapObject;

        // If the underlying JavascriptString is found in the heap, stick it into an internal property and we'll Visit it later.
        // Otherwise, the string might be allocated in an Arena so we will just report a string relationship here.
        // (this is the case for SingleCharString objects cached in the JavascriptString class)
        if (m_recycler.FindHeapObjectWithClearedAllocators(jsString, heapObject))
        {
            SetRelationshipInfo(optionalInfo->internalProperty, PROFILER_PROPERTY_TYPE_HEAP_OBJECT);
            optionalInfo->internalProperty.objectId = (PROFILER_HEAP_OBJECT_ID)jsString;
        }
        else
        {
            Assert(IsReportableJavascriptString(jsString));
            SetRelationshipInfo(optionalInfo->internalProperty, PROFILER_PROPERTY_TYPE_STRING);
            optionalInfo->internalProperty.stringValue = jsString->UnsafeGetBuffer();
        }
    }
    else if (Js::TypedArrayBase::Is(obj))
    {
        SetRelationshipInfo(optionalInfo->internalProperty, PROFILER_PROPERTY_TYPE_HEAP_OBJECT);
        optionalInfo->internalProperty.objectId = (PROFILER_HEAP_OBJECT_ID)Js::TypedArrayBase::FromVar(obj)->GetArrayBuffer();
    }
    else if (Js::GlobalObject::Is(obj))
    {
        Js::GlobalObject *globalObject = Js::GlobalObject::FromVar(obj);
        optionalInfo->infoType = PROFILER_HEAP_OBJECT_OPTIONAL_INFO_INTERNAL_PROPERTY;
        SetRelationshipInfo(optionalInfo->internalProperty, PROFILER_PROPERTY_TYPE_HEAP_OBJECT);
        optionalInfo->internalProperty.objectId = (PROFILER_HEAP_OBJECT_ID)globalObject->GetLibrary()->GetRegExpConstructor();

        if(globalObject->GetSecureDirectHostObject() != NULL)
        {
            optionalInfo = GetNextOptionalInfo(optionalInfo);
            optionalInfo->infoType = PROFILER_HEAP_OBJECT_OPTIONAL_INFO_INTERNAL_PROPERTY;
            SetRelationshipInfo(optionalInfo->internalProperty, PROFILER_PROPERTY_TYPE_HEAP_OBJECT);
            optionalInfo->internalProperty.objectId = (PROFILER_HEAP_OBJECT_ID)globalObject->GetDirectHostObject();
        }
    }
    else if (IsBoundFunction(obj))
    {
        Js::BoundFunction* boundFunction = (Js::BoundFunction*)Js::JavascriptFunction::FromVar(obj);
        SetRelationshipInfo(optionalInfo->internalProperty, PROFILER_PROPERTY_TYPE_HEAP_OBJECT);
        optionalInfo->internalProperty.objectId = (PROFILER_HEAP_OBJECT_ID)boundFunction->GetTargetFunction();

        if (boundFunction->GetBoundThis() != NULL)
        {
            optionalInfo = GetNextOptionalInfo(optionalInfo);
            optionalInfo->infoType = PROFILER_HEAP_OBJECT_OPTIONAL_INFO_INTERNAL_PROPERTY;
            SetRelationshipInfo(optionalInfo->internalProperty, PROFILER_PROPERTY_TYPE_HEAP_OBJECT);
            optionalInfo->internalProperty.objectId = (PROFILER_HEAP_OBJECT_ID)boundFunction->GetBoundThis();
        }

        if (boundFunction->GetArgsCountForHeapEnum() > 0)
        {
            optionalInfo = GetNextOptionalInfo(optionalInfo);
            optionalInfo->infoType = PROFILER_HEAP_OBJECT_OPTIONAL_INFO_INTERNAL_PROPERTY;
            SetRelationshipInfo(optionalInfo->internalProperty, PROFILER_PROPERTY_TYPE_HEAP_OBJECT);
            optionalInfo->internalProperty.objectId = (PROFILER_HEAP_OBJECT_ID)boundFunction->GetArgsForHeapEnum();
        }
    }
    else if (IsReportableJavascriptString(obj))
    {
        Js::JavascriptString* jsString = Js::JavascriptString::FromVar(obj);
        if ((this->m_enumFlags & PROFILER_HEAP_ENUM_FLAGS_SUBSTRINGS) && jsString->IsSubstring())
        {
            SetRelationshipInfo(optionalInfo->internalProperty, PROFILER_PROPERTY_TYPE_SUBSTRING);
            PROFILER_PROPERTY_TYPE_SUBSTRING_INFO* subString = (PROFILER_PROPERTY_TYPE_SUBSTRING_INFO*)CoTaskMemAlloc(sizeof(PROFILER_PROPERTY_TYPE_SUBSTRING_INFO));
            if (!subString)
            {
                Js::Throw::OutOfMemory();
            }
            subString->length = jsString->GetLength();
            subString->value = jsString->GetString();
            optionalInfo->internalProperty.subString = subString;
        }
        else
        {
            SetRelationshipInfo(optionalInfo->internalProperty, PROFILER_PROPERTY_TYPE_STRING);
            optionalInfo->internalProperty.stringValue = jsString->GetString();
        }
    }
    else if (Js::JavascriptSymbol::Is(obj))
    {
        SetRelationshipInfo(optionalInfo->internalProperty, PROFILER_PROPERTY_TYPE_STRING);
        optionalInfo->internalProperty.stringValue = Js::JavascriptSymbol::FromVar(obj)->GetValue()->GetBuffer();
    }
    else if (Js::JavascriptProxy::Is(obj))
    {
        Js::JavascriptProxy* proxy = Js::JavascriptProxy::FromVar(obj);
        SetRelationshipInfo(optionalInfo->internalProperty, PROFILER_PROPERTY_TYPE_HEAP_OBJECT);
        optionalInfo->internalProperty.objectId = (PROFILER_HEAP_OBJECT_ID)proxy->GetTarget();

        optionalInfo = GetNextOptionalInfo(optionalInfo);
        optionalInfo->infoType = PROFILER_HEAP_OBJECT_OPTIONAL_INFO_INTERNAL_PROPERTY;
        SetRelationshipInfo(optionalInfo->internalProperty, PROFILER_PROPERTY_TYPE_HEAP_OBJECT);
        optionalInfo->internalProperty.objectId = (PROFILER_HEAP_OBJECT_ID)proxy->GetHandler();
    }
    else if (Js::JavascriptArrayIterator::Is(obj))
    {
        Js::JavascriptArrayIterator* arrayIterator = Js::JavascriptArrayIterator::FromVar(obj);

        AssertMsg(arrayIterator->GetIteratorObjectForHeapEnum() != nullptr, "GetInternalPropertyCount() should have returned 0 if this is null");

        SetRelationshipInfo(optionalInfo->internalProperty, PROFILER_PROPERTY_TYPE_HEAP_OBJECT);
        optionalInfo->internalProperty.objectId = (PROFILER_HEAP_OBJECT_ID)arrayIterator->GetIteratorObjectForHeapEnum();
    }
    else if (Js::JavascriptMapIterator::Is(obj))
    {
        Js::JavascriptMapIterator* mapIterator = Js::JavascriptMapIterator::FromVar(obj);

        AssertMsg(mapIterator->GetMapForHeapEnum() != nullptr, "GetInternalPropertyCount() should have returned 0 if this is null");

        SetRelationshipInfo(optionalInfo->internalProperty, PROFILER_PROPERTY_TYPE_HEAP_OBJECT);
        optionalInfo->internalProperty.objectId = (PROFILER_HEAP_OBJECT_ID)mapIterator->GetMapForHeapEnum();
    }
    else if (Js::JavascriptSetIterator::Is(obj))
    {
        Js::JavascriptSetIterator* setIterator = Js::JavascriptSetIterator::FromVar(obj);

        AssertMsg(setIterator->GetSetForHeapEnum() != nullptr, "GetInternalPropertyCount() should have returned 0 if this is null");

        SetRelationshipInfo(optionalInfo->internalProperty, PROFILER_PROPERTY_TYPE_HEAP_OBJECT);
        optionalInfo->internalProperty.objectId = (PROFILER_HEAP_OBJECT_ID)setIterator->GetSetForHeapEnum();
    }
    else if (Js::JavascriptStringIterator::Is(obj))
    {
        Js::JavascriptStringIterator* stringIterator = Js::JavascriptStringIterator::FromVar(obj);

        AssertMsg(stringIterator->GetStringForHeapEnum() != nullptr, "GetInternalPropertyCount() should have returned 0 if this is null");

        SetRelationshipInfo(optionalInfo->internalProperty, PROFILER_PROPERTY_TYPE_HEAP_OBJECT);
        optionalInfo->internalProperty.objectId = (PROFILER_HEAP_OBJECT_ID)stringIterator->GetStringForHeapEnum();
    }
}

void ActiveScriptProfilerHeapEnum::VisitRelationshipList(PROFILER_HEAP_OBJECT_RELATIONSHIP_LIST& list)
{
    for (UINT j = 0; j < list.count; j++)
    {
        PROFILER_HEAP_OBJECT_RELATIONSHIP relationship = list.elements[j];
        USHORT flag = 0;

        if (IsRelationshipInfoType(relationship, PROFILER_PROPERTY_TYPE_HEAP_OBJECT))
        {
            if(Js::GlobalObject::Is((Js::Var)relationship.objectId))
            {
                flag = PROFILER_HEAP_OBJECT_FLAGS_IS_ROOT;
            }

            Visit((void*)(relationship.objectId), flag);
        }
    }
}

template <typename T>
void ActiveScriptProfilerHeapEnum::VisitDependencies(T* obj, USHORT optionalInfoCount, ULONG flags)
{
    ProfilerHeapObjectOptionalInfo* optionalInfo = (ProfilerHeapObjectOptionalInfo*)&obj->optionalInfo;
    for (USHORT i=0; i < optionalInfoCount; i++)
    {
        switch(optionalInfo->infoType)
        {
        case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_FUNCTION_NAME:
        case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_ELEMENT_ATTRIBUTES_SIZE:
        case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_ELEMENT_TEXT_CHILDREN_SIZE:
            {
                // Nothing to visit here
                break;
            }
        case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_PROTOTYPE:
            {
                Visit((void*)optionalInfo->prototype);
                break;
            }
        case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_INTERNAL_PROPERTY:
            {
                if (IsRelationshipInfoType(optionalInfo->internalProperty, PROFILER_PROPERTY_TYPE_HEAP_OBJECT))
                {
                    ULONG internalPropertyFlags = GetInternalPropertyFlags(&optionalInfo->internalProperty);
                    UINT numberOfElements = IsBoundFunctionArgs(internalPropertyFlags) ? ((Js::BoundFunction*)Js::JavascriptFunction::FromVar((Var)obj->objectId))->GetArgsCountForHeapEnum() : 0;
                    Visit((void*)optionalInfo->internalProperty.objectId, internalPropertyFlags, numberOfElements);
                }
                break;
            }
        case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_INDEX_PROPERTIES:
            {
                VisitRelationshipList(optionalInfo->indexPropertyList);
                break;
            }
        case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_NAME_PROPERTIES:
            {
                VisitRelationshipList(optionalInfo->namePropertyList);
                break;
            }
        case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_RELATIONSHIPS:
            {
                VisitRelationshipList(optionalInfo->relationshipList);
                break;
            }
        case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_SCOPE_LIST:
            {
                PROFILER_HEAP_OBJECT_SCOPE_LIST& scopeList = optionalInfo->scopeList;
                for (UINT j=0; j < scopeList.count; j++)
                {
                    // The scope is either a recylable object or an inline slot array. Using the allocated inline slot memory,
                    // rather than allocating our own memory for it, simplifies handling of multiple references to
                    // the same inline slot from different FrameDisplays. It also maintains the proper reporting order in the
                    // breadth-first scan, although that is less of an concern.
                    INT_PTR v = *(INT_PTR *)scopeList.scopes[j];
                    ULONG childFlags = 0;
                    if (!AutoSystemInfo::IsJscriptModulePointer(reinterpret_cast<void*>(v)))
                    {
                        childFlags = PROFILER_HEAP_OBJECT_INTERNAL_FLAGS_SCOPE_SLOT_ARRAY |
                            (IsSiteClosed(childFlags) ? PROFILER_HEAP_OBJECT_FLAGS_SITE_CLOSED : 0);
                    }
                    Visit((void*)scopeList.scopes[j], childFlags);
                }
                break;
            }
        case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_WINRTEVENTS:
            {
                VisitRelationshipList(optionalInfo->eventList);
                break;
            }
        case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_MAP_COLLECTION_LIST:
            {
                VisitRelationshipList(optionalInfo->mapCollectionList);
                break;
            }
        case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_SET_COLLECTION_LIST:
            {
                VisitRelationshipList(optionalInfo->setCollectionList);
                break;
            }
        case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_WEAKMAP_COLLECTION_LIST:
            {
                VisitRelationshipList(optionalInfo->weakMapCollectionList);
                break;
            }
        default:
            Assert(FALSE);
        }
        optionalInfo = GetNextOptionalInfo(optionalInfo);
    }
}

ULONG ActiveScriptProfilerHeapEnum::GetInternalPropertyFlags(PROFILER_HEAP_OBJECT_RELATIONSHIP *internalProp)
{
    // Internal property can be a recyclable object or an arguments list of bound Function
    if (internalProp->objectId == 0) return 0;
    INT_PTR v = *(INT_PTR *)internalProp->objectId;
    if (AutoSystemInfo::IsJscriptModulePointer(reinterpret_cast<void*>(v))) return 0;
    return PROFILER_HEAP_OBJECT_INTERNAL_FLAGS_BOUND_FUNCTION_ARGUMENT_LIST;
}


void ActiveScriptProfilerHeapEnum::VisitAllDependencies(ProfilerHeapObject* obj)
{
    VisitDependencies(&obj->jsInfo, obj->OptionalInfoCountJSOnly(), obj->jsInfo.flags);
    if (obj->hostInfo)
    {
        VisitDependencies(obj->hostInfo, obj->hostInfo->optionalInfoCount, obj->jsInfo.flags);
        EnqueueExternalObjects(obj->hostInfo);
    }
}


template <typename Fn>
bool ActiveScriptProfilerHeapEnum::EnumerateHeapHelper(ProfilerHeapObject* obj, Fn* callback)
{
    return (*callback)(obj);
}

#ifdef HEAP_ENUMERATION_VALIDATION
void ActiveScriptProfilerHeapEnum::EnsureRecyclableObjectsAreVisitedCallback(const RecyclerHeapObjectInfo& heapObject, void *data)
{
    INT_PTR v = *(INT_PTR *)(heapObject.GetObjectAddress());
    ULONG flags = 0;
    LPCSTR className;
    ActiveScriptProfilerHeapEnum *_this = reinterpret_cast<ActiveScriptProfilerHeapEnum *>(data);

    if (AutoSystemInfo::IsJscriptModulePointer(reinterpret_cast<void*>(v)) && _this->m_vtableMap.TryGetValue(v, (INT_PTR*)&className) && !heapObject.IsObjectMarked())
    {
        if (Js::DynamicType::Is(((Js::RecyclableObject*)heapObject.GetObjectAddress())->GetTypeId()))
        {
            Js::DynamicObject *obj = Js::DynamicObject::FromVar(heapObject.GetObjectAddress());
            if (obj->GetHeapEnumValidationCookie() != _this->enumerationCount)
            {
                if  (obj->GetHeapEnumValidationCookie() == HEAP_ENUMERATION_LIBRARY_OBJECT_COOKIE)
                {
                    Output::Print(_u("*** [%d/%d] %p object of %S in GC heap not enumerated (library object) ***\n"), libObjectCount, userObjectCount + libObjectCount, heapObject.GetObjectAddress(), className);
                    libObjectCount++;
                    currentEnumerator->Visit(obj, PROFILER_HEAP_OBJECT_INTERNAL_FLAGS_UNREPORTED_LIBRARY_OBJECT);
                }
                else
                {
//                    uint threshHoldForAssertingOnNonReportedObjects = 100;
//                    AssertMsg((userObjectCount + libObjectCount) < threshHoldForAssertingOnNonReportedObjects, "Exceeded threshold of Js::DynamicObject in GC heap that weren't enumerated");
                    Output::Print(_u("*** [%d/%d] %p object of %S in GC heap not enumerated (user object) ***\n"), userObjectCount, libObjectCount + userObjectCount, heapObject.GetObjectAddress(), className);
                    userObjectCount++;
                    currentEnumerator->Visit(obj, PROFILER_HEAP_OBJECT_INTERNAL_FLAGS_UNREPORTED_USER_OBJECT);
                }
            }
        }
    }
}
#endif

__inline void ActiveScriptProfilerHeapEnum::AddObjectToSummary(ProfilerHeapObject* obj, PROFILER_HEAP_SUMMARY* pHeapSummary)
{
    pHeapSummary->totalHeapSize += obj->jsInfo.size;
    FreeObjectAndOptionalInfo(obj);
}

template <typename Fn>
void ActiveScriptProfilerHeapEnum::EnumerateHeap(Fn callback, PROFILER_HEAP_SUMMARY* pHeapSummary)
{
    bool continueEnum = true;
    while (!m_scanQueue.Empty() && continueEnum==true)
    {
        HeapScanQueueElement e = m_scanQueue.Head();
        m_scanQueue.RemoveHead();
        e.m_profHeapObject->jsInfo.flags &= ~INTERNAL_PROFILER_HEAP_OBJECT_INTERNAL_FLAGS_MASK;
        VisitAllDependencies(e.m_profHeapObject);
        if (! pHeapSummary)
        {
            continueEnum = EnumerateHeapHelper(e.m_profHeapObject, &callback);
        }
        else
        {
            AddObjectToSummary(e.m_profHeapObject, pHeapSummary);
        }

        if (m_scanQueue.Empty())
        {
            // are done with GC mark bits now. This allows post-enumeration operations that
            // execute js w/o having to release the enum pointer.
            delete m_autoSetupRecyclerForNonCollectingMark;
            m_autoSetupRecyclerForNonCollectingMark = NULL;
#ifdef HEAP_ENUMERATION_VALIDATION
            static_assert( (INTERNAL_PROFILER_HEAP_OBJECT_INTERNAL_FLAGS_MASK & (PROFILER_HEAP_OBJECT_INTERNAL_FLAGS_UNREPORTED_LIBRARY_OBJECT | PROFILER_HEAP_OBJECT_INTERNAL_FLAGS_UNREPORTED_USER_OBJECT)) == 0,
                "PROFILER_HEAP_OBJECT_INTERNAL_FLAGS_UNREPORTED_OBJECT collision");
            if (Js::Configuration::Global.flags.ValidateHeapEnum)
            {
                if (currentEnumerator != NULL)
                {
                    // don't rescan if just have completed a post-enum scan
                    currentEnumerator = NULL;
                }
                else
                {
                    // Dump the object graph so if we have any unreported objects we can figure out where they came from
                    RecyclerObjectGraphDumper::Param param;
                    ZeroMemory(&param, sizeof(param));
                    param.skipStack = true;
                    m_recycler.DumpObjectGraph(&param);

                    {
                        // We needed to reset all the mark bits for the scan here to work, so re-enter heapenum mode.
                        Recycler::AutoSetupRecyclerForNonCollectingMark autoSetupRecyclerForNonCollectingMark(m_recycler, true);
                        autoSetupRecyclerForNonCollectingMark.SetupForHeapEnumeration();

                        libObjectCount = 0;
                        userObjectCount = 0;
                        currentEnumerator = this;
                        m_recycler.PostHeapEnumScan(&EnsureRecyclableObjectsAreVisitedCallback, this);
                    }

                    // Reset the enumeration state to report the unreported objects
                    m_autoSetupRecyclerForNonCollectingMark = new Recycler::AutoSetupRecyclerForNonCollectingMark(m_recycler, true);
                    if (! m_autoSetupRecyclerForNonCollectingMark)
                    {
                        Js::Throw::OutOfMemory();
                    }
                    m_autoSetupRecyclerForNonCollectingMark->SetupForHeapEnumeration();
                }
            }
#endif
        }
    }
}

ProfilerHeapObjectOptionalInfo* ActiveScriptProfilerHeapEnum::GetNextOptionalInfo(ProfilerHeapObjectOptionalInfo* optionalInfo)
{
    switch(optionalInfo->infoType)
    {
    case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_INTERNAL_PROPERTY:
        {
            return (ProfilerHeapObjectOptionalInfo*)((char*)optionalInfo +
                offsetof(ProfilerHeapObjectOptionalInfo, internalProperty) + sizeof(((ProfilerHeapObjectOptionalInfo*)0)->internalProperty));
        }
    case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_PROTOTYPE:
        {
            return (ProfilerHeapObjectOptionalInfo*)((char*)optionalInfo +
                offsetof(ProfilerHeapObjectOptionalInfo, prototype) + sizeof(((ProfilerHeapObjectOptionalInfo*)0)->prototype));
        }
    case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_FUNCTION_NAME:
        {
            return (ProfilerHeapObjectOptionalInfo*)((char*)optionalInfo +
                offsetof(ProfilerHeapObjectOptionalInfo, functionName) + sizeof(((ProfilerHeapObjectOptionalInfo*)0)->functionName));
        }
    case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_SCOPE_LIST:
        {
            return (ProfilerHeapObjectOptionalInfo*)((char*)optionalInfo +
                offsetof(ProfilerHeapObjectOptionalInfo, scopeList.scopes) + sizeof(PROFILER_HEAP_OBJECT_ID)*(optionalInfo->scopeList.count));
        }
    case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_NAME_PROPERTIES:
        {
            return (ProfilerHeapObjectOptionalInfo*)((char*)optionalInfo +
                offsetof(ProfilerHeapObjectOptionalInfo, namePropertyList.elements) + sizeof(PROFILER_HEAP_OBJECT_RELATIONSHIP)*(optionalInfo->namePropertyList.count));
        }
    case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_INDEX_PROPERTIES:
        {
            return (ProfilerHeapObjectOptionalInfo*)((char*)optionalInfo +
                offsetof(ProfilerHeapObjectOptionalInfo, indexPropertyList.elements) + sizeof(PROFILER_HEAP_OBJECT_RELATIONSHIP)*(optionalInfo->indexPropertyList.count));
        }
    case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_RELATIONSHIPS:
        {
            return (ProfilerHeapObjectOptionalInfo*)((char*)optionalInfo +
                offsetof(ProfilerHeapObjectOptionalInfo, relationshipList.elements) + sizeof(PROFILER_HEAP_OBJECT_RELATIONSHIP)*(optionalInfo->relationshipList.count));
        }
    case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_ELEMENT_ATTRIBUTES_SIZE:
        {
            return (ProfilerHeapObjectOptionalInfo*)((char*)optionalInfo +
                offsetof(ProfilerHeapObjectOptionalInfo, elementAttributesSize) + sizeof(((ProfilerHeapObjectOptionalInfo*)0)->elementAttributesSize));
        }
    case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_ELEMENT_TEXT_CHILDREN_SIZE:
        {
            return (ProfilerHeapObjectOptionalInfo*)((char*)optionalInfo +
                offsetof(ProfilerHeapObjectOptionalInfo, elementTextChildrenSize) + sizeof(((ProfilerHeapObjectOptionalInfo*)0)->elementTextChildrenSize));
        }
    case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_WINRTEVENTS:
        {
            return (ProfilerHeapObjectOptionalInfo*)((char*)optionalInfo +
                offsetof(ProfilerHeapObjectOptionalInfo, eventList.elements) + sizeof(PROFILER_HEAP_OBJECT_RELATIONSHIP)*(optionalInfo->eventList.count));
        }
    case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_MAP_COLLECTION_LIST:
        {
            return (ProfilerHeapObjectOptionalInfo*)((char*)optionalInfo +
                offsetof(ProfilerHeapObjectOptionalInfo, mapCollectionList.elements) + sizeof(PROFILER_HEAP_OBJECT_RELATIONSHIP)*(optionalInfo->mapCollectionList.count));
        }
    case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_SET_COLLECTION_LIST:
        {
            return (ProfilerHeapObjectOptionalInfo*)((char*)optionalInfo +
                offsetof(ProfilerHeapObjectOptionalInfo, setCollectionList.elements) + sizeof(PROFILER_HEAP_OBJECT_RELATIONSHIP)*(optionalInfo->setCollectionList.count));
        }
    case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_WEAKMAP_COLLECTION_LIST:
        {
            return (ProfilerHeapObjectOptionalInfo*)((char*)optionalInfo +
                offsetof(ProfilerHeapObjectOptionalInfo, weakMapCollectionList.elements) + sizeof(PROFILER_HEAP_OBJECT_RELATIONSHIP)*(optionalInfo->weakMapCollectionList.count));
        }
    default:
        Assert(FALSE);
    }
    return NULL;
}

UINT ActiveScriptProfilerHeapEnum::GetHeapObjectInternalPropertyInfoSize()
{
    return offsetof(ProfilerHeapObjectOptionalInfo, internalProperty) + sizeof(PROFILER_HEAP_OBJECT_RELATIONSHIP);
}

void ActiveScriptProfilerHeapEnum::FillHeapObjectInternalUnnamedJSVarProperty(ProfilerHeapObjectOptionalInfo *optionalInfo, Var jsVar)
{
    optionalInfo->infoType = PROFILER_HEAP_OBJECT_OPTIONAL_INFO_INTERNAL_PROPERTY;
    optionalInfo->internalProperty.relationshipId = PROFILER_HEAP_OBJECT_NAME_ID_UNAVAILABLE;
    SetRelationshipInfo(optionalInfo->internalProperty, PROFILER_PROPERTY_TYPE_HEAP_OBJECT);
    optionalInfo->internalProperty.objectId = (PROFILER_HEAP_OBJECT_ID)jsVar;
}

void ActiveScriptProfilerHeapEnum::FillHeapObjectInternalUnnamedExternalProperty(ProfilerHeapObjectOptionalInfo *optionalInfo, PROFILER_EXTERNAL_OBJECT_ADDRESS externalObjectAddress)
{
    optionalInfo->infoType = PROFILER_HEAP_OBJECT_OPTIONAL_INFO_INTERNAL_PROPERTY;
    optionalInfo->internalProperty.relationshipId = PROFILER_HEAP_OBJECT_NAME_ID_UNAVAILABLE;
    SetRelationshipInfo(optionalInfo->internalProperty, PROFILER_PROPERTY_TYPE_EXTERNAL_OBJECT);
    optionalInfo->internalProperty.externalObjectAddress = externalObjectAddress;
}

UINT ActiveScriptProfilerHeapEnum::GetHeapObjectIndexPropertiesInfoSize(int propertyCount)
{
    return offsetof(ProfilerHeapObjectOptionalInfo, indexPropertyList.elements) + (sizeof(PROFILER_HEAP_OBJECT_RELATIONSHIP) * propertyCount);
}

HRESULT ActiveScriptProfilerHeapEnum::VerifyOnEntry()
{
    if (isClosed)
    {
        return E_ACCESSDENIED;
    }
    return m_scriptEngine.VerifyOnEntry(TRUE);
}

void ActiveScriptProfilerHeapEnum::CloseHeapEnum()
{
    isClosed = TRUE;
    if  (m_autoSetupRecyclerForNonCollectingMark)
    {
        delete m_autoSetupRecyclerForNonCollectingMark;
        m_autoSetupRecyclerForNonCollectingMark = NULL;
    }
    if (isInitialized)
    {
        m_scriptEngine.GetScriptContext()->ClearHeapEnum();
    }
}

bool ActiveScriptProfilerHeapEnum::IsJavascriptString(Js::RecyclableObject* obj)
{
    return !Js::JavascriptStringObject::Is(obj) && Js::JavascriptString::Is(obj);
}

#ifdef HEAP_ENUMERATION_VALIDATION

ActiveScriptProfilerHeapEnum::VtableMap::VtableMap() :
    pageAllocator(nullptr),
    arenaAllocator(_u("Vtable map"), &pageAllocator, Js::Throw::OutOfMemory), m_vtableMapHash(nullptr)
{
}

ActiveScriptProfilerHeapEnum::VtableMap::~VtableMap()
{
    // We don't care to check for multi-thread access as the initialization of the vtablehash map
    // is done under a lock
    pageAllocator.ClearConcurrentThreadId();
}

void
ActiveScriptProfilerHeapEnum::VtableMap::Initialize()
{
    if (m_vtableMapHash != nullptr)
    {
        return;
    }

    AutoCriticalSection autocs(ThreadContext::GetCriticalSection());

    // recheck after taking the critical section
    if (m_vtableMapHash != nullptr)
    {
        return;
    }
    m_vtableMapHash = CreateVtableHashMap(&arenaAllocator);
}
#endif
