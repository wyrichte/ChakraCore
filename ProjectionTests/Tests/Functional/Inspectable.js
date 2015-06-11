if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {

    function easyMembersPrint(myObjectString, myObject) {
        var objectDump = "\n    var " + myObjectString + "Members = [";
        for (p in myObject) {
            if (typeof myObject[p] == 'function') {
                objectDump = objectDump + '\n        [\'' + p + '\', \'' + typeof myObject[p] + '\', ' + myObject[p].length + '],'
            }
            else {
                objectDump = objectDump + '\n        [\'' + p + '\', \'' + typeof myObject[p] + '\'],';
            }
        }
        objectDump = objectDump + "\n    ];";

        return objectDump;
    }

    function dumpObjectMembers(myObjectString, myObject) {
        var objectDump = easyMembersPrint(myObjectString, myObject);

        logger.comment("typeof " + myObjectString + ": " + typeof myObject);
        logger.comment("Dump of properties : " + objectDump);
    }

    function verifyMembers(myObjectString, myObject, expectedProperties, myArray) {
        dumpObjectMembers(myObjectString, myObject);

        verify(typeof myObject, "object", "typeof " + myObjectString);

        var checkArray = 0;
        var numberOfElements = 0;
        var propertiesIndex = 0;

        for (p in myObject) {
            if (Array.isArray(myArray) && checkArray < myArray.length) {
                verify(p, checkArray.toString(), myObjectString + '\'s property');
                verify(typeof myObject[p], typeof myArray[checkArray], 'typeof ' + myObjectString + '["' + p + '"]');
                verify(myObject[p], myArray[checkArray], myObjectString + '["' + p + '"]');
                checkArray++;
            }
            else {
                // Look in properties
                verify(p, expectedProperties[propertiesIndex][0], myObjectString + '\'s property');
                verify(typeof myObject[p], expectedProperties[propertiesIndex][1], 'typeof ' + myObjectString + '["' + p + '"]');

                if (typeof myObject[p] == 'function') {
                    verify(myObject[p].length, expectedProperties[propertiesIndex][2], myObjectString + '["' + p + '"].length');
                    logger.comment('Setting length of function to be 10');
                    myObject[p].length = 10;
                    verify(myObject[p].length, expectedProperties[propertiesIndex][2], myObjectString + '["' + p + '"].length');
                }
                propertiesIndex++;
            }

            numberOfElements++;
        }

        var exptectedPropertiesLength = expectedProperties.length;
        if (Array.isArray(myArray)) {
            exptectedPropertiesLength = exptectedPropertiesLength + myArray.length;
        }

        verify(numberOfElements, exptectedPropertiesLength, 'number of properties of ' + myObjectString);
    }

    function VerifyArrayAndVectorContents(myVectorString, actualVector, myArrayString, actualArray) {
        verify(actualVector.length, actualArray.length, myVectorString + ".length");

        for (var i = 0; i < actualArray.length; i++) {
            verify(actualVector[i], actualArray[i], myVectorString + "[" + i + "]");
        }
    }

    function VerifyIteratorAndContents(myIteratorString, actualIterator, myArrayString, actualArray) {
        var index = 0;
        while (actualIterator.hasCurrent) {
            verify(actualIterator.current, actualArray[index], "( " + index + ") " + myIteratorString);
            actualIterator.moveNext();
            index++;
        }
        verify(index, actualArray.length, "Number of elements in " + myIteratorString);
    }

    function VerifyIterableAndContents(myIterableString, actualIterable, myArrayString, actualArray) {
        VerifyIteratorAndContents(myIterableString + ".first()", actualIterable.first(), myArrayString, actualArray);
    }

    function verifyMapOfStructAndVector(myMapString, myMap, myExpectedMap) {
        // Verify the toString()
        verify(myMap.toString(), myExpectedMap.toString(), myMapString + ".toString()");

        // Check the sizes
        verify(myMap.size, myExpectedMap.size, myMapString + ".size");

        // Loop through each item in the map
        var myMapIterator = myMap.first();
        var myExpectedMapIterator = myExpectedMap.first();
        verify(myMapIterator.toString(), myExpectedMapIterator.toString(), myMapString + ".first().toString()");

        var i = 1;
        while (myExpectedMapIterator.hasCurrent) {
            verify(myMapIterator.current.toString(), myExpectedMapIterator.current.toString(), myMapString + ".first().current.toString()");

            // Key
            var myDimension = myMapIterator.current.key;
            var myExpectedDimension = myExpectedMapIterator.current.key;

            verify(myMapIterator.current.key.toString(), myExpectedMapIterator.current.key.toString(), myMapString + ".first().current.key.toString()");
            verify(myDimension.length, myExpectedDimension.length, '(Key: ' + i + ') myDimension.length');
            verify(myDimension.width, myExpectedDimension.width, '(Key: ' + i + ') myDimension.width');

            // Value
            var myVector = myMapIterator.current.value;
            var myExpectedVector = myExpectedMapIterator.current.value;
            verify(myMapIterator.current.value.toString(), myExpectedMapIterator.current.value.toString(), myMapString + ".first().current.value.toString()");
            VerifyArrayAndVectorContents('(Value: ' + i + ') myVector', myVector, '(Value: ' + i + ') myExpectedVector', myExpectedVector);

            // Go to next key value pair
            myMapIterator.moveNext();
            myExpectedMapIterator.moveNext();
            i++;
        }
    }

    runner.addTest({
        id: 1,
        desc: 'Inspectable_In: null',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testNull_InspectableIn(null);
            verify(outVar.isValidType, true, "testNull_InspectableIn(null).isValidType");
        }
    });

    runner.addTest({
        id: 2,
        desc: 'Inspectable_In: undefined',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testNull_InspectableIn(undefined);
            verify(outVar.isValidType, true, "testNull_InspectableIn(undefined).isValidType");
        }
    });

    runner.addTest({
        id: 3,
        desc: 'Inspectable_In: Boolean',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testBoolean_InspectableIn(true);
            verify(outVar.isValidType, true, "testBoolean_InspectableIn(true).isValidType");
            verify(outVar.outValue, true, "testBoolean_InspectableIn(true).outValue");

            outVar = propertyValueTests.testBoolean_InspectableIn(false);
            verify(outVar.isValidType, true, "testBoolean_InspectableIn(false).isValidType");
            verify(outVar.outValue, false, "testBoolean_InspectableIn(false).outValue");

            outVar = propertyValueTests.testBoolean_InspectableIn(new Boolean(0));
            verify(outVar.isValidType, true, "testBoolean_InspectableIn(new Boolean(0)).isValidType");
            verify(outVar.outValue, false, "testBoolean_InspectableIn(new Boolean(0)).outValue");

            outVar = propertyValueTests.testBoolean_InspectableIn(new Boolean(1));
            verify(outVar.isValidType, true, "testBoolean_InspectableIn(new Boolean(1)).isValidType");
            verify(outVar.outValue, true, "testBoolean_InspectableIn(new Boolean(1)).outValue");

            outVar = propertyValueTests.testBoolean_InspectableIn(new Boolean(""));
            verify(outVar.isValidType, true, 'testBoolean_InspectableIn(new Boolean("")).isValidType');
            verify(outVar.outValue, false, 'testBoolean_InspectableIn(new Boolean("")).outValue');

            outVar = propertyValueTests.testBoolean_InspectableIn(new Boolean(null));
            verify(outVar.isValidType, true, "testBoolean_InspectableIn(new Boolean(null)).isValidType");
            verify(outVar.outValue, false, "testBoolean_InspectableIn(new Boolean(null)).outValue");

            outVar = propertyValueTests.testBoolean_InspectableIn(new Boolean(NaN));
            verify(outVar.isValidType, true, "testBoolean_InspectableIn(new Boolean(NaN)).isValidType");
            verify(outVar.outValue, false, "testBoolean_InspectableIn(new Boolean(NaN)).outValue");

            outVar = propertyValueTests.testBoolean_InspectableIn(new Boolean("false"));
            verify(outVar.isValidType, true, 'testBoolean_InspectableIn(new Boolean("false")).isValidType');
            verify(outVar.outValue, true, 'testBoolean_InspectableIn(new Boolean("false")).outValue');
        }
    });

    runner.addTest({
        id: 4,
        desc: 'Inspectable_In: String',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testString_InspectableIn("String1");
            verify(outVar.isValidType, true, 'testString_InspectableIn("String1").isValidType');
            verify(outVar.outValue, "String1", 'testString_InspectableIn("String1").outValue');

            outVar = propertyValueTests.testString_InspectableIn("");
            verify(outVar.isValidType, true, 'testString_InspectableIn("").isValidType');
            verify(outVar.outValue, "", 'testString_InspectableIn("").outValue');

            outVar = propertyValueTests.testString_InspectableIn(new String("String2"));
            verify(outVar.isValidType, true, 'testString_InspectableIn(new String("String2")).isValidType');
            verify(outVar.outValue, "String2", 'testString_InspectableIn(new String("String2")).outValue');

            outVar = propertyValueTests.testString_InspectableIn(new String(""));
            verify(outVar.isValidType, true, 'testString_InspectableIn(new String("")).isValidType');
            verify(outVar.outValue, "", 'testString_InspectableIn(new String("")).outValue');
        }
    });

    runner.addTest({
        id: 5,
        desc: 'Inspectable_In: Number',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testNumber_InspectableIn(15);
            verify(outVar.isValidType, true, 'testNumber_InspectableIn(15).isValidType');
            verify(outVar.outValue, 15, 'testNumber_InspectableIn(15).outValue');

            outVar = propertyValueTests.testNumber_InspectableIn(10.2);
            verify(outVar.isValidType, true, 'testNumber_InspectableIn(10.2).isValidType');
            verify(outVar.outValue, 10.2, 'testNumber_InspectableIn(10.2).outValue');

            outVar = propertyValueTests.testNumber_InspectableIn(0);
            verify(outVar.isValidType, true, 'testNumber_InspectableIn(0).isValidType');
            verify(outVar.outValue, 0, 'testNumber_InspectableIn(0).outValue');

            outVar = propertyValueTests.testNumber_InspectableIn(new Number(15));
            verify(outVar.isValidType, true, 'testNumber_InspectableIn(new Number(15)).isValidType');
            verify(outVar.outValue, 15, 'testNumber_InspectableIn(new Number(15)).outValue');

            outVar = propertyValueTests.testNumber_InspectableIn(new Number(10.2));
            verify(outVar.isValidType, true, 'testNumber_InspectableIn(new Number(10.2)).isValidType');
            verify(outVar.outValue, 10.2, 'testNumber_InspectableIn(new Number(10.2)).outValue');

            outVar = propertyValueTests.testNumber_InspectableIn(new Number(0));
            verify(outVar.isValidType, true, 'testNumber_InspectableIn(new Number(0)).isValidType');
            verify(outVar.outValue, 0, 'testNumber_InspectableIn(new Number(0)).outValue');
        }
    });

    runner.addTest({
        id: 6,
        desc: 'Inspectable_In: Date',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // Winrt Date
            var myElephant = new Animals.Elephant(1);
            var winrtDate = myElephant.getAge();
            var outVar = propertyValueTests.testDate_InspectableIn(winrtDate);
            verify(outVar.isValidType, true, 'testDate_InspectableIn(winrtDate).isValidType');
            verify(outVar.outValue, winrtDate, 'testDate_InspectableIn(winrtDate).outValue');

            // Js Date:
            var jsDate = new Date(79, 5, 24, 11, 33, 0);
            outVar = propertyValueTests.testDate_InspectableIn(jsDate);
            verify(outVar.isValidType, true, 'testDate_InspectableIn(jsDate).isValidType');
            verify(outVar.outValue, jsDate, 'testDate_InspectableIn(jsDate).outValue');
        }
    });

    runner.addTest({
        id: 7,
        desc: 'Inspectable_In: unboxed inspectable',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            var myAnimal = new Animals.Animal(1);
            var outVar = propertyValueTests.testInspectable_InspectableIn(myAnimal);
            verify(outVar.isValidType, true, 'testInspectable_InspectableIn(new Animals.Animal(1)).isValidType');
            verify(outVar.outValue, myAnimal, 'testInspectable_InspectableIn(new Animals.Animal(1)).outValue');

            var myVector = myAnimal.getVector();
            outVar = propertyValueTests.testInspectable_InspectableIn(myVector);
            verify(outVar.isValidType, true, 'testInspectable_InspectableIn(myVector).isValidType');
            verify(outVar.outValue, myVector, 'testInspectable_InspectableIn(myVector).outValue');
        }
    });

    runner.addTest({
        id: 8,
        desc: 'Inspectable_In: Js Arrays',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // Js Array
            var myArray = [true, new Date(20, 20, 20), "This is Date Entry"];
            var outVar = propertyValueTests.testArray_InspectableIn(myArray);
            verify(outVar.isValidType, true, 'testArray_InspectableIn([true, new Date(20, 20, 20), "This is Date Entry"]).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testArray_InspectableIn([true, new Date(20, 20, 20), "This is Date Entry"]).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testArray_InspectableIn([true, new Date(20, 20, 20), "This is Date Entry"]).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 9,
        desc: 'Inspectable_In: ES5 Arrays',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myES5Array = [false, 30.5];
            Object.defineProperty(myES5Array, "2", { set: function (x) { value = x; }, get: function () { return value }, configurable: true });
            myES5Array[2] = "This is number Entry"
            var outVar = propertyValueTests.testArray_InspectableIn(myES5Array);
            verify(outVar.isValidType, true, 'testArray_InspectableIn(myES5Array).isValidType');
            verify(outVar.outValue.length, myES5Array.length, 'testArray_InspectableIn(myES5Array).outValue.length');
            for (var i = 0; i < myES5Array.length; i++) {
                verify(outVar.outValue[i], myES5Array[i], 'testArray_InspectableIn(myES5Array).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 10,
        desc: 'Inspectable_In: GUID Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveGuidArray();
            var outVar = propertyValueTests.testGuidArray_InspectableIn(myArray);
            verify(outVar.isValidType, true, 'testGuidArray_InspectableIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testGuidArray_InspectableIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testGuidArray_InspectableIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 11,
        desc: 'Inspectable_In: Class (Animal) Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveAnimalArray();
            var outVar = propertyValueTests.testArray_InspectableIn(myArray);
            verify(outVar.isValidType, true, 'testArray_InspectableIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testArray_InspectableIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testArray_InspectableIn(myArray).outValue[' + i + ']');
                if (myArray[i] !== null) {
                    verify(outVar.outValue[i].getGreeting(), myArray[i].getGreeting(), 'testArray_InspectableIn(myArray).outValue[' + i + '].getGreeting()');
                }
            }
        }
    });

    runner.addTest({
        id: 12,
        desc: 'Inspectable_In: IFish Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveFishArray();
            var outVar = propertyValueTests.testArray_InspectableIn(myArray);
            verify(outVar.isValidType, true, 'testArray_InspectableIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testArray_InspectableIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testArray_InspectableIn(myArray).outValue[' + i + ']');
                if (myArray[i] !== null) {
                    verify(outVar.outValue[i].name, myArray[i].name, 'testArray_InspectableIn(myArray).outValue[' + i + '].name');
                }
            }
        }
    });

    runner.addTest({
        id: 13,
        desc: 'Inspectable_In: IVector<int> Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveVectorArray();
            var outVar = propertyValueTests.testArray_InspectableIn(myArray);
            verify(outVar.isValidType, true, 'testArray_InspectableIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testArray_InspectableIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testArray_InspectableIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 14,
        desc: 'Inspectable_In: Date Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveDateArray();
            var outVar = propertyValueTests.testDateArray_InspectableIn(myArray);
            verify(outVar.isValidType, true, 'testDateArray_InspectableIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testDateArray_InspectableIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testDateArray_InspectableIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 15,
        desc: 'Inspectable_In: boolean Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveBooleanArray();
            var outVar = propertyValueTests.testBooleanArray_InspectableIn(myArray);
            verify(outVar.isValidType, true, 'testBooleanArray_InspectableIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testBooleanArray_InspectableIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testBooleanArray_InspectableIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 16,
        desc: 'Inspectable_In: string Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveStringArray();
            var outVar = propertyValueTests.testStringArray_InspectableIn(myArray);
            verify(outVar.isValidType, true, 'testStringArray_InspectableIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testStringArray_InspectableIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testStringArray_InspectableIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 17,
        desc: 'Inspectable_In: inspectable Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveInspectableArray();
            var outVar = propertyValueTests.testArray_InspectableIn(myArray);
            verify(outVar.isValidType, true, 'testArray_InspectableIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testArray_InspectableIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testArray_InspectableIn(myArray).outValue[' + i + ']');
                if (i === 0) {
                    verify(outVar.outValue[0].getGreeting(), myArray[0].getGreeting(), 'testArray_InspectableIn(myArray).outValue[0].getGreeting()');
                }
                else if (i === 2) {
                    verify(outVar.outValue[2].name, myArray[2].name, 'testArray_InspectableIn(myArray).outValue[2].name');
                }
            }
        }
    });

    runner.addTest({
        id: 18,
        desc: 'Inspectable_In: char16 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveChar16Array();
            var outVar = propertyValueTests.testChar16Array_InspectableIn(myArray);
            verify(outVar.isValidType, true, 'testChar16Array_InspectableIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testChar16Array_InspectableIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testChar16Array(myArray)_InspectableIn.outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 19,
        desc: 'Inspectable_In: Uint8 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveUInt8Array();
            var outVar = propertyValueTests.testUInt8Array_InspectableIn(myArray);
            verify(outVar.isValidType, true, 'testUInt8Array_InspectableIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testUInt8Array_InspectableIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testUInt8Array_InspectableIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 20,
        desc: 'Inspectable_In: int16 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveInt16Array();
            var outVar = propertyValueTests.testInt16Array_InspectableIn(myArray);
            verify(outVar.isValidType, true, 'testInt16Array_InspectableIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testInt16Array_InspectableIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testInt16Array_InspectableIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 21,
        desc: 'Inspectable_In: uint16 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveUInt16Array();
            var outVar = propertyValueTests.testUInt16Array_InspectableIn(myArray);
            verify(outVar.isValidType, true, 'testUInt16Array_InspectableIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testUInt16Array_InspectableIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testUInt16Array_InspectableIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 22,
        desc: 'Inspectable_In: int32 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveInt32Array();
            var outVar = propertyValueTests.testInt32Array_InspectableIn(myArray);
            verify(outVar.isValidType, true, 'testInt32Array_InspectableIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testInt32Array_InspectableIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testInt32Array_InspectableIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 23,
        desc: 'Inspectable_In: uint32 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveUInt32Array();
            var outVar = propertyValueTests.testUInt32Array_InspectableIn(myArray);
            verify(outVar.isValidType, true, 'testUInt32Array_InspectableIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testUInt32Array_InspectableIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testUInt32Array_InspectableIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 24,
        desc: 'Inspectable_In: int64 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveInt64Array();
            var outVar = propertyValueTests.testInt64Array_InspectableIn(myArray);
            verify(outVar.isValidType, true, 'testInt64Array_InspectableIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testInt64Array_InspectableIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testInt64Array_InspectableIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 25,
        desc: 'Inspectable_In: uint64 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveUInt64Array();
            var outVar = propertyValueTests.testUInt64Array_InspectableIn(myArray);
            verify(outVar.isValidType, true, 'testUInt64Array_InspectableIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testUInt64Array_InspectableIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testUInt64Array_InspectableIn(myArray).outValue[' + i + ']');
            }
        }
    });


    runner.addTest({
        id: 26,
        desc: 'Inspectable_In: float Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveFloatArray();
            var outVar = propertyValueTests.testFloatArray_InspectableIn(myArray);
            verify(outVar.isValidType, true, 'testFloatArray_InspectableIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testFloatArray_InspectableIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testFloatArray_InspectableIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 27,
        desc: 'Inspectable_In: double Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveDoubleArray();
            var outVar = propertyValueTests.testDoubleArray_InspectableIn(myArray);
            verify(outVar.isValidType, true, 'testDoubleArray_InspectableIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testDoubleArray_InspectableIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testDoubleArray_InspectableIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 28,
        desc: 'Inspectable_In: Struct from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myStruct = {
                length: 10,
                width: 40
            };
            verify.exception(function () {
                var outVar = propertyValueTests.testInspectable_InspectableIn(myStruct);
            }, TypeError, "testInspectable_InspectableIn(myStruct)");
        }
    });

    runner.addTest({
        id: 29,
        desc: 'Inspectable_In: Enum from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testNumber_InspectableIn(Animals.Phylum.cnidaria);
            verify(outVar.isValidType, true, 'testNumber_InspectableIn(Animals.Phylum.cnidaria).isValidType');
            verify(outVar.outValue, Animals.Phylum.cnidaria, 'testNumber_InspectableIn(Animals.Phylum.cnidaria).outValue');
        }
    });

    runner.addTest({
        id: 30,
        desc: 'Inspectable_In: Delegate from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myWinRTDelegate = (new Animals.Animal(1)).getNativeDelegateAsOutParam();
            verify.exception(function () {
                var outVar = propertyValueTests.testInspectable_InspectableIn(myWinRTDelegate);
            }, TypeError, "testInspectable_InspectableIn(myWinRTDelegate)");
        }
    });

    runner.addTest({
        id: 31,
        desc: 'Inspectable_In: Js Delegate from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            function myJSDelegate() {
            }
            verify.exception(function () {
                var outVar = propertyValueTests.testInspectable_InspectableIn(myJSDelegate);
            }, TypeError, "testInspectable_InspectableIn(myJSDelegate)");
        }
    });

    runner.addTest({
        id: 32,
        desc: 'Inspectable_In: Struct Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveStructArray();
            var outVar = propertyValueTests.testDimensionsArray_InspectableIn(myArray);
            verify(outVar.isValidType, true, 'testDimensionsArray_InspectableIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testDimensionsArray_InspectableIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i].length, myArray[i].length, 'testDimensionsArray_InspectableIn(myArray).outValue[' + i + '].length');
                verify(outVar.outValue[i].width, myArray[i].width, 'testDimensionsArray_InspectableIn(myArray).outValue[' + i + '].width');
            }
        }
    });

    runner.addTest({
        id: 33,
        desc: 'Inspectable_In: Enum Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveEnumArray();
            var outVar = propertyValueTests.testEnumArray_InspectableIn(myArray);
            verify(outVar.isValidType, true, 'testEnumArray_InspectableIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testEnumArray_InspectableIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testEnumArray_InspectableIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 34,
        desc: 'Inspectable_In: TimeSpan Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveTimeSpanArray();
            var outVar = propertyValueTests.testTimeSpanArray_InspectableIn(myArray);
            verify(outVar.isValidType, true, 'testTimeSpanArray_InspectableIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testTimeSpanArray_InspectableIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testTimeSpanArray_InspectableIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 35,
        desc: 'Inspectable_In: Point Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receivePointArray();
            var outVar = propertyValueTests.testPointArray_InspectableIn(myArray);
            verify(outVar.isValidType, true, 'testPointArray_InspectableIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testPointArray_InspectableIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i].x, myArray[i].x, 'testPointArray_InspectableIn(myArray).outValue[' + i + '].x');
                verify(outVar.outValue[i].y, myArray[i].y, 'testPointArray_InspectableIn(myArray).outValue[' + i + '].y');
            }
        }
    });

    runner.addTest({
        id: 36,
        desc: 'Inspectable_In: Size Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveSizeArray();
            var outVar = propertyValueTests.testSizeArray_InspectableIn(myArray);
            verify(outVar.isValidType, true, 'testSizeArray_InspectableIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testSizeArray_InspectableIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i].height, myArray[i].height, 'testSizeArray_InspectableIn(myArray).outValue[' + i + '].height');
                verify(outVar.outValue[i].width, myArray[i].width, 'testSizeArray_InspectableIn(myArray).outValue[' + i + '].width');
            }
        }
    });

    runner.addTest({
        id: 37,
        desc: 'Inspectable_In: Rect Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveRectArray();
            var outVar = propertyValueTests.testRectArray_InspectableIn(myArray);
            verify(outVar.isValidType, true, 'testRectArray_InspectableIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testRectArray_InspectableIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i].x, myArray[i].x, 'testRectArray_InspectableIn(myArray).outValue[' + i + '].x');
                verify(outVar.outValue[i].y, myArray[i].y, 'testRectArray_InspectableIn(myArray).outValue[' + i + '].y');
                verify(outVar.outValue[i].height, myArray[i].height, 'testRectArray_InspectableIn(myArray).outValue[' + i + '].height');
                verify(outVar.outValue[i].width, myArray[i].width, 'testRectArray_InspectableIn(myArray).outValue[' + i + '].width');
            }
        }
    });

    runner.addTest({
        id: 38,
        desc: 'Inspectable_In: Winrt Delegate Array from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveWinrtDelegateArray();
            var outVar = propertyValueTests.testDelegateArray_InspectableIn(myArray, myArray);
            verify(outVar.isValidType, true, 'testDelegateArray_InspectableIn(myArray, myArray).isValidType');
            for (var i = 0; i < myArray.length; i++) {
                verify(propertyValueTests.isSameDelegate(outVar.outValue[i], myArray[i]), true, 'testDelegateArray_InspectableIn(myArray).outValue[' + i + '] are same');
            }
        }
    });

    runner.addTest({
        id: 39,
        desc: 'Inspectable_In: Js Delegate Array from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            function delegate1() {
                return "delegate1";
            }

            function delegate2() {
                return "delegate2";
            }

            var myArray = propertyValueTests.receiveJSDelegateArray(delegate1, delegate2);
            var outVar = propertyValueTests.testDelegateArray_InspectableIn(myArray);
            verify(outVar.outValue.length, myArray.length, 'testDelegateArray_InspectableIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testDelegateArray_InspectableIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 40,
        desc: 'Inspectable_Out: null',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testNull_InspectableOut();
            verify(outVar, null, "testNull_InspectableOut()");
        }
    });

    runner.addTest({
        id: 41,
        desc: 'Inspectable_Out: Boolean',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testBoolean_InspectableOut(true);
            verify(outVar, true, "testBoolean_InspectableOut(true)");

            outVar = propertyValueTests.testBoolean_InspectableOut(false);
            verify(outVar, false, "testBoolean_InspectableOut(false)");
        }
    });

    runner.addTest({
        id: 42,
        desc: 'Inspectable_Out: String',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testString_InspectableOut("String1");
            verify(outVar, "String1", 'testString_InspectableOut("String1")');

            outVar = propertyValueTests.testString_InspectableOut("");
            verify(outVar, "", 'testString_InspectableOut("")');
        }
    });

    runner.addTest({
        id: 43,
        desc: 'Inspectable_Out: Char16',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testChar16_InspectableOut("S");
            verify(outVar, "S", 'testChar16_InspectableOut("S")');
        }
    });

    runner.addTest({
        id: 44,
        desc: 'Inspectable_Out: UInt8',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testUInt8_InspectableOut(2);
            verify(outVar, 2, 'testUInt8_InspectableOut(2)');
        }
    });

    runner.addTest({
        id: 45,
        desc: 'Inspectable_Out: Int16',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testInt16_InspectableOut(2);
            verify(outVar, 2, 'testInt16_InspectableOut(2)');
        }
    });

    runner.addTest({
        id: 46,
        desc: 'Inspectable_Out: UInt16',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testUInt16_InspectableOut(2);
            verify(outVar, 2, 'testUInt16_InspectableOut(2)');
        }
    });

    runner.addTest({
        id: 47,
        desc: 'Inspectable_Out: Int32',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testInt32_InspectableOut(2);
            verify(outVar, 2, 'testInt32_InspectableOut(2)');
        }
    });

    runner.addTest({
        id: 48,
        desc: 'Inspectable_Out: UInt32',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testUInt32_InspectableOut(2);
            verify(outVar, 2, 'testUInt32_InspectableOut(2)');
        }
    });

    runner.addTest({
        id: 49,
        desc: 'Inspectable_Out: Int64',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testInt64_InspectableOut(2);
            verify(outVar, 2, 'testInt64_InspectableOut(2)');
        }
    });

    runner.addTest({
        id: 50,
        desc: 'Inspectable_Out: UInt64',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testUInt64_InspectableOut(2);
            verify(outVar, 2, 'testUInt64_InspectableOut(2)');
        }
    });

    runner.addTest({
        id: 51,
        desc: 'Inspectable_Out: Float',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testFloat_InspectableOut(2);
            verify(outVar, 2, 'testFloat_InspectableOut(2)');
        }
    });

    runner.addTest({
        id: 52,
        desc: 'Inspectable_Out: Double',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testDouble_InspectableOut(2);
            verify(outVar, 2, 'testDouble_InspectableOut(2)');
        }
    });

    runner.addTest({
        id: 53,
        desc: 'Inspectable_Out: Guid',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testGuid_InspectableOut("{10C27311-B58F-476C-BFF4-6689B8A19836}");
            verify(outVar, "10c27311-b58f-476c-bff4-6689b8a19836", 'testGuid_InspectableOut("{10C27311-B58F-476C-BFF4-6689B8A19836}")');
        }
    });

    runner.addTest({
        id: 54,
        desc: 'Inspectable_Out: Date',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // Winrt Date
            var myElephant = new Animals.Elephant(1);
            var winrtDate = myElephant.getAge();
            var outVar = propertyValueTests.testDate_InspectableOut(winrtDate);
            verify(outVar, winrtDate, 'testDate_InspectableOut(winrtDate)');

            // Js Date:
            var jsDate = new Date(79, 5, 24, 11, 33, 0);
            outVar = propertyValueTests.testDate_InspectableOut(jsDate);
            verify(outVar, jsDate, 'testDate_InspectableOut(jsDate)');
        }
    });

    runner.addTest({
        id: 55,
        desc: 'Inspectable_Out: GUID Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveGuidArray();
            var outVar = propertyValueTests.testGuidArray_InspectableOut(myArray);
            verify(outVar.length, myArray.length, 'testGuidArray_InspectableOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testGuidArray_InspectableOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 56,
        desc: 'Inspectable_Out: Date Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveDateArray();
            var outVar = propertyValueTests.testDateArray_InspectableOut(myArray);
            verify(outVar.length, myArray.length, 'testDateArray_InspectableOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testDateArray_InspectableOut(myArray)[' + i + ']');
            }
        }
    });


    runner.addTest({
        id: 57,
        desc: 'Inspectable_Out: Boolean Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveBooleanArray();
            var outVar = propertyValueTests.testBooleanArray_InspectableOut(myArray);
            verify(outVar.length, myArray.length, 'testBooleanArray_InspectableOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testBooleanArray_InspectableOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 58,
        desc: 'Inspectable_Out: String Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveStringArray();
            var outVar = propertyValueTests.testStringArray_InspectableOut(myArray);
            verify(outVar.length, myArray.length, 'testStringArray_InspectableOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testStringArray_InspectableOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 59,
        desc: 'Inspectable_Out: Char16 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveChar16Array();
            var outVar = propertyValueTests.testChar16Array_InspectableOut(myArray);
            verify(outVar.length, myArray.length, 'testChar16Array_InspectableOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testChar16Array_InspectableOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 60,
        desc: 'Inspectable_Out: UInt8 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveUInt8Array();
            var outVar = propertyValueTests.testUInt8Array_InspectableOut(myArray);
            verify(outVar.length, myArray.length, 'testUInt8Array_InspectableOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testUInt8Array_InspectableOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 61,
        desc: 'Inspectable_Out: Int16 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveInt16Array();
            var outVar = propertyValueTests.testInt16Array_InspectableOut(myArray);
            verify(outVar.length, myArray.length, 'testInt16Array_InspectableOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testInt16Array_InspectableOut(myArray)[' + i + ']');
            }
        }
    });


    runner.addTest({
        id: 62,
        desc: 'Inspectable_Out: UInt16 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveUInt16Array();
            var outVar = propertyValueTests.testUInt16Array_InspectableOut(myArray);
            verify(outVar.length, myArray.length, 'testUInt16Array_InspectableOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testUInt16Array_InspectableOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 63,
        desc: 'Inspectable_Out: Int32 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveInt32Array();
            var outVar = propertyValueTests.testInt32Array_InspectableOut(myArray);
            verify(outVar.length, myArray.length, 'testInt32Array_InspectableOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testInt32Array_InspectableOut(myArray)[' + i + ']');
            }
        }
    });


    runner.addTest({
        id: 64,
        desc: 'Inspectable_Out: UInt32 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveUInt32Array();
            var outVar = propertyValueTests.testUInt32Array_InspectableOut(myArray);
            verify(outVar.length, myArray.length, 'testUInt32Array_InspectableOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testUInt32Array_InspectableOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 65,
        desc: 'Inspectable_Out: Int64 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveInt64Array();
            var outVar = propertyValueTests.testInt64Array_InspectableOut(myArray);
            verify(outVar.length, myArray.length, 'testInt64Array_InspectableOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testInt64Array_InspectableOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 66,
        desc: 'Inspectable_Out: UInt64 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveUInt64Array();
            var outVar = propertyValueTests.testUInt64Array_InspectableOut(myArray);
            verify(outVar.length, myArray.length, 'testUInt64Array_InspectableOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testUInt64Array_InspectableOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 67,
        desc: 'Inspectable_Out: Float Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveFloatArray();
            var outVar = propertyValueTests.testFloatArray_InspectableOut(myArray);
            verify(outVar.length, myArray.length, 'testFloatArray_InspectableOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testFloatArray_InspectableOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 68,
        desc: 'Inspectable_Out: Double Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveDoubleArray();
            var outVar = propertyValueTests.testDoubleArray_InspectableOut(myArray);
            verify(outVar.length, myArray.length, 'testDoubleArray_InspectableOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testDoubleArray_InspectableOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 69,
        desc: 'Inspectable_Out: Inspectable Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveInspectableArray();
            var outVar = propertyValueTests.testArray_InspectableOut(myArray);
            verify(outVar.length, myArray.length, 'testArray_InspectableOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testArray_InspectableOut(myArray)[' + i + ']');
                if (i === 0) {
                    verify(outVar[0].getGreeting(), myArray[0].getGreeting(), 'testArray_InspectableOut(myArray)[0].getGreeting()');
                }
                else if (i === 2) {
                    verify(outVar[2].name, myArray[2].name, 'testArray_InspectableOut(myArray)[2].name');
                }
            }
        }
    });

    runner.addTest({
        id: 70,
        desc: 'Inspectable_Out: Class (Animal) Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveAnimalArray();
            var outVar = propertyValueTests.testAnimalArray_InspectableOut(myArray);
            verify(outVar.length, myArray.length, 'testAnimalArray_InspectableOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testAnimalArray_InspectableOut(myArray)[' + i + ']');
                if (myArray[i] !== null) {
                    verify(outVar[i].getGreeting(), myArray[i].getGreeting(), 'testAnimalArray_InspectableOut(myArray)[' + i + '].getGreeting()');
                }
            }
        }
    });

    runner.addTest({
        id: 71,
        desc: 'Inspectable_Out: IFish Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveFishArray();
            var outVar = propertyValueTests.testFishArray_InspectableOut(myArray);
            verify(outVar.length, myArray.length, 'testFishArray_InspectableOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testFishArray_InspectableOut(myArray)[' + i + ']');
                if (myArray[i] !== null) {
                    verify(outVar[i].name, myArray[i].name, 'testFishArray_InspectableOut(myArray)[' + i + '].name');
                }
            }
        }
    });

    runner.addTest({
        id: 72,
        desc: 'Inspectable_Out: IVector<int> Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveVectorArray();
            var outVar = propertyValueTests.testVectorArray_InspectableOut(myArray);
            verify(outVar.length, myArray.length, 'testVectorArray_InspectableOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testVectorArray_InspectableOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 73,
        desc: 'Inspectable_Out: Js Arrays that are marshaled into inspectable arrays',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = [true, new Date(20, 20, 20), "This is Date Entry"];
            var outVar = propertyValueTests.testArray_InspectableOut(myArray);
            verify(outVar.length, myArray.length, 'testArray_InspectableOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testArray_InspectableOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 74,
        desc: 'Inspectable_Out: ES5 Arrays that are marshaled into inspectable arrays',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = [false, 30.5];
            Object.defineProperty(myArray, "2", { set: function (x) { value = x; }, get: function () { return value }, configurable: true }); var outVar = propertyValueTests.testArray_InspectableOut(myArray);
            myArray[2] = "This is number Entry"
            verify(outVar.length, myArray.length, 'testArray_InspectableOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testArray_InspectableOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 75,
        desc: 'Inspectable_Out: Box inspectable in winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            var myAnimal = new Animals.Animal(1);
            var outVar = propertyValueTests.testBoxInspectable_InspectableOut(myAnimal);
            verify(outVar, myAnimal, 'testBoxInspectable_InspectableOut(myAnimal)');

            var myVector = myAnimal.getVector();
            outVar = propertyValueTests.testBoxInspectable_InspectableOut(myVector);
            verify(outVar, myVector, 'testBoxInspectable_InspectableOut(myVector)');
        }
    });

    runner.addTest({
        id: 76,
        desc: 'Inspectable_Out: Plain inspectable in and out',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            var myAnimal = new Animals.Animal(1);
            var outVar = propertyValueTests.testInspectable_InspectableOut(myAnimal);
            verify(outVar, myAnimal, 'testInspectable_InspectableOut(myAnimal)');

            var myVector = myAnimal.getVector();
            outVar = propertyValueTests.testInspectable_InspectableOut(myVector);
            verify(outVar, myVector, 'testInspectable_InspectableOut(myVector)');
        }
    });

    runner.addTest({
        id: 77,
        desc: 'Inspectable_Out: IVector<IVector<int>>',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // Check we can marshal out as Inspectable
            var myVectorOfVectorInspectable = propertyValueTests.receiveVectorOfVector_InspectableOut();
            var myVectorOfVector = propertyValueTests.receiveVectorOfVector();
            for (var i = 0; i < myVectorOfVector.length; i++) {
                VerifyArrayAndVectorContents('myVectorOfVectorInspectable[' + i + ']', myVectorOfVectorInspectable[i], 'myVectorOfVector[' + i + ']', myVectorOfVector[i]);
            }

            // Box and get it back
            var outVar = propertyValueTests.testBoxInspectable_InspectableOut(myVectorOfVector);
            verify(outVar, myVectorOfVector, 'testBoxInspectable_InspectableOut(myVectorOfVector)');
            for (var i = 0; i < myVectorOfVector.length; i++) {
                verify(outVar[i], myVectorOfVector[i], 'testBoxInspectable_InspectableOut(myVectorOfVector)[' + i + ']');
            }

            // Get same vector back
            outVar = propertyValueTests.testInspectable_InspectableOut(myVectorOfVector);
            verify(outVar, myVectorOfVector, 'testInspectable_InspectableOut(myVectorOfVector)');
            for (var i = 0; i < myVectorOfVector.length; i++) {
                verify(outVar[i], myVectorOfVector[i], 'testInspectable_InspectableOut(myVectorOfVector)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 78,
        desc: 'Inspectable_Out: IVector<Dimensions>',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // Check we can marshal out as Inspectable
            var myVectorOfStructInspectable = propertyValueTests.receiveVectorOfStruct_InspectableOut();
            var myVectorOfStruct = propertyValueTests.receiveVectorOfStruct();
            for (var i = 0; i < myVectorOfStruct.length; i++) {
                verify(myVectorOfStructInspectable[i].length, myVectorOfStruct[i].length, 'myVectorOfStructInspectable[' + i + '].length');
                verify(myVectorOfStructInspectable[i].width, myVectorOfStruct[i].width, 'myVectorOfStructInspectable[' + i + '].width');
            }

            // Box and get it back
            var outVar = propertyValueTests.testBoxInspectable_InspectableOut(myVectorOfStruct);
            verify(outVar, myVectorOfStruct, 'testBoxInspectable_InspectableOut(myVectorOfStruct)');
            for (var i = 0; i < myVectorOfStruct.length; i++) {
                verify(outVar[i], myVectorOfStruct[i], 'testBoxInspectable_InspectableOut(myVectorOfStruct)[' + i + ']');
            }

            // Get same vector back
            outVar = propertyValueTests.testInspectable_InspectableOut(myVectorOfStruct);
            verify(outVar, myVectorOfStruct, 'testInspectable_InspectableOut(myVectorOfStruct)');
            for (var i = 0; i < myVectorOfStruct.length; i++) {
                verify(outVar[i], myVectorOfStruct[i], 'testInspectable_InspectableOut(myVectorOfStruct)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 79,
        desc: 'Inspectable_Out: IMap<Dimension, IVector<HSTRING>>',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // Check we can marshal out as Inspectable
            var myMapOfStructAndVectorInspectable = propertyValueTests.receiveMapOfStructAndVector_InspectableOut();
            var myMapOfStructAndVector = propertyValueTests.receiveMapOfStructAndVector();
            verifyMapOfStructAndVector("myMapOfStructAndVectorInspectable", myMapOfStructAndVectorInspectable, myMapOfStructAndVector);

            // Box and get it back
            var outVar = propertyValueTests.testBoxInspectable_InspectableOut(myMapOfStructAndVector);
            verify(outVar, myMapOfStructAndVector, 'testBoxInspectable_InspectableOut(myMapOfStructAndVector)');
            verifyMapOfStructAndVector("testBoxInspectable_InspectableOut(myMapOfStructAndVector)", outVar, myMapOfStructAndVector);
            
            // Get same vector back
            outVar = propertyValueTests.testInspectable_InspectableOut(myMapOfStructAndVector);
            verify(outVar, myMapOfStructAndVector, 'testInspectable_InspectableOut(myMapOfStructAndVector)');
            verifyMapOfStructAndVector("testInspectable_InspectableOut(myMapOfStructAndVector)", outVar, myMapOfStructAndVector);
        }
    });

    runner.addTest({
        id: 80,
        desc: 'Inspectable_Out: IVector<RCObservableVector>',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // Check we can marshal out as Inspectable
            var myVectorOfRCObservableVectorInspectable = propertyValueTests.receiveVectorOfRCObservableVector_InspectableOut();
            var myVectorOfRCObservableVector = propertyValueTests.receiveVectorOfRCObservableVector();
            for (var i = 0; i < myVectorOfRCObservableVector.length; i++) {
                VerifyArrayAndVectorContents('myVectorOfRCObservableVectorInspectable[' + i + ']', myVectorOfRCObservableVectorInspectable[i], 'myVectorOfRCObservableVector[' + i + ']', myVectorOfRCObservableVector[i]);
            }

            // Box and get it back
            var outVar = propertyValueTests.testBoxInspectable_InspectableOut(myVectorOfRCObservableVector);
            verify(outVar, myVectorOfRCObservableVector, 'testBoxInspectable_InspectableOut(myVectorOfRCObservableVector)');
            for (var i = 0; i < myVectorOfRCObservableVector.length; i++) {
                verify(outVar[i], myVectorOfRCObservableVector[i], 'testBoxInspectable_InspectableOut(myVectorOfRCObservableVector)[' + i + ']');
            }

            // Get same vector back
            outVar = propertyValueTests.testInspectable_InspectableOut(myVectorOfRCObservableVector);
            verify(outVar, myVectorOfRCObservableVector, 'testInspectable_InspectableOut(myVectorOfRCObservableVector)');
            for (var i = 0; i < myVectorOfRCObservableVector.length; i++) {
                verify(outVar[i], myVectorOfRCObservableVector[i], 'testInspectable_InspectableOut(myVectorOfRCObservableVector)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 81,
        desc: 'Inspectable_Out: Struct from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myStruct = {
                length: 10,
                width: 40
            };
            var outVar = propertyValueTests.testDimensions_InspectableOut(myStruct);
            verify(outVar.length, myStruct.length, 'testDimensions_InspectableOut(myStruct).length');
            verify(outVar.width, myStruct.width, 'testDimensions_InspectableOut(myStruct).width');
        }
    });

    runner.addTest({
        id: 82,
        desc: 'Inspectable_Out: TimeSpan',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // TimeSpan from WinRT
            var timeSpanTests = new DevTests.DateTimeAndTimeSpan.Tests();
            var winrtTimeSpan = timeSpanTests.produceTimeSpan(10000 * 1000 * 60 * 60);
            var outVar = propertyValueTests.testTimeSpan_InspectableOut(winrtTimeSpan);
            verify(outVar, winrtTimeSpan, 'testTimeSpan_InspectableOut(winrtTimeSpan)');

            // Js Number:
            var jsTimeSpan = 1000 * 60 * 60 * 24;
            outVar = propertyValueTests.testTimeSpan_InspectableOut(jsTimeSpan);
            verify(outVar, jsTimeSpan, 'testTimeSpan_InspectableOut(jsTimeSpan)');
        }
    });

    runner.addTest({
        id: 83,
        desc: 'Inspectable_Out: Point',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myPoint = {
                x: 10,
                y: 40
            };
            var outVar = propertyValueTests.testPoint_InspectableOut(myPoint);
            verify(outVar.x, myPoint.x, 'testPoint_InspectableOut(myPoint).x');
            verify(outVar.y, myPoint.y, 'testPoint_InspectableOut(myPoint).y');

        }
    });

    runner.addTest({
        id: 84,
        desc: 'Inspectable_Out: Size',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var mySize = {
                height: 10,
                width: 40
            };
            var outVar = propertyValueTests.testSize_InspectableOut(mySize);
            verify(outVar.height, mySize.height, 'testSize_InspectableOut(mySize).height');
            verify(outVar.width, mySize.width, 'testSize_InspectableOut(mySize).width');
        }
    });

    runner.addTest({
        id: 85,
        desc: 'Inspectable_Out: Rect',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myRect = {
                height: 10,
                width: 40,
                x: 1000,
                y: 3000
            };
            var outVar = propertyValueTests.testRect_InspectableOut(myRect);
            verify(outVar.x, myRect.x, 'testRect_InspectableOut(myRect).x');
            verify(outVar.y, myRect.y, 'testRect_InspectableOut(myRect).y');
            verify(outVar.height, myRect.height, 'testRect_InspectableOut(myRect).height');
            verify(outVar.width, myRect.width, 'testRect_InspectableOut(myRect).width');
        }
    });

    runner.addTest({
        id: 86,
        desc: 'Inspectable_Out: Enum',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testEnum_InspectableOut(Animals.Phylum.phoronida);
            verify(outVar, Animals.Phylum.phoronida, 'testEnum_InspectableOut(Animals.Phylum.phoronida)');
        }
    });

    runner.addTest({
        id: 87,
        desc: 'Inspectable_Out: Struct Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveStructArray();
            var outVar = propertyValueTests.testInspectable_InspectableOut(myArray);
            verify(outVar.length, myArray.length, 'testInspectable_InspectableOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i].length, myArray[i].length, 'testInspectable_InspectableOut(myArray)[' + i + '].length');
                verify(outVar[i].width, myArray[i].width, 'testInspectable_InspectableOut(myArray)[' + i + '].width');
            }
        }
    });

    runner.addTest({
        id: 88,
        desc: 'Inspectable_Out: TimeSpan Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveTimeSpanArray();
            var outVar = propertyValueTests.testTimeSpanArray_InspectableOut(myArray);
            verify(outVar.length, myArray.length, 'testTimeSpanArray_InspectableOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testTimeSpanArray_InspectableOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 89,
        desc: 'Inspectable_Out: Point Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receivePointArray();
            var outVar = propertyValueTests.testPointArray_InspectableOut(myArray);
            verify(outVar.length, myArray.length, 'testPointArray_InspectableOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i].x, myArray[i].x, 'testPointArray_InspectableOut(myArray)[' + i + '].x');
                verify(outVar[i].y, myArray[i].y, 'testPointArray_InspectableOut(myArray)[' + i + '].y');
            }
        }
    });

    runner.addTest({
        id: 90,
        desc: 'Inspectable_Out: Size Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveSizeArray();
            var outVar = propertyValueTests.testSizeArray_InspectableOut(myArray);
            verify(outVar.length, myArray.length, 'testSizeArray_InspectableOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i].height, myArray[i].height, 'testSizeArray_InspectableOut(myArray)[' + i + '].height');
                verify(outVar[i].width, myArray[i].width, 'testSizeArray_InspectableOut(myArray)[' + i + '].width');
            }
        }
    });

    runner.addTest({
        id: 91,
        desc: 'Inspectable_Out: Rect Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveRectArray();
            var outVar = propertyValueTests.testRectArray_InspectableOut(myArray);
            verify(outVar.length, myArray.length, 'testRectArray_InspectableOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i].x, myArray[i].x, 'testRectArray_InspectableOut(myArray)[' + i + '].x');
                verify(outVar[i].y, myArray[i].y, 'testRectArray_InspectableOut(myArray)[' + i + '].y');
                verify(outVar[i].height, myArray[i].height, 'testRectArray_InspectableOut(myArray)[' + i + '].height');
                verify(outVar[i].width, myArray[i].width, 'testRectArray_InspectableOut(myArray)[' + i + '].width');
            }
        }
    });

    runner.addTest({
        id: 92,
        desc: 'Inspectable_Out: Enum Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveEnumArray();
            var outVar = propertyValueTests.testInspectable_InspectableOut(myArray);
            verify(outVar.length, myArray.length, 'testInspectable_InspectableOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testInspectable_InspectableOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 93,
        desc: 'Inspectable_Out: Js Delegate Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            function delegate1() {
                return "delegate1";
            }

            function delegate2() {
                return "delegate2";
            }

            var myArray = propertyValueTests.receiveJSDelegateArray(delegate1, delegate2);
            var outVar = propertyValueTests.testInspectable_InspectableOut(myArray);
            verify(outVar.length, myArray.length, 'testInspectable_InspectableOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testInspectable_InspectableOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 94,
        desc: 'Inspectable_Out: Winrt Delegate Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveWinrtDelegateArray();
            var outVar = propertyValueTests.testInspectable_InspectableOut(myArray);
            verify(outVar.length, myArray.length, 'testInspectable_InspectableOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(propertyValueTests.isSameDelegate(outVar[i], myArray[i]), true, 'testInspectable_InspectableOut(myArray)[' + i + '] are same');
            }
        }
    });

    runner.addTest({
        id: 95,
        desc: 'Inspectable_Out: Js Array as Iterable',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = [1, 2, 3, 4];

            // Box
            var outVar = propertyValueTests.testBoxIterable_InspectableOut(myArray);
            VerifyIterableAndContents("testBoxIterable_InspectableOut(myArray)", outVar, "myArray", myArray);

            // UnBox
            outVar = propertyValueTests.testIterable_InspectableOut(myArray);
            VerifyIterableAndContents("testIterable_InspectableOut(myArray)", outVar, "myArray", myArray);
        }
    });

    runner.addTest({
        id: 96,
        desc: 'Inspectable_Out: Js Array as Iterator',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = [1, 2, 3, 4];

            // Box
            var outVar = propertyValueTests.testBoxIterator_InspectableOut(myArray);
            VerifyIteratorAndContents("testBoxIterator_InspectableOut(myArray)", outVar, "myArray", myArray);

            // UnBox
            outVar = propertyValueTests.testIterator_InspectableOut(myArray);
            VerifyIteratorAndContents("testIterator_InspectableOut(myArray)", outVar, "myArray", myArray);
        }
    });

    runner.addTest({
        id: 97,
        desc: 'Inspectable_Out: Js Array as Vector',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = [1, 2, 3, 4];

            // Box
            var outVar = propertyValueTests.testBoxIVector_InspectableOut(myArray);
            VerifyArrayAndVectorContents("testBoxIVector_InspectableOut(myArray)", outVar, "myArray", myArray);

            // UnBox
            outVar = propertyValueTests.testIVector_InspectableOut(myArray);
            VerifyArrayAndVectorContents("testIVector_InspectableOut(myArray)", outVar, "myArray", myArray);
        }
    });

    runner.addTest({
        id: 98,
        desc: 'Inspectable_Out: Js Array as VectorView',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = [1, 2, 3, 4];

            // Box
            var outVar = propertyValueTests.testBoxIVectorView_InspectableOut(myArray);
            VerifyArrayAndVectorContents("testBoxIVectorView_InspectableOut(myArray)", outVar, "myArray", myArray);

            // UnBox
            outVar = propertyValueTests.testIVectorView_InspectableOut(myArray);
            VerifyArrayAndVectorContents("testIVectorView_InspectableOut(myArray)", outVar, "myArray", myArray);
        }
    });

    runner.addTest({
        id: 99,
        desc: 'Inspectable_Out: IVector<AsyncInfo>',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // Check we can marshal out as Inspectable
            var myVectorOfAsyncInfoInspectable = propertyValueTests.receiveVectorOfAsyncInfo_InspectableOut();
            var myVectorOfAsyncInfo = propertyValueTests.receiveVectorOfAsyncInfo();
            verify(myVectorOfAsyncInfoInspectable.length, myVectorOfAsyncInfo.length, "myVectorOfAsyncInfoInspectable.length");

            for (var i = 0; i < myVectorOfAsyncInfoInspectable.length; i++) {
                verify(myVectorOfAsyncInfoInspectable[i].toString(), myVectorOfAsyncInfo[i].toString(), "myVectorOfAsyncInfoInspectable[" + i + "].toString()");
            }

            // Box and get it back
            var outVar = propertyValueTests.testBoxInspectable_InspectableOut(myVectorOfAsyncInfo);
            verify(outVar, myVectorOfAsyncInfo, 'testBoxInspectable_InspectableOut(myVectorOfAsyncInfo)');

            // Get same vector back
            outVar = propertyValueTests.testInspectable_InspectableOut(myVectorOfAsyncInfo);
            verify(outVar, myVectorOfAsyncInfo, 'testInspectable_InspectableOut(myVectorOfAsyncInfo)');
        }
    });

    runner.addTest({
        id: 100,
        desc: 'Inspectable_Out: IVector<Guid>',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // Check we can marshal out as Inspectable
            var myVectorOfGuidInspectable = propertyValueTests.receiveVectorOfGuid_InspectableOut();
            var myVectorOfGuid = propertyValueTests.receiveVectorOfGuid();
            VerifyArrayAndVectorContents('myVectorOfGuidInspectable', myVectorOfGuidInspectable, 'myVectorOfGuid', myVectorOfGuid);

            // Box and get it back
            var outVar = propertyValueTests.testBoxInspectable_InspectableOut(myVectorOfGuid);
            verify(outVar, myVectorOfGuid, 'testBoxInspectable_InspectableOut(myVectorOfGuid)');

            // Get same vector back
            outVar = propertyValueTests.testInspectable_InspectableOut(myVectorOfGuid);
            verify(outVar, myVectorOfGuid, 'testInspectable_InspectableOut(myVectorOfGuid)');
        }
    });

    runner.addTest({
        id: 101,
        desc: 'Inspectable_Out: IVector<DateTime>',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // Check we can marshal out as Inspectable
            var myVectorOfDateInspectable = propertyValueTests.receiveVectorOfDate_InspectableOut();
            var myVectorOfDate = propertyValueTests.receiveVectorOfDate();
            VerifyArrayAndVectorContents('myVectorOfDateInspectable', myVectorOfDateInspectable, 'myVectorOfDate', myVectorOfDate);

            // Box and get it back
            var outVar = propertyValueTests.testBoxInspectable_InspectableOut(myVectorOfDate);
            verify(outVar, myVectorOfDate, 'testBoxInspectable_InspectableOut(myVectorOfDate)');

            // Get same vector back
            outVar = propertyValueTests.testInspectable_InspectableOut(myVectorOfDate);
            verify(outVar, myVectorOfDate, 'testInspectable_InspectableOut(myVectorOfDate)');
        }
    });

    runner.addTest({
        id: 102,
        desc: 'Inspectable_Out: IVector<Delegate>',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // Check we can marshal out as Inspectable
            var myVectorOfDelegateInspectable = propertyValueTests.receiveVectorOfDelegate_InspectableOut();
            var myVectorOfDelegate = propertyValueTests.receiveVectorOfDelegate();
            verify(myVectorOfDelegateInspectable.length, myVectorOfDelegate.length, "myVectorOfDelegateInspectable.length");

            for (var i = 0; i < myVectorOfDelegateInspectable.length; i++) {
                verify(myVectorOfDelegateInspectable[i].toString(), myVectorOfDelegate[i].toString(), "myVectorOfDelegateInspectable[" + i + "].toString()");
            }

            // Box and get it back
            var outVar = propertyValueTests.testBoxInspectable_InspectableOut(myVectorOfDelegate);
            verify(outVar, myVectorOfDelegate, 'testBoxInspectable_InspectableOut(myVectorOfDelegate)');

            // Get same vector back
            outVar = propertyValueTests.testInspectable_InspectableOut(myVectorOfDelegate);
            verify(outVar, myVectorOfDelegate, 'testInspectable_InspectableOut(myVectorOfDelegate)');
        }
    });

    runner.addTest({
        id: 103,
        desc: 'Inspectable_Out: IVector<Enum>',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // Check we can marshal out as Inspectable
            var myVectorOfEnumInspectable = propertyValueTests.receiveVectorOfEnum_InspectableOut();
            var myVectorOfEnum = propertyValueTests.receiveVectorOfEnum();
            VerifyArrayAndVectorContents('myVectorOfEnumInspectable', myVectorOfEnumInspectable, 'myVectorOfEnum', myVectorOfEnum);

            // Box and get it back
            var outVar = propertyValueTests.testBoxInspectable_InspectableOut(myVectorOfEnum);
            verify(outVar, myVectorOfEnum, 'testBoxInspectable_InspectableOut(myVectorOfEnum)');

            // Get same vector back
            outVar = propertyValueTests.testInspectable_InspectableOut(myVectorOfEnum);
            verify(outVar, myVectorOfEnum, 'testInspectable_InspectableOut(myVectorOfEnum)');
        }
    });

    runner.addTest({
        id: 104,
        desc: 'Inspectable_Out: IVector<EventRegistrationToken>',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // Check we can marshal out as Inspectable
            var myVectorOfEventRegistrationInspectable = propertyValueTests.receiveVectorOfEventRegistration_InspectableOut();
            var myVectorOfEventRegistration = propertyValueTests.receiveVectorOfEventRegistration();
            VerifyArrayAndVectorContents('myVectorOfEventRegistrationInspectable', myVectorOfEventRegistrationInspectable, 'myVectorOfEventRegistration', myVectorOfEventRegistration);

            // Box and get it back
            var outVar = propertyValueTests.testBoxInspectable_InspectableOut(myVectorOfEventRegistration);
            verify(outVar, myVectorOfEventRegistration, 'testBoxInspectable_InspectableOut(myVectorOfEventRegistration)');

            // Get same vector back
            outVar = propertyValueTests.testInspectable_InspectableOut(myVectorOfEventRegistration);
            verify(outVar, myVectorOfEventRegistration, 'testInspectable_InspectableOut(myVectorOfEventRegistration)');
        }
    });

    runner.addTest({
        id: 105,
        desc: 'Inspectable_Out: IVector<TimeSpan>',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // Check we can marshal out as Inspectable
            var myVectorOfTimeSpanInspectable = propertyValueTests.receiveVectorOfTimeSpan_InspectableOut();
            var myVectorOfTimeSpan = propertyValueTests.receiveVectorOfTimeSpan();
            VerifyArrayAndVectorContents('myVectorOfTimeSpanInspectable', myVectorOfTimeSpanInspectable, 'myVectorOfTimeSpan', myVectorOfTimeSpan);

            // Box and get it back
            var outVar = propertyValueTests.testBoxInspectable_InspectableOut(myVectorOfTimeSpan);
            verify(outVar, myVectorOfTimeSpan, 'testBoxInspectable_InspectableOut(myVectorOfTimeSpan)');

            // Get same vector back
            outVar = propertyValueTests.testInspectable_InspectableOut(myVectorOfTimeSpan);
            verify(outVar, myVectorOfTimeSpan, 'testInspectable_InspectableOut(myVectorOfTimeSpan)');
        }
    });

    runner.addTest({
        id: 106,
        desc: 'Inspectable_Out: Box null',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testBoxedNull_InspectableOut(null);
            verify(outVar, null, "testBoxedNull_InspectableOut(null)");
        }
    });

    runner.addTest({
        id: 107,
        desc: 'Inspectable_Out: Custom PropertyValue out with GRCN as IReference<Dimensions>',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testRCPV1_InspectableOut();
            logger.comment(outVar);
            verify(outVar.length, 100, 'testRCPV1_InspectableOut().length');
            verify(outVar.width, 20, 'testRCPV1_InspectableOut().width');
        }
    });

    runner.addTest({
        id: 108,
        desc: 'Inspectable_Out: Custom PropertyValue out with GRCN as RCName',
        pri: '0',
        preReq: function () {
            // can't customize GRCN in CLR
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            verify.exception(function () {
                var outVar = propertyValueTests.testRCPV2_InspectableOut();
            }, TypeError, "testRCPV2_InspectableOut");
        }
    });

    runner.addTest({
        id: 109,
        desc: 'Inspectable_Out: Custom PropertyValue out with GRCN as Windows.Foundation.IPropertyValue',
        pri: '0',
        preReq: function () {
            // can't customize GRCN in CLR
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            verify.exception(function () {
                var outVar = propertyValueTests.testRCPV3_InspectableOut();
            }, TypeError, "testRCPV3_InspectableOut");
        }
    });

    runner.addTest({
        id: 110,
        desc: 'Inspectable_Out: Custom PropertyValue out with GRCN as IReference<Char16>',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testRCPV4_InspectableOut();
            logger.comment(outVar);
            verify(outVar, 'D', 'testRCPV4_InspectableOut()');
        }
    });

    runner.addTest({
        id: 111,
        desc: 'Inspectable_Out: Custom PropertyValue out with GRCN as RCName',
        pri: '0',
        preReq: function () {
            // can't customize GRCN in CLR
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            verify.exception(function () {
                var outVar = propertyValueTests.testRCPV5_InspectableOut();
            }, TypeError, "testRCPV5_InspectableOut");
        }
    });

    runner.addTest({
        id: 112,
        desc: 'Inspectable_Out: Custom PropertyValue out with GRCN as Windows.Foundation.IPropertyValue',
        pri: '0',
        preReq: function () {
            // can't customize GRCN in CLR
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            verify.exception(function () {
                var outVar = propertyValueTests.testRCPV6_InspectableOut();
            }, TypeError, "testRCPV6_InspectableOut");
        }
    });

    runner.addTest({
        id: 113,
        desc: 'Inspectable_Out: Get inspectable out that has empty string as GRCN',
        pri: '0',
        preReq: function () {
            // can't customize GRCN in CLR
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            var expectedMembers = [['toString', 'function', 0]];
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.getRuntimeClassWithEmptyString();
            verify(outVar !== undefined, true, "outVar !== undefined");
            verify(outVar !== null, true, "outVar !== null");
            verifyMembers("outVar", outVar, expectedMembers);

            verify(outVar.toString(), "[object ]", "outVar.toString");

            var isSame = propertyValueTests.verifyRuntimeClassWithEmptyString(outVar);
            verify(isSame, true, "propertyValueTests.VerifyRuntimeClassWithEmptyString(outVar)");
        }
    });

    runner.addTest({
        id: 114,
        desc: 'Inspectable_Out: Get inspectable out that has fails on GRCN',
        pri: '0',
        preReq: function () {
            // can't customize GRCN in CLR
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            verify.exception(function () {
                var outVar = propertyValueTests.getRuntimeClassWithFailingGRCN();
            }, TypeError, "propertyValueTests.getRuntimeClassWithFailingGRCN();");
        }
    });

    runner.addTest({
        id: 115,
        desc: 'Inspectable_Out: Get inspectable out that return null for GRCN as a known interface',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.getRuntimeClassWithEmptyStringAsInterface();
            verify(outVar !== undefined, true, "outVar !== undefined");
            verify(outVar !== null, true, "outVar !== null");
            verifyMembers("outVar", outVar, [['getMyClassName', 'function', 0],['toString', 'function', 0]]);

            verify(outVar.toString(), "[object Animals.IEmptyGRCN]", "outVar.toString");
        }
    });

    runner.addTest({
        id: 116,
        desc: 'Inspectable_In: CanvasPixelArray from winrt',
        pri: '0',
        preReq: function () {
            return (typeof WScript !== 'undefined');
        },
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = WScript.CreateCanvasPixelArray([2, 3, 4, 5, 6, 7, 8, 9]);
            var outVar = propertyValueTests.testUInt8Array_InspectableIn(myArray);
            verify(outVar.isValidType, true, 'testUInt8Array_InspectableIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testUInt8Array_InspectableIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testUInt8Array_InspectableIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 117,
        desc: 'Inspectable_Out: IMap<String, Object>',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // Check we can marshal out as Inspectable
            var myMapOfStringAndInspectableInspectable = propertyValueTests.receiveMapOfStringAndInspectable_InspectableOut();
            verify(myMapOfStringAndInspectableInspectable.toString(), "[object Windows.Foundation.Collections.IMap`2<String,Object>]", "myMapOfStringAndInspectableInspectable.toString()");

            // Verify Contents
            var myAnimal = myMapOfStringAndInspectableInspectable["Animals.Animal"];
            verify(myAnimal.toString(), "[object Animals.Animal]", "myAnimal.toString()");
            verify(myAnimal.getGreeting(), "Animal1", "myAnimal.getGreeting()");

            var myFish = myMapOfStringAndInspectableInspectable["Animals.Fish"];
            verify(myFish.toString(), "[object Animals.Fish]", "myFish.toString()");
            verify(myFish.name, "Nemo", "myFish.name");

            var myVector = myMapOfStringAndInspectableInspectable["Windows.Foundation.Collections.IVector<Int>"];
            verify(myVector.length, 5, "myVector.length");
            for (var i = 0; i < myVector.length; i++) {
                verify(myVector[i], i + 1, "myVector[" + i + "]");
            }

            var myMap = myMapOfStringAndInspectableInspectable["Windows.Foundation.Collections.IMap<Animals.Dimensions,Windows.Foundation.Collections.IVector<String>>"];
            logger.comment(myMap);
            var myMapOfStructAndVector = propertyValueTests.receiveMapOfStructAndVector();
            verifyMapOfStructAndVector("myMap", myMap, myMapOfStructAndVector);

            // Box and get it back
            var outVar = propertyValueTests.testBoxInspectable_InspectableOut(myMapOfStringAndInspectableInspectable);
            verify(outVar, myMapOfStringAndInspectableInspectable, 'testBoxInspectable_InspectableOut(myMapOfStringAndInspectableInspectable)');

            // Get same vector back
            outVar = propertyValueTests.testInspectable_InspectableOut(myMapOfStringAndInspectableInspectable);
            verify(outVar, myMapOfStringAndInspectableInspectable, 'testInspectable_InspectableOut(myMapOfStringAndInspectableInspectable)');
        }
    });

    Loader42_FileName = 'Marshaling into and out of Inspectable';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
