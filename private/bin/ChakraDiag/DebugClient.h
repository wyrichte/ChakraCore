//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#pragma once

namespace JsDiag
{
    struct LibraryName;
    struct RemoteStackFrameEnumerator;

#define CHAKRA_MODULE_NAME _u("chakra")

    //
    // Main entry point for debug/diag dealing with Chakra.dll internals.
    // Does not depend on backend technology (such as dbgeng, VS, etc).
    //
    class DebugClient
    {
    private:
        DiagProvider* m_diagProvider;           // We don't own diagProvider.
        UINT64 m_chakraBaseAddr;
        UINT64 m_chakraHighAddr;
        void* m_globals[Globals_Count];
        bool m_hasTargetHooks;

        static const unsigned int c_pointerSize = sizeof(void*);
        static const char16* c_chakraModuleName;
        static const char16* c_chakraFileName;
        static const char16* c_chakraTestModuleName;
        static const char16* c_chakraTestFileName;
        static const LibraryName c_librariesToTry[];

    public:
        DebugClient(DiagProvider* diagProvider);
        ~DebugClient();

    public:
        bool IsJsModuleAddress(void* address);

        template<class T>
        T GetGlobalValue(_In_range_(0, Globals_Max - 1) Globals valueId)
        {
            EnsureTargetHooks();
            Assert(valueId < Globals_Max);
            return VirtualReader::ReadVirtual<T>(m_diagProvider->GetReader(), m_globals[valueId]);
        }

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
        template <typename T> void LoadTargetAndExecute(T operation);
        void GetGlobals(const CStringA&, ULONG64 moduleBaseAddr);
    };

    struct LibraryName
    {
        const char16* ModuleName;
        const char16* FileName;
    };

} // namespace JsDiag.
