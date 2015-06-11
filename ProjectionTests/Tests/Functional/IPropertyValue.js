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
        desc: 'IPropertyValue_In: null',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testNull_IPropertyValueIn(null);
            verify(outVar.isValidType, true, "testNull_IPropertyValueIn(null).isValidType");
        }
    });

    runner.addTest({
        id: 2,
        desc: 'IPropertyValue_In: undefined',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testNull_IPropertyValueIn(undefined);
            verify(outVar.isValidType, true, "testNull_IPropertyValueIn(undefined).isValidType");
        }
    });

    runner.addTest({
        id: 3,
        desc: 'IPropertyValue_In: Boolean',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testBoolean_IPropertyValueIn(true);
            verify(outVar.isValidType, true, "testBoolean_IPropertyValueIn(true).isValidType");
            verify(outVar.outValue, true, "testBoolean_IPropertyValueIn(true).outValue");

            outVar = propertyValueTests.testBoolean_IPropertyValueIn(false);
            verify(outVar.isValidType, true, "testBoolean_IPropertyValueIn(false).isValidType");
            verify(outVar.outValue, false, "testBoolean_IPropertyValueIn(false).outValue");

            outVar = propertyValueTests.testBoolean_IPropertyValueIn(new Boolean(0));
            verify(outVar.isValidType, true, "testBoolean_IPropertyValueIn(new Boolean(0)).isValidType");
            verify(outVar.outValue, false, "testBoolean_IPropertyValueIn(new Boolean(0)).outValue");

            outVar = propertyValueTests.testBoolean_IPropertyValueIn(new Boolean(1));
            verify(outVar.isValidType, true, "testBoolean_IPropertyValueIn(new Boolean(1)).isValidType");
            verify(outVar.outValue, true, "testBoolean_IPropertyValueIn(new Boolean(1)).outValue");

            outVar = propertyValueTests.testBoolean_IPropertyValueIn(new Boolean(""));
            verify(outVar.isValidType, true, 'testBoolean_IPropertyValueIn(new Boolean("")).isValidType');
            verify(outVar.outValue, false, 'testBoolean_IPropertyValueIn(new Boolean("")).outValue');

            outVar = propertyValueTests.testBoolean_IPropertyValueIn(new Boolean(null));
            verify(outVar.isValidType, true, "testBoolean_IPropertyValueIn(new Boolean(null)).isValidType");
            verify(outVar.outValue, false, "testBoolean_IPropertyValueIn(new Boolean(null)).outValue");

            outVar = propertyValueTests.testBoolean_IPropertyValueIn(new Boolean(NaN));
            verify(outVar.isValidType, true, "testBoolean_IPropertyValueIn(new Boolean(NaN)).isValidType");
            verify(outVar.outValue, false, "testBoolean_IPropertyValueIn(new Boolean(NaN)).outValue");

            outVar = propertyValueTests.testBoolean_IPropertyValueIn(new Boolean("false"));
            verify(outVar.isValidType, true, 'testBoolean_IPropertyValueIn(new Boolean("false")).isValidType');
            verify(outVar.outValue, true, 'testBoolean_IPropertyValueIn(new Boolean("false")).outValue');
        }
    });

    runner.addTest({
        id: 4,
        desc: 'IPropertyValue_In: String',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testString_IPropertyValueIn("String1");
            verify(outVar.isValidType, true, 'testString_IPropertyValueIn("String1").isValidType');
            verify(outVar.outValue, "String1", 'testString_IPropertyValueIn("String1").outValue');

            outVar = propertyValueTests.testString_IPropertyValueIn("");
            verify(outVar.isValidType, true, 'testString_IPropertyValueIn("").isValidType');
            verify(outVar.outValue, "", 'testString_IPropertyValueIn("").outValue');

            outVar = propertyValueTests.testString_IPropertyValueIn(new String("String2"));
            verify(outVar.isValidType, true, 'testString_IPropertyValueIn(new String("String2")).isValidType');
            verify(outVar.outValue, "String2", 'testString_IPropertyValueIn(new String("String2")).outValue');

            outVar = propertyValueTests.testString_IPropertyValueIn(new String(""));
            verify(outVar.isValidType, true, 'testString_IPropertyValueIn(new String("")).isValidType');
            verify(outVar.outValue, "", 'testString_IPropertyValueIn(new String("")).outValue');
        }
    });

    runner.addTest({
        id: 5,
        desc: 'IPropertyValue_In: Number',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testNumber_IPropertyValueIn(15);
            verify(outVar.isValidType, true, 'testNumber_IPropertyValueIn(15).isValidType');
            verify(outVar.outValue, 15, 'testNumber_IPropertyValueIn(15).outValue');

            outVar = propertyValueTests.testNumber_IPropertyValueIn(10.2);
            verify(outVar.isValidType, true, 'testNumber_IPropertyValueIn(10.2).isValidType');
            verify(outVar.outValue, 10.2, 'testNumber_IPropertyValueIn(10.2).outValue');

            outVar = propertyValueTests.testNumber_IPropertyValueIn(0);
            verify(outVar.isValidType, true, 'testNumber_IPropertyValueIn(0).isValidType');
            verify(outVar.outValue, 0, 'testNumber_IPropertyValueIn(0).outValue');

            outVar = propertyValueTests.testNumber_IPropertyValueIn(new Number(15));
            verify(outVar.isValidType, true, 'testNumber_IPropertyValueIn(new Number(15)).isValidType');
            verify(outVar.outValue, 15, 'testNumber_IPropertyValueIn(new Number(15)).outValue');

            outVar = propertyValueTests.testNumber_IPropertyValueIn(new Number(10.2));
            verify(outVar.isValidType, true, 'testNumber_IPropertyValueIn(new Number(10.2)).isValidType');
            verify(outVar.outValue, 10.2, 'testNumber_IPropertyValueIn(new Number(10.2)).outValue');

            outVar = propertyValueTests.testNumber_IPropertyValueIn(new Number(0));
            verify(outVar.isValidType, true, 'testNumber_IPropertyValueIn(new Number(0)).isValidType');
            verify(outVar.outValue, 0, 'testNumber_IPropertyValueIn(new Number(0)).outValue');
        }
    });

    runner.addTest({
        id: 6,
        desc: 'IPropertyValue_In: Date',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // Winrt Date
            var myElephant = new Animals.Elephant(1);
            var winrtDate = myElephant.getAge();
            var outVar = propertyValueTests.testDate_IPropertyValueIn(winrtDate);
            verify(outVar.isValidType, true, 'testDate_IPropertyValueIn(winrtDate).isValidType');
            verify(outVar.outValue, winrtDate, 'testDate_IPropertyValueIn(winrtDate).outValue');

            // Js Date:
            var jsDate = new Date(79, 5, 24, 11, 33, 0);
            outVar = propertyValueTests.testDate_IPropertyValueIn(jsDate);
            verify(outVar.isValidType, true, 'testDate_IPropertyValueIn(jsDate).isValidType');
            verify(outVar.outValue, jsDate, 'testDate_IPropertyValueIn(jsDate).outValue');
        }
    });

    runner.addTest({
        id: 7,
        desc: 'IPropertyValue_In: unboxed inspectable',
        pri: '0',
        preReq: function () {
            // IPropertyValue is not visible to CLR user, tests those strictly depend on IPropertyValue in the signature are not supported
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            var myAnimal = new Animals.Animal(1);
            verify.exception(function () {
                var outVar = propertyValueTests.testInspectable_IPropertyValueIn(myAnimal);
            }, TypeError, "testInspectable_IPropertyValueIn(myAnimal)");

            var myVector = myAnimal.getVector();
            verify.exception(function () {
                var outVar = propertyValueTests.testInspectable_IPropertyValueIn(myVector);
            }, TypeError, "testInspectable_IPropertyValueIn(myAnimal)");
        }
    });

    runner.addTest({
        id: 8,
        desc: 'IPropertyValue_In: Js Arrays',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // Js Array
            var myArray = [true, new Date(20, 20, 20), "This is Date Entry"];
            var outVar = propertyValueTests.testArray_IPropertyValueIn(myArray);
            verify(outVar.isValidType, true, 'testArray_IPropertyValueIn([true, new Date(20, 20, 20), "This is Date Entry"]).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testArray_IPropertyValueIn([true, new Date(20, 20, 20), "This is Date Entry"]).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testArray_IPropertyValueIn([true, new Date(20, 20, 20), "This is Date Entry"]).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 9,
        desc: 'IPropertyValue_In: ES5 Arrays',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myES5Array = [false, 30.5];
            Object.defineProperty(myES5Array, "2", { set: function (x) { value = x; }, get: function () { return value }, configurable: true });
            myES5Array[2] = "This is number Entry"
            var outVar = propertyValueTests.testArray_IPropertyValueIn(myES5Array);
            verify(outVar.isValidType, true, 'testArray_IPropertyValueIn(myES5Array).isValidType');
            verify(outVar.outValue.length, myES5Array.length, 'testArray_IPropertyValueIn(myES5Array).outValue.length');
            for (var i = 0; i < myES5Array.length; i++) {
                verify(outVar.outValue[i], myES5Array[i], 'testArray_IPropertyValueIn(myES5Array).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 10,
        desc: 'IPropertyValue_In: GUID Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveGuidArray();
            var outVar = propertyValueTests.testGuidArray_IPropertyValueIn(myArray);
            verify(outVar.isValidType, true, 'testGuidArray_IPropertyValueIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testGuidArray_IPropertyValueIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testGuidArray_IPropertyValueIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 11,
        desc: 'IPropertyValue_In: Class (Animal) Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveAnimalArray();
            var outVar = propertyValueTests.testArray_IPropertyValueIn(myArray);
            verify(outVar.isValidType, true, 'testArray_IPropertyValueIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testArray_IPropertyValueIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testArray_IPropertyValueIn(myArray).outValue[' + i + ']');
                if (myArray[i] !== null) {
                    verify(outVar.outValue[i].getGreeting(), myArray[i].getGreeting(), 'testArray_IPropertyValueIn(myArray).outValue[' + i + '].getGreeting()');
                }
            }
        }
    });

    runner.addTest({
        id: 12,
        desc: 'IPropertyValue_In: IFish Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveFishArray();
            var outVar = propertyValueTests.testArray_IPropertyValueIn(myArray);
            verify(outVar.isValidType, true, 'testArray_IPropertyValueIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testArray_IPropertyValueIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testArray_IPropertyValueIn(myArray).outValue[' + i + ']');
                if (myArray[i] !== null) {
                    verify(outVar.outValue[i].name, myArray[i].name, 'testArray_IPropertyValueIn(myArray).outValue[' + i + '].name');
                }
            }
        }
    });

    runner.addTest({
        id: 13,
        desc: 'IPropertyValue_In: IVector<int> Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveVectorArray();
            var outVar = propertyValueTests.testArray_IPropertyValueIn(myArray);
            verify(outVar.isValidType, true, 'testArray_IPropertyValueIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testArray_IPropertyValueIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testArray_IPropertyValueIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 14,
        desc: 'IPropertyValue_In: Date Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveDateArray();
            var outVar = propertyValueTests.testDateArray_IPropertyValueIn(myArray);
            verify(outVar.isValidType, true, 'testDateArray_IPropertyValueIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testDateArray_IPropertyValueIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testDateArray_IPropertyValueIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 15,
        desc: 'IPropertyValue_In: boolean Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveBooleanArray();
            var outVar = propertyValueTests.testBooleanArray_IPropertyValueIn(myArray);
            verify(outVar.isValidType, true, 'testBooleanArray_IPropertyValueIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testBooleanArray_IPropertyValueIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testBooleanArray_IPropertyValueIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 16,
        desc: 'IPropertyValue_In: string Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveStringArray();
            var outVar = propertyValueTests.testStringArray_IPropertyValueIn(myArray);
            verify(outVar.isValidType, true, 'testStringArray_IPropertyValueIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testStringArray_IPropertyValueIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testStringArray_IPropertyValueIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 17,
        desc: 'IPropertyValue_In: inspectable Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveInspectableArray();
            var outVar = propertyValueTests.testArray_IPropertyValueIn(myArray);
            verify(outVar.isValidType, true, 'testArray_IPropertyValueIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testArray_IPropertyValueIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testArray_IPropertyValueIn(myArray).outValue[' + i + ']');
                if (i === 0) {
                    verify(outVar.outValue[0].getGreeting(), myArray[0].getGreeting(), 'testArray_IPropertyValueIn(myArray).outValue[0].getGreeting()');
                }
                else if (i === 2) {
                    verify(outVar.outValue[2].name, myArray[2].name, 'testArray_IPropertyValueIn(myArray).outValue[2].name');
                }
            }
        }
    });

    runner.addTest({
        id: 18,
        desc: 'IPropertyValue_In: char16 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveChar16Array();
            var outVar = propertyValueTests.testChar16Array_IPropertyValueIn(myArray);
            verify(outVar.isValidType, true, 'testChar16Array_IPropertyValueIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testChar16Array_IPropertyValueIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testChar16Array(myArray)_IPropertyValueIn.outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 19,
        desc: 'IPropertyValue_In: Uint8 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveUInt8Array();
            var outVar = propertyValueTests.testUInt8Array_IPropertyValueIn(myArray);
            verify(outVar.isValidType, true, 'testUInt8Array_IPropertyValueIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testUInt8Array_IPropertyValueIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testUInt8Array_IPropertyValueIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 20,
        desc: 'IPropertyValue_In: int16 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveInt16Array();
            var outVar = propertyValueTests.testInt16Array_IPropertyValueIn(myArray);
            verify(outVar.isValidType, true, 'testInt16Array_IPropertyValueIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testInt16Array_IPropertyValueIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testInt16Array_IPropertyValueIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 21,
        desc: 'IPropertyValue_In: uint16 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveUInt16Array();
            var outVar = propertyValueTests.testUInt16Array_IPropertyValueIn(myArray);
            verify(outVar.isValidType, true, 'testUInt16Array_IPropertyValueIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testUInt16Array_IPropertyValueIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testUInt16Array_IPropertyValueIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 22,
        desc: 'IPropertyValue_In: int32 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveInt32Array();
            var outVar = propertyValueTests.testInt32Array_IPropertyValueIn(myArray);
            verify(outVar.isValidType, true, 'testInt32Array_IPropertyValueIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testInt32Array_IPropertyValueIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testInt32Array_IPropertyValueIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 23,
        desc: 'IPropertyValue_In: uint32 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveUInt32Array();
            var outVar = propertyValueTests.testUInt32Array_IPropertyValueIn(myArray);
            verify(outVar.isValidType, true, 'testUInt32Array_IPropertyValueIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testUInt32Array_IPropertyValueIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testUInt32Array_IPropertyValueIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 24,
        desc: 'IPropertyValue_In: int64 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveInt64Array();
            var outVar = propertyValueTests.testInt64Array_IPropertyValueIn(myArray);
            verify(outVar.isValidType, true, 'testInt64Array_IPropertyValueIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testInt64Array_IPropertyValueIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testInt64Array_IPropertyValueIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 25,
        desc: 'IPropertyValue_In: uint64 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveUInt64Array();
            var outVar = propertyValueTests.testUInt64Array_IPropertyValueIn(myArray);
            verify(outVar.isValidType, true, 'testUInt64Array_IPropertyValueIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testUInt64Array_IPropertyValueIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testUInt64Array_IPropertyValueIn(myArray).outValue[' + i + ']');
            }
        }
    });


    runner.addTest({
        id: 26,
        desc: 'IPropertyValue_In: float Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveFloatArray();
            var outVar = propertyValueTests.testFloatArray_IPropertyValueIn(myArray);
            verify(outVar.isValidType, true, 'testFloatArray_IPropertyValueIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testFloatArray_IPropertyValueIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testFloatArray_IPropertyValueIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 27,
        desc: 'IPropertyValue_In: double Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveDoubleArray();
            var outVar = propertyValueTests.testDoubleArray_IPropertyValueIn(myArray);
            verify(outVar.isValidType, true, 'testDoubleArray_IPropertyValueIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testDoubleArray_IPropertyValueIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testDoubleArray_IPropertyValueIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 28,
        desc: 'IPropertyValue_In: Struct from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myStruct = {
                length: 10,
                width: 40
            };
            verify.exception(function () {
                var outVar = propertyValueTests.testInspectable_IPropertyValueIn(myStruct);
            }, TypeError, "testInspectable_IPropertyValueIn(myStruct)");
        }
    });

    runner.addTest({
        id: 29,
        desc: 'IPropertyValue_In: Enum from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testNumber_IPropertyValueIn(Animals.Phylum.cnidaria);
            verify(outVar.isValidType, true, 'testNumber_IPropertyValueIn(Animals.Phylum.cnidaria).isValidType');
            verify(outVar.outValue, Animals.Phylum.cnidaria, 'testNumber_IPropertyValueIn(Animals.Phylum.cnidaria).outValue');
        }
    });

    runner.addTest({
        id: 30,
        desc: 'IPropertyValue_In: Delegate from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myWinRTDelegate = (new Animals.Animal(1)).getNativeDelegateAsOutParam();
            verify.exception(function () {
                var outVar = propertyValueTests.testInspectable_IPropertyValueIn(myWinRTDelegate);
            }, TypeError, "testInspectable_IPropertyValueIn(myWinRTDelegate)");
        }
    });

    runner.addTest({
        id: 31,
        desc: 'IPropertyValue_In: Js Delegate from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            function myJSDelegate() {
            }
            verify.exception(function () {
                var outVar = propertyValueTests.testInspectable_IPropertyValueIn(myJSDelegate);
            }, TypeError, "testInspectable_IPropertyValueIn(myJSDelegate)");
        }
    });

    runner.addTest({
        id: 32,
        desc: 'IPropertyValue_In: Struct Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveStructArray();
            var outVar = propertyValueTests.testDimensionsArray_IPropertyValueIn(myArray);
            verify(outVar.isValidType, true, 'testDimensionsArray_IPropertyValueIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testDimensionsArray_IPropertyValueIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i].length, myArray[i].length, 'testDimensionsArray_IPropertyValueIn(myArray).outValue[' + i + '].length');
                verify(outVar.outValue[i].width, myArray[i].width, 'testDimensionsArray_IPropertyValueIn(myArray).outValue[' + i + '].width');
            }
        }
    });

    runner.addTest({
        id: 33,
        desc: 'IPropertyValue_In: Enum Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveEnumArray();
            var outVar = propertyValueTests.testEnumArray_IPropertyValueIn(myArray);
            verify(outVar.isValidType, true, 'testEnumArray_IPropertyValueIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testEnumArray_IPropertyValueIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testEnumArray_IPropertyValueIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 34,
        desc: 'IPropertyValue_In: TimeSpan Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveTimeSpanArray();
            var outVar = propertyValueTests.testTimeSpanArray_IPropertyValueIn(myArray);
            verify(outVar.isValidType, true, 'testTimeSpanArray_IPropertyValueIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testTimeSpanArray_IPropertyValueIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testTimeSpanArray_IPropertyValueIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 35,
        desc: 'IPropertyValue_In: Point Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receivePointArray();
            var outVar = propertyValueTests.testPointArray_IPropertyValueIn(myArray);
            verify(outVar.isValidType, true, 'testPointArray_IPropertyValueIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testPointArray_IPropertyValueIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i].x, myArray[i].x, 'testPointArray_IPropertyValueIn(myArray).outValue[' + i + '].x');
                verify(outVar.outValue[i].y, myArray[i].y, 'testPointArray_IPropertyValueIn(myArray).outValue[' + i + '].y');
            }
        }
    });

    runner.addTest({
        id: 36,
        desc: 'IPropertyValue_In: Size Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveSizeArray();
            var outVar = propertyValueTests.testSizeArray_IPropertyValueIn(myArray);
            verify(outVar.isValidType, true, 'testSizeArray_IPropertyValueIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testSizeArray_IPropertyValueIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i].height, myArray[i].height, 'testSizeArray_IPropertyValueIn(myArray).outValue[' + i + '].height');
                verify(outVar.outValue[i].width, myArray[i].width, 'testSizeArray_IPropertyValueIn(myArray).outValue[' + i + '].width');
            }
        }
    });

    runner.addTest({
        id: 37,
        desc: 'IPropertyValue_In: Rect Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveRectArray();
            var outVar = propertyValueTests.testRectArray_IPropertyValueIn(myArray);
            verify(outVar.isValidType, true, 'testRectArray_IPropertyValueIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testRectArray_IPropertyValueIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i].x, myArray[i].x, 'testRectArray_IPropertyValueIn(myArray).outValue[' + i + '].x');
                verify(outVar.outValue[i].y, myArray[i].y, 'testRectArray_IPropertyValueIn(myArray).outValue[' + i + '].y');
                verify(outVar.outValue[i].height, myArray[i].height, 'testRectArray_IPropertyValueIn(myArray).outValue[' + i + '].height');
                verify(outVar.outValue[i].width, myArray[i].width, 'testRectArray_IPropertyValueIn(myArray).outValue[' + i + '].width');
            }
        }
    });

    runner.addTest({
        id: 38,
        desc: 'IPropertyValue_In: Winrt Delegate Array from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveWinrtDelegateArray();
            var outVar = propertyValueTests.testDelegateArray_IPropertyValueIn(myArray, myArray);
            verify(outVar.isValidType, true, 'testDelegateArray_IPropertyValueIn(myArray, myArray).isValidType');
            for (var i = 0; i < myArray.length; i++) {
                verify(propertyValueTests.isSameDelegate(outVar.outValue[i], myArray[i]), true, 'testDelegateArray_IPropertyValueIn(myArray).outValue[' + i + '] are same');
            }
        }
    });

    runner.addTest({
        id: 39,
        desc: 'IPropertyValue_In: Js Delegate Array from winrt',
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
            var outVar = propertyValueTests.testDelegateArray_IPropertyValueIn(myArray);
            verify(outVar.outValue.length, myArray.length, 'testDelegateArray_IPropertyValueIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testDelegateArray_IPropertyValueIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 40,
        desc: 'IPropertyValue_Out: null',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testNull_IPropertyValueOut();
            verify(outVar, null, "testNull_IPropertyValueOut()");
        }
    });

    runner.addTest({
        id: 41,
        desc: 'IPropertyValue_Out: Boolean',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testBoolean_IPropertyValueOut(true);
            verify(outVar, true, "testBoolean_IPropertyValueOut(true)");

            outVar = propertyValueTests.testBoolean_IPropertyValueOut(false);
            verify(outVar, false, "testBoolean_IPropertyValueOut(false)");
        }
    });

    runner.addTest({
        id: 42,
        desc: 'IPropertyValue_Out: String',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testString_IPropertyValueOut("String1");
            verify(outVar, "String1", 'testString_IPropertyValueOut("String1")');

            outVar = propertyValueTests.testString_IPropertyValueOut("");
            verify(outVar, "", 'testString_IPropertyValueOut("")');
        }
    });

    runner.addTest({
        id: 43,
        desc: 'IPropertyValue_Out: Char16',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testChar16_IPropertyValueOut("S");
            verify(outVar, "S", 'testChar16_IPropertyValueOut("S")');
        }
    });

    runner.addTest({
        id: 44,
        desc: 'IPropertyValue_Out: UInt8',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testUInt8_IPropertyValueOut(2);
            verify(outVar, 2, 'testUInt8_IPropertyValueOut(2)');
        }
    });

    runner.addTest({
        id: 45,
        desc: 'IPropertyValue_Out: Int16',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testInt16_IPropertyValueOut(2);
            verify(outVar, 2, 'testInt16_IPropertyValueOut(2)');
        }
    });

    runner.addTest({
        id: 46,
        desc: 'IPropertyValue_Out: UInt16',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testUInt16_IPropertyValueOut(2);
            verify(outVar, 2, 'testUInt16_IPropertyValueOut(2)');
        }
    });

    runner.addTest({
        id: 47,
        desc: 'IPropertyValue_Out: Int32',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testInt32_IPropertyValueOut(2);
            verify(outVar, 2, 'testInt32_IPropertyValueOut(2)');
        }
    });

    runner.addTest({
        id: 48,
        desc: 'IPropertyValue_Out: UInt32',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testUInt32_IPropertyValueOut(2);
            verify(outVar, 2, 'testUInt32_IPropertyValueOut(2)');
        }
    });

    runner.addTest({
        id: 49,
        desc: 'IPropertyValue_Out: Int64',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testInt64_IPropertyValueOut(2);
            verify(outVar, 2, 'testInt64_IPropertyValueOut(2)');
        }
    });

    runner.addTest({
        id: 50,
        desc: 'IPropertyValue_Out: UInt64',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testUInt64_IPropertyValueOut(2);
            verify(outVar, 2, 'testUInt64_IPropertyValueOut(2)');
        }
    });

    runner.addTest({
        id: 51,
        desc: 'IPropertyValue_Out: Float',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testFloat_IPropertyValueOut(2);
            verify(outVar, 2, 'testFloat_IPropertyValueOut(2)');
        }
    });

    runner.addTest({
        id: 52,
        desc: 'IPropertyValue_Out: Double',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testDouble_IPropertyValueOut(2);
            verify(outVar, 2, 'testDouble_IPropertyValueOut(2)');
        }
    });

    runner.addTest({
        id: 53,
        desc: 'IPropertyValue_Out: Guid',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testGuid_IPropertyValueOut("{10C27311-B58F-476C-BFF4-6689B8A19836}");
            verify(outVar, "10c27311-b58f-476c-bff4-6689b8a19836", 'testGuid_IPropertyValueOut("{10C27311-B58F-476C-BFF4-6689B8A19836}")');
        }
    });

    runner.addTest({
        id: 54,
        desc: 'IPropertyValue_Out: Date',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // Winrt Date
            var myElephant = new Animals.Elephant(1);
            var winrtDate = myElephant.getAge();
            var outVar = propertyValueTests.testDate_IPropertyValueOut(winrtDate);
            verify(outVar, winrtDate, 'testDate_IPropertyValueOut(winrtDate)');

            // Js Date:
            var jsDate = new Date(79, 5, 24, 11, 33, 0);
            outVar = propertyValueTests.testDate_IPropertyValueOut(jsDate);
            verify(outVar, jsDate, 'testDate_IPropertyValueOut(jsDate)');
        }
    });

    runner.addTest({
        id: 55,
        desc: 'IPropertyValue_Out: GUID Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveGuidArray();
            var outVar = propertyValueTests.testGuidArray_IPropertyValueOut(myArray);
            verify(outVar.length, myArray.length, 'testGuidArray_IPropertyValueOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testGuidArray_IPropertyValueOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 56,
        desc: 'IPropertyValue_Out: Date Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveDateArray();
            var outVar = propertyValueTests.testDateArray_IPropertyValueOut(myArray);
            verify(outVar.length, myArray.length, 'testDateArray_IPropertyValueOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testDateArray_IPropertyValueOut(myArray)[' + i + ']');
            }
        }
    });


    runner.addTest({
        id: 57,
        desc: 'IPropertyValue_Out: Boolean Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveBooleanArray();
            var outVar = propertyValueTests.testBooleanArray_IPropertyValueOut(myArray);
            verify(outVar.length, myArray.length, 'testBooleanArray_IPropertyValueOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testBooleanArray_IPropertyValueOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 58,
        desc: 'IPropertyValue_Out: String Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveStringArray();
            var outVar = propertyValueTests.testStringArray_IPropertyValueOut(myArray);
            verify(outVar.length, myArray.length, 'testStringArray_IPropertyValueOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testStringArray_IPropertyValueOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 59,
        desc: 'IPropertyValue_Out: Char16 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveChar16Array();
            var outVar = propertyValueTests.testChar16Array_IPropertyValueOut(myArray);
            verify(outVar.length, myArray.length, 'testChar16Array_IPropertyValueOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testChar16Array_IPropertyValueOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 60,
        desc: 'IPropertyValue_Out: UInt8 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveUInt8Array();
            var outVar = propertyValueTests.testUInt8Array_IPropertyValueOut(myArray);
            verify(outVar.length, myArray.length, 'testUInt8Array_IPropertyValueOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testUInt8Array_IPropertyValueOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 61,
        desc: 'IPropertyValue_Out: Int16 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveInt16Array();
            var outVar = propertyValueTests.testInt16Array_IPropertyValueOut(myArray);
            verify(outVar.length, myArray.length, 'testInt16Array_IPropertyValueOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testInt16Array_IPropertyValueOut(myArray)[' + i + ']');
            }
        }
    });


    runner.addTest({
        id: 62,
        desc: 'IPropertyValue_Out: UInt16 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveUInt16Array();
            var outVar = propertyValueTests.testUInt16Array_IPropertyValueOut(myArray);
            verify(outVar.length, myArray.length, 'testUInt16Array_IPropertyValueOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testUInt16Array_IPropertyValueOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 63,
        desc: 'IPropertyValue_Out: Int32 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveInt32Array();
            var outVar = propertyValueTests.testInt32Array_IPropertyValueOut(myArray);
            verify(outVar.length, myArray.length, 'testInt32Array_IPropertyValueOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testInt32Array_IPropertyValueOut(myArray)[' + i + ']');
            }
        }
    });


    runner.addTest({
        id: 64,
        desc: 'IPropertyValue_Out: UInt32 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveUInt32Array();
            var outVar = propertyValueTests.testUInt32Array_IPropertyValueOut(myArray);
            verify(outVar.length, myArray.length, 'testUInt32Array_IPropertyValueOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testUInt32Array_IPropertyValueOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 65,
        desc: 'IPropertyValue_Out: Int64 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveInt64Array();
            var outVar = propertyValueTests.testInt64Array_IPropertyValueOut(myArray);
            verify(outVar.length, myArray.length, 'testInt64Array_IPropertyValueOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testInt64Array_IPropertyValueOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 66,
        desc: 'IPropertyValue_Out: UInt64 Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveUInt64Array();
            var outVar = propertyValueTests.testUInt64Array_IPropertyValueOut(myArray);
            verify(outVar.length, myArray.length, 'testUInt64Array_IPropertyValueOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testUInt64Array_IPropertyValueOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 67,
        desc: 'IPropertyValue_Out: Float Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveFloatArray();
            var outVar = propertyValueTests.testFloatArray_IPropertyValueOut(myArray);
            verify(outVar.length, myArray.length, 'testFloatArray_IPropertyValueOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testFloatArray_IPropertyValueOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 68,
        desc: 'IPropertyValue_Out: Double Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveDoubleArray();
            var outVar = propertyValueTests.testDoubleArray_IPropertyValueOut(myArray);
            verify(outVar.length, myArray.length, 'testDoubleArray_IPropertyValueOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testDoubleArray_IPropertyValueOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 69,
        desc: 'IPropertyValue_Out: Inspectable Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveInspectableArray();
            var outVar = propertyValueTests.testArray_IPropertyValueOut(myArray);
            verify(outVar.length, myArray.length, 'testArray_IPropertyValueOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testArray_IPropertyValueOut(myArray)[' + i + ']');
                if (i === 0) {
                    verify(outVar[0].getGreeting(), myArray[0].getGreeting(), 'testArray_IPropertyValueOut(myArray)[0].getGreeting()');
                }
                else if (i === 2) {
                    verify(outVar[2].name, myArray[2].name, 'testArray_IPropertyValueOut(myArray)[2].name');
                }
            }
        }
    });

    runner.addTest({
        id: 70,
        desc: 'IPropertyValue_Out: Class (Animal) Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveAnimalArray();
            var outVar = propertyValueTests.testAnimalArray_IPropertyValueOut(myArray);
            verify(outVar.length, myArray.length, 'testAnimalArray_IPropertyValueOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testAnimalArray_IPropertyValueOut(myArray)[' + i + ']');
                if (myArray[i] !== null) {
                    verify(outVar[i].getGreeting(), myArray[i].getGreeting(), 'testAnimalArray_IPropertyValueOut(myArray)[' + i + '].getGreeting()');
                }
            }
        }
    });

    runner.addTest({
        id: 71,
        desc: 'IPropertyValue_Out: IFish Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveFishArray();
            var outVar = propertyValueTests.testFishArray_IPropertyValueOut(myArray);
            verify(outVar.length, myArray.length, 'testFishArray_IPropertyValueOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testFishArray_IPropertyValueOut(myArray)[' + i + ']');
                if (myArray[i] !== null) {
                    verify(outVar[i].name, myArray[i].name, 'testFishArray_IPropertyValueOut(myArray)[' + i + '].name');
                }
            }
        }
    });

    runner.addTest({
        id: 72,
        desc: 'IPropertyValue_Out: IVector<int> Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveVectorArray();
            var outVar = propertyValueTests.testVectorArray_IPropertyValueOut(myArray);
            verify(outVar.length, myArray.length, 'testVectorArray_IPropertyValueOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testVectorArray_IPropertyValueOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 73,
        desc: 'IPropertyValue_Out: Js Arrays that are marshaled into inspectable arrays',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = [true, new Date(20, 20, 20), "This is Date Entry"];
            var outVar = propertyValueTests.testArray_IPropertyValueOut(myArray);
            verify(outVar.length, myArray.length, 'testArray_IPropertyValueOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testArray_IPropertyValueOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 74,
        desc: 'IPropertyValue_Out: ES5 Arrays that are marshaled into inspectable arrays',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = [false, 30.5];
            Object.defineProperty(myArray, "2", { set: function (x) { value = x; }, get: function () { return value }, configurable: true }); var outVar = propertyValueTests.testArray_IPropertyValueOut(myArray);
            myArray[2] = "This is number Entry"
            verify(outVar.length, myArray.length, 'testArray_IPropertyValueOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testArray_IPropertyValueOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 75,
        desc: 'IPropertyValue_Out: Box inspectable in winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            var myAnimal = new Animals.Animal(1);
            var outVar = propertyValueTests.testBoxInspectable_IPropertyValueOut(myAnimal);
            verify(outVar, myAnimal, 'testBoxInspectable_IPropertyValueOut(myAnimal)');

            var myVector = myAnimal.getVector();
            outVar = propertyValueTests.testBoxInspectable_IPropertyValueOut(myVector);
            verify(outVar, myVector, 'testBoxInspectable_IPropertyValueOut(myVector)');
        }
    });

    runner.addTest({
        id: 76,
        desc: 'IPropertyValue_Out: IVector<IVector<int>>',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myVectorOfVector = propertyValueTests.receiveVectorOfVector();

            var outVar = propertyValueTests.testBoxInspectable_IPropertyValueOut(myVectorOfVector);
            verify(outVar, myVectorOfVector, 'testBoxInspectable_IPropertyValueOut(myVectorOfVector)');
            for (var i = 0; i < myVectorOfVector.length; i++) {
                verify(outVar[i], myVectorOfVector[i], 'testBoxInspectable_IPropertyValueOut(myVectorOfVector)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 77,
        desc: 'IPropertyValue_Out: IVector<Dimensions>',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // Check we can marshal out as Inspectable
            var myVectorOfStruct = propertyValueTests.receiveVectorOfStruct();

            // Box and get it back
            var outVar = propertyValueTests.testBoxInspectable_IPropertyValueOut(myVectorOfStruct);
            verify(outVar, myVectorOfStruct, 'testBoxInspectable_IPropertyValueOut(myVectorOfStruct)');
            for (var i = 0; i < myVectorOfStruct.length; i++) {
                verify(outVar[i], myVectorOfStruct[i], 'testBoxInspectable_IPropertyValueOut(myVectorOfStruct)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 78,
        desc: 'IPropertyValue_Out: IMap<Dimension, IVector<HSTRING>>',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // Check we can marshal out as Inspectable
            var myMapOfStructAndVector = propertyValueTests.receiveMapOfStructAndVector();

            // Box and get it back
            var outVar = propertyValueTests.testBoxInspectable_IPropertyValueOut(myMapOfStructAndVector);
            verify(outVar, myMapOfStructAndVector, 'testBoxInspectable_IPropertyValueOut(myMapOfStructAndVector)');
            verifyMapOfStructAndVector("testBoxInspectable_IPropertyValueOut(myMapOfStructAndVector)", outVar, myMapOfStructAndVector);
        }
    });

    runner.addTest({
        id: 79,
        desc: 'IPropertyValue_Out: IVector<RCObservableVector>',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // Check we can marshal out as Inspectable
            var myVectorOfRCObservableVector = propertyValueTests.receiveVectorOfRCObservableVector();

            // Box and get it back
            var outVar = propertyValueTests.testBoxInspectable_IPropertyValueOut(myVectorOfRCObservableVector);
            verify(outVar, myVectorOfRCObservableVector, 'testBoxInspectable_IPropertyValueOut(myVectorOfRCObservableVector)');
            for (var i = 0; i < myVectorOfRCObservableVector.length; i++) {
                verify(outVar[i], myVectorOfRCObservableVector[i], 'testBoxInspectable_IPropertyValueOut(myVectorOfRCObservableVector)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 80,
        desc: 'IPropertyValue_Out: Struct from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myStruct = {
                length: 10,
                width: 40
            };
            var outVar = propertyValueTests.testDimensions_IPropertyValueOut(myStruct);
            verify(outVar.length, myStruct.length, 'testDimensions_IPropertyValueOut(myStruct).length');
            verify(outVar.width, myStruct.width, 'testDimensions_IPropertyValueOut(myStruct).width');
        }
    });

    runner.addTest({
        id: 81,
        desc: 'IPropertyValue_Out: TimeSpan',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // TimeSpan from WinRT
            var timeSpanTests = new DevTests.DateTimeAndTimeSpan.Tests();
            var winrtTimeSpan = timeSpanTests.produceTimeSpan(10000 * 1000 * 60 * 60);
            var outVar = propertyValueTests.testTimeSpan_IPropertyValueOut(winrtTimeSpan);
            verify(outVar, winrtTimeSpan, 'testTimeSpan_IPropertyValueOut(winrtTimeSpan)');

            // Js Number:
            var jsTimeSpan = 1000 * 60 * 60 * 24;
            outVar = propertyValueTests.testTimeSpan_IPropertyValueOut(jsTimeSpan);
            verify(outVar, jsTimeSpan, 'testTimeSpan_IPropertyValueOut(jsTimeSpan)');
        }
    });

    runner.addTest({
        id: 82,
        desc: 'IPropertyValue_Out: Point',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myPoint = {
                x: 10,
                y: 40
            };
            var outVar = propertyValueTests.testPoint_IPropertyValueOut(myPoint);
            verify(outVar.x, myPoint.x, 'testPoint_IPropertyValueOut(myPoint).x');
            verify(outVar.y, myPoint.y, 'testPoint_IPropertyValueOut(myPoint).y');

        }
    });

    runner.addTest({
        id: 83,
        desc: 'IPropertyValue_Out: Size',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var mySize = {
                height: 10,
                width: 40
            };
            var outVar = propertyValueTests.testSize_IPropertyValueOut(mySize);
            verify(outVar.height, mySize.height, 'testSize_IPropertyValueOut(mySize).height');
            verify(outVar.width, mySize.width, 'testSize_IPropertyValueOut(mySize).width');
        }
    });

    runner.addTest({
        id: 84,
        desc: 'IPropertyValue_Out: Rect',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myRect = {
                height: 10,
                width: 40,
                x: 1000,
                y: 3000
            };
            var outVar = propertyValueTests.testRect_IPropertyValueOut(myRect);
            verify(outVar.x, myRect.x, 'testRect_IPropertyValueOut(myRect).x');
            verify(outVar.y, myRect.y, 'testRect_IPropertyValueOut(myRect).y');
            verify(outVar.height, myRect.height, 'testRect_IPropertyValueOut(myRect).height');
            verify(outVar.width, myRect.width, 'testRect_IPropertyValueOut(myRect).width');
        }
    });

    runner.addTest({
        id: 85,
        desc: 'IPropertyValue_Out: Enum',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testEnum_IPropertyValueOut(Animals.Phylum.phoronida);
            verify(outVar, Animals.Phylum.phoronida, 'testEnum_IPropertyValueOut(Animals.Phylum.phoronida)');
        }
    });

    runner.addTest({
        id: 86,
        desc: 'IPropertyValue_Out: Struct Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveStructArray();
            var outVar = propertyValueTests.testInspectable_IPropertyValueOut(myArray);
            verify(outVar.length, myArray.length, 'testInspectable_IPropertyValueOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i].length, myArray[i].length, 'testInspectable_IPropertyValueOut(myArray)[' + i + '].length');
                verify(outVar[i].width, myArray[i].width, 'testInspectable_IPropertyValueOut(myArray)[' + i + '].width');
            }
        }
    });

    runner.addTest({
        id: 87,
        desc: 'IPropertyValue_Out: TimeSpan Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveTimeSpanArray();
            var outVar = propertyValueTests.testTimeSpanArray_IPropertyValueOut(myArray);
            verify(outVar.length, myArray.length, 'testTimeSpanArray_IPropertyValueOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testTimeSpanArray_IPropertyValueOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 88,
        desc: 'IPropertyValue_Out: Point Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receivePointArray();
            var outVar = propertyValueTests.testPointArray_IPropertyValueOut(myArray);
            verify(outVar.length, myArray.length, 'testPointArray_IPropertyValueOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i].x, myArray[i].x, 'testPointArray_IPropertyValueOut(myArray)[' + i + '].x');
                verify(outVar[i].y, myArray[i].y, 'testPointArray_IPropertyValueOut(myArray)[' + i + '].y');
            }
        }
    });

    runner.addTest({
        id: 89,
        desc: 'IPropertyValue_Out: Size Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveSizeArray();
            var outVar = propertyValueTests.testSizeArray_IPropertyValueOut(myArray);
            verify(outVar.length, myArray.length, 'testSizeArray_IPropertyValueOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i].height, myArray[i].height, 'testSizeArray_IPropertyValueOut(myArray)[' + i + '].height');
                verify(outVar[i].width, myArray[i].width, 'testSizeArray_IPropertyValueOut(myArray)[' + i + '].width');
            }
        }
    });

    runner.addTest({
        id: 90,
        desc: 'IPropertyValue_Out: Rect Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveRectArray();
            var outVar = propertyValueTests.testRectArray_IPropertyValueOut(myArray);
            verify(outVar.length, myArray.length, 'testRectArray_IPropertyValueOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i].x, myArray[i].x, 'testRectArray_IPropertyValueOut(myArray)[' + i + '].x');
                verify(outVar[i].y, myArray[i].y, 'testRectArray_IPropertyValueOut(myArray)[' + i + '].y');
                verify(outVar[i].height, myArray[i].height, 'testRectArray_IPropertyValueOut(myArray)[' + i + '].height');
                verify(outVar[i].width, myArray[i].width, 'testRectArray_IPropertyValueOut(myArray)[' + i + '].width');
            }
        }
    });

    runner.addTest({
        id: 91,
        desc: 'IPropertyValue_Out: Enum Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveEnumArray();
            var outVar = propertyValueTests.testInspectable_IPropertyValueOut(myArray);
            verify(outVar.length, myArray.length, 'testInspectable_IPropertyValueOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testInspectable_IPropertyValueOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 92,
        desc: 'IPropertyValue_Out: Js Delegate Arrays from winrt',
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
            var outVar = propertyValueTests.testInspectable_IPropertyValueOut(myArray);
            verify(outVar.length, myArray.length, 'testInspectable_IPropertyValueOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar[i], myArray[i], 'testInspectable_IPropertyValueOut(myArray)[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 93,
        desc: 'IPropertyValue_Out: Winrt Delegate Arrays from winrt',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = propertyValueTests.receiveWinrtDelegateArray();
            var outVar = propertyValueTests.testInspectable_IPropertyValueOut(myArray);
            verify(outVar.length, myArray.length, 'testInspectable_IPropertyValueOut(myArray).length');
            for (var i = 0; i < myArray.length; i++) {
                verify(propertyValueTests.isSameDelegate(outVar[i], myArray[i]), true, 'testInspectable_IPropertyValueOut(myArray)[' + i + '] are same');
            }
        }
    });

    runner.addTest({
        id: 94,
        desc: 'IPropertyValue_Out: IVector<AsyncInfo>',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // Check we can marshal out as Inspectable
            var myVectorOfAsyncInfo = propertyValueTests.receiveVectorOfAsyncInfo();
            if (myVectorOfAsyncInfo[0].then) {
            } else {
                throw "Expected an async operation";
            }

            // Box and get it back
            var outVar = propertyValueTests.testBoxInspectable_IPropertyValueOut(myVectorOfAsyncInfo);
            verify(outVar, myVectorOfAsyncInfo, 'testBoxInspectable_IPropertyValueOut(myVectorOfAsyncInfo)');
        }
    });

    runner.addTest({
        id: 95,
        desc: 'IPropertyValue_Out: IVector<Guid>',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // Check we can marshal out as Inspectable
            var myVectorOfGuid = propertyValueTests.receiveVectorOfGuid();

            // Box and get it back
            var outVar = propertyValueTests.testBoxInspectable_IPropertyValueOut(myVectorOfGuid);
            verify(outVar, myVectorOfGuid, 'testBoxInspectable_IPropertyValueOut(myVectorOfGuid)');
        }
    });

    runner.addTest({
        id: 96,
        desc: 'IPropertyValue_Out: IVector<DateTime>',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // Check we can marshal out as Inspectable
            var myVectorOfDate = propertyValueTests.receiveVectorOfDate();

            // Box and get it back
            var outVar = propertyValueTests.testBoxInspectable_IPropertyValueOut(myVectorOfDate);
            verify(outVar, myVectorOfDate, 'testBoxInspectable_IPropertyValueOut(myVectorOfDate)');
        }
    });

    runner.addTest({
        id: 97,
        desc: 'IPropertyValue_Out: IVector<Delegate>',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // Check we can marshal out as Inspectable
            var myVectorOfDelegate = propertyValueTests.receiveVectorOfDelegate();

            // Box and get it back
            var outVar = propertyValueTests.testBoxInspectable_IPropertyValueOut(myVectorOfDelegate);
            verify(outVar, myVectorOfDelegate, 'testBoxInspectable_IPropertyValueOut(myVectorOfDelegate)');
        }
    });

    runner.addTest({
        id: 98,
        desc: 'IPropertyValue_Out: IVector<Enum>',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // Check we can marshal out as Inspectable
            var myVectorOfEnum = propertyValueTests.receiveVectorOfEnum();

            // Box and get it back
            var outVar = propertyValueTests.testBoxInspectable_IPropertyValueOut(myVectorOfEnum);
            verify(outVar, myVectorOfEnum, 'testBoxInspectable_IPropertyValueOut(myVectorOfEnum)');
        }
    });

    runner.addTest({
        id: 99,
        desc: 'IPropertyValue_Out: IVector<EventRegistrationToken>',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // Check we can marshal out as Inspectable
            var myVectorOfEventRegistration = propertyValueTests.receiveVectorOfEventRegistration();

            // Box and get it back
            var outVar = propertyValueTests.testBoxInspectable_IPropertyValueOut(myVectorOfEventRegistration);
            verify(outVar, myVectorOfEventRegistration, 'testBoxInspectable_IPropertyValueOut(myVectorOfEventRegistration)');
        }
    });

    runner.addTest({
        id: 100,
        desc: 'IPropertyValue_Out: IVector<TimeSpan>',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // Check we can marshal out as Inspectable
            var myVectorOfTimeSpan = propertyValueTests.receiveVectorOfTimeSpan();

            // Box and get it back
            var outVar = propertyValueTests.testBoxInspectable_IPropertyValueOut(myVectorOfTimeSpan);
            verify(outVar, myVectorOfTimeSpan, 'testBoxInspectable_IPropertyValueOut(myVectorOfTimeSpan)');
        }
    });

    runner.addTest({
        id: 101,
        desc: 'IPropertyValue_Out: Box null',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testBoxedNull_IPropertyValueOut(null);
            verify(outVar, null, "testBoxedNull_IPropertyValueOut(null)");
        }
    });

    runner.addTest({
        id: 102,
        desc: 'IPropertyValue_Out: Custom PropertyValue out with GRCN as IReference<Dimensions>',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testRCPV1_IPropertyValueOut();
            logger.comment(outVar);
            verify(outVar.length, 100, 'testRCPV1_IPropertyValueOut().length');
            verify(outVar.width, 20, 'testRCPV1_IPropertyValueOut().width');
        }
    });

    runner.addTest({
        id: 103,
        desc: 'IPropertyValue_Out: Custom PropertyValue out with GRCN as RCName',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            verify.exception(function () {
                var outVar = propertyValueTests.testRCPV2_IPropertyValueOut();
            }, TypeError, "testRCPV2_IPropertyValueOut");
        }
    });

    runner.addTest({
        id: 104,
        desc: 'IPropertyValue_Out: Custom PropertyValue out with GRCN as Windows.Foundation.IPropertyValue',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            verify.exception(function () {
                var outVar = propertyValueTests.testRCPV3_IPropertyValueOut();
            }, TypeError, "testRCPV3_IPropertyValueOut");
        }
    });

    runner.addTest({
        id: 105,
        desc: 'IPropertyValue_Out: Custom PropertyValue out with GRCN as IReference<Char16>',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testRCPV4_IPropertyValueOut();
            logger.comment(outVar);
            verify(outVar, 'D', 'testRCPV4_IPropertyValueOut()');
        }
    });

    runner.addTest({
        id: 106,
        desc: 'IPropertyValue_Out: Custom PropertyValue out with GRCN as RCName',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            verify.exception(function () {
                var outVar = propertyValueTests.testRCPV5_IPropertyValueOut();
            }, TypeError, "testRCPV5_IPropertyValueOut");
        }
    });

    runner.addTest({
        id: 107,
        desc: 'IPropertyValue_Out: Custom PropertyValue out with GRCN as Windows.Foundation.IPropertyValue',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            verify.exception(function () {
                var outVar = propertyValueTests.testRCPV6_IPropertyValueOut();
            }, TypeError, "testRCPV6_IPropertyValueOut");
        }
    });

    runner.addTest({
        id: 108,
        desc: 'IPropertyValue_In: Uint8 Arrays from winrt',
        pri: '0',
        preReq: function () {
            return (typeof WScript !== 'undefined');
        },
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myArray = WScript.CreateCanvasPixelArray([2, 3, 4, 5, 6, 7, 8, 9]);
            var outVar = propertyValueTests.testUInt8Array_IPropertyValueIn(myArray);
            verify(outVar.isValidType, true, 'testUInt8Array_IPropertyValueIn(myArray).isValidType');
            verify(outVar.outValue.length, myArray.length, 'testUInt8Array_IPropertyValueIn(myArray).outValue.length');
            for (var i = 0; i < myArray.length; i++) {
                verify(outVar.outValue[i], myArray[i], 'testUInt8Array_IPropertyValueIn(myArray).outValue[' + i + ']');
            }
        }
    });

    runner.addTest({
        id: 109,
        desc: 'IPropertyValue_Out: IMap<String, Object>',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // Check we can marshal out as Inspectable
            var myMapOfStringAndInspectable = propertyValueTests.receiveMapOfStringAndInspectable();

            // Box and get it back
            var outVar = propertyValueTests.testBoxInspectable_IPropertyValueOut(myMapOfStringAndInspectable);
            verify(outVar, myMapOfStringAndInspectable, 'testBoxInspectable_IPropertyValueOut(myMapOfStringAndInspectable)');
        }
    });

    Loader42_FileName = 'Marshaling into and out of IPropertyValue';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
