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

    var animalVectorMembers = ["Blue", "Red", "Yellow", "Green", "Pink", "Black", "White", "Tan", "Magenta", "Orange"];

    function verifyVectorMemberAt(actualVector, expectedVector, iIndex) {
        verify(actualVector.getAt(iIndex), expectedVector[iIndex], 'myVector.getAt(' + iIndex + ')');
    }

    function verifyAllVectorMembers(actualVector, expectedVector) {
        verify(actualVector.size, expectedVector.length, 'myVector.size');
        for (var iIndex = 0; iIndex < actualVector.size; iIndex++) {
            verifyVectorMemberAt(actualVector, expectedVector, iIndex);
        }
        verify(iIndex, expectedVector.length, 'number of members');
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

    function verifyIterator(iterator, expected) {
        for (var iIndex = 0; iIndex < expected.length; iIndex++) {
            assert(iterator.hasCurrent, 'iterator.hasCurrent');
            verify(iterator.current, expected[iIndex], 'iterator.current');
            var hasNext = iterator.moveNext();
            if (iIndex < expected.length - 1) {
                verify(hasNext, true, 'iterator.moveNext()');
            }
            else {
                verify(hasNext, false, 'iterator.moveNext()');
            }
        }
        verify(iterator.hasCurrent, false, 'iterator.hasCurrent');
        verify(iIndex, expected.length, 'number of members');
    }

    runner.addTest({
        id: 1,
        desc: 'MarshalVectorOut',
        pri: '0',
        test: function () {
            var myVector = (new Animals.Animal(1)).getStringVector();
            verifyMembers("myVector", myVector, myVectorMembers, animalVectorMembers);
        }
    });

    runner.addTest({
        id: 2,
        desc: 'VectorGetAt_VectorSize',
        pri: '0',
        test: function () {
            var myVector = (new Animals.Animal(1)).getStringVector();
            verifyAllVectorMembers(myVector, animalVectorMembers);
        }
    });

    runner.addTest({
        id: 3,
        desc: 'VectorSetAt',
        pri: '0',
        test: function () {
            var myVector = (new Animals.Animal(1)).getStringVector();
            verifyVectorMemberAt(myVector, animalVectorMembers, 2);
            verifyVectorMemberAt(myVector, animalVectorMembers, 5);
            logger.comment('myVector.setAt(5) with myVector.getAt(2))');
            myVector.setAt(5, myVector.getAt(2));
            verifyVectorMemberAt(myVector, animalVectorMembers, 2);
            verify(myVector.getAt(5), myVector.getAt(2), 'myVector.getAt(5)');
        }
    });

    runner.addTest({
        id: 4,
        desc: 'VectorRemoveAt',
        pri: '0',
        test: function () {
            var myVector = (new Animals.Animal(1)).getStringVector();
            logger.comment('myVector.removeAt(6)');
            myVector.removeAt(6);
            var updatedAnimalVectorMembers = ["Blue", "Red", "Yellow", "Green", "Pink", "Black", "Tan", "Magenta", "Orange"]; ;
            verifyAllVectorMembers(myVector, updatedAnimalVectorMembers);
        }
    });

    runner.addTest({
        id: 5,
        desc: 'VectorInsertAt',
        pri: '0',
        test: function () {
            var myVector = (new Animals.Animal(1)).getStringVector();
            logger.comment('myVector.insertAt(1, "Purple")');
            myVector.insertAt(1, "Purple");
            var updatedAnimalVectorMembers = ["Blue", "Purple", "Red", "Yellow", "Green", "Pink", "Black", "White", "Tan", "Magenta", "Orange"]; ;
            verifyAllVectorMembers(myVector, updatedAnimalVectorMembers);
        }
    });

    runner.addTest({
        id: 6,
        desc: 'VectorRemoveAtEnd',
        pri: '0',
        test: function () {
            var myVector = (new Animals.Animal(1)).getStringVector();
            logger.comment('myVector.removeAtEnd()');
            myVector.removeAtEnd();
            var updatedAnimalVectorMembers = ["Blue", "Red", "Yellow", "Green", "Pink", "Black", "White", "Tan", "Magenta"];
            verifyAllVectorMembers(myVector, updatedAnimalVectorMembers);
        }
    });

    runner.addTest({
        id: 7,
        desc: 'VectorIndexOf',
        pri: '0',
        test: function () {
            var myVector = (new Animals.Animal(1)).getStringVector();
            var result = myVector.indexOf("Red");
            verify(result.index, 1, 'index of myVector.indexOf("Red")');
            verify(result.returnValue, true, 'returnValue of myVector.indexOf("Red")');
            var result = myVector.indexOf("Purple");
            verify(result.returnValue, false, 'returnValue of myVector.indexOf("Purple")');
        }
    });

    runner.addTest({
        id: 8,
        desc: 'VectorClear',
        pri: '0',
        test: function () {
            var myVector = (new Animals.Animal(1)).getStringVector();
            logger.comment('myVector.clear()');
            myVector.clear();
            var updatedAnimalVectorMembers = [];
            verifyAllVectorMembers(myVector, updatedAnimalVectorMembers);
        }
    });

    runner.addTest({
        id: 9,
        desc: 'VectorAppend',
        pri: '0',
        test: function () {
            var myVector = (new Animals.Animal(1)).getStringVector();
            myVector.append("Purple");
            myVector.append("Brown");
            var updatedAnimalVectorMembers = ["Blue", "Red", "Yellow", "Green", "Pink", "Black", "White", "Tan", "Magenta", "Orange", "Purple", "Brown"];
            verifyAllVectorMembers(myVector, updatedAnimalVectorMembers);
        }
    });

    runner.addTest({
        id: 10,
        desc: 'VectorIterableFirst',
        pri: '0',
        test: function () {
            var myVector = (new Animals.Animal(1)).getStringVector();
            var myVectorIterator = myVector.first();
            verifyMembers("myVectorIterator", myVectorIterator, myIteratorMembers);
            verifyIterator(myVectorIterator, animalVectorMembers);

            logger.comment("Iterator::getMany");
            var myNewIterator = myVector.first();
            verifyIteratorRangeWithGetMany("myNewIterator", myNewIterator, animalVectorMembers);
        }
    });

    runner.addTest({
        id: 11,
        desc: 'VectorGetView',
        pri: '0',
        preReq: function () {
            // CLR vector.getView not returning IVectorView, it returns IVector with IVectorVIew member functions
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            var myVector = (new Animals.Animal(1)).getStringVector();
            var myVectorView = myVector.getView();
            verifyMembers("myVectorView", myVectorView, myVectorViewMembers, animalVectorMembers);

            logger.comment('myVectorView::getAt and myVectorView::size');
            verifyAllVectorMembers(myVectorView, animalVectorMembers);

            logger.comment('myVectorView::indexOf');
            var result = myVectorView.indexOf("Red");
            verify(result.index, 1, 'index of myVectorView.indexOf("Red")');
            verify(result.returnValue, true, 'returnValue of myVectorView.indexOf("Red")');
            var result = myVectorView.indexOf("Purple");
            verify(result.returnValue, false, 'returnValue of myVectorView.indexOf("Purple")');

            logger.comment('myVectorView::first');
            var myVectorViewIterator = myVectorView.first();
            verifyMembers("myVectorViewIterator", myVectorViewIterator, myIteratorMembers);
            verifyIterator(myVectorViewIterator, animalVectorMembers);

            logger.comment("myVectorView::getMany");
            var manyItems = new Array(3);
            var gotManyItems = myVectorView.getMany(5, manyItems);
            verify(gotManyItems, 3, "gotManyItems");
            for (var i = 0; i < gotManyItems; i++) {
                verify(manyItems[i], animalVectorMembers[i + 5], "manyItems[" + i + "]");
            }
        }
    });

    runner.addTest({
        id: 12,
        desc: 'PassVectorAsInParam',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);
            var myVector = myAnimal.getStringVector();
            logger.comment('var myDuplicateVector =  myAnimal.copyStringVector(myVector)');
            var myDuplicateVector = myAnimal.copyStringVector(myVector);
            verifyMembers("myDuplicateVector", myDuplicateVector, myVectorMembers, animalVectorMembers);
            verifyAllVectorMembers(myDuplicateVector, animalVectorMembers);
        }
    });

    runner.addTest({
        id: 13,
        desc: 'VectorGetMany',
        pri: '0',
        test: function () {
            var myVector = (new Animals.Animal(1)).getStringVector();
            logger.comment("fill the manyItems by calling getMany");
            var manyItems = new Array(5);
            var gotManyItems = myVector.getMany(2, manyItems);
            verify(gotManyItems, 5, "gotManyItems");
            for (var i = 0; i < gotManyItems; i++) {
                verify(manyItems[i], animalVectorMembers[i + 2], "manyItems[" + i + "]");
            }
        }
    });

    runner.addTest({
        id: 14,
        desc: 'VectorReplaceAll',
        pri: '0',
        test: function () {
            var myVector = (new Animals.Animal(1)).getStringVector();
            logger.comment("replace the vector by new array");
            var manyItems = ["Crimson", "Turquoise", "Raspberry pink", "Lavender blue"];
            myVector.replaceAll(manyItems);
            verifyMembers("myVector", myVector, myVectorMembers, manyItems);
        }
    });

    Loader42_FileName = 'Generic Vector tests';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
