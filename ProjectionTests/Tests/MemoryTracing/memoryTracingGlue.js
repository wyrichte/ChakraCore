if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
var Loader42_FileName = "";
var runner = {
    allTests: [],
    addTest: function (runnerObj) {
        this.allTests.push(runnerObj);
    },
    run: function () {
        WScript.Echo(Loader42_FileName);
        var passed = 0;
        var failed = 0;

        for (var i = 0; i < this.allTests.length; i++) {
            var currentTest = this.allTests[i];
            WScript.Echo("\n=================================================");
            WScript.Echo("Starting [Test " + currentTest.id + ": " + currentTest.desc + "]");
            WScript.Echo("-------------------------------------------------");
            this.currentTestSucceeded = true;
            if (currentTest.preReq !== undefined && currentTest.preReq() !== true) {
                WScript.Echo("Test prereq returned false. Skipping");
            }
            else {
                try {
                    currentTest.test();
                }
                catch (e) {
                    WScript.Echo("    Excpetion: " + e.name + " - " + e.description);
                    runner.currentTestSucceeded = false;
                }
            }
            WScript.Echo("\n-------------------------------------------------");
            if (this.currentTestSucceeded) {
                WScript.Echo("PASS [Test " + currentTest.id + ": " + currentTest.desc + "]");
                passed++;
            }
            else {
                WScript.Echo("FAIL [Test " + currentTest.id + ": " + currentTest.desc + "]");
                failed++;
                currentTestSucceeded = true;
            }
            WScript.Echo("=================================================\n");
        }
        WScript.Echo("Passed: " + passed);
        WScript.Echo("Failed: " + failed);
    },
    currentTestSucceeded: true
};

var logger = {
    comment: function (msg) {
        WScript.Echo("Comment: " + msg);
    }
};

function verify(actualValue, expectedValue, valueMsg) {
    if (actualValue === expectedValue) {
        WScript.Echo("Pass: " + valueMsg);
    }
    else {
        WScript.Echo("Fail: " + valueMsg);
        WScript.Echo("    Expected: " + expectedValue);
        runner.currentTestSucceeded = false;
    }
    WScript.Echo("      Actual: " + actualValue + "\n");
}

verify.exception = function (callback, exception, msg) {
    try {
        var res = callback();
        verify.equal(res, "exception thrown", msg);
    } catch (e) {
        verify(e instanceof exception, true, msg + ":: " + e);
    }
};

function fail(msg) {
    WScript.Echo("Fail: " + msg);
    runner.currentTestSucceeded = false;
}

WScript.InitializeProjection();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
