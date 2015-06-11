if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    var animalFactory;
    var animalProperty;
    var animalFactoryProperty;
    var myAnimal;

    var expectedStruct = [
   "common : string = 'Wolverine'",
   "scientific : string = 'Gulo gulo'",
   "alsoKnownAs : string = 'Skunk Bear'"
    ];

    var expectedStruct1 = [
   "common : string = 'xyz'",
   "scientific : string = '123'",
   "alsoKnownAs : string = 'lmn'"
    ];

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

    runner.globalSetup(function () {
        animalFactory = Animals.Animal;
        myAnimal = new animalFactory(1);
    });

    runner.addTest({
        id: 1,
        desc: 'BasicString - Basic round-trip of a string',
        pri: '0',
        test: function () {
            verify(myAnimal.marshalHSTRING("abc"), "abc", "myAnimal.marshalHSTRING(\"abc\")");
        }
    });

    runner.addTest({
        id: 2,
        desc: 'NullInOutString - Null string in-out',
        pri: '0',
        test: function () {
            var result = myAnimal.marshalHSTRING(null);
            verify(result, 'null', "myAnimal.marshalHSTRING(null)");
            verify(typeof result, 'string', "typeof null"); // By design, this comes back as string "null"
        }
    });

    runner.addTest({
        id: 3,
        desc: 'NullOutString - Null string',
        pri: '0',
        test: function () {
            var result = myAnimal.getNULLHSTRING();
            verify(result, '', "myAnimal.getNULLHSTRING()");
            verify(typeof result, 'string', "typeof null");
        }
    });

    runner.addTest({
        id: 4,
        desc: 'EmptyString - Empty string',
        pri: '0',
        test: function () {
            verify(myAnimal.marshalHSTRING(""), '', "myAnimal.marshalHSTRING(\"\")");

        }
    });

    runner.addTest({
        id: 5,
        desc: 'IntString - String conversion to integer',
        pri: '0',
        test: function () {
            verify(myAnimal.marshalHSTRING(123), '123', "myAnimal.marshalHSTRING(123)");
        }
    });

    runner.addTest({
        id: 6,
        desc: 'StringStruct - A struct with strings in it',
        pri: '0',
        test: function () {
            verifyObject(myAnimal.getNames(), expectedStruct, 'object');
        }
    });

    runner.addTest({
        id: 7,
        desc: 'RoundTripStringStruct - Round trip a struct with strings in it',
        pri: '0',
        test: function () {
            verifyObject(myAnimal.marshalNames(myAnimal.getNames()), expectedStruct, 'object');
        }
    });

    runner.addTest({
        id: 8,
        desc: 'RoundTripJsOriginatedStringStruct - Round trip a struct with strings in it that originated from JS',
        pri: '0',
        test: function () {
            verifyObject(myAnimal.marshalNames({ common: "xyz", scientific: "123", alsoKnownAs: "lmn" }), expectedStruct1, 'object');
        }
    });

    /*
    Would like to test:
    - Array of string [in\out]
    */


    Loader42_FileName = "Struct tests";
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
