    runner.addTest({
        id: 1,
        desc: "Cold Start: Struct Method Call (Foreign Type)",
        test: function() {
            var obj = new Windows.RuntimeTest.Example.SampleTestObjectBothInProc();
            msWriteProfilerMark("JS_ColdStartForeignCall_Start");
            obj.vectorOfStructWithoutString = [];
            msWriteProfilerMark("JS_ColdStartForeignCall_End");
        }
    });
