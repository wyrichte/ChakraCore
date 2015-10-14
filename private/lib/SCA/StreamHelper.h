//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once
namespace Js
{
    //
    // Helper class to implement stream reader/writer. Note that this stream helper class
    // maintains its own stream position and ensures the stream position fits scaposition_t.
    //
    class StreamHelper: public ScriptContextHolder
    {
    private:
        CComPtr<IStream> m_pStream;
        scaposition_t m_position;

    protected:
        IStream* GetStream() const
        {
            return m_pStream;
        }

        void ThrowOverflow() const
        {
            ::Math::DefaultOverflowPolicy();
        }

        void IncPosition(ULONG cb);

    public:
        StreamHelper(ScriptContext* scriptContext, IStream* pStream)
            : ScriptContextHolder(scriptContext),
            m_pStream(pStream),
            m_position(0)
        {
        }

        scaposition_t GetPosition() const
        {
            return m_position;
        }
    };
}