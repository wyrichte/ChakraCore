//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

// Description: The base class for ScriptEngine that is used by static lib shared between
// trident and chakra.

//////////////////////////////////////////////
// IMPORTANT
// IF MEMORY LAYOUT OF THIS CLASS IS CHANGED, WE NEED TO HAVE MATCHING MSHTML BUILT TOGETHER WITH NEW JSCRIPT9
//
//////////////////////////////////////////////


#pragma once
namespace Js
{
    class ScriptContext;
    class RecyclableObject;
    class JavascriptExceptionObject;
}

class ScriptSite;
class ThreadContext;

typedef void (*InitIteratorFunction)(Var, Var);
typedef bool (*NextFunction)(Var, Var *, Var *);

class ScriptEngineBase : public IActiveScriptDirect
{
public:
    ScriptEngineBase() ;
    virtual ~ScriptEngineBase();

    Js::ScriptContext* GetScriptContext() const { return scriptContext; }
    ScriptSite* GetScriptSiteHolder() const { return scriptSiteHolder; }
    void SetScriptSiteHolder(ScriptSite* scriptSite) {scriptSiteHolder = scriptSite; }
    static  ScriptEngineBase* __stdcall FromIActiveScriptDirect(IActiveScriptDirect* activeScriptDirect)
    {
        return static_cast<ScriptEngineBase*>(activeScriptDirect);
    }

    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj) override;
    STDMETHOD_(ULONG,AddRef)(void) override;
    STDMETHOD_(ULONG,Release)(void) override;
    //
    // IActiveScriptDirect
    //

    HRESULT STDMETHODCALLTYPE SetHostObject(
        __in Var hostObject,
        __in Var secureHostObject);

    HRESULT STDMETHODCALLTYPE GetHostObject(__out Var* hostObject);

    HRESULT STDMETHODCALLTYPE ReserveGlobalProperty(
        __in PropertyId propertyId);

    HRESULT STDMETHODCALLTYPE GetOrAddPropertyId(
        __RPC__in LPCWSTR name,
        __RPC__out PropertyId* id);

    HRESULT STDMETHODCALLTYPE GetPropertyName(
        __RPC__in PropertyId id,
        __RPC__out LPCWSTR* name);

    HRESULT STDMETHODCALLTYPE Parse(
        __in LPWSTR scriptText,
        __out Var* scriptFunc);

    HRESULT STDMETHODCALLTYPE Execute(
        __in Var instance,
        __in CallInfo callInfo,
        __in_xcount(callInfo.Count)  Var* parameters,
        __in IServiceProvider* serviceProvider,
        __out_opt Var* varResult);

    HRESULT STDMETHODCALLTYPE GetGlobalObject(
        __out Var* globalObject);

    HRESULT STDMETHODCALLTYPE GetDefaultTypeOperations(
        __out ITypeOperations** operations);

    HRESULT STDMETHODCALLTYPE GetJavascriptOperations(__out IJavascriptOperations** jsOperations);

    HRESULT STDMETHODCALLTYPE GetTypeIdForType(
        __in HTYPE type,
        __out JavascriptTypeId* typeIdRef);

    HRESULT STDMETHODCALLTYPE GetTypeIdForVar(
        __in Var instance,
        __out JavascriptTypeId* typeIdRef);

    HRESULT STDMETHODCALLTYPE ReserveStaticTypeIds(
        __in int first,
        __in int last);

    HRESULT STDMETHODCALLTYPE ReserveTypeIds(
        __in int count,
        __out JavascriptTypeId* firstTypeId);

    HRESULT STDMETHODCALLTYPE RegisterWellKnownTypeId(
        __in WellKnownType wellKnownType,
        __in JavascriptTypeId typeId);

    HRESULT STDMETHODCALLTYPE CreateObject(
        __out Var* instance);

    HRESULT STDMETHODCALLTYPE CreateConstructor(
        __in Var objectPrototype,
        __in ScriptMethod entryPoint,
        __in PropertyId nameId,
        __in BOOL bindReference,
        __out Var* instance);

    HRESULT STDMETHODCALLTYPE CreateDeferredConstructor(
        __in_opt ScriptMethod entryPoint,
        __in PropertyId nameId,
        __in InitializeMethod method,
        __in unsigned short deferredTypeSlots,
        __in BOOL hasAccessors,
        __in BOOL bindReference,
        __out Var* instance);

    HRESULT STDMETHODCALLTYPE CreateType(
        __in JavascriptTypeId typeId,
        __in __RPC__in_ecount_full(inheritedTypeIdsCount) const JavascriptTypeId* inheritedTypeIds,
        __in UINT inheritedTypeIdsCount,
        __in Var varPrototype,
        __in ScriptMethod entryPoint,
        __in ITypeOperations* operations,
        __in BOOL fDeferred,
        __in PropertyId nameId,
        __in BOOL bindReference,
        __out HTYPE* typeRef);

    HRESULT STDMETHODCALLTYPE CreateTypeWithExtraSlots(
        __in JavascriptTypeId typeId,
        __in __RPC__in_ecount_full(inheritedTypeIdsCount) const JavascriptTypeId* inheritedTypeIds,
        __in UINT inheritedTypeIdsCount,
        __in Var varPrototype,
        __in ScriptMethod entryPoint,
        __in ITypeOperations* operations,
        __in BOOL fDeferred,
        __in PropertyId nameId,
        __in BOOL bindReference,
        __in UINT extraSlotsCount,
        __out HTYPE* typeRef) sealed;

    HRESULT STDMETHODCALLTYPE CreateTypedObject(
        __in HTYPE type,
        __in int byteCount,
        __in BOOL bindReference,
        __out Var* instance);

    HRESULT STDMETHODCALLTYPE CreateArrayObject(
        __in UINT length,
        __out Var* instance);

    HRESULT STDMETHODCALLTYPE IsArrayObject(
        __in Var instance,
        __out BOOL* isArray);

    HRESULT STDMETHODCALLTYPE ChangeTypeFromVar(
        __in Var instance,
        __in VARTYPE varType,
        __out VARIANT* outVariant);

    HRESULT STDMETHODCALLTYPE ChangeTypeToVar(
        __in VARIANT* inVariant,
        __out Var* instance);

    HRESULT STDMETHODCALLTYPE VarToDouble(
        __in Var instance,
        __out double* d);

    HRESULT STDMETHODCALLTYPE DoubleToVar(
        __in double d,
        __out Var* instance);

    HRESULT STDMETHODCALLTYPE DateToVar(
        __in double d,
        __out Var* instance);

    HRESULT STDMETHODCALLTYPE SYSTEMTIMEToVar(
            __in SYSTEMTIME *pst,
            __out Var* instance);

    HRESULT STDMETHODCALLTYPE VarToSYSTEMTIME(
            __in Var instance,
            __out SYSTEMTIME* result);

    HRESULT STDMETHODCALLTYPE VarToInt(
        __in Var instance,
        __out int* i);

    HRESULT STDMETHODCALLTYPE IntToVar(
        __in int i,
        __out Var* instance);

    HRESULT STDMETHODCALLTYPE VarToBOOL(
        __in Var instance,
        __out BOOL* b);

    HRESULT STDMETHODCALLTYPE BOOLToVar(
        __in BOOL b,
        __out Var* instance);

    HRESULT STDMETHODCALLTYPE VarToString(
        __in Var instance,
        __out BSTR* str);

    HRESULT STDMETHODCALLTYPE VarToRawString(
        __in Var instance,
        __out const WCHAR** str,
        __out unsigned int* length);

    HRESULT STDMETHODCALLTYPE StringToVar(
        __in_ecount(length) const WCHAR* str,
        __in int length,
        __out Var* instance);

    HRESULT STDMETHODCALLTYPE VarToExtension(
        __in Var instance,
        __deref_out void** buffer,
        __out JavascriptTypeId* typeIdRef);

    HRESULT STDMETHODCALLTYPE ExtensionToVar(
        __in void* buffer,
        __out Var* instance);

    HRESULT STDMETHODCALLTYPE DispExToVar(
        __in IDispatchEx* pdispex,
        __out Var* instance);

    HRESULT STDMETHODCALLTYPE VarToDispEx(
        __in Var instance,
        __out IDispatchEx** pdispex);

    HRESULT STDMETHODCALLTYPE VarToInt64(
        __in Var instance,
        __out __int64 * value);

    HRESULT STDMETHODCALLTYPE Int64ToVar(
        __in __int64 i,
        __out Var* instance);

    HRESULT STDMETHODCALLTYPE VarToUInt64(
        __in Var instance,
        __out unsigned __int64 * value);

    HRESULT STDMETHODCALLTYPE UInt64ToVar(
        __in unsigned __int64 i,
        __out Var* instance);

    HRESULT STDMETHODCALLTYPE InspectableUnknownToVar(
        __in IUnknown* unknown,
        __out Var* instance);

    HRESULT STDMETHODCALLTYPE GetScriptType(
        __in Var instance,
        __out ScriptType* scriptType);

    HRESULT STDMETHODCALLTYPE GetServiceProvider(
        IServiceProvider** serviceProvider);

    HRESULT STDMETHODCALLTYPE GetServiceProviderOfCaller(
        IServiceProvider** serviceProvider);

    HRESULT STDMETHODCALLTYPE ReleaseServiceProviderOfCaller(
        IServiceProvider* serviceProvider);

    HRESULT STDMETHODCALLTYPE GetNull(
        __out Var* nullValue);

    HRESULT STDMETHODCALLTYPE GetUndefined(
        __out Var* undefinedValue);

    HRESULT STDMETHODCALLTYPE IsFunctionObject(
        __in IDispatch* pdisp,
        __out BOOL* result);

    HRESULT STDMETHODCALLTYPE IsHostDispatch(
        __in Var instance,
        __out BOOL* result);

    HRESULT STDMETHODCALLTYPE BuildDOMDirectFunction(
        __in Var signature,
        __in ScriptMethod entryPoint,
        __in PropertyId nameId,
        __in JavascriptTypeId prototypeTypeId,
        __in UINT64 flags,
        __out Var* jsFunction);

    /* Static lib variant */
    virtual HRESULT STDMETHODCALLTYPE BuildDOMDirectFunction(
        __in Var signature,
        __in ScriptMethod entryPoint,
        __in PropertyId nameId,
        __in UINT64 flags,
        __in UCHAR length,
        __out Var* jsFunction);

    HRESULT STDMETHODCALLTYPE GetTypedObjectSlotAccessor(
        __in JavascriptTypeId typeId,
        __in PropertyId nameId,
        __in unsigned int slotIndex,
        __out_opt Var* getter,
        __out_opt Var* setter);

    virtual HRESULT STDMETHODCALLTYPE GetObjectSlotAccessor(
        __in JavascriptTypeId typeId,
        __in PropertyId nameId,
        __in unsigned int slotIndex,
        __in ScriptMethod getterFallBackEntryPoint,
        __in ScriptMethod setterFallBackEntryPoint,
        __out_opt Var* getter,
        __out_opt Var* setter);

    virtual HRESULT STDMETHODCALLTYPE GetTypeSlotAccessor(
        __in JavascriptTypeId typeId,
        __in PropertyId nameId,
        __in unsigned int slotIndex,
        __in ScriptMethod getterFallBackEntryPoint,
        __in ScriptMethod setterFallBackEntryPoint,
        __out_opt Var* getter,
        __out_opt Var* setter);

    HRESULT STDMETHODCALLTYPE CreateErrorObject(
        __in JsErrorType errorType,
        __in HRESULT errorCode,
        __in LPCWSTR message,
        __out Var* errorObject);

    HRESULT STDMETHODCALLTYPE ReinitializeObject(__in Var instance, __in HTYPE type, __in BOOL keepProperties);

    HRESULT STDMETHODCALLTYPE UnrootScriptEngine();

    HRESULT STDMETHODCALLTYPE ReleaseAndRethrowException(__in HRESULT hr);

    HRESULT STDMETHODCALLTYPE CreatePixelArray(
        __in UINT length,
        __out Var *instance);

    HRESULT STDMETHODCALLTYPE GetPixelArrayBuffer(
        __in Var instance,
        __deref_out_bcount(*pBufferLength) BYTE **ppBuffer,
        __out UINT *pBufferLength) sealed;

    HRESULT STDMETHODCALLTYPE CreateArrayBuffer(
        __in UINT length,
        __out Var *instance);

    HRESULT STDMETHODCALLTYPE CreateArrayBufferFromBuffer(
        __in BYTE * buffer,
        __in UINT length,
        __out Var *instance);

    HRESULT STDMETHODCALLTYPE CreateTypedArray(
        __in TypedArrayType typedArrayType,
        __in_opt Var baseVar,
        __in UINT length,
        __out Var* instance);

    HRESULT STDMETHODCALLTYPE CreatePromise(
        __deref_out Var* promise,
        __deref_out Var* resolveFunc,
        __deref_out Var* rejectFunc);

    virtual HRESULT STDMETHODCALLTYPE EnsurePromiseResolveFunction(
        __out Var* resolveFunc
    );

    virtual HRESULT STDMETHODCALLTYPE EnsurePromiseThenFunction(
        __out Var* thenFunc
    );

    virtual HRESULT STDMETHODCALLTYPE EnsureJSONStringifyFunction(
        __out Var* jsonStringifyFunc
    );

    virtual HRESULT STDMETHODCALLTYPE EnsureObjectFreezeFunction(
        __out Var* objectFreezeFunc
    );

    HRESULT STDMETHODCALLTYPE ParseJson(
        __in LPCWSTR str,
        __in UINT length,
        __deref_out Var* var);

    HRESULT STDMETHODCALLTYPE GetTypedArrayBuffer(
        __in Var instance,
        __deref_out_bcount(*pBufferLength) BYTE **ppBuffer,
        __out UINT *pBufferLength,
        __out_opt TypedArrayType* typedArrayType,
        __out_opt INT* elementSize);

    HRESULT STDMETHODCALLTYPE DetachTypedArrayBuffer(
        __in Var instance,
        __deref_out_bcount(*pBufferLength) BYTE** ppBuffer,
        __out UINT* pBufferLength,
        __out TypedArrayBufferAllocationType * pAllocationType,
        __out_opt TypedArrayType* typedArrayType,
        __out_opt INT* elementSize);

    HRESULT STDMETHODCALLTYPE FreeDetachedTypedArrayBuffer(
        __in BYTE * pBuffer,
        __in UINT bufferLength,
        __in TypedArrayBufferAllocationType allocationType);


    HRESULT STDMETHODCALLTYPE CreateRegex(
        __in LPCWSTR pattern,
        __in UINT patternLength,
        __in RegexFlags flags,
        __deref_out Var *regex);

    HRESULT STDMETHODCALLTYPE RegexTest(
        __in Var regex,
        __in LPCWSTR input,
        __in UINT inputLength,
        __in BOOL mustMatchEntireInput,
        __out BOOL *matched);

    HRESULT STDMETHODCALLTYPE Serialize(
        __in ISCAContext* context,
        __in Var instance,
        __in_xcount(cTransferableVars) Var* transferableVars,
        __in UINT cTransferableVars,
        __in IStream* pOutSteam,
        __in IServiceProvider* serviceProvider);

    HRESULT STDMETHODCALLTYPE Deserialize(
        __in ISCAContext* context,
        __in IStream* pInSteam,
        __in IServiceProvider* serviceProvider,
        __out Var* pValue);

    HRESULT STDMETHODCALLTYPE Discard(
        __in ISCAContext* context,
        __in IStream* pInSteam,
        __in IServiceProvider* serviceProvider);

    HRESULT STDMETHODCALLTYPE IsVarFunctionObject(
        __in Var instance,
        __out BOOL* result);

    HRESULT STDMETHODCALLTYPE VarToDate(
        __in Var instance,
        __out double* d);

    HRESULT STDMETHODCALLTYPE IsPrimitiveType(
        __in Var instance,
        __out BOOL* result);

    HRESULT STDMETHODCALLTYPE VerifyBinaryConsistency(__in void* dataContext);

    HRESULT STDMETHODCALLTYPE IsObjectCallable(Var obj, BOOL* isCallable);

    HRESULT STDMETHODCALLTYPE InvalidateHostObjects();

    HRESULT STDMETHODCALLTYPE GetCurrentSourceInfo(
        __out LPCWSTR* url,
        __out ULONG* line,
        __out ULONG* column);

    HRESULT STDMETHODCALLTYPE ClearRecordedException();

    HRESULT STDMETHODCALLTYPE EmitStackTraceEvent(__in UINT64 operationId, __in USHORT maxFrameCount);

    HRESULT STDMETHODCALLTYPE VarToNativeArray(Var arayObject,
        JsNativeValueType valueType,
        __deref_out_bcount_opt(*length*(*elementSize)) byte** contentBuffer,
        UINT* length,
        UINT* elementSize);

    HRESULT STDMETHODCALLTYPE ThrowException(_In_ Var exceptionObject);

    HRESULT STDMETHODCALLTYPE InitializeModuleRecord(
        /* [in] */ __RPC__in_opt ModuleRecord referencingModule,
        /* [size_is][in] */ __RPC__in_ecount_full(specifierLength) LPCWSTR normalizedSpecifier,
        /* [in] */ UINT specifierLength,
        /* [out] */ __RPC__deref_out_opt ModuleRecord *moduleRecord) override;

    HRESULT STDMETHODCALLTYPE ParseModuleSource(
        /* [in] */ __RPC__in ModuleRecord requestModule,
        /* [in] */ __RPC__in_opt IUnknown *punkContext,
        /* [in] */ __RPC__in void *sourceContext,
        /* [size_is][in] */ __RPC__in_ecount_full(sourceLength) byte *sourceText,
        /* [in] */ unsigned long sourceLength,
        /* [in] */ ParseModuleSourceFlags sourceFlag,
        /* [in] */ unsigned long startingLine,
        /* [in] */ unsigned long startingColumn,
        /* [in] */ unsigned long startingOffset,
        /* [out] */ __RPC__deref_out_opt Var *exceptionVar) override;

    HRESULT STDMETHODCALLTYPE ModuleEvaluation(
        /* [in] */ __RPC__in_opt ModuleRecord requestModule,
        /* [out] */ __RPC__deref_out_opt Var *varResult) override;

    HRESULT STDMETHODCALLTYPE SetModuleHostInfo(
        /* [in] */ __RPC__in_opt ModuleRecord requestModule,
        /* [in] */ ModuleHostInfoKind moduleHostState,
        /* [in] */ __RPC__in void *hostInfo) override;

    HRESULT STDMETHODCALLTYPE GetModuleHostInfo(
        /* [in] */ __RPC__in_opt ModuleRecord requestModule,
        /* [in] */ ModuleHostInfoKind moduleHostState,
        /* [out] */ __RPC__deref_out_opt void **hostInfo) override;

    HRESULT STDMETHODCALLTYPE TriggerDOMMutationBreakpoint();

    HRESULT STDMETHODCALLTYPE CreateScriptErrorFromVar(Var errorObject, IActiveScriptError** scriptError) override;

    HRESULT VerifyOnEntry(BOOL allowedInHeapEnum = FALSE);

    HRESULT CreateTypeFromScript(
        __in TypeId typeId,
        __in Var varPrototype,
        __in ScriptMethod entryPoint,
        __in ITypeOperations* operations,
        __in BOOL fDeferred,
        __in PropertyId nameId,
        __in BOOL bindReference,
        __out HTYPE* typeRef);

    HRESULT CreateServiceProvider(bool asCaller, IServiceProvider** serviceProvider);
    static HRESULT HandleSCAException(Js::JavascriptExceptionObject* exceptionObject, Js::ScriptContext* scriptContext, IServiceProvider* pspCaller);

    template <class EnsureFunc>
    HRESULT EnsureFunctionValidation(__out Var* func, EnsureFunc ensure)
    {
        IfNullReturnError(func, E_INVALIDARG);

        HRESULT hr = S_OK;
        hr = VerifyOnEntry();

        if (FAILED(hr))
        {
            return hr;
        }

        BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, false)
        {
            *func = ensure(scriptContext);
        }
        END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)

        return hr;
    }

    virtual HRESULT STDMETHODCALLTYPE EnsureArrayPrototypeForEachFunction(__out Var* func);
    virtual HRESULT STDMETHODCALLTYPE EnsureArrayPrototypeKeysFunction(__out Var* func);
    virtual HRESULT STDMETHODCALLTYPE EnsureArrayPrototypeEntriesFunction(__out Var* func);
    virtual HRESULT STDMETHODCALLTYPE EnsureArrayPrototypeValuesFunction(__out Var* func);

    virtual HRESULT STDMETHODCALLTYPE CreateWeakMap(__out Var *mapInstance);
    virtual HRESULT STDMETHODCALLTYPE WeakMapHas(Var mapInstance, Var key, __out bool *has);
    virtual HRESULT STDMETHODCALLTYPE WeakMapSet(Var mapInstance, Var key, Var value);
    virtual HRESULT STDMETHODCALLTYPE WeakMapGet(Var mapInstance, Var key, __out Var *value, __out bool *found);
    virtual HRESULT STDMETHODCALLTYPE WeakMapDelete(Var mapInstance, Var key, __out bool *has);

    virtual HRESULT STDMETHODCALLTYPE CreateIteratorEntriesFunction(JavascriptTypeId typeId,
        uint byteCount, Var prototypeForIterator, InitIteratorFunction initFunction, NextFunction nextFunction, __out Var* func);

    virtual HRESULT STDMETHODCALLTYPE CreateIteratorKeysFunction(JavascriptTypeId typeId,
        uint byteCount, Var prototypeForIterator, InitIteratorFunction initFunction, NextFunction nextFunction, __out Var* func);

    virtual HRESULT STDMETHODCALLTYPE CreateIteratorValuesFunction(JavascriptTypeId typeId,
        uint byteCount, Var prototypeForIterator, InitIteratorFunction initFunction, NextFunction nextFunction, __out Var* func);

    HRESULT STDMETHODCALLTYPE CreateIteratorValuesFunction(JavascriptTypeId typeId, __out Var* func);

    virtual HRESULT STDMETHODCALLTYPE CreateIteratorNextFunction(JavascriptTypeId typeId, __out Var* func);

