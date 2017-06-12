/*++

Module Name:

    jsthunk.cpp

Abstract:

    JScript thunks without writable executable memory, based closely on ATL thunks.

Author:

    Jay Krell (jaykrell) 22-Aug-2013

--*/

/*
Theory of operation:

    We have n almost identical functions.
    We have an array of n data.
    Each function references its corresponding data.
    When n is exhausted, map the .dll again to get another n.
        Not via LoadLibrary -- that will single instance.
        But CreateFile + CreateFileMapping(SEC_IMAGE) + MapViewOfFile.
    The functions are position independent.
    On AMD64 and ARM64 that is automatic.
    On other architectures it takes a little assembly -- to
        reference the global data, including the control flow guard function pointer.

    The point of these thunks is to establish unique return addresses
    for interpreted functions, so they can be identified in callstacks,
    instead of just seeing the interpreter call itself.

    Historicaly, besides VirtualAlloc(read/write/execute), what was done
    was page-long functions were produced, with a "switch" with
    small cases. Each case had a call, to establish a unique return
    address. That has great size/density advantages, but is very processor-specific.

    The solution here is fairly portable -- almost no assembly required.

    The thunks almost do not need associated data.

    The data however lets us:
      Store the interpreter thunk separate from the jscript data structurs.
        Therefore having one thunk .dll for jscript9.dll and chakra.dll instead of two.

      Have room to store an intrusive linked list link so deferred cleanup does not need to
        allocate memory and will not leak under low memory.

    As well, assembly is required for position independence in the face of CFG.

    This is based on enduser\vc_binaries\src\atlthunk2.
*/

#include <nt.h>
#include <ntrtl.h>
#include <ntosdef.h>
#if defined(_AMD64_)
#include <amd64\ntrtlamd64.h>
#elif defined(_ARM_)
#include <arm\ntrtlarm.h>
#elif defined(_ARM64_)
#include <arm64\ntrtlarm64.h>
#endif
#include <nturtl.h>
#include <windows.h>
#include <intsafe.h>
#include <set>
#include "util.h"
#if JSCRIPT_THUNK_STACK_INSTRUMENTATION
#include <atlstr.h>
#include <cstringt.h>
#endif

#include "jsthunk.h"
#include "jsthunk_private.h"

extern "C"
{
extern PVOID __guard_check_icall_fptr;
extern PVOID __guard_dispatch_icall_fptr;
extern const IMAGE_DOS_HEADER __ImageBase; // linker provided
}

#undef THUNK
#define THUNK(n) { NAME(n), { &JsThunkData[n + 1].u.free.Link } },

// +1: We would prefer the Link in the last element be NULL.
// However that is not easy to achieve (without a dynamic initializer). Therefore we allocate one extra
// thunk, have it be zero-initialized, and check for thunkData->Thunk != NULL before
// accepting a thunkData. This is a trade-off, vs. having code to do initialization.
#if defined(_ARM_) || defined(_X86_)
EXTERN_C // referenced from assembly
#endif
JsThunkData_t JsThunkData[COUNT + 1] = { THUNKS };

SRWLOCK JsThunk_Lock;
SINGLE_LIST_ENTRY JsThunk_Pool = {&JsThunkData[0].u.free.Link};
Handle_t JsThunk_FileMapping;
#ifndef _X86_
UINT32 JsThunk_FunctionTableOffset;
UINT32 JsThunk_FunctionTableEntryCount;
#endif
UINT32 JsThunk_SizeOfImage;
UINT32 JsThunk_Size;
// std::set<MappedViewOfFile_t<BYTE>> JsThunk_Views; // to cleanup (and the function tables)
std::set<PBYTE> JsThunk_Views;

