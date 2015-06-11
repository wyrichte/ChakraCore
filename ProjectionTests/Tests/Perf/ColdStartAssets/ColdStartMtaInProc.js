    runner.addTest({
        id: 1,
        desc: "Cold Start: Construction: Mta In Proc",
        test: function() {
            msWriteProfilerMark("JS_ColdStartMtaInProc_Start");
            new Windows.RuntimeTest.Example.SampleTestObjectMtaInProc();
            msWriteProfilerMark("JS_ColdStartMtaInProc_End");
        }
    });
