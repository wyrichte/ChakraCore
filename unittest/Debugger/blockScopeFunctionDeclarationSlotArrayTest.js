// Tests that function declaration bindings show correctly for the
// slot array case.

function test() {
    var a = 1;
    {
        a; /**bp:locals()**/
        function f() { }
        function g() {
            f();
        }
        g();
        a; /**bp:locals()**/
    }
    a;/**bp:locals()**/
}

test();
WScript.Echo("PASSED")