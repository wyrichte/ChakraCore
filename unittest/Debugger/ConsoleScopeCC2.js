var cc2Obj = new Object();
cc2Obj.func = function(obj) {
    WScript.Echo("Context2 func called");
}

function foo() {
    // Access and manipulate console scope variables created in different context to test cross context marshalling
    var x = 1; /**bp:evaluate('WScript.Echo(cc1Var1);');**/
    var x = 1; /**bp:evaluate('WScript.Echo(cc1Var2);');**/
    var x = 1; /**bp:evaluate('WScript.Echo(cc1Var3);');**/
    var x = 1; /**bp:evaluate('WScript.Echo(cc1Var2.func(cc2Obj));');**/
    var x = 1; /**bp:evaluate('delete cc1Var1;');**/
    var x = 1; /**bp:evaluate('WScript.Echo(cc1Var1);');**/
    var x = 1; /**bp:evaluate('delete cc1Var2;');**/
    var x = 1; /**bp:evaluate('WScript.Echo(cc1Var2);');**/
    var x = 1; /**bp:evaluate('delete cc1Var3;');**/
    var x = 1; /**bp:evaluate('WScript.Echo(cc1Var3);');**/
    var x = 1; /**bp:evaluate('delete cc2Obj;');**/
    var x = 1; /**bp:evaluate('WScript.Echo(cc2Obj);');**/
}