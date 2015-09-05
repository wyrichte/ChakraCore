//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "EnginePch.h"

namespace Js
{
    ULONG StreamReader::RealRead(void* pv, ULONG cb)
    {
        ScriptContext* scriptContext = GetScriptContext();
        HRESULT hr = S_OK;

        ULONG cbRead;
        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            hr = GetStream()->Read(pv, cb, &cbRead);
        }
        END_LEAVE_SCRIPT(scriptContext);

        ThrowIfFailed(hr);

        IncPosition(cbRead);
        return cbRead;
    }

    void StreamReader::Read(void* pv, ULONG cb)
    {
        ULONG bytesInBuffer = GetBytesInBuffer();
        if (cb > bytesInBuffer)
        {
            // Drain bytes already in buffer
            if (bytesInBuffer > 0)
            {
                js_memcpy_s(pv, cb, m_buf + m_cur, bytesInBuffer);
                pv = static_cast<BYTE*>(pv) + bytesInBuffer;
                cb -= bytesInBuffer;
            }
            m_count = 0;
            m_cur = 0;

            // Direct read if request size is large
            if (cb >= DIRECT_READ_SIZE)
            {
                ULONG cbRead = RealRead(pv, cb);
                if (cbRead != cb)
                {
                    ThrowSCADataCorrupt();
                }
                return;
            }

            // Restock buffer
            m_count = RealRead(m_buf, BUF_SIZE);
            if (cb > m_count)
            {
                ThrowSCADataCorrupt();
            }
        }

        // Read from buffer
        js_memcpy_s(pv, cb, m_buf + m_cur, cb);
        m_cur += cb;
    }

    //
    // Overload to count for buffer position.
    //
    scaposition_t StreamReader::GetPosition() const
    {
        return __super::GetPosition() - static_cast<scaposition_t>(GetBytesInBuffer());
    }
}