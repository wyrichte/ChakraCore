var test3 = WScript.LoadScriptFile("bug_760904_child2.js", "samethread");

function ConsoleLog1() {
    // Only the exception, but should not dispatched to the debugger.
    abc.def = 10;
}

function ConsoleLog2() {
    // This will be dispatched to the debugger.
    test3.testThis();
}

function ConsoleLog3() {
    // This will not be dispatched to the debugger.
    test3.Array.prototype.forEach(undefined)
}
