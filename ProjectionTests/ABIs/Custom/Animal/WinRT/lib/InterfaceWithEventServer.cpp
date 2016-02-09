#include "stdafx.h"

namespace Animals
{
    IFACEMETHODIMP CInterface2WithEventHandler::Invoke(__in IInterface2WithEvent *sender, __in HSTRING)
    {
        if (sender == nullptr)
        {
            return E_POINTER;
        }

        m_sender->m_nativeInvoked = true;
        if (m_sender != sender)
        {
            return E_INVALIDARG;
        }

        return S_OK;
    }
}
