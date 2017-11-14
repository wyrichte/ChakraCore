if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    var animalFactory;
    var myAnimal;

    var expected1 = [
   "length : number = '180'",
   "width : number = '360'"];

    var expected2 = [
   "length : number = '92'",
   "width : number = '181'"];

    var expected3 = [
   "length : number = '192'",
   "width : number = '186'"];

    var expected4 = ["inner : object = '[object Animals._InnerStruct]'"];
    var expected5 = ["a : number = '100'"];
    var expected6 = ["a : number = '52'"];

    function verifyObject(actual, expected, t) {
        verify(typeof actual, t, 'type', false);
        var i = 0;
        for (p in actual) {
            var actualStr = p + " : " + typeof actual[p] + " = '" + actual[p] + "'";
            verify(actualStr, expected[i], actualStr);
            i++;
        }
        verify(i, expected.length, 'number of members', false);
    }

    var JSERR_MissingStructProperty = function (name) {
        return "Could not convert object to struct: object missing expected property '" + name + "'";
    }

    runner.globalSetup(function () {
        animalFactory = Animals.Animal
        myAnimal = new Animals.Animal(1)
    });

    runner.addTest({
        id: 1,
        desc: 'BasicGetDimensions - Ensure that GetDimensions returns what we expect',
        pri: '0',
        test: function () {
            verifyObject(myAnimal.getDimensions(), expected1, 'object');
        }
    });

    runner.addTest({
        id: 2,
        desc: 'MarshalInlineStruct - Make sure that we can marshal an inline struct',
        pri: '0',
        test: function () {
            verifyObject(myAnimal.marshalDimensions({ length: 92, width: 181 }), expected2, 'object');
        }
    });

    runner.addTest({
        id: 3,
        desc: 'MarshalCoercibleInlineStruct - Make sure that we can marshal an inline struct',
        pri: '0',
        test: function () {
            verifyObject(myAnimal.marshalDimensions({ length: '192', width: '186' }), expected3, 'object');
        }
    });

    runner.addTest({
        id: 4,
        desc: 'MarshalNullDimensions - Behavior of null dimensions',
        pri: '0',
        test: function () {
            try {
                var nullDim = myAnimal.marshalDimensions(null);
            }
            catch (error) {
                logger.comment('Caught error: ' + error.message);
                verify.instanceOf(error, TypeError);
                verify(error.description, JSERR_MissingStructProperty('length'), 'error.description');
            }
        }
    });

    runner.addTest({
        id: 5,
        desc: 'RoundTripFromWinRT - Make sure that we can marshal an inline struct',
        pri: '0',
        test: function () {
            var fromWinRT = myAnimal.getDimensions();
            var roundTripped = myAnimal.marshalDimensions(fromWinRT);
            verifyObject(roundTripped, expected1, 'object');
            verify(!(fromWinRT == roundTripped), true, 'Do not expect == of roundtripped object'); // ES5 Section 11.9.3 (Equality)
            verify(!(fromWinRT === roundTripped), true, 'Do not expect === of roundtripped object'); // ES5 Section 11.9.6 (Strict Equality)
        }
    });

    runner.addTest({
        id: 6,
        desc: 'SimpleNestedStruct - Make sure that we can marshal an inline struct',
        pri: '0',
        test: function () {
            var outer = myAnimal.getOuterStruct();
            verifyObject(outer, expected4, 'object');
            verifyObject(outer.inner, expected5, 'object');
        }
    });

    runner.addTest({
        id: 7,
        desc: 'MarshalInlineNestedStruct - Check inline nested struct',
        pri: '0',
        test: function () {
            var outer = myAnimal.marshalOuterStruct({ inner: { a: 52} });
            verifyObject(outer, expected4, 'object');
            verifyObject(outer.inner, expected6, 'object');
        }
    });

    runner.addTest({
        id: 8,
        desc: 'RoundtripNestedStruct - Make sure a nested struct can round trip from WinRT object and back through',
        pri: '0',
        test: function () {
            var outer = myAnimal.getOuterStruct();
            verifyObject(outer, expected4, 'object');
            verifyObject(outer.inner, expected5, 'object');
        }
    });

    runner.addTest({
        id: 9,
        desc: 'MarshalNullNestedStruct - Null in the case of an inner struct',
        pri: '0',
        test: function () {
            try {
                var outer = myAnimal.marshalOuterStruct({ inner: null });
            }
            catch (error) {
                logger.comment('Caught error: ' + error.message);
                verify.instanceOf(error, TypeError);
                verify(error.description, JSERR_MissingStructProperty('a'), 'error.description');
            }
        }
    });

    Loader42_FileName = "Struct tests";
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
