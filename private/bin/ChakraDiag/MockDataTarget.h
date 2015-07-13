//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once
#include "winternl.h"

// Note: This is test code (implements IJsDiagTestDataTarget). Used by multiple test projects.

//
// Debugger thread setup helper. Ensures to setup and restore current DebugClient thread.
//
class DbgEngThreadSetup
{
private:
    IDebugSystemObjects* m_debugSystemObjects;
    ULONG m_previousThreadId;
    bool m_shouldRestore;

private:
    DbgEngThreadSetup(IDebugSystemObjects* debugSystemObjects):
        m_debugSystemObjects(debugSystemObjects),
        m_shouldRestore(false)
    {
    }

    HRESULT Init(ULONG threadId)
    {
        HRESULT hr = S_OK;

        IfFailGo(GetCurrentThreadSystemId(&m_previousThreadId));
        if (threadId != m_previousThreadId)
        {
            IfFailGo(SetCurrentThreadBySystemId(threadId));
            m_shouldRestore = true;
        }
Error:
        return hr;
    }

    HRESULT UnInit()
    {
        HRESULT hr = S_OK;
        if (m_shouldRestore)
        {
            hr = SetCurrentThreadBySystemId(m_previousThreadId);
        }
        return hr;
    }

    HRESULT GetCurrentThreadSystemId(ULONG* pSystemThreadId)
    {
        return m_debugSystemObjects->GetCurrentThreadSystemId(pSystemThreadId);
    }

    HRESULT SetCurrentThreadBySystemId(ULONG systemThreadId)
    {
        HRESULT hr = S_OK;

        ULONG threadId;
        IfFailGo(m_debugSystemObjects->GetThreadIdBySystemId(systemThreadId, &threadId));
        IfFailGo(m_debugSystemObjects->SetCurrentThreadId(threadId));
Error:
        return hr;
    }

public:
    //
    // Switch DebugClient to given threadId and run func. Restore original DebugClient current thread at exit.
    //
    template <class Func>
    static HRESULT Run(IDebugSystemObjects* debugSystemObjects, ULONG threadId, Func func)
    {
        HRESULT hr = S_OK;
        DbgEngThreadSetup threadSetup(debugSystemObjects);

        IfFailGo(threadSetup.Init(threadId));
        hr = func();
Error:
        // We always want to restore previous thread, but don't want to overwrite hr if it failed from func().
        {
            HRESULT hr2 = threadSetup.UnInit();
            if (SUCCEEDED(hr))
            {
                hr = hr2;
            }
        }
        return hr;
    }
};

// ------------------------------------------------------------------------------------------------
// dbghelp StackWalk64 based native stack walker
// ------------------------------------------------------------------------------------------------
class DbgHelpStackWalker
{
private:
    CComPtr<IDebugControl>          m_debugControl;
    CComPtr<IDebugDataSpaces>       m_debugDataSpaces;
    CComPtr<IDebugSymbols>          m_debugSymbols;
    CComPtr<IDebugSystemObjects>    m_debugSystemObjects;
    bool m_symInitialized;

    static DbgHelpStackWalker* s_instance; // Used temporarily by StackWalk64 callbacks

public:
    HRESULT Init(IDebugClient* client)
    {
        HRESULT hr = S_OK;
        
        IfFailGo(client->QueryInterface(IID_PPV_ARGS(&m_debugControl)));
        IfFailGo(client->QueryInterface(IID_PPV_ARGS(&m_debugDataSpaces)));
        IfFailGo(client->QueryInterface(IID_PPV_ARGS(&m_debugSymbols)));
        IfFailGo(client->QueryInterface(IID_PPV_ARGS(&m_debugSystemObjects)));
        m_symInitialized = false;
Error:
        return hr;
    }

