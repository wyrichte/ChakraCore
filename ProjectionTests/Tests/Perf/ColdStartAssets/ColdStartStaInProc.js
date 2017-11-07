    runner.addTest({
        id: 1,
        desc: "Cold Start: Construction: Sta In Proc",
        test: function() {
            msWriteProfilerMark("JS_ColdStartStaInProc_Start");
            new Windows.RuntimeTest.Example.SampleTestObjectStaInProc();
            msWriteProfilerMark("JS_ColdStartStaInProc_End");
        }
    });

