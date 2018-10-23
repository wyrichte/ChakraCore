//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "edgescriptDirect.h"

enum ErrorReason;

#define FATAL_ON_FAILED_API_RESULT(hr) if (FAILED(hr)) { JsStaticAPI::Error::ReportFatalException(hr, Fatal_Failed_API_Result); }

typedef interface IJavascriptThreadService IJavascriptThreadService;
class ChakraEngine;

namespace Js
{
    class RefCountedBuffer;
};

namespace Streams
{
    struct IByteChunk;
};

namespace JsStaticAPI
{
    typedef HRESULT (__stdcall ScriptCallbackMethod)(int numArg, void** arguments);
    typedef void(*InitIteratorFunction)(Var, Var);
    typedef bool (*NextFunction)(Var, Var *, Var *);
    typedef void (__stdcall *StaticHostPromiseRejectionTrackerCallback)(_In_ Var promise, _In_ Var reason, _In_ bool handled, _In_opt_ void *callbackState);

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

    enum ScriptEncodingType
    {
        Utf8,
        Utf16
    };

    enum ScriptContainerType
    {
        HeapAllocatedBuffer,
        ChunkPtr // for either IByteChunk or IWideCharChunk
    };

    struct ScriptContents
    {
        union ScriptContainer
        {
            LPCOLESTR strUtf16;
            BYTE*     strUtf8;
            Streams::IByteChunk* utf8ByteChunk;

            ScriptContainer() :
                strUtf16(NULL)
            {}

            ~ScriptContainer() {}
        } container;

        ScriptEncodingType encodingType;
        ScriptContainerType containerType;
        DWORD_PTR sourceContext;
        ULONG contentStartOffset;
        ULONG startingLineNumber;
        size_t contentLengthInBytes;
        LPCWSTR fullPath;

        ScriptContents() :
            encodingType(Utf8),
            containerType(HeapAllocatedBuffer),
            sourceContext(NULL),
            contentStartOffset(0),
            startingLineNumber(0),
            contentLengthInBytes(0),
            fullPath(NULL)
        {
        }
    };

    struct ScriptExecuteMetadata
    {
        IUnknown* context;
        DWORD flags;
        LPCOLESTR rootScriptName;

        ScriptExecuteMetadata() :
            context(nullptr),
            flags(0),
            rootScriptName(NULL)
        {
        }
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
        // Set the HostPromiseRejectionTrackerCallback
        //  activeScriptDirect      : The IActiveScriptDirect pointer
        static void __stdcall SetHostPromiseRejectionTracker(_In_ IActiveScriptDirect* activeScriptDirect, _In_opt_ StaticHostPromiseRejectionTrackerCallback promiseRejectionTrackerCallback, _In_opt_ void *callbackState);

        // Get the JSON.stringify function
        //  activeScriptDirect      : The IActiveScriptDirect pointer
        static Var __stdcall GetJSONStringify(IActiveScriptDirect* activeScriptDirect);
        // Get the Object.freeze function
        //  activeScriptDirect      : The IActiveScriptDirect pointer
        static Var __stdcall GetObjectFreeze(IActiveScriptDirect* activeScriptDirect);

        static HRESULT __stdcall SetNoScriptScope(__in IJavascriptThreadService *threadService, bool noScriptScope);
        static HRESULT __stdcall IsNoScriptScope(__in IJavascriptThreadService *threadService, __out bool *isNoScriptScope);

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
        static PropertyId __stdcall GetPropertyIdSymbolUnscopables(IActiveScriptDirect* activeScriptDirect);

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

