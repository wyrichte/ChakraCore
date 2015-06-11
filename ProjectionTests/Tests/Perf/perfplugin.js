(function() {
    var start, end;
    var trials = [];
    var perfData = {};

    var global = Function("return this")();
    global.CHAKRA_PROJECTIONS_ITERATION_COUNT = 10;
    
    // Facilitates laying out text on the console in a tabular format.
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

    // Default layout.
    var layout = new TableLayout(
        {width: 46},
        {width: 14},
        {width: 5, align: 'right'},
        {width: 7, align: 'right'},
        {width: 7, align: 'right'},
        {width: 10, align: 'right'}); 


    // Returns the total time taken by all trials.
    function getSum() {
        var sum = 0;

        for(var i = 0; i < trials.length; i++)
            sum += trials[i];

        return sum;
    }

    // Returns the standard deviation of all trials (where a trial is number of inner loops). 
    function getStandardDeviation() {
        var trialAverage = getSum() / trials.length;
        var sumOfSquares = 0;
        for(var i = 0; i < trials.length; i++) {
            sumOfSquares += Math.pow(trials[i] - trialAverage, 2);
        }

        return Math.sqrt(sumOfSquares / trials.length);
    }

    // Not used presently.
    function removeOutliers() {
        trials.sort(function(a, b) { return a-b });

        // Remove top 10%.
        for(var i = 0; i < trials.length/10; i++) {
            trials.pop();
        }
    }

    // Custom mark function, msWriteProfilerMark will call this.
    function mark(name) {
        if(name.match(/_Start$/)) {
            start = Date.now();
        } else {
            end = Date.now();
            trials.push(end-start);
        }
    }

    // print header.
    runner.subscribe('start', function() {
        WScript.Echo("");
        layout.header("Scenario", "Iters(O/I/T)", "Time", "Avg(I)", "StdD(I)", "Avg(ea)");
    });

    // Reset trials
    runner.subscribe('testStart', function(test) {
        trials = [];
    });

    runner.subscribe('testEnd', function(test) {
        var totalTime       = getSum();
        var innerIterations = parseInt(test.tags[0].split(" ")[1], 10);
        var outerIterations = trials.length;
        var totalIterations = innerIterations * outerIterations;
        var trialAverage    = totalTime / outerIterations
        var average         = totalTime / totalIterations;
        var stdDev          = getStandardDeviation();
        
        perfData[test.desc] = {
            id:              test.id,
            desc:            test.desc,
            innerIterations: innerIterations,
            outerIterations: outerIterations,
            totalIterations: totalIterations,
            totalTime:       totalTime,
            average:         average,
            trialAverage:    trialAverage,
            stdDev:          stdDev
        }

        layout.row(
            test.desc,
            outerIterations + "/" + innerIterations + "/" + totalIterations,
            totalTime.toFixed(0) + "ms",
            trialAverage.toFixed(1) + "ms",
            stdDev.toFixed(2) + "ms",
            (average * 1000).toFixed(3) + "us"
        );
    });

    // hook msWriteProfilerMark
    if(typeof global.msWriteProfilerMark !== "undefined")
        throw new Error("msWriteProfilerMark is present, and this is not expected.");
    global.msWriteProfilerMark = mark

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
}());
