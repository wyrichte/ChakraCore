//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "edgescriptDirect.h"

enum ErrorReason;

#define FATAL_ON_FAILED_API_RESULT(hr) if (FAILED(hr)) { JsStaticAPI::Error::ReportFatalException(hr, Fatal_Failed_API_Result); }

typedef interface ITrackingService ITrackingService;

namespace JsStaticAPI
{
    typedef HRESULT (__stdcall ScriptCallbackMethod)(int numArg, void** arguments);
    typedef void(*InitIteratorFunction)(Var, Var);
    typedef bool (*NextFunction)(Var, Var *, Var *);

    struct BinaryVerificationData
    {
        DWORD majorVersion;
        DWORD minorVersion;
        DWORD scriptEngineBaseSize;
        DWORD scriptEngineBaseOffset;
        DWORD scriptContextBaseSize;
        DWORD scriptContextBaseOffset;
        DWORD javascriptLibraryBaseSize;
        DWORD javascriptLibraryBaseOffset;
        DWORD typeOffset;
        DWORD typeIdOffset;
        DWORD taggedIntSize;
        DWORD javascriptNumberSize;
        DWORD customExternalObjectSize;
        DWORD typeIdLimit;
        DWORD numberUtilitiesBaseSize;
        DWORD numberUtilitiesBaseOffset;
    };

    class TaggedInt
    {
    public:
        static BOOL __stdcall Is(Var value);
        static Var __stdcall ToVarUncheck(long value);
        static int __stdcall ToInt32(Var obj);
    };

    class JavascriptNumber
    {
    public:
        static BOOL __stdcall Is(Var value);
        static double __stdcall GetValueUncheck(Var value);
    };

    class JavascriptLibrary
    {
    public:
        static Var __stdcall GetUndefined(IActiveScriptDirect* activeScriptdirect);
        static Var __stdcall GetNull(IActiveScriptDirect* activeScriptDirect);
        static Var __stdcall GetTrue(IActiveScriptDirect* activeScriptDirect);
        static Var __stdcall GetFalse(IActiveScriptDirect* activeScriptDirect);
        static Var __stdcall GetGlobalObject(IActiveScriptDirect* activeScriptDirect);
        // Get the Promise constructor function
        //  activeScriptDirect      : The IActiveScriptDirect pointer
        static Var __stdcall GetPromiseConstructor(IActiveScriptDirect* activeScriptDirect);
        // Get the Promise.resolve function
        //  activeScriptDirect      : The IActiveScriptDirect pointer
        static Var __stdcall GetPromiseResolve(IActiveScriptDirect* activeScriptDirect);
        // Get the Promise.prototype.then function
        //  activeScriptDirect      : The IActiveScriptDirect pointer
        static Var __stdcall GetPromiseThen(IActiveScriptDirect* activeScriptDirect);
        // Get the JSON.stringify function
        //  activeScriptDirect      : The IActiveScriptDirect pointer
        static Var __stdcall GetJSONStringify(IActiveScriptDirect* activeScriptDirect);
        // Get the Object.freeze function
        //  activeScriptDirect      : The IActiveScriptDirect pointer
        static Var __stdcall GetObjectFreeze(IActiveScriptDirect* activeScriptDirect);

        static HRESULT __stdcall SetNoScriptScope(__in ITrackingService *threadService, bool noScriptScope);
        static HRESULT __stdcall IsNoScriptScope(__in ITrackingService *threadService, __out bool *isNoScriptScope);

        static Var __stdcall GetArrayForEachFunction(IActiveScriptDirect* activeScriptDirect);
        static Var __stdcall GetArrayKeysFunction(IActiveScriptDirect* activeScriptDirect);
        static Var __stdcall GetArrayValuesFunction(IActiveScriptDirect* activeScriptDirect);
        static Var __stdcall GetArrayEntriesFunction(IActiveScriptDirect* activeScriptDirect);
        static PropertyId __stdcall GetPropertyIdSymbolIterator(IActiveScriptDirect* activeScriptDirect);

        static Var __stdcall CreateWeakMap(IActiveScriptDirect* activeScriptDirect);
        static HRESULT __stdcall WeakMapHas(IActiveScriptDirect* activeScriptDirect, Var instance, Var key, __out bool* has);
        static HRESULT __stdcall WeakMapSet(IActiveScriptDirect* activeScriptDirect, Var instance, Var key, Var value);
        static HRESULT __stdcall WeakMapGet(IActiveScriptDirect* activeScriptDirect, Var instance, Var key, __out Var *value, __out bool* result);
        static HRESULT __stdcall WeakMapDelete(IActiveScriptDirect* activeScriptDirect, Var instance, Var key, __out bool* result);

        static Var __stdcall GetIteratorPrototype(IActiveScriptDirect* activeScriptDirect);

        static PropertyId __stdcall GetPropertyIdSymbolToStringTag(IActiveScriptDirect* activeScriptDirect);