    HRESULT GetStackTrace(
        _In_ ULONG ThreadId,
        _Inout_ LPSTACKFRAME64 StackFrame,
        _Inout_ PVOID ContextRecord)
    {
        return DbgEngThreadSetup::Run(m_debugSystemObjects, ThreadId, [&]() -> HRESULT
        {
            HRESULT hr = S_OK;

            ULONG machineType;
            ULONG64 hProcess;
            ULONG64 hThread;
            IfFailGo(m_debugControl->GetEffectiveProcessorType(&machineType));
            IfFailGo(m_debugSystemObjects->GetCurrentProcessHandle(&hProcess));
            IfFailGo(m_debugSystemObjects->GetCurrentThreadHandle(&hThread));

            if (!m_symInitialized)
            {
                if (!SymInitialize((HANDLE)hProcess, NULL, FALSE))
                {
                    IfFailGo(HRESULT_FROM_WIN32(GetLastError()));
                }
                m_symInitialized = true;
            }

            Assert(!s_instance);
            {
                s_instance = this; // Temporarily set global instance used by callbacks
                BOOL result = StackWalk64(
                    machineType,
                    (HANDLE)hProcess,
                    (HANDLE)hThread,
                    StackFrame,
                    ContextRecord,
                    ReadProcessMemoryProc64,
                    FunctionTableAccessProc64,
                    GetModuleBaseProc64,
                    NULL);
                s_instance = NULL; // Clear global instance

                if (!result)
                {
                    hr = E_FAIL;
                }
            }
Error:
            return hr;
        });
    }

private:
    static DWORD64 CALLBACK GetModuleBaseProc64(
        _In_  HANDLE hProcess,
        _In_  DWORD64 Address)
    {
        UNREFERENCED_PARAMETER(hProcess);
        Assert(s_instance);

        ULONG64 base = 0;
        HRESULT hr = s_instance->m_debugSymbols->GetModuleByOffset(Address, 0, NULL, &base);
        hr;// Ignore result hr. Address may be unknown.
        return base;
    }

    static BOOL CALLBACK ReadProcessMemoryProc64(
        _In_   HANDLE hProcess,
        _In_   DWORD64 lpBaseAddress,
        _Out_  PVOID lpBuffer,
        _In_   DWORD nSize,
        _Out_  LPDWORD lpNumberOfBytesRead)
    {
        UNREFERENCED_PARAMETER(hProcess);
        Assert(s_instance);

        ULONG bytesRead;
        HRESULT hr = s_instance->m_debugDataSpaces->ReadVirtual(lpBaseAddress, lpBuffer, nSize, &bytesRead);
        if (SUCCEEDED(hr))
        {
            *lpNumberOfBytesRead = bytesRead;
            return TRUE;
        }

        return FALSE;
    }

    static PVOID CALLBACK FunctionTableAccessProc64(
        _In_  HANDLE hProcess,
        _In_  DWORD64 AddrBase)
    {
        return SymFunctionTableAccess64(hProcess, AddrBase);
    }
};

// ------------------------------------------------------------------------------------------------
// dbgeng IDebugControl::GetStackTrace based native stack walker
// ------------------------------------------------------------------------------------------------
class DbgEngStackWalker
{
private:
    static const ULONG c_initialFrameId = static_cast<ULONG>(-1);

    CComPtr<IDebugControl>          m_debugControl;
    CComPtr<IDebugSystemObjects>    m_debugSystemObjects;

    DEBUG_STACK_FRAME* m_buffer;
    ULONG m_cachedFrameCount;
    ULONG m_currentFrameId;
    ULONG m_growDelta;
    ULONG m_maxFrameCount;   // MAX number of requestable frames from provider. Before we get to last request, we put m_cachedFrameCount + 1 here.

public:
    DbgEngStackWalker() :
        m_buffer(NULL)
    {
        Reset();
    }

    ~DbgEngStackWalker()
    {
        delete[] m_buffer;
    }

    HRESULT Init(IDebugClient* client)
    {
        HRESULT hr = S_OK;
        
        IfFailGo(client->QueryInterface(IID_PPV_ARGS(&m_debugControl)));
        IfFailGo(client->QueryInterface(IID_PPV_ARGS(&m_debugSystemObjects)));
Error:
        return hr;
    }