void*
__stdcall
JsThunk_DataToCode(
    void* ThunkData
    )
{
    // For thunks in the first mapping, we can subtract from JsThunkData to get an index
    // and then switch on that to get a function, but that does not work for the repeat mappings,
    // because we do not know the base to subtract, without a binary search.
    // Also, since the functions are in C++, we cannot depend on them being a fixed size.
    // Therefore, store the thunk in the data.

    return ThunkData ? ((JsThunkData_t*)ThunkData)->Thunk : NULL;
}

bool
JsThunk_ProcessAttach(
    void
    )
// JsThunk_Lock must be locked exclusive.
{
    DWORD length = 64 * sizeof(WCHAR);
    HMODULE module = { 0 };

    // GetModuleFileName, CreateFile, CreateFileMapping early to narrow race
    // condition, where .dll could change between the time is loaded and the
    // time we map the view for additional thunks.
    // GET_MODULE_HANDLE_EX_FLAG_PIN since we do not cleanup.

    auto const base = (PBYTE)&__ImageBase;

    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN, (PCWSTR)base, &module))
        return false;

    while (1)
    {
        ProcessHeapPtr_t<WCHAR> path = (PWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, length);
        if (path == NULL)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return false;
        }

        if (!GetModuleFileNameW(module, path, length / sizeof(WCHAR)))
            return false;

        // GetModuleFileNameW return values vary per OS version and are confusing;
        // instead of depending on it, make sure the path fits in the buffer
        // by seeing if 0 near the end remains.

        if (path[(length / sizeof(WCHAR)) - 2] != 0)
        {
            if (DWordMult(length, 2, &length) != S_OK)
            {
                SetLastError(ERROR_ARITHMETIC_OVERFLOW);
                return false;
            }
            continue;
        }

        Handle_t fileHandle = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (fileHandle == INVALID_HANDLE_VALUE)
            return false;

        Handle_t fileMapping = CreateFileMappingW(fileHandle, NULL, PAGE_READONLY | SEC_IMAGE, 0, 0, 0);
        if (fileMapping == NULL)
            return false;

        MappedViewOfFile_t<BYTE> view = (PBYTE)MapViewOfFile(fileMapping, FILE_MAP_READ, 0, 0, 0);
        if (view == NULL)
            return false;

        // Make sure the file has not changed in the short time between
        // when it was first mapped and getting here, so that we can
        // safely remap it and apply offsets from the original to the new open.
        //
        // If this fails, the .dll load will fail, and a retry will likely succeed.
        //
        // Ideally we would reuse the original section, but that is not possible.
        // The handle has been closed.

        if (NtAreMappedFilesTheSame("", view))
            return false;

        // NOTE: We could add this mapping to our pool, or we could throw it out for now.
        // There are advantages and disadvantages either way.
        // Advantage: We paid for the mapping, pay a little more and hold on to it for later.
        // Disadvantage: Do not use up extra address space until/unless needed.
        // We throw it away, in the destructor.

        // We have a few requirements that are unusual and fragile.
        // It is tempting to use assembly to lock them, but for now we do not.
        // We need to know the sizes of the functions, at least the part after the call.
        // We need the functions to not have tail calls.
        //  Same thing: we need them to have pdata.
        // Even though they are identical, we require the linker to not combine them.
        // We need the functions to be position-independent.
        //   That is not verified, but on AMD64 is natural anyway and on other
        //   architectures should fall out of their lack of global references.
        //
        // Much of this is verified here.
        // It is fragile enough, e.g. compiler/linker switch-dependent, that it seems
        // worth checking at runtime in retail builds.

        // Go through the pdata and verify the sizes all match and record the size and export it
        // Might as well memcmp all the functions too, and then hard code the size for x86.
        // We do not really need the functions to be identical.

        // Make sure they are ascending, to give us confidence in our size computation for x86
        // and for all architectures to check for uniqueness, more efficiently than other ways.
        // The thunks are compiled without /Gy or with /Gy- to ensure this.
        for (size_t i = 0; i < COUNT - 1; ++i)
        {
            if (!NT_VERIFY(JsThunkData[i + 1].Thunk > JsThunkData[i].Thunk))
            {
                DbgPrintEx(0, ~0u, "%s ascending check %u %u\n", __FUNCTION__, __LINE__, (UINT)i);
                return false;
            }
        }

        UINT32 size = { 0 };
