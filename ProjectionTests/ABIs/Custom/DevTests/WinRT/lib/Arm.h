//  Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdafx.h>
#include <Windows.Foundation.h>
#include <strsafe.h>

using namespace Windows::Foundation;

namespace DevTests
{
    namespace Arm
    {
        struct AutoWStr
        {
            wchar_t* m_buf;

            // charCount: number of char not including null-terminator.
            AutoWStr(int charCount, wchar_t* format, va_list args) 
            { 
                int bufSize = 2 * (charCount + 1);
                m_buf = new wchar_t[bufSize];
                StringCchVPrintfW(m_buf, bufSize/sizeof(WCHAR), format, args);
            }

            ~AutoWStr() { delete[] m_buf; }

            operator wchar_t*() { return m_buf; }
        };

        class ArmTestsServer :
            public Microsoft::WRL::RuntimeClass<IJsToWinRTTests>
        {
            InspectableClass(L"DevTests.Arm.Tests", BaseTrust);

        private:
            HSTRING m_result;

        public:
            ArmTestsServer() : m_result(NULL)
            {
            }

            ~ArmTestsServer()
            {
                ReleaseResult();
            }

            IFACEMETHOD(get_Result)(__out HSTRING* result)
            {
                return WindowsDuplicateString(m_result, result);
            }

            // 01: simple case: 4 ints go to regs, 5th to the stack.
            IFACEMETHOD(IntX4)(__in int r1, __in int r2, __in int r3, __in int s0)
            {
                UpdateResult(4 * MaxIntCharCount + 3, L"%d|%d|%d|%d", r1, r2, r3, s0);  // 3 is for delimiters '|'.
                return S_OK;
            }

            // 02: int64 with alignment: r1 is a hole.
            IFACEMETHOD(Int64_Int)(__in __int64 r2r3, __in int s0)
            {
                UpdateResult(MaxIntCharCount + MaxInt64CharCount + 1, L"%lld|%d", r2r3, s0);
                return S_OK;
            }

            // 03: int64, make sure we don't split but leave r3 as hole.
            IFACEMETHOD(Int_Int_Int64)(__in int r1, __in int r2, __int64 s0)
            {
                UpdateResult(2 * MaxIntCharCount + MaxInt64CharCount + 2, L"%d|%d|%lld", r1, r2, s0);
                return S_OK;
            }

            // 04: float: simple case: #17 goes to the stack
            IFACEMETHOD(FloatX17)(__in float f0, __in float f1, __in float f2, __in float f3, 
                                  __in float f4, __in float f5, __in float f6, __in float f7, 
                                  __in float f8, __in float f9, __in float fa, __in float fb, 
                                  __in float fc, __in float fd, __in float fe, __in float ff, 
                                  __in float s0)
            {
                UpdateResult(17 * MaxFloatCharCount + 16, L"%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f", 
                    f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, fa, fb, fc, fd, fe, ff, s0);
                return S_OK;
            }

            // 05: double: simple case: #9 goes to the stack
            IFACEMETHOD(DoubleX9)(__in double d0, __in double d1, __in double d2, __in double d3, 
                                  __in double d4, __in double d5, __in double d6, __in double d7, 
                                  __in double s0)
            {
                UpdateResult(9 * MaxDoubleCharCount + 8, L"%llf|%llf|%llf|%llf|%llf|%llf|%llf|%llf|%llf", 
                    d0, d1, d2, d3, d4, d5, d6, d7, s0);
                return S_OK;
            }

            // 06: mixed float/double: no stack: make sure holes are populated and no splits
            IFACEMETHOD(MixedFloatDoubleLeaveFillHole)(
                __in float f0,  __in double f2f3, __in float f1, // 4 float slots
                __in float f4, __in float f5, __in float f6,     // f7 = hole hole would be filled later
                __in double f8f9, __in double f10f11, 
                __in double f12f13, __in float f7, __in float f14, double s0) // last double would go to the stack, leaving f15 = hole
            {
                UpdateResult(8 * MaxFloatCharCount + 3 * MaxDoubleCharCount + 12, 
                    L"%f|%llf|%f|%f|%f|%f|%llf|%llf|%llf|%f|%f|%llf", 
                    f0, f2f3, f1, f4, f5, f6, f8f9, f10f11, f12f13, f7, f14, s0);
                return S_OK;
            }

            // 07: simple mix of ints and floats
            IFACEMETHOD(MixedIntsAndFloats)(__in float f0, __in int i1, __in double f2f3, __in __int64 i2i3, __in float f1, __in int s0)
            {
                UpdateResult(2 * MaxFloatCharCount + MaxDoubleCharCount + MaxIntCharCount + MaxInt64CharCount + 5, 
                    L"%f|%d|%llf|%lld|%f|%d", f0, i1, f2f3, i2i3, f1, s0);
                return S_OK;
            }

            // 08: after float goes to stack, all ints and floats go to the stack
            IFACEMETHOD(AfterFloatisOverflownToStackEverythingGoesToStack)(
                __in double d0, __in double d1, __in double d2, __in double d3, 
                __in double d4, __in double d5, __in double d6, __in double d7, // Everything else after that goes to the stack.
                __in double s0s1, __in int s2, __in double s4s5)
            {
                UpdateResult(18 * MaxDoubleCharCount + MaxIntCharCount + 10, L"%llf|%llf|%llf|%llf|%llf|%llf|%llf|%llf|%llf|%d|%llf", 
                    d0, d1, d2, d3, d4, d5, d6, d7, s0s1, s2, s4s5);
                return S_OK;
            }

            // 09: structs: test the alignment inside the struct
            IFACEMETHOD(Structs_Int_Int64)(Struct_Int_Int64 data)
            {
                UpdateResult(MaxIntCharCount + MaxInt64CharCount + 1, L"%d|%lld", data.i0, data.i1);
                return S_OK;
            }

            // 10: structs: test the split -- 2nd int goes to the stack
            IFACEMETHOD(Structs_Int_Int64_Int)(Struct_Int_Int64_Int data)
            {
                UpdateResult(2 * MaxIntCharCount + MaxInt64CharCount + 2, L"%d|%lld|%d", data.i0, data.i1, data.i2);
                return S_OK;
            }

            // 11: structs: if there is a float and non-float, it goes to general regs and not float regs
            IFACEMETHOD(Structs_Int_Float)(Struct_Int_Float data)
            {
                UpdateResult(MaxIntCharCount + MaxFloatCharCount + 1, L"%d|%f", data.i0, data.f0);
                return S_OK;
            }
            
            // 12: structs: Non-HFP: float and double -- must go to general regs
            IFACEMETHOD(Structs_Float_Double)(Struct_Float_Double data)
            {
                UpdateResult(2 * MaxDoubleCharCount + 1, L"%f|%llf", data.f0, data.d0);
                return S_OK;
            }

            // 13: structs: Non-HFP: inner struct, same type -- more than 4 args
            IFACEMETHOD(Structs_DoubleX3_Inner_DoubleX2)(Struct_DoubleX3_Inner_DoubleX2 data)
            {
                UpdateResult(5 * MaxDoubleCharCount + 4, L"%llf|%llf|%llf|%llf|%llf", data.d0, data.d1, data.d2, data.inner.d0, data.inner.d1);
                return S_OK;
            }

            // 14: structs: Non-HFP: inner struct, different type
            IFACEMETHOD(Structs_Float_Inner_Double)(Struct_Float_Inner_Double data)
            {
                UpdateResult(MaxFloatCharCount + MaxDoubleCharCount + 1, L"%f|%llf", data.f0, data.inner.d0);
                return S_OK;
            }

            // 15: structs: Non-HFP: deep inner struct, different type
            //  { double, { double, { double, { { float } } } } }
            IFACEMETHOD(Structs_Double_Inner_Double_Inner_Double_Inner_Inner_Float)(Struct_Double_Inner_Double_Inner_Double_Inner_Inner_Float data)
            {
                UpdateResult(MaxFloatCharCount + 3 * MaxDoubleCharCount + 3, L"%llf|%llf|%llf|%f", 
                    data.d0, data.inner.d0, data.inner.inner.d0, data.inner.inner.inner.inner.inner.f0);
                return S_OK;
            }

            // 16: structs: HFP: 4 floats - regs { float, float, float, float } -- all must go to the regs
            IFACEMETHOD(Structs_FloatX4)(Struct_FloatX4 data)
            {
                UpdateResult(4 * MaxFloatCharCount + 3, L"%f|%f|%f|%f", data.f0, data.f1, data.f2, data.f3);
                return S_OK;
            }

            // 17: structs: Non-HFP: 4+ floats - stack { float, float, float, float, float } -- all must go to the stack
            IFACEMETHOD(Structs_FloatX5)(Struct_FloatX5 data)
            {
                UpdateResult(5 * MaxFloatCharCount + 3, L"%f|%f|%f|%f|%f", data.f0, data.f1, data.f2, data.f3, data.f4);
                return S_OK;
            }

            // 18: structs: HFP: 4 doubles - regs { double, double, double, double } -- all must go to the regs
            IFACEMETHOD(Structs_DoubleX4)(Struct_DoubleX4 data)
            {
                UpdateResult(4 * MaxDoubleCharCount + 3, L"%llf|%llf|%llf|%llf", data.d0, data.d1, data.d2, data.d3);
                return S_OK;
            }

            // 19: structs: HFP: 4+ doubles - stack { double, double, double, double, double } -- all must go to the stack
            IFACEMETHOD(Structs_DoubleX5)(Struct_DoubleX5 data)
            {
                UpdateResult(5 * MaxDoubleCharCount + 4, L"%llf|%llf|%llf|%llf|%llf", data.d0, data.d1, data.d2, data.d3, data.d4);
                return S_OK;
            }

            // 20: structs: HFP: inner struct, same type { float, float, { float, float } }
            IFACEMETHOD(Structs_FloatX2_Inner_FloatX2)(Struct_FloatX2_Inner_FloatX2 data)
            {
                UpdateResult(4 * MaxFloatCharCount + 3, L"%f|%f|%f|%f", data.f0, data.f1, data.inner.f0, data.inner.f1);
                return S_OK;
            }

            // 21: structs: HFP: inner struct, same type { float, float, { float, { { float } } } }
            IFACEMETHOD(Structs_FloatX2_Float_Inner_Float_Inner_Inner_Float)(Struct_FloatX2_Float_Inner_Float_Inner_Inner_Float data)
            {
                UpdateResult(4 * MaxFloatCharCount + 3, L"%f|%f|%f|%f", data.f0, data.f1, data.inner.f0, data.inner.inner.inner.inner.f0);
                return S_OK;
            }

            // 22: structs: HFP: 3 inner structs, same type { { double }, { double }, { double } }
            IFACEMETHOD(Structs_Inner_X3_Double)(Struct_Inner_X3_Double data)
            {
                UpdateResult(3 * MaxDoubleCharCount + 3, L"%llf|%llf|%llf", data.inner1.d0, data.inner2.d0, data.inner3.d0);
                return S_OK;
            }

            // 23: structs: HFP: float HFP and float: { float }, float
            IFACEMETHOD(Structs_Inner_Float_Pop_Float)(Struct_Float data, float f0)
            {
                UpdateResult(2 * MaxFloatCharCount + 1, L"%f|%f", data.f0, f0);
                return S_OK;
            }

            // 24: structs: double never split HFP { double, double, double, double, { double, double, double, double, double }}
            IFACEMETHOD(Structs_DoubleX4_Inner_DoubleX5)(double d0, double d1, double d2, double d3, Struct_DoubleX5 s0)
            {
                UpdateResult(9 * MaxDoubleCharCount + 8, L"%llf|%llf|%llf|%llf|%llf|%llf|%llf|%llf", 
                    d0, d1, d2, d3, s0.d0, s0.d1, s0.d2, s0.d3, s0.d4);
                return S_OK;
            }

        private:
            // charCount: number of char not including null-terminator.
            void UpdateResult(int charCount, wchar_t* format, ...)
            {
                va_list args;
                va_start(args, format);
                AutoWStr str(charCount, format, args);

                ReleaseResult();
                WindowsCreateString(str, (UINT32)wcslen(str), &m_result);

                va_end(args);
            }

            void ReleaseResult()
            {
                if (m_result != NULL)
                {
                    WindowsDeleteString(m_result);
                }
            }

            // Note: for 1 byte we need 2 hexadecimal digits, for dec it would be more than enough if we take 2 times more digits.
            static const int MaxIntCharCount = 10 + 1;   // int_max = 2147483647, 1 digit for potential sign.
            static const int MaxInt64CharCount = 19 + 1; // int64_max = 9223372036854775807, 1 digit for potential sign.
            static const int MaxFloatCharCount = 25;
            static const int MaxDoubleCharCount = 25;
        };
    }
}
