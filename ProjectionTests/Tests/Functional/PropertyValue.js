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
        desc: 'PropertyValue_In: Custom PropertyValue out with GRCN as IReference<Dimensions>',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            verify.exception(function () {
                propertyValueTests.testRCPV1_PropertyValueIn(200);
            }, TypeError, "testRCPV1_PropertyValueIn(200)");
        }
    });

    runner.addTest({
        id: 2,
        desc: 'PropertyValue_In: Custom PropertyValue out with GRCN as RCName',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            verify.exception(function () {
                propertyValueTests.testRCPV2_PropertyValueIn(200);
            }, TypeError, "testRCPV2_PropertyValueIn(200)");
        }
    });

    runner.addTest({
        id: 3,
        desc: 'PropertyValue_In: Custom PropertyValue out with GRCN as Windows.Foundation.IPropertyValue',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            verify.exception(function () {
                propertyValueTests.testRCPV3_PropertyValueIn(200);
            }, TypeError, "testRCPV3_PropertyValueIn(200)");
        }
    });

    runner.addTest({
        id: 4,
        desc: 'PropertyValue_In: Custom PropertyValue out with GRCN as IReference<Char16>',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            verify.exception(function () {
                propertyValueTests.testRCPV4_PropertyValueIn('D');
            }, TypeError, "testRCPV4_PropertyValueIn('D')");
        }
    });

    runner.addTest({
        id: 5,
        desc: 'PropertyValue_In: Custom PropertyValue out with GRCN as RCName',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            verify.exception(function () {
                propertyValueTests.testRCPV5_PropertyValueIn('D');
            }, TypeError, "testRCPV5_PropertyValueIn('D')");
        }
    });

    runner.addTest({
        id: 6,
        desc: 'PropertyValue_In: Custom PropertyValue out with GRCN as Windows.Foundation.IPropertyValue',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            verify.exception(function () {
                propertyValueTests.testRCPV6_PropertyValueIn('D');
            }, TypeError, "testRCPV6_PropertyValueIn('D')");
        }
    });

    runner.addTest({
        id: 7,
        desc: 'PropertyValue_Out: Custom PropertyValue out with GRCN as IReference<Dimensions>',
        pri: '0',
        preReq: function () {
            // IPropertyValue is not visible to CLR user, tests those strictly depend on IPropertyValue in the signature are not supported
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testRCPV1_PropertyValueOut();
            logger.comment(outVar);
            verify(outVar.length, 100, 'testRCPV1_PropertyValueOut().length');
            verify(outVar.width, 20, 'testRCPV1_PropertyValueOut().width');
        }
    });

    runner.addTest({
        id: 8,
        desc: 'PropertyValue_Out: Custom PropertyValue out with GRCN as RCName',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            verify.exception(function () {
                var outVar = propertyValueTests.testRCPV2_PropertyValueOut();
            }, TypeError, "testRCPV2_PropertyValueOut");
        }
    });

    runner.addTest({
        id: 9,
        desc: 'PropertyValue_Out: Custom PropertyValue out with GRCN as Windows.Foundation.IPropertyValue',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            verify.exception(function () {
                var outVar = propertyValueTests.testRCPV3_PropertyValueOut();
            }, TypeError, "testRCPV3_PropertyValueOut");
        }
    });

    runner.addTest({
        id: 10,
        desc: 'PropertyValue_Out: Custom PropertyValue out with GRCN as IReference<Char16>',
        pri: '0',
        preReq: function () {
            // IPropertyValue is not visible to CLR user, tests those strictly depend on IPropertyValue in the signature are not supported
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testRCPV4_PropertyValueOut();
            logger.comment(outVar);
            verify(outVar, 'D', 'testRCPV4_PropertyValueOut()');
        }
    });

    runner.addTest({
        id: 11,
        desc: 'PropertyValue_Out: Custom PropertyValue out with GRCN as RCName',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            verify.exception(function () {
                var outVar = propertyValueTests.testRCPV5_PropertyValueOut();
            }, TypeError, "testRCPV5_PropertyValueOut");
        }
    });

    runner.addTest({
        id: 12,
        desc: 'PropertyValue_Out: Custom PropertyValue out with GRCN as Windows.Foundation.IPropertyValue',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            verify.exception(function () {
                var outVar = propertyValueTests.testRCPV6_PropertyValueOut();
            }, TypeError, "testRCPV6_PropertyValueOut");
        }
    });

    runner.addTest({
        id: 13,
        desc: 'PropertyValue_In: CanvasPixelArray from winrt',
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

    Loader42_FileName = 'Marshaling into and out of PropertyValue';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
