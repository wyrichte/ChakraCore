// ES6 Array builtins using this['constructor'] property to construct their return values

if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in jc/jshost
    this.WScript.LoadScriptFile("..\\UnitTestFramework\\UnitTestFramework.js");
}

var tests = [
    {
        name: "Array.prototype.concat",
        body: function () {
            var arr = ['a','b','c'];
            arr['constructor'] = Number;
            
            var out = Array.prototype.concat.call(arr, [1,2,3]);
            
            assert.isFalse(Array.isArray(out), "Return from Array.prototype.concat should not be an Array object");
            assert.isTrue(out instanceof Number, "Return from Array.prototype.concat should have been constructed from Number");
            assert.isTrue(0 === out.valueOf(), "Construction passes 0 to Number. out.valueOf should strict equal 0");
            assert.areEqual(6, out.length, "Array.prototype.concat sets the length property of returned object");
        }
    },
    {
        name: "Array.prototype.filter",
        body: function () {
            var arr = ['a','b','c'];
            arr['constructor'] = Number;
            
            var out = Array.prototype.filter.call(arr, function() { return true; });
            
            assert.isFalse(Array.isArray(out), "Return from Array.prototype.filter should not be an Array object");
            assert.isTrue(out instanceof Number, "Return from Array.prototype.filter should have been constructed from Number");
            assert.isTrue(0 === out.valueOf(), "Construction passes 0 to Number. out.valueOf should strict equal 0");
            assert.areEqual(undefined, out.length, "Array.prototype.filter does not set the length property of returned object");
        }
    },
    {
        name: "Array.prototype.map",
        body: function () {
            var arr = ['a','b','c'];
            arr['constructor'] = Number;
            
            var out = Array.prototype.map.call(arr, function(val) { return val; });
            
            assert.isFalse(Array.isArray(out), "Return from Array.prototype.map should not be an Array object");
            assert.isTrue(out instanceof Number, "Return from Array.prototype.map should have been constructed from Number");
            assert.isTrue(3 === out.valueOf(), "Construction passes length to Number. out.valueOf should strict equal length (3)");
            assert.areEqual(undefined, out.length, "Array.prototype.map does not set the length property of returned object");
        }
    },
    {
        name: "Array.prototype.slice",
        body: function () {
            var arr = ['a','b','c'];
            arr['constructor'] = Number;
            
            var out = Array.prototype.slice.call(arr);
            
            assert.isFalse(Array.isArray(out), "Return from Array.prototype.slice should not be an Array object");
            assert.isTrue(out instanceof Number, "Return from Array.prototype.slice should have been constructed from Number");
            assert.isTrue(3 === out.valueOf(), "Construction passes length to Number. out.valueOf should strict equal length (3)");
            assert.areEqual(3, out.length, "Array.prototype.slice sets the length property of returned object");
        }
    },
    {
        name: "Array.prototype.splice - array source with constructor property set to Number",
        body: function () {
            var arr = ['a','b','c','d','e','f'];
            arr['constructor'] = Number;
            
            var out = Array.prototype.splice.call(arr, 0, 3);
            
            assert.isFalse(Array.isArray(out), "Return from Array.prototype.splice should not be an Array object");
            assert.isTrue(out instanceof Number, "Return from Array.prototype.splice should have been constructed from Number");
            assert.isTrue(3 === out.valueOf(), "Construction passes length to Number. out.valueOf should strict equal length (3)");
            assert.areEqual(3, out.length, "Array.prototype.splice sets the length property of returned object");
        }
    },
    {
        name: "Array.prototype.splice - array source with constructor property set to Array",
        body: function () {
            var arr = [1,2,3,4,5,6];
            arr['constructor'] = Array;
            
            var out = Array.prototype.splice.call(arr, 0, 3);
            
            assert.isTrue(Array.isArray(out), "Return from Array.prototype.splice should be an Array object");
            assert.isTrue(out instanceof Array, "Return from Array.prototype.splice should have been constructed from Array");
            assert.areEqual([1,2,3], out, "Array.prototype.splice output is correct");
            assert.areEqual(3, out.length, "Array.prototype.splice sets the length property of returned object");
        }
    },
    {
        name: "Array.prototype.splice - array source with no constructor property",
        body: function () {
            var arr = [1,2,3,4,5,6];
            
            var out = Array.prototype.splice.call(arr, 0, 3);
            
            assert.isTrue(Array.isArray(out), "Return from Array.prototype.splice should be an Array object");
            assert.isTrue(out instanceof Array, "Return from Array.prototype.splice should have been constructed from Array");
            assert.areEqual([1,2,3], out, "Array.prototype.splice output is correct");
            assert.areEqual(3, out.length, "Array.prototype.splice sets the length property of returned object");
        }
    },
    {
        name: "Array.prototype.splice - object source with no constructor property",
        body: function () {
            var arr = {0:1,1:2,2:3,3:4,4:5,5:6,'length':6};
            
            var out = Array.prototype.splice.call(arr, 0, 3);
            
            assert.isTrue(Array.isArray(out), "Return from Array.prototype.splice should be an Array object");
            assert.isTrue(out instanceof Array, "Return from Array.prototype.splice should have been constructed from Array");
            assert.areEqual([1,2,3], out, "Array.prototype.splice output is correct");
            assert.areEqual(3, out.length, "Array.prototype.splice sets the length property of returned object");
        }
    },
    {
        name: "Array.prototype.splice - object source with constructor property set to Number",
        body: function () {
            var arr = {0:1,1:2,2:3,3:4,4:5,5:6,'length':6};
            arr['constructor'] = Number;
            
            var out = Array.prototype.splice.call(arr, 0, 3);
            
            assert.isTrue(Array.isArray(out), "Return from Array.prototype.splice should be an Array object");
            assert.isTrue(out instanceof Array, "Return from Array.prototype.splice should have been constructed from Array");
            assert.areEqual([1,2,3], out, "Array.prototype.splice output is correct");
            assert.areEqual(3, out.length, "Array.prototype.splice sets the length property of returned object");
        }
    },
];

testRunner.runTests(tests, { verbose: WScript.Arguments[0] != "summary" });
