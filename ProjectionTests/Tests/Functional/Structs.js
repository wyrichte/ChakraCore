if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    var animalFactory;
    var myAnimal;

    var expected1 = [
   "length : number = '180'",
   "width : number = '360'"];

    var expected1obj = {
        length: '180',
        width: '360'
    };

    var expected2 = [
   "length : number = '92'",
   "width : number = '181'"];

    var expected3 = [
   "length : number = '192'",
   "width : number = '186'"];

    var expected4 = ["inner : object = '[object Animals._InnerStruct]'"];
    var expected5 = ["a : number = '100'"];
    var expected6 = ["a : number = '52'"];

    var expected11 = [
   "length : number = '280'",
   "width : number = '460'"];

    var expected11obj = {
        length: '280',
        width: '460'
    }

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
        animalFactory = Animals.Animal;
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
    runner.addTest({
        id: 10,
        desc: 'MarshalStudyInfo - Check content of StudyInfo',
        pri: '0',
        test: function () {
            var a = { studyName: 'hello', subjectID: "7c9af34a-ab59-425a-a6db-0c372ccaa322" };
            var expected = ["studyName : string = 'hello'",
            "subjectID : string = '7c9af34a-ab59-425a-a6db-0c372ccaa322'"];
            var result = myAnimal.marshalStudyInfo(a);
            verifyObject(result, expected, 'object');
        }
    });

    runner.addTest({
        id: 11,
        desc: 'StructsByRef - Check the behaviour of Structs when passed ByRef.',
        pri: '0',
        test: function () {
            var falseOnError = myAnimal.fillDimensions.bind(myAnimal).bindReturn(true, false);


            verifyFalse(falseOnError(null), "Can't pass null for ByRef.");
            verifyFalse(falseOnError(undefined), "Error expected.");
            verifyFalse(falseOnError({length: 0}), "Error expected.");
            verifyFalse(falseOnError({width: 0}), "Error expected.");
            
            myAnimal.fillDimensions(expected11obj);
            verifyObject(myAnimal.getDimensions(), expected11, 'object');
            myAnimal.fillDimensions(expected1obj);
            verifyObject(myAnimal.getDimensions(), expected1, 'object');

            falseOnError = myAnimal.areDimensionPointersEqual.bindOnError(false);
            verifyFalse(falseOnError(null, null), "Can't pass null for ByRef.");
            verifyFalse(falseOnError({length: 0, length:0}, {length: 0, length:0}), "Two seperate object instances definetly can't have equal pointers.");
            var a = {length: 0, length:0};
            verifyFalse(falseOnError(a,a), "At the time of this test, neither JS nor was it the requirement to support equal pointers for same object marshalled to WinRT");

            verifyFalse(myAnimal.isStructModified(function (obj) {
                if (obj.length !== 1 && obj.width !== 2) {
                    throw new Error("Incorrect struct passed through.")
                };
                obj.length = obj.width;
            }, { length: 1, width: 2 }), "Struct's modification shouldn't affect the struct given/passed from/to JS.");
        }
    });

    runner.addTest({
        id: 12,
        desc: 'PassIdByRef - Ensure that PassIdByRef correctly accepts a GUID passed by ref.',
        pri: '0',
        test: function () {
            var falseOnError = myAnimal.passIDByRef.bind(myAnimal).bindReturn(true, false);
            verifyFalse(falseOnError(null), "Error expected.");
            verifyFalse(falseOnError(undefined), "Error expected.");
            verifyFalse(falseOnError(["7c9af34a", "ab59", "425a", "a6db", "0c372ccaa323"]), "Error expected.");
            myAnimal.passIDByRef("{7c9af34a-ab59-425a-a6db-0c372ccaa323}");
            verify(myAnimal.id, '7c9af34a-ab59-425a-a6db-0c372ccaa323', 'string');
        }
    });

    Function.prototype.bindParams = function (...param) {
        return function (...params) { this(...param, ...params); }.bind(this);
    }

    Function.prototype.bindReturn = function (success, failure) {
        return function (...params) {
            try {
                this(...params);
                return success;
            } catch(ex) {
                runner.publish('comment', "Error that was consumed in bindReturn: " + ex.message);
                return failure;
            }
        }.bind(this);
    }

    Function.prototype.bindOnError = function (failure) {
        return function (...params) {
            try {
                return this(...params);
            } catch(ex) {
                runner.publish('comment', "Error that was consumed in bindOnError: " + ex.message);
                return failure;
            }
        }.bind(this);
    }

    var verifyFalse = verify.bindParams(false);
    var verifyTrue = verify.bindParams(true);

    function doCheckMotherTest(func, toPass, expectError) {
        try {
            func(toPass);
            verify(true, !expectError, "No exception thrown.");
        } catch (ex) {
            verify(true, expectError, "Exception thrown: " + ex.message);
        }
    }

    function performMotherTests(check, checkConcrete) {
        myAnimal.mother = null;

        check(myAnimal, true);
        checkConcrete(myAnimal, true);
        check(null, false);
        checkConcrete(null, false);
        check(undefined, false);
        checkConcrete(undefined, false);

        myAnimal.mother = myAnimal;

        check(myAnimal, false);
        checkConcrete(myAnimal, false);
        check(null, true);
        checkConcrete(null, true);
        check(undefined, true);
        checkConcrete(undefined, true);
    }

    runner.addTest({
        id: 13,
        desc: 'CheckMother - Ensure that both flavors of CheckMother succesfully compare the stored mother reference.',
        pri: '0',
        test: function () {
            performMotherTests(doCheckMotherTest.bindParams(myAnimal.checkMother.bind(myAnimal)), doCheckMotherTest.bindParams(myAnimal.checkMotherConcrete.bind(myAnimal)));
        }
    });

    runner.addTest({
        id: 14,
        desc: 'DelegatesByRef - Test to make sure that passing delegates by ref works as expected.',
        pri: '0',
        test: function () {
            function doTest(func, delegateFunc, ...arg) {
                func(delegateFunc, ...arg);
            }

            var testDelegateByRef_Struct = doTest.bindParams(myAnimal.delegateByRef_Struct.bind(myAnimal), myAnimal.fillDimensions.bind(myAnimal));
            var testDelegateByRef_GUID = doTest.bindParams(myAnimal.delegateByRef_GUID.bind(myAnimal), myAnimal.passIDByRef.bind(myAnimal));
            var testDelegateByRef_Interface = doTest.bindParams(myAnimal.delegateByRef_Interface.bind(myAnimal), myAnimal.checkMother.bind(myAnimal).bindReturn(true, false));
            var testDelegateByRef_Class = doTest.bindParams(myAnimal.delegateByRef_Class.bind(myAnimal), myAnimal.checkMotherConcrete.bind(myAnimal).bindReturn(true, false));
            var testDelegateByRef_Delegate = doTest.bindParams(myAnimal.delegateByRef_Delegate.bind(myAnimal), myAnimal.delegateByRef_Struct.bind(myAnimal), myAnimal.fillDimensions.bind(myAnimal));


            testDelegateByRef_Struct(expected11obj);
            verifyObject(myAnimal.getDimensions(), expected11, 'object');
            testDelegateByRef_Struct(expected1obj);
            verifyObject(myAnimal.getDimensions(), expected1, 'object');
            myAnimal.passIDByRef("{7c9af34a-ab59-425a-a6db-0c372ccab023}");
            verify(myAnimal.id, '7c9af34a-ab59-425a-a6db-0c372ccab023', 'string');

            performMotherTests(doCheckMotherTest.bindParams(testDelegateByRef_Interface), doCheckMotherTest.bindParams(testDelegateByRef_Class));

            testDelegateByRef_Delegate(expected11obj);
            verifyObject(myAnimal.getDimensions(), expected11, 'object');
            testDelegateByRef_Delegate(expected1obj);
            verifyObject(myAnimal.getDimensions(), expected1, 'object');
        }
    });

    runner.addTest({
        id: 15,
        desc: "MemoryTest - Ensure that we are deallocating memory after every call.",
        pri: '0',
        test: function () {
            var v = { value1: 1, value2: 2, value3: 3, value4: 4, value5: 5, value6: 6, value7: 7, value8: 8 };
            var m = { vector1: v, vector2: v, vector3: v, vector4: v, vector5: v, vector6: v, vector7: v, vector8: v };
            var kiloStruct = { matrix1: m, matrix2: m, matrix3: m, matrix4: m };

            var counter = 0;
            //Reduced to 1024 interations, otherwise would be slow
            for (var i = 1; i < 1024 /* * 1024 1GB */; i++) {
                //if (i % (10 * 1024) === 0) {
                //    counter++;
                //    runner.publish('comment', "" + counter + "0MB/1GB done.");
                //}
                myAnimal.acceptKiloStruct(kiloStruct); //1 KB
            }
        }
    });

    runner.addTest({
        id: 16,
        desc: "MixedStructs - Tests with mixed structs passed as ByRef and as IReference",
        pri: '0',
        test: function () {
            var v = { value1: 1, value2: 2, value3: 3, value4: 4, value5: 5, value6: 6, value7: 7, value8: 8 };
            var v2 = { value1: 11, value2: 22, value3: 33, value4: 44, value5: 55, value6: 66, value7: 77, value8: 88 };
            var m = { vector1: v, vector2: v2, vector3: v, vector4: v, vector5: v, vector6: v, vector7: v, vector8: v };
            var struct = { astring: "This is A String", matrixRef: m, intRef: Object(1), matrix: m, anInt: 1 };
            var struct2 = { astring: struct.astring, matrixRef: m, intRef: struct.intRef, matrix: m, anInt: struct.anInt };

            var result = myAnimal.checkByRefStruct(struct, struct2);
            verifyFalse(result.structPointerEqual, "We pass one as a pointer, while ByRef we still copy, can't be same pointer.");
            verifyFalse(result.structPointerEqual, "It isn't a requirement for the strings to be equal.");
            verifyFalse(result.structPointerEqual, "It isn't a requirement for matrix references to be equal.");
            verifyFalse(result.structPointerEqual, "It isn't a requirement for int references to be equal.");
        }
    });

    Loader42_FileName = "Struct tests";
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
