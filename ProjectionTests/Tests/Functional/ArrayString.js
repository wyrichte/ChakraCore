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

    function verifyStringVectorAndArrayItems(myVectorString, myVector, myArrayString, myArray) {
        verify(myVector.length, myArray.length, "Number of array Items and Vector Items")

        for (var iIndex = 0; iIndex < myVector.length; iIndex++) {
            verify(myVector[iIndex].toString(), myArray[iIndex], myVectorString + '[' + iIndex + '].toString() and ' + myArrayString + '[' + iIndex + ']');
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
        Array.prototype.arrayTestProperty = "Object's Array Test Property";
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
            verify(myArray[iIndex], "", myArrayString + '[' + iIndex + ']');
        }
    }

    runner.addTest({
        id: 1,
        desc: 'MarshalInArray',
        pri: '0',
        test: function () {
            logger.comment("var myArray = ['white','Red', 'Blue', 'Yellow', 'Pink', 'Black', 'Orange', 'Purple', 'Green'];");
            var myArray = ['white', 'Red', 'Blue', 'Yellow', 'Pink', 'Black', 'Orange', 'Purple', 'Green'];
            logger.comment("var myAnimal = new Animals.Animal(1);");
            var myAnimal = new Animals.Animal(1);

            logger.comment("var myVector = myAnimal.passArrayHSTRING(myArray);");
            var myVector = myAnimal.passArrayHSTRING(myArray);
            logger.comment('verifyVectorAndArrayItems("myVector", myVector, "myArray", myArray)');
            verifyVectorAndArrayItems("myVector", myVector, "myArray", myArray)

            logger.comment('myArray = null');
            myArray = null;

            logger.comment("myVector = myAnimal.passArrayHSTRING(myArray);");
            myVector = myAnimal.passArrayHSTRING(myArray);
            logger.comment('verifyVectorAndArrayItems("myVector", myVector, "new Array()", new Array())');
            verifyVectorAndArrayItems("myVector", myVector, "new Array()", new Array())

            logger.comment("myVector = myAnimal.passArrayHSTRING(10);");

            var foundException = false;
            try {
                myVector = myAnimal.passArrayHSTRING(10);
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
            logger.comment("var myArray = ['white','Red', 'Blue', 'Yellow', 'Pink', 'Black', 'Orange', 'Purple', 'Green'];");
            var myArray = ['white', 'Red', 'Blue', 'Yellow', 'Pink', 'Black', 'Orange', 'Purple', 'Green'];
            logger.comment("var myAnimal = new Animals.Animal(1);");
            var myAnimal = new Animals.Animal(1);

            logger.comment("var myVector = myAnimal.passArrayHSTRING(myArray);");
            var myVector = myAnimal.passArrayHSTRING(myArray);
            logger.comment('verifyVectorAndArrayItems("myVector", myVector, "myArray", myArray)');
            verifyVectorAndArrayItems("myVector", myVector, "myArray", myArray);

            logger.comment("var myNewArray = new Array(9);");
            var myNewArray = new Array(9);
            logger.comment("myVector = myAnimal.fillArrayHSTRING(myNewArray);");
            myVector = myAnimal.fillArrayHSTRING(myNewArray);
            logger.comment('verifyVectorAndArrayItems("myNewArray", myNewArray, "myVector", myVector)');
            verifyVectorAndArrayItems("myNewArray", myNewArray, "myVector", myVector);
            verify(Array.isArray(myNewArray), true, "Array.isArray(myNewArray)");

            logger.comment("myNewArray = [11, 22, 33, 44];");
            myNewArray = [11, 22, 33, 44];
            logger.comment("myVector = myAnimal.fillArrayHSTRING(myNewArray);");
            myVector = myAnimal.fillArrayHSTRING(myNewArray);
            logger.comment('verifyVectorAndArrayItems("myNewArray", myNewArray, "myVector", myVector)');
            verifyVectorAndArrayItems("myNewArray", myNewArray, "myVector", myVector);
            verify(Array.isArray(myNewArray), true, "Array.isArray(myNewArray)");

            logger.comment("myNewArray.length = 0;");
            myNewArray.length = 0;
            logger.comment("myVector = myAnimal.fillArrayHSTRING(myNewArray);");
            myVector = myAnimal.fillArrayHSTRING(myNewArray);
            logger.comment('verifyVectorAndArrayItems("myNewArray", myNewArray, "myVector", myVector)');
            verifyVectorAndArrayItems("myNewArray", myNewArray, "myVector", myVector);
            verify(Array.isArray(myNewArray), true, "Array.isArray(myNewArray)");

            logger.comment("myVector = myAnimal.fillArrayHSTRING(10);");

            var foundException = false;
            try {
                myVector = myAnimal.fillArrayHSTRING(10);
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
            logger.comment("var myArray = ['white','Red', 'Blue', 'Yellow', 'Pink', 'Black', 'Orange', 'Purple', 'Green'];");
            var myArray = ['white', 'Red', 'Blue', 'Yellow', 'Pink', 'Black', 'Orange', 'Purple', 'Green'];
            logger.comment("var myAnimal = new Animals.Animal(1);");
            var myAnimal = new Animals.Animal(1);

            logger.comment("var myVector = myAnimal.passArrayHSTRING(myArray);");
            var myVector = myAnimal.passArrayHSTRING(myArray);
            logger.comment('verifyVectorAndArrayItems("myVector", myVector, "myArray", myArray)');
            verifyVectorAndArrayItems("myVector", myVector, "myArray", myArray)

            logger.comment("var myResult = myAnimal.receiveArrayHSTRING();");
            var myResult = myAnimal.receiveArrayHSTRING();

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
            logger.comment(myArrayProjection.toString());
            verify(myArrayProjection.toString(), "[object StringArray]", "myArrayProjection == '[object StringArray]'");

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

            setAndCheckProperty(myArrayProjection, "6", "Grey", true);
            setAndCheckProperty(myArrayProjection, "11", "Silver", false);

            var attributes = { writable: true, enumerable: true, configurable: true, value: 10 }
            addAndCheckProperty(myArrayProjection, "4", false, attributes);
            addAndCheckProperty(myArrayProjection, "10", false, attributes);
            addAndCheckProperty(myArrayProjection, "14", false, attributes);

            deleteAndCheckProperty(myArrayProjection, "4", false);

            // Should not be able to add and manipulate expandos
            addAndCheckProperty(myArrayProjection, "FavoriteRecipe", false);
            setAndCheckProperty(myArrayProjection, "FavoriteRecipe", "Almond Cake", false);

            // Check if we can marshal in the projected array as inout parameter:
            logger.comment("myVector = myAnimal.fillArrayHSTRING(myArrayProjection);");
            myVector = myAnimal.fillArrayHSTRING(myArrayProjection);
            logger.comment('verifyVectorAndArrayItems("myVector", myVector, "myArrayProjection", myArrayProjection)');
            verifyVectorAndArrayItems("myVector", myVector, "myArrayProjection", myArrayProjection)
            verify(Array.isArray(myArrayProjection), false, "Array.isArray(myArrayProjection)");
            dumpArray("myArrayProjection", myArrayProjection);
            verifyArrayPrototypeInChain("myArrayProjection", myArrayProjection);

            // Check if we can marshal in the projected array as in parameter:
            logger.comment("myArrayProjection[4] = 'Gold'");
            myArrayProjection[4] = 'Gold'
            logger.comment("myVector = myAnimal.passArrayHSTRING(myArrayProjection);");
            myVector = myAnimal.passArrayHSTRING(myArrayProjection);
            logger.comment('verifyVectorAndArrayItems("myVector", myVector, "myArrayProjection", myArrayProjection)');
            verifyVectorAndArrayItems("myVector", myVector, "myArrayProjection", myArrayProjection)

            // Verify marshalling into different type throws exception - in param
            verify.exception(function () {
                logger.comment("myVector = myAnimal.passArray(myArrayProjection);");
                myVector = myAnimal.passArray(myArrayProjection);
            }, TypeError, "myAnimal.passArray(myArrayProjection)");

            // Verify marshalling into different type throws exception - inout param
            verify.exception(function () {
                logger.comment("myVector = myAnimal.fillArray(myArrayProjection);");
                myVector = myAnimal.fillArray(myArrayProjection);
            }, TypeError, "myAnimal.fillArray(myArrayProjection)");
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
            logger.comment("myResult = myAnimal.passArrayHSTRING(null);");
            myResult = myAnimal.passArrayHSTRING(null);

            logger.comment("myResult = myAnimal.receiveArrayHSTRING();");
            myResult = myAnimal.receiveArrayHSTRING();

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
            function delegatePassArray(animal, stringArray) {
                logger.comment("*** delegatePassArray Delegate");
                logger.comment("var stringVector = delegatePassArray.outVector;");
                var stringVector = delegatePassArray.outVector;
                logger.comment('verifyVectorAndArrayItems("stringArray", stringArray, "stringVector", stringVector)');
                verifyVectorAndArrayItems("stringArray", stringArray, "stringVector", stringVector)

                // Basic array tests
                verify(Array.isArray(stringArray), false, "Array.isArray(stringArray)");
                dumpArray("stringArray", stringArray);
                verifyArrayPrototypeInChain("stringArray", stringArray);

                logger.comment("var a = Array.apply(this, stringArray)");
                var a = Array.apply(this, stringArray);
                dumpArray("a", a);
                dumpArray("stringArray", stringArray);

                logger.comment("a = new Array(stringArray)");
                a = new Array(stringArray);
                dumpArray("a", a);
                dumpArray("stringArray", stringArray);

                // Verify length is non writable
                setAndCheckProperty(stringArray, "length", 3, false);

                // Check that we can enumerate over the members of the vector instance
                enumerateOver(stringArray, "ArrayProjection instance: stringArray", true);

                // Check that the property 3 exists and 11 doesnt
                checkHasProperty(stringArray, 3, true);
                checkHasProperty(stringArray, 11, false);

                setAndCheckProperty(stringArray, "6", "Grey", true);
                setAndCheckProperty(stringArray, "11", "Silver", false);

                var attributes = { writable: true, enumerable: true, configurable: true, value: 10 }
                addAndCheckProperty(stringArray, "4", false, attributes);
                addAndCheckProperty(stringArray, "10", false, attributes);
                addAndCheckProperty(stringArray, "14", false, attributes);

                deleteAndCheckProperty(stringArray, "4", false);

                // Should be able to add and manipulate expandos
                addAndCheckProperty(stringArray, "FavoriteRecipe", false);
                setAndCheckProperty(stringArray, "FavoriteRecipe", "Almond Cake", false);

                // Check if we can marshal in the projected array as inout parameter:
                logger.comment("myVector = myAnimal.fillArrayHSTRING(stringArray);");
                myVector = myAnimal.fillArrayHSTRING(stringArray);
                logger.comment('verifyVectorAndArrayItems("myVector", myVector, "stringArray", stringArray)');
                verifyVectorAndArrayItems("myVector", myVector, "stringArray", stringArray)
                verify(Array.isArray(stringArray), false, "Array.isArray(stringArray)");
                dumpArray("stringArray", stringArray);
                verifyArrayPrototypeInChain("stringArray", stringArray);

                // Check if we can marshal in the projected array as in parameter:
                logger.comment("stringArray[4] = 'Gold'");
                stringArray[4] = 'Gold'
                logger.comment("var myVector = animal.passArrayHSTRING(stringArray);");
                var myVector = animal.passArrayHSTRING(stringArray);
                logger.comment('verifyVectorAndArrayItems("myVector", myVector, "stringArray", stringArray)');
                verifyVectorAndArrayItems("myVector", myVector, "stringArray", stringArray)

                // Verify marshalling into different type throws exception - in param
                verify.exception(function () {
                    logger.comment("myVector = myAnimal.passArray(stringArray);");
                    myVector = myAnimal.passArray(stringArray);
                }, TypeError, "myAnimal.passArray(stringArray)");

                // Verify marshalling into different type throws exception - inout param
                verify.exception(function () {
                    logger.comment("myVector = myAnimal.fillArray(stringArray);");
                    myVector = myAnimal.fillArray(stringArray);
                }, TypeError, "myAnimal.fillArray(stringArray)");
            }

            logger.comment("var myArray = ['white','Red', 'Blue', 'Yellow', 'Pink', 'Black', 'Orange', 'Purple', 'Green'];");
            var myArray = ['white', 'Red', 'Blue', 'Yellow', 'Pink', 'Black', 'Orange', 'Purple', 'Green'];
            logger.comment("var myAnimal = new Animals.Animal(1);");
            var myAnimal = new Animals.Animal(1);

            logger.comment("delegatePassArray.outVector = myAnimal.passArrayHSTRING(myArray);");
            delegatePassArray.outVector = myAnimal.passArrayHSTRING(myArray);

            logger.comment("myAnimal.callDelegatePassArrayHSTRING(delegatePassArray)");
            myAnimal.callDelegatePassArrayHSTRING(delegatePassArray);

            verify(myArray, ['white', 'Red', 'Blue', 'Yellow', 'Pink', 'Black', 'Orange', 'Purple', 'Green'], "myArray");
        }
    });

    runner.addTest({
        id: 6,
        desc: 'Delegate_MarshalInArray_Null',
        pri: '0',
        test: function () {
            function delegatePassArray(animal, stringArray) {
                logger.comment("*** delegatePassArray Delegate");
                logger.comment("var stringVector = delegatePassArray.outVector;");
                var stringVector = delegatePassArray.outVector;
                logger.comment('verifyVectorAndArrayItems("stringArray", stringArray, "null", null)');
                verifyVectorAndArrayItems("stringArray", stringArray, "null", null)
            }

            logger.comment("var myAnimal = new Animals.Animal(1);");
            var myAnimal = new Animals.Animal(1);

            logger.comment('var myArray = null');
            var myArray = null;

            logger.comment("delegatePassArray.outVector = myAnimal.passArrayHSTRING(myArray);");
            delegatePassArray.outVector = myAnimal.passArrayHSTRING(myArray);
            logger.comment("myAnimal.callDelegatePassArrayHSTRING(delegatePassArray)");
            myAnimal.callDelegatePassArrayHSTRING(delegatePassArray);

            verify(myArray, null, "myArray");
        }
    });

    runner.addTest({
        id: 7,
        desc: 'Delegate_ProjectOutArrayByValue',
        pri: '0',
        test: function () {
            function delegateFillArray(animal, stringArray) {
                logger.comment("*** delegateFillArray Delegate");

                verify(Array.isArray(stringArray), false, "Array.isArray(stringArray)");
                verify(stringArray.length, delegateFillArray.arrayLength, "stringArray.length");
                verifyArrayPrototypeInChain("stringArray", stringArray);

                // Check if we can marshal in the projected array as inout parameter:
                logger.comment("myVector = myAnimal.fillArrayHSTRING(stringArray);");
                myVector = myAnimal.fillArrayHSTRING(stringArray);
                logger.comment('verifyVectorAndArrayItems("myVector", myVector, "stringArray", stringArray)');
                verifyVectorAndArrayItems("myVector", myVector, "stringArray", stringArray)
                verify(Array.isArray(stringArray), false, "Array.isArray(stringArray)");
                dumpArray("stringArray", stringArray);
                verifyArrayPrototypeInChain("stringArray", stringArray);

                // Verify length is non writable
                setAndCheckProperty(stringArray, "length", 3, false);

                // Check that we can enumerate over the members of the vector instance
                enumerateOver(stringArray, "ArrayProjection instance: stringArray", true);

                // Check that the property 3 exists and 11 doesnt
                checkHasProperty(stringArray, 3, true);
                checkHasProperty(stringArray, 11, false);

                setAndCheckProperty(stringArray, "6", "Grey", true);
                setAndCheckProperty(stringArray, "6", "Orange", true);
                setAndCheckProperty(stringArray, "11", "Silver", false);

                var attributes = { writable: true, enumerable: true, configurable: true, value: 10 }
                addAndCheckProperty(stringArray, "4", false, attributes);
                addAndCheckProperty(stringArray, "10", false, attributes);
                addAndCheckProperty(stringArray, "14", false, attributes);

                deleteAndCheckProperty(stringArray, "4", false);

                // Should be able to add and manipulate expandos
                addAndCheckProperty(stringArray, "FavoriteRecipe", false);
                setAndCheckProperty(stringArray, "FavoriteRecipe", "Almond Cake", false);

                // Check if we can marshal in the projected array as in parameter:
                logger.comment("stringArray[4] = 'Gold'");
                stringArray[4] = 'Gold'
                logger.comment("var myVector = animal.passArrayHSTRING(stringArray);");
                var myVector = animal.passArrayHSTRING(stringArray);
                logger.comment('verifyVectorAndArrayItems("myVector", myVector, "stringArray", stringArray)');
                verifyVectorAndArrayItems("myVector", myVector, "stringArray", stringArray)

                // Verify marshalling into different type throws exception - in param
                verify.exception(function () {
                    logger.comment("myVector = myAnimal.passArray(stringArray);");
                    myVector = myAnimal.passArray(stringArray);
                }, TypeError, "myAnimal.passArray(stringArray)");

                // Verify marshalling into different type throws exception - inout param
                verify.exception(function () {
                    logger.comment("myVector = myAnimal.fillArray(stringArray);");
                    myVector = myAnimal.fillArray(stringArray);
                }, TypeError, "myAnimal.fillArray(stringArray)");

                logger.comment('stringArray[7] = "Voilet";');
                stringArray[7] = "Voilet";

                // Save the stringArray to see lifetime after this function exit
                logger.comment('delegateFillArray.stringArray = stringArray;');
                delegateFillArray.stringArray = stringArray;
            }

            logger.comment("var myArray = ['white','Red', 'Blue', 'Yellow', 'Pink', 'Black', 'Orange', 'Purple', 'Green'];");
            var myArray = ['white', 'Red', 'Blue', 'Yellow', 'Pink', 'Black', 'Orange', 'Purple', 'Green'];
            logger.comment("var myAnimal = new Animals.Animal(1);");
            var myAnimal = new Animals.Animal(1);

            logger.comment("myAnimal.passArrayHSTRING(myArray);");
            myAnimal.passArrayHSTRING(myArray);
            logger.comment('delegateFillArray.arrayLength = myArray.length;')
            delegateFillArray.arrayLength = myArray.length;
            logger.comment("myAnimal.callDelegateFillArrayHSTRING(delegateFillArray)");
            myAnimal.callDelegateFillArrayHSTRING(delegateFillArray);

            verify(myArray, ['white', 'Red', 'Blue', 'Yellow', 'Pink', 'Black', 'Orange', 'Purple', 'Green'], "myArray");

            logger.comment("myResult = myAnimal.receiveArrayHSTRING();");
            myResult = myAnimal.receiveArrayHSTRING();

            if (typeof Animals._CLROnly === 'undefined') //C# ABI implemenation has issues on array copying so skip this checking
            {
                logger.comment('verifyVectorAndArrayItems("delegateFillArray.stringArray", delegateFillArray.stringArray, "myResult.outVector", myResult.outVector)');
                verifyVectorAndArrayItems("delegateFillArray.stringArray", delegateFillArray.stringArray, "myResult.outVector", myResult.outVector);
            }

            // Try changing the fields outside function - shouldnt affect either abi or existing vector or projected array
            logger.comment('delegateFillArray.stringArray[5] = "Brown"');
            delegateFillArray.stringArray[5] = "Brown";

            verify(myResult.value[5] != 'Brown', true, "myResult.value[5] != 'Brown'");
            logger.comment("myResult = myAnimal.receiveArrayHSTRING();");
            myResult = myAnimal.receiveArrayHSTRING();
            verify(myResult.value[5] != 'Brown', true, "myResult.value[5] != 'Brown'");
        }
    });

    runner.addTest({
        id: 8,
        desc: 'Delegate_ProjectOutArrayByRef_Array',
        pri: '0',
        test: function () {
            function delegateReceiveArray(animal) {
                logger.comment("*** delegateReceiveArray Delegate");
                logger.comment("delegateReceiveArray.myArray = ['white','Red', 'Blue', 'Yellow', 'Pink', 'Black', 'Orange', 'Purple', 'Green', 'Magenta'];");
                delegateReceiveArray.myArray = ['white', 'Red', 'Blue', 'Yellow', 'Pink', 'Black', 'Orange', 'Purple', 'Green', 'Magenta'];
                logger.comment('return delegateReceiveArray.myArray;');
                return delegateReceiveArray.myArray;
            }

            logger.comment("var myAnimal = new Animals.Animal(1);");
            var myAnimal = new Animals.Animal(1);

            logger.comment("myAnimal.callDelegateReceiveArrayHSTRING(delegateReceiveArray)");
            myAnimal.callDelegateReceiveArrayHSTRING(delegateReceiveArray);

            verify(delegateReceiveArray.myArray, ['white', 'Red', 'Blue', 'Yellow', 'Pink', 'Black', 'Orange', 'Purple', 'Green', 'Magenta'], "delegateReceiveArray.myArray");

            logger.comment("myResult = myAnimal.receiveArrayHSTRING();");
            myResult = myAnimal.receiveArrayHSTRING();

            logger.comment("*****");
            dumpArray("myResult.value", myResult.value);
            dumpArray("myResult.outVector", myResult.outVector);
            logger.comment("*****");

            logger.comment('verifyVectorAndArrayItems("myResult.outVector", myResult.outVector, "delegateReceiveArray.myArray", delegateReceiveArray.myArray)');
            verifyVectorAndArrayItems("myResult.outVector", myResult.outVector, "delegateReceiveArray.myArray", delegateReceiveArray.myArray);

            // Try changing the fields outside function - shouldnt affect either abi or existing vector or projected array
            logger.comment('delegateReceiveArray.myArray[5] = "Brown"');
            delegateReceiveArray.myArray[5] = 'Brown';

            verify(myResult.value[5] != 'Brown', true, "myResult.value[5] != 'Brown'");
            logger.comment("myResult = myAnimal.receiveArrayHSTRING();");
            myResult = myAnimal.receiveArrayHSTRING();
            verify(myResult.value[5] != 'Brown', true, "myResult.value[5] != 'Brown'");
        }
    });

    runner.addTest({
        id: 9,
        desc: 'Delegate_ProjectOutArrayByRef_ArrayProjection',
        pri: '0',
        test: function () {
            function delegateReceiveArray(animal) {
                logger.comment("*** delegateReceiveArray Delegate");

                logger.comment("delegateReceiveArray.myArray = ['white','Red', 'Blue', 'Yellow', 'Pink', 'Black', 'Orange', 'Purple', 'Green'];");
                delegateReceiveArray.myArray = ['white', 'Red', 'Blue', 'Yellow', 'Pink', 'Black', 'Orange', 'Purple', 'Green'];
                logger.comment("animal.passArrayHSTRING(delegateReceiveArray.myArray);");
                animal.passArrayHSTRING(delegateReceiveArray.myArray);
                logger.comment("var myResult = animal.receiveArrayHSTRING();");
                var myResult = animal.receiveArrayHSTRING();
                dumpArray("myResult.value", myResult.value);
                logger.comment('delegateReceiveArray.myArray[3] = "Brown";');
                delegateReceiveArray.myArray[3] = "Brown";
                logger.comment('myResult.value[3] = "Brown";');
                myResult.value[3] = "Brown";
                dumpArray("myResult.value", myResult.value);
                logger.comment("delegateReceiveArray.myArrayProjection = myResult.value;");
                delegateReceiveArray.myArrayProjection = myResult.value;
                logger.comment('return delegateReceiveArray.myArrayProjection;');
                return delegateReceiveArray.myArrayProjection;
            }

            logger.comment("var myAnimal = new Animals.Animal(1);");
            var myAnimal = new Animals.Animal(1);

            logger.comment("myAnimal.callDelegateReceiveArrayHSTRING(delegateReceiveArray)");
            myAnimal.callDelegateReceiveArrayHSTRING(delegateReceiveArray);

            logger.comment('verifyVectorAndArrayItems("delegateReceiveArray.myArray", delegateReceiveArray.myArray, "delegateReceiveArray.myArrayProjection", delegateReceiveArray.myArrayProjection)');
            verifyVectorAndArrayItems("delegateReceiveArray.myArray", delegateReceiveArray.myArray, "delegateReceiveArray.myArrayProjection", delegateReceiveArray.myArrayProjection);

            logger.comment("myResult = myAnimal.receiveArrayHSTRING();");
            myResult = myAnimal.receiveArrayHSTRING();
            logger.comment('verifyVectorAndArrayItems("myResult.outVector", myResult.outVector, "delegateReceiveArray.myArrayProjection", delegateReceiveArray.myArrayProjection)');
            verifyVectorAndArrayItems("myResult.outVector", myResult.outVector, "delegateReceiveArray.myArrayProjection", delegateReceiveArray.myArrayProjection);

            // Try changing the fields outside function - shouldnt affect either abi or existing vector or projected array
            logger.comment('delegateReceiveArray.myArrayProjection[5] = "Silver"');
            delegateReceiveArray.myArrayProjection[5] = 'Silver';

            verify(myResult.value[5] != 'Silver', true, "myResult.value[5] != 'Silver'");
            verify(delegateReceiveArray.myArray[5] != 'Silver', true, "delegateReceiveArray.myArray[5] != 'Silver'");
            logger.comment("myResult = myAnimal.receiveArrayHSTRING();");
            myResult = myAnimal.receiveArrayHSTRING();
            verify(myResult.value[5] != 'Silver', true, "myResult.value[5] != 'Silver'");
        }
    });

    runner.addTest({
        id: 11,
        desc: 'PassArray With [in] length attribute',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);

            logger.comment("Call PassArray with 2 elements on JsArray of 4 elements");
            myAnimal.passArrayWithInLengthHSTRING(["Once", "Upon", "a", "Time"], 2);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verifyArrayItems("resultArray", resultArray, ["Once", "Upon", "", ""]);

            logger.comment("Call PassArray with 4 elements on JsArray of 4 elements");
            myAnimal.passArrayWithInLengthHSTRING(["Once", "Upon", "a", "Time"], 4);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verifyArrayItems("resultArray", resultArray, ["Once", "Upon", "a", "Time"]);

            logger.comment("Call PassArray with 11 elements on JsArray of 4 elements");
            verify.exception(function () {
                myAnimal.passArrayWithInLengthHSTRING(["Once", "Upon", "a", "Time"], 11);
            }, TypeError, "PassArray with length > size");

            logger.comment("Call PassArray with 4 elements on StringArray of 4 elements");
            var myArray = resultArray;
            myAnimal.passArrayWithInLengthHSTRING(myArray, 4);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verifyArrayItems("resultArray", resultArray, ["Once", "Upon", "a", "Time"]);

            logger.comment("Call PassArray with 2 elements on StringArray of 4 elements");
            myAnimal.passArrayWithInLengthHSTRING(myArray, 2);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verifyArrayItems("resultArray", resultArray, ["Once", "Upon", "", ""]);

            logger.comment("Call PassArray with 11 elements on StringArray of 4 elements");
            verify.exception(function () {
                myAnimal.passArrayWithInLengthHSTRING(myArray, 11);
            }, TypeError, "PassArray with length > size");

            logger.comment("Call PassArray with 0 elements on null");
            myAnimal.passArrayWithInLengthHSTRING(null, 0);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verify(resultArray, null, "resultArray");

            logger.comment("Call PassArray with 11 elements on null");
            verify.exception(function () {
                myAnimal.passArrayWithInLengthHSTRING(null, 11);
            }, TypeError, "PassArray with length > size");
        }
    });

    runner.addTest({
        id: 12,
        desc: 'PassArray With [out] length attribute',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);

            logger.comment("Call PassArray JsArray");
            var elementsRead = myAnimal.passArrayWithOutLengthHSTRING(["Once", "Upon", "a", "Time"]);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verify(elementsRead, 4, "elementsRead");
            verifyArrayItems("resultArray", resultArray, ["Once", "Upon", "a", "Time"]);

            logger.comment("Call PassArray StringArray");
            elementsRead = myAnimal.passArrayWithOutLengthHSTRING(resultArray);
            resultArray = myAnimal.receiveArrayHSTRING().value;
            verify(elementsRead, 4, "elementsRead");
            verifyArrayItems("resultArray", resultArray, ["Once", "Upon", "a", "Time"]);

            logger.comment("Call PassArray with 0 elements on null");
            elementsRead = myAnimal.passArrayWithOutLengthHSTRING(null);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verify(elementsRead, 0, "elementsRead");
            verify(resultArray, null, "resultArray");
        }
    });

    runner.addTest({
        id: 13,
        desc: 'FillArray With [in] length attribute',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);
            myAnimal.passArrayHSTRING(["Mr. Gold", "Regina", "Mary Margaret", "David Nolan"]);

            logger.comment("Call FillArray with 2 elements on JsArray of 4 elements");
            var myArray = ["Rumplestiltskin", "The Evil Queen", "Snow White", "Prince Charming"];
            myAnimal.fillArrayWithInLengthHSTRING(myArray, 2);
            verifyArrayItems("myArray", myArray, ["Mr. Gold", "Regina", "", ""]);

            logger.comment("Call FillArray with 4 elements on JsArray of 4 elements");
            myArray = ["Rumplestiltskin", "The Evil Queen", "Snow White", "Prince Charming"];
            myAnimal.fillArrayWithInLengthHSTRING(myArray, 4);
            verifyArrayItems("myArray", myArray, ["Mr. Gold", "Regina", "Mary Margaret", "David Nolan"]);

            logger.comment("Call FillArray with 11 elements on JsArray of 4 elements");
            myArray = ["Rumplestiltskin", "The Evil Queen", "Snow White", "Prince Charming"];
            verify.exception(function () {
                myAnimal.fillArrayWithInLengthHSTRING(myArray, 11);
            }, TypeError, "FillArray with length > size");

            logger.comment("Call FillArray with 4 elements on StringArray of 4 elements");
            var myArray = myAnimal.receiveArrayHSTRING().value;
            myAnimal.fillArrayWithInLengthHSTRING(myArray, 4);
            verifyArrayItems("myArray", myArray, ["Mr. Gold", "Regina", "Mary Margaret", "David Nolan"]);

            logger.comment("Call FillArray with 2 elements on StringArray of 4 elements");
            var myArray = myAnimal.receiveArrayHSTRING().value;
            myAnimal.fillArrayWithInLengthHSTRING(myArray, 2);
            verifyArrayItems("myArray", myArray, ["Mr. Gold", "Regina", "", ""]);

            logger.comment("Call FillArray with 11 elements on StringArray of 4 elements");
            var myArray = myAnimal.receiveArrayHSTRING().value;
            verify.exception(function () {
                myAnimal.fillArrayWithInLengthHSTRING(myArray, 11);
            }, TypeError, "FillArray with length > size");

            logger.comment("Call FillArray with 0 elements on null");
            myAnimal.fillArrayWithInLengthHSTRING(null, 0);

            logger.comment("Call FillArray with 11 elements on null");
            verify.exception(function () {
                myAnimal.fillArrayWithInLengthHSTRING(null, 11);
            }, TypeError, "FillArray with length > size");
        }
    });

    runner.addTest({
        id: 14,
        desc: 'FillArray With [out] length attribute',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);
            myAnimal.passArrayHSTRING(["Rumplestiltskin", "The Evil Queen", "Snow White", "Prince Charming"]);

            logger.comment("Call FillArray with 4 elements on JsArray of 7 elements");
            var myArray = ["Mr. Gold", "Regina", "Mary Margaret", "David Nolan", "Emma Swan", "Henry Mills", "Sheriff Graham"];
            var elementsRead = myAnimal.fillArrayWithOutLengthHSTRING(myArray);
            verify(elementsRead, 4, "elementsRead");
            verifyArrayItems("myArray", myArray, ["Rumplestiltskin", "The Evil Queen", "Snow White", "Prince Charming", "", "", ""]);

            logger.comment("Call FillArray with 2 elements on JsArray of 2 elements");
            var myArray = ["Mr. Gold", "Regina"];
            var elementsRead = myAnimal.fillArrayWithOutLengthHSTRING(myArray);
            verify(elementsRead, 2, "elementsRead");
            verifyArrayItems("myArray", myArray, ["Rumplestiltskin", "The Evil Queen"]);

            logger.comment("Call FillArray with 4 elements on StringArray of 4 elements");
            var myArray = myAnimal.receiveArrayHSTRING().value;
            myArray[0] = "Mr. Gold";
            myArray[1] = "Regina";
            myArray[2] = "Mary Margaret";
            myArray[3] = "David Nolan";
            elementsRead = myAnimal.fillArrayWithOutLengthHSTRING(myArray);
            verify(elementsRead, 4, "elementsRead");
            verifyArrayItems("myArray", myArray, ["Rumplestiltskin", "The Evil Queen", "Snow White", "Prince Charming"]);

            logger.comment("Call FillArray with 4 elements on StringArray of 7 elements");
            myAnimal.passArrayHSTRING(["Mr. Gold", "Regina", "Mary Margaret", "David Nolan", "Emma Swan", "Henry Mills", "Sheriff Graham"]);
            myArray = myAnimal.receiveArrayHSTRING().value;
            myAnimal.passArrayHSTRING(["Rumplestiltskin", "The Evil Queen", "Snow White", "Prince Charming"]);
            var elementsRead = myAnimal.fillArrayWithOutLengthHSTRING(myArray);
            verify(elementsRead, 4, "elementsRead");
            verifyArrayItems("myArray", myArray, ["Rumplestiltskin", "The Evil Queen", "Snow White", "Prince Charming", "", "", ""]);

            logger.comment("Call FillArray with 0 elements on null");
            elementsRead = myAnimal.fillArrayWithOutLengthHSTRING(null);
            verify(elementsRead, 0, "elementsRead");
        }
    });

    runner.addTest({
        id: 15,
        desc: 'ReceiveArray With [in] length attribute',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);
            myAnimal.passArrayHSTRING(["Mr. Gold", "Regina", "Mary Margaret", "David Nolan", "Emma Swan", "Henry Mills", "Sheriff Graham"]);

            logger.comment("Call ReceiveArray with 4 elements");
            var myArray = myAnimal.receiveArrayWithInLengthHSTRING(4);
            verifyArrayItems("myArray", myArray, ["Mr. Gold", "Regina", "Mary Margaret", "David Nolan", "", "", ""]);

            logger.comment("Call ReceiveArray with 7 elements");
            var myArray = myAnimal.receiveArrayWithInLengthHSTRING(7);
            verifyArrayItems("myArray", myArray, ["Mr. Gold", "Regina", "Mary Margaret", "David Nolan", "Emma Swan", "Henry Mills", "Sheriff Graham"]);

            logger.comment("Call ReceiveArray with 9 elements");
            var myArray = myAnimal.receiveArrayWithInLengthHSTRING(9);
            verifyArrayItems("myArray", myArray, ["Mr. Gold", "Regina", "Mary Margaret", "David Nolan", "Emma Swan", "Henry Mills", "Sheriff Graham", "", ""]);

            logger.comment("Call ReceiveArray with 0 elements");
            var myArray = myAnimal.receiveArrayWithInLengthHSTRING(0);
            verifyArrayItems("myArray", myArray, ["", "", "", "", "", "", ""]);

            logger.comment("Call ReceiveArray with 0 elements");
            myAnimal.passArrayHSTRING([]);
            var myArray = myAnimal.receiveArrayWithInLengthHSTRING(0);
            verify(myArray, null, "myArray");
        }
    });

    runner.addTest({
        id: 16,
        desc: 'ReceiveArray With [out] length attribute',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);

            logger.comment("Call ReceiveArray with size = 4  length = 2");
            myAnimal.passArrayWithInLengthHSTRING(["Mr. Gold", "Regina", "Emma", "Henry"], 2);
            var result = myAnimal.receiveArrayWithOutLengthHSTRING();
            verify(result.lengthValue, 2, "elementsRead");
            verifyArrayItems("myArray", result.value, ["Mr. Gold", "Regina", "", ""]);

            logger.comment("Call ReceiveArray with size = 4  length = 4");
            myAnimal.passArrayWithInLengthHSTRING(["Mr. Gold", "Regina", "Emma", "Henry"], 4);
            var result = myAnimal.receiveArrayWithOutLengthHSTRING();
            verify(result.lengthValue, 4, "elementsRead");
            verifyArrayItems("myArray", result.value, ["Mr. Gold", "Regina", "Emma", "Henry"]);

            logger.comment("Call ReceiveArray with size = 0  length = 0");
            myAnimal.passArrayHSTRING([]);
            var result = myAnimal.receiveArrayWithOutLengthHSTRING();
            verify(result.lengthValue, 0, "elementsRead");
            verify(result.value, null, "myArray");
        }
    });

    runner.addTest({
        id: 17,
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
            myAnimal.passArrayWithInLengthHSTRING(["Sheldon", "Leonard", "Howard", "Rajesh"], 2);
            myAnimal.callDelegatePassArrayWithInLengthHSTRING(delegatePassArrayWithInLength);
            verify(actualLength, 2, "elementsRead");
            verifyArrayItems("myArray", actualArray, ["Sheldon", "Leonard", "", ""]);

            logger.comment("Call PassArray with size = 4  length = 4");
            myAnimal.passArrayWithInLengthHSTRING(["Sheldon", "Leonard", "Howard", "Rajesh"], 4);
            myAnimal.callDelegatePassArrayWithInLengthHSTRING(delegatePassArrayWithInLength);
            verify(actualLength, 4, "elementsRead");
            verifyArrayItems("myArray", actualArray, ["Sheldon", "Leonard", "Howard", "Rajesh"]);

            logger.comment("Call PassArray with size = 0  length = 0");
            myAnimal.passArrayHSTRING([]);
            myAnimal.callDelegatePassArrayWithInLengthHSTRING(delegatePassArrayWithInLength);
            verify(actualLength, 0, "elementsRead");
            verify(actualArray, null, "myArray");
        }
    });

    runner.addTest({
        id: 18,
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
            myAnimal.passArrayWithInLengthHSTRING(["Sheldon", "Leonard", "Howard", "Rajesh"], 2);
            sendLength = 2;
            myAnimal.callDelegatePassArrayWithOutLengthHSTRING(delegatePassArrayWithOutLength);
            verifyArrayItems("myArray", actualArray, ["Sheldon", "Leonard", "", ""]);

            logger.comment("Call PassArray with size = 4  length = 4");
            myAnimal.passArrayWithInLengthHSTRING(["Sheldon", "Leonard", "Howard", "Rajesh"], 4);
            sendLength = 4;
            myAnimal.callDelegatePassArrayWithOutLengthHSTRING(delegatePassArrayWithOutLength);
            verifyArrayItems("myArray", actualArray, ["Sheldon", "Leonard", "Howard", "Rajesh"]);

            logger.comment("Call PassArray with size = 0  length = 0");
            myAnimal.passArrayHSTRING([]);
            sendLength = 0;
            myAnimal.callDelegatePassArrayWithOutLengthHSTRING(delegatePassArrayWithOutLength);
            verify(actualArray, null, "myArray");
        }
    });

    runner.addTest({
        id: 19,
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
            actualArray = ["Sheldon", "Leonard", "Howard", "Rajesh"];
            actualLength = 2;
            myAnimal.passArrayWithInLengthHSTRING(actualArray, actualLength);
            myAnimal.callDelegateFillArrayWithInLengthHSTRING(delegateFillArrayWithInLength);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verifyArrayItems("resultArray", resultArray, ["Sheldon", "Leonard", "", ""]);

            logger.comment("Call FillArray with size = 4  length = 4");
            actualArray = ["Sheldon", "Leonard", "Howard", "Rajesh"];
            actualLength = 4;
            myAnimal.passArrayWithInLengthHSTRING(actualArray, actualLength);
            myAnimal.callDelegateFillArrayWithInLengthHSTRING(delegateFillArrayWithInLength);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verifyArrayItems("resultArray", resultArray, ["Sheldon", "Leonard", "Howard", "Rajesh"]);

            logger.comment("Call FillArray with size = 0  length = 0");
            myAnimal.passArrayHSTRING([]);
            excpectedNull = true;
            actualLength = 0;
            myAnimal.callDelegateFillArrayWithInLengthHSTRING(delegateFillArrayWithInLength);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verify(resultArray, null, "resultArray");
        }
    });

    runner.addTest({
        id: 20,
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
            myAnimal.passArrayHSTRING(["Sheldon", "Leonard", "Howard", "Rajesh"]);

            logger.comment("Call FillArray with size = 4  length = 2");
            actualArray = ["Sheldon", "Leonard", "Howard", "Rajesh"];
            actualLength = 2;
            myAnimal.callDelegateFillArrayWithOutLengthHSTRING(delegateFillArrayWithOutLength);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verifyArrayItems("resultArray", resultArray, ["Sheldon", "Leonard", "", ""]);

            logger.comment("Call FillArray with size = 4  length = 4");
            actualLength = 4;
            myAnimal.callDelegateFillArrayWithOutLengthHSTRING(delegateFillArrayWithOutLength);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verifyArrayItems("resultArray", resultArray, ["Sheldon", "Leonard", "Howard", "Rajesh"]);

            logger.comment("Call FillArray with size = 0  length = 0");
            myAnimal.passArrayHSTRING([]);
            actualArray = null;
            actualLength = 0;
            excpectedNull = true;
            myAnimal.callDelegateFillArrayWithOutLengthHSTRING(delegateFillArrayWithOutLength);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verify(resultArray, null, "resultArray");
        }
    });

    runner.addTest({
        id: 21,
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
            actualArray = ["Sheldon", "Leonard", "Howard", "Rajesh"];
            actualLength = 2;
            myAnimal.passArrayWithInLengthHSTRING(["Amy", "Priya", "Bernadette", "Penny"], actualLength);
            myAnimal.callDelegateReceiveArrayWithInLengthHSTRING(delegateReceiveArrayWithInLength);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verifyArrayItems("resultArray", resultArray, ["Sheldon", "Leonard", "", ""]);

            logger.comment("Call ReceiveArray with size = 4  length = 4");
            actualArray = ["Sheldon", "Leonard", "Howard", "Rajesh"];
            actualLength = 4;
            myAnimal.passArrayWithInLengthHSTRING(["Amy", "Priya", "Bernadette", "Penny"], actualLength);
            myAnimal.callDelegateReceiveArrayWithInLengthHSTRING(delegateReceiveArrayWithInLength);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verifyArrayItems("resultArray", resultArray, ["Sheldon", "Leonard", "Howard", "Rajesh"]);

            logger.comment("Call ReceiveArray with size = 0  length = 0");
            actualArray = null;
            actualLength = 0;
            myAnimal.passArrayHSTRING([]);
            myAnimal.callDelegateReceiveArrayWithInLengthHSTRING(delegateReceiveArrayWithInLength);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verify(resultArray, null, "resultArray");
        }
    });

    runner.addTest({
        id: 22,
        desc: 'Delegate ReceiveArray With [out] length attribute',
        pri: '0',
        test: function () {
            var actualArray = 0
            var actualLength = 0;
            function delegateReceiveArrayWithOutLength() {
                logger.comment("*** delegateReceiveArrayWithOutLength - Invoke ***");

                dumpArrayItems("returnArray", actualArray);
                logger.comment("returnLength: " + actualLength);

                logger.comment("*** delegateReceiveArrayWithOutLength - Exit ***");

                var outVar = new Object();
                outVar.value = actualArray;
                outVar.lengthValue = actualLength;

                return outVar;
            }

            logger.comment("Call ReceiveArray with size = 4  length = 2");
            var myAnimal = new Animals.Animal(1);
            actualArray = ["Amy", "Priya", "Bernadette", "Penny"];
            actualLength = 2;
            myAnimal.callDelegateReceiveArrayWithOutLengthHSTRING(delegateReceiveArrayWithOutLength);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verifyArrayItems("resultArray", resultArray, ["Amy", "Priya", "", ""]);

            logger.comment("Call ReceiveArray with size = 4  length = 4");
            actualArray = ["Amy", "Priya", "Bernadette", "Penny"];
            actualLength = 4;
            myAnimal.callDelegateReceiveArrayWithOutLengthHSTRING(delegateReceiveArrayWithOutLength);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verifyArrayItems("resultArray", resultArray, ["Amy", "Priya", "Bernadette", "Penny"]);

            logger.comment("Call ReceiveArray with size = 0  length = 0");
            actualArray = null;
            actualLength = 0;
            myAnimal.callDelegateReceiveArrayWithOutLengthHSTRING(delegateReceiveArrayWithOutLength);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verify(resultArray, null, "resultArray");
        }
    });

    runner.addTest({
        id: 23,
        desc: 'PassArray With [out] length attribute which has retVal attribute too',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);

            logger.comment("Call PassArray JsArray");
            var elementsRead = myAnimal.passArrayWithOutLengthWithRetValLengthHSTRING(["Once", "Upon", "a", "Time"]);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verify(elementsRead, 4, "elementsRead");
            verifyArrayItems("resultArray", resultArray, ["Once", "Upon", "a", "Time"]);

            logger.comment("Call PassArray StringArray");
            elementsRead = myAnimal.passArrayWithOutLengthWithRetValLengthHSTRING(resultArray);
            resultArray = myAnimal.receiveArrayHSTRING().value;
            verify(elementsRead, 4, "elementsRead");
            verifyArrayItems("resultArray", resultArray, ["Once", "Upon", "a", "Time"]);

            logger.comment("Call PassArray with 0 elements on null");
            elementsRead = myAnimal.passArrayWithOutLengthWithRetValLengthHSTRING(null);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verify(elementsRead, 0, "elementsRead");
            verify(resultArray, null, "resultArray");
        }
    });

    runner.addTest({
        id: 24,
        desc: 'FillArray With [out] length attribute which has retVal attribute too',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);
            myAnimal.passArrayHSTRING(["Rumplestiltskin", "The Evil Queen", "Snow White", "Prince Charming"]);

            logger.comment("Call FillArray with 4 elements on JsArray of 7 elements");
            var myArray = ["Mr. Gold", "Regina", "Mary Margaret", "David Nolan", "Emma Swan", "Henry Mills", "Sheriff Graham"];
            var elementsRead = myAnimal.fillArrayWithOutLengthWithRetValLengthHSTRING(myArray);
            verify(elementsRead, 4, "elementsRead");
            verifyArrayItems("myArray", myArray, ["Rumplestiltskin", "The Evil Queen", "Snow White", "Prince Charming", "", "", ""]);

            logger.comment("Call FillArray with 2 elements on JsArray of 2 elements");
            var myArray = ["Mr. Gold", "Regina"];
            var elementsRead = myAnimal.fillArrayWithOutLengthWithRetValLengthHSTRING(myArray);
            verify(elementsRead, 2, "elementsRead");
            verifyArrayItems("myArray", myArray, ["Rumplestiltskin", "The Evil Queen"]);

            logger.comment("Call FillArray with 4 elements on StringArray of 4 elements");
            var myArray = myAnimal.receiveArrayHSTRING().value;
            myArray[0] = "Mr. Gold";
            myArray[1] = "Regina";
            myArray[2] = "Mary Margaret";
            myArray[3] = "David Nolan";
            elementsRead = myAnimal.fillArrayWithOutLengthWithRetValLengthHSTRING(myArray);
            verify(elementsRead, 4, "elementsRead");
            verifyArrayItems("myArray", myArray, ["Rumplestiltskin", "The Evil Queen", "Snow White", "Prince Charming"]);

            logger.comment("Call FillArray with 4 elements on StringArray of 7 elements");
            myAnimal.passArrayHSTRING(["Mr. Gold", "Regina", "Mary Margaret", "David Nolan", "Emma Swan", "Henry Mills", "Sheriff Graham"]);
            myArray = myAnimal.receiveArrayHSTRING().value;
            myAnimal.passArrayHSTRING(["Rumplestiltskin", "The Evil Queen", "Snow White", "Prince Charming"]);
            var elementsRead = myAnimal.fillArrayWithOutLengthWithRetValLengthHSTRING(myArray);
            verify(elementsRead, 4, "elementsRead");
            verifyArrayItems("myArray", myArray, ["Rumplestiltskin", "The Evil Queen", "Snow White", "Prince Charming", "", "", ""]);

            logger.comment("Call FillArray with 0 elements on null");
            elementsRead = myAnimal.fillArrayWithOutLengthWithRetValLengthHSTRING(null);
            verify(elementsRead, 0, "elementsRead");
        }
    });

    runner.addTest({
        id: 25,
        desc: 'ReceiveArray With [out] length attribute which has retVal attribute too',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);

            logger.comment("Call ReceiveArray with size = 4  length = 2");
            myAnimal.passArrayWithInLengthHSTRING(["Mr. Gold", "Regina", "Emma", "Henry"], 2);
            var result = myAnimal.receiveArrayWithOutLengthWithRetValLengthHSTRING();
            verify(result.lengthValue, 2, "elementsRead");
            verifyArrayItems("myArray", result.value, ["Mr. Gold", "Regina", "", ""]);

            logger.comment("Call ReceiveArray with size = 4  length = 4");
            myAnimal.passArrayWithInLengthHSTRING(["Mr. Gold", "Regina", "Emma", "Henry"], 4);
            var result = myAnimal.receiveArrayWithOutLengthWithRetValLengthHSTRING();
            verify(result.lengthValue, 4, "elementsRead");
            verifyArrayItems("myArray", result.value, ["Mr. Gold", "Regina", "Emma", "Henry"]);

            logger.comment("Call ReceiveArray with size = 0  length = 0");
            myAnimal.passArrayHSTRING([]);
            var result = myAnimal.receiveArrayWithOutLengthWithRetValLengthHSTRING();
            verify(result.lengthValue, 0, "elementsRead");
            verify(result.value, null, "myArray");
        }
    });

    runner.addTest({
        id: 26,
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
            myAnimal.passArrayWithInLengthHSTRING(["Sheldon", "Leonard", "Howard", "Rajesh"], 2);
            sendLength = 2;
            myAnimal.callDelegatePassArrayWithOutLengthWithRetValLengthHSTRING(delegatePassArrayWithOutLengthWithRetValLength);
            verifyArrayItems("myArray", actualArray, ["Sheldon", "Leonard", "", ""]);

            logger.comment("Call PassArray with size = 4  length = 4");
            myAnimal.passArrayWithInLengthHSTRING(["Sheldon", "Leonard", "Howard", "Rajesh"], 4);
            sendLength = 4;
            myAnimal.callDelegatePassArrayWithOutLengthWithRetValLengthHSTRING(delegatePassArrayWithOutLengthWithRetValLength);
            verifyArrayItems("myArray", actualArray, ["Sheldon", "Leonard", "Howard", "Rajesh"]);

            logger.comment("Call PassArray with size = 0  length = 0");
            myAnimal.passArrayHSTRING([]);
            sendLength = 0;
            myAnimal.callDelegatePassArrayWithOutLengthWithRetValLengthHSTRING(delegatePassArrayWithOutLengthWithRetValLength);
            verify(actualArray, null, "myArray");
        }
    });

    runner.addTest({
        id: 27,
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
            myAnimal.passArrayHSTRING(["Sheldon", "Leonard", "Howard", "Rajesh"]);

            logger.comment("Call FillArray with size = 4  length = 2");
            actualArray = ["Sheldon", "Leonard", "Howard", "Rajesh"];
            actualLength = 2;
            myAnimal.callDelegateFillArrayWithOutLengthWithRetValLengthHSTRING(delegateFillArrayWithOutLengthWithRetValLength);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verifyArrayItems("resultArray", resultArray, ["Sheldon", "Leonard", "", ""]);

            logger.comment("Call FillArray with size = 4  length = 4");
            actualLength = 4;
            myAnimal.callDelegateFillArrayWithOutLengthWithRetValLengthHSTRING(delegateFillArrayWithOutLengthWithRetValLength);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verifyArrayItems("resultArray", resultArray, ["Sheldon", "Leonard", "Howard", "Rajesh"]);

            logger.comment("Call FillArray with size = 0  length = 0");
            myAnimal.passArrayHSTRING([]);
            actualArray = null;
            actualLength = 0;
            excpectedNull = true;
            myAnimal.callDelegateFillArrayWithOutLengthWithRetValLengthHSTRING(delegateFillArrayWithOutLengthWithRetValLength);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verify(resultArray, null, "resultArray");
        }
    });

    runner.addTest({
        id: 28,
        desc: 'Delegate ReceiveArray With [out] length attribute which has retVal attribute too',
        pri: '0',
        test: function () {
            var actualArray = 0
            var actualLength = 0;
            function delegateReceiveArrayWithOutLengthWithRetValLength() {
                logger.comment("*** delegateReceiveArrayWithOutLengthWithRetValLength - Invoke ***");

                dumpArrayItems("returnArray", actualArray);
                logger.comment("returnLength: " + actualLength);

                logger.comment("*** delegateReceiveArrayWithOutLengthWithRetValLength - Exit ***");

                var outVar = new Object();
                outVar.value = actualArray;
                outVar.lengthValue = actualLength;

                return outVar;
            }

            logger.comment("Call ReceiveArray with size = 4  length = 2");
            var myAnimal = new Animals.Animal(1);
            actualArray = ["Amy", "Priya", "Bernadette", "Penny"];
            actualLength = 2;
            myAnimal.callDelegateReceiveArrayWithOutLengthWithRetValLengthHSTRING(delegateReceiveArrayWithOutLengthWithRetValLength);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verifyArrayItems("resultArray", resultArray, ["Amy", "Priya", "", ""]);

            logger.comment("Call ReceiveArray with size = 4  length = 4");
            actualArray = ["Amy", "Priya", "Bernadette", "Penny"];
            actualLength = 4;
            myAnimal.callDelegateReceiveArrayWithOutLengthWithRetValLengthHSTRING(delegateReceiveArrayWithOutLengthWithRetValLength);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verifyArrayItems("resultArray", resultArray, ["Amy", "Priya", "Bernadette", "Penny"]);

            logger.comment("Call ReceiveArray with size = 0  length = 0");
            actualArray = null;
            actualLength = 0;
            myAnimal.callDelegateReceiveArrayWithOutLengthWithRetValLengthHSTRING(delegateReceiveArrayWithOutLengthWithRetValLength);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verify(resultArray, null, "resultArray");
        }
    });


    runner.addTest({
        id: 29,
        desc: 'PassArray With [out] length attribute which has random parameter with retVal attribute',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);

            logger.comment("Call PassArray JsArray");
            var result = myAnimal.passArrayWithOutLengthWithRetValRandomParamHSTRING(["Once", "Upon", "a", "Time"]);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verify(result.lengthValue, 4, "elementsRead");
            verify(result.randomRetVal, 100, "random parameter");
            verifyArrayItems("resultArray", resultArray, ["Once", "Upon", "a", "Time"]);

            logger.comment("Call PassArray StringArray");
            result = myAnimal.passArrayWithOutLengthWithRetValRandomParamHSTRING(resultArray);
            resultArray = myAnimal.receiveArrayHSTRING().value;
            verify(result.lengthValue, 4, "elementsRead");
            verify(result.randomRetVal, 100, "random parameter");
            verifyArrayItems("resultArray", resultArray, ["Once", "Upon", "a", "Time"]);

            logger.comment("Call PassArray with 0 elements on null");
            result = myAnimal.passArrayWithOutLengthWithRetValRandomParamHSTRING(null);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verify(result.lengthValue, 0, "elementsRead");
            verify(result.randomRetVal, 100, "random parameter");
            verify(resultArray, null, "resultArray");
        }
    });

    runner.addTest({
        id: 30,
        desc: 'FillArray With [out] length attribute which has random parameter with retVal attribute',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);
            myAnimal.passArrayHSTRING(["Rumplestiltskin", "The Evil Queen", "Snow White", "Prince Charming"]);

            logger.comment("Call FillArray with 4 elements on JsArray of 7 elements");
            var myArray = ["Mr. Gold", "Regina", "Mary Margaret", "David Nolan", "Emma Swan", "Henry Mills", "Sheriff Graham"];
            var result = myAnimal.fillArrayWithOutLengthWithRetValRandomParamHSTRING(myArray);
            verify(result.lengthValue, 4, "elementsRead");
            verify(result.randomRetVal, 100, "random parameter");
            verifyArrayItems("myArray", myArray, ["Rumplestiltskin", "The Evil Queen", "Snow White", "Prince Charming", "", "", ""]);

            logger.comment("Call FillArray with 2 elements on JsArray of 2 elements");
            var myArray = ["Mr. Gold", "Regina"];
            var result = myAnimal.fillArrayWithOutLengthWithRetValRandomParamHSTRING(myArray);
            verify(result.lengthValue, 2, "elementsRead");
            verify(result.randomRetVal, 100, "random parameter");
            verifyArrayItems("myArray", myArray, ["Rumplestiltskin", "The Evil Queen"]);

            logger.comment("Call FillArray with 4 elements on StringArray of 4 elements");
            var myArray = myAnimal.receiveArrayHSTRING().value;
            myArray[0] = "Mr. Gold";
            myArray[1] = "Regina";
            myArray[2] = "Mary Margaret";
            myArray[3] = "David Nolan";
            result = myAnimal.fillArrayWithOutLengthWithRetValRandomParamHSTRING(myArray);
            verify(result.lengthValue, 4, "elementsRead");
            verify(result.randomRetVal, 100, "random parameter");
            verifyArrayItems("myArray", myArray, ["Rumplestiltskin", "The Evil Queen", "Snow White", "Prince Charming"]);

            logger.comment("Call FillArray with 4 elements on StringArray of 7 elements");
            myAnimal.passArrayHSTRING(["Mr. Gold", "Regina", "Mary Margaret", "David Nolan", "Emma Swan", "Henry Mills", "Sheriff Graham"]);
            myArray = myAnimal.receiveArrayHSTRING().value;
            myAnimal.passArrayHSTRING(["Rumplestiltskin", "The Evil Queen", "Snow White", "Prince Charming"]);
            var result = myAnimal.fillArrayWithOutLengthWithRetValRandomParamHSTRING(myArray);
            verify(result.lengthValue, 4, "elementsRead");
            verify(result.randomRetVal, 100, "random parameter");
            verifyArrayItems("myArray", myArray, ["Rumplestiltskin", "The Evil Queen", "Snow White", "Prince Charming", "", "", ""]);

            logger.comment("Call FillArray with 0 elements on null");
            result = myAnimal.fillArrayWithOutLengthWithRetValRandomParamHSTRING(null);
            verify(result.lengthValue, 0, "elementsRead");
            verify(result.randomRetVal, 100, "random parameter");
        }
    });

    runner.addTest({
        id: 31,
        desc: 'ReceiveArray With [out] length attribute which has random parameter with retVal attribute',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);

            logger.comment("Call ReceiveArray with size = 4  length = 2");
            myAnimal.passArrayWithInLengthHSTRING(["Mr. Gold", "Regina", "Emma", "Henry"], 2);
            var result = myAnimal.receiveArrayWithOutLengthWithRetValRandomParamHSTRING();
            verify(result.lengthValue, 2, "elementsRead");
            verify(result.randomRetVal, 100, "random Parameter");
            verifyArrayItems("myArray", result.value, ["Mr. Gold", "Regina", "", ""]);

            logger.comment("Call ReceiveArray with size = 4  length = 4");
            myAnimal.passArrayWithInLengthHSTRING(["Mr. Gold", "Regina", "Emma", "Henry"], 4);
            var result = myAnimal.receiveArrayWithOutLengthWithRetValRandomParamHSTRING();
            verify(result.lengthValue, 4, "elementsRead");
            verify(result.randomRetVal, 100, "random Parameter");
            verifyArrayItems("myArray", result.value, ["Mr. Gold", "Regina", "Emma", "Henry"]);

            logger.comment("Call ReceiveArray with size = 0  length = 0");
            myAnimal.passArrayHSTRING([]);
            var result = myAnimal.receiveArrayWithOutLengthWithRetValRandomParamHSTRING();
            verify(result.lengthValue, 0, "elementsRead");
            verify(result.randomRetVal, 100, "random Parameter");
            verify(result.value, null, "myArray");
        }
    });

    runner.addTest({
        id: 32,
        desc: 'Delegate PassArray With [out] length attribute which has random parameter with retVal attribute',
        pri: '0',
        test: function () {
            var actualArray = 0
            var sendLength;
            function delegatePassArrayWithOutLengthWithRetValLength(arrayInDelegate) {
                logger.comment("*** delegatePassArrayWithOutLengthWithRetValLength - Invoke ***");
                dumpArrayItems("arrayInDelegate", arrayInDelegate);

                actualArray = arrayInDelegate;

                logger.comment("*** delegatePassArrayWithOutLengthWithRetValLength - Exit ***");
                var outVar = new Object();
                outVar.lengthValue = sendLength;
                outVar.randomRetVal = 100;
                return outVar;
            }

            logger.comment("Call PassArray with size = 4  length = 2");
            var myAnimal = new Animals.Animal(1);
            myAnimal.passArrayWithInLengthHSTRING(["Sheldon", "Leonard", "Howard", "Rajesh"], 2);
            sendLength = 2;
            var randomParam = myAnimal.callDelegatePassArrayWithOutLengthWithRetValRandomParamHSTRING(delegatePassArrayWithOutLengthWithRetValLength);
            verifyArrayItems("myArray", actualArray, ["Sheldon", "Leonard", "", ""]);
            verify(randomParam, 100, "randomParam");

            logger.comment("Call PassArray with size = 4  length = 4");
            myAnimal.passArrayWithInLengthHSTRING(["Sheldon", "Leonard", "Howard", "Rajesh"], 4);
            sendLength = 4;
            var randomParam = myAnimal.callDelegatePassArrayWithOutLengthWithRetValRandomParamHSTRING(delegatePassArrayWithOutLengthWithRetValLength);
            verifyArrayItems("myArray", actualArray, ["Sheldon", "Leonard", "Howard", "Rajesh"]);
            verify(randomParam, 100, "randomParam");

            logger.comment("Call PassArray with size = 0  length = 0");
            myAnimal.passArrayHSTRING([]);
            sendLength = 0;
            var randomParam = myAnimal.callDelegatePassArrayWithOutLengthWithRetValRandomParamHSTRING(delegatePassArrayWithOutLengthWithRetValLength);
            verify(actualArray, null, "myArray");
            verify(randomParam, 100, "randomParam");
        }
    });

    runner.addTest({
        id: 33,
        desc: 'Delegate FillArray With [out] length attribute which has random parameter with retVal attribute',
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

                var outVar = new Object();
                outVar.lengthValue = actualLength;
                outVar.randomRetVal = 100;
                return outVar;
            }

            var myAnimal = new Animals.Animal(1);
            myAnimal.passArrayHSTRING(["Sheldon", "Leonard", "Howard", "Rajesh"]);

            logger.comment("Call FillArray with size = 4  length = 2");
            actualArray = ["Sheldon", "Leonard", "Howard", "Rajesh"];
            actualLength = 2;
            var randomParam = myAnimal.callDelegateFillArrayWithOutLengthWithRetValRandomParamHSTRING(delegateFillArrayWithOutLengthWithRetValLength);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verifyArrayItems("resultArray", resultArray, ["Sheldon", "Leonard", "", ""]);
            verify(randomParam, 100, "randomParam");

            logger.comment("Call FillArray with size = 4  length = 4");
            actualLength = 4;
            var randomParam = myAnimal.callDelegateFillArrayWithOutLengthWithRetValRandomParamHSTRING(delegateFillArrayWithOutLengthWithRetValLength);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verifyArrayItems("resultArray", resultArray, ["Sheldon", "Leonard", "Howard", "Rajesh"]);
            verify(randomParam, 100, "randomParam");

            logger.comment("Call FillArray with size = 0  length = 0");
            myAnimal.passArrayHSTRING([]);
            actualArray = null;
            actualLength = 0;
            excpectedNull = true;
            var randomParam = myAnimal.callDelegateFillArrayWithOutLengthWithRetValRandomParamHSTRING(delegateFillArrayWithOutLengthWithRetValLength);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verify(resultArray, null, "resultArray");
            verify(randomParam, 100, "randomParam");
        }
    });

    runner.addTest({
        id: 34,
        desc: 'Delegate ReceiveArray With [out] length attribute which has random parameter with retVal attribute',
        pri: '0',
        test: function () {
            var actualArray = 0
            var actualLength = 0;
            function delegateReceiveArrayWithOutLengthWithRetValLength() {
                logger.comment("*** delegateReceiveArrayWithOutLengthWithRetValLength - Invoke ***");

                dumpArrayItems("returnArray", actualArray);
                logger.comment("returnLength: " + actualLength);

                logger.comment("*** delegateReceiveArrayWithOutLengthWithRetValLength - Exit ***");

                var outVar = new Object();
                outVar.value = actualArray;
                outVar.lengthValue = actualLength;
                outVar.randomRetVal = 100;
                return outVar;
            }

            logger.comment("Call ReceiveArray with size = 4  length = 2");
            var myAnimal = new Animals.Animal(1);
            actualArray = ["Amy", "Priya", "Bernadette", "Penny"];
            actualLength = 2;
            var randomParam = myAnimal.callDelegateReceiveArrayWithOutLengthWithRetValRandomParamHSTRING(delegateReceiveArrayWithOutLengthWithRetValLength);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verifyArrayItems("resultArray", resultArray, ["Amy", "Priya", "", ""]);
            verify(randomParam, 100, "randomParam");

            logger.comment("Call ReceiveArray with size = 4  length = 4");
            actualArray = ["Amy", "Priya", "Bernadette", "Penny"];
            actualLength = 4;
            var randomParam = myAnimal.callDelegateReceiveArrayWithOutLengthWithRetValRandomParamHSTRING(delegateReceiveArrayWithOutLengthWithRetValLength);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verifyArrayItems("resultArray", resultArray, ["Amy", "Priya", "Bernadette", "Penny"]);
            verify(randomParam, 100, "randomParam");

            logger.comment("Call ReceiveArray with size = 0  length = 0");
            actualArray = null;
            actualLength = 0;
            var randomParam = myAnimal.callDelegateReceiveArrayWithOutLengthWithRetValRandomParamHSTRING(delegateReceiveArrayWithOutLengthWithRetValLength);
            var resultArray = myAnimal.receiveArrayHSTRING().value;
            verify(resultArray, null, "resultArray");
            verify(randomParam, 100, "randomParam");
        }
    });

    runner.addTest({
        id: 35,
        desc: 'Array property on runtimeclass',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal();

            var myArray = ["OnceUponATime", "Revenge", "PersonOfInterest"];
            myAnimal.myArrayPropHSTRING = myArray;
            var propGet = myAnimal.myArrayPropHSTRING;
            verify(propGet.toString(), "[object StringArray]", "myAnimal.myArrayPropHSTRING.toString()");
            verify(propGet.length, myArray.length, "myAnimal.myArrayPropHSTRING.length");
            for (i = 0; i < myArray.length; i++) {
                verify(propGet[i], myArray[i], "myAnimal.myArrayPropHSTRING[" + i + "]");
            }
        }
    });

    runner.addTest({
        id: 36,
        desc: 'Array property on static interface',
        pri: '0',
        test: function () {
            var myArray = ["OnceUponATime", "Revenge", "PersonOfInterest"];
            Animals.Animal.myStaticArrayPropHSTRING = myArray;
            var propGet = Animals.Animal.myStaticArrayPropHSTRING;
            verify(propGet.toString(), "[object StringArray]", "Animals.Animal.myStaticArrayPropHSTRING.toString()");
            verify(propGet.length, myArray.length, "Animals.Animal.myStaticArrayPropHSTRING.length");
            for (i = 0; i < myArray.length; i++) {
                verify(propGet[i], myArray[i], "Animals.Animal.myStaticArrayPropHSTRING[" + i + "]");
            }
        }
    });

    Loader42_FileName = 'String Arrays marshaling and projection test case';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