        /// Detaches and returns the backing refCounted buffer from a JavaScript ArrayBuffer instance. Notes:
        ///     - After this call, code in JavaScript will see the ArrayBuffer as empty and will not be able to use it.
        ///     - In order to clean up the memory buffer when it's no longer needed, FreeDetachedTypedArrayBuffer must be called (must use RefCountedBuffer for that)
        ///     The RefCountedBuffer is a instance which is holding the memory buffer and refCount. Use GetBufferContent to get the underlying memory buffer.
        static HRESULT __stdcall DetachTypedArrayBuffer(IActiveScriptDirect* activeScriptDirect,
            __in Var instance,
            __out Js::RefCountedBuffer** refCountedDetachedBuffer,
            __out_opt BYTE** detachedBuffer,
            __out UINT* bufferLength,
            __out TypedArrayBufferAllocationType * allocationType,
            __out_opt TypedArrayType* typedArrayType,
            __out_opt INT* elementSize);

        /// Frees/Release the RefCounted buffer obtained by calling DetachTypedArrayBuffer.
        static HRESULT __stdcall FreeDetachedTypedArrayBuffer(IActiveScriptDirect* activeScriptDirect,
            __in Js::RefCountedBuffer* refCountedBuffer,
            __in UINT bufferLength,
            __in TypedArrayBufferAllocationType allocationType);

        /// Get underlying buffer blob from the RefCountedBuffer.
        static HRESULT __stdcall GetBufferContent(IActiveScriptDirect* activeScriptDirect, __in Js::RefCountedBuffer *buffer, __out BYTE **bufferContent);

        static HRESULT __stdcall SetPrivilegeLevelLowForDiagOM(IActiveScriptDirect* activeScriptDirect);
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

    class Legacy
    {
    public:
        static IActiveScriptDirect* __stdcall GetChakraEngineIASD(__in ChakraEngine *chakraEngine);
    };

    static HRESULT __stdcall EnterScriptCall(IActiveScriptDirect* activeScriptdirect, ScriptCallbackMethod callback);
    static HRESULT __stdcall LeaveScriptCall(IActiveScriptDirect* activeScriptdirect, ScriptCallbackMethod callback);

    class Error {
    public:
        static inline LONG FatalExceptionFilter(__in LPEXCEPTION_POINTERS lpep);
        static void ReportFatalException(__in HRESULT exceptionCode, __in ErrorReason reasonCode);
    };

    class Script
    {
    public:
        // Executes arbitrary script contents
        static HRESULT Execute(
            /* [in]      */ IActiveScriptDirect* activeScriptDirect,
            /* [in]      */ ScriptContents* contents,
            /* [in]      */ ScriptExecuteMetadata* metadata,
            /* [out_opt] */ VARIANT*  pvarResult,
            /* [out_opt] */ EXCEPINFO* pexcepinfo);

        static HRESULT GenerateByteCodeBuffer(
            IActiveScriptDirect* activeScriptDirect,
            DWORD dwSourceCodeLength,
            BYTE *utf8Code,
            IUnknown *punkContext,
            DWORD_PTR dwSourceContext,
            EXCEPINFO *pexcepinfo,
            DWORD     dwFlags,
            BYTE **byteCode,
            DWORD *pdwByteCodeSize);

        static HRESULT ExecuteByteCodeBuffer(
            IActiveScriptDirect* activeScriptDirect,
            DWORD dwByteCodeSize,
            BYTE *byteCode,
            IActiveScriptByteCodeSource *pbyteCodeSource,
            IUnknown *punkContext,
            DWORD_PTR dwSourceContext,
            DWORD     dwFlags,
            EXCEPINFO *pexcepinfo,
            VARIANT * pvarResult);
    };

    class BGParse
    {
    public:
        static HRESULT QueueBackgroundParse(ScriptContents* contents, DWORD* dwBgParseCookie);
        static HRESULT ExecuteBackgroundParse(DWORD dwBgParseCookie, IActiveScriptDirect* activeScriptDirect, DWORD_PTR dwSourceContext, DWORD dwFlags, VARIANT* pvarResult, EXCEPINFO* pexcepinfo);
        static bool    DiscardBackgroundParse(DWORD dwBgParseCookie, void* buffer);
    };
};
