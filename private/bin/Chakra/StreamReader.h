//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once
namespace Js
{
    //
    // A simple stream reader that hides low level stream details.
    //
    class StreamReader: public StreamHelper
    {
    private:
        static const ULONG BUF_SIZE = 1024;
        static const ULONG DIRECT_READ_SIZE = 32;

        BYTE m_buf[BUF_SIZE];                       // A buffer used by the stream reader
        _Field_range_(0, BUF_SIZE) ULONG m_count;   // Current content length in the buffer
        _Field_range_(0, m_count) ULONG m_cur;      // Current read position in the buffer content

        //
        // Get number of bytes left for read in the buffer.
        //
        ULONG GetBytesInBuffer() const
        {
            return m_count - m_cur;
        }

        ULONG RealRead(void* pv, ULONG cb);

    public:
        StreamReader(ScriptContext* scriptContext, IStream* pInStream)
            : StreamHelper(scriptContext, pInStream),
            m_count(0),
            m_cur(0)
        {
        }

        void Read(void* pv, ULONG cb);

        template <typename T>
        void Read(T* value)
        {
            if (GetBytesInBuffer() >= sizeof(T))
            {
                *value = *(T*)(m_buf + m_cur);
                m_cur += sizeof(T);
            }
            else
            {
                Read(value, sizeof(T));
            }
        }

        scaposition_t GetPosition() const;
    };

}