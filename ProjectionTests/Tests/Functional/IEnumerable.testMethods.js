// main test file - including all common methods and test methods
    var winery;

    verify.overload = function verifyOverload(obj, prop, args, expected) {
        verify.equal(obj[prop].apply(obj, args), expected, prop);
    }

    verify.members = function verifyMembers(obj, expected) {
        var expect;
        for(var mem in obj) {
            expect = expected[mem];
            verify.defined(expect, mem);
            verify.typeOf(obj[mem], expect, mem);
        }
    };

    var myIteratorMembers = [
        ['current', 'object'],
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

        logger.comment("Dump of properties: " + myObjectString);
        logger.comment("typeof " + myObjectString + ": " + typeof myObject);
        logger.comment("Properties : " + objectDump);
        logger.comment("/Dump of properties: " + myObjectString + "\n");
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

    function verifyIteratorRangeWithGetMany(iteratorString, iterator, methodOnIterator, expected, from, to) {
        if (from === undefined) {
            from = 0;
        }

        if (to === undefined) {
            to = expected.length;
        }

        var manyItems = new Array(2);
        var gotManyItems;
        var index = from;
		var isEmpty = false;
        for (; index < to; ) {
            gotManyItems = iterator.getMany(manyItems);
            logger.comment("gotManyItems: " + gotManyItems);
			
			if (gotManyItems == 0) {
				isEmpty = true;
				logger.comment("EMPTY many items");
			    break;
			}
			
            if (((expected.length - index) / manyItems.length) >= 1) {
                verify(gotManyItems, manyItems.length, "gotManyItems");
            } else {
                verify(gotManyItems, expected.length - index, "gotManyItems");
            }

            for (var i = 0; i < gotManyItems && index < to; i++) {
				verify.equal(typeof(manyItems[i][methodOnIterator]), "function", "manyItems[i] " + methodOnIterator + " present")
				var actualValue = manyItems[i][methodOnIterator]();

                verify(actualValue, expected[index], "manyItems[" + i + "]");
                index++;
            }
        }

		if (isEmpty === false) {
			verify(iterator.hasCurrent, (index < expected.length - 1), iteratorString + '.hasCurrent');
			verify(index - from, to - from, 'number of members iterated over');
		}
    }

    function verifyIteratorRange(iteratorString, iterator, methodOnIterator, expected, from, to) {
        verifyMembers(iteratorString, iterator, myIteratorMembers);

        var iIndex = from;
        for (; iIndex < to; iIndex++) {
            assert(iterator.hasCurrent, iteratorString + '.hasCurrent');

			logger.comment("iterator.current = " + iterator.current);
			verify.equal(typeof(iterator.current[methodOnIterator]), "function", "methodOnIterator " + methodOnIterator + " present on iterator " + iteratorString)
            dumpObjectMembers("iterator.current (index = " + iIndex + ")", iterator.current);
			logger.comment("HW function: " + iterator.current["helloWorld"]());
			var actualValue = iterator.current[methodOnIterator]();
			
            verify(actualValue, expected[iIndex], iteratorString + '.current.' + methodOnIterator + '()');
            var hasNext = iterator.moveNext();
            verify(hasNext, (iIndex < expected.length - 1), iteratorString + '.moveNext()');
        }

        verify(iterator.hasCurrent, (iIndex < expected.length - 1), iteratorString + '.hasCurrent');
        verify(iIndex - from, to - from, 'number of members iterated over');
    }

    function verifyIterator(iteratorString, iterator, methodOnIterator, expected) {
        verifyIteratorRange(iteratorString, iterator, methodOnIterator, expected, 0, expected.length);
    }

    runner.globalSetup(function () {
        winery = new Winery.RWinery(1);
    });

// IEnumerable<Interface> - not reproing bug BLUE#183883/155545
function test1() {
	var c = winery.getEnumerableOfDefaultInterface();
	logger.comment("getEnumerableOfDefaultInterface: " + c);

	var d = JSON.stringify(Winery.IEnumerable.EnumerableOfDefaultInterface);
	logger.comment("JSON: " + d);

	dumpObjectMembers("EnumerableOfDefaultInterface.RTC instance", c);

	var cIterable = c.first();
	logger.comment("JSON cIterable: " + JSON.stringify(cIterable));

	dumpObjectMembers("EnumerableOfDefaultInterface.RTC.First() instance", cIterable);

	dumpObjectMembers("EnumerableOfDefaultInterface.RTC.First()[0] instance", cIterable.current);

	var expected = 
		[
			"IMethod.HelloWorld(int) called with index=1",
			"IMethod.HelloWorld(int) called with index=2",
			"IMethod.HelloWorld(int) called with index=3"
		];
		
	logger.comment("Get (item by item) test");
	verifyIterator("myIterator", cIterable, "helloWorld", expected);

	logger.comment("GetMany test");
	cIterable = c.first();
	verifyIteratorRangeWithGetMany("myIteratorWithGetMany", cIterable, "helloWorld", expected);

	logger.comment("GetMany test (end of iterator)");
	verifyIteratorRangeWithGetMany("myIteratorWithGetMany", cIterable, "helloWorld", expected);
}

// IEnumerable<Interface> - reproing bug BLUE#232596
function test2() {
	var c = winery.getEnumerableOfDefaultInterfaceWithMultipleSameName();
	logger.comment("getEnumerableOfDefaultInterface: " + c);

	var d = JSON.stringify(Winery.IEnumerable.EnumerableOfDefaultInterfaceWithMultipleSameName);
	logger.comment("JSON: " + d);

	dumpObjectMembers("EnumerableOfDefaultInterfaceWithMultipleSameName.RTC instance", c);

	var cIterable = c.first();
	logger.comment("JSON cIterable: " + JSON.stringify(cIterable));

	verify.equal(c.helloWorld(), "IMethod.HelloWorld(int) called with index=-1", "top level hw()");
	
	dumpObjectMembers("EnumerableOfDefaultInterfaceWithMultipleSameName.RTC.First() instance", cIterable);

	dumpObjectMembers("EnumerableOfDefaultInterfaceWithMultipleSameName.RTC.First()[0] instance", cIterable.current);

	var expected = 
		[
			"IMethod.HelloWorld(int) called with index=10",
			"IMethod.HelloWorld(int) called with index=20",
			"IMethod.HelloWorld(int) called with index=30"
		];
		
	logger.comment("Get (item by item) test");
	verifyIterator("myIterator", cIterable, "helloWorld", expected);

	logger.comment("GetMany test");
	cIterable = c.first();
	verifyIteratorRangeWithGetMany("myIteratorWithGetMany", cIterable, "helloWorld", expected);

	logger.comment("GetMany test (end of iterator)");
	verifyIteratorRangeWithGetMany("myIteratorWithGetMany", cIterable, "helloWorld", expected);
}

// IEnumerable<IMethod*> by itself - reproing bug BLUE#183883/155545
function test3() {
	var c = winery.getEnumerableOfItself();
	logger.comment("getEnumerableOfItself: " + c);

	var d = JSON.stringify(Winery.IEnumerable.EnumerableOfItself);
	logger.comment("JSON: " + d);

	dumpObjectMembers("EnumerableOfItself.RTC instance", c);

	var cIterable = c.first();
	logger.comment("JSON cIterable: " + JSON.stringify(cIterable));

	verify.equal(c.helloWorld(), "IMethod.HelloWorld(int) called with index=-100", "top level hw()");
	verify.equal(c.helloWorld2(), "IMethod.HelloWorld2() called", "top level hw2()");
	
	dumpObjectMembers("EnumerableOfItself.RTC.First() instance", cIterable);

	dumpObjectMembers("EnumerableOfItself.RTC.First()[0] instance", cIterable.current);

	var expected = 
		[
			"IMethod.HelloWorld(int) called with index=100",
			"IMethod.HelloWorld(int) called with index=200",
			"IMethod.HelloWorld(int) called with index=300"
		];
		
	logger.comment("Get (item by item) test");
	verifyIterator("myIterator", cIterable, "helloWorld", expected);

	logger.comment("GetMany test");
	cIterable = c.first();
	verifyIteratorRangeWithGetMany("myIteratorWithGetMany", cIterable, "helloWorld", expected);

	logger.comment("GetMany test (end of iterator)");
	verifyIteratorRangeWithGetMany("myIteratorWithGetMany", cIterable, "helloWorld", expected);
}

// IEnumerable<RTC*> by itself - reproing bug BLUE#183883/155545
function test4() {
	var c = winery.getEnumerableOfItselfAsRTC();
	logger.comment("getEnumerableOfItself: " + c);

	var d = JSON.stringify(Winery.IEnumerable.EnumerableOfItselfAsRTC);
	logger.comment("JSON: " + d);

	dumpObjectMembers("EnumerableOfItselfAsRTC.RTC instance", c);

	var cIterable = c.first();
	logger.comment("JSON cIterable: " + JSON.stringify(cIterable));

	verify.equal(c.helloWorld(), "IMethod.HelloWorld(int) called with index=-1000", "top level hw()");
	verify.equal(c.helloWorld2(), "RTC.HelloWorld2() called", "top level hw2()");
	
	dumpObjectMembers("EnumerableOfItselfAsRTC.RTC.First() instance", cIterable);

	dumpObjectMembers("EnumerableOfItselfAsRTC.RTC.First()[0] instance", cIterable.current);

	var expected = 
		[
			"IMethod.HelloWorld(int) called with index=1000",
			"IMethod.HelloWorld(int) called with index=2000",
			"IMethod.HelloWorld(int) called with index=3000"
		];
		
	logger.comment("Get (item by item) test");
	verifyIterator("myIterator", cIterable, "helloWorld", expected);

	logger.comment("GetMany test");
	cIterable = c.first();
	verifyIteratorRangeWithGetMany("myIteratorWithGetMany", cIterable, "helloWorld", expected);

	logger.comment("GetMany test (end of iterator)");
	verifyIteratorRangeWithGetMany("myIteratorWithGetMany", cIterable, "helloWorld", expected);

	// hello world 2 (on iMethod2)
	expected = 
		[
			"RTC.HelloWorld2() called",
			"RTC.HelloWorld2() called",
			"RTC.HelloWorld2() called",
		];
	cIterable = c.first();
	verifyIterator("myIterator", cIterable, "helloWorld2", expected);
	cIterable = c.first();
	verifyIteratorRangeWithGetMany("myIteratorWithGetMany", cIterable, "helloWorld2", expected);
}
