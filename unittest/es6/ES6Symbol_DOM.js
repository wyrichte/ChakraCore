/// <reference path="../../core/test/UnitTestFramework/UnitTestFramework.js" />

var tests = [
    {
        name: "Symbols are supported in DOM objects",
        body: function() {
            var sym = Symbol('DOM symbol');
            var el = document.getElementById("testdiv");
            
            el[sym] = 'symbol value';
            
            assert.areEqual('symbol value', el[sym], "Properties of DOM objects can be indexed via symbol");
            
            var symbols = Object.getOwnPropertySymbols(el);
            
            assert.areEqual(1, symbols.length, "Object.getOwnPropertySymbols works for DOM objects");
            assert.areEqual('symbol', typeof symbols[0], "Object.getOwnPropertySymbols returns actual symbols");
            assert.areEqual('Symbol(DOM symbol)', symbols[0].toString(), "Object.getOwnPropertySymbols returns the correct symbol objects");
            assert.isTrue(symbols[0] === sym, "Object.getOwnPropertySymbols returns the correct symbol objects");
        }
    }
];

testRunner.run(tests);
