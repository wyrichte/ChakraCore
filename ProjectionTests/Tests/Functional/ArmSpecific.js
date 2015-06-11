if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
// ARM-specific unit tests
// These are tests for ARM calling convention boundary conditions.
// See also: ProjectionTests\ABIs\Custom\DevTests\WinRT\idl\DevTests.idl, ProjectionTests\ABIs\Custom\DevTests\WinRT\lib\Arm.h.

(function () {
    var testClass;
    runner.globalSetup(function () {
        testClass = new DevTests.Arm.Tests();
    });

    runner.addTest({
        id: 1,
        desc: 'simple case: 4 ints go to regs, 5th to the stack',
        pri: '0',
        test: function () {
            testClass.intX4(11, 22, 33, 44);
            verify(testClass.result, "11|22|33|44", "intX4");
        }
    });

    runner.addTest({
        id: 2,
        desc: 'int64 with alignment: r1 is a hole',
        pri: '0',
        test: function () {
            testClass.int64_Int(1234567890123, 101);
            verify(testClass.result, "1234567890123|101", "int64_Int");
        }
    });

    runner.addTest({
        id: 3,
        desc: 'int64, make sure we don"t split but leave r3 as hole',
        pri: '0',
        test: function () {
            testClass.int_Int_Int64(101, 102, 1234567890123);
            verify(testClass.result, "101|102|1234567890123", "int_Int_Int64");
        }
    });

    runner.addTest({
        id: 4,
        desc: 'float: simple case: #17 goes to the stack',
        pri: '0',
        test: function () {
            testClass.floatX17(1.01, 1.02, 1.03, 1.04, 1.05, 1.06, 1.07, 1.08, 1.09, 1.10, 1.11, 1.12, 1.13, 1.14, 1.15, 1.16, 1.17);
            verify(testClass.result,
                "1.010000|1.020000|1.030000|1.040000|1.050000|1.060000|1.070000|1.080000|1.090000|1.100000|1.110000|1.120000|1.130000|1.140000|1.150000|1.160000|1.170000",
                "floatX17");
        }
    });

    runner.addTest({
        id: 5,
        desc: 'double: simple case: #9 goes to the stack',
        pri: '0',
        test: function () {
            testClass.doubleX9(2.01, 2.02, 2.03, 2.04, 2.05, 2.06, 2.07, 2.08, 2.09);
            verify(testClass.result, "2.010000|2.020000|2.030000|2.040000|2.050000|2.060000|2.070000|2.080000|2.090000", "doubleX9");
        }
    });

    runner.addTest({
        id: 6,
        desc: 'mixed float/double: no stack: make sure holes are populated and no splits',
        pri: '0',
        test: function () {
            testClass.mixedFloatDoubleLeaveFillHole(1.00, 2.0203, 1.01, 1.04, 1.05, 1.06, 2.0809, 2.1011, 2.1213, 1.07, 1.14, 2.3);
            verify(testClass.result, 
                "1.000000|2.020300|1.010000|1.040000|1.050000|1.060000|2.080900|2.101100|2.121300|1.070000|1.140000|2.300000", 
                "mixedFloatDoubleLeaveFillHole");
        }
    });

    runner.addTest({
        id: 7,
        desc: 'simple mix of ints and floats',
        pri: '0',
        test: function () {
            testClass.mixedIntsAndFloats(1.00, 101, 2.0203, 10000000223, 1.01, 300);
            verify(testClass.result, "1.000000|101|2.020300|10000000223|1.010000|300", "mixedIntsAndFloats");
        }
    });

    runner.addTest({
        id: 8,
        desc: 'after float goes to stack, all ints and floats go to the stack',
        pri: '0',
        test: function () {
            testClass.afterFloatisOverflownToStackEverythingGoesToStack(2.00, 2.01, 2.02, 2.03, 2.04, 2.05, 2.06, 2.07, 2.0001, 202, 2.0405);
            verify(testClass.result,
                "2.000000|2.010000|2.020000|2.030000|2.040000|2.050000|2.060000|2.070000|2.000100|202|2.040500", 
                "afterFloatisOverflownToStackEverythingGoesToStack");
        }
    });

    runner.addTest({
        id: 9,
        desc: 'structs: test the alignment inside the struct',
        pri: '0',
        test: function () {
            testClass.structs_Int_Int64({ i0: 101, i1: 1234567890123 });
            verify(testClass.result, "101|1234567890123", "structs_Int_Int64");
        }
    });

    runner.addTest({
        id: 10,
        desc: 'structs: test the split -- 2nd int goes to the stack',
        pri: '0',
        test: function () {
            testClass.structs_Int_Int64_Int({ i0: 101, i1: 1234567890123, i2: 102 });
            verify(testClass.result, "101|1234567890123|102", "structs_Int_Int64_Int");
        }
    });

    runner.addTest({
        id: 11,
        desc: 'structs: if there is a float and non-float, it goes to general regs and not float regs',
        pri: '0',
        test: function () {
            testClass.structs_Int_Float({ i0: 101, f0: 1.01 });
            verify(testClass.result, "101|1.010000", "structs_Int_Float");
        }
    });

    runner.addTest({
        id: 12,
        desc: 'structs: Non-HFP: float and double -- must go to general regs',
        pri: '0',
        test: function () {
            testClass.structs_Float_Double({ f0: 1.01, d0: 2.01 });
            verify(testClass.result, "1.010000|2.010000", "structs_Float_Double");
        }
    });

    runner.addTest({
        id: 13,
        desc: 'structs: Non-HFP: inner struct, same type -- more than 4 args',
        pri: '0',
        test: function () {
            testClass.structs_DoubleX3_Inner_DoubleX2({ d0: 2.00, d1: 2.01, d2: 2.02, inner: { d0: 2.03, d1: 2.04} });
            verify(testClass.result, "2.000000|2.010000|2.020000|2.030000|2.040000", "structs_DoubleX3_Inner_DoubleX2");
        }
    });

    runner.addTest({
        id: 14,
        desc: 'structs: Non-HFP: inner struct, different type { float, double }',
        pri: '0',
        test: function () {
            testClass.structs_Float_Inner_Double({ f0: 1.00, inner: { d0: 2.00} });
            verify(testClass.result, "1.000000|2.000000", "structs_Float_Inner_Double");
        }
    });

    runner.addTest({
        id: 15,
        desc: 'structs: Non-HFP: deep inner struct, different type { double, { double, { double, { { float } } } } }',
        pri: '0',
        test: function () {
            testClass.structs_Double_Inner_Double_Inner_Double_Inner_Inner_Float({ d0: 2.0001, inner: { d0: 2.0203, inner: { d0: 2.0405, inner: { inner: { inner: { f0: 1.06}}}}} });
            verify(testClass.result, "2.000100|2.020300|2.040500|1.060000", "structs_Double_Inner_Double_Inner_Double_Inner_Inner_Float");
        }
    });

    runner.addTest({
        id: 16,
        desc: 'structs: HFP: 4 floats - regs { float, float, float, float } -- all must go to the regs',
        pri: '0',
        test: function () {
            testClass.structs_FloatX4({ f0: 1.1, f1: 1.2, f2: 1.3, f3: 1.4 });
            verify(testClass.result, "1.100000|1.200000|1.300000|1.400000", "structs_FloatX4");
        }
    });

    runner.addTest({
        id: 17,
        desc: 'structs: Non-HFP: 4+ floats - stack { float, float, float, float, float } -- all must go to the stack',
        pri: '0',
        test: function () {
            testClass.structs_FloatX5({ f0: 1.1, f1: 1.2, f2: 1.3, f3: 1.4, f4: 1.5 });
            verify(testClass.result, "1.100000|1.200000|1.300000|1.400000|1.500000", "structs_FloatX5");
        }
    });

    runner.addTest({
        id: 18,
        desc: 'structs: HFP: 4 doubles - regs { double, double, double, double } -- all must go to the regs',
        pri: '0',
        test: function () {
            testClass.structs_DoubleX4({ d0: 2.1, d1: 2.2, d2: 2.3, d3: 2.4 });
            verify(testClass.result, "2.100000|2.200000|2.300000|2.400000", "structs_DoubleX4");
        }
    });

    runner.addTest({
        id: 19,
        desc: 'structs: HFP: 4+ doubles - stack { double, double, double, double, double } -- all must go to the stack',
        pri: '0',
        test: function () {
            testClass.structs_DoubleX5({ d0: 2.1, d1: 2.2, d2: 2.3, d3: 2.4, d4: 2.5 });
            verify(testClass.result, "2.100000|2.200000|2.300000|2.400000|2.500000", "structs_DoubleX5");
        }
    });

    runner.addTest({
        id: 20,
        desc: 'structs: HFP: inner struct, same type { float, float, { float, float } }',
        pri: '0',
        test: function () {
            testClass.structs_FloatX2_Float_Inner_Float_Inner_Inner_Float({ f0: 1.00, f1: 1.01, inner: { f0: 1.02, inner: { inner: { inner: { f0: 1.03}}}} });
            verify(testClass.result, "1.000000|1.010000|1.020000|1.030000", "structs_FloatX2_Float_Inner_Float_Inner_Inner_Float");
        }
    });

    runner.addTest({
        id: 21,
        desc: 'structs: HFP: inner struct, same type { float, float, { float, { { float } } } }',
        pri: '0',
        test: function () {
            testClass.structs_Inner_X3_Double({ inner1: { d0: 2.00 }, inner2: { d0: 2.01 }, inner3: { d0: 2.02} });
            verify(testClass.result, "2.000000|2.010000|2.020000", "structs_Inner_X3_Double");
        }
    });

    runner.addTest({
        id: 22,
        desc: 'structs: HFP: 3 inner structs, same type { { double }, { double }, { double } }',
        pri: '0',
        test: function () {
            testClass.structs_Inner_Float_Pop_Float({ f0: 1.01 }, 1.02);
            verify(testClass.result, "1.010000|1.020000", "structs_Inner_Float_Pop_Float");
        }
    });

    runner.addTest({
        id: 23,
        desc: 'structs: HFP: float HFP and float { { float }, float }',
        pri: '0',
        test: function () {
            testClass.structs_Inner_Float_Pop_Float({ f0: 1.01 }, 1.02);
            verify(testClass.result, "1.010000|1.020000", "structs_Inner_Float_Pop_Float");
        }
    });

    runner.addTest({
        id: 24,
        desc: 'structs: double never split HFP { double, double, double, double, { double, double, double, double, double }}',
        pri: '0',
        test: function () {
            testClass.structs_DoubleX4_Inner_DoubleX5(2.00, 2.01, 2.02, 2.03, { d0: 2.04, d1: 2.05, d2: 2.06, d3: 2.07, d4: 2.08 });
            verify(testClass.result, "2.000000|2.010000|2.020000|2.030000|2.040000|2.050000|2.060000|2.070000", "structs_DoubleX4_Inner_DoubleX5");
        }
    });

    Loader42_FileName = "WinRT ARM-specific Marshaling tests";
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
