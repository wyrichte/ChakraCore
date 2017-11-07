//    Copyright (c) Microsoft Corporation.  All rights reserved.

#include <WexTestClass.h>

namespace WEX 
{ 
    namespace TestExecution 
    { 
        namespace WineryTests
        {
            class NapaWineryTests
                : public WEX::TestClass<NapaWineryTests>
            {
            public:

                // Declare this class as a TestClass, and supply metadata if necessary.
                BEGIN_TEST_CLASS(NapaWineryTests)
                    TEST_CLASS_PROPERTY(L"TestClassification", L"E2E")
                    TEST_CLASS_PROPERTY(L"Priority", L"1")
                    TEST_CLASS_PROPERTY(L"Description", L"Test the component.")
                    TEST_CLASS_PROPERTY(L"BinaryUnderTest", L"WineryServer.dll")
                END_TEST_CLASS()

				TEST_METHOD(TestStruct);
				TEST_METHOD(TestString);
				TEST_METHOD(TestEvent);
				TEST_METHOD(TestDelegate);
				TEST_METHOD(TestParameterizedInterface);
				TEST_METHOD(TestActivation);
				TEST_METHOD(TestMap);

            private:
            };
        }
    }
}