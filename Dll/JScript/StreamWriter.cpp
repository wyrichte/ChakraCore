//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "stdafx.h"

namespace Js
{
    void StreamWriter::RealWrite(const void* pv, ULONG cb)
    {
        ScriptContext* scriptContext = GetScriptContext();
        HRESULT hr = S_OK;

        ULONG cbWritten;
        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            hr = GetStream()->Write(pv, cb, &cbWritten);
        }
        END_LEAVE_SCRIPT(scriptContext);

        if (SUCCEEDED(hr))
        {
            IncPosition(cbWritten);
        }

        ThrowIfFailed(hr);
    }

    void StreamWriter::Write(const void* pv, ULONG cb)
    {
        ULONG bufferSpace = BUF_SIZE - m_cur;
        if (cb > bufferSpace)
        {
            // Flush existing buffer content to the stream
            Flush();
            bufferSpace = BUF_SIZE;

            // Direct write if request size is large
            if (cb >= DIRECT_WRITE_SIZE)
            {
                RealWrite(pv, cb);
                return;
            }
        }

        // Write to buffer
        Assert(m_cur + bufferSpace == BUF_SIZE);
        js_memcpy_s(m_buf + m_cur, bufferSpace, pv, cb);
        m_cur += cb;
    }

    //
    // Flush buffer content to the stream.
    //
    _Post_satisfies_(m_cur == 0)
    void StreamWriter::Flush()
    {
        if (m_cur != 0)
        {
            RealWrite(m_buf, m_cur);
            m_cur = 0;
        }
    }

    //
    // Overload to count for buffer position.
    //
    scaposition_t StreamWriter::GetPosition() const
    {
        // If this overflows, we will throw during Flush/RealWrite. So skip checking here.
        return __super::GetPosition() + static_cast<scaposition_t>(m_cur);
    }
}