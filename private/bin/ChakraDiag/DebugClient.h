//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#pragma once

namespace JsDiag
{
    struct LibraryName;
    struct RemoteStackFrameEnumerator;
    typedef const void* VTABLE_PTR;

#define JS9_MODULE_NAME _u("chakra")

    //
    // Main entry point for debug/diag dealing with jscript9.dll internals. 
    // Does not depend on backend technology (such as dbgeng, VS, etc).
    //
    class DebugClient
    {
    private:
        DiagProvider* m_diagProvider;           // We don't own diagProvider.
        UINT64 m_js9BaseAddr;
        UINT64 m_js9HighAddress;
        VTABLE_PTR m_vtables[Diag_MaxVTable];
        void* m_globals[Globals_Count];
        bool m_hasTargetHooks;

        static const unsigned int c_pointerSize = sizeof(void*);
        static const char16* c_js9ModuleName;
        static const char16* c_js9FileName;
        static const char16* c_js9TestModuleName;
        static const char16* c_js9TestFileName;
        static const CLSID c_CLSID_DiagHook;
        static const LibraryName c_librariesToTry[];

    public:
        DebugClient(DiagProvider* diagProvider);
        ~DebugClient();

    public:
        void GetVTables(_Outptr_result_buffer_(*vtablesSize) const VTABLE_PTR** vtables, _Out_ ULONG* vtablesSize);
        ThreadContextTLSEntry* GetThreadContextTlsEntry(ULONG threadId);
        bool IsJsModuleAddress(void* address);

        template<class T>
        T GetGlobalValue(_In_range_(0, Globals_Max - 1) Globals valueId)
        {
            EnsureTargetHooks();
            Assert(valueId < Globals_Max);
            return VirtualReader::ReadVirtual<T>(m_diagProvider->GetReader(), m_globals[valueId]);
        }

        VTABLE_PTR GetVTable(_In_range_(0, Diag_MaxVTable - 1) Diag_VTableType vtableType)
        {
            EnsureTargetHooks();
            Assert(vtableType < Diag_MaxVTable);
            return m_vtables[vtableType];
        }

        bool IsVTable(VTABLE_PTR vtable,
            _In_range_(0, Diag_MaxVTable - 1) Diag_VTableType first,
            _In_range_(0, Diag_MaxVTable - 1) Diag_VTableType last);

        template<class T>
        T* GetGlobalPointer(_In_range_(0, Globals_Max - 1) Globals valueId)
        {
            EnsureTargetHooks();
            Assert(valueId < Globals_Max);
            return reinterpret_cast<T*>(m_globals[valueId]);
        }

        RemoteStackFrameEnumerator* CreateStackFrameEnumerator(ULONG threadId, void* advanceToAddr);

        DiagProvider* GetDiagProvider() const { return m_diagProvider; }
        IVirtualReader* GetReader() const;

    private:
        void InitializeTargetHooks();
        void EnsureTargetHooks();
        CComPtr<IDiagHook> GetDiagHook(HMODULE libraryHandle);
        template <typename T> void LoadTargetAndExecute(T operation);
        void GetGlobals(IDiagHook* diagHook, ULONG64 moduleBaseAddr);
        void GetVTables(IDiagHook* diagHook, ULONG64 moduleBaseAddr, _Out_writes_all_(vtablesSize) VTABLE_PTR* vtables, ULONG vtablesSize);
    };

    struct LibraryName
    {
        const char16* ModuleName;
        const char16* FileName;
    };

    // Loads the library unelss it's already loaded, unloads in dtor if it had to load it.
    // Like AutoPtr but for library/DLL.
    struct AutoLibrary
    {
        // The library handle. Note: for Win32 and later, HMODULE, HINSTANCE and HANDLE are same and is base addr of the module.
        HINSTANCE Handle;

        AutoLibrary(LPCWSTR fileName = NULL);
        ~AutoLibrary();

        void Load(LPCWSTR fileName);
    };

} // namespace JsDiag.
