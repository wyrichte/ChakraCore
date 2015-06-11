/*++

Module Name:

    jsthunk_private.h

Abstract:

    JScript thunks without writable executable memory, based closely on ATL thunks.

Author:

    Jay Krell (jaykrell) 22-Aug-2013

--*/

/*
namespace Js
{
    enum CallFlags : unsigned;

    struct CallInfo
    {
        unsigned  Count : 24;
        CallFlags Flags : 8;
#if _WIN64
        unsigned unused : 32;
#endif
    };

    typedef void* Var;

    class FinalizableObject;
        class FunctionInfo;
            class FunctionProxy; // contains ScriptContext*  m_scriptContext
                class ParseableFunctionInfo;
                    class FunctionBody;

    class ScriptContext; // jscriptlegacy ScriptContext contains profiling thunk or non-profiling thunk

    class FinalizableObject;
        class RecyclableObject;
            class DynamicObject;
                class JavaScriptfunction; // contains FunctionInfo * functionInfo
                    class ScriptFunction;
    typedef Var (__cdecl *JavascriptMethod)(RecyclableObject*, CallInfo, ...); // first parameter is a ScriptFunction
    typedef Var (__cdecl *AsmJsMethod)(RecyclableObject*, ...); // AsmJs; first parameter is a ScriptFunction, CallInfo is omitted
}

struct AsmJsCallStackLayout
{
    Js::JavascriptFunction * functionObject;
    Js::Var args[0];
};

struct JavascriptCallStackLayout
{
    Js::JavascriptFunction * functionObject;
    Js::CallInfo callInfo;
    Js::Var args[0];
};

A first level function takes a ScriptFunction, possibly followed by a CallInfo, followed by anything.
These functions typically delegate, i.e. by taking the address of the first parameter.

A JsThunk is a first level function and delegates.
*/

#pragma warning(push)
#pragma warning(disable:4201) // nonstandard extension
#include <ntdef.h>
#pragma warning(pop)

typedef void* (__cdecl * JsFunc0_t)(void* functionObject, ...); // first level
typedef void* (__cdecl * JsFunc1_t)(void** functionObject);     // second level

extern "C"
{
#if JSCRIPT_THUNK_STACK_INSTRUMENTATION
void JScriptThunkInstrumentation(void);
decltype(&JScriptThunkInstrumentation) pJScriptThunkInstrumentation;
#else
#define pJScriptThunkInstrumentation() // nothing
#endif

// Ideally the image's size is close to but less than a multiple of 64K.
#define COUNT 1024 // fairly arbitrary number -- size of chunk allocation

#define THUNK16(base) \
    THUNK(base##0) THUNK(base##1) THUNK(base##2) THUNK(base##3) \
    THUNK(base##4) THUNK(base##5) THUNK(base##6) THUNK(base##7) \
    THUNK(base##8) THUNK(base##9) THUNK(base##A) THUNK(base##B) \
    THUNK(base##C) THUNK(base##D) THUNK(base##E) THUNK(base##F) \

#define THUNKS256(base) \
    THUNK16(base##0) THUNK16(base##1) THUNK16(base##2) THUNK16(base##3) \
    THUNK16(base##4) THUNK16(base##5) THUNK16(base##6) THUNK16(base##7) \
    THUNK16(base##8) THUNK16(base##9) THUNK16(base##A) THUNK16(base##B) \
    THUNK16(base##C) THUNK16(base##D) THUNK16(base##E) THUNK16(base##F) \

#define THUNKS \
    THUNKS256(0x0) THUNKS256(0x1) THUNKS256(0x2) THUNKS256(0x3) \

#define NAME(n) JsThunk_##n
#define DECL(n) void*/*Js::Var*/ __cdecl NAME(n)(void* /* Js::ScriptFunction* */ function, ...)

// Relocations might not get applied in the repeat mappings.
// So the thunks must be position independent.
// AMD64 and ARM64 are naturally position independent.
// x86 and ARM are not position independent by virtue of control flow guard,
// because they reference a global.
// Handle that by turning off the checks and reimplementing
// them with position independent code.
// NOTE: This manual check would be correct if needed for AMD64
// but is incorrect for ARM64 due to the custom calling
// convention of __guard_check_icall_fptr (parameter in x15 instead of x0).
//
// This version further would be position dependent, on x86 and ARM, due to
// ThunkData. That can be eliminated by retreiving the InterpreterThunk
// out of JScript data structures. Instead we currently use assembly to achieve
// position independence. Getting the data from JScript data structures
// has the following downside: it makes the thunk .dll specific to
// jscript9 vs. chakra, and it is difficult to expose the data layout here.

typedef void (__fastcall * _guard_check_icall_t)(void*);

#if defined(_AMD64_) || defined(_ARM64_) || !defined(_CONTROL_FLOW_GUARD)

#define GUARD_CHECK(x) // nothing

#else

_guard_check_icall_t* __cdecl get_guard_check_icall_fptr(void); // position independent assembly
#define GUARD_CHECK(x) ((*get_guard_check_icall_fptr())(x)) // position independent check

#undef THUNK
#define THUNK(n) __pragma(guard(ignore, JsThunk_##n))
THUNKS

#endif

#define IMPL(n) DECL(n) \
    {                                                           \
        pJScriptThunkInstrumentation();                         \
        auto const data = &GetJsThunkData()[n].u.in_use;        \
        /* __debugbreak(); */                                   \
        auto const InterpreterThunk = data->InterpreterThunk;   \
        GUARD_CHECK(InterpreterThunk);                          \
        return InterpreterThunk(&function);                     \
    }

#undef THUNK
#define THUNK(n) DECL(n);
THUNKS

struct JsThunkData_t
{
    JsFunc0_t Thunk;
    union {
        struct {
            SINGLE_LIST_ENTRY Link;
        } free; // also used for deferred free
        struct {
            JsFunc1_t InterpreterThunk;
        } in_use;
    } u;
};

#if defined(_AMD64_) || defined(_ARM64_) // naturally position-independent
#define GetJsThunkData() JsThunkData
#else // written in assembly to avoid relocs

// GetJsThunkData is offset by 128 so that the codegen is
// not too different for "small" vs. "large" offsets -- they are all "large".
// 128 occurs in jsthunkasm.asm. "Too different" is that the functions
// would vary in size.

char* __cdecl GetJsThunkData(void);
#define GetJsThunkData() ((JsThunkData_t*)((GetJsThunkData)() + 128))

#endif

extern JsThunkData_t JsThunkData[];

} // extern C

