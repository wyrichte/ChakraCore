//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

namespace JsDiag
{
    //
    // Represents a buffer in remote/target address space, which is cached locally,
    // and supports sequential access which is done via RemoteBuffer::Iterator.
    // To read, it uses IVirtualReader, although it is easy to extract out this aspect.
    // Example:
    //     RemoteBuffer<wchar_t> buf(m_reader, remoteStr, remoteStrSizeInBytes, /* cacheSize = */ 64);
    //     for (int i = 0; i < remoteStrLen; ++i) { buf.EnsurePtr(sizeof(wchar_t)); wprintf("%s", *buf.Ptr++); }
    //
    // Parameters:
    // T: the type of underlying elements in the remote buffer.
    //  
    template <typename T>
    class RemoteBuffer
    {
        IVirtualReader* m_reader;
        const BYTE* m_remoteAddr;
        const ULONG m_bufLenInBytes;
        ULONG m_totalBytesCopied;

        const ULONG m_cacheSize;
        ULONG m_bytesCached;
        AutoArrayPtr<BYTE> m_cache;
    public:
        const T* Ptr;

    public:
        RemoteBuffer(IVirtualReader* reader, void* remoteBuffer, ULONG maxLenInBytes, ULONG cacheSize);
        ULONG EnsurePtr(ULONG minBytes, bool isOkTohaveAtLeastOneByte = true);

    private:
        ULONG CopyFromRemote(BYTE* dst, ULONG bytesToCopy);
    }; // class RemoteBuffer.
} // namespace JsDiag.
