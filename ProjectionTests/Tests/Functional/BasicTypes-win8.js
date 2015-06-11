if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {

    var animalFactory;
    var animalProperty;
    var animalFactoryProperty;
    var myAnimal;

    var TWO_POW_8 = 256
    var TWO_POW_16 = 65536
    var TWO_POW_32 = 4294967296
    var JSERR_FunctionArgument_NeedWinRTChar = "TypeError: Could not convert argument to type 'char'"

    var typeTests = [
    // Base name,    input, expected output, expected output type
    ["UInt32", 6, 6, "number"],
    ["Int64", 52, 52, "number"],
    ["Int32", 5, 5, "number"],
    ["Double", 1.1, 1.1, "number"],
    ["Bool", true, true, "boolean"],
    ["Bool", false, false, "boolean"],
    ["Char16", 'a', 'a', "string"],
    ["UInt8", 7, 7, "number"],
    ["Single", 59, 59, "number"],
    ["UInt64", 53, 53, "number"],
    ["Int16", 53, 53, "number"],
    ["UInt16", 53, 53, "number"],

    // Int16\UInt16 support
    ["Int16", TWO_POW_16 - 1, -1, "number"],
    ["Int16", TWO_POW_16, 0, "number"],
    ["Int16", -1, -1, "number"],

    // Rounding of number to Int16
    ["Int16", 1.5, 1, "number"],
    ["Int16", -1.5, -1, "number"],

    ["UInt16", TWO_POW_16 - 1, TWO_POW_16 - 1, "number"],
    ["UInt16", TWO_POW_16, 0, "number"],
    ["UInt16", -1, TWO_POW_16 - 1, "number"],

    // Rounding of number to UInt16
    ["UInt16", 1.5, 1, "number"],
    ["UInt16", -1.5, TWO_POW_16 - 1, "number"],

    // 7.4.1.1 If the JavaScript number value is outside the range of INT32, the result will have modulo 2^32 applied
    ["Int32", TWO_POW_32 - 1, -1, "number"],
    ["Int32", TWO_POW_32, 0, "number"],
    ["Int32", -1, -1, "number"],

    // Unspec-ed: Rounding of number to Int32
    ["Int32", 1.5, 1, "number"],
    ["Int32", -1.5, -1, "number"],

    // 7.4.1.3 If the JavaScript number value is outside the range of UINT8, the result will have modulo 2^8 applied
    ["UInt8", TWO_POW_8 - 1, TWO_POW_8 - 1, "number"],
    ["UInt8", TWO_POW_8, 0, "number"],
    ["UInt8", -1, TWO_POW_8 - 1, "number"],

    // Unspec-ed: Rounding of number to UInt32
    ["UInt8", 1.5, 1, "number"],
    ["UInt8", -1.5, TWO_POW_8 - 1, "number"],

    // 7.4.3.4 If the JavaScript number value is outside the range of UINT32, the result will have modulo 2^32 applied
    ["UInt32", TWO_POW_32 - 1, TWO_POW_32 - 1, "number"],
    ["UInt32", TWO_POW_32, 0, "number"],
    ["UInt32", -1, TWO_POW_32 - 1, "number"],

    // Unspec-ed: Rounding of number to UInt32
    ["UInt32", 1.5, 1, "number"],
    ["UInt32", -1.5, TWO_POW_32 - 1, "number"],

    // 7.4.1.8 WinRT defines Boolean as an 8 bit value where zero is false and any value other than zero is true
    ["Bool", TWO_POW_8, true, "boolean"],
    ["Bool", TWO_POW_8 - 1, true, "boolean"],
    ["Bool", 1, true, "boolean"],
    ["Bool", 0, false, "boolean"],
    ["Bool", "test", true, "boolean"],

    // 7.4.1.9 A JavaScript value being marshaled into a Char16 is type coerced into a JavaScript String an then length
    // checked to ensure the string is only a single character long.
    // jomof--I interpret 'only' above to mean 'exactly'
    ["Char16", null, JSERR_FunctionArgument_NeedWinRTChar],
    ["Char16", '', JSERR_FunctionArgument_NeedWinRTChar],
    ["Char16", "ab", JSERR_FunctionArgument_NeedWinRTChar],
    ["Char16", 92, JSERR_FunctionArgument_NeedWinRTChar],

    // Unspec-ed: Behavior of null
    ["UInt32", null, 0, "number"],
    ["Int64", null, 0, "number"],
    ["Int32", null, 0, "number"],
    ["Double", null, 0, "number"],
    ["Bool", null, false, "boolean"],
    ["UInt8", null, 0, "number"],
    ["Single", null, 0, "number"],
    ["UInt64", null, 0, "number"],

    ];

    runner.globalSetup(function () {
        animalFactory = Animals.Animal
        myAnimal = new Animals.Animal(1)
    });

    for (i in typeTests) {
        var crtTest = typeTests[i];
        var baseName = crtTest[0];
        var input = crtTest[1];
        var methodName = "marshal" + baseName;
        var expected = crtTest[2];
        var expectedType = crtTest[3];

        runner.addTest({
            id: i,
            desc: methodName,
            pri: "0",
            tags: [baseName, input, expected, expectedType],
            test: function () {
                try {
                    var actual = myAnimal[this.desc](this.tags[1]);
                    //var actual = this.tags[1];
                    verify(typeof actual, this.tags[3], 'typeof  ' + actual);
                    verify(actual, this.tags[2], this.tags[0]);
                } catch (actualError) {
                    var expectedError = this.tags[2];
                    verify(actualError + '', expectedError, this.tags[0] + ' : expected error');
                }
            }

        });

    }

    Loader42_FileName = "type tests";
})();

if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
