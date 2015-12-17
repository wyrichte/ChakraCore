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

    function verifyVectorAndArrayItems(myVectorString, myVector, myArray) {
        dumpVector(myVector, myVectorString);
        dumpVector(myArray, "myArray");
        verify(myVector.length, myArray.length, myVectorString + ".length")

        for (var iIndex = 0; iIndex < myVector.length; iIndex++) {
            verify(myVector[iIndex], myArray[iIndex], myVectorString + '[' + iIndex + ']');
        }
    }

    function assertException(expression, exceptionDescription) {
        var fExceptionFound = false;
        try {
            logger.comment(expression);
            eval(expression);
        }
        catch (e) {
            verify(e.toString(), exceptionDescription, 'e.toString()');
            fExceptionFound = true;
        }

        assert(fExceptionFound, 'Expected exception was caught');
    }

    var animalFactory;
    var myAnimal;
    var myVector;
    var myArray;

    // name, type, length if function
    var vectorMembers = [
    ['addEventListener', 'function', 2],
    ['append', 'function', 1],
    ['clear', 'function', 0],
    ['first', 'function', 0],
    ['getAt', 'function', 1],
    ['getMany', 'function', 2],
    ['getView', 'function', 0],
    ['indexOf', 'function', 1],
    ['insertAt', 'function', 2],
    ['onvectorchanged', 'object'],
    ['removeAt', 'function', 1],
    ['removeAtEnd', 'function', 0],
    ['removeEventListener', 'function', 2],
    ['replaceAll', 'function', 1],
    ['setAt', 'function', 2],
    ['size', 'number'],
    ];

    var errorObjectMemberExpected = 'Error: Object member expected';
    var errorObjectDoesntSupportAction = 'Error: Object doesn\'t support this action';
    var errorArrayLengthShouldBeFinite = 'RangeError: Array length must be assigned a finite positive integer';
    var errorTypeObjectDoesntSupportAction = 'TypeError: Object doesn\'t support this action';

    function IsLessThan50(x) {
        if (x < 50) {
            return true;
        }
        return false;
    }

    function IsLessThan70(x) {
        if (x < 70) {
            return true;
        }
        return false;
    }

    function IsLessThan101(x) {
        if (x < 101) {
            return true;
        }
        return false;
    }

    function IsGreaterThan100(x) {
        if (x > 100) {
            return true;
        }
        return false;
    }

    runner.globalSetup(function () {
        logger.comment("animalFactory = Animals.Animal");
        animalFactory = Animals.Animal;

        logger.comment("myAnimal = new animalFactory(1)");
        myAnimal = new animalFactory(1);

        logger.comment("\nfunction IsLessThan50(x)\n{\n    if (x < 50)\n    {\n        return true;\n    }\n    return false;\n}");
        logger.comment("\nfunction IsLessThan70(x)\n{\n    if (x < 70)\n    {\n        return true;\n    }\n    return false;\n}");
        logger.comment("\nfunction IsLessThan101(x)\n{\n    if (x < 101)\n    {\n        return true;\n    }\n    return false;\n}");
        logger.comment("\nfunction IsGreaterThan100(x)\n{\n    if (x > 100)\n    {\n        return true;\n    }\n    return false;\n}");
    });

    function GetVector() {
        logger.comment("myVector = new Animals.RCIObservable();");
        myVector = new Animals.RCIObservable();
        myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
    }

    function ApplyArrayToVector(arrayToApply) {
        logger.comment("Applying Array [" + arrayToApply + "] to myVector");
        myVector.length = 0;
        var iIndex = 0;
        while (iIndex < arrayToApply.length) {
            myVector[iIndex] = arrayToApply[iIndex];
            iIndex++;
        }
        dumpVector(myVector, "myVector");
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

        if (inChain === false) {
            verify(mySecondVector.arrayTestProperty, "Object's Array Test Property", mySecondVectorString + ".arrayTestProperty");
        }
        else {
            verify(mySecondVector.arrayTestProperty, "Array's Array Test Property", mySecondVectorString + ".arrayTestProperty");
        }
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
        desc: 'MarshalRCObservableVector',
        pri: '0',
        test: function () {
            GetVector();
            verifyVectorObjectDump("myVector", myVector, myArray, vectorMembers);
        }
    });

    runner.addTest({
        id: 2,
        desc: 'RCObservableVectorAsArray_Index_length',
        pri: '0',
        test: function () {
            GetVector();
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            logger.comment("myVector[2] = 78;");
            myVector[2] = 78;
            myArray[2] = 78;
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            logger.comment("myVector[9] = 10;");
            myVector[9] = 10;
            myArray[9] = 10;
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            logger.comment("var n = delete myVector[5]");
            var n = delete myVector[5];
            verify(n, false, "n");
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            logger.comment("n = delete myVector[12]");
            n = delete myVector[12];
            verify(n, false, "n");
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            logger.comment("n = delete myVector");
            n = delete myVector;
            verify(n, false, "n");
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            logger.comment("myVector[12] = 12");
            myVector[12] = 12;
            verifyVectorAndArrayItems("myVector", myVector, myArray);
            verify(myVector[12], undefined, "myVector[12]");

            assertException("myVector.length = 11", errorObjectDoesntSupportAction);
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            logger.comment("myVector.length = 5;");
            myVector.length = 5;
            myArray.length = 5;
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            assertException("myVector.length = 30;", errorObjectDoesntSupportAction);
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            assertException("myVector.length = 'abc';", errorArrayLengthShouldBeFinite);
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            logger.comment("myVector.length = 0;");
            myVector.length = 0;
            myArray.length = 0;
            verifyVectorAndArrayItems("myVector", myVector, myArray);
        }
    });

    runner.addTest({
        id: 3,
        desc: 'RCObservableVectorAsArray_Push_Pop',
        pri: '0',
        test: function () {
            GetVector();
            myArray = [];
            ApplyArrayToVector(myArray);

            logger.comment("var n = myVector.pop()")
            var n = myVector.pop()
            verify(n, undefined, "n");
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            for (var iIndex = 1; iIndex < 8; iIndex++) {
                verify(myVector.push(iIndex * 11), myArray.push(iIndex * 11), "myVector.push(" + (iIndex * 11) + ")");
            }
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            verify(myVector.push(11, 22, 33, 44, 55, 66), myArray.push(11, 22, 33, 44, 55, 66), 'myVector.push(11, 22, 33, 44, 55, 66)');
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            verify(Array.prototype.push.call(myVector, 100), Array.prototype.push.call(myArray, 100), 'Array.prototype.push.call(myVector, 100)');
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            assertException("n = myVector.pop()", errorTypeObjectDoesntSupportAction);
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            assertException("n = Array.prototype.pop.call(myVector);", errorTypeObjectDoesntSupportAction);
            verifyVectorAndArrayItems("myVector", myVector, myArray);
        }
    });

    runner.addTest({
        id: 4,
        desc: 'RCObservableVectorAsArray_Shift_Unshift',
        pri: '0',
        test: function () {
            GetVector();
            myArray = [11, 22, 33, 44, 55, 66, 77, 11, 22, 33, 44, 55, 66, 100];
            ApplyArrayToVector(myArray);

            assertException("myVector.shift()", errorTypeObjectDoesntSupportAction);
            myArray = [22, 33, 44, 55, 66, 77, 11, 22, 33, 44, 55, 66, 100, 100];
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            verify(myVector.unshift(), myArray.unshift(), "myVector.unshift()");
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            verify(myVector.unshift(0), myArray.unshift(0), "myVector.unshift(0)");
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            assertException("myVector.unshift(25, 50, 100)", errorTypeObjectDoesntSupportAction);
            verifyVectorAndArrayItems("myVector", myVector, myArray);
        }
    });

    runner.addTest({
        id: 5,
        desc: 'RCObservableVectorAsArray_Slice',
        pri: '0',
        test: function () {
            GetVector();
            myArray = [0, 22, 33, 44, 55, 66, 77, 11, 22, 33, 44, 55, 66, 100, 100];
            ApplyArrayToVector(myArray);

            verify(myVector.slice(), myArray.slice(), 'myVector.slice()');
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            verify(myVector.slice(5, 2), myArray.slice(5, 2), "myVector.slice(5, 2)");
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            verify(myVector.slice(5, 8), myArray.slice(5, 8), 'myVector.slice(5, 8)');
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            verify(myVector.slice(5), myArray.slice(5), "myVector.slice(5)");
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            verify(myVector.slice(-2), myArray.slice(-2), "myVector.slice(-2)");
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            verify(myVector.slice(-7, -2), myArray.slice(-7, -2), "myVector.slice(-7, -2)");
            verifyVectorAndArrayItems("myVector", myVector, myArray);
        }
    });

    runner.addTest({
        id: 6,
        desc: 'RCObservableVectorAsArray_Splice',
        pri: '0',
        test: function () {
            GetVector();
            myArray = [0, 22, 33, 44, 55, 66, 77, 11, 22, 33, 44, 55, 66, 100, 100];
            ApplyArrayToVector(myArray);

            verify(myVector.splice(), myArray.splice(), "myVector.splice()");
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            verify(myVector.splice(5, 0), myArray.splice(5, 0), "myVector.splice(5, 0)");
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            assertException("myVector.splice(5, 2)", errorTypeObjectDoesntSupportAction);
            myArray = [0, 22, 33, 44, 55, 11, 22, 33, 44, 55, 66, 100, 100, 100, 100];
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            verify(myVector.splice(2, 2, 14, 98, 54), myArray.splice(2, 2, 14, 98, 54), "myVector.splice(2, 2, 14, 98, 54)");
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            assertException("myVector.splice(4, 0, 32, 83)", errorTypeObjectDoesntSupportAction);
            verifyVectorAndArrayItems("myVector", myVector, myArray);
        }
    });

    runner.addTest({
        id: 7,
        desc: 'RCObservableVectorAsArray_Every',
        pri: '0',
        test: function () {
            GetVector();
            myArray = [0, 22, 14, 98, 54, 55, 11, 22, 33, 44, 55, 66, 100, 100, 100, 100];
            ApplyArrayToVector(myArray);

            verify(myVector.every(IsLessThan70), myArray.every(IsLessThan70), 'myVector.every(IsLessThan70)');
            verify(myVector.every(IsLessThan101), myArray.every(IsLessThan101), "myVector.every(IsLessThan101)");
        }
    });

    runner.addTest({
        id: 8,
        desc: 'RCObservableVectorAsArray_Some',
        pri: '0',
        test: function () {
            GetVector();
            myArray = [0, 22, 14, 98, 54, 55, 11, 22, 33, 44, 55, 66, 100, 100, 100, 100];
            ApplyArrayToVector(myArray);

            verify(myVector.some(IsLessThan70), myArray.some(IsLessThan70), "myVector.some(IsLessThan70)");
            verify(myVector.some(IsGreaterThan100), myArray.some(IsGreaterThan100), "myVector.some(IsGreaterThan100)");
        }
    });

    runner.addTest({
        id: 9,
        desc: 'RCObservableVectorAsArray_Filter',
        pri: '0',
        test: function () {
            GetVector();
            myArray = [0, 22, 14, 98, 54, 55, 11, 22, 33, 44, 55, 66, 100, 100, 100, 100];
            ApplyArrayToVector(myArray);

            verify(myVector.filter(IsLessThan50), myArray.filter(IsLessThan50), "myVector.filter(IsLessThan50)");
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            verify(myVector.filter(IsGreaterThan100), myArray.filter(IsGreaterThan100), "myVector.filter(IsGreaterThan100)");
            verifyVectorAndArrayItems("myVector", myVector, myArray);
        }
    });

    runner.addTest({
        id: 10,
        desc: 'RCObservableVectorAsArray_ForEach',
        pri: '0',
        test: function () {
            GetVector();
            myArray = [0, 22, 14, 98, 54, 55, 11, 22, 33, 44, 55, 66, 100, 100, 100, 100];
            ApplyArrayToVector(myArray);

            logger.comment("\nfunction forEachMethod(x, idx)\n{\n    print('[' + idx + ']', '=', x);\n}");
            function forEachMethod(x, idx) {
                logger.comment('[' + idx + ']', '=', x);
            }

            logger.comment("myVector.forEach(forEachMethod);");
            myVector.forEach(forEachMethod);
            verifyVectorAndArrayItems("myVector", myVector, myArray);
        }
    });

    runner.addTest({
        id: 11,
        desc: 'RCObservableVectorAsArray_Map',
        pri: '0',
        test: function () {
            GetVector();
            myArray = [0, 22, 14, 98, 54, 55, 11, 22, 33, 44, 55, 66, 100, 100, 100, 100];
            ApplyArrayToVector(myArray);

            logger.comment("\nfunction mapMethod(x)\n{\n    if (IsLessThan50(x))\n    {\n        return x;\n    }\n    return -1;\n}");
            function mapMethod(x) {
                if (IsLessThan50(x)) {
                    return x;
                }
                return -1;
            }

            verify(myVector.map(mapMethod), myArray.map(mapMethod), "myVector.map(mapMethod)");
            verifyVectorAndArrayItems("myVector", myVector, myArray);
        }
    });

    runner.addTest({
        id: 12,
        desc: 'RCObservableVectorAsArray_Reduce_ReduceRight',
        pri: '0',
        test: function () {
            GetVector();
            myArray = [0, 22, 14, 98, 54, 55, 11, 22, 33, 44, 55, 66, 100, 100, 100, 100];
            ApplyArrayToVector(myArray);

            logger.comment("\nfunction doTotal(a, b)\n{\n    return a + b;\n}");
            function doTotal(a, b) {
                return a + b;
            }

            verify(myVector.reduce(doTotal), myArray.reduce(doTotal), "myVector.reduce(doTotal)");
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            verify(myVector.reduceRight(doTotal), myArray.reduceRight(doTotal), "myVector.reduceRight(doTotal)");
            verifyVectorAndArrayItems("myVector", myVector, myArray);
        }
    });

    runner.addTest({
        id: 13,
        desc: 'RCObservableVectorAsArray_Reverse_Sort',
        pri: '0',
        test: function () {
            GetVector();
            myArray = [0, 22, 14, 98, 54, 55, 11, 22, 33, 44, 55, 66, 100, 100, 100, 100];
            ApplyArrayToVector(myArray);

            logger.comment("var a = myVector.sort();");
            var a = myVector.sort();
            assert(a === myVector, "a === myVector");
            myArray = [0, 100, 100, 100, 100, 11, 14, 22, 22, 33, 44, 54, 55, 55, 66, 98];
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            logger.comment("a = myVector.reverse();");
            a = myVector.reverse();
            assert(a === myVector, "a === myVector");
            myArray.reverse();
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            logger.comment("a = Array.prototype.sort.call(myVector)");
            a = Array.prototype.sort.call(myVector);
            assert(a === myVector, "a === myVector");
            myArray = [0, 100, 100, 100, 100, 11, 14, 22, 22, 33, 44, 54, 55, 55, 66, 98];
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            logger.comment("a = Array.prototype.reverse.call(myVector);");
            a = Array.prototype.reverse.call(myVector);
            assert(a === myVector, "a === myVector");
            Array.prototype.reverse.call(myArray);
            verifyVectorAndArrayItems("myVector", myVector, myArray);
        }
    });

    runner.addTest({
        id: 14,
        desc: 'RCObservableVectorAsArray_Concat',
        pri: '0',
        test: function () {
            GetVector();
            myArray = [98, 66, 55, 55, 54, 44, 33, 22, 22, 14, 11, 100, 100, 100, 100, 0];
            ApplyArrayToVector(myArray);

            verify(myVector.concat([1, 2, 3]), [myVector, 1, 2, 3], "myVector.concat([1,2,3])");
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            logger.comment("var mySecondVector = new Animals.RCIObservable();");
            var mySecondVector = new Animals.RCIObservable();
            var mySecondArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            verifyVectorAndArrayItems("mySecondVector", mySecondVector, mySecondArray);

            verify(myVector.concat(mySecondVector), [myVector, mySecondVector], "myVector.concat(mySecondVector)");
            verifyVectorAndArrayItems("myVector", myVector, myArray);
            verifyVectorAndArrayItems("mySecondVector", mySecondVector, mySecondArray);

            logger.comment("var jsArray = [1,2,3];");
            var jsArray = [1, 2, 3];
            verify(jsArray.concat(myVector), [1, 2, 3, myVector], "jsArray.concat(myVector)");
            verifyVectorAndArrayItems("myVector", myVector, myArray);
            verify(jsArray, [1, 2, 3], "jsArray");

            verify(myVector.concat(1, 2, 3, 4), [myVector, 1, 2, 3, 4], "myVector.concat(1,2,3,4)");
            verifyVectorAndArrayItems("myVector", myVector, myArray);
        }
    });

    runner.addTest({
        id: 15,
        desc: 'RCObservableVectorAsArray_Join',
        pri: '0',
        test: function () {
            GetVector();
            myArray = [98, 66, 55, 55, 54, 44, 33, 22, 22, 14, 11, 100, 100, 100, 100, 0];
            ApplyArrayToVector(myArray);

            verify(myVector.join(' , '), myArray.join(' , '), "myVector.join(' , ')");
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            verify(myVector.join([9, 'b', 'c']), myArray.join([9, 'b', 'c']), "myVector.join([9,'b','c'])");
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            logger.comment("var mySecondVector = new Animals.RCIObservable();");
            var mySecondVector = new Animals.RCIObservable();
            var mySecondArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            verifyVectorAndArrayItems("mySecondVector", mySecondVector, mySecondArray);

            verify(myVector.join(mySecondVector), myArray.join(mySecondVector), "myVector.join(mySecondVector)");
            verifyVectorAndArrayItems("myVector", myVector, myArray);
            verifyVectorAndArrayItems("mySecondVector", mySecondVector, mySecondArray);

            logger.comment("var jsArray = [1,2,3];");
            var jsArray = [1, 2, 3];
            verify(jsArray.join(myVector), jsArray.join(myArray), "jsArray.join(myVector)");
            verifyVectorAndArrayItems("myVector", myVector, myArray);
            verify(jsArray, [1, 2, 3], "jsArray");
        }
    });

    runner.addTest({
        id: 16,
        desc: 'RCObservableVectorAsArray_IndexOf',
        pri: '0',
        test: function () {
            GetVector();
            myArray = [98, 66, 55, 55, 54, 44, 33, 22, 22, 14, 11, 100, 100, 100, 100, 0];
            ApplyArrayToVector(myArray);

            // Old test cases for Array.prototype.indexOf (shadowed by camel cased IVector.indexOf)
            verify(Array.prototype.indexOf.call(myVector, 55), myArray.indexOf(55), "Array.prototype.indexOf.call(myVector, 55)");
            verify(Array.prototype.indexOf.call(myVector, 786), myArray.indexOf(786), "Array.prototype.indexOf.call(myVector, 786)");
            verify(Array.prototype.indexOf.call(myVector, 55, 5), myArray.indexOf(55, 5), "Array.prototype.indexOf.call(myVector, 55, 5)");
            verify(Array.prototype.indexOf.call(myVector, 22, 8), myArray.indexOf(22, 8), "Array.prototype.indexOf.call(myVector, 22, 8)");
            verify(Array.prototype.indexOf.call(myVector, 22, 7), myArray.indexOf(22, 7), "Array.prototype.indexOf.call(myVector, 22, 7)");

            // New cases for IVector.indexOf
            verify(myVector.indexOf(55).index, myArray.indexOf(55), "myVector.indexOf(55)");
            verify(myVector.indexOf(786).returnValue, false, "myVector.indexOf(786)");
            verify(myVector.indexOf(22).index, myArray.indexOf(22), "myVector.indexOf(22, 8)");
        }
    });

    runner.addTest({
        id: 17,
        desc: 'RCObservableVectorAsArray_LastIndexOf',
        pri: '0',
        test: function () {
            GetVector();
            myArray = [98, 66, 55, 55, 54, 44, 33, 22, 22, 14, 11, 100, 100, 100, 100, 0];
            ApplyArrayToVector(myArray);

            verify(myVector.lastIndexOf(55), myArray.lastIndexOf(55), "myVector.lastIndexOf(55)");
            verify(myVector.lastIndexOf(786), myArray.lastIndexOf(786), "myVector.lastIndexOf(786)");
            verify(myVector.lastIndexOf(22, 3), myArray.lastIndexOf(22, 3), "myVector.lastIndexOf(22, 3)");
            verify(myVector.lastIndexOf(22, 8), myArray.lastIndexOf(22, 8), "myVector.lastIndexOf(22, 8)");
            verify(myVector.lastIndexOf(22, 7), myArray.lastIndexOf(22, 7), "myVector.lastIndexOf(22, 7)");
        }
    });

    runner.addTest({
        id: 18,
        desc: 'RCObservableVectorAsArray_IsArray_Apply',
        pri: '0',
        test: function () {
            GetVector();
            myArray = [98, 66, 55, 55, 54, 44, 33, 22, 22, 14, 11, 100, 100, 100, 100, 0];
            ApplyArrayToVector(myArray);

            verify(Array.isArray(myVector), false, "Array.isArray(myVector)");

            verify(Array.apply(this, myVector), Array.apply(this, myArray), "Array.apply(this, myVector)");
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            verify(new Array(myVector), new Array(myArray), "new Array(myVector)");
            verifyVectorAndArrayItems("myVector", myVector, myArray);
        }
    });

    runner.addTest({
        id: 19,
        desc: 'RCObservableVectorAsArray_PropertyOperations',
        pri: '0',
        test: function () {
            GetVector();
            myArray = [98, 66, 55, 55, 54, 44, 33, 22, 22, 14, 11, 100, 100, 100, 100, 0];
            ApplyArrayToVector(myArray);

            // Check that we can enumerate over the members of the vector instance
            enumerateOver(myVector, "myVector", true);

            // Check that the property 3 exists and 11 doesnt
            checkHasProperty(myVector, 3, true);
            checkHasProperty(myVector, 18, false);

            setAndCheckProperty(myVector, "6", 88, true);
            setAndCheckProperty(myVector, "18", 11, false);
            setAndCheckProperty(myVector, "16", 111, true);

            var attributes = { writable: true, enumerable: true, configurable: true, value: 10 }
            addAndCheckProperty(myVector, "4", false, attributes);
            addAndCheckProperty(myVector, "17", false, attributes);
            addAndCheckProperty(myVector, "20", false, attributes);

            deleteAndCheckProperty(myVector, "4", false);
            deleteAndCheckProperty(myVector, "10", false);

            // Should not be able to add and manipulate expandos
            addAndCheckProperty(myVector, "FavoriteRecipe", false);
            setAndCheckProperty(myVector, "FavoriteRecipe", "Almond Cake", false);
        }
    });


    runner.addTest({
        id: 20,
        desc: 'RCObservableVectorAsArray_Prototype',
        pri: '0',
        test: function () {
            GetVector();
            dumpVector(myVector, "myVector");

            logger.comment("var mySecondVector = new Animals.RCIObservable()");
            var mySecondVector = new Animals.RCIObservable();
            verifyArrayPrototypeInChain(myVector, "myVector", mySecondVector, "mySecondVector");
        }
    });

    runner.addTest({
        id: 21,
        desc: 'RCObservableVectorAsArray_Events',
        pri: '0',
        test: function () {
            GetVector();
            dumpVector(myVector, "myVector");
            verifyVectorAndArrayItems("myVector", myVector, [1, 2, 3, 4, 5, 6, 7, 8, 9]);

            function vectorChanged(ev) {
                logger.comment("*** ObservableVector Change invoke");

                vectorChanged.myObservableVector = ev.target;
                vectorChanged.vectorChangedArgs = ev.detail[0];

                logger.comment("    newObservableVector: " + ev.target);

                logger.comment("    vectorChangedArgs.index: " + ev.index);
                logger.comment("    vectorChangedArgs.collectionChange: " + ev.collectionChange);

                logger.comment("*** ObservableVector Change End");
            }

            // Assign the event handler and get no error.
            logger.comment("myVector.addEventListener('vectorchanged', vectorChanged);");
            myVector.addEventListener('vectorchanged', vectorChanged);

            logger.comment("myVector[5] = 88;");
            myVector[5] = 88;

            logger.comment("Verifying results that were received in delegate");
            verifyVectorAndArrayItems('vectorChanged.myObservableVector', vectorChanged.myObservableVector, [1, 2, 3, 4, 5, 88, 7, 8, 9]);
            verify(vectorChanged.vectorChangedArgs.index, 5, "vectorChanged.vectorChangedArgs.index");
            verify(vectorChanged.vectorChangedArgs.collectionChange, 3, "vectorChanged.vectorChangedArgs.collectionChange");
            verifyVectorAndArrayItems("myVector", myVector, [1, 2, 3, 4, 5, 88, 7, 8, 9]);

            logger.comment("myVector.insertAt(5, 1800);");
            myVector.insertAt(5, 1800);

            logger.comment("Verifying results that were received in delegate");
            verifyVectorAndArrayItems('vectorChanged.myObservableVector', vectorChanged.myObservableVector, [1, 2, 3, 4, 5, 1800, 88, 7, 8, 9]);
            verify(vectorChanged.vectorChangedArgs.index, 5, "vectorChanged.vectorChangedArgs.index");
            verify(vectorChanged.vectorChangedArgs.collectionChange, 1, "vectorChanged.vectorChangedArgs.collectionChange");
            verifyVectorAndArrayItems("myVector", myVector, [1, 2, 3, 4, 5, 1800, 88, 7, 8, 9]);

            logger.comment("myVector.removeAt(5)");
            myVector.removeAt(5);

            logger.comment("Verifying results that were received in delegate");
            verifyVectorAndArrayItems('vectorChanged.myObservableVector', vectorChanged.myObservableVector, [1, 2, 3, 4, 5, 88, 7, 8, 9]);
            verify(vectorChanged.vectorChangedArgs.index, 5, "vectorChanged.vectorChangedArgs.index");
            verify(vectorChanged.vectorChangedArgs.collectionChange, 2, "vectorChanged.vectorChangedArgs.collectionChange");
            verifyVectorAndArrayItems("myVector", myVector, [1, 2, 3, 4, 5, 88, 7, 8, 9]);

            logger.comment("myVector.append(1800);");
            myVector.append(1800);

            logger.comment("Verifying results that were received in delegate");
            verifyVectorAndArrayItems('vectorChanged.myObservableVector', vectorChanged.myObservableVector, [1, 2, 3, 4, 5, 88, 7, 8, 9, 1800]);
            verify(vectorChanged.vectorChangedArgs.index, 9, "vectorChanged.vectorChangedArgs.index");
            verify(vectorChanged.vectorChangedArgs.collectionChange, 1, "vectorChanged.vectorChangedArgs.collectionChange");
            verifyVectorAndArrayItems("myVector", myVector, [1, 2, 3, 4, 5, 88, 7, 8, 9, 1800]);

            logger.comment("myVector.removeAtEnd()");
            myVector.removeAtEnd();

            logger.comment("Verifying results that were received in delegate");
            verifyVectorAndArrayItems('vectorChanged.myObservableVector', vectorChanged.myObservableVector, [1, 2, 3, 4, 5, 88, 7, 8, 9]);
            verify(vectorChanged.vectorChangedArgs.index, 9, "vectorChanged.vectorChangedArgs.index");
            verify(vectorChanged.vectorChangedArgs.collectionChange, 2, "vectorChanged.vectorChangedArgs.collectionChange");
            verifyVectorAndArrayItems("myVector", myVector, [1, 2, 3, 4, 5, 88, 7, 8, 9]);

            logger.comment("myVector.clear()");
            myVector.clear();

            logger.comment("Verifying results that were received in delegate");
            verifyVectorAndArrayItems('vectorChanged.myObservableVector', vectorChanged.myObservableVector, []);
            verify(vectorChanged.vectorChangedArgs.index, 0, "vectorChanged.vectorChangedArgs.index");
            verify(vectorChanged.vectorChangedArgs.collectionChange, 0, "vectorChanged.vectorChangedArgs.collectionChange");
            verifyVectorAndArrayItems("myVector", myVector, []);

            logger.comment("myVector.replaceAll([50, 100, 150])");
            myVector.replaceAll([50, 100, 150]);

            logger.comment("Verifying results that were received in delegate");
            verifyVectorAndArrayItems('vectorChanged.myObservableVector', vectorChanged.myObservableVector, [50, 100, 150]);
            if (typeof Animals._CLROnly === 'undefined') //native implemenation
            {
                verify(vectorChanged.vectorChangedArgs.index, 0, "vectorChanged.vectorChangedArgs.index");
                verify(vectorChanged.vectorChangedArgs.collectionChange, 0, "vectorChanged.vectorChangedArgs.collectionChange");
            }
            else
            {
                verify(vectorChanged.vectorChangedArgs.index, 2, "vectorChanged.vectorChangedArgs.index");
                verify(vectorChanged.vectorChangedArgs.collectionChange, 1, "vectorChanged.vectorChangedArgs.collectionChange");
            }
            verifyVectorAndArrayItems('myVector', myVector, [50, 100, 150]);
        }
    });

    Loader42_FileName = 'RCObservable Vector Collections test';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
