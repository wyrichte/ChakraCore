runner.addTest({
    id: 1,
    desc: "Cold Start: Construction: Both In Proc",
    test: function() {
        msWriteProfilerMark("JS_ColdStartBothInProc_Start");
        new Windows.RuntimeTest.Example.SampleTestObjectBothInProc();
        msWriteProfilerMark("JS_ColdStartBothInProc_End");
    }
});
