//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "EnginePch.h"

namespace Js
{
    void StreamHelper::IncPosition(ULONG cb)
    {
        scaposition_t bytes = static_cast<scaposition_t>(cb);
        if (static_cast<ULONG>(bytes) != cb)
        {
            ThrowOverflow();
        }

        m_position += bytes;

        if (m_position < bytes)
        {
            ThrowOverflow();
        }
    }
}