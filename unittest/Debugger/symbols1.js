function test() {
    var o = {};
    o[Symbol('my symbol')] = Symbol('my value');
    o/**bp:evaluate('o',2, LOCALS_FULLNAME)**/
}

test();

WScript.Echo("Pass");
