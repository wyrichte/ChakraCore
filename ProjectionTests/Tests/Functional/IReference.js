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
        // Check the sizes
        verify(myMap.size, myExpectedMap.size, myMapString + ".size");

        // Loop through each item in the map
        var myMapIterator = myMap.first();
        var myExpectedMapIterator = myExpectedMap.first();

        var i = 1;
        while (myExpectedMapIterator.hasCurrent) {
            // Key
            var myDimension = myMapIterator.current.key;
            var myExpectedDimension = myExpectedMapIterator.current.key;

            verify(myDimension.length, myExpectedDimension.length, '(Key: ' + i + ') myDimension.length');
            verify(myDimensionInspectable.width, myExpectedDimension.width, '(Key: ' + i + ') myDimension.width');

            // Value
            var myVector = myMapIterator.current.value;
            var myExpectedVector = myExpectedMapIterator.current.value;
            VerifyArrayAndVectorContents('(Value: ' + i + ') myVector', myVector, '(Value: ' + i + ') myExpectedVector', myExpectedVector);

            // Go to next key value pair
            myMapIterator.moveNext();
            myMapInspectableIterator.moveNext();
            i++;
        }
    }

    runner.addTest({
        id: 1,
        desc: 'Reference_In: null',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testBoolean_ReferenceIn(null);
            verify(outVar.isValidType, true, "testBoolean_ReferenceIn(null).isValidType");
            verify(outVar.isNull, true, "testBoolean_ReferenceIn(null).isNull");

            verify.exception(function () {
                outVar = propertyValueTests.testString_ReferenceIn(null);
            }, Error, "testString_ReferenceIn(null)");

            outVar = propertyValueTests.testChar16_ReferenceIn(null);
            verify(outVar.isValidType, true, "testChar16_ReferenceIn(null).isValidType");
            verify(outVar.isNull, true, "testChar16_ReferenceIn(null).isNull");

            outVar = propertyValueTests.testUInt8_ReferenceIn(null);
            verify(outVar.isValidType, true, "testUInt8_ReferenceIn(null).isValidType");
            verify(outVar.isNull, true, "testUInt8_ReferenceIn(null).isNull");

            outVar = propertyValueTests.testInt16_ReferenceIn(null);
            verify(outVar.isValidType, true, "testInt16_ReferenceIn(null).isValidType");
            verify(outVar.isNull, true, "testInt16_ReferenceIn(null).isNull");

            outVar = propertyValueTests.testUInt16_ReferenceIn(null);
            verify(outVar.isValidType, true, "testUInt16_ReferenceIn(null).isValidType");
            verify(outVar.isNull, true, "testUInt16_ReferenceIn(null).isNull");

            outVar = propertyValueTests.testInt32_ReferenceIn(null);
            verify(outVar.isValidType, true, "testInt32_ReferenceIn(null).isValidType");
            verify(outVar.isNull, true, "testInt32_ReferenceIn(null).isNull");

            outVar = propertyValueTests.testUInt32_ReferenceIn(null);
            verify(outVar.isValidType, true, "testUInt32_ReferenceIn(null).isValidType");
            verify(outVar.isNull, true, "testUInt32_ReferenceIn(null).isNull");

            outVar = propertyValueTests.testInt64_ReferenceIn(null);
            verify(outVar.isValidType, true, "testInt64_ReferenceIn(null).isValidType");
            verify(outVar.isNull, true, "testInt64_ReferenceIn(null).isNull");

            outVar = propertyValueTests.testUInt64_ReferenceIn(null);
            verify(outVar.isValidType, true, "testUInt64_ReferenceIn(null).isValidType");
            verify(outVar.isNull, true, "testUInt64_ReferenceIn(null).isNull");

            outVar = propertyValueTests.testFloat_ReferenceIn(null);
            verify(outVar.isValidType, true, "testFloat_ReferenceIn(null).isValidType");
            verify(outVar.isNull, true, "testFloat_ReferenceIn(null).isNull");

            outVar = propertyValueTests.testDouble_ReferenceIn(null);
            verify(outVar.isValidType, true, "testDouble_ReferenceIn(null).isValidType");
            verify(outVar.isNull, true, "testDouble_ReferenceIn(null).isNull");

            outVar = propertyValueTests.testGuid_ReferenceIn(null);
            verify(outVar.isValidType, true, "testGuid_ReferenceIn(null).isValidType");
            verify(outVar.isNull, true, "testGuid_ReferenceIn(null).isNull");

            outVar = propertyValueTests.testDate_ReferenceIn(null);
            verify(outVar.isValidType, true, "testDate_ReferenceIn(null).isValidType");
            verify(outVar.isNull, true, "testDate_ReferenceIn(null).isNull");

            verify.exception(function () {
                outVar = propertyValueTests.testInspectable_ReferenceIn(null);
            }, Error, "testInspectable_ReferenceIn(null)");

            verify.exception(function () {
                outVar = propertyValueTests.testFish_ReferenceIn(null);
            }, Error, "testFish_ReferenceIn(null)");

            verify.exception(function () {
                outVar = propertyValueTests.testVector_ReferenceIn(null);
            }, Error, "testVector_ReferenceIn(null)");

            verify.exception(function () {
                outVar = propertyValueTests.testRCIObservable_ReferenceIn(null);
            }, Error, "testRCIObservable_ReferenceIn(null)");

            outVar = propertyValueTests.testDimensions_ReferenceIn(null);
            verify(outVar.isValidType, true, "testDimensions_ReferenceIn(null).isValidType");
            verify(outVar.isNull, true, "testDimensions_ReferenceIn(null).isNull");

            outVar = propertyValueTests.testTimeSpan_ReferenceIn(null);
            verify(outVar.isValidType, true, "testTimeSpan_ReferenceIn(null).isValidType");
            verify(outVar.isNull, true, "testTimeSpan_ReferenceIn(null).isNull");

            outVar = propertyValueTests.testPoint_ReferenceIn(null);
            verify(outVar.isValidType, true, "testPoint_ReferenceIn(null).isValidType");
            verify(outVar.isNull, true, "testPoint_ReferenceIn(null).isNull");

            outVar = propertyValueTests.testSize_ReferenceIn(null);
            verify(outVar.isValidType, true, "testSize_ReferenceIn(null).isValidType");
            verify(outVar.isNull, true, "testSize_ReferenceIn(null).isNull");

            outVar = propertyValueTests.testRect_ReferenceIn(null);
            verify(outVar.isValidType, true, "testRect_ReferenceIn(null).isValidType");
            verify(outVar.isNull, true, "testRect_ReferenceIn(null).isNull");

            outVar = propertyValueTests.testEnum_ReferenceIn(null);
            verify(outVar.isValidType, true, "testEnum_ReferenceIn(null).isValidType");
            verify(outVar.isNull, true, "testEnum_ReferenceIn(null).isNull");

            verify.exception(function () {
                outVar = propertyValueTests.testDelegate_ReferenceIn(null);
            }, Error, "testDelegate_ReferenceIn(null)");
        }
    });

    runner.addTest({
        id: 2,
        desc: 'Reference_In: undefined',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testBoolean_ReferenceIn(undefined);
            verify(outVar.isValidType, true, "testBoolean_ReferenceIn(undefined).isValidType");
            verify(outVar.isNull, true, "testBoolean_ReferenceIn(undefined).isNull");

            verify.exception(function () {
                outVar = propertyValueTests.testString_ReferenceIn(undefined);
            }, Error, "testString_ReferenceIn(undefined)");

            outVar = propertyValueTests.testChar16_ReferenceIn(undefined);
            verify(outVar.isValidType, true, "testChar16_ReferenceIn(undefined).isValidType");
            verify(outVar.isNull, true, "testChar16_ReferenceIn(undefined).isNull");

            outVar = propertyValueTests.testUInt8_ReferenceIn(undefined);
            verify(outVar.isValidType, true, "testUInt8_ReferenceIn(undefined).isValidType");
            verify(outVar.isNull, true, "testUInt8_ReferenceIn(undefined).isNull");

            outVar = propertyValueTests.testInt16_ReferenceIn(undefined);
            verify(outVar.isValidType, true, "testInt16_ReferenceIn(undefined).isValidType");
            verify(outVar.isNull, true, "testInt16_ReferenceIn(undefined).isNull");

            outVar = propertyValueTests.testUInt16_ReferenceIn(undefined);
            verify(outVar.isValidType, true, "testUInt16_ReferenceIn(undefined).isValidType");
            verify(outVar.isNull, true, "testUInt16_ReferenceIn(undefined).isNull");

            outVar = propertyValueTests.testInt32_ReferenceIn(undefined);
            verify(outVar.isValidType, true, "testInt32_ReferenceIn(undefined).isValidType");
            verify(outVar.isNull, true, "testInt32_ReferenceIn(undefined).isNull");

            outVar = propertyValueTests.testUInt32_ReferenceIn(undefined);
            verify(outVar.isValidType, true, "testUInt32_ReferenceIn(undefined).isValidType");
            verify(outVar.isNull, true, "testUInt32_ReferenceIn(undefined).isNull");

            outVar = propertyValueTests.testInt64_ReferenceIn(undefined);
            verify(outVar.isValidType, true, "testInt64_ReferenceIn(undefined).isValidType");
            verify(outVar.isNull, true, "testInt64_ReferenceIn(undefined).isNull");

            outVar = propertyValueTests.testUInt64_ReferenceIn(undefined);
            verify(outVar.isValidType, true, "testUInt64_ReferenceIn(undefined).isValidType");
            verify(outVar.isNull, true, "testUInt64_ReferenceIn(undefined).isNull");

            outVar = propertyValueTests.testFloat_ReferenceIn(undefined);
            verify(outVar.isValidType, true, "testFloat_ReferenceIn(undefined).isValidType");
            verify(outVar.isNull, true, "testFloat_ReferenceIn(undefined).isNull");

            outVar = propertyValueTests.testDouble_ReferenceIn(undefined);
            verify(outVar.isValidType, true, "testDouble_ReferenceIn(undefined).isValidType");
            verify(outVar.isNull, true, "testDouble_ReferenceIn(undefined).isNull");

            outVar = propertyValueTests.testGuid_ReferenceIn(undefined);
            verify(outVar.isValidType, true, "testGuid_ReferenceIn(undefined).isValidType");
            verify(outVar.isNull, true, "testGuid_ReferenceIn(undefined).isNull");

            outVar = propertyValueTests.testDate_ReferenceIn(undefined);
            verify(outVar.isValidType, true, "testDate_ReferenceIn(undefined).isValidType");
            verify(outVar.isNull, true, "testDate_ReferenceIn(undefined).isNull");

            verify.exception(function () {
                outVar = propertyValueTests.testInspectable_ReferenceIn(undefined);
            }, Error, "testInspectable_ReferenceIn(undefined)");

            verify.exception(function () {
                outVar = propertyValueTests.testFish_ReferenceIn(undefined);
            }, Error, "testFish_ReferenceIn(undefined)");

            verify.exception(function () {
                outVar = propertyValueTests.testVector_ReferenceIn(undefined);
            }, Error, "testVector_ReferenceIn(undefined)");

            verify.exception(function () {
                outVar = propertyValueTests.testRCIObservable_ReferenceIn(undefined);
            }, Error, "testRCIObservable_ReferenceIn(undefined)");

            outVar = propertyValueTests.testDimensions_ReferenceIn(undefined);
            verify(outVar.isValidType, true, "testDimensions_ReferenceIn(undefined).isValidType");
            verify(outVar.isNull, true, "testDimensions_ReferenceIn(undefined).isNull");

            outVar = propertyValueTests.testTimeSpan_ReferenceIn(undefined);
            verify(outVar.isValidType, true, "testTimeSpan_ReferenceIn(undefined).isValidType");
            verify(outVar.isNull, true, "testTimeSpan_ReferenceIn(undefined).isNull");

            outVar = propertyValueTests.testPoint_ReferenceIn(undefined);
            verify(outVar.isValidType, true, "testPoint_ReferenceIn(undefined).isValidType");
            verify(outVar.isNull, true, "testPoint_ReferenceIn(undefined).isNull");

            outVar = propertyValueTests.testSize_ReferenceIn(undefined);
            verify(outVar.isValidType, true, "testSize_ReferenceIn(undefined).isValidType");
            verify(outVar.isNull, true, "testSize_ReferenceIn(undefined).isNull");

            outVar = propertyValueTests.testRect_ReferenceIn(undefined);
            verify(outVar.isValidType, true, "testRect_ReferenceIn(undefined).isValidType");
            verify(outVar.isNull, true, "testRect_ReferenceIn(undefined).isNull");

            outVar = propertyValueTests.testEnum_ReferenceIn(undefined);
            verify(outVar.isValidType, true, "testEnum_ReferenceIn(undefined).isValidType");
            verify(outVar.isNull, true, "testEnum_ReferenceIn(undefined).isNull");

            verify.exception(function () {
                outVar = propertyValueTests.testDelegate_ReferenceIn(undefined);
            }, Error, "testDelegate_ReferenceIn(undefined)");
        }
    });

    runner.addTest({
        id: 3,
        desc: 'Reference_In: Boolean',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            var outVar = propertyValueTests.testBoolean_ReferenceIn(true);
            verify(outVar.isNull, false, "testBoolean_ReferenceIn(true).isNull");
            verify(outVar.isValidType, true, "testBoolean_ReferenceIn(true).isValidType");
            verify(outVar.outValue, true, "testBoolean_ReferenceIn(true).outValue");

            outVar = propertyValueTests.testBoolean_ReferenceIn(false);
            verify(outVar.isNull, false, "testBoolean_ReferenceIn(false).isNull");
            verify(outVar.isValidType, true, "testBoolean_ReferenceIn(false).isValidType");
            verify(outVar.outValue, false, "testBoolean_ReferenceIn(false).outValue");

            outVar = propertyValueTests.testBoolean_ReferenceIn(new Boolean(0));
            verify(outVar.isNull, false, "testBoolean_ReferenceIn(new Boolean(0)).isNull");
            verify(outVar.isValidType, true, "testBoolean_ReferenceIn(new Boolean(0)).isValidType");
            verify(outVar.outValue, false, "testBoolean_ReferenceIn(new Boolean(0)).outValue");

            outVar = propertyValueTests.testBoolean_ReferenceIn(new Boolean(1));
            verify(outVar.isNull, false, "testBoolean_ReferenceIn(new Boolean(1)).isNull");
            verify(outVar.isValidType, true, "testBoolean_ReferenceIn(new Boolean(1)).isValidType");
            verify(outVar.outValue, true, "testBoolean_ReferenceIn(new Boolean(1)).outValue");

            outVar = propertyValueTests.testBoolean_ReferenceIn(new Boolean(""));
            verify(outVar.isNull, false, 'testBoolean_ReferenceIn(new Boolean("")).isNull');
            verify(outVar.isValidType, true, 'testBoolean_ReferenceIn(new Boolean("")).isValidType');
            verify(outVar.outValue, false, 'testBoolean_ReferenceIn(new Boolean("")).outValue');

            outVar = propertyValueTests.testBoolean_ReferenceIn(new Boolean(null));
            verify(outVar.isNull, false, "testBoolean_ReferenceIn(new Boolean(null)).isNull");
            verify(outVar.isValidType, true, "testBoolean_ReferenceIn(new Boolean(null)).isValidType");
            verify(outVar.outValue, false, "testBoolean_ReferenceIn(new Boolean(null)).outValue");

            outVar = propertyValueTests.testBoolean_ReferenceIn(new Boolean(NaN));
            verify(outVar.isNull, false, "testBoolean_ReferenceIn(new Boolean(NaN)).isNull");
            verify(outVar.isValidType, true, "testBoolean_ReferenceIn(new Boolean(NaN)).isValidType");
            verify(outVar.outValue, false, "testBoolean_ReferenceIn(new Boolean(NaN)).outValue");

            outVar = propertyValueTests.testBoolean_ReferenceIn(new Boolean("false"));
            verify(outVar.isNull, false, 'testBoolean_ReferenceIn(new Boolean("false")).isNull');
            verify(outVar.isValidType, true, 'testBoolean_ReferenceIn(new Boolean("false")).isValidType');
            verify(outVar.outValue, true, 'testBoolean_ReferenceIn(new Boolean("false")).outValue');
        }
    });

    runner.addTest({
        id: 4,
        desc: 'Reference_In: String',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            verify.exception(function () {
                var outVar = propertyValueTests.testString_ReferenceIn("String1");
            }, Error, 'testString_ReferenceIn("String1")');

            verify.exception(function () {
                var outVar = propertyValueTests.testString_ReferenceIn("");
            }, Error, 'testString_ReferenceIn("")');

            verify.exception(function () {
                var outVar = propertyValueTests.testString_ReferenceIn(new String(""));
            }, Error, 'testString_ReferenceIn(new String("String2"))');

            verify.exception(function () {
                var outVar = propertyValueTests.testString_ReferenceIn(new String(""));
            }, Error, 'testString_ReferenceIn(new String("String2"))');
        }
    });

    runner.addTest({
        id: 5,
        desc: 'Reference_In: Char16',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testChar16_ReferenceIn("a");
            verify(outVar.isNull, false, 'testChar16_ReferenceIn("a").isNull');
            verify(outVar.isValidType, true, 'testChar16_ReferenceIn("a").isValidType');
            verify(outVar.outValue, "a", 'testChar16_ReferenceIn("a").outValue');
        }
    });

    runner.addTest({
        id: 6,
        desc: 'Reference_In: UInt8',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testUInt8_ReferenceIn(0x18);
            verify(outVar.isNull, false, 'testCharUInt8_ReferenceIn(0x18).isNull');
            verify(outVar.isValidType, true, 'testUInt8_ReferenceIn(0x18).isValidType');
            verify(outVar.outValue, 0x18, 'testUInt8_ReferenceIn(0x18).outValue');

            outVar = propertyValueTests.testUInt8_ReferenceIn(0);
            verify(outVar.isNull, false, 'testCharUInt8_ReferenceIn(0).isNull');
            verify(outVar.isValidType, true, 'testUInt8_ReferenceIn(0).isValidType');
            verify(outVar.outValue, 0, 'testUInt8_ReferenceIn(0).outValue');
        }
    });

    runner.addTest({
        id: 7,
        desc: 'Reference_In: Int16',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testInt16_ReferenceIn(-40);
            verify(outVar.isNull, false, 'testInt16_ReferenceIn(-40).isNull');
            verify(outVar.isValidType, true, 'testInt16_ReferenceIn(-40).isValidType');
            verify(outVar.outValue, -40, 'testInt16_ReferenceIn(-40).outValue');

            outVar = propertyValueTests.testInt16_ReferenceIn(0);
            verify(outVar.isNull, false, 'testInt16_ReferenceIn(0).isNull');
            verify(outVar.isValidType, true, 'testInt16_ReferenceIn(0).isValidType');
            verify(outVar.outValue, 0, 'testInt16_ReferenceIn(0).outValue');
        }
    });

    runner.addTest({
        id: 8,
        desc: 'Reference_In: UInt16',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testUInt16_ReferenceIn(40);
            verify(outVar.isNull, false, 'testUInt16_ReferenceIn(40).isNull');
            verify(outVar.isValidType, true, 'testUInt16_ReferenceIn(40).isValidType');
            verify(outVar.outValue, 40, 'testUInt16_ReferenceIn(40).outValue');

            outVar = propertyValueTests.testUInt16_ReferenceIn(0);
            verify(outVar.isNull, false, 'testUInt16_ReferenceIn(0).isNull');
            verify(outVar.isValidType, true, 'testUInt16_ReferenceIn(0).isValidType');
            verify(outVar.outValue, 0, 'testUInt16_ReferenceIn(0).outValue');
        }
    });

    runner.addTest({
        id: 9,
        desc: 'Reference_In: Int32',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testInt32_ReferenceIn(-4000);
            verify(outVar.isNull, false, 'testCharInt32_ReferenceIn(-4000).isNull');
            verify(outVar.isValidType, true, 'testInt32_ReferenceIn(-4000).isValidType');
            verify(outVar.outValue, -4000, 'testInt32_ReferenceIn(-4000).outValue');

            outVar = propertyValueTests.testInt32_ReferenceIn(0);
            verify(outVar.isNull, false, 'testCharInt32_ReferenceIn(0).isNull');
            verify(outVar.isValidType, true, 'testInt32_ReferenceIn(0).isValidType');
            verify(outVar.outValue, 0, 'testInt32_ReferenceIn(0).outValue');
        }
    });

    runner.addTest({
        id: 10,
        desc: 'Reference_In: UInt32',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testUInt32_ReferenceIn(4000);
            verify(outVar.isNull, false, 'testCharUInt32_ReferenceIn(4000).isNull');
            verify(outVar.isValidType, true, 'testUInt32_ReferenceIn(4000).isValidType');
            verify(outVar.outValue, 4000, 'testUInt32_ReferenceIn(4000).outValue');

            outVar = propertyValueTests.testUInt32_ReferenceIn(0);
            verify(outVar.isNull, false, 'testCharUInt32_ReferenceIn(0).isNull');
            verify(outVar.isValidType, true, 'testUInt32_ReferenceIn(0).isValidType');
            verify(outVar.outValue, 0, 'testUInt32_ReferenceIn(0).outValue');
        }
    });

    runner.addTest({
        id: 11,
        desc: 'Reference_In: Int64',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testInt64_ReferenceIn(-4000);
            verify(outVar.isNull, false, 'testCharInt64_ReferenceIn(-4000).isNull');
            verify(outVar.isValidType, true, 'testInt64_ReferenceIn(-4000).isValidType');
            verify(outVar.outValue, -4000, 'testInt64_ReferenceIn(-4000).outValue');

            outVar = propertyValueTests.testInt64_ReferenceIn(0);
            verify(outVar.isNull, false, 'testCharInt64_ReferenceIn(0).isNull');
            verify(outVar.isValidType, true, 'testInt64_ReferenceIn(0).isValidType');
            verify(outVar.outValue, 0, 'testInt64_ReferenceIn(0).outValue');
        }
    });

    runner.addTest({
        id: 12,
        desc: 'Reference_In: UInt64',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testUInt64_ReferenceIn(4000);
            verify(outVar.isNull, false, 'testCharUInt64_ReferenceIn(4000).isNull');
            verify(outVar.isValidType, true, 'testUInt64_ReferenceIn(4000).isValidType');
            verify(outVar.outValue, 4000, 'testUInt64_ReferenceIn(4000).outValue');

            outVar = propertyValueTests.testUInt64_ReferenceIn(0);
            verify(outVar.isNull, false, 'testCharUInt64_ReferenceIn(0).isNull');
            verify(outVar.isValidType, true, 'testUInt64_ReferenceIn(0).isValidType');
            verify(outVar.outValue, 0, 'testUInt64_ReferenceIn(0).outValue');
        }
    });

    runner.addTest({
        id: 13,
        desc: 'Reference_In: Float',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testFloat_ReferenceIn(88.30000305175781);
            verify(outVar.isNull, false, 'testCharFloat_ReferenceIn(88.30000305175781).isNull');
            verify(outVar.isValidType, true, 'testFloat_ReferenceIn(88.30000305175781).isValidType');
            verify(outVar.outValue, 88.30000305175781, 'testFloat_ReferenceIn(88.30000305175781).outValue');

            outVar = propertyValueTests.testFloat_ReferenceIn(0);
            verify(outVar.isNull, false, 'testCharFloat_ReferenceIn(0).isNull');
            verify(outVar.isValidType, true, 'testFloat_ReferenceIn(0).isValidType');
            verify(outVar.outValue, 0, 'testFloat_ReferenceIn(0).outValue');
        }
    });

    runner.addTest({
        id: 14,
        desc: 'Reference_In: Double',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testDouble_ReferenceIn(88.3);
            verify(outVar.isNull, false, 'testCharDouble_ReferenceIn(88.3).isNull');
            verify(outVar.isValidType, true, 'testDouble_ReferenceIn(88.3).isValidType');
            verify(outVar.outValue, 88.3, 'testDouble_ReferenceIn(88.3).outValue');

            outVar = propertyValueTests.testDouble_ReferenceIn(0);
            verify(outVar.isNull, false, 'testCharDouble_ReferenceIn(0).isNull');
            verify(outVar.isValidType, true, 'testDouble_ReferenceIn(0).isValidType');
            verify(outVar.outValue, 0, 'testDouble_ReferenceIn(0).outValue');
        }
    });

    runner.addTest({
        id: 15,
        desc: 'Reference_In: Guid',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testGuid_ReferenceIn("{40435D4E-D37B-42E6-8A3E-9E17D020609E}");
            verify(outVar.isNull, false, 'testCharGuid_ReferenceIn("{40435D4E-D37B-42E6-8A3E-9E17D020609E}").isNull');
            verify(outVar.isValidType, true, 'testGuid_ReferenceIn("{40435D4E-D37B-42E6-8A3E-9E17D020609E}").isValidType');
            verify(outVar.outValue, "40435d4e-d37b-42e6-8a3e-9e17d020609e", 'testGuid_ReferenceIn("{40435D4E-D37B-42E6-8A3E-9E17D020609E}").outValue');
        }
    });

    runner.addTest({
        id: 16,
        desc: 'Reference_In: Date',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // Winrt Date
            var myElephant = new Animals.Elephant(1);
            var winrtDate = myElephant.getAge();
            var outVar = propertyValueTests.testDate_ReferenceIn(winrtDate);
            verify(outVar.isNull, false, 'testDate_ReferenceIn(winrtDate).isNull');
            verify(outVar.isValidType, true, 'testDate_ReferenceIn(winrtDate).isValidType');
            verify(outVar.outValue, winrtDate, 'testDate_ReferenceIn(winrtDate).outValue');

            // Js Date:
            var jsDate = new Date(79, 5, 24, 11, 33, 0);
            outVar = propertyValueTests.testDate_ReferenceIn(jsDate);
            verify(outVar.isNull, false, 'testDate_ReferenceIn(jsDate).isNull');
            verify(outVar.isValidType, true, 'testDate_ReferenceIn(jsDate).isValidType');
            verify(outVar.outValue, jsDate, 'testDate_ReferenceIn(jsDate).outValue');
        }
    });

    runner.addTest({
        id: 17,
        desc: 'Reference_In: Inspectable',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            var myAnimal = new Animals.Animal(1);
            verify.exception(function () {
                var outVar = propertyValueTests.testInspectable_ReferenceIn(myAnimal);
            }, Error, 'testInspectable_ReferenceIn(myAnimal)');

            var myFish = new Animals.Fish();
            verify.exception(function () {
                var outVar = propertyValueTests.testInspectable_ReferenceIn(myFish);
            }, Error, 'testInspectable_ReferenceIn(myFish)');

            var myVector = myAnimal.getVector();
            verify.exception(function () {
                var outVar = propertyValueTests.testInspectable_ReferenceIn(myVector);
            }, Error, 'testInspectable_ReferenceIn(myVector)');

            var myObservableVector = new Animals.RCIObservable();
            verify.exception(function () {
                var outVar = propertyValueTests.testInspectable_ReferenceIn(myObservableVector);
            }, Error, 'testInspectable_ReferenceIn(myObservableVector)');
        }
    });

    runner.addTest({
        id: 18,
        desc: 'Reference_In: Animal',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            var myAnimal = new Animals.Animal(1);
            verify.exception(function () {
                var outVar = propertyValueTests.testAnimal_ReferenceIn(myAnimal);
            }, Error, 'testAnimal_ReferenceIn(myAnimal)');

            var myFish = new Animals.Fish();
            verify.exception(function () {
                var outVar = propertyValueTests.testAnimal_ReferenceIn(myFish);
            }, Error, 'testAnimal_ReferenceIn(myFish)');

            var myVector = myAnimal.getVector();
            verify.exception(function () {
                var outVar = propertyValueTests.testAnimal_ReferenceIn(myVector);
            }, Error, 'testAnimal_ReferenceIn(myVector)');

            var myObservableVector = new Animals.RCIObservable();
            verify.exception(function () {
                var outVar = propertyValueTests.testAnimal_ReferenceIn(myObservableVector);
            }, Error, 'testAnimal_ReferenceIn(myObservableVector)');
        }
    });

    runner.addTest({
        id: 19,
        desc: 'Reference_In: IFish',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            var myAnimal = new Animals.Animal(1);
            verify.exception(function () {
                var outVar = propertyValueTests.testFish_ReferenceIn(myAnimal);
            }, Error, 'testFish_ReferenceIn(myAnimal)');

            var myFish = new Animals.Fish();
            verify.exception(function () {
                var outVar = propertyValueTests.testFish_ReferenceIn(myFish);
            }, Error, 'testFish_ReferenceIn(myFish)');

            var myVector = myAnimal.getVector();
            verify.exception(function () {
                var outVar = propertyValueTests.testFish_ReferenceIn(myVector);
            }, Error, 'testFish_ReferenceIn(myVector)');

            var myObservableVector = new Animals.RCIObservable();
            verify.exception(function () {
                var outVar = propertyValueTests.testFish_ReferenceIn(myObservableVector);
            }, Error, 'testFish_ReferenceIn(myObservableVector)');
        }
    });

    runner.addTest({
        id: 20,
        desc: 'Reference_In: IVector<int>',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            var myAnimal = new Animals.Animal(1);
            verify.exception(function () {
                var outVar = propertyValueTests.testVector_ReferenceIn(myAnimal);
            }, Error, 'testVector_ReferenceIn(myAnimal)');

            var myFish = new Animals.Fish();
            verify.exception(function () {
                var outVar = propertyValueTests.testVector_ReferenceIn(myFish);
            }, Error, 'testVector_ReferenceIn(myFish)');

            var myVector = myAnimal.getVector();
            verify.exception(function () {
                var outVar = propertyValueTests.testVector_ReferenceIn(myVector);
            }, Error, 'testVector_ReferenceIn(myVector)');

            var myObservableVector = new Animals.RCIObservable();
            verify.exception(function () {
                var outVar = propertyValueTests.testVector_ReferenceIn(myObservableVector);
            }, Error, 'testVector_ReferenceIn(myObservableVector)');
        }
    });

    runner.addTest({
        id: 21,
        desc: 'Reference_In: Class with IVector<int> as default interface',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            var myAnimal = new Animals.Animal(1);
            verify.exception(function () {
                var outVar = propertyValueTests.testRCIObservable_ReferenceIn(myAnimal);
            }, Error, 'testRCIObservable_ReferenceIn(myAnimal)');

            var myFish = new Animals.Fish();
            verify.exception(function () {
                var outVar = propertyValueTests.testRCIObservable_ReferenceIn(myFish);
            }, Error, 'testRCIObservable_ReferenceIn(myFish)');

            var myVector = myAnimal.getVector();
            verify.exception(function () {
                var outVar = propertyValueTests.testRCIObservable_ReferenceIn(myVector);
            }, Error, 'testRCIObservable_ReferenceIn(myVector)');

            var myObservableVector = new Animals.RCIObservable();
            verify.exception(function () {
                var outVar = propertyValueTests.testRCIObservable_ReferenceIn(myObservableVector);
            }, Error, 'testRCIObservable_ReferenceIn(myObservableVector)');
        }
    });

    runner.addTest({
        id: 22,
        desc: 'Reference_In: User defined struct',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            var myDimensions = {
                length: 10,
                width: 40
            };
            var outVar = propertyValueTests.testDimensions_ReferenceIn(myDimensions);
            verify(outVar.isNull, false, 'testDimensions_ReferenceIn(myDimensions).isNull');
            verify(outVar.isValidType, true, 'testDimensions_ReferenceIn(myDimensions).isValidType');
            verify(outVar.outValue.length, myDimensions.length, 'testDimensions_ReferenceIn(myDimensions).outValue.length');
            verify(outVar.outValue.width, myDimensions.width, 'testDimensions_ReferenceIn(myDimensions).outValue.width');
        }
    });

    runner.addTest({
        id: 23,
        desc: 'Reference_In: TimeSpan',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myTimeSpan = 40;
            var outVar = propertyValueTests.testTimeSpan_ReferenceIn(myTimeSpan);
            verify(outVar.isNull, false, 'testTimeSpan_ReferenceIn(myTimeSpan).isNull');
            verify(outVar.isValidType, true, 'testTimeSpan_ReferenceIn(myTimeSpan).isValidType');
            verify(outVar.outValue, myTimeSpan, 'testTimeSpan_ReferenceIn(myTimeSpan).outValue');
        }
    });

    runner.addTest({
        id: 24,
        desc: 'Reference_In: Point',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            var myPoint = {
                x: 40,
                y: 70
            };
            var outVar = propertyValueTests.testPoint_ReferenceIn(myPoint);
            verify(outVar.isNull, false, 'testPoint_ReferenceIn(myPoint).isNull');
            verify(outVar.isValidType, true, 'testPoint_ReferenceIn(myPoint).isValidType');
            verify(outVar.outValue.x, myPoint.x, 'testPoint_ReferenceIn(myPoint).outValue.x');
            verify(outVar.outValue.y, myPoint.y, 'testPoint_ReferenceIn(myPoint).outValue.y');
        }
    });

    runner.addTest({
        id: 25,
        desc: 'Reference_In: Size',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            var mySize = {
                width: 40,
                height: 70
            };
            var outVar = propertyValueTests.testSize_ReferenceIn(mySize);
            verify(outVar.isNull, false, 'testSize_ReferenceIn(mySize).isNull');
            verify(outVar.isValidType, true, 'testSize_ReferenceIn(mySize).isValidType');
            verify(outVar.outValue.width, mySize.width, 'testSize_ReferenceIn(mySize).outValue.width');
            verify(outVar.outValue.height, mySize.height, 'testSize_ReferenceIn(mySize).outValue.height');
        }
    });

    runner.addTest({
        id: 26,
        desc: 'Reference_In: Rect',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            var myRect = {
                x: 40,
                y: 70,
                width: 80,
                height: 400
            };
            var outVar = propertyValueTests.testRect_ReferenceIn(myRect);
            verify(outVar.isNull, false, 'testRect_ReferenceIn(myRect).isNull');
            verify(outVar.isValidType, true, 'testRect_ReferenceIn(myRect).isValidType');
            verify(outVar.outValue.x, myRect.x, 'testRect_ReferenceIn(myRect).outValue.x');
            verify(outVar.outValue.y, myRect.y, 'testRect_ReferenceIn(myRect).outValue.y');
            verify(outVar.outValue.width, myRect.width, 'testSize_ReferenceIn(myRect).outValue.width');
            verify(outVar.outValue.height, myRect.height, 'testSize_ReferenceIn(myRect).outValue.height');
        }
    });

    runner.addTest({
        id: 27,
        desc: 'Reference_In: Enum',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testEnum_ReferenceIn(Animals.Phylum.cycliophora);
            verify(outVar.isNull, false, 'testEnum_ReferenceIn(Animals.Phylum.cycliophora).isNull');
            verify(outVar.isValidType, true, 'testEnum_ReferenceIn(Animals.Phylum.cycliophora).isValidType');
            verify(outVar.outValue, Animals.Phylum.cycliophora, 'testEnum_ReferenceIn(Animals.Phylum.cycliophora).outValue');
        }
    });

    runner.addTest({
        id: 28,
        desc: 'Reference_In: Js Delegate',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            function myDelegate() {
            }

            verify.exception(function () {
                var outVar = propertyValueTests.testDelegate_ReferenceIn(myDelegate);
            }, Error, 'testDelegate_ReferenceIn(myDelegate)');
        }
    });

    runner.addTest({
        id: 29,
        desc: 'Reference_In: Winrt Delegate',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myWinRTDelegate = (new Animals.Animal(1)).getNativeDelegateAsOutParam();

            verify.exception(function () {
                var outVar = propertyValueTests.testDelegate_ReferenceIn(myWinRTDelegate);
            }, Error, 'testDelegate_ReferenceIn(myWinRTDelegate)');
        }
    });

    runner.addTest({
        id: 30,
        desc: 'Reference_Out: Boolean',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testBoolean_ReferenceOut(true);
            verify(outVar, true, "testBoolean_ReferenceOut(true)");

            outVar = propertyValueTests.testBoolean_ReferenceOut(false);
            verify(outVar, false, "testBoolean_ReferenceOut(false)");
        }
    });

    runner.addTest({
        id: 32,
        desc: 'Reference_Out: Char16',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testChar16_ReferenceOut("S");
            verify(outVar, "S", 'testChar16_ReferenceOut("S")');
        }
    });

    runner.addTest({
        id: 33,
        desc: 'Reference_Out: UInt8',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testUInt8_ReferenceOut(2);
            verify(outVar, 2, 'testUInt8_ReferenceOut(2)');
        }
    });

    runner.addTest({
        id: 34,
        desc: 'Reference_Out: Int16',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testInt16_ReferenceOut(2);
            verify(outVar, 2, 'testInt16_ReferenceOut(2)');
        }
    });

    runner.addTest({
        id: 35,
        desc: 'Reference_Out: UInt16',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testUInt16_ReferenceOut(2);
            verify(outVar, 2, 'testUInt16_ReferenceOut(2)');
        }
    });

    runner.addTest({
        id: 36,
        desc: 'Reference_Out: Int32',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testInt32_ReferenceOut(2);
            verify(outVar, 2, 'testInt32_ReferenceOut(2)');
        }
    });

    runner.addTest({
        id: 37,
        desc: 'Reference_Out: UInt32',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testUInt32_ReferenceOut(2);
            verify(outVar, 2, 'testUInt32_ReferenceOut(2)');
        }
    });

    runner.addTest({
        id: 38,
        desc: 'Reference_Out: Int64',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testInt64_ReferenceOut(2);
            verify(outVar, 2, 'testInt64_ReferenceOut(2)');
        }
    });

    runner.addTest({
        id: 39,
        desc: 'Reference_Out: UInt64',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testUInt64_ReferenceOut(2);
            verify(outVar, 2, 'testUInt64_ReferenceOut(2)');
        }
    });

    runner.addTest({
        id: 40,
        desc: 'Reference_Out: Float',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testFloat_ReferenceOut(2);
            verify(outVar, 2, 'testFloat_ReferenceOut(2)');
        }
    });

    runner.addTest({
        id: 41,
        desc: 'Reference_Out: Double',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testDouble_ReferenceOut(2);
            verify(outVar, 2, 'testDouble_ReferenceOut(2)');
        }
    });

    runner.addTest({
        id: 42,
        desc: 'Reference_Out: Guid',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testGuid_ReferenceOut("{10C27311-B58F-476C-BFF4-6689B8A19836}");
            verify(outVar, "10c27311-b58f-476c-bff4-6689b8a19836", 'testGuid_ReferenceOut("{10C27311-B58F-476C-BFF4-6689B8A19836}")');
        }
    });

    runner.addTest({
        id: 43,
        desc: 'Reference_Out: Date',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            // Winrt Date
            var myElephant = new Animals.Elephant(1);
            var winrtDate = myElephant.getAge();
            var outVar = propertyValueTests.testDate_ReferenceOut(winrtDate);
            verify(outVar, winrtDate, 'testDate_ReferenceOut(winrtDate)');

            // Js Date:
            var jsDate = new Date(79, 5, 24, 11, 33, 0);
            outVar = propertyValueTests.testDate_ReferenceOut(jsDate);
            verify(outVar, jsDate, 'testDate_ReferenceOut(jsDate)');
        }
    });

    runner.addTest({
        id: 44,
        desc: 'Reference_Out: User defined struct',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            var myDimensions = {
                length: 10,
                width: 40
            };
            var outVar = propertyValueTests.testDimensions_ReferenceOut(myDimensions);
            verify(outVar.length, myDimensions.length, 'testDimensions_ReferenceOut(myDimensions).length');
            verify(outVar.width, myDimensions.width, 'testDimensions_ReferenceOut(myDimensions).width');
        }
    });

    runner.addTest({
        id: 45,
        desc: 'Reference_Out: TimeSpan',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var myTimeSpan = 1000 * 60 * 60 * 24;
            var outVar = propertyValueTests.testTimeSpan_ReferenceOut(myTimeSpan);
            verify(outVar, myTimeSpan, 'testTimeSpan_ReferenceOut(myTimeSpan)');
        }
    });

    runner.addTest({
        id: 46,
        desc: 'Reference_Out: Point',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            var myPoint = {
                x: 40,
                y: 70
            };
            var outVar = propertyValueTests.testPoint_ReferenceOut(myPoint);
            verify(outVar.x, myPoint.x, 'testPoint_ReferenceOut(myPoint).x');
            verify(outVar.y, myPoint.y, 'testPoint_ReferenceOut(myPoint).y');
        }
    });

    runner.addTest({
        id: 47,
        desc: 'Reference_Out: Size',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            var mySize = {
                width: 40,
                height: 70
            };
            var outVar = propertyValueTests.testSize_ReferenceOut(mySize);
            verify(outVar.width, mySize.width, 'testSize_ReferenceOut(mySize).width');
            verify(outVar.height, mySize.height, 'testSize_ReferenceOut(mySize).height');
        }
    });

    runner.addTest({
        id: 48,
        desc: 'Reference_Out: Rect',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            var myRect = {
                x: 40,
                y: 70,
                width: 80,
                height: 400
            };
            var outVar = propertyValueTests.testRect_ReferenceOut(myRect);
            verify(outVar.x, myRect.x, 'testRect_ReferenceOut(myRect).x');
            verify(outVar.y, myRect.y, 'testRect_ReferenceOut(myRect).y');
            verify(outVar.width, myRect.width, 'testSize_ReferenceOut(myRect).width');
            verify(outVar.height, myRect.height, 'testSize_ReferenceOut(myRect).height');
        }
    });

    runner.addTest({
        id: 49,
        desc: 'Reference_Out: Enum',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testEnum_ReferenceOut(Animals.Phylum.cycliophora);
            verify(outVar, Animals.Phylum.cycliophora, 'testEnum_ReferenceOut(Animals.Phylum.cycliophora)');
        }
    });

    Loader42_FileName = 'Marshaling in and out of IReference';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
