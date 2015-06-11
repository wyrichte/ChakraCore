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
        id: 'fpa2',
        desc: 'Out of memory',
        pri: '0',
        test: function () {
            var arr = [];
            arr.length = 1024 * 1024 * 1024;
            var r = new DevTests.Repros.Performance.RefClass();
            logger.comment("Verify that the exception is thrown when pass string array pattern is used");
            verify.exception(function () {
                r.passRetrievableStringArray(arr);
            }, Error, "r.passRetrievableStringArray(arr);");
        }
    });

    Loader42_FileName = 'Arrays projection test case (restricted due to slowness on amd64)';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
