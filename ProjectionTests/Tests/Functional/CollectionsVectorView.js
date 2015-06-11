if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    function verifyDescriptor(obj, prop, attributes) {
        if (((typeof obj !== "object") && (typeof obj !== "function")) || (typeof attributes !== "object")) {
            logger.comment("Error: Invalid Arg to verifyDescriptor");
            return false;
        }
        if (!(prop in obj)) {
            logger.comment("Error: Object does not have property: " + prop);
            return false;
        }

        // Get property descriptor from either the object or prototype
        var desc = Object.getOwnPropertyDescriptor(obj, prop);
        if (desc === undefined) {
            var prototype = Object.getPrototypeOf(obj);
            if (prototype.hasOwnProperty(prop)) {
                desc = Object.getOwnPropertyDescriptor(prototype, prop);
            } else {
                // If the property is not a property of either the object or prototype, 
                // evaluate based on value alone
                return (obj[prop] === attributes.value);
            }
        }
        // Handle getter/setter case
        if ((desc.get !== undefined) || (desc.set !== undefined)) {
            if (attributes.writable && (desc.set === undefined)) { return false; }
            if (!attributes.writable && (desc.set !== undefined)) { return false; }
            for (var x in attributes) {
                if ((x != "writable") && (x != "value")) {
                    if (!desc.hasOwnProperty(x) || (desc[x] !== attributes[x])) {
                        return false;
                    }
                }
            }
            return (obj[prop] === attributes.value);
        }
        // Verify all attributes specified are correct on the property descriptor 
        for (var x in attributes) {
            if (!desc.hasOwnProperty(x) || (desc[x] !== attributes[x])) {
                return false;
            }
        }
        return true;
    }

    function checkPrototype(prototype, obj) {
        logger.comment("Checking prototype of object");
        verify(prototype.isPrototypeOf(obj), true, "prototype.isPrototypeOf(obj)");
    }

    function enumerateOver(obj, name, enumerable, child) {
        logger.comment("Enumerating over: " + name);
        var found = false;
        var propList = [];
        try {
            for (var prop in obj) {
                if (prop == child) { found = true; }
                propList.push(prop);
                logger.comment("\t" + prop);
            }
        }
        catch (e) {
            logger.comment("got exception when calling enumerateOver: " + e);
        }

        verify(propList.length > 0, enumerable, "Enumerate children");
        if (child !== undefined) {
            verify(found, enumerable, "Expected child (" + child + ") on " + name);
        }
    }

    function checkHasProperty(obj, prop, defined) {
        var succeeded;
        logger.comment("Checking if object has property: " + prop);
        try {
            succeeded = obj.hasOwnProperty(prop);
        }
        catch (e) {
            succeeded = false;
            logger.comment("got exception when calling checkHasProperty: " + e);
        }

        verify(succeeded, defined, "Object has property: " + prop);
    }

    function setAndCheckProperty(obj, prop, val, writable) {
        logger.comment("Attempting to set property (" + prop + ") to: [" + val + "]");
        var succeeded;
        try {
            obj[prop] = val;
            succeeded = obj[prop] === val;
        }
        catch (e) {
            succeeded = false;
            logger.comment("got exception when calling setAndCheckProperty: " + e);
        }

        verify(succeeded, writable, "Able to write to property: " + prop);
    }

    function addAndCheckProperty(obj, prop, extensible, attrib) {
        var succeeded;
        var attributes;
        logger.comment("Attempting to add property: " + prop);
        if (attrib !== undefined) {
            attributes = attrib;
        } else {
            attributes = { writable: true, enumerable: true, configurable: true, value: undefined }
        }
        try {
            Object.defineProperty(obj, prop, attributes);
            succeeded = obj.hasOwnProperty(prop) && verifyDescriptor(obj, prop, attributes);
        }
        catch (e) {
            succeeded = false;
            logger.comment("got exception when calling addAndCheckProperty: " + e);
        }

        verify(succeeded, extensible, "Able to add property: " + prop);
    }

    function deleteAndCheckProperty(obj, prop, configurable) {
        var succeeded;
        logger.comment("Attempting to delete property: " + prop);
        verify(prop in obj, true, "Object has property [" + prop + "]");

        try {
            succeeded = delete obj[prop];
            succeeded = succeeded && (!(prop in obj));
        }
        catch (e) {
            succeeded = false;
            logger.comment("got exception when calling deleteAndCheckProperty: " + e);
        }
        verify(succeeded, configurable, "Able to delete property: " + prop);
    }

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

    function dump(myObjectString, myObject) {
        var objectDump = easyMembersPrint(myObjectString, myObject);

        logger.comment("typeof " + myObjectString + ": " + typeof myObject);
        logger.comment("Dump of properties : " + objectDump);
    }

    function verifyVectorObjectDump(stringVector, vectorObject, arrayToVerify, restOfProperties) {
        dump(stringVector, vectorObject);

        verify(typeof vectorObject, "object", "typeof " + stringVector);
        var numberOfMembers = 0;
        var checkInArray = 0;
        var propertiesIndex = 0;
        for (p in vectorObject) {
            // Look in properties
            if (checkInArray >= arrayToVerify.length) {
                verify(p, restOfProperties[propertiesIndex][0], stringVector + ' : property ' + p);
                verify(typeof vectorObject[p], restOfProperties[propertiesIndex][1], 'typeof ' + stringVector + '[' + p + ']');

                if (typeof vectorObject[p] == 'function') {
                    verify(vectorObject[p].length, restOfProperties[propertiesIndex][2], stringVector + '[' + p + '].length');
                    logger.comment('Setting length of function to be 10');
                    vectorObject[p].length = 10;
                    verify(vectorObject[p].length, restOfProperties[propertiesIndex][2], stringVector + '[' + p + '].length');
                }
                propertiesIndex++;
            }
            // in the array
            else {
                verify(p, checkInArray.toString(), stringVector + ' : property ' + p);
                verify(typeof vectorObject[p], typeof arrayToVerify[checkInArray], 'typeof ' + stringVector + '[' + p + ']');
                checkInArray++;
            }
            numberOfMembers++;
        }

        verify(numberOfMembers, arrayToVerify.length + restOfProperties.length, 'number of members of ' + stringVector);
    }

    function dumpVector(vector, stringVector) {
        logger.comment(stringVector + "[" + vector.length + "]: " + vector);
    }

    function verifyVectorAndArrayItems(myVectorString, myVectorView, myArray) {
        dumpVector(myVectorView, myVectorString);
        dumpVector(myArray, "myArray");
        verify(myVectorView.length, myArray.length, myVectorString + ".length")

        for (var iIndex = 0; iIndex < myVectorView.length; iIndex++) {
            verify(myVectorView[iIndex], myArray[iIndex], myVectorString + '[' + iIndex + ']');
        }
    }

    function assertException(expression, errorType) {
        logger.comment(expression);
        verify.exception(function () {
            eval(expression);
        }, errorType, expression);
    }

    function assertNoException(expression) {
        logger.comment(expression);
        verify.noException(function () {
            eval(expression);
        }, expression);
    }

    var animalFactory;
    var myAnimal;
    var myVectorView;
    var myArray;

    // name, type, length of function
    var vectorMembers = [
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

    // name, type, length if function
    var vectorViewMembers = [
    ['first', 'function', 0],
    ['getAt', 'function', 1],
    ['getMany', 'function', 2],
    ['indexOf', 'function', 1],
    ['size', 'number'],
    ];

    function IsLessThan5(x) {
        if (x < 5) {
            return true;
        }
        return false;
    }

    function IsLessThan7(x) {
        if (x < 7) {
            return true;
        }
        return false;
    }

    function IsLessThan10(x) {
        if (x < 10) {
            return true;
        }
        return false;
    }

    function IsGreaterThan10(x) {
        if (x > 10) {
            return true;
        }
        return false;
    }

    runner.globalSetup(function () {
        logger.comment("animalFactory = Animals.Animal");
        animalFactory = Animals.Animal;

        logger.comment("myAnimal = new animalFactory(1)");
        myAnimal = new animalFactory(1);

        logger.comment("\nfunction IsLessThan5(x)\n{\n    if (x < 5)\n    {\n        return true;\n    }\n    return false;\n}");
        logger.comment("\nfunction IsLessThan7(x)\n{\n    if (x < 7)\n    {\n        return true;\n    }\n    return false;\n}");
        logger.comment("\nfunction IsLessThan10(x)\n{\n    if (x < 10)\n    {\n        return true;\n    }\n    return false;\n}");
        logger.comment("\nfunction IsGreaterThan10(x)\n{\n    if (x > 10)\n    {\n        return true;\n    }\n    return false;\n}");
    });

    function GetVectorView() {
        logger.comment("var myVectorView = myAnimal.getVector()");
        var myVector = myAnimal.getVector();
        myVectorView = myVector.getView();
        myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
        dumpVector(myVectorView, "myVectorView");
    }

    function verifyArrayPrototypeInChain(myFirstVector, myFirstVectorString, mySecondVector, mySecondVectorString, inChain) {
        logger.comment(myFirstVectorString + ": " + myFirstVector);
        logger.comment(mySecondVectorString + ": " + mySecondVector);

        assert(Object.getPrototypeOf(myFirstVector) === Object.getPrototypeOf(mySecondVector), "Object.getPrototypeOf(" + myFirstVectorString + ") === Object.getPrototypeOf(" + mySecondVectorString + ")");

        logger.comment('Array.prototype.arrayTestProperty = "Array\'s Array Test Property"');
        Array.prototype.arrayTestProperty = "Array's Array Test Property";
        logger.comment('Object.prototype.arrayTestProperty = "Object\'s Array Test Property"');
        Object.prototype.arrayTestProperty = "Object's Array Test Property";
        logger.comment('Object.prototype.objectTestProperty = "Object\'s Object Test Property";');
        Object.prototype.objectTestProperty = "Object's Object Test Property";

        if (inChain === false) {
            verify(myFirstVector.arrayTestProperty, "Object's Array Test Property", myFirstVectorString + ".arrayTestProperty");
        }
        else {
            verify(myFirstVector.arrayTestProperty, "Array's Array Test Property", myFirstVectorString + ".arrayTestProperty");
        }

        verify(myFirstVector.objectTestProperty, "Object's Object Test Property", myFirstVectorString + ".objectTestProperty");

        verify(mySecondVector.arrayTestProperty, "Array's Array Test Property", mySecondVectorString + ".arrayTestProperty");
        verify(mySecondVector.objectTestProperty, "Object's Object Test Property", mySecondVectorString + ".objectTestProperty");

        logger.comment('delete Array.prototype.arrayTestProperty');
        delete Array.prototype.arrayTestProperty;
        logger.comment('delete Object.prototype.arrayTestProperty');
        delete Object.prototype.arrayTestProperty;
        logger.comment('delete Object.prototype.objectTestProperty');
        delete Object.prototype.objectTestProperty;
    }

    runner.addTest({
        id: 1,
        desc: 'MarshalVectorViewOut',
        pri: '0',
        preReq: function () {
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            logger.comment("var myVectorView = myAnimal.getVector()");
            var myVector = myAnimal.getVector();
            myVectorView = myVector.getView();
            myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            verifyVectorObjectDump("myVector", myVector, myArray, vectorMembers);
            verifyVectorObjectDump("myVectorView", myVectorView, myArray, vectorViewMembers);
        }
    });

    runner.addTest({
        id: 2,
        desc: 'VectorViewArray_Index_length',
        pri: '0',
        preReq: function () {
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            GetVectorView();
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            assertNoException("myVectorView[2] = 78;");
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            assertNoException("myVectorView[9] = 10;");
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            logger.comment("var n = delete myVectorView[5]");
            var n = delete myVectorView[5];
            verify(n, false, "n");
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            logger.comment("n = delete myVectorView[12]");
            n = delete myVectorView[12];
            verify(n, false, "n");
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            logger.comment("n = delete myVectorView");
            n = delete myVectorView;
            verify(n, false, "n");
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            assertNoException("myVectorView[12] = 12");
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);
            verify(myVectorView[12], undefined, "myVectorView[12]");

            assertException("myVectorView.length = 10", Error);
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            assertException("myVectorView.length = 5", Error);
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            assertException("myVectorView.length = 30;", Error);
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            assertException("myVectorView.length = 'abc';", Error);
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            assertException("myVectorView.length = 0;", Error);
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);
        }
    });

    runner.addTest({
        id: 3,
        desc: 'VectorViewAsArray_Push_Pop',
        pri: '0',
        preReq: function () {
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            GetVectorView();

            assertException("var n = myVectorView.pop()", Error)
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            for (var iIndex = 1; iIndex < 8; iIndex++) {
                assertException("myVectorView.push(" + (iIndex * 11) + ")", TypeError);
            }
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            assertException('myVectorView.push(11, 22, 33, 44, 55, 66)', TypeError);
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            assertException('Array.prototype.push.call(myVectorView, 100)', TypeError);
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            assertException("n = myVectorView.pop()", TypeError);
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            assertException("n = Array.prototype.pop.call(myVectorView);", TypeError);
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);
        }
    });

    runner.addTest({
        id: 4,
        desc: 'VectorViewAsArray_Shift_Unshift',
        pri: '0',
        preReq: function () {
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            GetVectorView();

            assertException("myVectorView.shift()", TypeError);
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            assertException("myVectorView.unshift()", Error);
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            assertException("myVectorView.unshift(0)", TypeError);
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            assertException("myVectorView.unshift(25, 50, 100)", TypeError);
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);
        }
    });

    runner.addTest({
        id: 5,
        desc: 'VectorViewAsArray_Slice',
        pri: '0',
        test: function () {
            GetVectorView();

            verify(myVectorView.slice(), myArray.slice(), 'myVectorView.slice()');
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            verify(myVectorView.slice(5, 2), myArray.slice(5, 2), "myVectorView.slice(5, 2)");
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            verify(myVectorView.slice(5, 8), myArray.slice(5, 8), 'myVectorView.slice(5, 8)');
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            verify(myVectorView.slice(5), myArray.slice(5), "myVectorView.slice(5)");
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            verify(myVectorView.slice(-2), myArray.slice(-2), "myVectorView.slice(-2)");
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            verify(myVectorView.slice(-7, -2), myArray.slice(-7, -2), "myVectorView.slice(-7, -2)");
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);
        }
    });

    runner.addTest({
        id: 6,
        desc: 'VectorViewAsArray_Splice',
        pri: '0',
        preReq: function () {
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            GetVectorView();

            assertException("myVectorView.splice()", Error);
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            assertException("myVectorView.splice(5, 0)", Error);
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            assertException("myVectorView.splice(5, 2)", TypeError);
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            assertException('myVectorView.splice(2, 2, 14, 98, 54)', TypeError);
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            assertException("myVectorView.splice(4, 0, 32, 83)", TypeError);
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);
        }
    });

    runner.addTest({
        id: 7,
        desc: 'VectorViewAsArray_Every',
        pri: '0',
        test: function () {
            GetVectorView();

            verify(myVectorView.every(IsLessThan7), myArray.every(IsLessThan7), 'myVectorView.every(IsLessThan7)');
            verify(myVectorView.every(IsLessThan10), myArray.every(IsLessThan10), "myVectorView.every(IsLessThan10)");
        }
    });

    runner.addTest({
        id: 8,
        desc: 'VectorViewAsArray_Some',
        pri: '0',
        test: function () {
            GetVectorView();

            verify(myVectorView.some(IsLessThan7), myArray.some(IsLessThan7), "myVectorView.some(IsLessThan7)");
            verify(myVectorView.some(IsGreaterThan10), myArray.some(IsGreaterThan10), "myVectorView.some(IsGreaterThan10)");
        }
    });

    runner.addTest({
        id: 9,
        desc: 'VectorViewAsArray_Filter',
        pri: '0',
        test: function () {
            GetVectorView();

            verify(myVectorView.filter(IsLessThan5), myArray.filter(IsLessThan5), "myVectorView.filter(IsLessThan5)");
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            verify(myVectorView.filter(IsGreaterThan10), myArray.filter(IsGreaterThan10), "myVectorView.filter(IsGreaterThan10)");
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);
        }
    });

    runner.addTest({
        id: 10,
        desc: 'VectorViewAsArray_ForEach',
        pri: '0',
        test: function () {
            GetVectorView();

            logger.comment("\nfunction forEachMethod(x, idx)\n{\n    print('[' + idx + ']', '=', x);\n}");
            function forEachMethod(x, idx) {
                logger.comment('[' + idx + ']', '=', x);
            }

            logger.comment("myVectorView.forEach(forEachMethod);");
            myVectorView.forEach(forEachMethod);
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);
        }
    });

    runner.addTest({
        id: 11,
        desc: 'VectorViewAsArray_Map',
        pri: '0',
        test: function () {
            GetVectorView();

            logger.comment("\nfunction mapMethod(x)\n{\n    if (IsLessThan5(x))\n    {\n        return x;\n    }\n    return -1;\n}");
            function mapMethod(x) {
                if (IsLessThan5(x)) {
                    return x;
                }
                return -1;
            }

            verify(myVectorView.map(mapMethod), myArray.map(mapMethod), "myVectorView.map(mapMethod)");
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);
        }
    });

    runner.addTest({
        id: 12,
        desc: 'VectorViewAsArray_Reduce_ReduceRight',
        pri: '0',
        test: function () {
            GetVectorView();

            logger.comment("\nfunction doTotal(a, b)\n{\n    return a + b;\n}");
            function doTotal(a, b) {
                return a + b;
            }

            verify(myVectorView.reduce(doTotal), myArray.reduce(doTotal), "myVectorView.reduce(doTotal)");
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            verify(myVectorView.reduceRight(doTotal), myArray.reduceRight(doTotal), "myVectorView.reduceRight(doTotal)");
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);
        }
    });

    runner.addTest({
        id: 13,
        desc: 'VectorViewAsArray_Reverse_Sort',
        pri: '0',
        preReq: function () {
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            GetVectorView();

            assertException("var a = myVectorView.sort()", TypeError);
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            assertException("var a = myVectorView.reverse()", TypeError);
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            assertException("a = Array.prototype.sort.call(myVectorView)", TypeError);
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            assertException("a = Array.prototype.reverse.call(myVectorView);", TypeError);
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);
        }
    });

    runner.addTest({
        id: 14,
        desc: 'VectorViewAsArray_Concat',
        pri: '0',
        test: function () {
            GetVectorView();

            verify(myVectorView.concat([1, 2, 3]), [myVectorView, 1, 2, 3], "myVectorView.concat([1,2,3])");
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            logger.comment("var mySecondVector = myAnimal.getVector()");
            var mySecondVector = myAnimal.getVector();
            logger.comment("var mySecondVectorView = mySecondVector.getView();");
            var mySecondVectorView = mySecondVector.getView();
            var mySecondArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            verifyVectorAndArrayItems("mySecondVectorView", mySecondVectorView, mySecondArray);

            verify(myVectorView.concat(mySecondVectorView), [myVectorView, mySecondVectorView], "myVectorView.concat(mySecondVectorView)");
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);
            verifyVectorAndArrayItems("mySecondVectorView", mySecondVectorView, mySecondArray);

            logger.comment("var jsArray = [1,2,3];");
            var jsArray = [1, 2, 3];
            verify(jsArray.concat(myVectorView), [1, 2, 3, myVectorView], "jsArray.concat(myVectorView)");
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);
            verify(jsArray, [1, 2, 3], "jsArray");

            verify(myVectorView.concat(1, 2, 3, 4), [myVectorView, 1, 2, 3, 4], "myVectorView.concat(1,2,3,4)");
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);
        }
    });

    runner.addTest({
        id: 15,
        desc: 'VectorViewAsArray_Join',
        pri: '0',
        test: function () {
            GetVectorView();

            verify(myVectorView.join(' , '), myArray.join(' , '), "myVectorView.join(' , ')");
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            verify(myVectorView.join([9, 'b', 'c']), myArray.join([9, 'b', 'c']), "myVectorView.join([9,'b','c'])");
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            logger.comment("var mySecondVector = myAnimal.getVector()");
            var mySecondVector = myAnimal.getVector();
            logger.comment("var mySecondVectorView = mySecondVector.getView();");
            var mySecondVectorView = mySecondVector.getView();
            var mySecondArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            verifyVectorAndArrayItems("mySecondVectorView", mySecondVectorView, mySecondArray);

            verify(myVectorView.join(mySecondVectorView), myArray.join(mySecondVectorView), "myVectorView.join(mySecondVector)");
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);
            verifyVectorAndArrayItems("mySecondVectorView", mySecondVectorView, mySecondArray);

            logger.comment("var jsArray = [1,2,3];");
            var jsArray = [1, 2, 3];
            verify(jsArray.join(myVectorView), jsArray.join(myArray), "jsArray.join(myVectorView)");
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);
            verify(jsArray, [1, 2, 3], "jsArray");
        }
    });

    runner.addTest({
        id: 16,
        desc: 'VectorViewAsArray_IndexOf',
        pri: '0',
        test: function () {
            GetVectorView();

            // Old test cases for Array.prototype.indexOf (shadowed by camel cased IVectorView.indexOf)
            verify(Array.prototype.indexOf.call(myVectorView, 5), myArray.indexOf(5), "Array.prototype.indexOf.call(myVectorView, 5)");
            verify(Array.prototype.indexOf.call(myVectorView, 786), myArray.indexOf(786), "Array.prototype.indexOf.call(myVectorView, 786)");
            verify(Array.prototype.indexOf.call(myVectorView, 5, 5), myArray.indexOf(5, 5), "Array.prototype.indexOf.call(myVectorView, 5, 5)");
            verify(Array.prototype.indexOf.call(myVectorView, 5, 2), myArray.indexOf(5, 2), "Array.prototype.indexOf.call(myVectorView, 5, 2)");

            // New cases for IVectorView.indexOf
            verify(myVectorView.indexOf(5).index, myArray.indexOf(5), "myVectorView.indexOf(5)");
            verify(myVectorView.indexOf(786).returnValue, false, "myVectorView.indexOf(786)");
        }
    });

    runner.addTest({
        id: 17,
        desc: 'VectorViewAsArray_LastIndexOf',
        pri: '0',
        test: function () {
            GetVectorView();

            verify(myVectorView.lastIndexOf(5), myArray.lastIndexOf(5), "myVectorView.lastIndexOf(5)");
            verify(myVectorView.lastIndexOf(786), myArray.lastIndexOf(786), "myVectorView.lastIndexOf(786)");
            verify(myVectorView.lastIndexOf(2, 3), myArray.lastIndexOf(2, 3), "myVectorView.lastIndexOf(2, 3)");
            verify(myVectorView.lastIndexOf(5, 4), myArray.lastIndexOf(5, 4), "myVectorView.lastIndexOf(5, 4)");
            verify(myVectorView.lastIndexOf(5, 2), myArray.lastIndexOf(5, 2), "myVectorView.lastIndexOf(5, 2)");
        }
    });

    runner.addTest({
        id: 18,
        desc: 'VectorViewArray_IsArray_Apply',
        pri: '0',
        test: function () {
            GetVectorView();

            verify(Array.isArray(myVectorView), false, "Array.isArray(myVectorView)");

            verify(Array.apply(this, myVectorView), Array.apply(this, myArray), "Array.apply(this, myVectorView)");
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);

            verify(new Array(myVectorView), new Array(myArray), "new Array(myVectorView)");
            verifyVectorAndArrayItems("myVectorView", myVectorView, myArray);
        }
    });

    runner.addTest({
        id: 19,
        desc: 'VectorViewArray_PropertyOperations',
        pri: '0',
        preReq: function () {
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            GetVectorView();

            // Check that we can enumerate over the members of the vector instance
            enumerateOver(myVectorView, "myVectorView", true);

            // Check that the property 3 exists and 11 doesnt
            checkHasProperty(myVectorView, 3, true);
            checkHasProperty(myVectorView, 11, false);

            setAndCheckProperty(myVectorView, "6", 88, false);
            setAndCheckProperty(myVectorView, "11", 11, false);
            setAndCheckProperty(myVectorView, "9", 111, false);

            var attributes = { writable: true, enumerable: true, configurable: true, value: 10 }
            addAndCheckProperty(myVectorView, "4", false, attributes);
            addAndCheckProperty(myVectorView, "10", false, attributes);
            addAndCheckProperty(myVectorView, "9", false, attributes);

            deleteAndCheckProperty(myVectorView, "4", false);

            // Should be able to add and manipulate expandos
            addAndCheckProperty(myVectorView, "FavoriteRecipe", false);
            setAndCheckProperty(myVectorView, "FavoriteRecipe", "Almond Cake", false);
        }
    });


    runner.addTest({
        id: 20,
        desc: 'VectorViewArray_Prototype',
        pri: '0',
        test: function () {
            GetVectorView();
            dumpVector(myVectorView, "myVectorView");

            logger.comment("var mySecondVector = myAnimal.getVector()");
            var mySecondVector = myAnimal.getVector();
            logger.comment("var mySecondVectorView = mySecondVector.getView();");
            var mySecondVectorView = mySecondVector.getView();

            verifyArrayPrototypeInChain(myVectorView, "myVectorView", mySecondVectorView, "mySecondVectorView");
        }
    });

    Loader42_FileName = 'Collections Vector View test';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
