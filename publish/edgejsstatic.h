//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "edgescriptDirect.h"

typedef interface ITrackingService ITrackingService;

namespace JsStaticAPI
{
    typedef HRESULT (__stdcall ScriptCallbackMethod)(int numArg, void** arguments);
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
        static HTYPE GetTypeFromVar(Var instance);          // Requires CustomExternalObject
    };

    static HRESULT __stdcall EnterScriptCall(IActiveScriptDirect* activeScriptdirect, ScriptCallbackMethod callback);
    static HRESULT __stdcall LeaveScriptCall(IActiveScriptDirect* activeScriptdirect, ScriptCallbackMethod callback);
};
