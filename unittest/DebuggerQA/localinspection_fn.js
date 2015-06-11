/*
    Local Insepction Testing
*/

function foo() {
    var a = [];
    var x = 1; /**bp:locals(3)**/
}
foo();
WScript.Echo('PASSED');


