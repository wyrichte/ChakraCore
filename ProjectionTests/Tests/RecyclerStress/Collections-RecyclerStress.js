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
        verify(myVector.length, myArray.length, myVectorString + ".length");

        verify(myVector.toString(), myArray.toString(), myVectorString);
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

    var errorObjectMemberExpected = 'Error: Object member expected';
    var errorObjectDoesntSupportAction = 'Error: Object doesn\'t support this action';
    var errorArrayLengthShouldBeFinite = 'Error: Array length must be assigned a finite positive integer';
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
        logger.comment("myVector = myAnimal.getVector()");
        myVector = myAnimal.getVector();
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
        desc: 'MarshalVectorOut',
        pri: '0',
        test: function () {
            GetVector();
            verifyVectorObjectDump("myVector", myVector, myArray, vectorMembers);
        }
    });

    runner.addTest({
        id: 2,
        desc: 'VectorArray_Index_length',
        pri: '0',
        test: function () {
            GetVector();
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            logger.comment("myVector[2] = 78;");
            myVector[2] = 78;
            myArray[2] = 78;
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            logger.comment("myVector.length = 5;");
            myVector.length = 5;
            myArray.length = 5;
            verifyVectorAndArrayItems("myVector", myVector, myArray);
        }
    });

    runner.addTest({
        id: 3,
        desc: 'VectorAsArray_Push_Pop',
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
        }
    });

    runner.addTest({
        id: 4,
        desc: 'VectorAsArray_Shift_Unshift',
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
        }
    });

    runner.addTest({
        id: 5,
        desc: 'VectorAsArray_Slice',
        pri: '0',
        test: function () {
            GetVector();
            myArray = [0, 22, 33, 44, 55, 66, 77, 11, 22, 33, 44, 55, 66, 100, 100];
            ApplyArrayToVector(myArray);

            verify(myVector.slice(), myArray.slice(), 'myVector.slice()');
            verifyVectorAndArrayItems("myVector", myVector, myArray);
        }
    });

    runner.addTest({
        id: 6,
        desc: 'VectorAsArray_Splice',
        pri: '0',
        test: function () {
            GetVector();
            myArray = [0, 22, 33, 44, 55, 66, 77, 11, 22, 33, 44, 55, 66, 100, 100];
            ApplyArrayToVector(myArray);

            verify(myVector.splice(), myArray.splice(), "myVector.splice()");
            verifyVectorAndArrayItems("myVector", myVector, myArray);
        }
    });

    runner.addTest({
        id: 7,
        desc: 'VectorAsArray_Every',
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
        desc: 'VectorAsArray_Reverse_Sort',
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
        }
    });

    runner.addTest({
        id: 9,
        desc: 'VectorAsArray_Concat',
        pri: '0',
        test: function () {
            GetVector();
            myArray = [98, 66, 55, 55, 54, 44, 33, 22, 22, 14, 11, 100, 100, 100, 100, 0];
            ApplyArrayToVector(myArray);

            verify(myVector.concat([1, 2, 3]), [myVector, 1, 2, 3], "myVector.concat([1,2,3])");
            verifyVectorAndArrayItems("myVector", myVector, myArray);
        }
    });

    runner.addTest({
        id: 10,
        desc: 'VectorAsArray_Join',
        pri: '0',
        test: function () {
            GetVector();
            myArray = [98, 66, 55, 55, 54, 44, 33, 22, 22, 14, 11, 100, 100, 100, 100, 0];
            ApplyArrayToVector(myArray);

            verify(myVector.join(' , '), myArray.join(' , '), "myVector.join(' , ')");
            verifyVectorAndArrayItems("myVector", myVector, myArray);
        }
    });

    runner.addTest({
        id: 11,
        desc: 'VectorAsArray_IndexOf',
        pri: '0',
        test: function () {
            GetVector();
            myArray = [98, 66, 55, 55, 54, 44, 33, 22, 22, 14, 11, 100, 100, 100, 100, 0];
            ApplyArrayToVector(myArray);

            // Old test cases for Array.prototype.indexOf (shadowed by camel cased IVector.indexOf)
            verify(Array.prototype.indexOf.call(myVector, 55), myArray.indexOf(55), "Array.prototype.indexOf.call(myVector, 55)");

            // New cases for IVector.indexOf
            verify(myVector.indexOf(55).index, myArray.indexOf(55), "myVector.indexOf(55)");
        }
    });

    runner.addTest({
        id: 12,
        desc: 'VectorArray_IsArray_Apply',
        pri: '0',
        test: function () {
            GetVector();
            myArray = [98, 66, 55, 55, 54, 44, 33, 22, 22, 14, 11, 100, 100, 100, 100, 0];
            ApplyArrayToVector(myArray);

            verify(Array.isArray(myVector), false, "Array.isArray(myVector)");

            verify(Array.apply(this, myVector), Array.apply(this, myArray), "Array.apply(this, myVector)");
            verifyVectorAndArrayItems("myVector", myVector, myArray);
        }
    });

    runner.addTest({
        id: 13,
        desc: 'VectorArray_PropertyOperations',
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

            // Should not be able to add and manipulate expandos (non-extensible)
            addAndCheckProperty(myVector, "FavoriteRecipe", false);
            setAndCheckProperty(myVector, "FavoriteRecipe", "Almond Cake", false);
        }
    });

    Loader42_FileName = 'Recycler Stress - Collections.js';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
