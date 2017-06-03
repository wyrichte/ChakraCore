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

// Check the property descriptor for var declaration
diagnosticsScript.debugEval("var x7 = 7", true);
compare(consoleScopeObj1.x7, 7);
var varDesc = Object.getOwnPropertyDescriptor(consoleScopeObj1, "x7");
compare(varDesc.configurable, true);
compare(varDesc.writable, true);
compare(varDesc.enumerable, true);

// Check the property descriptor for let declaration
diagnosticsScript.debugEval("let x8 = 8", true);
compare(consoleScopeObj1.x8, 8);
var letDesc = Object.getOwnPropertyDescriptor(consoleScopeObj1, "x8");
compare(letDesc.configurable, true);
compare(letDesc.writable, true);
compare(letDesc.enumerable, true);

// Check the property descriptor for implicit declaration
diagnosticsScript.debugEval("x9 = 9", true);
compare(consoleScopeObj1.x9, 9);
var desc = Object.getOwnPropertyDescriptor(consoleScopeObj1, "x9");
compare(desc.configurable, true);
compare(desc.writable, true);
compare(desc.enumerable, true);

// Check the property descriptor for const declaration
diagnosticsScript.debugEval("const x10 = 10", true);
compare(consoleScopeObj1.x10, 10);
var constDesc = Object.getOwnPropertyDescriptor(consoleScopeObj1, "x10");
compare(constDesc.configurable, true);
compare(constDesc.writable, false);
compare(constDesc.enumerable, true);

// Force the console object to be cross site and try to retrieve properties
var global2 = WScript.LoadScript("", "samethread", "diagnostics");
diagnosticsScript.debugEval("x11 = 11", true);
diagnosticsScript.debugEval.call(global2, "y11 = 111", true);
compare(global2.y11, undefined);
compare(global2.diagnosticsScript.getConsoleScope().y11, 111);
compare(global2.diagnosticsScript.getConsoleScope().x11, 11);

// Try creating let vars in a cross site console scope
var global3 = WScript.LoadScript("diagnosticsScript.getConsoleScope();", "samethread", "diagnostics");
diagnosticsScript.debugEval("let x12 = 12", true);
compare(consoleScopeObj1.x12, 12);
