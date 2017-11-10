if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    var animalFactory;
    var myAnimal;

    runner.globalSetup(function () {
        animalFactory = Animals.Animal;
        myAnimal = new animalFactory(1);
    });

    var dimensionsExpected = [
    ['length', 'number', 180],
    ['width', 'number', 360]
    ];

    var updatedDimensions = [
    ['length', 'number', 200],
    ['width', 'number', 400]
    ];

    var hresults = Winery.WinRTErrorTests.RestrictedErrorAccess.errorCodes;

    var dimensionsToUpdate = { "length": 200, "width": 400 };

    function verifyDimensions(actualString, actual, expected) {
        var iIndex = 0;

        verify.defined(actual, 'Actual Dimensions');
        for (p in actual) {
            verify(p, expected[iIndex][0], p);
            verify(typeof actual[p], expected[iIndex][1], 'typeof ' + actualString + '[' + p + ']');
            verify(actual[p], expected[iIndex][2], actualString + '[' + p + ']');
            iIndex++;
        }
    }

    runner.addTest({
        id: 1,
        desc: 'GetProperty',
        pri: '0',
        test: function () {
            verify(myAnimal.weight, 50, 'myAnimal.weight');
        }
    });

    runner.addTest({
        id: 2,
        desc: 'SetProperty',
        pri: '0',
        test: function () {
            myAnimal.weight = 100;
            verify(myAnimal.weight, 100, 'myAnimal.weight');
        }
    });

    runner.addTest({
        id: 3,
        desc: 'SetPropertyFail',
        pri: '0',
        test: function () {
            try {
                myAnimal.weight = 800;
            } catch (e) {
                verify.instanceOf(e, WinRTError);
                verify(e.name, 'WinRTError', 'Error caught');
                if (typeof Animals._CLROnly === 'undefined') //HRESULT exception not applicable to C# ABI
                    verify(e.number, hresults.fail, 'e.number');
            }
            verify(myAnimal.weight, 100, 'myAnimal.weight');

        }
    });

    runner.addTest({
        id: "Bug133943 - 1",
        desc: "property test - assign null (interface)",
        pri: '0',
        test: function () {
            myAnimal.mother = null;
            verify(myAnimal.mother, null, "myAnimal.mother");
        }
    });

    runner.addTest({
        id: "Bug133943 - 2",
        desc: "property test - assign undefined (interface)",
        pri: '0',
        test: function () {
            myAnimal.mother = undefined;
            verify(myAnimal.mother, null, "myAnimal.mother");
        }
    });


    runner.addTest({
        id: "Win8: 195355",
        desc: 'struct and enum properties',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);
            verifyDimensions("myAnimal.myDimensions", myAnimal.myDimensions, dimensionsExpected);
            myAnimal.myDimensions = dimensionsToUpdate;
            verifyDimensions("myAnimal.myDimensions", myAnimal.myDimensions, updatedDimensions);

            verify(myAnimal.myPhylum, 1, "myAnimal.myPhylum");
            myAnimal.myPhylum = 3;
            verify(myAnimal.myPhylum, 3, "myAnimal.myPhylum");
        }
    });

    runner.addTest({
        id: "Win8: 837548 - 1",
        desc: 'Read-only property versioned to read-write property',
        pri: '0',
        test: function () {
            var obj = new DevTests.Repros.VersionedProperties.VersionedProperty();
            verify.defined(obj.testProperty, "obj.testProperty");

            var prototype = Object.getPrototypeOf(obj);
            var desc = Object.getOwnPropertyDescriptor(prototype, "testProperty");
            verify.defined(desc, "obj.testProperty property descriptor");
            verify.typeOf(desc.get, "function");
            verify.typeOf(desc.set, "function");

            verify(obj.testProperty, 0, "obj.testProperty");
            obj.testProperty = 42;
            verify(obj.testProperty, 42, "obj.testProperty");
        }
    });

    runner.addTest({
        id: "Win8: 837548 - 2",
        desc: 'Versioned Property conflicting with property from other interface',
        pri: '0',
        test: function () {
            var obj = new DevTests.Repros.VersionedProperties.ConflictWithVersionedProperty();
            verify.notDefined(obj.testProperty, "obj.testProperty");
            verify.defined(obj['DevTests.Repros.VersionedProperties.IReadOnlyProperty.testProperty'], "obj['DevTests.Repros.VersionedProperties.IReadOnlyProperty.testProperty']");
            verify.notDefined(obj['DevTests.Repros.VersionedProperties.IToReadWriteProperty.testProperty'], "obj['DevTests.Repros.VersionedProperties.IToReadWriteProperty.testProperty']");
            verify.defined(obj['DevTests.Repros.VersionedProperties.IConflictingReadWriteProperty.testProperty'], "obj['DevTests.Repros.VersionedProperties.IConflictingReadWriteProperty.testProperty']");

            var prototype = Object.getPrototypeOf(obj);
            var desc = Object.getOwnPropertyDescriptor(prototype, 'DevTests.Repros.VersionedProperties.IReadOnlyProperty.testProperty');
            verify.defined(desc, "obj['DevTests.Repros.VersionedProperties.IReadOnlyProperty.testProperty'] property descriptor");
            verify.typeOf(desc.get, "function");
            verify.typeOf(desc.set, "function");

            desc = Object.getOwnPropertyDescriptor(prototype, 'DevTests.Repros.VersionedProperties.IToReadWriteProperty.testProperty');
            verify.notDefined(desc, "obj['DevTests.Repros.VersionedProperties.IToReadWriteProperty.testProperty'] property descriptor");

            desc = Object.getOwnPropertyDescriptor(prototype, 'DevTests.Repros.VersionedProperties.IConflictingReadWriteProperty.testProperty');
            verify.defined(desc, "obj['DevTests.Repros.VersionedProperties.IConflictingReadWriteProperty.testProperty'] property descriptor");
            verify.typeOf(desc.get, "function");
            verify.typeOf(desc.set, "function");

            verify(obj['DevTests.Repros.VersionedProperties.IReadOnlyProperty.testProperty'], 0, "obj['DevTests.Repros.VersionedProperties.IReadOnlyProperty.testProperty']");
            verify(obj['DevTests.Repros.VersionedProperties.IToReadWriteProperty.testProperty'], undefined, "obj['DevTests.Repros.VersionedProperties.IToReadWriteProperty.testProperty']");
            verify(obj['DevTests.Repros.VersionedProperties.IConflictingReadWriteProperty.testProperty'], "DevTests.Repros.VersionedProperties.IConflictingReadWriteProperty.testProperty", "obj['DevTests.Repros.VersionedProperties.IConflictingReadWriteProperty.testProperty']");

            obj['DevTests.Repros.VersionedProperties.IConflictingReadWriteProperty.testProperty'] = "hello!";
            verify(obj['DevTests.Repros.VersionedProperties.IConflictingReadWriteProperty.testProperty'], "hello!", "obj['DevTests.Repros.VersionedProperties.IConflictingReadWriteProperty.testProperty']");

            obj['DevTests.Repros.VersionedProperties.IReadOnlyProperty.testProperty'] = 42;
            verify(obj['DevTests.Repros.VersionedProperties.IReadOnlyProperty.testProperty'], 42, "obj['DevTests.Repros.VersionedProperties.IReadOnlyProperty.testProperty']");
            obj['DevTests.Repros.VersionedProperties.IToReadWriteProperty.testProperty'] = -23;
            verify(obj['DevTests.Repros.VersionedProperties.IToReadWriteProperty.testProperty'], undefined, "obj['DevTests.Repros.VersionedProperties.IToReadWriteProperty.testProperty']");
            verify(obj['DevTests.Repros.VersionedProperties.IReadOnlyProperty.testProperty'], 42, "obj['DevTests.Repros.VersionedProperties.IReadOnlyProperty.testProperty']");
        }
    });

    // winblue bug:21530 (write only property is actually never supported, this test actually exploited a bug in MIDLRT)
    /*
    runner.addTest({
        id: "Win8: 837548 - 3",
        desc: 'Interface with write-only property',
        pri: '0',
        test: function () {
            var obj = DevTests.Repros.VersionedProperties.VersionedProperty.getInterfaceWithWriteOnlyProp();
            verify.notDefined(obj.testProperty, "obj.testProperty");

            var prototype = Object.getPrototypeOf(obj);
            var desc = Object.getOwnPropertyDescriptor(prototype, "testProperty");
            verify.typeOf(desc.get, "undefined");
            verify.typeOf(desc.set, "function");

            verify(obj.testProperty, undefined, "obj.testProperty");
            verify.defined(desc, "obj.testProperty property descriptor");
            obj.testProperty = 42;
            verify(obj.testProperty, undefined, "obj.testProperty");
            assert(obj.verifyTestProperty(42), "obj.verifyTestProperty(42)");
        }
    });
    */

    Loader42_FileName = "Property tests";
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
