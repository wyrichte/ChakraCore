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

    function verifyVectorMembers(vectorViewString, vectorView, expected, isView) {
        if (isView === false) {
            verifyMembers(vectorViewString, vectorView, myVectorMembers, expected);
        }
        else {
            verifyMembers(vectorViewString, vectorView, myVectorViewMembers, expected);
        }

        verify(vectorView.size, expected.length, vectorViewString + ".size");
    }

    function verifyVectorViewRange(vectorViewString, vectorView, iteratorString, iterator, iteratorForManyString, iteratorForMany, expected, from, to, isView) {
        verifyVectorMembers(vectorViewString, vectorView, expected, isView);
        if (expected.length !== 0) {
            verifyIteratorRange(iteratorString, iterator, expected, from, to);
            verifyIteratorRangeWithGetMany(iteratorForManyString, iteratorForMany, expected, from, to);
        }
    }

    function verifyVectorView(vectorViewString, vectorView, expected, isView) {
        logger.comment("var myIterator = " + vectorViewString + ".first()");
        var myIterator = vectorView.first();
        var myIteratorForMany = vectorView.first();
        verifyVectorViewRange(vectorViewString, vectorView, "myIterator", myIterator, "myIteratorForMany", myIteratorForMany, expected, 0, expected.length, isView);
    }

    runner.addTest({
        id: 1,
        desc: 'MarshalInArrayAsVectorView',
        pri: '0',
        preReq: function () {
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            logger.comment('var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];');
            var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];
            logger.comment("var myVectorView = (new Animals.Animal(1)).duplicateStringVectorView(myArray);");
            var myVectorView = (new Animals.Animal(1)).duplicateStringVectorView(myArray);
            logger.comment('verifyVectorView("myVectorView", myVectorView, ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"])');
            verifyVectorView("myVectorView", myVectorView, ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"]);

            var foundException = false;
            try {
                var result = myVectorView.getAt(11);
            }
            catch (e) {
                verify(e.number, hresults.bounds, 'e.number while myVectorView.getAt(11)');
                foundException = true;
            }
            assert(foundException, 'Expected exception was caught');

            logger.comment('myVectorView::IndexOf');
            var result = myVectorView.indexOf("Yellow");
            verify(result.index, 1, 'index of myVectorView.indexOf("Yellow")');
            verify(result.returnValue, true, 'returnValue of myVectorView.indexOf("Yellow")');
            result = myVectorView.indexOf("Purple");
            verify(result.returnValue, false, 'returnValue of myVectorView.indexOf("Purple")');
        }
    });

    runner.addTest({
        id: 2,
        desc: 'CheckIVectorViewLifeTime',
        pri: '0',
        preReq: function () {
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            logger.comment('var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];');
            var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];
            logger.comment("var myVectorView = (new Animals.Animal(1)).duplicateStringVectorView(myArray);");
            var myVectorView = (new Animals.Animal(1)).duplicateStringVectorView(myArray);
            logger.comment("myArray = undefined");
            myArray = undefined;
            logger.comment('verifyVectorView("myVectorView", myVectorView, ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"])');
            verifyVectorView("myVectorView", myVectorView, ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"]);

            var foundException = false;
            try {
                var result = myVectorView.getAt(11);
            }
            catch (e) {
                verify(e.number, hresults.bounds, 'e.number while myVectorView.getAt(11)');
                foundException = true;
            }
            assert(foundException, 'Expected exception was caught');

            logger.comment('myVectorView::IndexOf');
            var result = myVectorView.indexOf("Yellow");
            verify(result.index, 1, 'index of myVectorView.indexOf("Yellow")');
            verify(result.returnValue, true, 'returnValue of myVectorView.indexOf("Yellow")');
            result = myVectorView.indexOf("Purple");
            verify(result.returnValue, false, 'returnValue of myVectorView.indexOf("Purple")');
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
            logger.comment('var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];');
            var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];
            logger.comment("var myVectorView = (new Animals.Animal(1)).duplicateStringVectorView(myArray);");
            var myVectorView = (new Animals.Animal(1)).duplicateStringVectorView(myArray);
            verifyMembers("myVectorView", myVectorView, myVectorViewMembers, ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"]);
            logger.comment('myArray[11] = "Purple"');
            myArray[11] = "Purple";
            logger.comment('verifyVectorView("myVectorView", myVectorView, ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange", "undefined", "undefined", "Purple"])');
            verifyVectorView("myVectorView", myVectorView, ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange", "undefined", "undefined", "Purple"]);

            var foundException = false;
            try {
                var result = myVectorView.getAt(15);
            }
            catch (e) {
                verify(e.number, hresults.bounds, 'e.number while myVectorView.getAt(15)');
                foundException = true;
            }
            assert(foundException, 'Expected exception was caught');

            logger.comment('myVectorView::IndexOf');
            var result = myVectorView.indexOf("Yellow");
            verify(result.index, 1, 'index of myVectorView.indexOf("Yellow")');
            verify(result.returnValue, true, 'returnValue of myVectorView.indexOf("Yellow")');
            result = myVectorView.indexOf("Purple");
            verify(result.returnValue, true, 'returnValue of myVectorView.indexOf("Purple")');
            verify(result.index, 11, 'index of myVectorView.indexOf("Purple")');
            result = myVectorView.indexOf("White");
            verify(result.returnValue, false, 'returnValue of myVectorView.indexOf("White")');
        }
    });

    runner.addTest({
        id: 4,
        desc: 'AlterArrayWhileViewing',
        pri: '0',
        preReq: function () {
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            logger.comment('var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];');
            var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];
            logger.comment("var myVectorView = (new Animals.Animal(1)).duplicateStringVectorView(myArray);");
            var myVectorView = (new Animals.Animal(1)).duplicateStringVectorView(myArray);
            verifyMembers("myVectorView", myVectorView, myVectorViewMembers, ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"]);
            logger.comment('var myArrayToVerify = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];');
            var myArrayToVerify = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];
            logger.comment('var myIterator = myVectorView.first();');
            var myIterator = myVectorView.first();
            var myIteratorMany = myVectorView.first();
            logger.comment('verifyVectorViewRange("myVectorView", myVectorView, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 0, 2);');
            verifyVectorViewRange("myVectorView", myVectorView, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 0, 2);

            logger.comment('myArray[3] = "White";');
            myArray[3] = "White";
            logger.comment('myArrayToVerify[3] = "White";');
            myArrayToVerify[3] = "White";
            logger.comment('verifyVectorViewRange("myVectorView", myVectorView, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 2, 5);');
            verifyVectorViewRange("myVectorView", myVectorView, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 2, 5);

            logger.comment('myArray[5] = "Teal";');
            myArray[5] = "Teal";
            logger.comment('myArrayToVerify[5] = "Teal";');
            myArrayToVerify[5] = "Teal";
            logger.comment('verifyVectorViewRange("myVectorView", myVectorView, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 5, 9);');
            verifyVectorViewRange("myVectorView", myVectorView, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 5, 9);
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
            logger.comment('var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];');
            var myArray = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];
            logger.comment("var myVectorView = (new Animals.Animal(1)).duplicateStringVectorView(myArray);");
            var myVectorView = (new Animals.Animal(1)).duplicateStringVectorView(myArray);
            verifyMembers("myVectorView", myVectorView, myVectorViewMembers, ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"]);
            logger.comment('var myArrayToVerify = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];');
            var myArrayToVerify = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "Brown", "Orange"];
            logger.comment('var myIterator = myVectorView.first();');
            var myIterator = myVectorView.first();
            var myIteratorMany = myVectorView.first();
            logger.comment('verifyVectorViewRange("myVectorView", myVectorView, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 0, 2);');
            verifyVectorViewRange("myVectorView", myVectorView, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 0, 2);

            logger.comment('myArray.length = 7');
            myArray.length = 7;
            logger.comment('myArrayToVerify.length = 7');
            myArrayToVerify.length = 7;
            logger.comment('verifyVectorViewRange("myVectorView", myVectorView, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 2, 7);');
            verifyVectorViewRange("myVectorView", myVectorView, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 2, 7);

            logger.comment('myArray.length = 10;');
            myArray.length = 10;
            logger.comment('myArrayToVerify = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "undefined", "undefined", "undefined"];');
            myArrayToVerify = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "undefined", "undefined", "undefined"];
            logger.comment('verifyVectorViewRange("myVectorView", myVectorView, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 7, 10);');
            verifyVectorViewRange("myVectorView", myVectorView, "myIterator", myIterator, "myIteratorMany", myIteratorMany, myArrayToVerify, 7, 10);

            logger.comment('myArray.length = 13');
            myArray.length = 13;
            logger.comment('verify(myIterator.hasCurrent, true, "myIterator.hasCurrent");');
            verify(myIterator.hasCurrent, true, "myIterator.hasCurrent");
            logger.comment('myArrayToVerify = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "undefined", "undefined", "undefined", "undefined", "undefined", "undefined"];');
            myArrayToVerify = ["Pink", "Yellow", "Green", "Red", "Tan", "Blue", "Black", "undefined", "undefined", "undefined", "undefined", "undefined", "undefined"];
            logger.comment('verifyVectorMembers("myVectorView", myVectorView, myArrayToVerify);');
            verifyVectorMembers("myVectorView", myVectorView, myArrayToVerify);

            logger.comment('myArray.length = 5');
            myArray.length = 5;
            logger.comment('verify(myIterator.hasCurrent, false, "myIterator.hasCurrent")');
            logger.comment('myArrayToVerify.length = 5;');
            myArrayToVerify.length = 5;
            logger.comment('verifyVectorMembers("myVectorView", myVectorView, myArrayToVerify);');
            verifyVectorMembers("myVectorView", myVectorView, myArrayToVerify);
        }
    });

    runner.addTest({
        id: 6,
        desc: 'ProjectAlreadyMarshaledJsArrayAsVectorView',
        pri: '0',
        test: function () {
            logger.comment('var myArray = ["Pink", "Yellow", "Blue", "Red", "Green", "Brown", "Tan", "Black", "White"];');
            var myArray = ["Pink", "Yellow", "Blue", "Red", "Green", "Brown", "Tan", "Black", "White"];
            logger.comment("var myVectorView = (new Animals.Animal(1)).sendBackSameStringVectorView(myArray);");
            var myVectorView = (new Animals.Animal(1)).sendBackSameStringVectorView(myArray);

            logger.comment('verifyVectorView("myVectorView", myVectorView, myArray)');
            verifyVectorView("myVectorView", myVectorView, myArray);
            assert(!Array.isArray(myVectorView), "!Array.isArray(myVectorView)");
            assert(myArray !== myVectorView, "myArray !== myVectorView");

            logger.comment('myArray[5] = "Purple";');
            myArray[5] = "Purple";

            logger.comment('verifyVectorView("myVectorView", myVectorView, myArray)');
            verifyVectorView("myVectorView", myVectorView, myArray);
            assert(!Array.isArray(myVectorView), "!Array.isArray(myVectorView)");
            assert(myArray !== myVectorView, "myArray !== myVectorView");
        }
    });

    runner.addTest({
        id: 7,
        desc: 'VectorGetMany',
        pri: '0',
        test: function () {
            var myArray = ["Pink", "Yellow", "Blue", "Red", "Green", "Brown", "Tan", "Black", "White"];
            var myVector = (new Animals.Animal(1)).duplicateStringVectorView(myArray);
            logger.comment("fill the manyItems by calling getMany");
            var manyItems = new Array(5);
            var gotManyItems = myVector.getMany(2, manyItems);
            verify(gotManyItems, 5, "gotManyItems");
            for (var i = 0; i < gotManyItems; i++) {
                verify(manyItems[i], myArray[i + 2], "manyItems[" + i + "]");
            }
        }
    });

    Loader42_FileName = 'String array as Vector view tests';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
