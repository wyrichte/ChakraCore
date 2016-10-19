if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {

    var myIteratorMembers = [
        ['current', 'number'],
        ['getMany', 'function', 1],
        ['hasCurrent', 'boolean'],
        ['moveNext', 'function', 0],
        ['toString', 'function', 0],
    ];

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
        var gotManyItems;
        var index = from;
        for (; index < to; ) {
            gotManyItems = iterator.getMany(manyItems);
            logger.comment("gotManyItems: " + gotManyItems);
            if (((expected.length - index) / manyItems.length) >= 1) {
                verify(gotManyItems, manyItems.length, "gotManyItems");
            } else {
                verify(gotManyItems, expected.length - index, "gotManyItems");
            }

            for (var i = 0; i < gotManyItems && index < to; i++) {
                verify(manyItems[i], expected[index], "manyItems[" + i + "]");
                index++;
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

    function verifyIterator(iteratorString, iterator, expected) {
        verifyIteratorRange(iteratorString, iterator, expected, 0, expected.length);
    }

    runner.addTest({
        id: 1,
        desc: 'MarshalInArrayAsIterator',
        pri: '0',
        test: function () {
            logger.comment("var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];");
            var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            var myAnimal = new Animals.Animal(1);
            logger.comment("var myIterator = (new Animals.Animal(1)).duplicateIterator(myArray);");
            var myIterator = myAnimal.duplicateIterator(myArray);
            logger.comment('verifyIterator("myIterator", myIterator, [1, 2, 3, 4, 5, 6, 7, 8, 9])');
            verifyIterator("myIterator", myIterator, [1, 2, 3, 4, 5, 6, 7, 8, 9]);

            logger.comment("Iterator with getMany");
            var myIterator = myAnimal.duplicateIterator(myArray);
            verifyIteratorRangeWithGetMany("myIterator", myIterator, [1, 2, 3, 4, 5, 6, 7, 8, 9]);
        }
    });

    runner.addTest({
        id: 2,
        desc: 'CheckIIteratorLifeTime',
        pri: '0',
        test: function () {
            logger.comment("var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];");
            var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            logger.comment("var myIterator = (new Animals.Animal(1)).duplicateIterator(myArray);");
            var myIterator = (new Animals.Animal(1)).duplicateIterator(myArray);
            logger.comment("myArray = undefined");
            myArray = undefined;
            logger.comment('verifyIterator("myIterator", myIterator, [1, 2, 3, 4, 5, 6, 7, 8, 9])');
            verifyIterator("myIterator", myIterator, [1, 2, 3, 4, 5, 6, 7, 8, 9]);
        }
    });


    runner.addTest({
        id: 3,
        desc: 'AlterArrayBeforeIterating',
        pri: '0',
        test: function () {
            logger.comment("var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];");
            var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            logger.comment("var myIterator = (new Animals.Animal(1)).duplicateIterator(myArray);");
            var myIterator = (new Animals.Animal(1)).duplicateIterator(myArray);
            logger.comment('myArray[11] = 30;');
            myArray[11] = 30;
            logger.comment('verifyIterator("myIterator", myIterator, [1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 30]);');
            verifyIterator("myIterator", myIterator, [1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 30]);
        }
    });

    runner.addTest({
        id: 4,
        desc: 'AlterArrayWhileIterating',
        pri: '0',
        test: function () {
            logger.comment("var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];");
            var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            logger.comment("var myIterator = (new Animals.Animal(1)).duplicateIterator(myArray);");
            var myIterator = (new Animals.Animal(1)).duplicateIterator(myArray);
            logger.comment('var myArrayToVerify = [1, 2, 3, 4, 5, 6, 7, 8, 9];');
            var myArrayToVerify = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            logger.comment('verifyIteratorRange("myIterator", myIterator, myArrayToVerify, 0, 2);');
            verifyIteratorRange("myIterator", myIterator, myArrayToVerify, 0, 2);

            logger.comment('myArray[3] = 30;');
            myArray[3] = 30;
            logger.comment('myArrayToVerify[3] = 30;');
            myArrayToVerify[3] = 30;
            logger.comment('verifyIteratorRange("myIterator", myIterator, myArrayToVerify, 2, 5);');
            verifyIteratorRange("myIterator", myIterator, myArrayToVerify, 2, 5);

            logger.comment('myArray[5] = 50;');
            myArray[5] = 50;
            logger.comment('myArrayToVerify[5] = 50;');
            myArrayToVerify[5] = 50;
            logger.comment('verifyIteratorRange("myIterator", myIterator, myArrayToVerify, 5, 9);');
            verifyIteratorRange("myIterator", myIterator, myArrayToVerify, 5, 9);
        }
    });

    runner.addTest({
        id: 5,
        desc: 'AlterArrayLengthWhileIterating',
        pri: '0',
        test: function () {
            logger.comment("var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];");
            var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            logger.comment("var myIterator = (new Animals.Animal(1)).duplicateIterator(myArray);");
            var myIterator = (new Animals.Animal(1)).duplicateIterator(myArray);
            logger.comment('var myArrayToVerify = [1, 2, 3, 4, 5, 6, 7, 8, 9];');
            var myArrayToVerify = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            logger.comment('verifyIteratorRange("myIterator", myIterator, myArrayToVerify, 0, 2);');
            verifyIteratorRange("myIterator", myIterator, myArrayToVerify, 0, 2);

            logger.comment('myArray.length = 7');
            myArray.length = 7;
            logger.comment('myArrayToVerify.length = 7');
            myArrayToVerify.length = 7;
            logger.comment('verifyIteratorRange("myIterator", myIterator, myArrayToVerify, 2, 7)');
            verifyIteratorRange("myIterator", myIterator, myArrayToVerify, 2, 7);

            logger.comment('myArray.length = 10;');
            myArray.length = 10;
            logger.comment('myArrayToVerify = [1, 2, 3, 4, 5, 6, 7, 0, 0, 0];');
            myArrayToVerify = [1, 2, 3, 4, 5, 6, 7, 0, 0, 0];
            logger.comment('verifyIteratorRange("myIterator", myIterator, myArrayToVerify, 7, 10);');
            verifyIteratorRange("myIterator", myIterator, myArrayToVerify, 7, 10);

            logger.comment('myArray.length = 13');
            myArray.length = 13;
            logger.comment('verify(myIterator.hasCurrent, true, "myIterator.hasCurrent");');
            verify(myIterator.hasCurrent, true, "myIterator.hasCurrent");

            logger.comment('myArray.length = 5');
            myArray.length = 5;
            logger.comment('verify(myIterator.hasCurrent, false, "myIterator.hasCurrent")');
            verify(myIterator.hasCurrent, false, "myIterator.hasCurrent");
        }
    });

    runner.addTest({
        id: 6,
        desc: 'ProjectAlreadyMarshaledJsArrayAsIterator',
        pri: '0',
        test: function () {
            logger.comment("var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];");
            var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            logger.comment("var myIterator = (new Animals.Animal(1)).sendBackSameIterator(myArray);");
            var myIterator = (new Animals.Animal(1)).sendBackSameIterator(myArray);

            logger.comment('verifyIteratorRange("myIterator", myIterator, myArray, 0, 4);');
            verifyIteratorRange("myIterator", myIterator, myArray, 0, 4);
            assert(!Array.isArray(myIterator), "!Array.isArray(myIterator)");
            assert(myArray !== myIterator, "myArray !== myIterator");

            logger.comment('myArray[5] = 10;');
            myArray[5] = 10;

            logger.comment('verifyIteratorRange("myIterator", myIterator, myArray, 4, 9);');
            verifyIteratorRange("myIterator", myIterator, myArray, 4, 9);
            assert(!Array.isArray(myIterator), "!Array.isArray(myIterator)");
            assert(myArray !== myIterator, "myArray !== myIterator");
        }
    });

    Loader42_FileName = 'Int array as Iterator tests';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
