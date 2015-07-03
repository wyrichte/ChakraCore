var cc1Obj = new Object();
cc1Obj.func = function(obj) {
    WScript.Echo("Context1 func called");
    obj.func();
}
cc1Global = this;
function foo() {
    // Add new variables to console scope which references this context global var and func
    var x = 1; /**bp:evaluate('var cc1Var1 = 1;cc1Var2 = cc1Obj;var cc1Var3 = cc1Global;');**/
}