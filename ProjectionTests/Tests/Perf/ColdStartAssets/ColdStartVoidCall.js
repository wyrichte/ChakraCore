    runner.addTest({
        id: 1,
        desc: "Cold Start: Void Arguments Call",
        test: function() {
            var obj = new Windows.RuntimeTest.Example.SampleTestObject2();
            msWriteProfilerMark("JS_ColdStartVoidCall_Start");
            obj.voidArgumentsCall();
            msWriteProfilerMark("JS_ColdStartVoidCall_End");
        }
    });
