//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//---------------------------------------------------------------------------
#include "stdafx.h"

namespace JsDiag
{
    VirtualReader::VirtualReader(IStackProviderDataTarget* dataTarget):
        m_dataTarget(dataTarget)
    {
        dataTarget->QueryInterface(&m_diagTestDataTarget);
        //
        // Ignore QI failure. WER doesn't provide private IJsDiagTestDataTarget interface.
        //
    }

    // IVirtualReader::ReadVirtual
    HRESULT VirtualReader::ReadVirtual(_In_ const void* addr, _Out_writes_bytes_(bufferSize) void* buffer, _In_ ULONG bufferSize, _Out_ PULONG bytesRead)
    {
        HRESULT hr = m_dataTarget->ReadVirtual((ULONG64)(addr), (PBYTE)buffer, bufferSize, bytesRead);
        //Assert(SUCCEEDED(hr)); // Memory read failure is likely due to bad addr, could be bug or stack/memory corruption
        return hr;
    }

    /*static*/
    BYTE* VirtualReader::ReadBuffer(IVirtualReader* reader, const void* remoteAddr, const ULONG size)
    {
        AutoArrayPtr<BYTE> localBuffer(new(oomthrow) BYTE[size]);

        ULONG bytesRead;

        HRESULT hr = reader->ReadVirtual(remoteAddr, localBuffer, size, &bytesRead);
        CheckHR(hr, DiagErrorCode::READ_VIRTUAL);
        if (bytesRead != size)
        {
            DiagException::Throw(E_UNEXPECTED, DiagErrorCode::READ_VIRTUAL_MISMATCH);
        }
        return localBuffer.Detach();
    }

    // Read a NULL terminated string from the debugging target's memory. If the actual string contains
    // more characters than bufferSize, fill buffer with a null-terminated truncated string and returns
    // S_FALSE.
    //  addr                : The address of the string to read.
    //  buffer              : Receives the string from the target.
    //  bufferElementCount  : Specifies the size of buffer in characters.
    HRESULT VirtualReader::ReadString(_In_z_ const void* addr, _Out_writes_z_(bufferElementCount) LPWSTR buffer, ULONG bufferElementCount)
    {
        return ReadString(addr, buffer, bufferElementCount,
            [this](_In_ ULONG64 address, _Out_writes_bytes_to_(bufferSize, *bytesRead) PBYTE buffer, _In_ ULONG bufferSize, _Out_opt_ PULONG bytesRead) -> HRESULT
        {
            HRESULT hr = m_dataTarget->ReadVirtual(address, buffer, bufferSize, bytesRead);
            
            if (hr == HRESULT_FROM_WIN32(ERROR_PARTIAL_COPY)) // Only part of a ReadProcessMemory or WriteProcessMemory request was completed
            {
                hr = S_FALSE; // ReadString allows partial read
            }

            return hr;
        });
    }
} // namespace JsDiag.
