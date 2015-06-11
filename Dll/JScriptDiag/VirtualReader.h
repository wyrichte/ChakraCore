//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#pragma once

namespace JsDiag
{
    //
    // Internal non-COM interface, represents reader from target process memory.
    //
    struct IVirtualReader
    {
        virtual HRESULT ReadVirtual(_In_ const void* addr, _Out_writes_bytes_(bufferSize) void* buffer, _In_ ULONG bufferSize, _Out_ PULONG bytesRead) = 0;
        virtual HRESULT ReadString(_In_z_ const void* addr, _Out_writes_z_(bufferElementCount) LPWSTR buffer, ULONG bufferElementCount) = 0;
        virtual HRESULT WriteMemory(_In_ const void* addr, _In_reads_bytes_(bufferSize) const void *buffer, ULONG bufferSize) = 0;
        virtual HRESULT AllocateVirtualMemory(_In_ UINT64 address, _In_ DWORD size, _In_ DWORD allocationType, _In_ DWORD pageProtection, _Out_ UINT64 *pAllocatedAddress) = 0;
        virtual HRESULT FreeVirtualMemory(_In_ UINT64 address, _In_ DWORD size, _In_ DWORD freeType) = 0;
        virtual ~IVirtualReader() {};
   };

    //
    // dbgeng-like IVirtualReader adapter
    //
    class VirtualReader sealed : public IVirtualReader
    {
        CComPtr<IStackProviderDataTarget> m_dataTarget;
        CComPtr<IJsDiagTestDataTarget> m_diagTestDataTarget;

    public:
        VirtualReader(IStackProviderDataTarget* dataTarget);

        // IVirtualReader::ReadVirtual
        virtual HRESULT ReadVirtual(_In_ const void* addr, _Out_writes_bytes_(bufferSize) void* buffer, _In_ ULONG bufferSize, _Out_ PULONG bytesRead) override;

        // IVirtualReader::ReadString.
        // Read a NULL terminated string from the debugging target's memory.
        virtual HRESULT ReadString(_In_z_ const void* addr, _Out_writes_z_(bufferElementCount) LPWSTR buffer, ULONG bufferElementCount) override;

        virtual HRESULT WriteMemory(_In_ const void* addr, _In_reads_bytes_(bufferSize) const void *buffer, ULONG bufferSize) override;

        virtual HRESULT AllocateVirtualMemory(_In_ UINT64 address, _In_ DWORD size, _In_ DWORD allocationType, _In_ DWORD pageProtection, _Out_ UINT64 *pAllocatedAddress) override;
        virtual HRESULT FreeVirtualMemory(_In_ UINT64 address, _In_ DWORD size, _In_ DWORD freeType) override;

        template <typename T> T ReadVirtual(const void* addr)
        {
            return ReadVirtual<T>(this, addr);
        }

        template <typename T>
        static T ReadVirtual(IVirtualReader* reader, const void* addr)
        {
            // Note: simply using "T data;" implies default ctor being available which may not be always the case.
            BYTE data[sizeof(T)];
            ULONG bytesRead;

            HRESULT hr = reader->ReadVirtual(addr, &data, sizeof(T), &bytesRead);
            CheckHR(hr, DiagErrorCode::READ_VIRTUAL);
            if (bytesRead != sizeof(T))
            {
                DiagException::Throw(E_UNEXPECTED, DiagErrorCode::READ_VIRTUAL_MISMATCH);
            }

            return *reinterpret_cast<T*>(data);
        }

        // Read a NULL terminated string from the debugging target's memory. If the actual string contains more
        // characters than bufferElementCount, fill buffer with a null-terminated truncated string and returns S_FALSE.
        //  addr                : The address of the string to read.
        //  buffer              : Receives the string from the target.
        //  bufferElementCount  : Specifies the size of buffer in characters (including null-terminator).
        template <typename ReadMemoryFunc>
        static HRESULT ReadString(const void* addr, _Out_writes_z_(bufferElementCount) LPWSTR buffer, ULONG bufferElementCount, ReadMemoryFunc readMemory)
        {
            Assert(addr);
            Assert(buffer);
            if (bufferElementCount < 1)
            {
                return E_INVALIDARG;    // Need space for at least one character.
            }

            const ULONG CHUNK_CHARS = 32; // Let's try to read by chunks and look for null-terminator
            LPCWSTR address = (LPCWSTR)addr;

            for (;;)
            {
                const ULONG requestChars = min(bufferElementCount, CHUNK_CHARS);

                ULONG bytesRead;
                HRESULT hr = readMemory((ULONG64)address, (BYTE*)buffer, requestChars * sizeof(WCHAR), &bytesRead);
                //Assert(SUCCEEDED(hr)); // Memory read failure is likely due to bad addr, could be bug or stack/memory corruption
                IfFailRet(hr);

                const ULONG charsRead = bytesRead / sizeof(WCHAR);

                if (wmemchr(buffer, NULL, charsRead))
                {
                    return S_OK; // found null-terminator
                }
                else if (charsRead == bufferElementCount)
                {
                    buffer[bufferElementCount - 1] = L'\0';
                    return S_FALSE;
                }

                Assert(charsRead < bufferElementCount);
                address += charsRead;
                buffer += charsRead;
                bufferElementCount -= charsRead;
            }
        }

        static BYTE* ReadBuffer(IVirtualReader* reader, const void* remoteAddr, const ULONG size);
    };

    class JsDebugVirtualReader sealed : public IVirtualReader
    {
        CComPtr<IJsDebugDataTarget> m_debugDataTarget;
    public:
        JsDebugVirtualReader(IJsDebugDataTarget* target) : m_debugDataTarget(target) {} 

        // IVirtualReader::ReadVirtual
        virtual HRESULT ReadVirtual(_In_ const void* addr, _Out_writes_bytes_(bufferSize) void* buffer, ULONG bufferSize, _Out_ PULONG bytesRead) override;

        // IVirtualReader::ReadString.
        // Read a NULL terminated string from the debugging target's memory.
        virtual HRESULT ReadString(_In_z_ const void* addr, _Out_writes_z_(bufferElementCount) LPWSTR buffer, ULONG bufferElementCount) override;

        virtual HRESULT WriteMemory(_In_ const void* addr, _In_reads_bytes_(bufferSize) const void *buffer, ULONG bufferSize) override;
        virtual HRESULT AllocateVirtualMemory(_In_ UINT64 address, _In_ DWORD size, _In_ DWORD allocationType, _In_ DWORD pageProtection, _Out_ UINT64 *pAllocatedAddress) override;
        virtual HRESULT FreeVirtualMemory(_In_ UINT64 address, _In_ DWORD size, _In_ DWORD freeType) override;
    };

} // namespace JsDiag.