#ifndef _X86_
        // Make sure they are all the same size and have pdata -- no tail calls.
        for (size_t i = 0; i < COUNT; ++i)
        {
            SIZE_T functionBase = { 0 };
            auto const runtimeFunction = RtlLookupFunctionEntry((size_t)JsThunkData[i].Thunk, &functionBase, NULL);
            if (!NT_VERIFY(runtimeFunction))
            {
                DbgPrintEx(0, ~0u, "%s failed check %u %u\n", __FUNCTION__, __LINE__, (UINT)i);
                return false;
            }
            if (!NT_VERIFY(base == (PBYTE)functionBase))
            {
                DbgPrintEx(0, ~0u, "%s failed check %u %u\n", __FUNCTION__, __LINE__, (UINT)i);
                return false;
            }
            size = (UINT)RtlpGetFunctionEndAddress(runtimeFunction, (size_t)base) - runtimeFunction->BeginAddress;
            if (i == 0)
            {
                JsThunk_Size = size;
            }
            else
            {
                if (!NT_VERIFY(size == RtlpGetFunctionEndAddress(runtimeFunction, (size_t)base) - runtimeFunction->BeginAddress))
                {
                    DbgPrintEx(0, ~0u, "%s failed check %u %u\n", __FUNCTION__, __LINE__, (UINT)i);
                    return false;
                }
            }
        }
#else
        // Make sure they are all the same size.
        // Size is estimated as next function - previous function, trimming trailing padding breakpoints.
        // Notice that we do not check the last function, but we do check the memcmp, so ok.

        for (size_t i = 0; i < COUNT - 1; ++i)
        {
            size = (size_t)JsThunkData[i + 1].Thunk - (size_t)JsThunkData[i].Thunk;
            while (size > 0 && ((PBYTE)JsThunkData[i].Thunk)[size - 1] == 0xCC) // remove trailing breakpoints
                size -= 1;

            if (!NT_VERIFY(size != 0))
            {
                DbgPrintEx(0, ~0u, "%s failed check %u %u\n", __FUNCTION__, __LINE__, (UINT)i);
                //__debugbreak();
                return false;
            }

            if (i == 0)
            {
                JsThunk_Size = size;
            }
            else
            {
                if (!NT_VERIFY(size == JsThunk_Size))
                {
                    DbgPrintEx(0, ~0u, "%s failed check %u %u\n", __FUNCTION__, __LINE__, (UINT)i);
                    //__debugbreak();
                    return false;
                }
            }
        }
#endif

#if defined(_X86_) || defined(_AMD64_)
        // Check that the last instruction is a ret, i.e. not a tail call (assuming they do not look similar).
        // This does not guarantee that the byte is run.


        if (!NT_VERIFY(((PBYTE)JsThunkData[0].Thunk)[JsThunk_Size - 1] == 0xC3))
        {
            DbgPrintEx(0, ~0u, "%s failed check %u\n", __FUNCTION__, __LINE__);
            //__debugbreak();
            return false;
        }
#endif
#if 0   // The data offsets now make the functions not identical.
        // Make sure they all have the same bytes (NOTE: this means you cannot
        // set breakpoints on the thunks before DllMain, which is "unintended anti-debugging").
#if !defined(_CONTROL_FLOW_GUARD) && !JSCRIPT_THUNK_STACK_INSTRUMENTATION
        for (size_t i = 1; i < COUNT; ++i)
        {
            if (!NT_VERIFY(memcmp(JsThunkData[i].Thunk, JsThunkData[0].Thunk, size) == 0))
            {
                DbgPrintEx(0, ~0u, "%s memcmp failed %X\n", __FUNCTION__, (UINT)i);
                //__debugbreak();
                return false;
            }
        }
