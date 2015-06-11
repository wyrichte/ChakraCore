//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//---------------------------------------------------------------------------
#include "stdafx.h"

extern HANDLE g_hInstance;

namespace JsDiag
{
    const wchar_t* DebugClient::c_js9ModuleName = JS9_MODULE_NAME;
    const wchar_t* DebugClient::c_js9FileName = JS9_MODULE_NAME L".dll";
    const wchar_t* DebugClient::c_js9TestModuleName = JS9_MODULE_NAME L"test";
    const wchar_t* DebugClient::c_js9TestFileName = JS9_MODULE_NAME L"test.dll";

    const LibraryName DebugClient::c_librariesToTry[] = {
        { DebugClient::c_js9ModuleName, DebugClient::c_js9FileName }, 
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        // ENABLE_DEBUG_CONFIG_OPTIONS is to make sure we don't ship with check for jscript9test.dll.
        { DebugClient::c_js9TestModuleName, DebugClient::c_js9TestFileName } 
#endif
    };

    DebugClient::DebugClient(DiagProvider* diagProvider) : 
        m_diagProvider(diagProvider),
        m_hasTargetHooks(false),
        m_js9BaseAddr(0),
        m_js9HighAddress(0)
    {
        Assert(diagProvider);
    }

    DebugClient::~DebugClient()
    {
    }

    void DebugClient::EnsureTargetHooks()
    {
        if(!m_hasTargetHooks)
        {
            this->InitializeTargetHooks();
        }
    }

    void DebugClient::GetVTables(_Outptr_result_buffer_(*vtablesSize) const VTABLE_PTR** vtables, _Out_ ULONG* vtablesSize)
    {
        EnsureTargetHooks();
        *vtables = m_vtables;
        *vtablesSize = _countof(m_vtables);
    }

    bool DebugClient::IsVTable(const void* vtable,
        _In_range_(0, Diag_MaxVTable - 1) Diag_VTableType first,
        _In_range_(0, Diag_MaxVTable - 1) Diag_VTableType last)
    {
        Assert(first < Diag_MaxVTable && last < Diag_MaxVTable);
        EnsureTargetHooks();

        for (int i = first; i <= last; i++)
        {
            if (m_vtables[i] == vtable)
            {
                return true;
            }
        }

        return false;
    }

    void DebugClient::InitializeTargetHooks()
    {
        // Load/unload jscript9.dll instance locally and get offsets from it so that we can hook to target instance.
        this->LoadTargetAndExecute([&](IDiagHook* hook, ULONG64 moduleBaseAddr64) {
            this->GetGlobals(hook, moduleBaseAddr64);
            this->GetVTables(hook, moduleBaseAddr64, m_vtables, _countof(m_vtables));
            this->GetErrorStrings(hook);
            m_hasTargetHooks = true;
        });
    }

    // Returns the index of TLS slot used by ThreadContextTLSEntry in remote process.
    // This is used to get the actual TLS slot, similar to TlsGetValue(index).
    template <typename T>
    void DebugClient::LoadTargetAndExecute(T operation)
    {
        // Load the library (try jscript9.dll, then jscript9test.dll).
        int tryCount = sizeof(c_librariesToTry) / sizeof(LibraryName);
        for (int i = 0; i < tryCount; ++i)
        {
            LibraryName libraryName = c_librariesToTry[i];

            // Get the module base from target process.
            Assert(m_js9BaseAddr == 0);
            bool isSuccess = m_diagProvider->TryGetTargetModuleBaseAddr(libraryName.ModuleName, &m_js9BaseAddr);
            if (!isSuccess)
            {
                continue;   // The specified module should not be loaded in target process, try next module.
            }
            void* moduleBaseAddr = reinterpret_cast<void*>(m_js9BaseAddr);
            Assert((ULONG64)moduleBaseAddr == m_js9BaseAddr);

            // Load the library (with auto-unload).
            AutoLibrary library(libraryName.FileName);
            {
                CComPtr<IDiagHook> diagHook;
                diagHook = this->GetDiagHook(library.Handle);
                operation(diagHook, m_js9BaseAddr);
            }
            return;
        }

        // Could not load any of supported libraries.
        DiagException::Throw(E_INVALIDARG, DiagErrorCode::CHAKRA_MODULE_NOTFOUND);
    }

    // Returns address of Js::Congfiguration::Global, in target process address space.
    void DebugClient::GetGlobals(IDiagHook* diagHook, ULONG64 moduleBaseAddr)
    {
        size_t globalsSize = _countof(m_globals);
        HRESULT hr = diagHook->GetGlobals(reinterpret_cast<void**>(&m_globals), globalsSize);
        CheckHR(hr, DiagErrorCode::DIAGHOOK_GETGLOBALS);

        for (ULONG i = 0; i < globalsSize; i++)
        {
            m_globals[i] = reinterpret_cast<LPVOID>(reinterpret_cast<LPBYTE>(m_globals[i]) + moduleBaseAddr);
        }
    }

