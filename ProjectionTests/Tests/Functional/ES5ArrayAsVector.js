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
        ['current', 'number'],
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

    function GetArray() {
        logger.comment("var myArray = [1, 2, 3, 4, 5, 6, 7, 8];");
        var myArray = [1, 2, 3, 4, 5, 6, 7, 8];

        logger.comment('Object.defineProperty(myArray, "8", { set: function (x) { value = x; }, get: function () { return value }, configurable: true });');
        Object.defineProperty(myArray, "8", { set: function (x) { value = x; }, get: function () { return value }, configurable: true });

        logger.comment('myArray[8] = 9;');
        myArray[8] = 9;

        return myArray;
    }

    runner.addTest({
        id: 1,
        desc: 'MarshalInArrayAsVector',
        pri: '0',
        test: function () {
            var myArray = GetArray();
            logger.comment("var myVector = (new Animals.Animal(1)).duplicateVector(myArray);");
            var myVector = (new Animals.Animal(1)).duplicateVector(myArray);
            logger.comment('verifyVector("myVector", myVector, [1, 2, 3, 4, 5, 6, 7, 8, 9])');
            verifyVector("myVector", myVector, [1, 2, 3, 4, 5, 6, 7, 8, 9]);

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
            var result = myVector.indexOf(2);
            verify(result.index, 1, 'index of myVector.indexOf(2)');
            verify(result.returnValue, true, 'returnValue of myVector.indexOf(2)');
            result = myVector.indexOf(11);
            verify(result.returnValue, false, 'returnValue of myVector.indexOf(11)');
        }
    });

    runner.addTest({
        id: 2,
        desc: 'CheckIVectorLifeTime',
        pri: '0',
        test: function () {
            var myArray = GetArray();
            logger.comment("var myVector = (new Animals.Animal(1)).duplicateVector(myArray);");
            var myVector = (new Animals.Animal(1)).duplicateVector(myArray);
            logger.comment("myArray = undefined");
            myArray = undefined;
            logger.comment('verifyVector("myVector", myVector, [1, 2, 3, 4, 5, 6, 7, 8, 9])');
            verifyVector("myVector", myVector, [1, 2, 3, 4, 5, 6, 7, 8, 9]);

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
            var result = myVector.indexOf(2);
            verify(result.index, 1, 'index of myVector.indexOf(2)');
            verify(result.returnValue, true, 'returnValue of myVector.indexOf(2)');
            result = myVector.indexOf(11);
            verify(result.returnValue, false, 'returnValue of myVector.indexOf(11)');
        }
    });


    runner.addTest({
        id: 3,
        desc: 'AlterArrayBeforeViewing',
        pri: '0',
        preReq: function () {
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            var myArray = GetArray();
            logger.comment("var myVector = (new Animals.Animal(1)).duplicateVector(myArray);");
            var myVector = (new Animals.Animal(1)).duplicateVector(myArray);
            verifyMembers("myVector", myVector, myVectorMembers, [1, 2, 3, 4, 5, 6, 7, 8, 9]);
            logger.comment("myArray[11] = 30");
            myArray[11] = 30;
            logger.comment('verifyVector("myVector", myVector, [1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 30])');
            verifyVector("myVector", myVector, [1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 30]);

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
            var result = myVector.indexOf(2);
            verify(result.returnValue, true, 'returnValue of myVector.indexOf(2)');
            verify(result.index, 1, 'index of myVector.indexOf(2)');
            result = myVector.indexOf(30);
            verify(result.returnValue, true, 'returnValue of myVector.indexOf(30)');
            verify(result.index, 11, 'index of myVector.indexOf(30)');
            result = myVector.indexOf(11);
            verify(result.returnValue, false, 'returnValue of myVector.indexOf(11)');
        }
    });

    runner.addTest({
        id: 4,
        desc: 'AlterArrayWhileViewing',
        pri: '0',
        test: function () {
            var myArray = GetArray();
            logger.comment("var myVector = (new Animals.Animal(1)).duplicateVector(myArray);");
            var myVector = (new Animals.Animal(1)).duplicateVector(myArray);
            verifyMembers("myVector", myVector, myVectorMembers, [1, 2, 3, 4, 5, 6, 7, 8, 9]);
            logger.comment('var myArrayToVerify = [1, 2, 3, 4, 5, 6, 7, 8, 9];');
            var myArrayToVerify = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            logger.comment('var myIterator = myVector.first();');
            var myIterator = myVector.first();
            var myIteratorMany = myVector.first();
            logger.comment('verifyVectorRange("myVector", myVector, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 0, 2);');
            verifyVectorRange("myVector", myVector, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 0, 2);

            if (typeof Animals._CLROnly === 'undefined') //Not applicable to C# ABI due to CLR's copying of contents instead of native's pointer usage
            {
                logger.comment('myArray[3] = 30;');
                myArray[3] = 30;
                logger.comment('myArrayToVerify[3] = 30;');
                myArrayToVerify[3] = 30;
                logger.comment('verifyVectorRange("myVector", myVector, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 2, 5);');
                verifyVectorRange("myVector", myVector, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 2, 5);

                logger.comment('myArray[5] = 50;');
                myArray[5] = 50;
                logger.comment('myArrayToVerify[5] = 50;');
                myArrayToVerify[5] = 50;
                logger.comment('verifyVectorRange("myVector", myVector, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 5, 9);');
                verifyVectorRange("myVector", myVector, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 5, 9);
            }
        }
    });

    runner.addTest({
        id: 5,
        desc: 'AlterArrayLengthWhileViewing',
        pri: '0',
        preReq: function () {
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            var myArray = GetArray();
            logger.comment("var myVector = (new Animals.Animal(1)).duplicateVector(myArray);");
            var myVector = (new Animals.Animal(1)).duplicateVector(myArray);
            verifyMembers("myVector", myVector, myVectorMembers, [1, 2, 3, 4, 5, 6, 7, 8, 9]);
            logger.comment('var myArrayToVerify = [1, 2, 3, 4, 5, 6, 7, 8, 9];');
            var myArrayToVerify = [1, 2, 3, 4, 5, 6, 7, 8, 9];
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
            logger.comment('myArrayToVerify = [1, 2, 3, 4, 5, 6, 7, 0, 0, 0];');
            myArrayToVerify = [1, 2, 3, 4, 5, 6, 7, 0, 0, 0];
            logger.comment('verifyVectorRange("myVector", myVector, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 7, 10);');
            verifyVectorRange("myVector", myVector, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 7, 10);

            logger.comment('myArray.length = 13');
            myArray.length = 13;
            logger.comment('verify(myIterator.hasCurrent, true, "myIterator.hasCurrent");');
            verify(myIterator.hasCurrent, true, "myIterator.hasCurrent");
            logger.comment('myArrayToVerify = [1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0];');
            myArrayToVerify = [1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0];
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
        preReq: function () {
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            var myArray = GetArray();
            logger.comment("var myVector = (new Animals.Animal(1)).duplicateVector(myArray);");
            var myVector = (new Animals.Animal(1)).duplicateVector(myArray);
            logger.comment('var myArrayToVerify = [1, 2, 3, 4, 5, 6, 7, 8, 9];');
            var myArrayToVerify = [1, 2, 3, 4, 5, 6, 7, 8, 9];
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
                var result = myVector.setAt(15, 30);
            }
            catch (e) {
                verify(e.number, hresults.bounds, 'e.number while myVector.setAt(15, 30)');
                foundException = true;
            }
            assert(foundException, 'Expected exception was caught');
        }
    });

    runner.addTest({
        id: 7,
        desc: 'VectorRemoveAt',
        pri: '0',
        preReq: function () {
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            var myArray = GetArray();
            logger.comment("var myVector = (new Animals.Animal(1)).duplicateVector(myArray);");
            var myVector = (new Animals.Animal(1)).duplicateVector(myArray);
            logger.comment('myVector.removeAt(6)');
            myVector.removeAt(6);
            logger.comment('verifyVector("myVector", myVector, [1, 2, 3, 4, 5, 6, 8, 9])');
            verifyVector("myVector", myVector, [1, 2, 3, 4, 5, 6, 8, 9]);
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
        preReq: function () {
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            var myArray = GetArray();
            logger.comment("var myVector = (new Animals.Animal(1)).duplicateVector(myArray);");
            var myVector = (new Animals.Animal(1)).duplicateVector(myArray);
            logger.comment('myVector.insertAt(1, 1111)');
            myVector.insertAt(1, 1111);
            logger.comment('verifyVector("myVector", myVector, [1, 1111, 2, 3, 4, 5, 6, 7, 8, 9])');
            verifyVector("myVector", myVector, [1, 1111, 2, 3, 4, 5, 6, 7, 8, 9]);
            logger.comment('verifyVector("myVector", myVector, myArray);');
            verifyVector("myVector", myVector, myArray);

            var foundException = false;
            try {
                var result = myVector.insertAt(15, 13);
            }
            catch (e) {
                verify(e.number, hresults.bounds, 'e.number while myVector.insertAt(15, 13)');
                foundException = true;
            }
            assert(foundException, 'Expected exception was caught');
        }
    });

    runner.addTest({
        id: 9,
        desc: 'VectorRemoveAtEnd',
        pri: '0',
        preReq: function () {
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            var myArray = GetArray();
            logger.comment("var myVector = (new Animals.Animal(1)).duplicateVector(myArray);");
            var myVector = (new Animals.Animal(1)).duplicateVector(myArray);
            logger.comment('myVector.removeAtEnd()');
            myVector.removeAtEnd();
            logger.comment('verifyVector("myVector", myVector, [1, 2, 3, 4, 5, 6, 7, 8])');
            verifyVector("myVector", myVector, [1, 2, 3, 4, 5, 6, 7, 8]);
            logger.comment('verifyVector("myVector", myVector, myArray);');
            verifyVector("myVector", myVector, myArray);
        }
    });

    runner.addTest({
        id: 10,
        desc: 'VectorClear',
        pri: '0',
        preReq: function () {
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            var myArray = GetArray();
            logger.comment("var myVector = (new Animals.Animal(1)).duplicateVector(myArray);");
            var myVector = (new Animals.Animal(1)).duplicateVector(myArray);
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
        preReq: function () {
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            var myArray = GetArray();
            logger.comment("var myVector = (new Animals.Animal(1)).duplicateVector(myArray);");
            var myVector = (new Animals.Animal(1)).duplicateVector(myArray);
            for (var iIndex = 1; iIndex < 10; iIndex++) {
                logger.comment('myVector.append(' + (iIndex * 11) + ');');
                myVector.append(iIndex * 11);
            }
            logger.comment('verifyVector("myVector", myVector, [1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 22, 33, 44, 55, 66, 77, 88, 99])');
            verifyVector("myVector", myVector, [1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 22, 33, 44, 55, 66, 77, 88, 99]);
            logger.comment('verifyVector("myVector", myVector, myArray);');
            verifyVector("myVector", myVector, myArray);
        }
    });

    runner.addTest({
        id: 12,
        desc: 'VectorGetView',
        pri: '0',
        preReq: function () {
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            var myArray = GetArray();
            logger.comment("var myVector = (new Animals.Animal(1)).duplicateVector(myArray);");
            var myVector = (new Animals.Animal(1)).duplicateVector(myArray);
            logger.comment('var myVectorView = myVector.getView();');
            var myVectorView = myVector.getView();
            var myArrayToVerify = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            verifyVector("myVectorView", myVectorView, myArrayToVerify, true);

            logger.comment('myVectorView::IndexOf');
            var result = myVectorView.indexOf(2);
            verify(result.index, 1, 'index of myVectorView.indexOf(2)');
            verify(result.returnValue, true, 'returnValue of myVectorView.indexOf(2)');
            result = myVectorView.indexOf(11);
            verify(result.returnValue, false, 'returnValue of myVectorView.indexOf(11)');

            logger.comment("myVectorView::getMany");
            var manyItems = new Array(3);
            var gotManyItems = myVectorView.getMany(5, manyItems);
            verify(gotManyItems, 3, "gotManyItems");
            for (var i = 0; i < gotManyItems; i++) {
                verify(manyItems[i], myArrayToVerify[i + 5], "manyItems[" + i + "]");
            }

            logger.comment('myVector.removeAtEnd()');
            myVector.removeAtEnd();
            logger.comment('verifyVector("myVector", myVector, [1, 2, 3, 4, 5, 6, 7, 8])');
            verifyVector("myVector", myVector, [1, 2, 3, 4, 5, 6, 7, 8]);
            logger.comment('verifyVector("myVectorView", myVectorView, [1, 2, 3, 4, 5, 6, 7, 8])');
            verifyVector("myVectorView", myVectorView, [1, 2, 3, 4, 5, 6, 7, 8], true);
        }
    });


    runner.addTest({
        id: 13,
        desc: 'ProjectAlreadyMarshaledJsArrayAsVector',
        pri: '0',
        test: function () {
            var myArray = GetArray();
            logger.comment("var myVector = (new Animals.Animal(1)).sendBackSameVector(myArray);");
            var myVector = (new Animals.Animal(1)).sendBackSameVector(myArray);

            logger.comment('verifyVector("myVector", myVector, myArray)');
            verifyVector("myVector", myVector, myArray);
            assert(!Array.isArray(myVector), "!Array.isArray(myVector)");
            assert(myArray !== myVector, "myArray !== myVector");

            logger.comment('myArray[5] = 10;');
            myArray[5] = 10;

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
            var myArray = GetArray();
            var myVector = (new Animals.Animal(1)).duplicateVector(myArray);
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
            var myArray = GetArray();
            var myVector = (new Animals.Animal(1)).duplicateVector(myArray);
            logger.comment("replace the vector by new array");
            var manyItems = [50, 100, 150, 200];
            myVector.replaceAll(manyItems);
            verifyMembers("myVector", myVector, myVectorMembers, manyItems);
        }
    });

    Loader42_FileName = 'ES5 Int array as Vector tests';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
