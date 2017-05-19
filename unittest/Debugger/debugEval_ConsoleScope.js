// jshost -DiagnosticsEngine
function compare(x, y) {
    if ( x === y) {
        print("PASSED");
    } else {
        print("FAILED");
    }
}

// Declare a variable and verify it can be retrieved from the console scope
diagnosticsScript.debugEval("var x1 = 1", true);
compare(diagnosticsScript.getConsoleScope().x1, 1);
diagnosticsScript.debugEval("var x2 = 2", false);
compare(diagnosticsScript.getConsoleScope().x2, 2);

// Declare a let var and verify it can be retrieved from the console scope
diagnosticsScript.debugEval("let x3 = 3", true);
compare(diagnosticsScript.getConsoleScope().x3, 3);

// Do an implicit declaration and verify it can be retrieved from the console scope
diagnosticsScript.debugEval("x4 = 4", true);
compare(diagnosticsScript.getConsoleScope().x4, 4);

// Add a variable to the console scope through implicit declaration and delete it
var consoleScopeObj1 = diagnosticsScript.getConsoleScope();
diagnosticsScript.debugEval("x5 = 5", true);
compare(consoleScopeObj1.x5, 5);
diagnosticsScript.debugEval("delete x5", true);
compare(consoleScopeObj1.x5, undefined);

// Add a var directly to the console scope object and verify it is visible through debugEval
var consoleScopeObj2 = diagnosticsScript.getConsoleScope();
consoleScopeObj2.x6 = 6;
compare(diagnosticsScript.debugEval("x6", true), 6);

// Compare the console scope objects and make sure they are the same
compare(consoleScopeObj1, consoleScopeObj2);
