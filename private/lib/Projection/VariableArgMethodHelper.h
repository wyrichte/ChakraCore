//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------
#pragma once

// Description: Macros used for methods that have variable arguments and need manual stack cleanup
namespace Projection
{
// Static Method Name that stores the register and calls helper
#define ClassMethodImplName(className, methodName) className::##methodName##Impl
#define ClassMethodName(className, methodName) className::##methodName

#if _M_IX86
#define ClassMethod_VarArgT_ImplName(className, methodName) ClassMethodImplName(className, methodName)
#elif _M_AMD64 || defined(_M_ARM32_OR_ARM64)

#define Delegate_Invoke 0
#define ArrayAsVector_IndexOf 1
#define ArrayAsVector_SetAt 2
#define ArrayAsVector_InsertAt 3
#define ArrayAsVector_Append 4

#define CUnknownImpl_Method(id) __CUnknownImpl_Method##id
#define ClassMethod_VarArgT_ImplName_From_MethodId(id) CUnknownImpl_Method(id)
#define ClassMethod_VarArgT_ImplName(className, methodName) ClassMethod_VarArgT_ImplName_From_MethodId(className##_##methodName)

#define GetMethodId(className, methodName) className##_##methodName
#endif

// VTable Manipulation
#define GetClassFromIUnknown(className, This) CONTAINING_RECORD(This, className, m_pvtbl)

#define CUnknownImpl_VTableEntry(className, methodName) (PFN_VTABLE_ENTRY)(&ClassMethodImplName(className, methodName))
#define CUnknownImpl_VarArgT_VTableEntry(className, methodName) (PFN_VTABLE_ENTRY)(&ClassMethod_VarArgT_ImplName(className, methodName))

#define UnknownImpl_VTable_Entries()                                                                                                                                        \
    CUnknownImpl_VTableEntry(CUnknownImpl, QueryInterface),                                                                                                                 \
    CUnknownImpl_VTableEntry(CUnknownImpl, AddRef),                                                                                                                         \
    CUnknownImpl_VTableEntry(CUnknownImpl, Release),

#define InspectableImpl_VTable_Entries()                                                                                                                                    \
    UnknownImpl_VTable_Entries()                                                                                                                                            \
    CUnknownImpl_VTableEntry(CUnknownImpl, GetIids),                                                                                                                        \
    CUnknownImpl_VTableEntry(CUnknownImpl, GetRuntimeClassName),                                                                                                            \
    CUnknownImpl_VTableEntry(CUnknownImpl, GetTrustLevel),

#define Declare_UnknownImpl_Extern_VTable(VTableName) extern const PFN_VTABLE_ENTRY VTableName##[];

#define Define_UnknownImpl_VTable(VTableName, ...)                                                                                                                          \
    extern const PFN_VTABLE_ENTRY VTableName##[] =                                                                                                                          \
    {                                                                                                                                                                       \
        UnknownImpl_VTable_Entries()                                                                                                                                        \
        __VA_ARGS__                                                                                                                                                         \
    };

#define Declare_InspectableImpl_Extern_VTable(VTableName) Declare_UnknownImpl_Extern_VTable(VTableName);

#define Define_InspectableImpl_VTable(VTableName, ...)                                                                                                                      \
    extern const PFN_VTABLE_ENTRY VTableName##[] =                                                                                                                          \
    {                                                                                                                                                                       \
        InspectableImpl_VTable_Entries()                                                                                                                                    \
        __VA_ARGS__                                                                                                                                                         \
    };

#define CUnknown_BeginScriptEnter(scriptClosedHR)                                                                                                                           \
    /* Use the script site to determine if we can access ourselves */                                                                                                       \
    if (m_threadId != ::GetCurrentThreadId())                                                                                                                               \
    {                                                                                                                                                                       \
        return RPC_E_WRONG_THREAD;                                                                                                                                          \
    }                                                                                                                                                                       \
    /* When scriptSite is closed, the UnknownImpl's scriptSite ptr is set to null */                                                                                        \
    if (m_pScriptSite == nullptr)                                                                                                                                           \
    {                                                                                                                                                                       \
        return (scriptClosedHR);                                                                                                                                            \
    }                                                                                                                                                                       \
    auto scriptSite = m_pScriptSite;                                                                                                                                        \
    scriptSite->AddRef();                                                                                                                                                   \
                                                                                                                                                                            \
    Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();                                                                                               \
    if (!scriptContext->GetThreadContext()->IsStackAvailableNoThrow(Js::Constants::MinStackDefault))                                                                        \
    {                                                                                                                                                                       \
        return HRESULT_FROM_WIN32(ERROR_STACK_OVERFLOW);                                                                                                                    \
    }                                                                                                                                                                       \
    AddRef();                                                                                                                                                               \
    AutoCallerPointer callerPointer(scriptSite, nullptr);                                                                                                                   \
    HRESULT hr = S_OK;                                                                                                                                                      \
    BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT                                                                                                                    \
    BEGIN_JS_RUNTIME_CALLROOT_EX(scriptContext, false)                                                                                                                      \
    {                                                                                                                                                                       \
        /* Ignore stack walking exception because this might be called without root */                                                                                      \
        IGNORE_STACKWALK_EXCEPTION(scriptContext);

#define CUnknown_EndScriptEnter(reportError)                                                                                                                                \
    }                                                                                                                                                                       \
    END_JS_RUNTIME_CALL(scriptContext);                                                                                                                                     \
    TRANSLATE_EXCEPTION_TO_HRESULT_ENTRY(const Js::JavascriptException& err)                                                                                                \
    {                                                                                                                                                                       \
        Js::JavascriptExceptionObject * exceptionObject = err.GetAndClear();                                                                                                \
        Var errorObject = exceptionObject->GetThrownObject(nullptr);                                                                                                        \
        if (errorObject != nullptr && (Js::JavascriptError::Is(errorObject) || Js::JavascriptError::IsRemoteError(errorObject)))                                            \
        {                                                                                                                                                                   \
            hr = Js::JavascriptError::GetRuntimeErrorWithScriptEnter(Js::RecyclableObject::FromVar(errorObject), nullptr);                                                  \
                                                                                                                                                                            \
            if (Js::JavascriptErrorDebug::Is(errorObject))                                                                                                                  \
            {                                                                                                                                                               \
                Js::JavascriptErrorDebug::FromVar(errorObject)->SetErrorInfo();                                                                                             \
            }                                                                                                                                                               \
        }                                                                                                                                                                   \
        else                                                                                                                                                                \
        {                                                                                                                                                                   \
            hr = E_FAIL;                                                                                                                                                    \
        }                                                                                                                                                                   \
                                                                                                                                                                            \
        if (reportError)                                                                                                                                                    \
        {                                                                                                                                                                   \
            ScriptSite::HandleJavascriptException(exceptionObject, scriptContext, nullptr);                                                                                 \
        }                                                                                                                                                                   \
    }                                                                                                                                                                       \
    END_TRANSLATE_EXCEPTION_TO_HRESULT(hr)                                                                                                                                  \
    scriptSite->Release();                                                                                                                                                  \
    Release();

// Core Method defs
#define CUnknownMethod_Def(methodName, ...)                                                                                                                                 \
    __declspec(noinline)                                                                                                                                                    \
    virtual HRESULT __stdcall methodName(__VA_ARGS__)

#define CUnknownMethod_Prolog(className, methodName, ...)                                                                                                                   \
    /* ClassName::MethodName: Method that will do the actual work                                                           */                                              \
    /* info : a stub generated to actually do the work                                                                      */                                              \
    /* parameters: stackArgs - addressOfThisPtr of the original caller we use this to get parameter call stack address      */                                              \
    /*             stackBytesToCleanup - return the number of bytes to cleanup on stack                                     */                                              \
    HRESULT __stdcall ClassMethodName(className, methodName)(__VA_ARGS__)                                                                                                   \
    {                                                                                                                                                                       \
        CUnknown_BeginScriptEnter(E_ACCESSDENIED)

#define CUnknownMethod_Epilog()                                                                                                                                             \
        CUnknown_EndScriptEnter(/*reportError*/false)                                                                                                                       \
        return hr;                                                                                                                                                          \
    }


// Core method defs with no errors - used for Inspectable and Iunknown methods
#define CUnknownMethodNoError_Def(methodName, ...)                                                                                                                          \
    __declspec(noinline)                                                                                                                                                    \
    virtual HRESULT __stdcall methodName(__VA_ARGS__)

#define CUnknownMethodNoError_Prolog(className, methodName, ...)                                                                                                            \
    /* ClassName::MethodName: Method that will do the actual work                                                           */                                              \
    /* info : a stub generated to actually do the work                                                                      */                                              \
    /* parameters: stackArgs - addressOfThisPtr of the original caller we use this to get parameter call stack address      */                                              \
    /*             stackBytesToCleanup - return the number of bytes to cleanup on stack                                     */                                              \
    HRESULT __stdcall ClassMethodName(className, methodName)(__VA_ARGS__)                                                                                                   \
    {

#define CUnknownMethodNoError_Epilog()                                                                                                                                      \
    }


// Core Method returning ULONG defs
#define CUnknownMethod_ULONGReturn_Def(methodName)                                                                                                                          \
    __declspec(noinline)                                                                                                                                                    \
    virtual ULONG __stdcall methodName()

#define CUnknownMethod_ULONGReturn_Prolog(className, methodName)                                                                                                            \
    /* ClassName::MethodName: Method that will do the actual work                                                           */                                              \
    /* info : a stub generated to actually do the work                                                                      */                                              \
    /* parameters: stackArgs - addressOfThisPtr of the original caller we use this to get parameter call stack address      */                                              \
    /*             stackBytesToCleanup - return the number of bytes to cleanup on stack                                     */                                              \
    ULONG __stdcall ClassMethodName(className, methodName)()                                                                                                                \
    {                                                                                                                                                                       \
        ULONG uRetVal = 0;

#define CUnknownMethod_ULONGReturn_Epilog()                                                                                                                                 \
        return uRetVal;                                                                                                                                                     \
    }

// Own Impl Method with arguments
#define CUnknownMethodNoErrorImpl_Def(className, methodName, ...)                                                                                                           \
    __declspec(noinline)                                                                                                                                                    \
    static HRESULT STDMETHODCALLTYPE ClassMethodImplName(className, methodName)(IUnknown* This, __VA_ARGS__);                                                               \
    CUnknownMethodNoError_Def(methodName, __VA_ARGS__)

#define CUnknownMethodNoErrorImpl_Prolog(className, methodName, passArgs, ...)                                                                                              \
    /* ClassName::MethodName : Stub that calls the core method that can have the actual work done                            */                                             \
    /*                         We need to make sure that we convert the vtable that is passed in as this ptr into            */                                             \
    /*                         the actual object when calling the core method                                                */                                             \
    HRESULT STDMETHODCALLTYPE ClassMethodImplName(className, methodName)(IUnknown* This, __VA_ARGS__)                                                                       \
    {                                                                                                                                                                       \
        IfNullReturnError(This, E_POINTER);                                                                                                                                 \
        return GetClassFromIUnknown(className, This)->##methodName##passArgs##;                                                                                             \
    }                                                                                                                                                                       \
                                                                                                                                                                            \
    CUnknownMethodNoError_Prolog(className, methodName, __VA_ARGS__)

#define CUnknownMethodNoErrorImpl_Epilog()                                                                                                                                  \
    CUnknownMethodNoError_Epilog()


// Impl Method with arguments
#define CUnknownMethodImpl_Def(className, methodName, ...)                                                                                                                  \
    __declspec(noinline)                                                                                                                                                    \
    static HRESULT STDMETHODCALLTYPE ClassMethodImplName(className, methodName)(IUnknown* This, __VA_ARGS__);                                                               \
    CUnknownMethod_Def(methodName, __VA_ARGS__)

#define CUnknownMethodImpl_Prolog(className, methodName, passArgs, ...)                                                                                                     \
    /* ClassName::MethodName : Stub that calls the core method that can have the actual work done                            */                                             \
    /*                         We need to make sure that we convert the vtable that is passed in as this ptr into            */                                             \
    /*                         the actual object when calling the core method                                                */                                             \
    HRESULT STDMETHODCALLTYPE ClassMethodImplName(className, methodName)(IUnknown* This, __VA_ARGS__)                                                                       \
    {                                                                                                                                                                       \
        IfNullReturnError(This, E_POINTER);                                                                                                                                 \
        return GetClassFromIUnknown(className, This)->##methodName##passArgs##;                                                                                             \
    }                                                                                                                                                                       \
                                                                                                                                                                            \
    CUnknownMethod_Prolog(className, methodName, __VA_ARGS__)

#define CUnknownMethodImpl_Epilog()                                                                                                                                         \
    CUnknownMethod_Epilog()


// Impl Method with no arguments
#define CUnknownMethodImpl_NoArgs_Def(className, methodName)                                                                                                                \
    __declspec(noinline)                                                                                                                                                    \
    static HRESULT STDMETHODCALLTYPE ClassMethodImplName(className, methodName)(IUnknown* This);                                                                            \
    CUnknownMethod_Def(methodName)

#define CUnknownMethodImpl_NoArgs_Prolog(className, methodName)                                                                                                             \
    /* ClassName::MethodName : Stub that calls the core method that can have the actual work done                            */                                             \
    /*                         We need to make sure that we convert the vtable that is passed in as this ptr into            */                                             \
    /*                         the actual object when calling the core method                                                */                                             \
    HRESULT STDMETHODCALLTYPE ClassMethodImplName(className, methodName)(IUnknown* This)                                                                                    \
    {                                                                                                                                                                       \
        IfNullReturnError(This, E_POINTER);                                                                                                                                 \
        return GetClassFromIUnknown(className, This)->##methodName();                                                                                                       \
    }                                                                                                                                                                       \
                                                                                                                                                                            \
    CUnknownMethod_Prolog(className, methodName)

#define CUnknownMethodImpl_NoArgs_Epilog()                                                                                                                                  \
    CUnknownMethod_Epilog()


// Impl Method with no arguments returning ULONG
#define CUnknownMethodImpl_NoArgs_ULONGReturn_Def(className, methodName)                                                                                                    \
    __declspec(noinline)                                                                                                                                                    \
    static ULONG STDMETHODCALLTYPE ClassMethodImplName(className, methodName)(IUnknown* This);                                                                              \
    CUnknownMethod_ULONGReturn_Def(methodName)

#define CUnknownMethodImpl_NoArgs_ULONGReturn_Prolog(className, methodName)                                                                                                 \
    /* ClassName::MethodName : Stub that calls the core method that can have the actual work done                            */                                             \
    /*                         We need to make sure that we convert the vtable that is passed in as this ptr into            */                                             \
    /*                         the actual object when calling the core method                                                */                                             \
    ULONG STDMETHODCALLTYPE ClassMethodImplName(className, methodName)(IUnknown* This)                                                                                      \
    {                                                                                                                                                                       \
        if (nullptr == This)                                                                                                                                                   \
        {                                                                                                                                                                   \
            return (ULONG)-1;                                                                                                                                               \
        }                                                                                                                                                                   \
        return GetClassFromIUnknown(className, This)->##methodName();                                                                                                       \
    }                                                                                                                                                                       \
    CUnknownMethod_ULONGReturn_Prolog(className, methodName)

#define CUnknownMethodImpl_NoArgs_ULONGReturn_Epilog()                                                                                                                      \
    CUnknownMethod_ULONGReturn_Epilog()

#if _M_IX86
#define UpdateStackArgsWithThisPtr(paramsCount) stackArgs = paramsCount > 0 ? (stackArgs + 4) : nullptr;
#elif _M_AMD64 || defined(_M_ARM32_OR_ARM64)
#define UpdateStackArgsWithThisPtr(paramsCount) /* Not needed the function to read from reg/stack would take care of this */
#endif

// Core method of the Var ArgT Invokes
#define CUnknownMethodImpl_ArgTCore_Def(methodName)                                                                                                                         \
    CUnknownMethodNoError_Def(methodName, StackVarArg* stackArgs, DWORD *stackBytesToCleanup)

#define CUnknownMethodImpl_ArgTCore_Prolog(className, methodName, paramsCount, paramStackSize, localVariableDefines, scriptClosedHR)                                        \
    /* ClassName::Core_MethodName: Method that will do the actual work                                                      */                                              \
    /* info : a stub generated to actually do the work                                                                      */                                              \
    /* parameters: stackArgs - addressOfThisPtr of the original caller we use this to get parameter call stack address      */                                              \
    /*             stackBytesToCleanup - return the number of bytes to cleanup on stack                                     */                                              \
    CUnknownMethodNoError_Prolog(className, methodName, StackVarArg* stackArgs, DWORD *stackBytesToCleanup)                                                                 \
                                                                                                                                                                            \
        /* stackArgs contains address of this ptr */                                                                                                                        \
        localVariableDefines                                                                                                                                                \
                                                                                                                                                                            \
        /* This value is always used by the exit code so determine it right away. */                                                                                        \
        AssertMsg(paramStackSize + sizeof(void*) < DWORD_MAX, "Invalid metadata: Calculated stack size exceeds allowable size based on metadata constraints.");             \
        *stackBytesToCleanup = (DWORD)(paramStackSize + sizeof(void*));                                                                                                     \
                                                                                                                                                                            \
        CUnknown_BeginScriptEnter(scriptClosedHR)                                                                                                                           \
        {                                                                                                                                                                   \
            /* Stack arguments are present at offset of 4 bytes from the thisPtr we receives as part of stackArgs */                                                        \
            /* so set the stackArgs to correct address                                                            */                                                        \
            UpdateStackArgsWithThisPtr(paramsCount);                                                                                                                        \
            int stackBytesRead = 0;                                                                                                                                         \
            int paramIndex = 0;

#define CUnknownMethodImpl_ArgTCore_Epilog(reportError)                                                                                                                     \
        }                                                                                                                                                                   \
        CUnknown_EndScriptEnter(reportError)                                                                                                                                \
        return hr;                                                                                                                                                          \
    CUnknownMethodNoError_Epilog()


// Method with ArgT
#if _M_IX86
#define CUnknownMethodImpl_ArgT_Def(className, methodName)                                                                                                                  \
    __declspec(noinline)                                                                                                                                                    \
    static HRESULT STDMETHODCALLTYPE ClassMethodImplName(className, methodName)(IUnknown* This, ...);                                                                       \
    CUnknownMethodImpl_ArgTCore_Def(methodName)

// Stack looks like this
// | old old ebp    |   [old ebp]           <= old ebp
// | old local 1    |
// | old local 2    |
// | old local 3    |
// | old local n    |
// |----------------|
// | fn param n     |   [ebp + 4n + 12]
// | fn param n-1   |   [ebp + 4(n-1) + 12]
// |    ...         |   ...
// |    ...         |   ...
// |    ...         |   ...
// | fn param 2     |   [ebp + 16]
// | fn param 1     |   [ebp + 12]          <= Function params start
// | this pointer   |   [ebp + 8]
// | retun address  |   [ebp + 4]
// | old ebp        |   [ebp]               <= ebp
// | local 1        |
// | local 2        |
// | local 3        |
// | local 4        |                       <= esp
#define CUnknownMethodImpl_ArgT_Prolog(className, methodName, paramsCount, paramStackSize, localVariableDefines, scriptClosedHR)                                            \
                                                                                                                                                                            \
    /* ClassName::MethodName : Stub that calls the core method that can have the actual work done                            */                                             \
    /*                         Since this is declared as naked, we need to manipulate stack, thisPtr and parameters ourselves*/                                             \
    /*                         Also we need to make sure the core function is called using the volatile registers only.      */                                             \
    __declspec(naked)                                                                                                                                                       \
    HRESULT STDMETHODCALLTYPE ClassMethodImplName(className, methodName)(IUnknown* This, ...)                                                                               \
    {                                                                                                                                                                       \
        /* Prolog */                                                                                                                                                        \
        /* Manually establish a stack frame so that references to locals herein will work */                                                                                \
        __asm                                                                                                                                                               \
        {                                                                                                                                                                   \
            /* link the stack frame */                                                                                                                                      \
            __asm push ebp                                                                                                                                                  \
            __asm mov ebp, esp                                                                                                                                              \
                                                                                                                                                                            \
            /* Reserve space for getting the stackBytesToCleanup */                                                                                                         \
            __asm sub esp, 4                                                                                                                                                \
                                                                                                                                                                            \
            /* | fn param 2     |  <= [ebp + 16]  */                                                                                                                        \
            /* | fn param 1     |  <= [ebp + 12]  */                                                                                                                        \
            /* | this pointer   |  <= [ebp + 8]   */                                                                                                                        \
            /* | retun address  |  <= [ebp + 4]   */                                                                                                                        \
            /* | old ebp        |  <= [ebp]       */                                                                                                                        \
                                                                                                                                                                            \
            /* Establish stack for calling the core method that does the work*/                                                                                             \
                                                                                                                                                                            \
            /* Push the address of reserved space for stackBytesToCleanup */                                                                                                \
            __asm mov eax, esp                                                                                                                                              \
            __asm push eax                                                                                                                                                  \
                                                                                                                                                                            \
            /* the this ptr is at ebp + 8 so that is the address of stack that we want to send */                                                                           \
            __asm mov eax, ebp                                                                                                                                              \
            __asm add eax, 8                                                                                                                                                \
            __asm push eax                                                                                                                                                  \
                                                                                                                                                                            \
            /* push the this ptr from the vtable */                                                                                                                         \
            __asm mov ecx, dword ptr [eax]                                                                                                                                  \
            __asm sub ecx, 4                                                                                                                                                \
            __asm push ecx                                                                                                                                                  \
                                                                                                                                                                            \
            /* Call the core function - passing the stack address if the method takes the arguments */                                                                      \
            __asm call methodName                                                                                                                                           \
                                                                                                                                                                            \
            /* Epilog */                                                                                                                                                    \
            /* Replace function epilogue since we need to clean up the parameters from the stack.   */                                                                      \
            /* Compiler won't do it for us since Invoke method is declared without real parameters. */                                                                      \
            /* Deal with the return values, and return to the caller....                            */                                                                      \
                                                                                                                                                                            \
            /* Return value (HRESULT) is already in EAX */                                                                                                                  \
                                                                                                                                                                            \
            /* store the amount of bytes we need to clean up in ECX since locals    */                                                                                      \
            /* will not be accessible once we restore the EBP.                      */                                                                                      \
            /* It is in top of the stack as we saved space for it                   */                                                                                      \
            __asm pop ecx                                                                                                                                                   \
                                                                                                                                                                            \
            /* Emulate RET with stack cleanup */                                                                                                                            \
            /* de-alloc our own stack - local variables */                                                                                                                  \
                                                                                                                                                                            \
            /* unlink the stack frame - Restore old esp and pop old ebp */                                                                                                  \
            __asm pop ebp                                                                                                                                                   \
                                                                                                                                                                            \
            /* Get the return address */                                                                                                                                    \
            __asm pop edx                                                                                                                                                   \
                                                                                                                                                                            \
            /* Cleanup stack parameters (ecx has the stackBytesToCleanup value) */                                                                                          \
            __asm add esp, ecx                                                                                                                                              \
                                                                                                                                                                            \
            /* Stack now would look like this   */                                                                                                                          \
            /* Stack looks like this            */                                                                                                                          \
            /* | old old ebp    |   [ebp]       */                                                                                                                          \
            /* | old local 1    |               */                                                                                                                          \
            /* | old local 2    |               */                                                                                                                          \
            /* | old local 3    |               */                                                                                                                          \
            /* | old local n    |   [esp]       */                                                                                                                          \
                                                                                                                                                                            \
            /* return to the old function caller - eip we popped earlier    */                                                                                              \
            __asm push edx                                                                                                                                                  \
            __asm ret /* Return to return address at the caller function  */                                                                                                \
                                                                                                                                                                            \
        }                                                                                                                                                                   \
    }                                                                                                                                                                       \
                                                                                                                                                                            \
    CUnknownMethodImpl_ArgTCore_Prolog(className, methodName, paramsCount, paramStackSize, localVariableDefines, scriptClosedHR)

#elif _M_AMD64 || defined(_M_ARM32_OR_ARM64)
#if defined(_M_ARM64)
    const int REG_PARAMS = 8;
#else
    // Per AMD64/ARM calling convention, first 4 function args are passed in registers.
    // The fist arg is 'this' pointer so we substract 1.
const int REG_PARAMS = 4;
#endif

#define Declare_Extern_ClassMethodImpl(className, methodName)                                                                                                               \
    extern "C" HRESULT ClassMethod_VarArgT_ImplName(className, methodName)(IUnknown* This, ...);                                                                            \

#define CUnknownMethodImpl_ArgT_Def(className, methodName)                                                                                                                  \
    CUnknownMethodImpl_ArgTCore_Def(methodName)

#define CUnknownMethodImpl_ArgT_Prolog(className, methodName, paramsCount, paramStackSize, localVariableDefines, scriptClosedHR)                                            \
    CUnknownMethodImpl_ArgTCore_Prolog(className, methodName, paramsCount, paramStackSize, localVariableDefines, scriptClosedHR)                                            \

#define CallIndirectImpl_CUnknownMethod(className, methodName)                                                                                                              \
    if (GetMethodId(className, methodName) == iMethod)                                                                                                                      \
    {                                                                                                                                                                       \
        return ((className *)this)->##methodName((StackVarArg *)pvArgs, pcbArgs);                                                                                           \
    }
#endif

#define CUnknownMethodImpl_ArgT_Epilog()                                                                                                                                    \
    CUnknownMethodImpl_ArgTCore_Epilog(/*reportError*/false)

#define CUnknownMethodImpl_ArgT_ReportError_Epilog()                                                                                                                        \
    CUnknownMethodImpl_ArgTCore_Epilog(/*reportError*/true)


// Parameter reading
#if _M_IX86
#define GetNextInParameterAddress(fDouble, fFloat)                                                                                                                          \
    (stackArgs + stackBytesRead)

#define GetNextInParamAddressFromIndexAndOffset(nextIndex, offSet, fDouble, fFloat)                                                                                         \
    (stackArgs + stackBytesRead + ##offSet)

#define GetNextInParameterAddressFromParamType(paramType)                                                                                                                   \
    (stackArgs + stackBytesRead)

#define GetInParameterAddressFromParamType(paramIndex, stackOffset)                                                                                                         \
    (stackArgs + ##stackOffset)

#define GetOutParameterAddressFromParamType(paramIndex, stackOffset)                                                                                                        \
    (*(byte**)(stackArgs + ##stackOffset))

#define GetInParameterAddressFromParamIndexAndOffset(paramIndex, stackOffset, fDouble, fFloat)                                                                              \
    (stackArgs + ##stackOffset)

#define GetOutParameterAddressFromParamIndexAndOffset(paramIndex, stackOffset, fDouble, fFloat)                                                                             \
    (*(byte**)(stackArgs + ##stackOffset))

#define GetNextOutParameterAddress(fDouble, fFloat)                                                                                                                         \
    (*(byte**)(stackArgs + stackBytesRead))

#define UpdateParameterRead(paramSize)                                                                                                                                      \
    paramIndex++;                                                                                                                                                           \
    stackBytesRead += ##paramSize;

#define UpdateParameterReadByModelParameter(modelParameter) \
    AssertMsg(parameter->GetParameterOnStackCount() < 65536 && parameter->GetSizeOnStack() < INT_MAX, "Invalid metadata: Parameter projection exceeds allowable sizes by metadata."); \
    paramIndex += (int)parameter->GetParameterOnStackCount(); \
    stackBytesRead += (int)parameter->GetSizeOnStack();

#define GetArrayLengthParamLocation() \
    lengthParamLocation = stackArgs; \
    signature->GetParameters()->allParameters->Cast<RtABIPARAMETER>()->IterateFirstN(((AbiArrayParameterWithLengthAttribute *)parameter)->lengthIsParameter, [&](RtABIPARAMETER parameter) { \
        AssertMsg(parameter->GetSizeOnStack() < UINT_MAX, "Invalid metadata: Parameter projection exceeds allowable sizes by metadata."); \
        lengthParamLocation += (uint)parameter->GetSizeOnStack(); \
    });

// These are unused for this architecture and thus expand into nothing.
#define DefineAndInitParameterLocations(parameterCount)
#define DefineParameterLocationAsGetArrayItem(parameterLocations, parameterLocationIndex)
#define DefineCallingConventionLocals()
#define GetNextParameterLocation_(sizeOnStack, isFloatingPoint, is64BitAlignRequired, parameterLocation, elementType)

#elif _M_AMD64
#define GetNextInParameterAddress(fDouble, fFloat)                                                                                                                          \
    (StackVarArgReader::Read(paramIndex, stackArgs, ##fDouble, ##fFloat))

#define GetNextInParamAddressFromIndexAndOffset(nextIndex, offSet, fDouble, fFloat)                                                                                         \
    (StackVarArgReader::Read(paramIndex + ##nextIndex, stackArgs, ##fDouble, ##fFloat))

#define GetNextInParameterAddressFromParamType(paramType)                                                                                                                   \
    (StackVarArgReader::Read(paramIndex, ##paramType, stackArgs))

#define GetInParameterAddressFromParamType(paramIndex, stackOffset)                                                                                                         \
    (StackVarArgReader::Read(##paramIndex, paramType, stackArgs))

#define GetOutParameterAddressFromParamType(paramIndex, stackOffset)                                                                                                        \
    (*(byte**)(StackVarArgReader::Read(##paramIndex, stackArgs, false, false)))

#define GetInParameterAddressFromParamIndexAndOffset(paramIndex, stackOffset, fDouble, fFloat)                                                                              \
    (StackVarArgReader::Read(##paramIndex, stackArgs, ##fDouble, ##fFloat))

#define GetOutParameterAddressFromParamIndexAndOffset(paramIndex, stackOffset, fDouble, fFloat)                                                                             \
    (*(byte**)(StackVarArgReader::Read(##paramIndex, stackArgs, ##fDouble, ##fFloat)))

#define GetNextOutParameterAddress(fDouble, fFloat)                                                                                                                         \
    (*(byte**)(StackVarArgReader::Read(paramIndex, stackArgs, ##fDouble, ##fFloat)))

#define UpdateParameterRead(paramSize)                                                                                                                                      \
    paramIndex++;

#define UpdateParameterReadByModelParameter(parameter) \
    AssertMsg(parameter->GetParameterOnStackCount() < 65536 && parameter->GetSizeOnStack() < INT_MAX, "Invalid metadata: Parameter projection exceeds allowable sizes by metadata."); \
    paramIndex += (int)parameter->GetParameterOnStackCount(); \
    stackBytesRead += (int)parameter->GetSizeOnStack();

#define GetArrayLengthParamLocation() \
    size_t lengthParamIndex = 0; \
    signature->GetParameters()->allParameters->Cast<RtABIPARAMETER>()->IterateFirstN(((AbiArrayParameterWithLengthAttribute *)parameter)->lengthIsParameter, [&](RtABIPARAMETER parameter) { \
        AssertMsg(parameter->GetSizeOnStack() < UINT_MAX, "Invalid metadata: Parameter projection exceeds allowable sizes by metadata."); \
        lengthParamIndex += parameter->GetParameterOnStackCount(); \
    }); \
    lengthParamLocation = StackVarArgReader::Read((int)lengthParamIndex, ConcreteType::From(lengthParam->type), stackArgs);

// These are unused for this architecture and thus expand into nothing.
#define DefineAndInitParameterLocations(parameterCount)
#define DefineParameterLocationAsGetArrayItem(parameterLocations, parameterLocationIndex)
#define DefineCallingConventionLocals()
#define GetNextParameterLocation_(sizeOnStack, isFloatingPoint, is64BitAlignRequired, parameterLocation, elementType)

#elif defined(_M_ARM)

// TODO: Evanesco/Projection: refactor all macros to use "next parameter" rather than offset.
//       Although currently there seem to be no broken scenarios, but there could be in future.
#define GetInParameterAddress(parameterLocation)                                                                                                                        \
    (StackVarArgReader::Read(parameterLocation, stackArgs))

#define GetInParameterAddressFromIndexAndOffset(parameterLocation, offset)                                                                                              \
    (GetInParameterAddress(parameterLocation) + offset)

#define GetOutParameterAddress(parameterLocation)                                                                                                                       \
    (*(byte**)GetInParameterAddress(parameterLocation))

#define GetOutParameterAddressFromIndexAndOffset(parameterLocation, offset)                                                                                             \
    (*(byte**)(GetInParameterAddress(parameterLocation) + offset))

#define GetNextInParameterAddress(fDouble, fFloat)                                                                                                                      \
        GetInParameterAddress(parameterLocation)

#define GetNextInParamAddressFromIndexAndOffset(nextIndex, offset, fDouble, fFloat)                                                                                     \
        GetInParameterAddressFromIndexAndOffset(parameterLocation, offset)

#define GetNextInParameterAddressFromParamType(paramType)                                                                                                               \
        GetInParameterAddress(parameterLocation)

#define GetOutParameterAddressFromParamType(paramIndex, offset)                                                                                                         \
        GetOutParameterAddressFromIndexAndOffset(parameterLocation, offset)

#define GetInParameterAddressFromParamIndexAndOffset(paramIndex, offset, fDouble, fFloat)                                                                               \
        GetInParameterAddressFromIndexAndOffset(parameterLocation, offset)

#define GetOutParameterAddressFromParamIndexAndOffset(paramIndex, offset, fDouble, fFloat)                                                                              \
        GetOutParameterAddressFromIndexAndOffset(parameterLocation, offset)

#define GetNextOutParameterAddress(fDouble, fFloat)                                                                                                                     \
        GetOutParameterAddress(parameterLocation)

#define UpdateParameterRead(paramSize)                                                                                                                                  \
    paramIndex++;

#define GetArrayLengthParamLocation() \
    lengthParamLocation = StackVarArgReader::Read(&parameterLocations[((AbiArrayParameterWithLengthAttribute *)parameter)->lengthIsParameter], stackArgs);

// Not used on this architechture.
// Note: it's important that stackBytesRead is always 0 --
//       as places using GetNextXXX macros often pass stackBytesRead + sizeeof(LPVOID) as offset,
//       otherwise we will be getting wrong offsets.
#define UpdateParameterReadByModelParameter(modelParameter)

#define InitParameterLocations(parameterLocations)                                                                                                                      \
    ProjectionModel::CallingConventionHelper callingConvention(1);                                                                                                      \
    this->signature->GetParameters()->allParameters->Cast<RtABIPARAMETER>()->IterateN([&](int parameterLocationIndex, RtABIPARAMETER parameter) {                       \
        callingConvention.GetNextParameterLocation(parameter, &parameterLocations[parameterLocationIndex]);                                                             \
    });

#define DefineAndInitParameterLocations(parameterCount)                                                                                                                 \
    AutoArrayPtr<ProjectionModel::ParameterLocation> parameterLocations(HeapNewArrayZ(ParameterLocation, parameterCount), parameterCount);                              \
    InitParameterLocations(parameterLocations)

#define DefineParameterLocationAsGetArrayItem(parameterLocations, parameterLocationIndex)                                                                               \
    ParameterLocation* parameterLocation = &parameterLocations[parameterLocationIndex];

#define DefineCallingConventionLocals()                                                                                                                                 \
    ProjectionModel::CallingConventionHelper callingConvention(1);                                                                                                      \
    ProjectionModel::ParameterLocation parameterLocationValue;                                                                                                          \
    ProjectionModel::ParameterLocation* parameterLocation = &parameterLocationValue;

// Note: '_' is added to the end of macro name to avoid conflict with CallingConventionHelper::GetNextParameterLocation.
#define GetNextParameterLocation_(sizeOnStack, isFloatingPoint, is64BitAlignRequired, parameterLocation, elementType)                                                    \
    callingConvention.GetNextParameterLocation(sizeOnStack, isFloatingPoint, is64BitAlignRequired, parameterLocation)

#elif defined(_M_ARM64)

// TODO: Evanesco/Projection: refactor all macros to use "next parameter" rather than offset.
//       Although currently there seem to be no broken scenarios, but there could be in future.
#define GetInParameterAddress(parameterLocation)                                                                                                                        \
    (StackVarArgReader::Read(parameterLocation, stackArgs))

#define GetInParameterAddressFromIndexAndOffset(parameterLocation, offset)                                                                                              \
    (GetInParameterAddress(parameterLocation) + offset)

#define GetOutParameterAddress(parameterLocation)                                                                                                                       \
    (*(byte**)GetInParameterAddress(parameterLocation))

#define GetOutParameterAddressFromIndexAndOffset(parameterLocation, offset)                                                                                             \
    (*(byte**)(GetInParameterAddress(parameterLocation) + offset))

#define GetNextInParameterAddress(fDouble, fFloat)                                                                                                                      \
        GetInParameterAddress(parameterLocation)

#define GetNextInParamAddressFromIndexAndOffset(nextIndex, offset, fDouble, fFloat)                                                                                     \
        GetInParameterAddressFromIndexAndOffset(parameterLocation, offset)

#define GetNextInParameterAddressFromParamType(paramType)                                                                                                               \
        GetInParameterAddress(parameterLocation)

#define GetOutParameterAddressFromParamType(paramIndex, offset)                                                                                                         \
        GetOutParameterAddressFromIndexAndOffset(parameterLocation, offset)

#define GetInParameterAddressFromParamIndexAndOffset(paramIndex, offset, fDouble, fFloat)                                                                               \
        GetInParameterAddressFromIndexAndOffset(parameterLocation, offset)

#define GetOutParameterAddressFromParamIndexAndOffset(paramIndex, offset, fDouble, fFloat)                                                                              \
        GetOutParameterAddressFromIndexAndOffset(parameterLocation, offset)

#define GetNextOutParameterAddress(fDouble, fFloat)                                                                                                                     \
        GetOutParameterAddress(parameterLocation)

#define UpdateParameterRead(paramSize)                                                                                                                                  \
    paramIndex++;

#define GetArrayLengthParamLocation() \
    lengthParamLocation = StackVarArgReader::Read(&parameterLocations[((AbiArrayParameterWithLengthAttribute *)parameter)->lengthIsParameter], stackArgs);

// Not used on this architechture.
// Note: it's important that stackBytesRead is always 0 --
//       as places using GetNextXXX macros often pass stackBytesRead + sizeeof(LPVOID) as offset,
//       otherwise we will be getting wrong offsets.
#define UpdateParameterReadByModelParameter(modelParameter)

#define InitParameterLocations(parameterLocations)                                                                                                                      \
    ProjectionModel::CallingConventionHelper callingConvention(1);                                                                                                      \
    this->signature->GetParameters()->allParameters->Cast<RtABIPARAMETER>()->IterateN([&](int parameterLocationIndex, RtABIPARAMETER parameter) {                       \
        callingConvention.GetNextParameterLocation(parameter, &parameterLocations[parameterLocationIndex]);                                                             \
    });

#define DefineAndInitParameterLocations(parameterCount)                                                                                                                 \
    AutoArrayPtr<ProjectionModel::ParameterLocation> parameterLocations(HeapNewArrayZ(ParameterLocation, parameterCount), parameterCount);                              \
    InitParameterLocations(parameterLocations)

#define DefineParameterLocationAsGetArrayItem(parameterLocations, parameterLocationIndex)                                                                               \
    ParameterLocation* parameterLocation = &parameterLocations[parameterLocationIndex];

#define DefineCallingConventionLocals()                                                                                                                                 \
    ProjectionModel::CallingConventionHelper callingConvention(1);                                                                                                      \
    ProjectionModel::ParameterLocation parameterLocationValue;                                                                                                          \
    ProjectionModel::ParameterLocation* parameterLocation = &parameterLocationValue;

// Note: '_' is added to the end of macro name to avoid conflict with CallingConventionHelper::GetNextParameterLocation.
#define GetNextParameterLocation_(sizeOnStack, isFloatingPoint, is64BitAlignRequired, parameterLocation, elementType)                                                   \
    AssertMsg(sizeOnStack < INT_MAX, "ARM64: Element size on stack should not exceed available stack space.");                                             \
    callingConvention.GetNextParameterLocation((int)sizeOnStack, isFloatingPoint, parameterLocation, elementType)

#endif

#if _M_IX86
typedef byte StackVarArg;
#elif _M_AMD64
//  Defines structure for passing registry values on AMD64
typedef struct StackVarArg
{
    double * pFloatRegisters;
    __int64 * pStack;
} StackVarArg;

// Amd64 register read
class StackVarArgReader
{
public:
    static byte *Read(int paramIndex, RtCONCRETETYPE paramType, StackVarArg *stackArgs);
    static byte *Read(int paramIndex, StackVarArg *stackArgs, bool fDouble = false, bool fFloat = false);
};
#elif defined(_M_ARM)
typedef ProjectionModel::ApcsCallLayout StackVarArg;

// ARM register read
class StackVarArgReader
{
public:
    static byte *Read(const ProjectionModel::ParameterLocation* loc, StackVarArg* callLayout);
};
#elif defined(_M_ARM64)
typedef ProjectionModel::ApcsCallLayout StackVarArg;

// ARM64 register read
class StackVarArgReader
{
public:
    static byte *Read(const ProjectionModel::ParameterLocation* loc, StackVarArg* callLayout);
};
#endif
}
