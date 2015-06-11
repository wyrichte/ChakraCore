// Basic Special Properties Tests
// Work Items: 782730 & 782731

if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in jc/jshost
    this.WScript.LoadScriptFile("..\\UnitTestFramework\\UnitTestFramework.js");
}

var tests = {
    test01: {
        name: "Non-Enumerable Special Properties Array Buffer Test",
        body: function () {
            var arrBuffer = new ArrayBuffer(5);
            /**bp:evaluate('arrBuffer', 1, LOCALS_ATTRIBUTES)**/
        }
    },
    test02: {
        name: "Non-Enumerable Special Properties Array Test",
        body: function () {
            var arr = [1, 2, 3, 4, 5];
            /**bp:evaluate('arr', 1, LOCALS_ATTRIBUTES)**/
        }
    },
    test03: {
        name: "Non-Enumerable Special Properties Pixel Array Test",
        body: function () {
            var uint8Arr = new Uint8Array(4);
            var pixelArr = WScript.CreateCanvasPixelArray(uint8Arr);
            /**bp:evaluate('pixelArr', 1, LOCALS_ATTRIBUTES)**/
        }
    },
    test04: {
        name: "Non-Enumerable Special Properties Regex Constructor Test",
        body: function () {
            var regex = /.*/;
            /**bp:evaluate('regex.constructor', 1, LOCALS_ATTRIBUTES)**/
        }
    },
    test05: {
        name: "Non-Enumerable Special Properties Regex Test",
        body: function () {
            var regex = new RegExp(".\\s?");
            var regex2 = /.\s?/igm;
            /**bp:evaluate('regex', 1, LOCALS_ATTRIBUTES)**/
            regex;
            /**bp:evaluate('regex2', 1, LOCALS_ATTRIBUTES)**/
        }
    },
    test06: {
        name: "Non-Enumerable Special Properties String Test",
        body: function () {
            var str = new String("123456");
            /**bp:evaluate('str', 1, LOCALS_ATTRIBUTES)**/
        }
    },
    test07: {
        name: "Non-Enumerable Special Properties Typed Array Test",
        body: function () {
            var int8Arr = new Int8Array(1);
            var uint8Arr = new Uint8Array(2);
            var int16Arr = new Int16Array(3);
            var uint16Arr = new Uint16Array(4);
            var int32Arr = new Int32Array(5);
            var uint32Arr = new Uint32Array(6);
            var float32Arr = new Float32Array(7);
            var float64Arr = new Float64Array(8);
            /**bp:evaluate('int8Arr', 2, LOCALS_ATTRIBUTES)**/
            int8Arr;
            /**bp:evaluate('uint8Arr', 2, LOCALS_ATTRIBUTES)**/
            int8Arr;
            /**bp:evaluate('int16Arr', 2, LOCALS_ATTRIBUTES)**/
            int8Arr;
            /**bp:evaluate('uint16Arr', 2, LOCALS_ATTRIBUTES)**/
            int8Arr;
            /**bp:evaluate('int32Arr', 2, LOCALS_ATTRIBUTES)**/
            int8Arr;
            /**bp:evaluate('uint32Arr', 2, LOCALS_ATTRIBUTES)**/
            int8Arr;
            /**bp:evaluate('float32Arr', 2, LOCALS_ATTRIBUTES)**/
            int8Arr;
            /**bp:evaluate('float64Arr', 2, LOCALS_ATTRIBUTES)**/
        }
    }
};

testRunner.runTests(tests);
