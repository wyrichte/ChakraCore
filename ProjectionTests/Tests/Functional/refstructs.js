if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    var animalFactory;
    var myAnimal;

    var expected1 = [
   "field1 : number = '20'",
   "field2 : number = '20'"];

    var expected2 = [
   "field1 : number = '-1'",
   "field2 : number = '-1'"];

    var expected3 = [
   "field1 : object = 'null'",
   "field2 : object = 'null'"];


    var expected4 = [
    "field1 : number = '20'",
    "dimensions : object = '[object Animals.Dimensions]'",
    "field3 : number = '20'"];

    var expected5 = ["length : number = '20'", "width : number = '20'"];

    var expected6 = [
    "field1 : number = '-1'",
    "dimensions : object = '[object Animals.Dimensions]'",
    "field3 : number = '255'"];

    var expected7 = ["length : number = '-1'", "width : number = '-1'"];
    var expected8 = [
	"wcharField : string = 'A'",
	"byteField : number = '65'",
	"int16Field : number = '65'",
	"int32Field : number = '65'",
	"uint16Field : number = '65'",
	"uint32Field : number = '65'",
	"booleanField : boolean = 'true'",
	"floatField : number = '65'",
	"doubleField : number = '65'" ];

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
        desc: 'TestInSimpleIRefStruct - Ensure that TestInSimpleIRefStruct set the right value of 20',
        pri: '0',
        test: function () {
            myAnimal.testInSimpleIRefStruct({field1: 20, field2: 20});
        }
    });

    runner.addTest({
        id: 2,
        desc: 'TestInSimpleIRefStruct - Ensure that TestInSimpleIRefStruct set the right value of -1',
        pri: '0',
        test: function () {
            myAnimal.testInSimpleIRefStruct({field1: -1, field2: -1});
        }
    });

    runner.addTest({
        id: 3,
        desc: 'TestInSimpleIRefStruct - Ensure that TestInSimpleIRefStruct allows null',
        pri: '0',
        test: function () {
            myAnimal.testInSimpleIRefStruct({field1: null, field2: null});
        }
    });

    runner.addTest({
        id: 4,
        desc: 'TestInSimpleIRefStruct - Ensure that TestInSimpleIRefStruct failed when invalue is different',
        pri: '0',
        test: function () {
            try {
                myAnimal.testInSimpleIRefStruct({field1: 0, field2: -1});
            }
            catch (error) {
                logger.comment('Caught error: ' + error.message);
                verify.instanceOf(error, WinRTError);
                WScript.Echo(error.description);
            }
        }
    });

    runner.addTest({
        id: 5,
        desc: 'TestInSimpleIRefStruct - Ensure that TestInSimpleIRefStruct failed when invalue is different',
        pri: '0',
        test: function () {
            try {
                myAnimal.testInSimpleIRefStruct({field1: 0, field2: null});
            }
            catch (error) {
                logger.comment('Caught error: ' + error.message);
                verify.instanceOf(error, WinRTError);
                WScript.Echo(error.description);
            }
        }
    });

    runner.addTest({
        id: 6,
        desc: 'TestOutSimpleIRefStruct - Ensure that TestOutSimpleIRefStruct get the right positive value',
        pri: '0',
        test: function () {
            var fromWinRT = myAnimal.testOutSimpleIRefStruct(20);
            verifyObject(fromWinRT, expected1, "object");

        }
    });

    runner.addTest({
        id: 7,
        desc: 'TestOutSimpleIRefStruct - Ensure that TestOutSimpleIRefStruct get the right negative value',
        pri: '0',
        test: function () {
            var fromWinRT = myAnimal.testOutSimpleIRefStruct(-1);
            verifyObject(fromWinRT, expected2, "object");

        }
    });

    runner.addTest({
        id: 8,
        desc: 'TestOutSimpleIRefStruct - Ensure that TestOutSimpleIRefStruct get the right null value',
        pri: '0',
        test: function () {
            var fromWinRT = myAnimal.testOutSimpleIRefStruct(0);
            verifyObject(fromWinRT, expected3, "object");

        }
    });

    runner.addTest({
        id: 9,
        desc: 'testInMixIRefStruct - Ensure that testInMixIRefStruct set the right value of 20',
        pri: '0',
        test: function () {
            myAnimal.testInMixIRefStruct({field1: 20, field2: 20, field3: 20});
        }
    });

    runner.addTest({
        id: 10,
        desc: 'testInMixIRefStruct - Ensure that testInMixIRefStruct set the right value of -1',
        pri: '0',
        test: function () {
            myAnimal.testInMixIRefStruct({field1: -1, field2: -1, field3: 1});
        }
    });

    runner.addTest({
        id: 11,
        desc: 'testInMixIRefStruct - Ensure that testInMixIRefStruct allows null',
        pri: '0',
        test: function () {
            myAnimal.testInMixIRefStruct({field1: null, field2: null, field3: 0});
        }
    });

    runner.addTest({
        id: 12,
        desc: 'testInNestedIRefNestedStruct - Ensure that NestedIRefNestedStruct set the right value of 20',
        pri: '0',
        test: function () {
            myAnimal.testInNestedIRefNestedStruct({field1: 20, dimensions: {length: 20, width: 20}, field3: 20});
        }
    });

    runner.addTest({
        id: 13,
        desc: 'testOutNestedIRefNestedStruct- Ensure that testOutNestedIRefNestedStruct get the right value of 20',
        pri: '0',
        test: function () {
            var fromWinRT = myAnimal.testOutNestedIRefNestedStruct(20);
            verifyObject(fromWinRT, expected4, "object");
            verifyObject(fromWinRT.dimensions, expected5, "object");
        }
    });

    runner.addTest({
        id: 14,
        desc: 'testInNestedIRefNestedStruct - Ensure that NestedIRefNestedStruct set the right value of -1',
        pri: '0',
        test: function () {
            myAnimal.testInNestedIRefNestedStruct({field1: -1, dimensions: {length:-1, width:-1}, field3: -1});
        }
    });

    runner.addTest({
        id: 15,
        desc: 'testOutNestedIRefNestedStruct - Ensure that NestedIRefNestedStruct get the right value of -1',
        pri: '0',
        test: function () {
            var fromWinRT = myAnimal.testOutNestedIRefNestedStruct(-1);
            verifyObject(fromWinRT, expected6, "object");
            verifyObject(fromWinRT.dimensions, expected7, "object");
        }
    });

    runner.addTest({
        id: 16,
        desc: 'testInNestedIRefNestedStruct - Ensure that NestedIRefNestedStruct set the right value of null',
        pri: '0',
        test: function () {
            myAnimal.testInNestedIRefNestedStruct({field1: null, dimensions: null, field3: 0});
        }
    });

    runner.addTest({
        id: 16,
        desc: 'AllIRefStruct - Ensure that AllIRefStruct set the right value of null',
        pri: '0',
        test: function () {
            myAnimal.testInAllIRefStruct({wcharField: null, int16Field: null, int32Field: null, uint16Field:null, 
                     uint32Field: null, floatField: null, doubleField: null, booleanField: null, byteField: null});
        }
    });

    runner.addTest({
        id: 17,
        desc: 'AllIRefStruct - Ensure that AllIRefStruct set the right value of 20',
        pri: '0',
        test: function () {
            myAnimal.testInAllIRefStruct({wcharField: 'L', int16Field: 20, int32Field: 20, uint16Field:20, 
                     uint32Field: 20, floatField: 20, doubleField: 20, booleanField: true, byteField: 20});
        }
    });

    runner.addTest({
        id: 17,
        desc: 'AllIRefStruct - Ensure that AllIRefStruct get the right value of 20',
        pri: '0',
        test: function () {
            var fromWinRT = myAnimal.testOutAllIRefStruct(65);
            verifyObject( fromWinRT, expected8, "object");
        }
    });

    Loader42_FileName = "IReference Struct tests";
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