public:
    Js::ScriptContext*  scriptContext;

protected:
    // Data field places here
    long                m_refCount;
    ThreadContext*      threadContext;
    ScriptSite*         scriptSiteHolder;
    BOOL                wasBinaryVerified;
    BOOL                wasScriptDirectEnabled;


    HRESULT CreateTypeFromPrototype(
        __in TypeId typeId,
        __in Js::RecyclableObject* objPrototype,
        __in ScriptMethod entryPoint,
        __in ITypeOperations* operations,
        __in BOOL fDeferred,
        __in PropertyId nameId,
        __in BOOL bindReference,
        __out HTYPE* typeRef);

    HRESULT CreateTypeFromPrototypeInternal(
        __in TypeId typeId,
        __in __RPC__in_ecount_full(inheritedTypeIdsCount) const JavascriptTypeId* inheritedTypeIds,
        __in UINT inheritedTypeIdsCount,
        __in Js::RecyclableObject* objPrototype,
        __in ScriptMethod entryPoint,
        __in ITypeOperations* operations,
        __in BOOL fDeferred,
        __in PropertyId nameId,
        __in BOOL bindReference,
        __in UINT extraSlotCount,
        __out HTYPE* typeRef);

    HRESULT STDMETHODCALLTYPE CreateTypedObjectFromScript(
        __in HTYPE type,
        __in int byteCount,
        __in BOOL bindReference,
        __out Var* instance);

    HRESULT GetOPrototypeInformationForTypeCreation(
        __in Var &varPrototype,
        __in PropertyId nameId,
        __out Js::RecyclableObject** objPrototype);

    HRESULT ValidateBaseThread(void);

    HRESULT STDMETHODCALLTYPE CreateIteratorCreatorFunction(
        JavascriptTypeId typeId,
        Js::JavascriptMethod entryPoint,
        uint byteCount,
        Var prototypeForIterator, 
        InitIteratorFunction initFunction,
        NextFunction nextFunction,
        __out Var* func);

};
