(function() {
    var global = Function("return this")();
    var preCache = WScript.GetWorkingSet(); // pull this in as doing it first later has a negative ws impact.
    var workingSetBaseline;
    var workingSetAllocations;
    var workingSetSnapshot;
    var GARBAGE_COLLECTION_COUNT = 10;

    function collectAllGarbage() {
        for(var i = 0; i < GARBAGE_COLLECTION_COUNT; i++)
            CollectGarbage();
    }

    global.PerfPlugin = {
        workingSetBaseline: function() {
            collectAllGarbage();
            workingSetBaseline = WScript.GetWorkingSet();
        },

        workingSetSnapshot: function(allocations) {
            collectAllGarbage();
            workingSetAllocations = allocations;
            workingSetSnapshot = WScript.GetWorkingSet();
        }
    }

    function TableLayout() {
        var padding    = "  "; // 2 spaces
        var columns    = arguments;
        
        var alignments = {};
        alignments['left']      = leftAlign;
        alignments['undefined'] = leftAlign;
        alignments['right']     = rightAlign;
        alignments['center']    = centerAlign;
        
        function repeat(str, times) {
            return (new Array(times + 1).join(str));
        }

        function rightAlign(str, len) {
            var str = str.toString();
            return repeat(" ", len - str.length) + str;
        }

        function leftAlign(str, len) {
            var str = str.toString();
            return str + repeat(" ", len - str.length);
        }

        function centerAlign(str, len) {
            var str = str.toString();
            var padding = Math.floor((len - str.length) / 2);
            var remainder = (len - str.length) % 2;

            return repeat(" ", padding) + str + repeat(" ", padding + remainder);
        }

        this.header = function() {
            var cells = [];
            for(var i = 0; i < arguments.length; i++) {
                cells.push(leftAlign(arguments[i], columns[i].width));
            }

            var str = cells.join(padding);
            WScript.Echo(str);
            WScript.Echo(repeat("-", str.length));
        }

        this.row = function() {
            var cells = [];
            for(var i = 0; i < arguments.length; i++) {
                if(arguments[i].length > columns[i].width) {
                    cells.push(arguments[i].substring(0, columns[i].width))
                } else {
                    cells.push(alignments[columns[i].align](arguments[i], columns[i].width));
                }
            }

            var str = cells.join(padding);
            WScript.Echo(str);
        }
    }

    var layout = new TableLayout(
        {width: 55},
        {width: 13, align: 'right'},
        {width: 13, align: 'right'},
        {width: 12, align: 'right'}
    );

    // print header.
    runner.subscribe('start', function() {
        WScript.Echo("");
        layout.header("Scenario", "WS (kb)", "PU (kb)", "WS/Alloc");
    });

    // Reset trials
    runner.subscribe('testStart', function(test) {
        workingSetBaseline = null;
        workingSetAllocations = 0;
        workingSetSnapshot = null;
    });

    runner.subscribe('testEnd', function(test) {
        try {
        var ws = workingSetSnapshot.workingSet - workingSetBaseline.workingSet;
        var pu = workingSetSnapshot.privateUsage - workingSetBaseline.privateUsage;
        var allocs = '';
        var wsPerAlloc = '';

        if(workingSetAllocations > 0) {
            allocs = workingSetAllocations;
            wsPerAlloc = pu/allocs;
        }

        layout.row(
            test.desc,
            ws,
            pu,
            wsPerAlloc
        );
        } catch(e) {
            WScript.Echo("ERROR: " + e.description);
        }
    });


    // overwrite verify methods so they don't spam
    verify.equal = function(){};

    // overwrite WScript.Echo to prevent passed notifications and superfluous whitespace.
    // NOTE: This will have to change with any changes to the console logger.
    var originalWScriptEcho = WScript.Echo;
    var squelchNext = false;
    WScript.Echo = function(str) {
        if(squelchNext) {
            squelchNext = false;
        } else if(str.lastIndexOf("PASS", 0) === -1 && str.lastIndexOf("Passed: ", 0) === -1 && str.lastIndexOf("Failed: ", 0) === -1) {
            // dont' display no filename warning but don't squelch the next line.
            if(str !== "No filename given") {
                originalWScriptEcho(str);
            }
        } else {
            squelchNext = true;
        }
    }
})();