#else
        // The code bytes vary because distance to pJScriptThunkInstrumentation varies, ok.
        // Similarly for CFG -- distance to __guard_dispatch_icall_fptr varies, ok.
#endif
#endif
        JsThunk_SizeOfImage = ((PIMAGE_NT_HEADERS)(((PIMAGE_DOS_HEADER)base)->e_lfanew + base))->OptionalHeader.SizeOfImage;
#ifndef _X86_
        ULONG functionTableSize = { 0 };
        auto const functionTable = (PRUNTIME_FUNCTION)RtlImageDirectoryEntryToData(
            base, TRUE, IMAGE_DIRECTORY_ENTRY_EXCEPTION, &functionTableSize);
        if (functionTable && functionTableSize && (functionTableSize % sizeof(RUNTIME_FUNCTION)) == 0)
        {
            JsThunk_FunctionTableEntryCount = functionTableSize / sizeof(RUNTIME_FUNCTION);
            JsThunk_FunctionTableOffset = (UINT32)((PBYTE)functionTable - base);
        }
#endif

        JsThunk_FileMapping = fileMapping.Detach();

        return true;
    }
}

JsThunkData_t*
JsThunk_AllocateDataInternal(
    void
    )
{
    while (1)
    {
        auto entry = PopEntryList(&JsThunk_Pool);
        if (entry == NULL)
            return NULL;

        auto const thunk = CONTAINING_RECORD(entry, JsThunkData_t, u.free.Link);
        if (thunk->Thunk)
            return thunk;
    }
}

JsThunkData_t*
__stdcall
JsThunk_AllocateData(
    void
    )
{
    //__debugbreak();

    SRWLockHolderExclusive_t lock(&JsThunk_Lock);

    auto thunk = JsThunk_AllocateDataInternal();
    if (thunk)
        return thunk;

    // The rest is rare slow path -- get another chunk of thunks.
    // In the unlikely event that we are called before DllMain, attempt
    // to initialize on-demand here.

    if (JsThunk_FileMapping == NULL)
    {
        if (JsThunk_ProcessAttach() == false)
            return NULL;

        // Try again in case on-demand process attach worked.

        auto thunk = JsThunk_AllocateDataInternal();
        if (thunk)
            return thunk;
    }

    MappedViewOfFile_t<BYTE> view = (PBYTE)MapViewOfFile(JsThunk_FileMapping, FILE_MAP_READ, 0, 0, 0);
    if (view == NULL)
        return NULL;

    auto const base = (PBYTE)&__ImageBase;

#ifdef _CONTROL_FLOW_GUARD

    // Fill in __guard_check_icall_fptr and __guard_dispatch_icall_fptr.
    // They are in the new mapping at the same offset as the first mapping.

    //__debugbreak();

#ifdef _AMD64_
    PVOID* guard_pointers[] = {&__guard_check_icall_fptr, &__guard_dispatch_icall_fptr};
#else
    PVOID* guard_pointers[] = {&__guard_check_icall_fptr};
#endif

    for (auto guard_pointer: guard_pointers)
    {
        auto const p = (PVOID*)(((PBYTE)guard_pointer) - base + view);
        if (*p != *guard_pointer)
        {
            DWORD oldProtect;
            BOOL success = VirtualProtect(p, sizeof(PVOID), PAGE_READWRITE, &oldProtect);
            if (!success) // This can fail due to low resources.
                return NULL;

            *p = *guard_pointer;
            success = VirtualProtect(p, sizeof(PVOID), oldProtect, &oldProtect);
            if (!success) // This should never fail.
                return NULL;

            if (*p != *guard_pointer)
                RtlFailFast(FAST_FAIL_MRDATA_MODIFIED);
        }
    }

#endif

    RtlGrowableFunctionTable_t functionTable;

#ifndef _X86_
    // RtlInsertInvertedFunctionTable   // not exported, alas
    // RtlAddFunctionTable              // pre-Win8, no kernel->user stack traces (e.g. ETW sampling)
    auto const offset = JsThunk_FunctionTableOffset;
    auto const count = JsThunk_FunctionTableEntryCount;
    if (offset && count)
    {
        auto const error = RtlAddGrowableFunctionTable(&functionTable, (PRUNTIME_FUNCTION)(view.get() + offset),
                count, count, (size_t)view.get(), (size_t)view.get() + JsThunk_SizeOfImage);
        if (error)
            return NULL;
    }
#endif // x86

    JsThunk_Views.insert(JsThunk_Views.end(), view);

    // Get thunks in the new mapping, at the same offset as in the first mapping,
    // and add them to the pool. Reserve the first to be the return value.
    // Relocate, in case kernel does not.
    // NOTE The thunks are position independent.

    auto const entry = (PSINGLE_LIST_ENTRY)((PBYTE)&JsThunkData[0].u.free.Link - base + view);
    thunk = CONTAINING_RECORD(entry, JsThunkData_t, u.free.Link);

    for (size_t i = 0; i < COUNT; ++i)
    {
        thunk[i].Thunk = (JsFunc0_t)((PBYTE)JsThunkData[i].Thunk - base + view);
        thunk[i].u.free.Link.Next = &thunk[i + 1].u.free.Link;
    }

    JsThunk_Pool.Next = &thunk[1].u.free.Link;

#if JSCRIPT_THUNK_STACK_INSTRUMENTATION
    *(PVOID*)((size_t)&pJScriptThunkInstrumentation - (size_t)&__ImageBase + view.get()) = (PVOID)pJScriptThunkInstrumentation;
#endif

    functionTable.Detach();
    view.Detach();

    return thunk;
}

