// This test validates that if a function throws an exception after attach that function can still be generated in the debug mode.

function bar() {
    try {
        function foo() {
            abc.def = 10;                   /**bp:stack();locals()**/
            return 10;
        }
        foo();
    }
    catch (e) {
        e;
    }

    WScript.Echo("Pass");
}

bar();
WScript.Attach(bar);