    // Addr of jscript9 TEB entry, in the remote process address space.
    ThreadContextTLSEntry* DebugClient::GetThreadContextTlsEntry(ULONG threadId)
    {
        EnsureTargetHooks();

        DWORD tlsSlotIndex = this->GetGlobalValue<DWORD>(Globals_TlsSlot);
        
        UINT64 tebEntryAddr;
        HRESULT hr = m_diagProvider->GetTlsValue(threadId, tlsSlotIndex, &tebEntryAddr);
        CheckHR(hr, DiagErrorCode::DIAG_GET_TLSVALUE);
        
        return (ThreadContextTLSEntry*)tebEntryAddr;
    }

    bool DebugClient::IsJsModuleAddress(void* address)
    {
        if(!m_js9HighAddress)
        {
            EnsureTargetHooks();
            m_js9HighAddress = (UINT64)(this->GetGlobalValue<UINT_PTR>(Globals_DllHighAddress));
            Assert(this->GetGlobalValue<UINT_PTR>(Globals_DllBaseAddress) == m_js9BaseAddr);
            Assert(m_js9HighAddress != NULL);
        }
        if(address >= (void*)m_js9BaseAddr && address <= (void*)m_js9HighAddress)
        {
            return true;
        }
        return false;
    }

    //
    // Obtain needed vtable info in the specified module
    //
    void DebugClient::GetVTables(IDiagHook* diagHook, ULONG64 moduleBaseAddr,
        _Out_writes_all_(vtablesSize) VTABLE_PTR* vtables, ULONG vtablesSize)
    {
        HRESULT hr = diagHook->GetVTables((void**)vtables, vtablesSize);
        CheckHR(hr, DiagErrorCode::DIAGHOOK_GETVTABLE);

        for (ULONG i = 0; i < vtablesSize; i++)
        {
            vtables[i] = reinterpret_cast<VTABLE_PTR>(reinterpret_cast<const BYTE*>(vtables[i]) + moduleBaseAddr);
        }
    }

    void DebugClient::GetErrorStrings(IDiagHook* diagHook)
    {
        static HRESULT errorCodes[] =
        {
            DIAGERR_FunctionCallNotSupported,
            DIAGERR_EvaluateNotSupported,
            JSERR_Property_CannotGet_NullOrUndefined,
        };

        for (int i = 0; i < _countof(errorCodes); i++)
        {
            HRESULT code = errorCodes[i];
            CComBSTR bs;
            CheckHR(diagHook->GetErrorString(code, &bs));
            m_errorStrings[code] = bs;
        }
    }

    CComPtr<IDiagHook> DebugClient::GetDiagHook(HMODULE libraryHandle)
    {
        typedef HRESULT (STDAPICALLTYPE* FN_DllGetClassObject)(REFCLSID, REFIID, LPVOID*);
        FN_DllGetClassObject proc = (FN_DllGetClassObject)::GetProcAddress(libraryHandle, "DllGetClassObject");
        if (proc == NULL)
        {
            DiagException::Throw(HRESULT_FROM_WIN32(::GetLastError()), DiagErrorCode::DIAGHOOK_GETPROC);
        }

        CComPtr <IClassFactory> classFactory;
        HRESULT hr = proc(CLSID_DiagHook, __uuidof(IClassFactory), (LPVOID*)&classFactory);
        CheckHR(hr, DiagErrorCode::DIAGHOOK_CREATE);

        CComPtr<IDiagHook> diagHook;
        hr = classFactory->CreateInstance(NULL, __uuidof(IDiagHook), (PVOID*)&diagHook);
        CheckHR(hr, DiagErrorCode::DIAGHOOK_CREATE);

        return diagHook;
    }

     RemoteStackFrameEnumerator* DebugClient::CreateStackFrameEnumerator(ULONG threadId, void* advanceToAddr)
     {
         return m_diagProvider->CreateStackFrameEnumerator(threadId, advanceToAddr);
     }

     IVirtualReader* DebugClient::GetReader() const
     { 
         return m_diagProvider->GetReader(); 
     }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // AutoLibrary
    AutoLibrary::AutoLibrary(LPCWSTR fileName):
        Handle(NULL)
    {
        if (fileName)
        {
            Load(fileName);
        }
    }

    void AutoLibrary::Load(LPCWSTR fileName)
    {
        // Note that loading library increments its ref count, so that it's already loaded, we'll get same module.
        // Unloading decrements ref count and actually unloads the library only when ref count becomes 0.
        // TODO: how about search paths? Can this load wrong version of DLL from more important path?
        this->Handle = ::LoadLibraryExW(fileName, NULL, 0);
        if (!this->Handle)
        {
            DiagException::Throw(HRESULT_FROM_WIN32(::GetLastError()), DiagErrorCode::LOAD_LIBRARY);
        }
    }

    AutoLibrary::~AutoLibrary()
    {
        if (this->Handle)
        {
            ::FreeLibrary(this->Handle);
        }
    }

} // namespace JsDiag.
