#include "edgescriptDirect.h"

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

        static HRESULT __stdcall SetNoScriptScope(__in IUnknown *threadService, bool noScriptScope);
        static HRESULT __stdcall IsNoScriptScope(__in IUnknown *threadService, __out bool *isNoScriptScope);
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

    static HRESULT __stdcall EnterScriptCall(IActiveScriptDirect* activeScriptdirect, ScriptCallbackMethod callback);
    static HRESULT __stdcall LeaveScriptCall(IActiveScriptDirect* activeScriptdirect, ScriptCallbackMethod callback);
};
