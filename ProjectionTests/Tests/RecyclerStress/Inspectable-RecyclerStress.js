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
        }
    });

    runner.addTest({
        id: 3,
        desc: 'Inspectable_In: String',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testString_InspectableIn("String1");
            verify(outVar.isValidType, true, 'testString_InspectableIn("String1").isValidType');
            verify(outVar.outValue, "String1", 'testString_InspectableIn("String1").outValue');

            outVar = propertyValueTests.testString_InspectableIn(new String("String2"));
            verify(outVar.isValidType, true, 'testString_InspectableIn(new String("String2")).isValidType');
            verify(outVar.outValue, "String2", 'testString_InspectableIn(new String("String2")).outValue');
        }
    });

    runner.addTest({
        id: 4,
        desc: 'Inspectable_In: Number',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testNumber_InspectableIn(15);
            verify(outVar.isValidType, true, 'testNumber_InspectableIn(15).isValidType');
            verify(outVar.outValue, 15, 'testNumber_InspectableIn(15).outValue');

            outVar = propertyValueTests.testNumber_InspectableIn(new Number(15));
            verify(outVar.isValidType, true, 'testNumber_InspectableIn(new Number(15)).isValidType');
            verify(outVar.outValue, 15, 'testNumber_InspectableIn(new Number(15)).outValue');
        }
    });

    runner.addTest({
        id: 5,
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
        id: 6,
        desc: 'Inspectable_In: unboxed inspectable',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            var myAnimal = new Animals.Animal(1);
            var outVar = propertyValueTests.testInspectable_InspectableIn(myAnimal);
            verify(outVar.isValidType, true, 'testInspectable_InspectableIn(new Animals.Animal(1)).isValidType');
            verify(outVar.outValue, myAnimal, 'testInspectable_InspectableIn(new Animals.Animal(1)).outValue');
        }
    });

    runner.addTest({
        id: 7,
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
        id: 8,
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
        id: 9,
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
        id: 10,
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
        id: 11,
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
        id: 12,
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
        id: 13,
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
        id: 14,
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
        id: 15,
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
        id: 16,
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
        id: 17,
        desc: 'Inspectable_Out: null',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testNull_InspectableOut();
            verify(outVar, null, "testNull_InspectableOut()");
        }
    });

    runner.addTest({
        id: 18,
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
        id: 19,
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
        id: 20,
        desc: 'Inspectable_Out: Int32',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testInt32_InspectableOut(2);
            verify(outVar, 2, 'testInt32_InspectableOut(2)');
        }
    });

    runner.addTest({
        id: 21,
        desc: 'Inspectable_Out: Guid',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testGuid_InspectableOut("{10C27311-B58F-476C-BFF4-6689B8A19836}");
            verify(outVar, "10c27311-b58f-476c-bff4-6689b8a19836", 'testGuid_InspectableOut("{10C27311-B58F-476C-BFF4-6689B8A19836}")');
        }
    });

    runner.addTest({
        id: 22,
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
        id: 23,
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
        id: 24,
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
        id: 25,
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
        id: 26,
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
        id: 27,
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
        id: 28,
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
        id: 29,
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
        id: 30,
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
        id: 31,
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
        id: 32,
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
        id: 33,
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
        id: 34,
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
        id: 35,
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
        id: 36,
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
        id: 37,
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
        id: 38,
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
        id: 39,
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
        id: 40,
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

    Loader42_FileName = 'RecyclerStress - inspectable.js';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