    HRESULT GetStackTrace(
        _In_ ULONG ThreadId,
        _Inout_ LPSTACKFRAME64 StackFrame,
        _Inout_ PVOID ContextRecord)
    {
        UNREFERENCED_PARAMETER(ContextRecord);

        return DbgEngThreadSetup::Run(m_debugSystemObjects, ThreadId, [&]() -> HRESULT
        {
            HRESULT hr = S_OK;

            // If this is a new stackwalk session, reset to restart.
            // WARNING: The following test assumes usage pattern from DbgEngStackFrameEnumerator. For the initial call we pass an empty ReturnAddress.
            if (StackFrame->AddrReturn.Offset == 0)
            {
                Reset();
            }

            SecureZeroMemory(StackFrame, sizeof(*StackFrame));
            hr = Next();
            if (hr == S_OK) // S_FALSE for no more frames
            {
                IfFailGo(Current(StackFrame));
            }
Error:
            return hr;
        });
    }

private:
    // Reset this stack walker for a new stack walk session
    void Reset()
    {
        delete[] m_buffer;
        m_buffer = NULL;

        m_cachedFrameCount = 0;
        m_currentFrameId = c_initialFrameId;

        ULONG cacheSize = 64;
        {
#if defined(DBG) || defined(ENABLE_DEBUG_CONFIG_OPTIONS)
            wchar_t envBuf[16];
            DWORD storedCharCount = GetEnvironmentVariableW(L"FrameCacheSize", envBuf, sizeof(envBuf) / sizeof(wchar_t));
            if (storedCharCount > 0)
            {
                cacheSize = _wtoi(envBuf);
            }
#endif
        }

        Assert(cacheSize > 1);
        m_growDelta = cacheSize;
        m_maxFrameCount = cacheSize;
    }

    // Advances to next frame. Returns S_FALSE if no more frames.
    HRESULT Next()
    {
        HRESULT hr = S_OK;

        Assert(m_currentFrameId + 1 <= m_maxFrameCount);
        if (m_currentFrameId + 1 == m_maxFrameCount)
        {
            // There are no more frames available in provider.
            return S_FALSE;
        }

        // Check if cache needs to be refreshed.
        if (m_currentFrameId + 1 == m_cachedFrameCount)
        {
            ULONG newCachedFrameCount = m_cachedFrameCount + m_growDelta;

            delete[] m_buffer;
            m_buffer = new DEBUG_STACK_FRAME[newCachedFrameCount + 1];
            // +1 is to account for EffectiveFrameBase for amd64, so that we always have one extra frame below m_currentFrameId.
            if (!m_buffer)
            {
                IfFailGo(E_OUTOFMEMORY);
            }

            ULONG requestedFrameCount = newCachedFrameCount + 1;
            ULONG filledFrameCount;
            IfFailGo(this->GetStackFrames(0, 0, 0, m_buffer, requestedFrameCount, &filledFrameCount));
            if (filledFrameCount < requestedFrameCount)
            {
                if (filledFrameCount == m_cachedFrameCount)
                {
                    // No more new frames in this request. Note: we could delete[] the buffer here.
                    m_maxFrameCount = m_currentFrameId + 1;
                    return S_FALSE;
                }

                // Fill 'bottom + 1' frame with zeroes (used for amd64's EffectiveFrameBase).
                SecureZeroMemory(m_buffer + filledFrameCount, sizeof(DEBUG_STACK_FRAME));
                m_maxFrameCount = filledFrameCount;
            }
            else
            {
                m_maxFrameCount = newCachedFrameCount + 1;    // Set to some big number greater than m_cachedFrameCount.
            }
            m_cachedFrameCount = newCachedFrameCount;
        }

        ++m_currentFrameId;
        hr = S_OK;
Error:
        return hr;
    }

    // Returns current frame
    HRESULT Current(_Out_ LPSTACKFRAME64 frame)
    {
        HRESULT hr = S_OK;

        ULONG effectiveMaxFrameCount = m_maxFrameCount < m_cachedFrameCount ? m_maxFrameCount : m_cachedFrameCount;
        if (m_currentFrameId == c_initialFrameId || m_currentFrameId >= effectiveMaxFrameCount)
        {
            IfFailGo(E_INVALIDARG);
        }

        // scope
        {
            DEBUG_STACK_FRAME currentFrame = m_buffer[m_currentFrameId];
            Assert(frame);
            SetAddress64(frame->AddrFrame, currentFrame.FrameOffset);
            SetAddress64(frame->AddrPC, currentFrame.InstructionOffset);
            SetAddress64(frame->AddrReturn, currentFrame.ReturnOffset);
            SetAddress64(frame->AddrStack, currentFrame.StackOffset);
        }
Error:
        return hr;
    }

