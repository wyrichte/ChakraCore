// Tests that nested function name displays show properly in the locals window "value" column.
function f1() {
    var a = 0;
    function f2() {
        var b = 1;
        function f3() {
            var c = a + b;
            c; /**bp:locals()**/
        }
        f3();
    }
    f2();
}
f1();
WScript.Echo("PASSED");