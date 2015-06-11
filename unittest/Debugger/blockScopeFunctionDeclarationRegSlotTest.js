// Tests that function declaration bindings show correctly for the
// register slot case.

function test() {
    var a = 1;
    {
        a; /**bp:locals()**/
        function f1() { }
        a; /**bp:locals()**/
    }
    a;/**bp:locals()**/
}

test();
WScript.Echo("PASSED")