function test() {
    var o = {};
    
    o[Symbol.iterator] = '[@@iterator]';
    o['string'] = '["string"]';
    o[Symbol('my symbol')] = Symbol('my value');
    o[Symbol('my symbol object')] = Object(Symbol('symobject'));
    o[Symbol('short symbol name')] = Symbol('s');
    o[Symbol('symbol with no name')] = Symbol();

    /**bp:evaluate('o',5)**/
}

test();

WScript.Echo("PASSED");