unsigned
__stdcall
JsThunk_GetSize(
    void
    )
{
    return JsThunk_Size;
}

void
__stdcall
JsThunk_InitData(
    JsThunkData_t* ThunkData,
    void* /*JsFunc1_t*/ InterpreterThunk
    )
{
    if (ThunkData == NULL)
        return;

    ThunkData->u.in_use.InterpreterThunk = (JsFunc1_t)InterpreterThunk;
}

void
__stdcall
JsThunk_Cleanup(
    void* ThunkData
    )
{
    if (ThunkData == NULL)
        return;

    SRWLockHolderExclusive_t lock(&JsThunk_Lock);

    PushEntryList(&JsThunk_Pool, &((JsThunkData_t*)ThunkData)->u.free.Link);
}

void
__stdcall
JsThunk_CleanupFinish(PSINGLE_LIST_ENTRY DeferredCleanup)
// flush deferred cleanup
{
    // __debugbreak();

    SRWLockHolderExclusive_t lock(&JsThunk_Lock);

    while (auto const entry = PopEntryList(DeferredCleanup))
    {
        auto const data = CONTAINING_RECORD(entry, JsThunkData_t, u.free.Link);
        if (data->Thunk)
            PushEntryList(&JsThunk_Pool, &data->u.free.Link);
    }
}

void
__stdcall
JsThunk_CleanupDefer(
    PSINGLE_LIST_ENTRY DeferredCleanup,
    PVOID ThunkData
    )
// deferred cleanup of one thunk
{
    //__debugbreak();

    if (DeferredCleanup == NULL || ThunkData == NULL)
        return;

    PushEntryList(DeferredCleanup, &((JsThunkData_t*)ThunkData)->u.free.Link);
}

bool
JsThunk_Is(
    PVOID VoidThunk
    )
