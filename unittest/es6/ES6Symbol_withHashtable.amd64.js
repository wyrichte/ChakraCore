// ES6 Symbol tests -- verifies the API shape and basic functionality

if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in jc/jshost
    this.WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js");
}

var tests = [
    {
        name: "Object as hashtable should not store symbols",
        body: function() {
            // Name of the TypeHandler for object as hashmap (for comparison to Debug.getTypeHandlerName())
            var hashtableTypeHandlerName = "class Js::SimpleDictionaryUnorderedTypeHandler<unsigned short,class Js::JavascriptString * __ptr64,0> const * __ptr64";
            
            function getHashtable(obj) {
                var o = {};
                if (obj !== undefined) {
                    o = obj;
                }
                
                // Object will get converted to hashtable after deleting 32 properties
                for(var i = 0; i < 32; i++) {
                    var key = 'key' + i;
                    o[key] = i;
                    delete o[key];
                }
                
                return o;
            }
            
            function verifyObject(h, symbols, names, values) {
                if (typeof Debug.getTypeHandlerName === 'function') {
                    assert.areNotEqual(hashtableTypeHandlerName, Debug.getTypeHandlerName(h), "Object is not a string-keyed type handler");
                }
                
                var arr = Object.getOwnPropertySymbols(h);
                
                assert.areEqual(symbols.length, arr.length, "Expected number of symbols returned from the object");
                
                for (var i = 0; i < symbols.length; i++) {
                    var found = false;
                    
                    for (var j = 0; j < arr.length; j++) {
                        if (symbols[i] === arr[j]) {
                            found = true;
                        }
                    }
                    
                    assert.isTrue(found, `Found expected symbol (${symbols[i].toString()}) via Object.getOwnPropertySymbols`);
                    assert.areEqual(values[symbols[i]], h[symbols[i]], "Value for symbol matches expected value");
                }
                
                arr = Object.getOwnPropertyNames(h);
                
                assert.areEqual(names.length, arr.length, "Expected number of non-symbols returned from object");
                
                for (var i = 0; i < names.length; i++) {
                    var found = false;
                    
                    for (var j = 0; j < arr.length; j++) {
                        if (names[i] === arr[j]) {
                            found = true;
                        }
                    }
                    
                    assert.isTrue(found, `Found expected name (${names[i]}) via Object.getOwnPropertyNames`);
                    assert.areEqual(values[names[i]], h[names[i]], "Value for name matches expected value");
                }
            }
            
            // List of symbols to assign to and ensure are present in the object
            var s = [Symbol('s'), Symbol('s'), Symbol('s'), Symbol('s')];
            // List of names used as string property keys
            var n = ['s'];
            // Normal object used to compare values for all property keys
            var v = {};
            v['s'] = '5';
            v[s[0]] = '1';
            v[s[1]] = '2';
            v[s[2]] = '4';
            v[s[3]] = '6';
            
            // First case: Object is PathType because last property is delete before we add any symbol properties
            var h = getHashtable();

            if (typeof Debug.getTypeHandlerName === 'function') {
                assert.areEqual("class Js::PathTypeHandlerNoAttr const * __ptr64", Debug.getTypeHandlerName(h), "Object is a string-keyed type handler");
            }

            h['s'] = '0';
            h[s[0]] = '1';
            h[s[1]] = '2';
            h['s'] = '3';
            h[s[2]] = '4';
            h['s'] = '5';
            h[s[3]] = '6';

            verifyObject(h, s, n, v);
            
            // Second case: Object with symbol properties should not be converted to hashtable
            var h = {};
            
            h['s'] = '0';
            h[s[0]] = '1';
            h[s[1]] = '2';
            h['s'] = '3';
            h[s[2]] = '4';
            h['s'] = '5';
            h[s[3]] = '6';
            
            h = getHashtable(h);
            
            verifyObject(h, s, n, v);
        }
    }
];

testRunner.runTests(tests);
