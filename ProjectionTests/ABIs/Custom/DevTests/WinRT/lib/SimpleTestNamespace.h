//  Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdafx.h>
#include "DevTests.h"

namespace DevTests
{
    namespace SimpleTestNamespace
    {
        class SimpleClassServer :
            public Microsoft::WRL::RuntimeClass<ISimpleInterface,IEmptyInterface>
        {
            InspectableClass(L"DevTests.SimpleTestNamespace.SimpleClass", BaseTrust);

        private:
            HSTRING m_message;
            int m_value;

        public:
            SimpleClassServer() : m_value(50) {
               WindowsCreateString(L"Hello", 5, &m_message);
            }
            ~SimpleClassServer() {
                WindowsDeleteString(m_message);
            }

            IFACEMETHOD(SetMessage)(__in HSTRING message) {
                WindowsDeleteString(m_message);
                return WindowsDuplicateString(message, &m_message);
            }

            IFACEMETHOD(GetMessage)(__out HSTRING * message) {
                return WindowsDuplicateString(m_message, message);
            }

            IFACEMETHOD(get_Value)(__out int * value) {
                *value = m_value;
                return S_OK;
            }

            IFACEMETHOD(put_Value)(__in int value) {
                m_value = value;
                return S_OK;
            }
        };

    }
}