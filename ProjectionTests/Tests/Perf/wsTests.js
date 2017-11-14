(function() {
    var counter = 1;

    [5,6,7,8,9,10,11,12,13,15,16,17].forEach(function(power) {
        for(var count = 0; count < 4; count++) {
            runner.addTest({
                id: counter++,
                desc: "WinRT RTC object overhead - " + Math.pow(2, power) + " allocations",
                tags: ["working set"],
                test: function() {
                    var arr = new Array(Math.pow(2, power));

                    for(var i = 0; i < Math.pow(2, power); i++) {
                        if(i === 15)
                            PerfPlugin.workingSetBaseline();
                        arr[i] = new Animals.SimplestClass();
                    }

                    PerfPlugin.workingSetSnapshot(Math.pow(2, power));
                }
            });
        }
    });
})();

