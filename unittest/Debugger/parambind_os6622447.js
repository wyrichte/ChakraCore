var bar1;
function foo(a, b) {
    function bar(c, d) {
        print("Pass");
    }
    bar1 = bar;
}

function test() {
    bar1();
}
foo(1, 2);

// The trick is :
//           put 'bar' to defer mode (create a deferred state in the non-debug mode) using -forcedeferparse
//           put the script context in the source rundown mode.
//           perform some operation to undefer 'bar' (So essentially bar's state was created in non-debug but used in rundown mode)

WScript.PerformSourceRundown();
WScript.SetTimeout(test, 100);