    void SetAddress64(ADDRESS64& addr, ULONG64 offset)
    {
        addr.Mode = ADDRESS_MODE::AddrModeFlat;
        addr.Offset = offset;
    }

    // Call the provider to get new frames.
    HRESULT GetStackFrames(ULONG64 frameOffset, ULONG64 stackOffset, ULONG64 instructionOffset, DEBUG_STACK_FRAME* frames, ULONG frameCount, PULONG frameCountFilled)
    {
        return m_debugControl->GetStackTrace(frameOffset, stackOffset, instructionOffset, frames, frameCount, frameCountFilled);
    }

    void DumpCache()
    {
        wprintf(L"\n");
        ULONG frameCount = m_maxFrameCount < m_cachedFrameCount ? m_maxFrameCount : m_cachedFrameCount;
        for (ULONG i = 0; i < frameCount; ++i)
        {
            DEBUG_STACK_FRAME frame = m_buffer[i];
#ifdef _M_X64
            wprintf(L"Frm=0x%016llX Stk=0x%016llX IP=0x%016llX Ret=0x%016llX\n", frame.FrameOffset, frame.StackOffset, frame.InstructionOffset, frame.ReturnOffset);
#else
            wprintf(L"Frm=0x%08X Stk=0x%08X IP=0x%08X Ret=0x%08X\n", static_cast<ULONG>(frame.FrameOffset), static_cast<ULONG>(frame.StackOffset),
                static_cast<ULONG>(frame.InstructionOffset), static_cast<ULONG>(frame.ReturnOffset));
#endif
        }
    }
};