// Return if the thunk is in one of our mappings, even if it has not been allocated or is not a thunk.
{
    //__debugbreak();

    PBYTE thunk = (PBYTE)VoidThunk;

    if (thunk >= (PBYTE)&__ImageBase && thunk < (JsThunk_SizeOfImage + (PBYTE)&__ImageBase))
        return true;

    SRWLockHolderShared_t lock(&JsThunk_Lock);

    // Find the first mapping after this thunk and go back one.
    // If all mappings start before or at this thunk, end() is returned, check the last one, which is also going back one.

    if (JsThunk_Views.size() == 0)
        return false;

    auto upper_bound = JsThunk_Views.upper_bound(thunk);
    NT_ASSERT(upper_bound == JsThunk_Views.end() || *upper_bound > thunk);
    auto const mapping = *--upper_bound;
    return thunk >= mapping && thunk < (mapping + JsThunk_SizeOfImage);
}

#if JSCRIPT_THUNK_STACK_INSTRUMENTATION

struct JSThunkThreadData_t
// The instrumentation deliberately leaks so data is available after threads exit.
{
    JSThunkThreadData_t * volatile Next;
    size_t volatile StackUsed;
    size_t volatile StackBase;
    size_t volatile DeallocationStack;
    UINT32 volatile ThreadId;
};

__declspec(thread) JSThunkThreadData_t * JSThunkThreadData_tls;
SRWLOCK                                  JSThunkThreadData_lock;
JSThunkThreadData_t * volatile           JSThunkThreadData;

DWORD
__stdcall
JScriptThunkInstrumentation_ReportThread(PVOID)
{
    while (1)
    {
        Sleep(10000);
        {
            auto const parameters = NtCurrentPeb()->ProcessParameters;
            auto const & commandLine = parameters->CommandLine;
            auto const & currentDirectory = parameters->CurrentDirectory.DosPath;
            CStringW base = CStringW(commandLine.Buffer, commandLine.Length / sizeof(WCHAR));
            base += _u(" ");
            base += CStringW(currentDirectory.Buffer, currentDirectory.Length / sizeof(WCHAR));
            for (auto data = JSThunkThreadData; data; data = data->Next)
            {
                CStringW s = base;
                s.AppendFormat(_u(" threadId:%X MaxStack:%Iuk\n"), data->ThreadId, data->StackUsed / 1024);
                DbgPrintEx(0, ~0u, "%ls\n", s.GetString());
            }
        }
    }
    return 0;
}

__declspec(noinline)
void
JScriptThunkInstrumentation(void)
{
    //__debugbreak();
    auto tls = JSThunkThreadData_tls;
    if (!tls)
    {
        tls = (JSThunkThreadData_t*)calloc(1, sizeof(JSThunkThreadData_t));
        if (tls)
        {
            JSThunkThreadData_tls = tls;
            tls->ThreadId = RtlGetCurrentThreadId();
            tls->DeallocationStack = (size_t)NtCurrentTeb()->DeallocationStack;
            tls->StackBase = (size_t)NtCurrentTeb()->NtTib.StackBase;

            MemoryBarrier();
            SRWLockHolderExclusive_t lock(&JSThunkThreadData_lock);
            tls->Next = JSThunkThreadData;
            MemoryBarrier();
            JSThunkThreadData = tls;
        }
    }
    if (tls)
    {
        size_t stack = (size_t)_alloca(1);
        size_t stackUsed = tls->StackBase - stack;
        if (stackUsed > tls->StackUsed)
            tls->StackUsed = stackUsed;
    }
}

#endif

BOOL
DllMain(
    HMODULE DllHandle,
    DWORD Reason,
    PVOID Reserved
    )
{
    switch (Reason)
    {
    case DLL_PROCESS_ATTACH:
        {
            //__debugbreak();
            DisableThreadLibraryCalls(DllHandle);
            SRWLockHolderExclusive_t lock(&JsThunk_Lock);
#if JSCRIPT_THUNK_STACK_INSTRUMENTATION
            CreateThread(0, 0, JScriptThunkInstrumentation_ReportThread, 0, 0, 0);
#endif
            return JsThunk_ProcessAttach();
        }
    }
    return TRUE;
}
