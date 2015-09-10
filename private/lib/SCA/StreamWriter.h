//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once
namespace Js
{
    //
    // A simple stream writer that hides low level stream details.
    //
    // Note:
    //      This stream writer uses an internal buffer. Must call Flush() at the end to ensure
    //      any remained content in the internal buffer is sent to the output stream.
    //
    class StreamWriter: public StreamHelper
    {
    private:
        static const ULONG BUF_SIZE = 1024;
        static const ULONG DIRECT_WRITE_SIZE = 32;

        BYTE m_buf[BUF_SIZE];                   // A buffer used by the stream writer
        _Field_range_(0, BUF_SIZE) ULONG m_cur; // Current write position in the buffer

        void RealWrite(const void* pv, ULONG cb);

    public:
        StreamWriter(ScriptContext* scriptContext, IStream* pOutStream)
            : StreamHelper(scriptContext, pOutStream),
            m_cur(0)
        {
        }

        void Write(const void* pv, ULONG cb);

        template <typename T>
        void Write(const T& value)
        {
            C_ASSERT(BUF_SIZE >= sizeof(T));

            if (m_cur <= BUF_SIZE - sizeof(T))
            {
                *(T*)(m_buf + m_cur) = value;
                m_cur += sizeof(T);
            }
            else
            {
                Write(&value, sizeof(T));
            }
        }

        _Post_satisfies_(m_cur == 0)
        void Flush();

        scaposition_t GetPosition() const;
    };
}