// ------------------------------------------------------------------------------------------------
// dbgeng-based IJsDiagTestDataTarget implementation. (Having everything in header file because it is used by different proj.)
// ------------------------------------------------------------------------------------------------
class DbgEngDataTarget:
    public CComObjectRoot,
    public DummyTestGroup,
    public IJsDiagTestDataTarget
{
private:
    CComPtr<IDebugClient> m_debugClient;
    CComPtr<IDebugDataSpaces4> m_debugDataSpaces;
    CComPtr<IDebugSystemObjects> m_debugSystemObjects;

    // Currently dbghelp StackWalk64 doesn't work for us on amd64.
#ifdef JS_USE_DBGHELP_STACKWALK64
    DbgHelpStackWalker m_stackWalker;
#else
    DbgEngStackWalker m_stackWalker;
#endif

    std::vector<std::pair<ULONG64, ULONG32>> m_memoryRegions;

public:
    BEGIN_COM_MAP(DbgEngDataTarget)
        COM_INTERFACE_ENTRY(IStackProviderDataTarget)
        COM_INTERFACE_ENTRY(IJsDiagTestDataTarget)
    END_COM_MAP()

    HRESULT Init(IDebugClient* client)
    {
        HRESULT hr = S_OK;
        
        m_debugClient = client;
        IfFailGo(m_debugClient->QueryInterface(IID_PPV_ARGS(&m_debugDataSpaces)));
        IfFailGo(m_debugClient->QueryInterface(IID_PPV_ARGS(&m_debugSystemObjects)));

        IfFailGo(m_stackWalker.Init(client));
Error:
        return hr;
    }

    typedef std::vector<std::pair<ULONG64, ULONG32>> MemoryRegionsType;
    const MemoryRegionsType& GetMemoryRegions() const
    {
        return m_memoryRegions;
    }

    // *** IStackProviderDataTarget ***
    STDMETHODIMP GetModuleByModuleName(
        _In_ PCWSTR Name,
        _Out_opt_ PULONG64 Base)
    {
        CComPtr<IDebugSymbols3> symbols;
        HRESULT hr = m_debugClient->QueryInterface(IID_PPV_ARGS(&symbols));
        if (SUCCEEDED(hr))
        {
            hr = symbols->GetModuleByModuleNameWide(Name, /*StartIndex*/0, /*Index*/NULL, Base);
        }
        return hr;
    }

    STDMETHODIMP ReadVirtual(
        _In_ ULONG64 Address,
        _Out_writes_bytes_(Request) PBYTE Buffer,
        _In_ ULONG Request,
        _Out_opt_ PULONG Done)
    {
        return m_debugDataSpaces->ReadVirtual(Address, Buffer, Request, Done);
    }

    STDMETHODIMP GetThreadTeb(
        _In_ ULONG ThreadId,
        _Out_ PULONG64 Offset,
        _Out_ PULONG Size)
    {
        return DbgEngThreadSetup::Run(m_debugSystemObjects, ThreadId, [&]() -> HRESULT
        {        
            HRESULT hr = S_OK;

            ULONG64 tebAddr = 0;
            IfFailGo(m_debugSystemObjects->GetCurrentThreadTeb(&tebAddr)); // Now we have the TEB.
#ifndef _M_X64
            tebAddr &= 0xffffffff;  // TODO: figure out why high part on x86 is FFFFFFFF.
#endif

            *Offset = tebAddr;
            *Size = sizeof(_TEB);
Error:
            return hr;
        });
    }

    STDMETHODIMP GetThreadContext(
        _In_ ULONG ThreadId,
        _In_ ULONG ContextFlags,
        _In_ ULONG ContextSize,
        _Out_writes_bytes_(ContextSize) PBYTE Context)
    {
        UNREFERENCED_PARAMETER(ContextFlags);

        return DbgEngThreadSetup::Run(m_debugSystemObjects, ThreadId, [&]() -> HRESULT
        {
            HRESULT hr = S_OK;

            CComPtr<IDebugAdvanced> adv;
            IfFailGo(m_debugClient->QueryInterface(IID_PPV_ARGS(&adv)));
            IfFailGo(adv->GetThreadContext(Context, ContextSize));
Error:
            return hr;
        });
    }

    STDMETHODIMP GetStackTrace(
        _In_ ULONG ThreadId,
        _Inout_ LPSTACKFRAME64 StackFrame,
        _Inout_ PVOID ContextRecord)
    {
        return m_stackWalker.GetStackTrace(ThreadId, StackFrame, ContextRecord);
    }

    STDMETHODIMP EnumMemoryRegion(
        _In_ ULONG64 Address,
        _In_ ULONG32 Size
        )
    {
        try
        {
            m_memoryRegions.push_back(std::make_pair(Address, Size));
        }
        catch(const std::bad_alloc&)
        {
            return E_OUTOFMEMORY;
        }
        
        return S_OK;
    }
    
    // *** IJsDiagTestDataTarget ***
    STDMETHODIMP AllocateVirtual(
        _In_ ULONG64 address,
        _In_ SIZE_T size,
        _In_ DWORD allocationType,
        _In_ DWORD pageProtection,
        _Out_ ULONG64* pAllocatedAddress)
    {
        HRESULT hr = S_OK;

        ULONG64 handle;
        IfFailGo(m_debugSystemObjects->GetCurrentProcessHandle(&handle));

        void* allocatedAddress = VirtualAllocEx((HANDLE)handle, (void*)address, size, allocationType, pageProtection);
        if(allocatedAddress != NULL)
        {
            *pAllocatedAddress = (UINT64)allocatedAddress;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
Error:
        return hr;
    }

    STDMETHODIMP FreeVirtual(
        _In_ ULONG64 address,
        _In_ SIZE_T size,
        _In_ DWORD freeType)
    {
        HRESULT hr = S_OK;

        ULONG64 handle;
        IfFailGo(m_debugSystemObjects->GetCurrentProcessHandle(&handle));

        if(!VirtualFreeEx((HANDLE)handle, (void*)address, size, freeType))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
Error:
        return hr;
    }

    STDMETHODIMP WriteVirtual(
        _In_ ULONG64 address,
        _In_reads_bytes_(bufferSize) PVOID buffer,
        _In_ ULONG bufferSize,
        _Out_opt_ PULONG bytesWritten)
    {
        return m_debugDataSpaces->WriteVirtual(address, buffer, bufferSize, bytesWritten);
    }
};

//
// Call this macro in one source file to use DbgEngDataTarget. (Details: DbgEngDataTarget may use DbgHelpStackWalker
// to walk native stacks. The later uses a global for StackWalk64 callbacks. This macro instantiates needed global.)
//
#define USE_DbgEngDataTarget() \
    DbgHelpStackWalker* DbgHelpStackWalker::s_instance = NULL
