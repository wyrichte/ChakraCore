if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    // mock CanvasPixelArray
    if (typeof WScript === 'undefined') {
        WScript = {};
        runner.subscribe('end', function () {
            delete WScript;
            WScript = undefined;
        });
    }
    if (typeof WScript.CreateCanvasPixelArray === 'undefined') {
        WScript.CreateCanvasPixelArray = function (arr) {
            var canvas = document.createElement('canvas');
            var context = canvas.getContext("2d");
            var imageData = context.getImageData(0, 0, 1, arr.length / 4);
            for (var i = 0; i < arr.length; i++) {
                imageData.data[i] = arr[i];
            }
            var pixelArray = imageData.data;
            return pixelArray;
        }
        runner.subscribe('end', function () {
            if (typeof WScript !== 'undefined'
                && typeof WScript.CreateCanvasPixelArray !== 'undefined') {
                delete WScript.CreateCanvasPixelArray;
                WScript.CreateCanvasPixelArray = undefined;
            }
        });
    }
    /////

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

    function verifyVectorAndArrayItems(myVectorString, myVector, myArrayString, myArray) {
        if (myArray == null) {
            verify(myVector, myArray, myVectorString);
        }
        else {
            verify(myVector.length, myArray.length, "Number of array Items and Vector Items")

            for (var iIndex = 0; iIndex < myVector.length; iIndex++) {
                verify(myVector[iIndex], myArray[iIndex], myVectorString + '[' + iIndex + '] and ' + myArrayString + '[' + iIndex + ']');
            }
        }
    }

    function verifyVectorAndStringArrayItems(myVectorString, myVector, myArrayString, myArray) {
        verify(myVector.length, myArray.length, "Number of array Items and Vector Items")

        for (var iIndex = 0; iIndex < myVector.length; iIndex++) {
            verify(myVector[iIndex], myArray[iIndex].toString(), myVectorString + '[' + iIndex + '] and ' + myArrayString + '[' + iIndex + '].toString()');
        }
    }

    function verifyResultObject(actualString, actual, expected, expectedType) {
        verify(typeof actual, expectedType, 'typeof ' + actualString);

        var i = 0;
        for (p in actual) {
            verify(p, expected[i][0], actualString + '.' + p);
            verify(typeof actual[p], expected[i][1], 'typeof ' + actualString + '.' + p);
            if (typeof actual[p] == 'function') {
                verify(actual[p].length, expected[i][2], actualString + '.' + p + ".length");
                logger.comment('Setting length of function to be 10');
                actual[p].length = 10;
                verify(actual[p].length, expected[i][2], actualString + '.' + p + ".length");
            }
            i++;
        }

        verify(i, expected.length, 'number of members');
    }

    function dumpArray(stringArray, myArray) {
        logger.comment(stringArray + "[" + myArray.length + "]: " + myArray);
        for (var i = 0; i < myArray.length; i++) {
            logger.comment(i + " = " + myArray[i]);
        }
    }

    function verifyArrayPrototypeInChain(arrayProjectionString, arrayProjection) {
        logger.comment('Array.prototype.arrayTestProperty = "Array\'s Array Test Property"');
        Array.prototype.arrayTestProperty = "Array's Array Test Property";
        logger.comment('Object.prototype.arrayTestProperty = "Object\'s Array Test Property";');
        Object.prototype.arrayTestProperty = "Object's Array Test Property";
        logger.comment('Object.prototype.objectTestProperty = "Object\'s Object Test Property";');
        Object.prototype.objectTestProperty = "Object's Object Test Property";

        verify(arrayProjection.arrayTestProperty, "Object's Array Test Property", arrayProjectionString + ".arrayTestProperty");
        verify(arrayProjection.objectTestProperty, "Object's Object Test Property", arrayProjectionString + ".objectTestProperty");

        logger.comment('delete Array.prototype.arrayTestProperty;');
        delete Array.prototype.arrayTestProperty;
        logger.comment('delete Object.prototype.arrayTestProperty;');
        delete Object.prototype.arrayTestProperty;
        logger.comment('delete Object.prototype.objectTestProperty;');
        delete Object.prototype.objectTestProperty;
    }

    function GetES5Array(arrayName, withValues) {
        var myArray;
        if (withValues == true) {
            logger.comment("var " + arrayName + " = [1, 2, 3, 4, 5, 6, 7, 8];");
            myArray = [1, 2, 3, 4, 5, 6, 7, 8];
        }
        else {
            logger.comment("var " + arrayName + " = new Array(9);");
            myArray = new Array(9);
        }

        logger.comment('Object.defineProperty(myArray, "8", { set: function (x) { value = x; }, get: function () { return value }, configurable: true });');
        Object.defineProperty(myArray, "8", { set: function (x) { value = x; }, get: function () { return value }, configurable: true });

        if (withValues == true) {
            logger.comment('myArray[8] = 9;');
            myArray[8] = 9;
        }

        return myArray;
    }

    function verifyCanvasPixelArray(pixelArray, expectedArray, stringPixelArray) {
        logger.comment(pixelArray);
        verify(pixelArray.length, expectedArray.length, stringPixelArray + ".length");
        for (var pixelIndex = 0; pixelIndex < expectedArray.length; pixelIndex++) {
            verify(pixelArray[pixelIndex], expectedArray[pixelIndex], stringPixelArray + "[" + pixelIndex + "]");
        }
    }

    function verifySameArray(arr1, arr2) {
        verify(arr1.length == arr2.length, true, "Expected same length: " + arr1.length + " vs " + arr2.length);
        for (var i = 0; i < arr1.length; ++i) {
            verify(arr1[i] == arr2[i], true, "Expected same element");
        }
    }

    function verifyArrayItems(myArrayString, myArray, myExpectedArray) {
        verify(myArray.length, myExpectedArray.length, myArrayString + ".length")

        for (var iIndex = 0; iIndex < myExpectedArray.length; iIndex++) {
            verify(myArray[iIndex], myExpectedArray[iIndex], myArrayString + '[' + iIndex + ']');
        }
    }

    function dumpArrayItems(myArrayString, myArray) {
        if (myArray === null) {
            logger.comment(myArrayString + " : null");
            return;
        }

        var arrayPrint = myArrayString + " : [ length = " + myArray.length + " ] : [ contents = ";
        for (var i = 0; i < myArray.length; i++) {
            arrayPrint = arrayPrint + " " + myArray[i];
        }
        arrayPrint = arrayPrint + " ]";
        logger.comment(arrayPrint);
    }

    function verifyAllZeroItems(myArrayString, myArray, expectedLength, expectedNull) {
        if (expectedNull === true) {
            verify(myArray, null, myArrayString);
            return;
        }

        verify(myArray.length, expectedLength, myArrayString + ".length")
        for (var iIndex = 0; iIndex < expectedLength; iIndex++) {
            verify(myArray[iIndex], 0, myArrayString + '[' + iIndex + ']');
        }
    }


    runner.addTest({
        id: 'fpa0',
        desc: 'Simple pass array of string',
        pri: '0',
        test: function () {

            var arr = ["a", "b", "c"];
            var r = new DevTests.Repros.Performance.RefClass();
            r.passRetrievableStringArray(arr);
            var result = r.retrievePassedStringArray();
            verifySameArray(arr, result);
        }
    });

    runner.addTest({
        id: 'fpa1',
        desc: 'Simple canvas pixel array as string array (is type mismatch)',
        pri: '0',
        test: function () {
            if (typeof WScript !== 'undefined') { // jshost
                var arr = WScript.CreateCanvasPixelArray([2, 3, 4, 5]);
                var r = new DevTests.Repros.Performance.RefClass();
                verify.exception(function () {
                    r.passRetrievableStringArray(arr);
                }, TypeError, "r.passRetrievableStringArray(arr);");
            }
        }
    });

    runner.addTest({
        id: 'fpa3',
        desc: 'Pass null to [in] array fastsig',
        pri: '0',
        test: function () {
            var arr = null;
            var r = new DevTests.Repros.Performance.RefClass();
            r.passRetrievableStringArray(arr);
            var result = r.retrievePassedStringArray();
            verify(result, null, "r.retrievePassedStringArray()");
        }
    });

    runner.addTest({
        id: 'fpa4',
        desc: 'Pass undefined to [in] array fastsig',
        pri: '0',
        test: function () {
            var arr = undefined;
            var r = new DevTests.Repros.Performance.RefClass();
            r.passRetrievableStringArray(arr);
            var result = r.retrievePassedStringArray();
            verify(result, null, "r.retrievePassedStringArray()");
        }
    });

    runner.addTest({
        id: 'fpa5',
        desc: 'Pass arrayProjection[string] to [in] array fastsig',
        pri: '0',
        test: function () {
            // Get typed array
            var myArray = ['white', 'Red', 'Blue', 'Yellow', 'Pink', 'Black', 'Orange', 'Purple', 'Green'];
            var myAnimal = new Animals.Animal(1);
            myAnimal.passArrayHSTRING(myArray);
            var arr = myAnimal.receiveArrayHSTRING().value;
            dumpArrayItems("arr", arr);
            var r = new DevTests.Repros.Performance.RefClass();
            r.passRetrievableStringArray(arr);
            var result = r.retrievePassedStringArray();
            verifySameArray(arr, result);
        }
    });

    runner.addTest({
        id: 'fpa6',
        desc: 'Pass TypedArray[int] to [in] array fastsig (is type mismatch)',
        pri: '0',
        test: function () {
            // Get typed array
            var myArray = [1, 2, 3, 4];
            var myAnimal = new Animals.Animal(1);
            myAnimal.purePassArray(myArray);
            var arr = myAnimal.pureReceiveArray();
            dumpArrayItems("arr", arr);
            var r = new DevTests.Repros.Performance.RefClass();
            verify.exception(function () {
                r.passRetrievableStringArray(arr);
            }, TypeError, "r.passRetrievableStringArray(arr)");
        }
    });

    runner.addTest({
        id: 'fpa7',
        desc: 'Pass ArrayProjection[Struct] to [in] array fastsig (is type mismatch)',
        pri: '0',
        test: function () {
            // Get typed array
            var arr = Animals.Animal.getPackedByteArray();
            dumpArrayItems("arr", arr);
            var r = new DevTests.Repros.Performance.RefClass();
            verify.exception(function () {
                r.passRetrievableStringArray(arr);
            }, TypeError, "r.passRetrievableStringArray(arr)");
        }
    });

    runner.addTest({
        id: 'fpa8',
        desc: 'Pass integer to [in] array fastsig (is type mismatch)',
        pri: '0',
        test: function () {
            var arr = 5;
            var r = new DevTests.Repros.Performance.RefClass();
            verify.exception(function () {
                r.passRetrievableStringArray(arr);
            }, TypeError, "r.passRetrievableStringArray(arr)");
        }
    });


    runner.addTest({
        id: 1,
        desc: 'MarshalInArray',
        pri: '0',
        test: function () {
            logger.comment("var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];");
            var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            logger.comment("var myAnimal = new Animals.Animal(1);");
            var myAnimal = new Animals.Animal(1);

            logger.comment("var myVector = myAnimal.passArray(myArray);");
            var myVector = myAnimal.passArray(myArray);
            logger.comment('verifyVectorAndArrayItems("myVector", myVector, "myArray", myArray)');
            verifyVectorAndArrayItems("myVector", myVector, "myArray", myArray)

            logger.comment('myArray = null');
            myArray = null;

            logger.comment("myVector = myAnimal.passArray(myArray);");
            myVector = myAnimal.passArray(myArray);
            logger.comment('verifyVectorAndArrayItems("myVector", myVector, "new Array()", new Array())');
            verifyVectorAndArrayItems("myVector", myVector, "new Array()", new Array())

            logger.comment("myVector = myAnimal.passArray(10);");

            var foundException = false;
            try {
                myVector = myAnimal.passArray(10);
            }
            catch (e) {
                verify.instanceOf(e, TypeError);
                verify(e.description, 'Array object expected', 'e.description');
                foundException = true;
            }

            assert(foundException, 'Expected exception was caught');
        }
    });

    runner.addTest({
        id: 2,
        desc: 'ProjectOutArrayByValue',
        pri: '0',
        test: function () {
            logger.comment("var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];");
            var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            logger.comment("var myAnimal = new Animals.Animal(1);");
            var myAnimal = new Animals.Animal(1);

            logger.comment("var myVector = myAnimal.passArray(myArray);");
            var myVector = myAnimal.passArray(myArray);
            logger.comment('verifyVectorAndArrayItems("myVector", myVector, "myArray", myArray)');
            verifyVectorAndArrayItems("myVector", myVector, "myArray", myArray);

            logger.comment("var myNewArray = new Array(9);");
            var myNewArray = new Array(9);
            logger.comment("myVector = myAnimal.fillArray(myNewArray);");
            myVector = myAnimal.fillArray(myNewArray);
            logger.comment('verifyVectorAndArrayItems("myNewArray", myNewArray, "myVector", myVector)');
            verifyVectorAndArrayItems("myNewArray", myNewArray, "myVector", myVector);
            verify(Array.isArray(myNewArray), true, "Array.isArray(myNewArray)");

            logger.comment("myNewArray = [11, 22, 33, 44];");
            myNewArray = [11, 22, 33, 44];
            logger.comment("myVector = myAnimal.fillArray(myNewArray);");
            myVector = myAnimal.fillArray(myNewArray);
            logger.comment('verifyVectorAndArrayItems("myNewArray", myNewArray, "myVector", myVector)');
            verifyVectorAndArrayItems("myNewArray", myNewArray, "myVector", myVector);
            verify(Array.isArray(myNewArray), true, "Array.isArray(myNewArray)");

            logger.comment("myNewArray.length = 0;");
            myNewArray.length = 0;
            logger.comment("myVector = myAnimal.fillArray(myNewArray);");
            myVector = myAnimal.fillArray(myNewArray);
            logger.comment('verifyVectorAndArrayItems("myNewArray", myNewArray, "myVector", myVector)');
            verifyVectorAndArrayItems("myNewArray", myNewArray, "myVector", myVector);
            verify(Array.isArray(myNewArray), true, "Array.isArray(myNewArray)");

            logger.comment("myVector = myAnimal.fillArray(10);");

            var foundException = false;
            try {
                myVector = myAnimal.fillArray(10);
            }
            catch (e) {
                verify.instanceOf(e, TypeError);
                verify(e.description, 'Array object expected', 'e.description');
                foundException = true;
            }

            assert(foundException, 'Expected exception was caught');
        }
    });

    runner.addTest({
        id: 3,
        desc: 'ProjectOutByRefArray_Basic',
        pri: '0',
        test: function () {
            logger.comment("var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];");
            var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            logger.comment("var myAnimal = new Animals.Animal(1);");
            var myAnimal = new Animals.Animal(1);

            logger.comment("var myVector = myAnimal.passArray(myArray);");
            var myVector = myAnimal.passArray(myArray);
            logger.comment('verifyVectorAndArrayItems("myVector", myVector, "myArray", myArray)');
            verifyVectorAndArrayItems("myVector", myVector, "myArray", myArray)

            logger.comment("var myResult = myAnimal.receiveArray();");
            var myResult = myAnimal.receiveArray();

            logger.comment('var expectedResult = [["value", "object"], ["outVector", "object"]];');
            var expectedResult = [["value", "object"], ["outVector", "object"]];

            logger.comment('verifyResultObject("myResult", myResult, expectedResult, "object");');
            verifyResultObject("myResult", myResult, expectedResult, "object");
            logger.comment('verifyVectorAndArrayItems("myResult.value", myResult.value, "myResult.outVector", myResult.outVector)');
            verifyVectorAndArrayItems("myResult.value", myResult.value, "myResult.outVector", myResult.outVector);

            logger.comment('var myArrayProjection = myResult.value;');
            var myArrayProjection = myResult.value;

            // Basic array tests
            verify(Array.isArray(myArrayProjection), false, "Array.isArray(myArrayProjection)");
            dumpArray("myArrayProjection", myArrayProjection);
            verifyArrayPrototypeInChain("myArrayProjection", myArrayProjection);

            logger.comment("var a = Array.apply(this, myArrayProjection)");
            var a = Array.apply(this, myArrayProjection);
            dumpArray("a", a);
            dumpArray("myArrayProjection", myArrayProjection);

            logger.comment("a = new Array(myArrayProjection)");
            a = new Array(myArrayProjection);
            dumpArray("a", a);
            dumpArray("myArrayProjection", myArrayProjection);

            // Verify length is non writable
            setAndCheckProperty(myArrayProjection, "length", 3, false);

            // Check that we can enumerate over the members of the vector instance
            enumerateOver(myArrayProjection, "ArrayProjection instance: myArrayProjection", true);

            // Check that the property 3 exists and 11 doesnt
            checkHasProperty(myArrayProjection, 3, true);
            checkHasProperty(myArrayProjection, 11, false);

            setAndCheckProperty(myArrayProjection, "6", 88, true);
            setAndCheckProperty(myArrayProjection, "11", 11, false);

            var attributes = { writable: true, enumerable: true, configurable: true, value: 10 }
            addAndCheckProperty(myArrayProjection, "4", false, attributes);
            addAndCheckProperty(myArrayProjection, "10", false, attributes);
            addAndCheckProperty(myArrayProjection, "14", false, attributes);

            deleteAndCheckProperty(myArrayProjection, "4", false);

            // Should be able to add and manipulate expandos
            addAndCheckProperty(myArrayProjection, "FavoriteRecipe", false);
            setAndCheckProperty(myArrayProjection, "FavoriteRecipe", "Almond Cake", false);

            // Check if we can marshal in the projected array as inout parameter:
            logger.comment("myVector = myAnimal.fillArray(myArrayProjection);");
            myVector = myAnimal.fillArray(myArrayProjection);
            logger.comment('verifyVectorAndArrayItems("myVector", myVector, "myArrayProjection", myArrayProjection)');
            verifyVectorAndArrayItems("myVector", myVector, "myArrayProjection", myArrayProjection)
            verify(Array.isArray(myArrayProjection), false, "Array.isArray(myArrayProjection)");
            dumpArray("myArrayProjection", myArrayProjection);
            verifyArrayPrototypeInChain("myArrayProjection", myArrayProjection);

            // Check if we can marshal in the projected array as in parameter:
            logger.comment("myArrayProjection[4] = 300");
            myArrayProjection[4] = 300
            logger.comment("myVector = myAnimal.passArray(myArrayProjection);");
            myVector = myAnimal.passArray(myArrayProjection);
            logger.comment('verifyVectorAndArrayItems("myVector", myVector, "myArrayProjection", myArrayProjection)');
            verifyVectorAndArrayItems("myVector", myVector, "myArrayProjection", myArrayProjection)

            // Verify marshalling into different type throws exception - in param
            verify.exception(function () {
                logger.comment("myVector = myAnimal.passArrayHSTRING(myArrayProjection);");
                myVector = myAnimal.passArrayHSTRING(myArrayProjection);
            }, TypeError, "myAnimal.passArrayHSTRING(myArrayProjection)");

            // Verify marshalling into different type throws exception - inout param
            verify.exception(function () {
                logger.comment("myVector = myAnimal.fillArrayHSTRING(myArrayProjection);");
                myVector = myAnimal.fillArrayHSTRING(myArrayProjection);
            }, TypeError, "myAnimal.fillArrayHSTRING(myArrayProjection)");
        }
    });

    runner.addTest({
        id: 4,
        desc: 'ProjectOutByRefArrayNull_Basic',
        pri: '0',
        test: function () {
            logger.comment("var myAnimal = new Animals.Animal(1);");
            var myAnimal = new Animals.Animal(1);

            // Null as array
            logger.comment("myResult = myAnimal.passArray(null);");
            myResult = myAnimal.passArray(null);

            logger.comment("myResult = myAnimal.receiveArray();");
            myResult = myAnimal.receiveArray();

            logger.comment('expectedResult = [["value", "object"], ["outVector", "object"]];');
            expectedResult = [["value", "object"], ["outVector", "object"]];
            logger.comment('verifyVectorAndArrayItems("myResult.value", myResult.value, "null", null)');
            verifyVectorAndArrayItems("myResult.value", myResult.value, "null", null);
        }
    });

    runner.addTest({
        id: 5,
        desc: 'Delegate_MarshalInArray_Basic',
        pri: '0',
        test: function () {
            function delegatePassArray(animal, intArray) {
                logger.comment("*** delegatePassArray Delegate");
                logger.comment("var intVector = delegatePassArray.outVector;");
                var intVector = delegatePassArray.outVector;
                logger.comment('verifyVectorAndArrayItems("intArray", intArray, "intVector", intVector)');
                verifyVectorAndArrayItems("intArray", intArray, "intVector", intVector)

                // Basic array tests
                verify(Array.isArray(intArray), false, "Array.isArray(intArray)");
                dumpArray("intArray", intArray);
                verifyArrayPrototypeInChain("intArray", intArray);

                logger.comment("var a = Array.apply(this, intArray)");
                var a = Array.apply(this, intArray);
                dumpArray("a", a);
                dumpArray("intArray", intArray);

                logger.comment("a = new Array(intArray)");
                a = new Array(intArray);
                dumpArray("a", a);
                dumpArray("intArray", intArray);

                // Verify length is non writable
                setAndCheckProperty(intArray, "length", 3, false);

                // Check that we can enumerate over the members of the vector instance
                enumerateOver(intArray, "ArrayProjection instance: intArray", true);

                // Check that the property 3 exists and 11 doesnt
                checkHasProperty(intArray, 3, true);
                checkHasProperty(intArray, 11, false);

                setAndCheckProperty(intArray, "6", 88, true);
                setAndCheckProperty(intArray, "11", 11, false);

                var attributes = { writable: true, enumerable: true, configurable: true, value: 10 }
                addAndCheckProperty(intArray, "4", false, attributes);
                addAndCheckProperty(intArray, "10", false, attributes);
                addAndCheckProperty(intArray, "14", false, attributes);

                deleteAndCheckProperty(intArray, "4", false);

                // Should be able to add and manipulate expandos
                addAndCheckProperty(intArray, "FavoriteRecipe", false);
                setAndCheckProperty(intArray, "FavoriteRecipe", "Almond Cake", false);

                // Check if we can marshal in the projected array as inout parameter:
                logger.comment("myVector = myAnimal.fillArray(intArray);");
                myVector = myAnimal.fillArray(intArray);
                logger.comment('verifyVectorAndArrayItems("myVector", myVector, "intArray", intArray)');
                verifyVectorAndArrayItems("myVector", myVector, "intArray", intArray)
                verify(Array.isArray(intArray), false, "Array.isArray(intArray)");
                dumpArray("intArray", intArray);
                verifyArrayPrototypeInChain("intArray", intArray);

                // Check if we can marshal in the projected array as in parameter:
                logger.comment("intArray[4] = 300");
                intArray[4] = 300
                logger.comment("var myVector = animal.passArray(intArray);");
                var myVector = animal.passArray(intArray);
                logger.comment('verifyVectorAndArrayItems("myVector", myVector, "intArray", intArray)');
                verifyVectorAndArrayItems("myVector", myVector, "intArray", intArray)

                // Verify marshalling into different type throws exception - in param
                verify.exception(function () {
                    logger.comment("myVector = myAnimal.passArrayHSTRING(intArray);");
                    myVector = myAnimal.passArrayHSTRING(intArray);
                }, TypeError, "myAnimal.passArrayHSTRING(intArray)");

                // Verify marshalling into different type throws exception - inout param
                verify.exception(function () {
                    logger.comment("myVector = myAnimal.fillArrayHSTRING(intArray);");
                    myVector = myAnimal.fillArrayHSTRING(intArray);
                }, TypeError, "myAnimal.fillArrayHSTRING(intArray)");
            }

            logger.comment("var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];");
            var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            logger.comment("var myAnimal = new Animals.Animal(1);");
            var myAnimal = new Animals.Animal(1);

            logger.comment("delegatePassArray.outVector = myAnimal.passArray(myArray);");
            delegatePassArray.outVector = myAnimal.passArray(myArray);

            logger.comment("myAnimal.callDelegatePassArray(delegatePassArray)");
            myAnimal.callDelegatePassArray(delegatePassArray);

            verify(myArray, [1, 2, 3, 4, 5, 6, 7, 8, 9], "myArray");
        }
    });

    runner.addTest({
        id: 6,
        desc: 'Delegate_MarshalInArray_Null',
        pri: '0',
        test: function () {
            function delegatePassArray(animal, intArray) {
                logger.comment("*** delegatePassArray Delegate");
                logger.comment("var intVector = delegatePassArray.outVector;");
                var intVector = delegatePassArray.outVector;
                logger.comment('verifyVectorAndArrayItems("intArray", intArray, "null", null)');
                verifyVectorAndArrayItems("intArray", intArray, "null", null)
            }

            logger.comment("var myAnimal = new Animals.Animal(1);");
            var myAnimal = new Animals.Animal(1);

            logger.comment('var myArray = null');
            var myArray = null;

            logger.comment("delegatePassArray.outVector = myAnimal.passArray(myArray);");
            delegatePassArray.outVector = myAnimal.passArray(myArray);
            logger.comment("myAnimal.callDelegatePassArray(delegatePassArray)");
            myAnimal.callDelegatePassArray(delegatePassArray);

            verify(myArray, null, "myArray");
        }
    });

    runner.addTest({
        id: 7,
        desc: 'Delegate_ProjectOutArrayByValue',
        pri: '0',
        test: function () {
            function delegateFillArray(animal, intArray) {
                logger.comment("*** delegateFillArray Delegate");

                verify(Array.isArray(intArray), false, "Array.isArray(intArray)");
                verify(intArray.length, delegateFillArray.arrayLength, "intArray.length");
                verifyArrayPrototypeInChain("intArray", intArray);

                // Check if we can marshal in the projected array as inout parameter:
                logger.comment("myVector = myAnimal.fillArray(intArray);");
                myVector = myAnimal.fillArray(intArray);
                logger.comment('verifyVectorAndArrayItems("myVector", myVector, "intArray", intArray)');
                verifyVectorAndArrayItems("myVector", myVector, "intArray", intArray)
                verify(Array.isArray(intArray), false, "Array.isArray(intArray)");
                dumpArray("intArray", intArray);
                verifyArrayPrototypeInChain("intArray", intArray);

                // Verify length is non writable
                setAndCheckProperty(intArray, "length", 3, false);

                // Check that we can enumerate over the members of the vector instance
                enumerateOver(intArray, "ArrayProjection instance: intArray", true);

                // Check that the property 3 exists and 11 doesnt
                checkHasProperty(intArray, 3, true);
                checkHasProperty(intArray, 11, false);

                setAndCheckProperty(intArray, "6", 88, true);
                setAndCheckProperty(intArray, "6", 6, true);
                setAndCheckProperty(intArray, "11", 11, false);

                var attributes = { writable: true, enumerable: true, configurable: true, value: 10 }
                addAndCheckProperty(intArray, "4", false, attributes);
                addAndCheckProperty(intArray, "10", false, attributes);
                addAndCheckProperty(intArray, "14", false, attributes);

                deleteAndCheckProperty(intArray, "4", false);

                // Should be able to add and manipulate expandos
                addAndCheckProperty(intArray, "FavoriteRecipe", false);
                setAndCheckProperty(intArray, "FavoriteRecipe", "Almond Cake", false);

                // Check if we can marshal in the projected array as in parameter:
                logger.comment("intArray[4] = 300");
                intArray[4] = 300
                logger.comment("var myVector = animal.passArray(intArray);");
                var myVector = animal.passArray(intArray);
                logger.comment('verifyVectorAndArrayItems("myVector", myVector, "intArray", intArray)');
                verifyVectorAndArrayItems("myVector", myVector, "intArray", intArray)

                // Verify marshalling into different type throws exception - in param
                verify.exception(function () {
                    logger.comment("myVector = myAnimal.passArrayHSTRING(intArray);");
                    myVector = myAnimal.passArrayHSTRING(intArray);
                }, TypeError, "myAnimal.passArrayHSTRING(intArray)");

                // Verify marshalling into different type throws exception - inout param
                verify.exception(function () {
                    logger.comment("myVector = myAnimal.fillArrayHSTRING(intArray);");
                    myVector = myAnimal.fillArrayHSTRING(intArray);
                }, TypeError, "myAnimal.fillArrayHSTRING(intArray)");

                logger.comment('intArray[7] = 45;');
                intArray[7] = 45;

                // Save the intArray to see lifetime after this function exit
                logger.comment('delegateFillArray.intArray = intArray;');
                delegateFillArray.intArray = intArray;
            }

            logger.comment("var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];");
            var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            logger.comment("var myAnimal = new Animals.Animal(1);");
            var myAnimal = new Animals.Animal(1);

            logger.comment("myAnimal.passArray(myArray);");
            myAnimal.passArray(myArray);
            logger.comment('delegateFillArray.arrayLength = myArray.length;')
            delegateFillArray.arrayLength = myArray.length;
            logger.comment("myAnimal.callDelegateFillArray(delegateFillArray)");
            myAnimal.callDelegateFillArray(delegateFillArray);

            verify(myArray, [1, 2, 3, 4, 5, 6, 7, 8, 9], "myArray");

            logger.comment("myResult = myAnimal.receiveArray();");
            myResult = myAnimal.receiveArray();

            logger.comment('verifyVectorAndArrayItems("delegateFillArray.intArray", delegateFillArray.intArray, "myResult.outVector", myResult.outVector)');
            if (typeof Animals._CLROnly === 'undefined') //Not applicable to C# ABI due to CLR's copying of contents instead of native's pointer usage
                verifyVectorAndArrayItems("delegateFillArray.intArray", delegateFillArray.intArray, "myResult.outVector", myResult.outVector);

            // Try changing the fields outside function - shouldnt affect either abi or existing vector or projected array
            logger.comment('delegateFillArray.intArray[5] = 1234');
            delegateFillArray.intArray[5] = 1234;

            verify(myResult.value[5] != 1234, true, "myResult.value[5] != 1234");
            logger.comment("myResult = myAnimal.receiveArray();");
            myResult = myAnimal.receiveArray();
            verify(myResult.value[5] != 1234, true, "myResult.value[5] != 1234");
        }
    });

    runner.addTest({
        id: 8,
        desc: 'Delegate_ProjectOutArrayByRef_Array',
        pri: '0',
        test: function () {
            function delegateReceiveArray(animal) {
                logger.comment("*** delegateReceiveArray Delegate");
                logger.comment("delegateReceiveArray.myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];");
                delegateReceiveArray.myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
                logger.comment('return delegateReceiveArray.myArray;');
                return delegateReceiveArray.myArray;
            }

            logger.comment("var myAnimal = new Animals.Animal(1);");
            var myAnimal = new Animals.Animal(1);

            logger.comment("myAnimal.callDelegateReceiveArray(delegateReceiveArray)");
            myAnimal.callDelegateReceiveArray(delegateReceiveArray);

            verify(delegateReceiveArray.myArray, [1, 2, 3, 4, 5, 6, 7, 8, 9, 10], "delegateReceiveArray.myArray");

            logger.comment("myResult = myAnimal.receiveArray();");
            myResult = myAnimal.receiveArray();

            logger.comment('verifyVectorAndArrayItems("myResult.outVector", myResult.outVector, "delegateReceiveArray.myArray", delegateReceiveArray.myArray)');
            verifyVectorAndArrayItems("myResult.outVector", myResult.outVector, "delegateReceiveArray.myArray", delegateReceiveArray.myArray);

            // Try changing the fields outside function - shouldnt affect either abi or existing vector or projected array
            logger.comment('delegateReceiveArray.myArray[5] = 1234');
            delegateReceiveArray.myArray[5] = 1234;

            verify(myResult.value[5] != 1234, true, "myResult.value[5] != 1234");
            logger.comment("myResult = myAnimal.receiveArray();");
            myResult = myAnimal.receiveArray();
            verify(myResult.value[5] != 1234, true, "myResult.value[5] != 1234");
        }
    });

    runner.addTest({
        id: 9,
        desc: 'Delegate_ProjectOutArrayByRef_ArrayProjection',
        pri: '0',
        test: function () {
            function delegateReceiveArray(animal) {
                logger.comment("*** delegateReceiveArray Delegate");

                logger.comment("delegateReceiveArray.myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];");
                delegateReceiveArray.myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
                logger.comment("animal.passArray(delegateReceiveArray.myArray);");
                animal.passArray(delegateReceiveArray.myArray);
                logger.comment("var myResult = animal.receiveArray();");
                var myResult = animal.receiveArray();
                dumpArray("myResult.value", myResult.value);
                logger.comment('delegateReceiveArray.myArray[3] = 999;');
                delegateReceiveArray.myArray[3] = 999;
                logger.comment('myResult.value[3] = 999;');
                myResult.value[3] = 999;
                dumpArray("myResult.value", myResult.value);
                logger.comment("delegateReceiveArray.myArrayProjection = myResult.value;");
                delegateReceiveArray.myArrayProjection = myResult.value;
                logger.comment('return delegateReceiveArray.myArrayProjection;');
                return delegateReceiveArray.myArrayProjection;
            }

            logger.comment("var myAnimal = new Animals.Animal(1);");
            var myAnimal = new Animals.Animal(1);

            logger.comment("myAnimal.callDelegateReceiveArray(delegateReceiveArray)");
            myAnimal.callDelegateReceiveArray(delegateReceiveArray);

            logger.comment('verifyVectorAndArrayItems("delegateReceiveArray.myArray", delegateReceiveArray.myArray, "delegateReceiveArray.myArrayProjection", delegateReceiveArray.myArrayProjection)');
            verifyVectorAndArrayItems("delegateReceiveArray.myArray", delegateReceiveArray.myArray, "delegateReceiveArray.myArrayProjection", delegateReceiveArray.myArrayProjection);

            logger.comment("myResult = myAnimal.receiveArray();");
            myResult = myAnimal.receiveArray();

            logger.comment('verifyVectorAndArrayItems("myResult.outVector", myResult.outVector, "delegateReceiveArray.myArrayProjection", delegateReceiveArray.myArrayProjection)');
            verifyVectorAndArrayItems("myResult.outVector", myResult.outVector, "delegateReceiveArray.myArrayProjection", delegateReceiveArray.myArrayProjection);

            // Try changing the fields outside function - shouldnt affect either abi or existing vector or projected array
            logger.comment('delegateReceiveArray.myArrayProjection[5] = 1234');
            delegateReceiveArray.myArrayProjection[5] = 1234;

            verify(myResult.value[5] != 1234, true, "myResult.value[5] != 1234");
            verify(delegateReceiveArray.myArray[5] != 1234, true, "delegateReceiveArray.myArray[5] != 1234");
            logger.comment("myResult = myAnimal.receiveArray();");
            myResult = myAnimal.receiveArray();
            verify(myResult.value[5] != 1234, true, "myResult.value[5] != 1234");
        }
    });

    runner.addTest({
        id: 11,
        desc: 'ArrayProjection_ArrayPrototypeMethods',
        pri: '0',
        test: function () {
            function VerifyTypedArray(obj, content, comment) {
                logger.comment(comment);
                for (var i = 0; i < obj.length; i++) {
                    verify(obj[i], content[i], i);
                }
            }
            logger.comment("var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];");
            var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            logger.comment("var myAnimal = new Animals.Animal(1);");
            var myAnimal = new Animals.Animal(1);

            logger.comment("var myVector = myAnimal.passArray(myArray);");
            var myVector = myAnimal.passArray(myArray);
            logger.comment("myResult = myAnimal.receiveArray();");
            myResult = myAnimal.receiveArray();
            logger.comment("var myArrayProjection = myResult.value");
            var myArrayProjection = myResult.value;

            dumpArray("myArrayProjection", myArrayProjection);

            logger.comment("var secondResult = myAnimal.receiveArray();");
            var secondResult = myAnimal.receiveArray();
            logger.comment("var secondArray = secondResult.value;");
            var secondArray = secondResult.value;
            logger.comment("Object.getPrototypeOf(secondArray) === Object.getPrototypeOf(myArrayProjection)");
            verify(Object.getPrototypeOf(secondArray) === Object.getPrototypeOf(myArrayProjection), true, "compare prototype of array projection");


            var foundException = false;

            verify.exception(function () {
                logger.comment("var n = myArrayProjection.pop()")
                var n = myArrayProjection.pop();
                logger.comment('n : ' + n);
                dumpArray("myArrayProjection", myArrayProjection);
            }, TypeError, "Object doesn't support property or method 'pop'");

            verify.exception(function () {
                logger.comment("n = myArrayProjection.push()")
                n = myArrayProjection.push();
                logger.comment('n : ' + n);
                dumpArray("myArrayProjection", myArrayProjection);
            }, TypeError, "Object doesn't support property or method 'push'");

            verify.exception(function () {
                logger.comment("n = myArrayProjection.push(11)")
                n = myArrayProjection.push(11);
                logger.comment('n : ' + n);
                dumpArray("myArrayProjection", myArrayProjection);
            }, TypeError, "Object doesn't support property or method 'push'");

            verify.exception(function () {
                logger.comment("n = myArrayProjection.push(11, 22, 33)")
                n = myArrayProjection.push(11, 22, 33);
                logger.comment('n : ' + n);
                dumpArray("myArrayProjection", myArrayProjection);
            }, TypeError, "Object doesn't support property or method 'push'");

            verify.exception(function () {
                logger.comment("n = myArrayProjection.shift()")
                n = myArrayProjection.shift()
                logger.comment('n : ' + n);
                dumpArray("myArrayProjection", myArrayProjection);
            }, TypeError, "Object doesn't support property or method 'shift'");

            verify.exception(function () {
                logger.comment("n = myArrayProjection.unshift()")
                n = myArrayProjection.unshift()
                logger.comment('n : ' + n);
                dumpArray("myArrayProjection", myArrayProjection);
            }, TypeError, "Object doesn't support property or method 'unshift'");

            verify.exception(function () {
                logger.comment("n = myArrayProjection.unshift(0)")
                n = myArrayProjection.unshift(0)
                logger.comment('n : ' + n);
                dumpArray("myArrayProjection", myArrayProjection);
            }, TypeError, "Object doesn't support property or method 'unshift'");

            verify.exception(function () {
                logger.comment("n = myArrayProjection.unshift(25, 50, 100)")
                n = myArrayProjection.unshift(25, 50, 100);
                logger.comment('n : ' + n);
                dumpArray("myArrayProjection", myArrayProjection);
            }, TypeError, "Object doesn't support property or method 'unshift'");

            verify.exception(function () {
                logger.comment("n = myArrayProjection.slice()")
                n = myArrayProjection.slice()
                dumpArray("n", n);
                dumpArray("myArrayProjection", myArrayProjection);
            }, TypeError, "Object doesn't support property or method 'slice'");

            verify.exception(function () {
                logger.comment("n = myArrayProjection.slice(5, 2)")
                n = myArrayProjection.slice(5, 2)
                dumpArray("n", n);
                dumpArray("myArrayProjection", myArrayProjection);
            }, TypeError, "Object doesn't support property or method 'slice'");

            verify.exception(function () {
                logger.comment("n = myArrayProjection.slice(5, 8)")
                n = myArrayProjection.slice(5, 8)
                dumpArray("n", n);
                dumpArray("myArrayProjection", myArrayProjection);
            }, TypeError, "Object doesn't support property or method 'slice'");

            verify.exception(function () {
                logger.comment("n = myArrayProjection.slice(5)")
                n = myArrayProjection.slice(5)
                dumpArray("n", n);
                dumpArray("myArrayProjection", myArrayProjection);
            }, TypeError, "Object doesn't support property or method 'slice'");

            verify.exception(function () {
                logger.comment("n = myArrayProjection.slice(-2)")
                n = myArrayProjection.slice(-2)
                dumpArray("n", n);
                dumpArray("myArrayProjection", myArrayProjection);
            }, TypeError, "Object doesn't support property or method 'slice'");

            verify.exception(function () {
                logger.comment("n = myArrayProjection.slice(-7, -2)")
                n = myArrayProjection.slice(-7, -2)
                dumpArray("n", n);
                dumpArray("myArrayProjection", myArrayProjection);
            }, TypeError, "Object doesn't support property or method 'slice'");

            verify.exception(function () {
                logger.comment("n = myArrayProjection.splice()")
                n = myArrayProjection.splice()
                dumpArray("n", n);
                dumpArray("myArrayProjection", myArrayProjection);
            }, TypeError, "Object doesn't support property or method 'splice'");

            verify.exception(function () {
                logger.comment("n = myArrayProjection.splice(5, 0)")
                n = myArrayProjection.splice(5, 0)
                dumpArray("n", n);
                dumpArray("myArrayProjection", myArrayProjection);
            }, TypeError, "Object doesn't support property or method 'splice'");

            verify.exception(function () {
                logger.comment("n = myArrayProjection.splice(5, 2)")
                n = myArrayProjection.splice(5, 2)
                dumpArray("n", n);
                dumpArray("myArrayProjection", myArrayProjection);
            }, TypeError, "Object doesn't support property or method 'splice'");

            verify.exception(function () {
                logger.comment("n = myArrayProjection.splice(2, 2, 14, 98, 54)")
                n = myArrayProjection.splice(2, 2, 14, 98, 54)
                dumpArray("n", n);
                dumpArray("myArrayProjection", myArrayProjection);
            }, TypeError, "Object doesn't support property or method 'splice'");

            verify.exception(function () {
                logger.comment("n = myArrayProjection.splice(4, 0, 32, 83)")
                n = myArrayProjection.splice(4, 0, 32, 83);
                dumpArray("n", n);
                dumpArray("myArrayProjection", myArrayProjection);
            }, TypeError, "Object doesn't support property or method 'splice'");

            logger.comment("buffer = myArrayProjection.buffer");
            buffer = myArrayProjection.buffer;
            // buffer's properties are not enumerable
            enumerateOver(buffer, "typed array instance: myArrayProjection.buffer", false);
            VerifyTypedArray(myArrayProjection, [1, 2, 3, 4, 5, 6, 7, 8, 9], "typed array of myArrayProjection");
            var subArray = myArrayProjection.subarray(1);
            VerifyTypedArray(subArray, [2, 3, 4, 5, 6, 7, 8, 9], "typed array of myArrayProjection");
        }
    });

    runner.addTest({
        id: 12,
        desc: 'MarshalInES5Array',
        pri: '0',
        test: function () {
            var myArray = GetES5Array("myArray", true);
            logger.comment("var myAnimal = new Animals.Animal(1);");
            var myAnimal = new Animals.Animal(1);

            logger.comment("var myVector = myAnimal.passArray(myArray);");
            var myVector = myAnimal.passArray(myArray);
            logger.comment('verifyVectorAndArrayItems("myVector", myVector, "myArray", myArray)');
            verifyVectorAndArrayItems("myVector", myVector, "myArray", myArray)
        }
    });

    runner.addTest({
        id: 13,
        desc: 'ProjectOutES5ArrayByValue',
        pri: '0',
        test: function () {
            logger.comment("var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];");
            var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            logger.comment("var myAnimal = new Animals.Animal(1);");
            var myAnimal = new Animals.Animal(1);

            logger.comment("var myVector = myAnimal.passArray(myArray);");
            var myVector = myAnimal.passArray(myArray);
            logger.comment('verifyVectorAndArrayItems("myVector", myVector, "myArray", myArray)');
            verifyVectorAndArrayItems("myVector", myVector, "myArray", myArray);

            myNewArray = GetES5Array("myNewArray", false);
            logger.comment("myVector = myAnimal.fillArray(myNewArray);");
            myVector = myAnimal.fillArray(myNewArray);
            logger.comment('verifyVectorAndArrayItems("myNewArray", myNewArray, "myVector", myVector)');
            verifyVectorAndArrayItems("myNewArray", myNewArray, "myVector", myVector);
            verify(Array.isArray(myNewArray), true, "Array.isArray(myNewArray)");

            logger.comment("myNewArray.length = 4");
            myNewArray.length = 4;
            logger.comment("myNewArray[0] = 11;");
            myNewArray[0] = 11;
            logger.comment("myNewArray[1] = 22;");
            myNewArray[1] = 22;
            logger.comment("myNewArray[2] = 33;");
            myNewArray[2] = 33;
            logger.comment("myNewArray[3] = 44;");
            myNewArray[3] = 44;
            logger.comment("myVector = myAnimal.fillArray(myNewArray);");
            myVector = myAnimal.fillArray(myNewArray);
            logger.comment('verifyVectorAndArrayItems("myNewArray", myNewArray, "myVector", myVector)');
            verifyVectorAndArrayItems("myNewArray", myNewArray, "myVector", myVector);
            verify(Array.isArray(myNewArray), true, "Array.isArray(myNewArray)");

            logger.comment("myNewArray.length = 0;");
            myNewArray.length = 0;
            logger.comment("myVector = myAnimal.fillArray(myNewArray);");
            myVector = myAnimal.fillArray(myNewArray);
            logger.comment('verifyVectorAndArrayItems("myNewArray", myNewArray, "myVector", myVector)');
            verifyVectorAndArrayItems("myNewArray", myNewArray, "myVector", myVector);
            verify(Array.isArray(myNewArray), true, "Array.isArray(myNewArray)");
        }
    });

    runner.addTest({
        id: 14,
        desc: 'Delegate_ProjectOutArrayByRef_ES5Array',
        pri: '0',
        test: function () {
            function delegateReceiveArray(animal) {
                logger.comment("*** delegateReceiveArray Delegate");
                delegateReceiveArray.myArray = GetES5Array("delegateReceiveArray.myArray", true);
                logger.comment('return delegateReceiveArray.myArray;');
                return delegateReceiveArray.myArray;
            }

            logger.comment("var myAnimal = new Animals.Animal(1);");
            var myAnimal = new Animals.Animal(1);

            logger.comment("myAnimal.callDelegateReceiveArray(delegateReceiveArray)");
            myAnimal.callDelegateReceiveArray(delegateReceiveArray);

            verify(delegateReceiveArray.myArray, [1, 2, 3, 4, 5, 6, 7, 8, 9], "delegateReceiveArray.myArray");

            logger.comment("myResult = myAnimal.receiveArray();");
            myResult = myAnimal.receiveArray();

            logger.comment('verifyVectorAndArrayItems("myResult.outVector", myResult.outVector, "delegateReceiveArray.myArray", delegateReceiveArray.myArray)');
            verifyVectorAndArrayItems("myResult.outVector", myResult.outVector, "delegateReceiveArray.myArray", delegateReceiveArray.myArray);

            // Try changing the fields outside function - shouldnt affect either abi or existing vector or projected array
            logger.comment('delegateReceiveArray.myArray[5] = 1234');
            delegateReceiveArray.myArray[5] = 1234;

            verify(myResult.value[5] != 1234, true, "myResult.value[5] != 1234");
            logger.comment("myResult = myAnimal.receiveArray();");
            myResult = myAnimal.receiveArray();
            verify(myResult.value[5] != 1234, true, "myResult.value[5] != 1234");
        }
    });

    runner.addTest({
        id: 15,
        desc: 'ArrayOnlyMarshalingAndProjecting',
        pri: '0',
        test: function () {
            logger.comment("var myAnimal = new Animals.Animal(1);");
            var myAnimal = new Animals.Animal(1);

            logger.comment("PassArray:");
            var myArray = [1, 2, 3, 4, 5];
            myAnimal.purePassArray(myArray);
            verify(myArray, [1, 2, 3, 4, 5], "myArray");

            logger.comment("FillArray:");
            myAnimal.pureFillArray(myArray);
            verify(myArray, [1, 2, 3, 4, 5], "myArray");

            logger.comment("ReceiveArray:");
            var myArray2 = myAnimal.pureReceiveArray();
            var str = myArray2.toString();
            
            // Support ES5/ES6 behavior
            if (str === "[object Int32Array]") {
                verify(str, "[object Int32Array]", "myArray2.toString()");
            } else {
                verify(str, "1,2,3,4,5", "myArray2.toString()");
            }
        }
    });

    runner.addTest({
        id: 16,
        desc: 'Receive a small struct array',
        pri: '0',
        test: function () {
            var packedByteArray = Animals.Animal.getPackedByteArray();
            verify(packedByteArray.length, 5, "packedByteArray.length");
            for (var arrayIndex = 0; arrayIndex < 5; arrayIndex++) {
                verify(packedByteArray[arrayIndex].toString(), "[object Animals.PackedByte]", "packedByteArray[" + arrayIndex + "].toString()");
                verify(packedByteArray[arrayIndex].field0, arrayIndex, "packedByteArray[" + arrayIndex + "].field0");
            }
        }
    });

    runner.addTest({
        id: 17,
        desc: 'Receive a small packed boolean array',
        pri: '0',
        test: function () {
            var packedBooleanArray = Animals.Animal.getPackedBooleanArray();
            verify(packedBooleanArray.length, 5, "packedBooleanArray.length");
            for (var arrayIndex = 0; arrayIndex < 5; arrayIndex++) {
                verify(packedBooleanArray[arrayIndex].toString(), "[object Animals.PackedBoolean4]", "packedBooleanArray[" + arrayIndex + "].toString()");
                verify(packedBooleanArray[arrayIndex].field0, false, "packedBooleanArray[" + arrayIndex + "].field0");
                verify(packedBooleanArray[arrayIndex].field1, true, "packedBooleanArray[" + arrayIndex + "].field1");
                verify(packedBooleanArray[arrayIndex].field2, true, "packedBooleanArray[" + arrayIndex + "].field2");
                verify(packedBooleanArray[arrayIndex].field3, false, "packedBooleanArray[" + arrayIndex + "].field3");
            }
        }
    });

    runner.addTest({
        id: 18,
        desc: 'Receive a odd sized struct',
        pri: '0',
        test: function () {
            var oddSizedStructArray = Animals.Animal.getOddSizedStructArray();
            verify(oddSizedStructArray.length, 5, "oddSizedStructArray.length");
            for (var arrayIndex = 0; arrayIndex < 5; arrayIndex++) {
                verify(oddSizedStructArray[arrayIndex].toString(), "[object Animals.OddSizedStruct]", "oddSizedStructArray[" + arrayIndex + "].toString()");
                verify(oddSizedStructArray[arrayIndex].field0, arrayIndex, "oddSizedStructArray[" + arrayIndex + "].field0");
                verify(oddSizedStructArray[arrayIndex].field1, arrayIndex + 50, "oddSizedStructArray[" + arrayIndex + "].field1");
                verify(oddSizedStructArray[arrayIndex].field2, arrayIndex + 200, "oddSizedStructArray[" + arrayIndex + "].field2");
            }
        }
    });

    runner.addTest({
        id: 19,
        desc: 'Receive a small complex struct',
        pri: '0',
        test: function () {
            var smallComplexStructArray = Animals.Animal.getSmallComplexStructArray();
            verify(smallComplexStructArray.length, 5, "smallComplexStructArray.length");
            for (var arrayIndex = 0; arrayIndex < 5; arrayIndex++) {
                verify(smallComplexStructArray[arrayIndex].toString(), "[object Animals.SmallComplexStruct]", "smallComplexStructArray[" + arrayIndex + "].toString()");
                verify(smallComplexStructArray[arrayIndex].field0, arrayIndex, "smallComplexStructArray[" + arrayIndex + "].field0");
                verify(smallComplexStructArray[arrayIndex].field1.toString(), "[object Animals.PackedByte]", "smallComplexStructArray[" + arrayIndex + "].field1.toString()");
                verify(smallComplexStructArray[arrayIndex].field1.field0, arrayIndex + 50, "smallComplexStructArray[" + arrayIndex + "].field1.field0");
                verify(smallComplexStructArray[arrayIndex].field2, arrayIndex + 200, "smallComplexStructArray[" + arrayIndex + "].field2");
            }
        }
    });

    runner.addTest({
        id: 20,
        desc: 'Receive a big complex struct',
        pri: '0',
        test: function () {
            var bigComplexStructArray = Animals.Animal.getBigComplexStructArray();
            verify(bigComplexStructArray.length, 5, "bigComplexStructArray.length");
            for (var arrayIndex = 0; arrayIndex < 5; arrayIndex++) {
                verify(bigComplexStructArray[arrayIndex].toString(), "[object Animals.BigComplexStruct]", "bigComplexStructArray[" + arrayIndex + "].toString()");

                verify(bigComplexStructArray[arrayIndex].field0, arrayIndex, "bigComplexStructArray[" + arrayIndex + "].field0");

                verify(bigComplexStructArray[arrayIndex].field1.toString(), "[object Animals.PackedByte]", "bigComplexStructArray[" + arrayIndex + "].field1.toString()");
                verify(bigComplexStructArray[arrayIndex].field1.field0, arrayIndex + 50, "bigComplexStructArray[" + arrayIndex + "].field1.field0");

                verify(bigComplexStructArray[arrayIndex].field2, arrayIndex + 200, "bigComplexStructArray[" + arrayIndex + "].field2");

                verify(bigComplexStructArray[arrayIndex].field3.toString(), "[object Animals.PackedBoolean4]", "bigComplexStructArray[" + arrayIndex + "].field3.toString()");
                verify(bigComplexStructArray[arrayIndex].field3.field0, false, "bigComplexStructArray[" + arrayIndex + "].field3.field0");
                verify(bigComplexStructArray[arrayIndex].field3.field1, true, "bigComplexStructArray[" + arrayIndex + "].field3.field1");
                verify(bigComplexStructArray[arrayIndex].field3.field2, false, "bigComplexStructArray[" + arrayIndex + "].field3.field2");
                verify(bigComplexStructArray[arrayIndex].field3.field3, true, "bigComplexStructArray[" + arrayIndex + "].field3.field3");

                verify(bigComplexStructArray[arrayIndex].field4.toString(), "[object Animals.SmallComplexStruct]", "bigComplexStructArray[" + arrayIndex + "].field3.toString()");
                verify(bigComplexStructArray[arrayIndex].field4.field0, arrayIndex + 180, "bigComplexStructArray[" + arrayIndex + "].field4.field0");
                verify(bigComplexStructArray[arrayIndex].field4.field1.toString(), "[object Animals.PackedByte]", "bigComplexStructArray[" + arrayIndex + "].field4.field1.toString()");
                verify(bigComplexStructArray[arrayIndex].field4.field1.field0, arrayIndex + 150, "bigComplexStructArray[" + arrayIndex + "].field4.field1.field0");
                verify(bigComplexStructArray[arrayIndex].field4.field2, arrayIndex + 190, "bigComplexStructArray[" + arrayIndex + "].field4.field2");

                verify(bigComplexStructArray[arrayIndex].field5.toString(), "[object Animals.SmallComplexStruct]", "bigComplexStructArray[" + arrayIndex + "].field5.toString()");
                verify(bigComplexStructArray[arrayIndex].field5.field0, arrayIndex + 80, "bigComplexStructArray[" + arrayIndex + "].field5.field0");
                verify(bigComplexStructArray[arrayIndex].field5.field1.toString(), "[object Animals.PackedByte]", "bigComplexStructArray[" + arrayIndex + "].field5.field1.toString()");
                verify(bigComplexStructArray[arrayIndex].field5.field1.field0, arrayIndex + 50, "bigComplexStructArray[" + arrayIndex + "].field5.field1.field0");
                verify(bigComplexStructArray[arrayIndex].field5.field2, arrayIndex + 90, "bigComplexStructArray[" + arrayIndex + "].field5.field2");

                verify(bigComplexStructArray[arrayIndex].field6, arrayIndex + 7, "bigComplexStructArray[" + arrayIndex + "].field6");

                verify(bigComplexStructArray[arrayIndex].field7, arrayIndex + 2000, "bigComplexStructArray[" + arrayIndex + "].field7");
            }
        }
    });

    runner.addTest({
        id: 22,
        desc: 'Passing CanvasPixelArray as Uint8Array',
        pri: '0',
        preReq: function () {
            return (typeof WScript !== 'undefined');
        },
        test: function () {
            logger.comment("Create Canvas Pixel Array");
            var canvasPixelArray = WScript.CreateCanvasPixelArray([2, 3, 4, 5, 6, 7, 8, 9]);
            verifyCanvasPixelArray(canvasPixelArray, [2, 3, 4, 5, 6, 7, 8, 9], "canvasPixelArray");

            logger.comment("Pass as Uint8Array");
            var myVector = Animals.Animal.passUInt8Array(canvasPixelArray);
            verifyCanvasPixelArray(myVector, canvasPixelArray, "myVector");
        }
    });

    runner.addTest({
        id: 23,
        desc: 'Filling CanvasPixelArray as Uint8Array',
        pri: '0',
        preReq: function () {
            return (typeof WScript !== 'undefined');
        },
        test: function () {
            logger.comment("Create Canvas Pixel Array");
            var canvasPixelArray = WScript.CreateCanvasPixelArray([2, 3, 4, 5, 6, 7, 8, 9]);
            verifyCanvasPixelArray(canvasPixelArray, [2, 3, 4, 5, 6, 7, 8, 9], "canvasPixelArray");

            logger.comment("Fill it as Uint8Array");
            Animals.Animal.fillUInt8Array(canvasPixelArray, [22, 33, 44]);
            verifyCanvasPixelArray(canvasPixelArray, [22, 33, 44, 0, 0, 0, 0, 0], "canvasPixelArray");
        }
    });

    runner.addTest({
        id: 24,
        desc: 'Passing CanvasPixelArray as IntArray',
        pri: '0',
        preReq: function () {
            return (typeof WScript !== 'undefined');
        },
        test: function () {
            logger.comment("Create Canvas Pixel Array");
            var canvasPixelArray = WScript.CreateCanvasPixelArray([2, 3, 4, 5, 6, 7, 8, 9]);
            verifyCanvasPixelArray(canvasPixelArray, [2, 3, 4, 5, 6, 7, 8, 9], "canvasPixelArray");

            logger.comment("Pass as IntArray");
            verify.exception(function () {
                var myVector = new Animals.Animal(1).passArray(canvasPixelArray);
            }, TypeError, "new Animals.Animal(1).passArray(canvasPixelArray)");
        }
    });

    runner.addTest({
        id: 25,
        desc: 'Filling CanvasPixelArray as IntArray',
        pri: '0',
        preReq: function () {
            return (typeof WScript !== 'undefined');
        },
        test: function () {
            logger.comment("Create Canvas Pixel Array");
            var canvasPixelArray = WScript.CreateCanvasPixelArray([2, 3, 4, 5, 6, 7, 8, 9]);
            verifyCanvasPixelArray(canvasPixelArray, [2, 3, 4, 5, 6, 7, 8, 9], "canvasPixelArray");

            logger.comment("Pass Array");
            var myAnimal = new Animals.Animal(1);
            myAnimal.passArray([22, 33, 44]);

            logger.comment("Fill it as IntArray");
            verify.exception(function () {
                myAnimal.fillArray(canvasPixelArray);
            }, TypeError, "myAnimal.fillArray(canvasPixelArray)");
        }
    });

    runner.addTest({
        id: 26,
        desc: 'PassArray With [in] length attribute',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);

            logger.comment("Call PassArray with 4 elements on JsArray of 9 elements");
            myAnimal.passArrayWithInLength([1, 2, 3, 4, 5, 6, 7, 8, 9], 4);
            var resultArray = myAnimal.pureReceiveArray();
            verifyArrayItems("resultArray", resultArray, [1, 2, 3, 4, 0, 0, 0, 0, 0]);

            logger.comment("Call PassArray with 9 elements on JsArray of 9 elements");
            myAnimal.passArrayWithInLength([11, 22, 33, 44, 55, 66, 77, 88, 99], 9);
            resultArray = myAnimal.pureReceiveArray();
            verifyArrayItems("resultArray", resultArray, [11, 22, 33, 44, 55, 66, 77, 88, 99]);

            logger.comment("Call PassArray with 11 elements on JsArray of 9 elements");
            verify.exception(function () {
                myAnimal.passArrayWithInLength([111, 222, 333, 444, 555, 666, 777, 888, 999], 11);
            }, TypeError, "PassArray with length > size");

            logger.comment("Call PassArray with 4 elements on Int32Array of 9 elements");
            myAnimal.passArrayWithInLength(new Int32Array([1, 2, 3, 4, 5, 6, 7, 8, 9]), 4);
            var resultArray = myAnimal.pureReceiveArray();
            verifyArrayItems("resultArray", resultArray, [1, 2, 3, 4, 0, 0, 0, 0, 0]);

            logger.comment("Call PassArray with 9 elements on Int32Array of 9 elements");
            myAnimal.passArrayWithInLength(new Int32Array([11, 22, 33, 44, 55, 66, 77, 88, 99]), 9);
            resultArray = myAnimal.pureReceiveArray();
            verifyArrayItems("resultArray", resultArray, [11, 22, 33, 44, 55, 66, 77, 88, 99]);

            logger.comment("Call PassArray with 11 elements on Int32Array of 9 elements");
            verify.exception(function () {
                myAnimal.passArrayWithInLength(new Int32Array([111, 222, 333, 444, 555, 666, 777, 888, 999]), 11);
            }, TypeError, "PassArray with length > size");

            logger.comment("Call PassArray with 0 elements on null");
            myAnimal.passArrayWithInLength(null, 0);
            var resultArray = myAnimal.pureReceiveArray();
            verify(resultArray, null, "resultArray");

            logger.comment("Call PassArray with 11 elements on null");
            verify.exception(function () {
                myAnimal.passArrayWithInLength(null, 11);
            }, TypeError, "PassArray with length > size");
        }
    });

    runner.addTest({
        id: 27,
        desc: 'PassArray With [out] length attribute',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);

            logger.comment("Call PassArray JsArray");
            var elementsRead = myAnimal.passArrayWithOutLength([1, 2, 3, 4, 5, 6, 7, 8, 9]);
            var resultArray = myAnimal.pureReceiveArray();
            verify(elementsRead, 9, "elementsRead");
            verifyArrayItems("resultArray", resultArray, [1, 2, 3, 4, 5, 6, 7, 8, 9]);

            logger.comment("Call PassArray Int32Array");
            elementsRead = myAnimal.passArrayWithOutLength(new Int32Array([11, 22, 33, 44]));
            resultArray = myAnimal.pureReceiveArray();
            verify(elementsRead, 4, "elementsRead");
            verifyArrayItems("resultArray", resultArray, [11, 22, 33, 44]);

            logger.comment("Call PassArray with 0 elements on null");
            elementsRead = myAnimal.passArrayWithOutLength(null);
            var resultArray = myAnimal.pureReceiveArray();
            verify(elementsRead, 0, "elementsRead");
            verify(resultArray, null, "resultArray");
        }
    });

    runner.addTest({
        id: 28,
        desc: 'FillArray With [in] length attribute',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);
            myAnimal.purePassArray([12, 23, 34, 45, 56, 67, 78, 89, 90]);

            logger.comment("Call FillArray with 4 elements on JsArray of 9 elements");
            var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            myAnimal.fillArrayWithInLength(myArray, 4);
            verifyArrayItems("myArray", myArray, [12, 23, 34, 45, 0, 0, 0, 0, 0]);

            logger.comment("Call FillArray with 9 elements on JsArray of 9 elements");
            myArray = [11, 22, 33, 44, 55, 66, 77, 88, 99];
            myAnimal.fillArrayWithInLength(myArray, 9);
            verifyArrayItems("myArray", myArray, [12, 23, 34, 45, 56, 67, 78, 89, 90]);

            logger.comment("Call FillArray with 11 elements on JsArray of 9 elements");
            myArray = [111, 222, 333, 444, 555, 666, 777, 888, 999];
            verify.exception(function () {
                myAnimal.fillArrayWithInLength(myArray, 11);
            }, TypeError, "FillArray with length > size");

            logger.comment("Call FillArray with 4 elements on Int32Array of 9 elements");
            var myArray = new Int32Array([1, 2, 3, 4, 5, 6, 7, 8, 9]);
            myAnimal.fillArrayWithInLength(myArray, 4);
            verifyArrayItems("myArray", myArray, [12, 23, 34, 45, 0, 0, 0, 0, 0]);

            logger.comment("Call FillArray with 9 elements on Int32Array of 9 elements");
            myArray = new Int32Array([11, 22, 33, 44, 55, 66, 77, 88, 99]);
            myAnimal.fillArrayWithInLength(myArray, 9);
            verifyArrayItems("myArray", myArray, [12, 23, 34, 45, 56, 67, 78, 89, 90]);

            logger.comment("Call FillArray with 11 elements on Int32Array of 9 elements");
            myArray = new Int32Array([111, 222, 333, 444, 555, 666, 777, 888, 999]);
            verify.exception(function () {
                myAnimal.fillArrayWithInLength(myArray, 11);
            }, TypeError, "FillArray with length > size");

            logger.comment("Call FillArray with 0 elements on null");
            myAnimal.fillArrayWithInLength(null, 0);

            logger.comment("Call FillArray with 11 elements on null");
            verify.exception(function () {
                myAnimal.fillArrayWithInLength(null, 11);
            }, TypeError, "FillArray with length > size");
        }
    });

    runner.addTest({
        id: 29,
        desc: 'FillArray With [out] length attribute',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);
            myAnimal.purePassArray([12, 23, 34, 45]);

            logger.comment("Call FillArray with 4 elements on JsArray of 9 elements");
            var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            var elementsRead = myAnimal.fillArrayWithOutLength(myArray);
            verify(elementsRead, 4, "elementsRead");
            verifyArrayItems("myArray", myArray, [12, 23, 34, 45, 0, 0, 0, 0, 0]);

            logger.comment("Call FillArray with 2 elements on JsArray of 2 elements");
            var myArray = [1, 2];
            var elementsRead = myAnimal.fillArrayWithOutLength(myArray);
            verify(elementsRead, 2, "elementsRead");
            verifyArrayItems("myArray", myArray, [12, 23]);

            logger.comment("Call FillArray with 4 elements on Int32Array of 9 elements");
            var myArray = new Int32Array([1, 2, 3, 4, 5, 6, 7, 8, 9]);
            elementsRead = myAnimal.fillArrayWithOutLength(myArray);
            verify(elementsRead, 4, "elementsRead");
            verifyArrayItems("myArray", myArray, [12, 23, 34, 45, 0, 0, 0, 0, 0]);

            logger.comment("Call FillArray with 2 elements on Int32Array of 2 elements");
            var myArray = new Int32Array([1, 2]);
            var elementsRead = myAnimal.fillArrayWithOutLength(myArray);
            verify(elementsRead, 2, "elementsRead");
            verifyArrayItems("myArray", myArray, [12, 23]);

            logger.comment("Call FillArray with 0 elements on null");
            elementsRead = myAnimal.fillArrayWithOutLength(null);
            verify(elementsRead, 0, "elementsRead");
        }
    });

    runner.addTest({
        id: 30,
        desc: 'ReceiveArray With [in] length attribute',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);
            myAnimal.purePassArray([12, 23, 34, 45, 56, 67, 78, 89, 90]);

            logger.comment("Call ReceiveArray with 4 elements");
            var myArray = myAnimal.receiveArrayWithInLength(4);
            verifyArrayItems("myArray", myArray, [12, 23, 34, 45, 0, 0, 0, 0, 0]);

            logger.comment("Call ReceiveArray with 9 elements");
            var myArray = myAnimal.receiveArrayWithInLength(9);
            verifyArrayItems("myArray", myArray, [12, 23, 34, 45, 56, 67, 78, 89, 90]);

            logger.comment("Call ReceiveArray with 11 elements");
            var myArray = myAnimal.receiveArrayWithInLength(11);
            verifyArrayItems("myArray", myArray, [12, 23, 34, 45, 56, 67, 78, 89, 90, 0, 0]);

            logger.comment("Call ReceiveArray with 0 elements");
            var myArray = myAnimal.receiveArrayWithInLength(0);
            verifyArrayItems("myArray", myArray, [0, 0, 0, 0, 0, 0, 0, 0, 0]);

            logger.comment("Call ReceiveArray with 0 elements");
            myAnimal.purePassArray([]);
            var myArray = myAnimal.receiveArrayWithInLength(0);
            verify(myArray, null, "myArray");
        }
    });

    runner.addTest({
        id: 31,
        desc: 'ReceiveArray With [out] length attribute',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);

            logger.comment("Call ReceiveArray with size = 4  length = 2");
            myAnimal.passArrayWithInLength([12, 23, 34, 45], 2);
            var result = myAnimal.receiveArrayWithOutLength();
            verify(result.lengthValue, 2, "elementsRead");
            verifyArrayItems("myArray", result.value, [12, 23, 0, 0]);

            logger.comment("Call ReceiveArray with size = 4  length = 4");
            myAnimal.passArrayWithInLength([12, 23, 34, 45], 4);
            var result = myAnimal.receiveArrayWithOutLength();
            verify(result.lengthValue, 4, "elementsRead");
            verifyArrayItems("myArray", result.value, [12, 23, 34, 45]);

            logger.comment("Call ReceiveArray with size = 0  length = 0");
            myAnimal.purePassArray([]);
            var result = myAnimal.receiveArrayWithOutLength();
            verify(result.lengthValue, 0, "elementsRead");
            verify(result.value, null, "myArray");
        }
    });

    runner.addTest({
        id: 32,
        desc: 'Delegate PassArray With [in] length attribute',
        pri: '0',
        test: function () {
            var actualLength = 0;
            var actualArray = 0

            function delegatePassArrayWithInLength(arrayInDelegate, lengthAttribute) {
                logger.comment("*** delegatePassArrayWithInLength - Invoke ***");

                dumpArrayItems("arrayInDelegate", arrayInDelegate);
                logger.comment("lengthAttribute : " + lengthAttribute);

                actualLength = lengthAttribute;
                actualArray = arrayInDelegate;

                logger.comment("*** delegatePassArrayWithInLength - Exit ***");
            }

            logger.comment("Call PassArray with size = 4  length = 2");
            var myAnimal = new Animals.Animal(1);
            myAnimal.passArrayWithInLength([12, 23, 34, 45], 2);
            myAnimal.callDelegatePassArrayWithInLength(delegatePassArrayWithInLength);
            verify(actualLength, 2, "elementsRead");
            verifyArrayItems("myArray", actualArray, [12, 23, 0, 0]);

            logger.comment("Call PassArray with size = 4  length = 4");
            myAnimal.passArrayWithInLength([12, 23, 34, 45], 4);
            myAnimal.callDelegatePassArrayWithInLength(delegatePassArrayWithInLength);
            verify(actualLength, 4, "elementsRead");
            verifyArrayItems("myArray", actualArray, [12, 23, 34, 45]);

            logger.comment("Call PassArray with size = 0  length = 0");
            myAnimal.purePassArray([]);
            myAnimal.callDelegatePassArrayWithInLength(delegatePassArrayWithInLength);
            verify(actualLength, 0, "elementsRead");
            verify(actualArray, null, "myArray");
        }
    });

    runner.addTest({
        id: 33,
        desc: 'Delegate PassArray With [out] length attribute',
        pri: '0',
        test: function () {
            var actualArray = 0
            var sendLength;
            function delegatePassArrayWithOutLength(arrayInDelegate) {
                logger.comment("*** delegatePassArrayWithOutLength - Invoke ***");
                dumpArrayItems("arrayInDelegate", arrayInDelegate);

                actualArray = arrayInDelegate;

                logger.comment("*** delegatePassArrayWithOutLength - Exit ***");
                return sendLength;
            }

            logger.comment("Call PassArray with size = 4  length = 2");
            var myAnimal = new Animals.Animal(1);
            myAnimal.passArrayWithInLength([12, 23, 34, 45], 2);
            sendLength = 2;
            myAnimal.callDelegatePassArrayWithOutLength(delegatePassArrayWithOutLength);
            verifyArrayItems("myArray", actualArray, [12, 23, 0, 0]);

            logger.comment("Call PassArray with size = 4  length = 4");
            myAnimal.passArrayWithInLength([12, 23, 34, 45], 4);
            sendLength = 4;
            myAnimal.callDelegatePassArrayWithOutLength(delegatePassArrayWithOutLength);
            verifyArrayItems("myArray", actualArray, [12, 23, 34, 45]);

            logger.comment("Call PassArray with size = 0  length = 0");
            myAnimal.purePassArray([]);
            sendLength = 0;
            myAnimal.callDelegatePassArrayWithOutLength(delegatePassArrayWithOutLength);
            verify(actualArray, null, "myArray");
        }
    });

    runner.addTest({
        id: 34,
        desc: 'Delegate FillArray With [in] length attribute',
        pri: '0',
        test: function () {
            var actualArray = 0
            var actualLength = 0;
            var excpectedNull = false;
            function delegateFillArrayWithInLength(arrayInDelegate, lengthAttribute) {
                logger.comment("*** delegateFillArrayWithInLength - Invoke ***");

                verifyAllZeroItems("arrayInDelegate", arrayInDelegate, excpectedNull ? 0 : actualArray.length, excpectedNull);
                verify(lengthAttribute, actualLength, "lengthAttribute");

                for (var i = 0; i < actualLength; i++) {
                    arrayInDelegate[i] = actualArray[i];
                }

                logger.comment("*** delegateFillArrayWithInLength - Exit ***");
            }

            logger.comment("Call FillArray with size = 4  length = 2");
            var myAnimal = new Animals.Animal(1);
            actualArray = [12, 23, 34, 45];
            actualLength = 2;
            myAnimal.passArrayWithInLength(actualArray, actualLength);
            myAnimal.callDelegateFillArrayWithInLength(delegateFillArrayWithInLength);
            var resultArray = myAnimal.pureReceiveArray();
            verifyArrayItems("resultArray", resultArray, [12, 23, 0, 0]);

            logger.comment("Call FillArray with size = 4  length = 4");
            actualArray = [12, 23, 34, 45];
            actualLength = 4;
            myAnimal.passArrayWithInLength(actualArray, actualLength);
            myAnimal.callDelegateFillArrayWithInLength(delegateFillArrayWithInLength);
            var resultArray = myAnimal.pureReceiveArray();
            verifyArrayItems("resultArray", resultArray, [12, 23, 34, 45]);

            logger.comment("Call FillArray with size = 0  length = 0");
            myAnimal.purePassArray([]);
            excpectedNull = true;
            actualLength = 0;
            myAnimal.callDelegateFillArrayWithInLength(delegateFillArrayWithInLength);
            var resultArray = myAnimal.pureReceiveArray();
            verify(resultArray, null, "resultArray");
        }
    });

    runner.addTest({
        id: 35,
        desc: 'Delegate FillArray With [out] length attribute',
        pri: '0',
        test: function () {
            var actualArray = 0
            var actualLength = 0;
            var excpectedNull = false;
            function delegateFillArrayWithOutLength(arrayInDelegate) {
                logger.comment("*** delegateFillArrayWithOutLength - Invoke ***");

                verifyAllZeroItems("arrayInDelegate", arrayInDelegate, excpectedNull ? 0 : actualArray.length, excpectedNull);
                for (var i = 0; i < actualLength; i++) {
                    arrayInDelegate[i] = actualArray[i];
                }
                logger.comment("returnLength : " + actualLength);

                logger.comment("*** delegateFillArrayWithOutLength - Exit ***");

                return actualLength;
            }

            var myAnimal = new Animals.Animal(1);
            myAnimal.purePassArray([11, 22, 33, 44]);

            logger.comment("Call FillArray with size = 4  length = 2");
            actualArray = [12, 23, 34, 45];
            actualLength = 2;
            myAnimal.callDelegateFillArrayWithOutLength(delegateFillArrayWithOutLength);
            var resultArray = myAnimal.pureReceiveArray();
            verifyArrayItems("resultArray", resultArray, [12, 23, 0, 0]);

            logger.comment("Call FillArray with size = 4  length = 4");
            actualLength = 4;
            myAnimal.callDelegateFillArrayWithOutLength(delegateFillArrayWithOutLength);
            var resultArray = myAnimal.pureReceiveArray();
            verifyArrayItems("resultArray", resultArray, [12, 23, 34, 45]);

            logger.comment("Call FillArray with size = 0  length = 0");
            myAnimal.purePassArray([]);
            actualArray = null;
            actualLength = 0;
            excpectedNull = true;
            myAnimal.callDelegateFillArrayWithOutLength(delegateFillArrayWithOutLength);
            var resultArray = myAnimal.pureReceiveArray();
            verify(resultArray, null, "resultArray");
        }
    });

    runner.addTest({
        id: 36,
        desc: 'Delegate ReceiveArray With [in] length attribute',
        pri: '0',
        test: function () {
            var actualArray = 0
            var actualLength = 0;
            function delegateReceiveArrayWithInLength(lengthAttribute) {
                logger.comment("*** delegateReceiveArrayWithInLength - Invoke ***");

                verify(lengthAttribute, actualLength, "lengthAttribute");
                dumpArrayItems("returnArray", actualArray);
                logger.comment("*** delegateReceiveArrayWithInLength - Exit ***");
                return actualArray;
            }

            logger.comment("Call ReceiveArray with size = 4  length = 2");
            var myAnimal = new Animals.Animal(1);
            actualArray = [12, 23, 34, 45];
            actualLength = 2;
            myAnimal.passArrayWithInLength([99, 88, 77, 66], actualLength);
            myAnimal.callDelegateReceiveArrayWithInLength(delegateReceiveArrayWithInLength);
            var resultArray = myAnimal.pureReceiveArray();
            verifyArrayItems("resultArray", resultArray, [12, 23, 0, 0]);

            logger.comment("Call ReceiveArray with size = 4  length = 4");
            actualArray = [12, 23, 34, 45];
            actualLength = 4;
            myAnimal.passArrayWithInLength([99, 88, 77, 66], actualLength);
            myAnimal.callDelegateReceiveArrayWithInLength(delegateReceiveArrayWithInLength);
            var resultArray = myAnimal.pureReceiveArray();
            verifyArrayItems("resultArray", resultArray, [12, 23, 34, 45]);

            logger.comment("Call ReceiveArray with size = 0  length = 0");
            actualArray = null;
            actualLength = 0;
            myAnimal.purePassArray([]);
            myAnimal.callDelegateReceiveArrayWithInLength(delegateReceiveArrayWithInLength);
            var resultArray = myAnimal.pureReceiveArray();
            verify(resultArray, null, "resultArray");
        }
    });

    runner.addTest({
        id: 37,
        desc: 'Delegate ReceiveArray With [out] length attribute',
        pri: '0',
        test: function () {
            var actualArray = 0
            var actualLength = 0;
            function delegateReceiveArrayWithOutLength() {
                logger.comment("*** delegateReceiveArrayWithOutLength - Invoke ***");

                dumpArrayItems("returnArray", actualArray);
                logger.comment("returnLength : " + actualLength);

                logger.comment("*** delegateReceiveArrayWithOutLength - Exit ***");

                var outVar = new Object();
                outVar.value = actualArray;
                outVar.lengthValue = actualLength;

                return outVar;
            }

            logger.comment("Call ReceiveArray with size = 4  length = 2");
            var myAnimal = new Animals.Animal(1);
            actualArray = [12, 23, 34, 45];
            actualLength = 2;
            myAnimal.callDelegateReceiveArrayWithOutLength(delegateReceiveArrayWithOutLength);
            var resultArray = myAnimal.pureReceiveArray();
            verifyArrayItems("resultArray", resultArray, [12, 23, 0, 0]);

            logger.comment("Call ReceiveArray with size = 4  length = 4");
            actualArray = [12, 23, 34, 45];
            actualLength = 4;
            myAnimal.callDelegateReceiveArrayWithOutLength(delegateReceiveArrayWithOutLength);
            var resultArray = myAnimal.pureReceiveArray();
            verifyArrayItems("resultArray", resultArray, [12, 23, 34, 45]);

            logger.comment("Call ReceiveArray with size = 0  length = 0");
            actualArray = null;
            actualLength = 0;
            myAnimal.callDelegateReceiveArrayWithOutLength(delegateReceiveArrayWithOutLength);
            var resultArray = myAnimal.pureReceiveArray();
            verify(resultArray, null, "resultArray");
        }
    });

    runner.addTest({
        id: 38,
        desc: 'PassArray With [out] length attribute which has retVal attribute too',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);

            logger.comment("Call PassArray JsArray");
            var elementsRead = myAnimal.passArrayWithOutLengthWithRetValLength([1, 2, 3, 4, 5, 6, 7, 8, 9]);
            var resultArray = myAnimal.pureReceiveArray();
            verify(elementsRead, 9, "elementsRead");
            verifyArrayItems("resultArray", resultArray, [1, 2, 3, 4, 5, 6, 7, 8, 9]);

            logger.comment("Call PassArray Int32Array");
            elementsRead = myAnimal.passArrayWithOutLengthWithRetValLength(new Int32Array([11, 22, 33, 44]));
            resultArray = myAnimal.pureReceiveArray();
            verify(elementsRead, 4, "elementsRead");
            verifyArrayItems("resultArray", resultArray, [11, 22, 33, 44]);

            logger.comment("Call PassArray with 0 elements on null");
            elementsRead = myAnimal.passArrayWithOutLengthWithRetValLength(null);
            var resultArray = myAnimal.pureReceiveArray();
            verify(elementsRead, 0, "elementsRead");
            verify(resultArray, null, "resultArray");
        }
    });

    runner.addTest({
        id: 39,
        desc: 'FillArray With [out] length attribute which has retVal attribute too',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);
            myAnimal.purePassArray([12, 23, 34, 45]);

            logger.comment("Call FillArray with 4 elements on JsArray of 9 elements");
            var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            var elementsRead = myAnimal.fillArrayWithOutLengthWithRetValLength(myArray);
            verify(elementsRead, 4, "elementsRead");
            verifyArrayItems("myArray", myArray, [12, 23, 34, 45, 0, 0, 0, 0, 0]);

            logger.comment("Call FillArray with 2 elements on JsArray of 2 elements");
            var myArray = [1, 2];
            var elementsRead = myAnimal.fillArrayWithOutLengthWithRetValLength(myArray);
            verify(elementsRead, 2, "elementsRead");
            verifyArrayItems("myArray", myArray, [12, 23]);

            logger.comment("Call FillArray with 4 elements on Int32Array of 9 elements");
            var myArray = new Int32Array([1, 2, 3, 4, 5, 6, 7, 8, 9]);
            elementsRead = myAnimal.fillArrayWithOutLengthWithRetValLength(myArray);
            verify(elementsRead, 4, "elementsRead");
            verifyArrayItems("myArray", myArray, [12, 23, 34, 45, 0, 0, 0, 0, 0]);

            logger.comment("Call FillArray with 2 elements on Int32Array of 2 elements");
            var myArray = new Int32Array([1, 2]);
            var elementsRead = myAnimal.fillArrayWithOutLengthWithRetValLength(myArray);
            verify(elementsRead, 2, "elementsRead");
            verifyArrayItems("myArray", myArray, [12, 23]);

            logger.comment("Call FillArray with 0 elements on null");
            elementsRead = myAnimal.fillArrayWithOutLengthWithRetValLength(null);
            verify(elementsRead, 0, "elementsRead");
        }
    });

    runner.addTest({
        id: 40,
        desc: 'ReceiveArray With [out] length attribute which has retVal attribute too',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);

            logger.comment("Call ReceiveArray with size = 4  length = 2");
            myAnimal.passArrayWithInLength([12, 23, 34, 45], 2);
            var result = myAnimal.receiveArrayWithOutLengthWithRetValLength();
            logger.comment(result);
            verify(result.lengthValue, 2, "elementsRead");
            verifyArrayItems("myArray", result.value, [12, 23, 0, 0]);

            logger.comment("Call ReceiveArray with size = 4  length = 4");
            myAnimal.passArrayWithInLength([12, 23, 34, 45], 4);
            var result = myAnimal.receiveArrayWithOutLengthWithRetValLength();
            verify(result.lengthValue, 4, "elementsRead");
            verifyArrayItems("myArray", result.value, [12, 23, 34, 45]);

            logger.comment("Call ReceiveArray with size = 0  length = 0");
            myAnimal.purePassArray([]);
            var result = myAnimal.receiveArrayWithOutLengthWithRetValLength();
            verify(result.lengthValue, 0, "elementsRead");
            verify(result.value, null, "myArray");
        }
    });

    runner.addTest({
        id: 41,
        desc: 'Delegate PassArray With [out] length attribute which has retVal attribute too',
        pri: '0',
        test: function () {
            var actualArray = 0
            var sendLength;
            function delegatePassArrayWithOutLengthWithRetValLength(arrayInDelegate) {
                logger.comment("*** delegatePassArrayWithOutLengthWithRetValLength - Invoke ***");
                dumpArrayItems("arrayInDelegate", arrayInDelegate);

                actualArray = arrayInDelegate;

                logger.comment("*** delegatePassArrayWithOutLengthWithRetValLength - Exit ***");
                return sendLength;
            }

            logger.comment("Call PassArray with size = 4  length = 2");
            var myAnimal = new Animals.Animal(1);
            myAnimal.passArrayWithInLength([12, 23, 34, 45], 2);
            sendLength = 2;
            myAnimal.callDelegatePassArrayWithOutLengthWithRetValLength(delegatePassArrayWithOutLengthWithRetValLength);
            verifyArrayItems("myArray", actualArray, [12, 23, 0, 0]);

            logger.comment("Call PassArray with size = 4  length = 4");
            myAnimal.passArrayWithInLength([12, 23, 34, 45], 4);
            sendLength = 4;
            myAnimal.callDelegatePassArrayWithOutLengthWithRetValLength(delegatePassArrayWithOutLengthWithRetValLength);
            verifyArrayItems("myArray", actualArray, [12, 23, 34, 45]);

            logger.comment("Call PassArray with size = 0  length = 0");
            myAnimal.purePassArray([]);
            sendLength = 0;
            myAnimal.callDelegatePassArrayWithOutLengthWithRetValLength(delegatePassArrayWithOutLengthWithRetValLength);
            verify(actualArray, null, "myArray");
        }
    });

    runner.addTest({
        id: 42,
        desc: 'Delegate FillArray With [out] length attribute which has retVal attribute too',
        pri: '0',
        test: function () {
            var actualArray = 0
            var actualLength = 0;
            var excpectedNull = false;
            function delegateFillArrayWithOutLengthWithRetValLength(arrayInDelegate) {
                logger.comment("*** delegateFillArrayWithOutLengthWithRetValLength - Invoke ***");

                verifyAllZeroItems("arrayInDelegate", arrayInDelegate, excpectedNull ? 0 : actualArray.length, excpectedNull);
                for (var i = 0; i < actualLength; i++) {
                    arrayInDelegate[i] = actualArray[i];
                }
                logger.comment("returnLength : " + actualLength);

                logger.comment("*** delegateFillArrayWithOutLengthWithRetValLength - Exit ***");

                return actualLength;
            }

            var myAnimal = new Animals.Animal(1);
            myAnimal.purePassArray([11, 22, 33, 44]);

            logger.comment("Call FillArray with size = 4  length = 2");
            actualArray = [12, 23, 34, 45];
            actualLength = 2;
            myAnimal.callDelegateFillArrayWithOutLengthWithRetValLength(delegateFillArrayWithOutLengthWithRetValLength);
            var resultArray = myAnimal.pureReceiveArray();
            verifyArrayItems("resultArray", resultArray, [12, 23, 0, 0]);

            logger.comment("Call FillArray with size = 4  length = 4");
            actualLength = 4;
            myAnimal.callDelegateFillArrayWithOutLengthWithRetValLength(delegateFillArrayWithOutLengthWithRetValLength);
            var resultArray = myAnimal.pureReceiveArray();
            verifyArrayItems("resultArray", resultArray, [12, 23, 34, 45]);

            logger.comment("Call FillArray with size = 0  length = 0");
            myAnimal.purePassArray([]);
            actualArray = null;
            actualLength = 0;
            excpectedNull = true;
            myAnimal.callDelegateFillArrayWithOutLengthWithRetValLength(delegateFillArrayWithOutLengthWithRetValLength);
            var resultArray = myAnimal.pureReceiveArray();
            verify(resultArray, null, "resultArray");
        }
    });

    runner.addTest({
        id: 43,
        desc: 'Delegate ReceiveArray With [out] length attribute which has retVal attribute too',
        pri: '0',
        test: function () {
            var actualArray = 0
            var actualLength = 0;
            function delegateReceiveArrayWithOutLengthWithRetValLength() {
                logger.comment("*** delegateReceiveArrayWithOutLengthWithRetValLength - Invoke ***");

                dumpArrayItems("returnArray", actualArray);
                logger.comment("returnLength : " + actualLength);

                logger.comment("*** delegateReceiveArrayWithOutLengthWithRetValLength - Exit ***");

                var outVar = new Object();
                outVar.value = actualArray;
                outVar.lengthValue = actualLength;

                return outVar;
            }

            logger.comment("Call ReceiveArray with size = 4  length = 2");
            var myAnimal = new Animals.Animal(1);
            actualArray = [12, 23, 34, 45];
            actualLength = 2;
            myAnimal.callDelegateReceiveArrayWithOutLengthWithRetValLength(delegateReceiveArrayWithOutLengthWithRetValLength);
            var resultArray = myAnimal.pureReceiveArray();
            verifyArrayItems("resultArray", resultArray, [12, 23, 0, 0]);

            logger.comment("Call ReceiveArray with size = 4  length = 4");
            actualArray = [12, 23, 34, 45];
            actualLength = 4;
            myAnimal.callDelegateReceiveArrayWithOutLengthWithRetValLength(delegateReceiveArrayWithOutLengthWithRetValLength);
            var resultArray = myAnimal.pureReceiveArray();
            verifyArrayItems("resultArray", resultArray, [12, 23, 34, 45]);

            logger.comment("Call ReceiveArray with size = 0  length = 0");
            actualArray = null;
            actualLength = 0;
            myAnimal.callDelegateReceiveArrayWithOutLengthWithRetValLength(delegateReceiveArrayWithOutLengthWithRetValLength);
            var resultArray = myAnimal.pureReceiveArray();
            verify(resultArray, null, "resultArray");
        }
    });

    runner.addTest({
        id: 44,
        desc: 'PassArray With [out] length attribute which has random parameter with retVal attribute',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);

            logger.comment("Call PassArray JsArray");
            var result = myAnimal.passArrayWithOutLengthWithRetValRandomParam([1, 2, 3, 4, 5, 6, 7, 8, 9]);
            var resultArray = myAnimal.pureReceiveArray();
            verify(result.lengthValue, 9, "elementsRead");
            verify(result.randomRetVal, 100, "random parameter");
            verifyArrayItems("resultArray", resultArray, [1, 2, 3, 4, 5, 6, 7, 8, 9]);

            logger.comment("Call PassArray Int32Array");
            result = myAnimal.passArrayWithOutLengthWithRetValRandomParam(new Int32Array([11, 22, 33, 44]));
            resultArray = myAnimal.pureReceiveArray();
            verify(result.lengthValue, 4, "elementsRead");
            verify(result.randomRetVal, 100, "random parameter");
            verifyArrayItems("resultArray", resultArray, [11, 22, 33, 44]);

            logger.comment("Call PassArray with 0 elements on null");
            result = myAnimal.passArrayWithOutLengthWithRetValRandomParam(null);
            var resultArray = myAnimal.pureReceiveArray();
            verify(result.lengthValue, 0, "elementsRead");
            verify(result.randomRetVal, 100, "random parameter");
            verify(resultArray, null, "resultArray");
        }
    });

    runner.addTest({
        id: 45,
        desc: 'FillArray With [out] length attribute which has random parameter with retVal attribute',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);
            myAnimal.purePassArray([12, 23, 34, 45]);

            logger.comment("Call FillArray with 4 elements on JsArray of 9 elements");
            var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            var result = myAnimal.fillArrayWithOutLengthWithRetValRandomParam(myArray);
            verify(result.lengthValue, 4, "elementsRead");
            verify(result.randomRetVal, 100, "random parameter");
            verifyArrayItems("myArray", myArray, [12, 23, 34, 45, 0, 0, 0, 0, 0]);

            logger.comment("Call FillArray with 2 elements on JsArray of 2 elements");
            var myArray = [1, 2];
            var result = myAnimal.fillArrayWithOutLengthWithRetValRandomParam(myArray);
            verify(result.lengthValue, 2, "elementsRead");
            verify(result.randomRetVal, 100, "random parameter");
            verifyArrayItems("myArray", myArray, [12, 23]);

            logger.comment("Call FillArray with 4 elements on Int32Array of 9 elements");
            var myArray = new Int32Array([1, 2, 3, 4, 5, 6, 7, 8, 9]);
            result = myAnimal.fillArrayWithOutLengthWithRetValRandomParam(myArray);
            verify(result.lengthValue, 4, "elementsRead");
            verify(result.randomRetVal, 100, "random parameter");
            verifyArrayItems("myArray", myArray, [12, 23, 34, 45, 0, 0, 0, 0, 0]);

            logger.comment("Call FillArray with 2 elements on Int32Array of 2 elements");
            var myArray = new Int32Array([1, 2]);
            var result = myAnimal.fillArrayWithOutLengthWithRetValRandomParam(myArray);
            verify(result.lengthValue, 2, "elementsRead");
            verify(result.randomRetVal, 100, "random parameter");
            verifyArrayItems("myArray", myArray, [12, 23]);

            logger.comment("Call FillArray with 0 elements on null");
            result = myAnimal.fillArrayWithOutLengthWithRetValRandomParam(null);
            verify(result.lengthValue, 0, "elementsRead");
            verify(result.randomRetVal, 100, "random parameter");
        }
    });

    runner.addTest({
        id: 46,
        desc: 'ReceiveArray With [out] length attribute which has random parameter with retVal attribute',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);

            logger.comment("Call ReceiveArray with size = 4  length = 2");
            myAnimal.passArrayWithInLength([12, 23, 34, 45], 2);
            var result = myAnimal.receiveArrayWithOutLengthWithRetValRandomParam();
            verify(result.lengthValue, 2, "elementsRead");
            verify(result.randomRetVal, 100, "random parameter");
            verifyArrayItems("myArray", result.value, [12, 23, 0, 0]);

            logger.comment("Call ReceiveArray with size = 4  length = 4");
            myAnimal.passArrayWithInLength([12, 23, 34, 45], 4);
            var result = myAnimal.receiveArrayWithOutLengthWithRetValRandomParam();
            verify(result.lengthValue, 4, "elementsRead");
            verify(result.randomRetVal, 100, "random parameter");
            verifyArrayItems("myArray", result.value, [12, 23, 34, 45]);

            logger.comment("Call ReceiveArray with size = 0  length = 0");
            myAnimal.purePassArray([]);
            var result = myAnimal.receiveArrayWithOutLengthWithRetValRandomParam();
            verify(result.lengthValue, 0, "elementsRead");
            verify(result.randomRetVal, 100, "random parameter");
            verify(result.value, null, "myArray");
        }
    });

    runner.addTest({
        id: 47,
        desc: 'Delegate PassArray With [out] length attribute which has random parameter with retVal attribute',
        pri: '0',
        test: function () {
            var actualArray = 0
            var sendLength;
            function delegatePassArrayWithOutLengthWithRetValRandomParam(arrayInDelegate) {
                logger.comment("*** delegatePassArrayWithOutLengthWithRetValRandomParam - Invoke ***");
                dumpArrayItems("arrayInDelegate", arrayInDelegate);

                actualArray = arrayInDelegate;

                logger.comment("*** delegatePassArrayWithOutLengthWithRetValRandomParam - Exit ***");
                var outVar = new Object();
                outVar.lengthValue = sendLength;
                outVar.randomRetVal = 100;
                return outVar;
            }

            logger.comment("Call PassArray with size = 4  length = 2");
            var myAnimal = new Animals.Animal(1);
            myAnimal.passArrayWithInLength([12, 23, 34, 45], 2);
            sendLength = 2;
            var randomParam = myAnimal.callDelegatePassArrayWithOutLengthWithRetValRandomParam(delegatePassArrayWithOutLengthWithRetValRandomParam);
            verifyArrayItems("myArray", actualArray, [12, 23, 0, 0]);
            verify(randomParam, 100, "randomParam");

            logger.comment("Call PassArray with size = 4  length = 4");
            myAnimal.passArrayWithInLength([12, 23, 34, 45], 4);
            sendLength = 4;
            randomParam = myAnimal.callDelegatePassArrayWithOutLengthWithRetValRandomParam(delegatePassArrayWithOutLengthWithRetValRandomParam);
            verifyArrayItems("myArray", actualArray, [12, 23, 34, 45]);
            verify(randomParam, 100, "randomParam");

            logger.comment("Call PassArray with size = 0  length = 0");
            myAnimal.purePassArray([]);
            sendLength = 0;
            randomParam = myAnimal.callDelegatePassArrayWithOutLengthWithRetValRandomParam(delegatePassArrayWithOutLengthWithRetValRandomParam);
            verify(actualArray, null, "myArray");
            verify(randomParam, 100, "randomParam");
        }
    });

    runner.addTest({
        id: 48,
        desc: 'Delegate FillArray With [out] length attribute which has random parameter with retVal attribute',
        pri: '0',
        test: function () {
            var actualArray = 0
            var actualLength = 0;
            var excpectedNull = false;
            function delegateFillArrayWithOutLengthWithRetValRandomParam(arrayInDelegate) {
                logger.comment("*** delegateFillArrayWithOutLengthWithRetValRandomParam - Invoke ***");

                verifyAllZeroItems("arrayInDelegate", arrayInDelegate, excpectedNull ? 0 : actualArray.length, excpectedNull);
                for (var i = 0; i < actualLength; i++) {
                    arrayInDelegate[i] = actualArray[i];
                }
                logger.comment("returnLength : " + actualLength);

                logger.comment("*** delegateFillArrayWithOutLengthWithRetValRandomParam - Exit ***");

                var outVar = new Object();
                outVar.lengthValue = actualLength;
                outVar.randomRetVal = 100;
                return outVar;
            }

            var myAnimal = new Animals.Animal(1);
            myAnimal.purePassArray([11, 22, 33, 44]);

            logger.comment("Call FillArray with size = 4  length = 2");
            actualArray = [12, 23, 34, 45];
            actualLength = 2;
            var randomParam = myAnimal.callDelegateFillArrayWithOutLengthWithRetValRandomParam(delegateFillArrayWithOutLengthWithRetValRandomParam);
            var resultArray = myAnimal.pureReceiveArray();
            verifyArrayItems("resultArray", resultArray, [12, 23, 0, 0]);
            verify(randomParam, 100, "randomParam");

            logger.comment("Call FillArray with size = 4  length = 4");
            actualLength = 4;
            randomParam = myAnimal.callDelegateFillArrayWithOutLengthWithRetValRandomParam(delegateFillArrayWithOutLengthWithRetValRandomParam);
            var resultArray = myAnimal.pureReceiveArray();
            verifyArrayItems("resultArray", resultArray, [12, 23, 34, 45]);
            verify(randomParam, 100, "randomParam");

            logger.comment("Call FillArray with size = 0  length = 0");
            myAnimal.purePassArray([]);
            actualArray = null;
            actualLength = 0;
            excpectedNull = true;
            randomParam = myAnimal.callDelegateFillArrayWithOutLengthWithRetValRandomParam(delegateFillArrayWithOutLengthWithRetValRandomParam);
            var resultArray = myAnimal.pureReceiveArray();
            verify(resultArray, null, "resultArray");
            verify(randomParam, 100, "randomParam");
        }
    });

    runner.addTest({
        id: 49,
        desc: 'Delegate ReceiveArray With [out] length attribute which has random parameter with retVal attribute',
        pri: '0',
        test: function () {
            var actualArray = 0
            var actualLength = 0;
            function delegateReceiveArrayWithOutLengthWithRetValRandomParam() {
                logger.comment("*** delegateReceiveArrayWithOutLengthWithRetValRandomParam - Invoke ***");

                dumpArrayItems("returnArray", actualArray);
                logger.comment("returnLength : " + actualLength);

                logger.comment("*** delegateReceiveArrayWithOutLengthWithRetValRandomParam - Exit ***");

                var outVar = new Object();
                outVar.value = actualArray;
                outVar.lengthValue = actualLength;
                outVar.randomRetVal = 100;
                return outVar;
            }

            logger.comment("Call ReceiveArray with size = 4  length = 2");
            var myAnimal = new Animals.Animal(1);
            actualArray = [12, 23, 34, 45];
            actualLength = 2;
            var randomParam = myAnimal.callDelegateReceiveArrayWithOutLengthWithRetValRandomParam(delegateReceiveArrayWithOutLengthWithRetValRandomParam);
            var resultArray = myAnimal.pureReceiveArray();
            verifyArrayItems("resultArray", resultArray, [12, 23, 0, 0]);
            verify(randomParam, 100, "randomParam");

            logger.comment("Call ReceiveArray with size = 4  length = 4");
            actualArray = [12, 23, 34, 45];
            actualLength = 4;
            var randomParam = myAnimal.callDelegateReceiveArrayWithOutLengthWithRetValRandomParam(delegateReceiveArrayWithOutLengthWithRetValRandomParam);
            var resultArray = myAnimal.pureReceiveArray();
            verifyArrayItems("resultArray", resultArray, [12, 23, 34, 45]);
            verify(randomParam, 100, "randomParam");

            logger.comment("Call ReceiveArray with size = 0  length = 0");
            actualArray = null;
            actualLength = 0;
            var randomParam = myAnimal.callDelegateReceiveArrayWithOutLengthWithRetValRandomParam(delegateReceiveArrayWithOutLengthWithRetValRandomParam);
            var resultArray = myAnimal.pureReceiveArray();
            verify(resultArray, null, "resultArray");
            verify(randomParam, 100, "randomParam");
        }
    });

    runner.addTest({
        id: 50,
        desc: 'Array property on runtimeclass',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal();

            var myArray = [1, 2, 3, 4];
            myAnimal.myArrayProp = myArray;
            var propGet = myAnimal.myArrayProp;
            var str = propGet.toString();
            // Support ES5/ES6 behavior
            if (str === "[object Int32Array]") {
                verify(str, "[object Int32Array]", "myAnimal.myArrayProp.toString()");
            } else {
                verify(str, "1,2,3,4", "myAnimal.myArrayProp.toString()");
            }
            verify(propGet.length, myArray.length, "myAnimal.myArrayProp.length");
            for (i = 0; i < myArray.length; i++) {
                verify(propGet[i], myArray[i], "myAnimal.myArrayProp[" + i + "]");
            }
        }
    });

    runner.addTest({
        id: 51,
        desc: 'Array property on static interface',
        pri: '0',
        test: function () {
            var myArray = [1, 2, 3, 4];
            Animals.Animal.myStaticArrayProp = myArray;
            var propGet = Animals.Animal.myStaticArrayProp;
            var str = propGet.toString();
            
            // Support ES5/ES6 behavior
            if (str === "[object Int32Array]") {
                verify(str, "[object Int32Array]", "Animals.Animal.myStaticArrayProp.toString()");
            } else {
                verify(str, "1,2,3,4", "Animals.Animal.myStaticArrayProp.toString()");
            }
            
            verify(propGet.length, myArray.length, "Animals.Animal.myStaticArrayProp.length");
            for (i = 0; i < myArray.length; i++) {
                verify(propGet[i], myArray[i], "Animals.Animal.myStaticArrayProp[" + i + "]");
            }
        }
    });

    Loader42_FileName = 'Arrays marshaling and projection test case';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
