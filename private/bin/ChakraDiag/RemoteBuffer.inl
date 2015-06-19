//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

namespace JsDiag
{
    template <typename T>
    inline RemoteBuffer<T>::RemoteBuffer(IVirtualReader* reader, void* remoteBuffer, ULONG maxLenInBytes, ULONG cacheSize) :
        m_reader(reader), m_remoteAddr(reinterpret_cast<const BYTE*>(remoteBuffer)),
        m_bufLenInBytes(maxLenInBytes), m_totalBytesCopied(0),
        m_cacheSize(cacheSize), m_bytesCached(0), m_cache(nullptr)
    {
        Assert(reader);

        m_cache = new(oomthrow) BYTE[m_cacheSize];
        m_bytesCached = this->CopyFromRemote(m_cache, m_cacheSize);
        this->Ptr = reinterpret_cast<T*>(static_cast<BYTE*>(m_cache));
    }

    // Notes:
    //   Always assume that at least one byte is required. If not, we throw.
    //   Is isOkTohaveAtLeastOneByte is true, we don't throw in the case when minBytes can't be satisfied but there is at least one byte still available.
    // Returns the number of bytes available for read from the cache.
    template <typename T>
    inline ULONG RemoteBuffer<T>::EnsurePtr(ULONG minBytes, bool isOkTohaveAtLeastOneByte /* = true */)
    {
        const BYTE* p = reinterpret_cast<const BYTE*>(this->Ptr);
        if (p < m_cache || p > m_cache + m_bytesCached) // We should never move Ptr out of range
        {
            Assert(false);
            DiagException::Throw(E_UNEXPECTED, DiagErrorCode::REMOTEBUFFER_PTR_OUTOFBOUND);
        }
        
        // Since m_bytesCached is a ULONG, above checked that p - m_cache is in range for ULONG
        const ULONG usedBytes = static_cast<ULONG>(p - m_cache);
        const ULONG unusedBytes = m_bytesCached - usedBytes;
        Assert(usedBytes <= m_bytesCached);
        Assert(unusedBytes <= m_bytesCached);
        Assert(m_bytesCached <= m_cacheSize);

        ULONG availableBytes = unusedBytes;
        if (minBytes > availableBytes)
        {
            // Copy ununsed bytes in current cache to the beginning of cache, they are both in local address space.
            // Note: what's in cache up to cacheSize never exceeds m_bufLenInBytes.
            memmove(m_cache, p, unusedBytes);

            // Now copy to fill the rest of cache.
            const ULONG bytesCopied = this->CopyFromRemote(m_cache + unusedBytes, m_cacheSize - unusedBytes);
            availableBytes = unusedBytes + bytesCopied;
            if (availableBytes == 0 ||
                availableBytes < minBytes && !isOkTohaveAtLeastOneByte)
            {
                DiagException::Throw(E_UNEXPECTED, DiagErrorCode::REMOTEBUFFER_NO_MORE_BYTES);
            }
            m_bytesCached = availableBytes;

            // Reassign the ptr.
            this->Ptr = reinterpret_cast<T*>(static_cast<BYTE*>(m_cache));
        }

        return availableBytes;
    }
    
    // Copies requested number of bytes from m_remoteBuffer, 
    // unless it's greater than m_bufLenInBytes in which case it copies just valid amount of bytes.
    // Adjusts m_totalBytesCopied.
    // Returns the number of actual bytes copied.
    template <typename T>
    inline ULONG RemoteBuffer<T>::CopyFromRemote(BYTE* dst, ULONG bytesToCopy)
    {
        Assert(m_totalBytesCopied <= m_bufLenInBytes);
        if (bytesToCopy > m_bufLenInBytes - m_totalBytesCopied)
        {
            bytesToCopy = m_bufLenInBytes - m_totalBytesCopied; // Limit to valid range
        }

        if (bytesToCopy > 0)
        {
            ULONG bytesRead = 0;
            HRESULT hr = m_reader->ReadVirtual(m_remoteAddr + m_totalBytesCopied, dst, bytesToCopy, &bytesRead);
            CheckHR(hr, DiagErrorCode::READ_VIRTUAL); // bytesToCopy is limited to valid range and read should succeed. But we could always fail to read memory.
            Assert(bytesRead <= bytesToCopy);

            m_totalBytesCopied += bytesRead;
            return bytesRead;
        }

        return 0;
    }

} // namespace JsDiag.
