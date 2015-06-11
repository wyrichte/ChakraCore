// Validating dynamic-attach on function which declares arguments as a params (204064)

function test() {
    function foo(arguments) {
        eval('arguments');                 /**bp:locals()**/
    }
    foo("11");                     
    WScript.Echo("Pass");
}
WScript.Attach(test);