        static Var __stdcall CreateExternalEntriesFunction(IActiveScriptDirect* activeScriptDirect,
            JavascriptTypeId type, UINT byteCount, Var prototypeForIterator, InitIteratorFunction initFunction, NextFunction nextFunction);

        static Var __stdcall CreateExternalKeysFunction(IActiveScriptDirect* activeScriptDirect,
            JavascriptTypeId type, UINT byteCount, Var prototypeForIterator, InitIteratorFunction initFunction, NextFunction nextFunction);

        static Var __stdcall CreateExternalValuesFunction(IActiveScriptDirect* activeScriptDirect,
            JavascriptTypeId type, UINT byteCount, Var prototypeForIterator, InitIteratorFunction initFunction, NextFunction nextFunction);


        static void * CustomIteratorToExtension(Var iterator);

        static Var __stdcall CreateIteratorNextFunction(IActiveScriptDirect* activeScriptDirect, JavascriptTypeId type);

        // Get the DebugEval function from diagnostics object
        //  activeScriptDirect      : The IActiveScriptDirect pointer
        static Var __stdcall GetDebugEval(IActiveScriptDirect* activeScriptDirect);
        // Get the GetStackTrace function from diagnostics object
        //  activeScriptDirect      : The IActiveScriptDirect pointer
        static Var __stdcall GetStackTraceFunction(IActiveScriptDirect* activeScriptDirect);
#ifdef EDIT_AND_CONTINUE
        // Get the EditSource function from diagnostics object
        //  activeScriptDirect      : The IActiveScriptDirect pointer
        static Var __stdcall GetEditSource(IActiveScriptDirect* activeScriptDirect);
#endif
    };

    class DataConversion
    {
    public:
        static HRESULT __stdcall VarToBOOL(Var obj, BOOL* value);
        static HRESULT __stdcall VarToInt(Var obj, int* value);
        static HRESULT __stdcall VarToDate(Var obj, int* value);
        static IActiveScriptDirect* __stdcall VarToScriptDirectNoRef(Var obj);
        static JavascriptTypeId __stdcall GetTypeId(Var obj);
        static HRESULT VarToExtensionWithTypeIdRange(Var obj, void** buffer, JavascriptTypeId* typeId, JavascriptTypeId beginTypeId, JavascriptTypeId endTypeId);
        static Var __stdcall BOOLToVar(BOOL value, IActiveScriptDirect* activeScriptDirect) 
        {
            if (value)
            {
                return JavascriptLibrary::GetTrue(activeScriptDirect);
            }
            return JavascriptLibrary::GetFalse(activeScriptDirect);
        }
        static void __stdcall FillInBinaryVerificationData(BinaryVerificationData* binaryVerificationData);
    };

    class ExternalObject
    {
    public:
        static void ** TypeToExtension(HTYPE instance);     // Requires CustomExteranlType
        static void ** VarToExtension(Var obj);             // Requires CustomExternalObject
        static Var ExtensionToVar(void * buffer);           // Requires CustomExternalObject
        static HTYPE GetTypeFromVar(Var instance);          // Requires CustomExternalObject

        static HRESULT __stdcall BuildDOMDirectFunction(
            IActiveScriptDirect* activeScriptDirect,
            Var signature,
            ScriptMethod entryPoint,
            PropertyId nameId,
            UINT64 flags,
            UCHAR length,
            Var* jsFunction);
    };

    class FastDOM
    {
    public:
        static HRESULT GetObjectSlotAccessor(
            __in IActiveScriptDirect* activeScriptDirect,
            __in JavascriptTypeId typeId,
            __in PropertyId nameId,
            __in unsigned int slotIndex,
            __in_opt ScriptMethod getterFallBackEntryPoint,
            __in_opt ScriptMethod setterFallBackEntryPoint,
            __out_opt Var* getter,
            __out_opt Var* setter);

        static HRESULT GetTypeSlotAccessor(
            __in IActiveScriptDirect* activeScriptDirect,
            __in JavascriptTypeId typeId,
            __in PropertyId nameId,
            __in unsigned int slotIndex,
            __in_opt ScriptMethod getterFallBackEntryPoint,
            __in_opt ScriptMethod setterFallBackEntryPoint,
            __out_opt Var* getter,
            __out_opt Var* setter);
    };

    static HRESULT __stdcall EnterScriptCall(IActiveScriptDirect* activeScriptdirect, ScriptCallbackMethod callback);
    static HRESULT __stdcall LeaveScriptCall(IActiveScriptDirect* activeScriptdirect, ScriptCallbackMethod callback);

    class Error {
    public:
        static inline LONG FatalExceptionFilter(__in LPEXCEPTION_POINTERS lpep);
        static void ReportFatalException(__in HRESULT exceptionCode, __in ErrorReason reasonCode);
    };
};
