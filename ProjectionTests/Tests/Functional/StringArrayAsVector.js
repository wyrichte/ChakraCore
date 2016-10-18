if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {

    var myVectorMembers = [
        ['append', 'function', 1],
        ['clear', 'function', 0],
        ['first', 'function', 0],
        ['getAt', 'function', 1],
        ['getMany', 'function', 2],
        ['getView', 'function', 0],
        ['indexOf', 'function', 1],
        ['insertAt', 'function', 2],
        ['removeAt', 'function', 1],
        ['removeAtEnd', 'function', 0],
        ['replaceAll', 'function', 1],
        ['setAt', 'function', 2],
        ['size', 'number'],
    ];

    var myIteratorMembers = [
        ['current', 'string'],
        ['getMany', 'function', 1],
        ['hasCurrent', 'boolean'],
        ['moveNext', 'function', 0],
        ['toString', 'function', 0],
    ];

    var myVectorViewMembers = [
        ['first', 'function', 0],
        ['getAt', 'function', 1],
        ['getMany', 'function', 2],
        ['indexOf', 'function', 1],
        ['size', 'number'],
    ];

    var hresults = Winery.WinRTErrorTests.RestrictedErrorAccess.errorCodes;

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

    function verifyIteratorRangeWithGetMany(iteratorString, iterator, expected, from, to) {
        if (from === undefined) {
            from = 0;
        }

        if (to === undefined) {
            to = expected.length;
        }

        var manyItems = new Array(3);
        if (to - from < 3) {
            manyItems.length = to - from;
        }

        var gotManyItems;
        var index = from;
        for (; index < to && index < expected.length; ) {
            gotManyItems = iterator.getMany(manyItems);
            logger.comment("gotManyItems: " + gotManyItems);
            if (((expected.length - index) / manyItems.length) >= 1) {
                verify(gotManyItems, manyItems.length, "gotManyItems");
            } else {
                verify(gotManyItems, expected.length - index, "gotManyItems");
            }

            for (var i = 0; i < gotManyItems && index < to && index < expected.length; i++, index++) {
                verify(manyItems[i], expected[index], "manyItems[" + i + "]");
            }
        }

        verify(iterator.hasCurrent, (index < expected.length - 1), iteratorString + '.hasCurrent');
        verify(index - from, to - from, 'number of members iterated over');
    }

    function verifyIteratorRange(iteratorString, iterator, expected, from, to) {
        verifyMembers(iteratorString, iterator, myIteratorMembers);

        var iIndex = from;
        for (; iIndex < to; iIndex++) {
            assert(iterator.hasCurrent, iteratorString + '.hasCurrent');
            verify(iterator.current, expected[iIndex], iteratorString + '.current');
            var hasNext = iterator.moveNext();
            verify(hasNext, (iIndex < expected.length - 1), iteratorString + '.moveNext()');
        }

        verify(iterator.hasCurrent, (iIndex < expected.length - 1), iteratorString + '.hasCurrent');
        verify(iIndex - from, to - from, 'number of members iterated over');
    }

    function verifyVectorMemberAt(vectorString, vector, underlyingArrayString, underlyingArray, expected, iIndex) {
        verify(underlyingArray.length, expected.length, underlyingArrayString + ".length");
        verify(vector.size, expected.length, vectorString + ".size");
        verify(underlyingArray[iIndex], expected[iIndex], underlyingArrayString + '[' + iIndex + ']');
        verify(vector.getAt(iIndex), expected[iIndex], vectorString + '.getAt(' + iIndex + ')');
    }

    function verifyVectorMembers(vectorString, vector, expected, isView) {
        if (isView === true) {
            verifyMembers(vectorString, vector, myVectorViewMembers, expected);
        }
        else {
            verifyMembers(vectorString, vector, myVectorMembers, expected);
        }

        verify(vector.size, expected.length, vectorString + ".size");
    }

    function verifyVectorRange(vectorString, vector, iteratorString, iterator, iteratorForManyString, iteratorForMany, expected, from, to, isView) {
        verifyVectorMembers(vectorString, vector, expected, isView);
        if (expected.length !== 0) {
            verifyIteratorRange(iteratorString, iterator, expected, from, to);
            verifyIteratorRangeWithGetMany(iteratorForManyString, iteratorForMany, expected, from, to);
        }
    }

    function verifyVector(vectorString, vector, expected, isView) {
        logger.comment("var myIterator = " + vectorString + ".first()");
        var myIterator = vector.first();
        var myIteratorForMany = vector.first();
        verifyVectorRange(vectorString, vector, "myIterator", myIterator, "myIteratorForMany", myIteratorForMany, expected, 0, expected.length, isView);
    }

    runner.addTest({
        id: 1,
        desc: 'MarshalInArrayAsVector',
        pri: '0',
        test: function () {
            logger.comment('var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];');
            var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];
            logger.comment("var myVector = (new Animals.Animal(1)).duplicateStringVector(myArray);");
            var myVector = (new Animals.Animal(1)).duplicateStringVector(myArray);
            logger.comment('verifyVector("myVector", myVector, ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"])');
            verifyVector("myVector", myVector, ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"]);

            var foundException = false;
            try {
                var result = myVector.getAt(11);
            }
            catch (e) {
                verify(e.number, hresults.bounds, 'e.number while myVector.getAt(11)');
                foundException = true;
            }
            assert(foundException, 'Expected exception was caught');

            logger.comment('myVector::IndexOf');
            var result = myVector.indexOf("Yellow");
            verify(result.index, 1, 'index of myVector.indexOf("Yellow")');
            verify(result.returnValue, true, 'returnValue of myVector.indexOf("Yellow")');
            result = myVector.indexOf("Purple");
            verify(result.returnValue, false, 'returnValue of myVector.indexOf("Purple")');
        }
    });

    runner.addTest({
        id: 2,
        desc: 'CheckIVectorLifeTime',
        pri: '0',
        test: function () {
            logger.comment('var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];');
            var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];
            logger.comment("var myVector = (new Animals.Animal(1)).duplicateStringVector(myArray);");
            var myVector = (new Animals.Animal(1)).duplicateStringVector(myArray);
            logger.comment("myArray = undefined");
            myArray = undefined;
            logger.comment('verifyVector("myVector", myVector, ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"])');
            verifyVector("myVector", myVector, ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"]);

            var foundException = false;
            try {
                var result = myVector.getAt(11);
            }
            catch (e) {
                verify(e.number, hresults.bounds, 'e.number while myVector.getAt(11)');
                foundException = true;
            }
            assert(foundException, 'Expected exception was caught');

            logger.comment('myVector::IndexOf');
            var result = myVector.indexOf("Yellow");
            verify(result.index, 1, 'index of myVector.indexOf("Yellow")');
            verify(result.returnValue, true, 'returnValue of myVector.indexOf("Yellow")');
            result = myVector.indexOf("Purple");
            verify(result.returnValue, false, 'returnValue of myVector.indexOf("Purple")');
        }
    });


    runner.addTest({
        id: 3,
        desc: 'AlterArrayBeforeViewing',
        pri: '0',
        test: function () {
            logger.comment('var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];');
            var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];
            logger.comment("var myVector = (new Animals.Animal(1)).duplicateStringVector(myArray);");
            var myVector = (new Animals.Animal(1)).duplicateStringVector(myArray);
            verifyMembers("myVector", myVector, myVectorMembers, ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"]);
            logger.comment('myArray[11] = "Purple"');
            myArray[11] = "Purple";
            logger.comment('verifyVector("myVector", myVector, ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange", "undefined", "undefined", "Purple"])');
            verifyVector("myVector", myVector, ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange", "undefined", "undefined", "Purple"]);

            var foundException = false;
            try {
                var result = myVector.getAt(15);
            }
            catch (e) {
                verify(e.number, hresults.bounds, 'e.number while myVector.getAt(15)');
                foundException = true;
            }
            assert(foundException, 'Expected exception was caught');

            logger.comment('myVector::IndexOf');
            var result = myVector.indexOf("Yellow");
            verify(result.returnValue, true, 'returnValue of myVector.indexOf("Yellow")');
            verify(result.index, 1, 'index of myVector.indexOf("Yellow")');
            result = myVector.indexOf("Purple");
            verify(result.returnValue, true, 'returnValue of myVector.indexOf("Purple")');
            verify(result.index, 11, 'index of myVector.indexOf(30)');
            result = myVector.indexOf("White");
            verify(result.returnValue, false, 'returnValue of myVector.indexOf("White")');
        }
    });

    runner.addTest({
        id: 4,
        desc: 'AlterArrayWhileViewing',
        pri: '0',
        test: function () {
            logger.comment('var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];');
            var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];
            logger.comment("var myVector = (new Animals.Animal(1)).duplicateStringVector(myArray);");
            var myVector = (new Animals.Animal(1)).duplicateStringVector(myArray);
            verifyMembers("myVector", myVector, myVectorMembers, ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"]);
            logger.comment('var myArrayToVerify = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];');
            var myArrayToVerify = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];
            logger.comment('var myIterator = myVector.first();');
            var myIterator = myVector.first();
            var myIteratorMany = myVector.first();
            logger.comment('verifyVectorRange("myVector", myVector, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 0, 2);');
            verifyVectorRange("myVector", myVector, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 0, 2);

            logger.comment('myArray[3] = "White";');
            myArray[3] = "White";
            logger.comment('myArrayToVerify[3] = "White";');
            myArrayToVerify[3] = "White";
            logger.comment('verifyVectorRange("myVector", myVector, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 2, 5);');
            verifyVectorRange("myVector", myVector, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 2, 5);

            logger.comment('myArray[5] = "Teal";');
            myArray[5] = "Teal";
            logger.comment('myArrayToVerify[5] = "Teal";');
            myArrayToVerify[5] = "Teal";
            logger.comment('verifyVectorRange("myVector", myVector, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 5, 9);');
            verifyVectorRange("myVector", myVector, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 5, 9);
        }
    });

    runner.addTest({
        id: 5,
        desc: 'AlterArrayLengthWhileViewing',
        pri: '0',
        test: function () {
            logger.comment('var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];');
            var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];
            logger.comment("var myVector = (new Animals.Animal(1)).duplicateStringVector(myArray);");
            var myVector = (new Animals.Animal(1)).duplicateStringVector(myArray);
            verifyMembers("myVector", myVector, myVectorMembers, ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"]);
            logger.comment('var myArrayToVerify = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];');
            var myArrayToVerify = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];
            logger.comment('var myIterator = myVector.first();');
            var myIterator = myVector.first();
            var myIteratorMany = myVector.first();
            logger.comment('verifyVectorRange("myVector", myVector, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 0, 2);');
            verifyVectorRange("myVector", myVector, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 0, 2);

            logger.comment('myArray.length = 7');
            myArray.length = 7;
            logger.comment('myArrayToVerify.length = 7');
            myArrayToVerify.length = 7;
            logger.comment('verifyVectorRange("myVector", myVector, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 2, 7);');
            verifyVectorRange("myVector", myVector, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 2, 7);

            logger.comment('myArray.length = 10;');
            myArray.length = 10;
            logger.comment('myArrayToVerify = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "undefined", "undefined", "undefined"];');
            myArrayToVerify = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "undefined", "undefined", "undefined"];
            logger.comment('verifyVectorRange("myVector", myVector, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 7, 10);');
            verifyVectorRange("myVector", myVector, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 7, 10);

            logger.comment('myArray.length = 13');
            myArray.length = 13;
            logger.comment('verify(myIterator.hasCurrent, true, "myIterator.hasCurrent");');
            verify(myIterator.hasCurrent, true, "myIterator.hasCurrent");
            logger.comment('myArrayToVerify = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "undefined", "undefined", "undefined", "undefined", "undefined", "undefined"];');
            myArrayToVerify = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "undefined", "undefined", "undefined", "undefined", "undefined", "undefined"];
            logger.comment('verifyVectorMembers("myVector", myVector, myArrayToVerify);');
            verifyVectorMembers("myVector", myVector, myArrayToVerify);

            logger.comment('myArray.length = 5');
            myArray.length = 5;
            logger.comment('verify(myIterator.hasCurrent, false, "myIterator.hasCurrent")');
            logger.comment('myArrayToVerify.length = 5;');
            myArrayToVerify.length = 5;
            logger.comment('verifyVectorMembers("myVector", myVector, myArrayToVerify);');
            verifyVectorMembers("myVector", myVector, myArrayToVerify);
        }
    });

    runner.addTest({
        id: 6,
        desc: 'VectorSetAt',
        pri: '0',
        test: function () {
            logger.comment('var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];');
            var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];
            logger.comment("var myVector = (new Animals.Animal(1)).duplicateStringVector(myArray);");
            var myVector = (new Animals.Animal(1)).duplicateStringVector(myArray);
            logger.comment('var myArrayToVerify = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];');
            var myArrayToVerify = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];
            verifyVectorMemberAt("myVector", myVector, "myArray", myArray, myArrayToVerify, 2);
            verifyVectorMemberAt("myVector", myVector, "myArray", myArray, myArrayToVerify, 5);
            logger.comment('myVector.setAt(5) with myVector.getAt(2));');
            myVector.setAt(5, myVector.getAt(2));
            logger.comment('myArrayToVerify[5] = myArrayToVerify[2];');
            myArrayToVerify[5] = myArrayToVerify[2];
            verifyVectorMemberAt("myVector", myVector, "myArray", myArray, myArrayToVerify, 2);
            verifyVectorMemberAt("myVector", myVector, "myArray", myArray, myArrayToVerify, 5);
            verify(myVector.getAt(5), myVector.getAt(2), 'myVector.getAt(5)');

            var foundException = false;
            try {
                var result = myVector.setAt(15, "White");
            }
            catch (e) {
                verify(e.number, hresults.bounds, 'e.number while myVector.setAt(15, "White")');
                foundException = true;
            }
            assert(foundException, 'Expected exception was caught');
        }
    });

    runner.addTest({
        id: 7,
        desc: 'VectorRemoveAt',
        pri: '0',
        test: function () {
            logger.comment('var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];');
            var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];
            logger.comment("var myVector = (new Animals.Animal(1)).duplicateStringVector(myArray);");
            var myVector = (new Animals.Animal(1)).duplicateStringVector(myArray);
            logger.comment('myVector.removeAt(6)');
            myVector.removeAt(6);
            logger.comment('verifyVector("myVector", myVector, ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Brown", "Orange"])');
            verifyVector("myVector", myVector, ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Brown", "Orange"]);
            logger.comment('verifyVector("myVector", myVector, myArray);');
            verifyVector("myVector", myVector, myArray);

            var foundException = false;
            try {
                var result = myVector.removeAt(15);
            }
            catch (e) {
                verify(e.number, hresults.bounds, 'e.number while myVector.removeAt(15)');
                foundException = true;
            }
            assert(foundException, 'Expected exception was caught');
        }
    });

    runner.addTest({
        id: 8,
        desc: 'VectorInsertAt',
        pri: '0',
        test: function () {
            logger.comment('var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];');
            var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];
            logger.comment("var myVector = (new Animals.Animal(1)).duplicateStringVector(myArray);");
            var myVector = (new Animals.Animal(1)).duplicateStringVector(myArray);
            logger.comment('myVector.insertAt(1, "Purple")');
            myVector.insertAt(1, "Purple");
            logger.comment('verifyVector("myVector", myVector, ["Pink", "Purple", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"])');
            verifyVector("myVector", myVector, ["Pink", "Purple", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"]);
            logger.comment('verifyVector("myVector", myVector, myArray);');
            verifyVector("myVector", myVector, myArray);

            var foundException = false;
            try {
                var result = myVector.insertAt(15, "White");
            }
            catch (e) {
                verify(e.number, hresults.bounds, 'e.number while myVector.insertAt(15, "White")');
                foundException = true;
            }
            assert(foundException, 'Expected exception was caught');
        }
    });

    runner.addTest({
        id: 9,
        desc: 'VectorRemoveAtEnd',
        pri: '0',
        test: function () {
            logger.comment('var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];');
            var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];
            logger.comment("var myVector = (new Animals.Animal(1)).duplicateStringVector(myArray);");
            var myVector = (new Animals.Animal(1)).duplicateStringVector(myArray);
            logger.comment('myVector.removeAtEnd()');
            myVector.removeAtEnd();
            logger.comment('verifyVector("myVector", myVector, ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown"])');
            verifyVector("myVector", myVector, ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown"]);
            logger.comment('verifyVector("myVector", myVector, myArray);');
            verifyVector("myVector", myVector, myArray);
        }
    });

    runner.addTest({
        id: 10,
        desc: 'VectorClear',
        pri: '0',
        test: function () {
            logger.comment('var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];');
            var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];
            logger.comment("var myVector = (new Animals.Animal(1)).duplicateStringVector(myArray);");
            var myVector = (new Animals.Animal(1)).duplicateStringVector(myArray);
            logger.comment('myVector.clear()');
            myVector.clear();
            logger.comment('verifyVector("myVector", myVector, new Array())');
            verifyVector("myVector", myVector, new Array());
            logger.comment('verifyVector("myVector", myVector, myArray);');
            verifyVector("myVector", myVector, myArray);
        }
    });

    runner.addTest({
        id: 11,
        desc: 'VectorAppend',
        pri: '0',
        test: function () {
            logger.comment('var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];');
            var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];
            logger.comment("var myVector = (new Animals.Animal(1)).duplicateStringVector(myArray);");
            var myVector = (new Animals.Animal(1)).duplicateStringVector(myArray);
            logger.comment('myVector.append("Purple");');
            myVector.append("Purple");
            logger.comment('myVector.append("White");');
            myVector.append("White");
            logger.comment('myVector.append("Teal");');
            myVector.append("Teal");
            logger.comment('myVector.append("Silver");');
            myVector.append("Silver");
            logger.comment('verifyVector("myVector", myVector, ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange", "Purple", "White", "Teal", "Silver"])');
            verifyVector("myVector", myVector, ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange", "Purple", "White", "Teal", "Silver"]);
            logger.comment('verifyVector("myVector", myVector, myArray);');
            verifyVector("myVector", myVector, myArray);
        }
    });

    runner.addTest({
        id: 12,
        desc: 'VectorGetView',
        pri: '0',
        test: function () {
            logger.comment('var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];');
            var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];
            logger.comment("var myVector = (new Animals.Animal(1)).duplicateStringVector(myArray);");
            var myVector = (new Animals.Animal(1)).duplicateStringVector(myArray);
            logger.comment('var myVectorView = myVector.getView();');
            var myVectorView = myVector.getView();
            var myArrayToVerify = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];
            verifyVector("myVectorView", myVectorView, myArrayToVerify, true);

            logger.comment('myVectorView::IndexOf');
            var result = myVectorView.indexOf("Yellow");
            verify(result.index, 1, 'index of myVectorView.indexOf("Yellow")');
            verify(result.returnValue, true, 'returnValue of myVectorView.indexOf("Yellow")');
            result = myVectorView.indexOf("White");
            verify(result.returnValue, false, 'returnValue of myVectorView.indexOf("White")');

            logger.comment("myVectorView::getMany");
            var manyItems = new Array(3);
            var gotManyItems = myVectorView.getMany(5, manyItems);
            verify(gotManyItems, 3, "gotManyItems");
            for (var i = 0; i < gotManyItems; i++) {
                verify(manyItems[i], myArrayToVerify[i + 5], "manyItems[" + i + "]");
            }

            logger.comment('myVector.removeAtEnd()');
            myVector.removeAtEnd();
            logger.comment('verifyVector("myVector", myVector, ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown"])');
            verifyVector("myVector", myVector, ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown"]);
            logger.comment('verifyVector("myVectorView", myVectorView, ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown"])');
            verifyVector("myVectorView", myVectorView, ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown"], true);
        }
    });

    runner.addTest({
        id: 13,
        desc: 'ProjectAlreadyMarshaledJsArrayAsVector',
        pri: '0',
        test: function () {
            logger.comment('var myArray = ["Pink", "Yellow", "Blue", "Red", "Green", "Brown", "Tan", "Black", "White"];');
            var myArray = ["Pink", "Yellow", "Blue", "Red", "Green", "Brown", "Tan", "Black", "White"];
            logger.comment("var myVector = (new Animals.Animal(1)).sendBackSameStringVector(myArray);");
            var myVector = (new Animals.Animal(1)).sendBackSameStringVector(myArray);

            logger.comment('verifyVector("myVector", myVector, myArray)');
            verifyVector("myVector", myVector, myArray);
            assert(!Array.isArray(myVector), "!Array.isArray(myVector)");
            assert(myArray !== myVector, "myArray !== myVector");

            logger.comment('myArray[10] = "Purple";');
            myArray[5] = "Purple";

            logger.comment('verifyVector("myVector", myVector, myArray)');
            verifyVector("myVector", myVector, myArray);
            assert(!Array.isArray(myVector), "!Array.isArray(myVector)");
            assert(myArray !== myVector, "myArray !== myVector");
        }
    });

    runner.addTest({
        id: 14,
        desc: 'VectorGetMany',
        pri: '0',
        test: function () {
            var myArray = ["Pink", "Yellow", "Blue", "Red", "Green", "Brown", "Tan", "Black", "White"];
            var myVector = (new Animals.Animal(1)).duplicateStringVector(myArray);
            logger.comment("fill the manyItems by calling getMany");
            var manyItems = new Array(5);
            var gotManyItems = myVector.getMany(2, manyItems);
            verify(gotManyItems, 5, "gotManyItems");
            for (var i = 0; i < gotManyItems; i++) {
                verify(manyItems[i], myArray[i + 2], "manyItems[" + i + "]");
            }
        }
    });

    runner.addTest({
        id: 15,
        desc: 'VectorReplaceAll',
        pri: '0',
        test: function () {
            var myArray = ["Pink", "Yellow", "Blue", "Red", "Green", "Brown", "Tan", "Black", "White"];
            var myVector = (new Animals.Animal(1)).duplicateStringVector(myArray);
            logger.comment("replace the vector by new array");
            var manyItems = ["Crimson", "Turquoise", "Raspberry pink", "Lavender blue"];
            myVector.replaceAll(manyItems);
            verifyMembers("myVector", myVector, myVectorMembers, manyItems);
        }
    });

    Loader42_FileName = 'String array as Vector tests';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
