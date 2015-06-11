if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    var animalFactory;
    var myAnimal;
    var JSERR_FunctionArgument_NeedWinRTGUID = "TypeError: Could not convert argument to type 'GUID'"

    var marshalTests = [
    // GUIDs
    // Success Cases
        ["GUID", "{00000000-0000-0000-0000-000000000123}", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "00000000000000000000000000000123", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "(00000000-0000-0000-0000-000000000123)", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "00000000-0000-0000-0000-000000000123", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "{0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x23}}", "00000000-0000-0000-0000-000000000123", "string"],
    // Other Types
        ["GUID", 1, JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", true, JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", 'a', JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", { Data1: 0x00000000, Data2: 0x0000, Data3: 0x0000, Data4: [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x23] }, JSERR_FunctionArgument_NeedWinRTGUID],
    // Invalid Strings
        ["GUID", "", JSERR_FunctionArgument_NeedWinRTGUID],
    // invalid hex digit
        ["GUID", "{00000000-0000-0000-h000-000000000123}", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "00000000-0000-0000-h000-000000000123", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "(00000000-0000-0000-h000-000000000123)", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "0000000000000000h000000000000123", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "{0x00000000,0x0000,0xh000,{0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x23}}", JSERR_FunctionArgument_NeedWinRTGUID],
    // leading whitespace
        ["GUID", " {00000000-0000-0000-0000-000000000123}", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", " 00000000-0000-0000-0000-000000000123", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", " (00000000-0000-0000-0000-000000000123)", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", " 00000000000000000000000000000123", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", " {0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x23}}", "00000000-0000-0000-0000-000000000123", "string"],
    //trailing whitespace
        ["GUID", "{00000000-0000-0000-0000-000000000123} ", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "00000000-0000-0000-0000-000000000123 ", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "(00000000-0000-0000-0000-000000000123) ", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "00000000000000000000000000000123 ", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "{0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x23}} ", "00000000-0000-0000-0000-000000000123", "string"],
    // inserted whitespace
        ["GUID", "{00000000-0000-00 00-0000-000000000123}", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "00000000-0000-00 00-0000-000000000123", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "(00000000-0000-00 00-0000-000000000123)", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "00000000000000 000000000000000123", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "{0x00000000,0x0000,0x00 00,{0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x23}}", "00000000-0000-0000-0000-000000000123", "string"],
    // contains non-ascii character
        ["GUID", "{00000000-0000-0000-\u00df000-000000000123}", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "00000000-0000-0000-\u00df000-000000000123", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "(00000000-0000-0000-\u00df000-000000000123)", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "0000000000000000\u00df000000000000123", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "{0x00000000,0x0000,0x0000,{0x00,0x\u00df0,0x00,0x00,0x00,0x00,0x01,0x23}}", JSERR_FunctionArgument_NeedWinRTGUID],
    // extra braces\parenthesis
        ["GUID", "{{00000000-0000-0000-0000-000000000123}}", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "((00000000-0000-0000-0000-000000000123))", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "{0x00000000,0x0000,0x0000,{{0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x23}}}", JSERR_FunctionArgument_NeedWinRTGUID],
    // extra dashes
        ["GUID", "{00000000--0000--0000--0000--000000000123}", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "00000000--0000--0000--0000--000000000123", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "(00000000--0000--0000--0000--000000000123)", JSERR_FunctionArgument_NeedWinRTGUID],
    // Additional X format tests
        ["GUID", "{0x0,0x0,0x0,{0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x23}}", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "{0X0,0X0,0X0,{0X0,0X0,0X0,0X0,0X0,0X0,0X1,0X23}}", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "{0x0,0X0,0X0,{0X0,0x0,0x0,0x0,0X0,0X0,0x1,0X23}}", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "{0x00000000123,0x45,0x00678,{0x9,0xA,0xb,0x0cD,0x0000000e,0xFf,0x01,0x23}}", "00000123-0045-0678-090a-0bcd0eff0123", "string"],
        ["GUID", "{0x0,0x0,0x0,{0x0,0x0,0x0,0x0,0x0,0x0,0x+1,0x23}}", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "{0x0,0x0,0x0,{0x0,0x0,0x0,0x0,0x0,0x0,0x-1,0x23}}", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "{0x0,0x0,0x0,{0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x2.3}}", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "{0x0,0x0,{0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x23}}", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "{0x0,0x0,0x0,{0x0,0x0,0x0,0x0,0x1,0x23}}", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "{0x0,,0x0,{0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x23}}", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "{0x0,0x0,0x0,{0x0,0x0,0x0,,0x0,0x0,0x1,0x23}}", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "{0x0,0x0,0x0,0x0,{0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x23}}", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "{0x0,0x0,0x0,{0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x23}}", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "{0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x23}", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "{0x0,0x0,0x0,[0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x23]}", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "{0x0,0x0,0x0,(0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x23)}", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "{0x00000000,0x11111,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x23}}", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "{0x00000000,0x000011111,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x23}}", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "{0x00000000,0x,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x23}}", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "/*Example GUID*/{0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x23}}", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "{0x00000000,0x0000,0x0000,/*Example GUID*/{0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x23}}", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "{0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x23}}/*Example GUID*/", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "{0x0,0x0,0x0,{0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x23}}\0/*Example GUID*/", "00000000-0000-0000-0000-000000000123", "string"],
    // Additional whitespace tests
        ["GUID", "\t\n{00000000-0000-0000-0000-000000000123}    \t", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "   \t00000000000000000000000000000123\n", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "\f(00000000-0000-0000-0000-000000000123)\r", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "\n\f00000000-0000-0000-0000-000000000123\r\n", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "{0x0,0x0,     0x0,{0x0,0x0,0x0,         0x0,0x0,0x0,0x1,0x23}}", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "{\n0x0000    0000,\t\f\n0x00 00,\r\n0x00 00,{0x00\n, \n0x0 0,0x0  0,0x00,0x0  0,0x00,\t0x01,0x23}\n}", "00000000-0000-0000-0000-000000000123", "string"],

        ["GUID", "{0000\t0000-0000-0000-0000-00 00 00 00 01 23}", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "00000000\n0000\n0000\n0000\n0000\n0000\n0123", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "(0000\r0000-0000-0000-0000-000000000123)", JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", "0000\f0000-0000-0000-0000-000000000123", JSERR_FunctionArgument_NeedWinRTGUID],

        ["GUID", "\u0020{00000000-0000-0000-0000-000000000123}", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "\u1680{00000000-0000-0000-0000-000000000123}", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "\u180E{00000000-0000-0000-0000-000000000123}", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "\u2000{00000000-0000-0000-0000-000000000123}", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "\u2001{00000000-0000-0000-0000-000000000123}", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "\u2002{00000000-0000-0000-0000-000000000123}", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "\u2003{00000000-0000-0000-0000-000000000123}", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "\u2004{00000000-0000-0000-0000-000000000123}", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "\u2005{00000000-0000-0000-0000-000000000123}", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "\u2006{00000000-0000-0000-0000-000000000123}", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "\u2007{00000000-0000-0000-0000-000000000123}", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "\u2008{00000000-0000-0000-0000-000000000123}", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "\u2009{00000000-0000-0000-0000-000000000123}", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "\u200a{00000000-0000-0000-0000-000000000123}", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "\u202F{00000000-0000-0000-0000-000000000123}", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "\u205f{00000000-0000-0000-0000-000000000123}", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "\u3000{00000000-0000-0000-0000-000000000123}", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "\u2028{00000000-0000-0000-0000-000000000123}", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "\u2029{00000000-0000-0000-0000-000000000123}", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "\u0009{00000000-0000-0000-0000-000000000123}", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "\u000A{00000000-0000-0000-0000-000000000123}", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "\u000b{00000000-0000-0000-0000-000000000123}", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "\u000c{00000000-0000-0000-0000-000000000123}", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "\u000D{00000000-0000-0000-0000-000000000123}", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "\u0085{00000000-0000-0000-0000-000000000123}", "00000000-0000-0000-0000-000000000123", "string"],
        ["GUID", "\u00a0{00000000-0000-0000-0000-000000000123}", "00000000-0000-0000-0000-000000000123", "string"],

        ["GUID", "\u0020\u1680\u180E{\u20050x0000\u20060000,\u2009\u200A0x0000,0x\u2028\u2029\u0009\u000A0000,\u2007\u2008{0x00,0x00,\u000B\u000C\u000D0x00,0x00,0x0\u0085\u00A00,0x0\u2000\u20010,0x01,0x23}}\u2002\u202F\u205F\u3000\u2003\u2004", "00000000-0000-0000-0000-000000000123", "string"],
    // Other invalid input
        ["GUID", null, JSERR_FunctionArgument_NeedWinRTGUID],
        ["GUID", undefined, JSERR_FunctionArgument_NeedWinRTGUID],
    // Marshal Struct containing GUID
        ["StudyInfo", { studyName: "Social Practices of Gorillas", subjectID: "{00000000-0000-0000-0000-000000000123}" }, { studyName: "Social Practices of Gorillas", subjectID: "00000000-0000-0000-0000-000000000123" }, "object"]
    ];

    runner.globalSetup(function () {
        animalFactory = Animals.Animal
        myAnimal = new Animals.Animal(1)
    });

    function verifyObject(parentName, obj, expectedOutput) {
        var match = true;
        verify(typeof expectedOutput, 'object', 'typeof expectedOutput');

        for (var field in obj) {
            if (typeof obj[field] == "object") {
                verifyObject((parentName + "." + field), obj[field], expectedOutput[field]);
            }
            verify(obj[field], expectedOutput[field], (parentName + "[" + field + "]"));
        }
    }

    function addMarshalTests() {
        for (i in marshalTests) {
            var crtTest = marshalTests[i];
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
                        verify(typeof actual, this.tags[3], 'typeof  ' + actual);
                        if (typeof this.tags[2] === "object") {
                            verifyObject(baseName, actual, this.tags[2]);
                        } else {
                            verify(actual, this.tags[2], this.tags[0]);
                        }
                    } catch (actualError) {
                        var expectedError = this.tags[2];
                        verify(actualError + '', expectedError, this.tags[0] + ' : expected error');
                    }
                }

            });

        }
    }

    addMarshalTests();

    var nextId = marshalTests.length;

    runner.addTest({
        id: nextId,
        desc: "Get/Set Id",
        pri: "0",
        test: function () {
            try {
                myAnimal.id = "{00000000-0000-0000-0000-000000000123}";
                verify(myAnimal.id, "00000000-0000-0000-0000-000000000123", "Animal.id");
            } catch (e) {
                fail(e);
            }
        }
    });

    nextId++;

    runner.addTest({
        id: nextId,
        desc: "Set Id null",
        pri: "0",
        test: function () {
            try {
                myAnimal.id = null;
                fail("Error Expected");
            } catch (error) {
                logger.comment('Caught error: ' + error.message);
                verify.instanceOf(error, TypeError);
                verify(error.description, "Could not convert argument to type 'GUID'", 'error.description');
            }
        }
    });

    nextId++;

    runner.addTest({
        id: nextId,
        desc: "Set Id undefined",
        pri: "0",
        test: function () {
            try {
                myAnimal.id = undefined;
                fail("Error Expected");
            } catch (error) {
                logger.comment('Caught error: ' + error.message);
                verify.instanceOf(error, TypeError);
                verify(error.description, "Could not convert argument to type 'GUID'", 'error.description');
            }
        }
    });

    nextId++;

    runner.addTest({
        id: nextId,
        desc: "verifyMarshalGUID",
        pri: "0",
        test: function () {
            try {
                var actual = myAnimal.verifyMarshalGUID("{60A36D60-6AC9-4266-B872-581295CE0C50}", "{60A36D60-6AC9-4266-B872-581295CE0C50}");
                verify(actual, "60a36d60-6ac9-4266-b872-581295ce0c50", "Verified GUID");
            } catch (e) {
                fail(e);
            }
        }
    });

    nextId++;

    runner.addTest({
        id: nextId,
        desc: "verifyMarshalGUID",
        pri: "0",
        test: function () {
            try {
                var actual = myAnimal.verifyMarshalGUID("{E55C507B-7C8F-4BDA-B576-A5AB98CF982B}", "E55C507B-7C8F-4BDA-B576-A5AB98CF982B");
                verify(actual, "e55c507b-7c8f-4bda-b576-a5ab98cf982b", "Verified GUID");
            } catch (e) {
                fail(e);
            }
        }
    });

    nextId++;

    runner.addTest({
        id: nextId,
        desc: "verifyMarshalGUID",
        pri: "0",
        test: function () {
            try {
                var actual = myAnimal.verifyMarshalGUID("{992810BA-C394-4429-B136-1BA8C5B54C5D}", "(992810BA-C394-4429-B136-1BA8C5B54C5D)");
                verify(actual, "992810ba-c394-4429-b136-1ba8c5b54c5d", "Verified GUID");
            } catch (e) {
                fail(e);
            }
        }
    });

    nextId++;

    runner.addTest({
        id: nextId,
        desc: "verifyMarshalGUID",
        pri: "0",
        test: function () {
            try {
                var actual = myAnimal.verifyMarshalGUID("{AADA726E-AFF5-4F56-9B03-C1EFE8497A0E}", "AADA726EAFF54F569B03C1EFE8497A0E");
                verify(actual, "aada726e-aff5-4f56-9b03-c1efe8497a0e", "Verified GUID");
            } catch (e) {
                fail(e);
            }
        }
    });

    nextId++;

    runner.addTest({
        id: nextId,
        desc: "verifyMarshalGUID",
        pri: "0",
        test: function () {
            try {
                var actual = myAnimal.verifyMarshalGUID("{0E7AB08B-F8AD-4187-80D9-CF2FB15EE7D7}", "{0xE7AB08B,0xF8AD,0x4187,{0x80,0xD9,0xCF,0x2F,0xB1,0x5E,0xE7,0xD7}}");
                verify(actual, "0e7ab08b-f8ad-4187-80d9-cf2fb15ee7d7", "Verified GUID");
            } catch (e) {
                fail(e);
            }
        }
    });

    Loader42_FileName = "GUID tests";
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
