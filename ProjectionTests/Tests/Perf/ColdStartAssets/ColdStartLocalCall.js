    runner.addTest({
        id: 1,
        desc: "Cold Start: Struct Method Call (Local Type)",
        test: function() {
            var obj = new Windows.RuntimeTest.Example.SampleTestObjectBothInProc();
            msWriteProfilerMark("JS_ColdStartLocalCall_Start");
            obj.structWithoutString = {
                doubleValue: 1,
                integerValue: 1,
                guidValue: "21EC2020-3AEA-1069-A2DD-08002B30309D"
            };
            msWriteProfilerMark("JS_ColdStartLocalCall_End");
        }
    });
