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
        desc: 'Reference_In: Boolean',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();

            var outVar = propertyValueTests.testBoolean_ReferenceIn(true);
            verify(outVar.isNull, false, "testBoolean_ReferenceIn(true).isNull");
            verify(outVar.isValidType, true, "testBoolean_ReferenceIn(true).isValidType");
            verify(outVar.outValue, true, "testBoolean_ReferenceIn(true).outValue");

            outVar = propertyValueTests.testBoolean_ReferenceIn(new Boolean(1));
            verify(outVar.isNull, false, "testBoolean_ReferenceIn(new Boolean(1)).isNull");
            verify(outVar.isValidType, true, "testBoolean_ReferenceIn(new Boolean(1)).isValidType");
            verify(outVar.outValue, true, "testBoolean_ReferenceIn(new Boolean(1)).outValue");
        }
    });

    runner.addTest({
        id: 2,
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
        id: 3,
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
        id: 4,
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
        id: 5,
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
        id: 6,
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
        id: 7,
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
        id: 8,
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
        id: 9,
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
        id: 10,
        desc: 'Reference_Out: Char16',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testChar16_ReferenceOut("S");
            verify(outVar, "S", 'testChar16_ReferenceOut("S")');
        }
    });

    runner.addTest({
        id: 11,
        desc: 'Reference_Out: UInt8',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testUInt8_ReferenceOut(2);
            verify(outVar, 2, 'testUInt8_ReferenceOut(2)');
        }
    });

    runner.addTest({
        id: 12,
        desc: 'Reference_Out: Double',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testDouble_ReferenceOut(2);
            verify(outVar, 2, 'testDouble_ReferenceOut(2)');
        }
    });

    runner.addTest({
        id: 13,
        desc: 'Reference_Out: Guid',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testGuid_ReferenceOut("{10C27311-B58F-476C-BFF4-6689B8A19836}");
            verify(outVar, "10c27311-b58f-476c-bff4-6689b8a19836", 'testGuid_ReferenceOut("{10C27311-B58F-476C-BFF4-6689B8A19836}")');
        }
    });

    runner.addTest({
        id: 14,
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
        id: 15,
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
        id: 16,
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
        id: 17,
        desc: 'Reference_Out: Enum',
        pri: '0',
        test: function () {
            var propertyValueTests = new Animals.PropertyValueTests();
            var outVar = propertyValueTests.testEnum_ReferenceOut(Animals.Phylum.cycliophora);
            verify(outVar, Animals.Phylum.cycliophora, 'testEnum_ReferenceOut(Animals.Phylum.cycliophora)');
        }
    });

    Loader42_FileName = 'RecyclerStress - IReference.js';
})();Run() 
