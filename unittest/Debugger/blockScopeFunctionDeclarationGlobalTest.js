// Tests that function declaration bindings show correctly for the
// global function case.

f;/**bp:locals()**/
{
    function f() { }
    f(); /**bp:locals()**/
}
f();/**bp:locals()**/

WScript.Echo("PASSED");