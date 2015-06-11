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

    function easyPrint(myDoubleVectorString, myDoubleVector) {
        var objectDump = "\n    var " + myDoubleVectorString + "Members = [";
        for (p in myDoubleVector) {
            if (typeof myDoubleVector[p] == 'function') {
                objectDump = objectDump + '\n        [\'' + p + '\', \'' + typeof myDoubleVector[p] + '\', ' + myDoubleVector[p].length + '],'
            }
            else {
                objectDump = objectDump + '\n        [\'' + p + '\', \'' + typeof myDoubleVector[p] + '\'],';
            }
        }
        objectDump = objectDump + "\n    ];";

        return objectDump;
    }

    function verifyMembers(stringVector, vectorObject, restOfProperties, fStatic) {
        var objectDump = easyPrint(stringVector, vectorObject);
        logger.comment("typeof " + stringVector + ": " + typeof vectorObject);
        logger.comment("Dump of properties : " + objectDump);

        if (fStatic == true) {
            verify(typeof vectorObject, "function", "typeof " + stringVector);
        }
        else {
            verify(typeof vectorObject, "object", "typeof " + stringVector);
        }

        var propertiesIndex = 0;
        for (p in vectorObject) {
            // Look in properties
            verify(p, restOfProperties[propertiesIndex][0], stringVector + '["' + p + '"]');
            verify(typeof vectorObject[p], restOfProperties[propertiesIndex][1], 'typeof ' + stringVector + '["' + p + '"]');

            if (typeof vectorObject[p] == 'function') {
                verify(vectorObject[p].length, restOfProperties[propertiesIndex][2], stringVector + '["' + p + '"].length');
                logger.comment('Setting length of function to be 10');
                vectorObject[p].length = 10;
                verify(vectorObject[p].length, restOfProperties[propertiesIndex][2], stringVector + '["' + p + '"].length');
            }
            propertiesIndex++;
        }

        verify(propertiesIndex, restOfProperties.length, 'number of members of ' + stringVector);
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

    var myDoubleVectorMembers = [
        ['Windows.Foundation.Collections.IIterable`1<Int32>.first', 'function', 0],
        ['Windows.Foundation.Collections.IIterable`1<String>.first', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Int32>.append', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.clear', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Int32>.getAt', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.getMany', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<Int32>.getView', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Int32>.indexOf', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.insertAt', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<Int32>.removeAt', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.removeAtEnd', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Int32>.replaceAll', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.setAt', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<Int32>.size', 'number'],
        ['Windows.Foundation.Collections.IVector`1<String>.append', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<String>.clear', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<String>.getAt', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<String>.getMany', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<String>.getView', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<String>.indexOf', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<String>.insertAt', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<String>.removeAt', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<String>.removeAtEnd', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<String>.replaceAll', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<String>.setAt', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<String>.size', 'number'],
        ['toString', 'function', 0],
    ];

    var myMultipleIVectorMembers = [
        ['Windows.Foundation.Collections.IIterable`1<Animals.IAnimal>.first', 'function', 0],
        ['Windows.Foundation.Collections.IIterable`1<Int32>.first', 'function', 0],
        ['Windows.Foundation.Collections.IIterable`1<String>.first', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.append', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.clear', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.getAt', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.getMany', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.getView', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.indexOf', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.insertAt', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.removeAt', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.removeAtEnd', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.replaceAll', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.setAt', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.size', 'number'],
        ['Windows.Foundation.Collections.IVector`1<Int32>.append', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.clear', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Int32>.getAt', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.getMany', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<Int32>.getView', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Int32>.indexOf', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.insertAt', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<Int32>.removeAt', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.removeAtEnd', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Int32>.replaceAll', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.setAt', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<Int32>.size', 'number'],
        ['Windows.Foundation.Collections.IVector`1<String>.append', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<String>.clear', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<String>.getAt', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<String>.getMany', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<String>.getView', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<String>.indexOf', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<String>.insertAt', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<String>.removeAt', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<String>.removeAtEnd', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<String>.replaceAll', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<String>.setAt', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<String>.size', 'number'],
        ['toString', 'function', 0],
    ];

    var myInterfaceWithSingleIVectorMembers = [
        ['Windows.Foundation.Collections.IIterable`1<Int32>.first', 'function', 0],
        ['Windows.Foundation.Collections.IIterable`1<Single>.first', 'function', 0],
        ['Windows.Foundation.Collections.IVectorView`1<Single>.getAt', 'function', 1],
        ['Windows.Foundation.Collections.IVectorView`1<Single>.getMany', 'function', 2],
        ['Windows.Foundation.Collections.IVectorView`1<Single>.indexOf', 'function', 1],
        ['Windows.Foundation.Collections.IVectorView`1<Single>.size', 'number'],
        ['Windows.Foundation.Collections.IVector`1<Int32>.getAt', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.getMany', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<Int32>.indexOf', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.size', 'number'],
        ['append', 'function', 1],
        ['clear', 'function', 0],
        ['getView', 'function', 0],
        ['insertAt', 'function', 2],
        ['removeAt', 'function', 1],
        ['removeAtEnd', 'function', 0],
        ['replaceAll', 'function', 1],
        ['setAt', 'function', 2],
        ['toString', 'function', 0],
    ];

    var myInterfaceWithDoubleIVectorMembers = [
        ['Windows.Foundation.Collections.IIterable`1<Animals.IAnimal>.first', 'function', 0],
        ['Windows.Foundation.Collections.IIterable`1<Int32>.first', 'function', 0],
        ['Windows.Foundation.Collections.IIterable`1<String>.first', 'function', 0],
        ['Windows.Foundation.Collections.IVectorView`1<String>.getAt', 'function', 1],
        ['Windows.Foundation.Collections.IVectorView`1<String>.getMany', 'function', 2],
        ['Windows.Foundation.Collections.IVectorView`1<String>.indexOf', 'function', 1],
        ['Windows.Foundation.Collections.IVectorView`1<String>.size', 'number'],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.append', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.clear', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.getAt', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.getMany', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.getView', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.indexOf', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.insertAt', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.removeAt', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.removeAtEnd', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.replaceAll', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.setAt', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.size', 'number'],
        ['Windows.Foundation.Collections.IVector`1<Int32>.append', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.clear', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Int32>.getAt', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.getMany', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<Int32>.getView', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Int32>.indexOf', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.insertAt', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<Int32>.removeAt', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.removeAtEnd', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Int32>.replaceAll', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.setAt', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<Int32>.size', 'number'],
        ['toString', 'function', 0],
    ];

    var myInterfaceWithMultipleIVectorMembers = [
        ['Windows.Foundation.Collections.IIterable`1<Animals.IAnimal>.first', 'function', 0],
        ['Windows.Foundation.Collections.IIterable`1<Int32>.first', 'function', 0],
        ['Windows.Foundation.Collections.IIterable`1<String>.first', 'function', 0],
        ['Windows.Foundation.Collections.IIterable`1<System.Guid>.first', 'function', 0],
        ['Windows.Foundation.Collections.IVectorView`1<String>.getAt', 'function', 1],
        ['Windows.Foundation.Collections.IVectorView`1<String>.getMany', 'function', 2],
        ['Windows.Foundation.Collections.IVectorView`1<String>.indexOf', 'function', 1],
        ['Windows.Foundation.Collections.IVectorView`1<String>.size', 'number'],
        ['Windows.Foundation.Collections.IVectorView`1<System.Guid>.getAt', 'function', 1],
        ['Windows.Foundation.Collections.IVectorView`1<System.Guid>.getMany', 'function', 2],
        ['Windows.Foundation.Collections.IVectorView`1<System.Guid>.indexOf', 'function', 1],
        ['Windows.Foundation.Collections.IVectorView`1<System.Guid>.size', 'number'],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.append', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.clear', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.getAt', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.getMany', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.getView', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.indexOf', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.insertAt', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.removeAt', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.removeAtEnd', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.replaceAll', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.setAt', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.size', 'number'],
        ['Windows.Foundation.Collections.IVector`1<Int32>.append', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.clear', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Int32>.getAt', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.getMany', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<Int32>.getView', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Int32>.indexOf', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.insertAt', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<Int32>.removeAt', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.removeAtEnd', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Int32>.replaceAll', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.setAt', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<Int32>.size', 'number'],
        ['toString', 'function', 0],
    ];

    var myRCIDoubleObservableMembers = [
        ['Windows.Foundation.Collections.IIterable`1<Int32>.first', 'function', 0],
        ['Windows.Foundation.Collections.IIterable`1<String>.first', 'function', 0],
        ['Windows.Foundation.Collections.IObservableVector`1<Int32>.onvectorchanged', 'object'],
        ['Windows.Foundation.Collections.IObservableVector`1<String>.onvectorchanged', 'object'],
        ['Windows.Foundation.Collections.IVector`1<Int32>.append', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.clear', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Int32>.getAt', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.getMany', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<Int32>.getView', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Int32>.indexOf', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.insertAt', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<Int32>.removeAt', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.removeAtEnd', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<Int32>.replaceAll', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<Int32>.setAt', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<Int32>.size', 'number'],
        ['Windows.Foundation.Collections.IVector`1<String>.append', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<String>.clear', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<String>.getAt', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<String>.getMany', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<String>.getView', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<String>.indexOf', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<String>.insertAt', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<String>.removeAt', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<String>.removeAtEnd', 'function', 0],
        ['Windows.Foundation.Collections.IVector`1<String>.replaceAll', 'function', 1],
        ['Windows.Foundation.Collections.IVector`1<String>.setAt', 'function', 2],
        ['Windows.Foundation.Collections.IVector`1<String>.size', 'number'],
        ['addEventListener', 'function', 2],
        ['removeEventListener', 'function', 2],
        ['toString', 'function', 0],
    ];

    var errorObjectMemberExpected = 'Error: Object member expected';
    var errorObjectDoesntSupportAction = 'Error: Object doesn\'t support this action';
    var errorArrayLengthShouldBeFinite = 'RangeError: Array length must be assigned a finite positive number';
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
        logger.comment("myVector = new Animals.SingleIVector()");
        myVector = new Animals.SingleIVector();
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

    function verifyArrayPrototypeInChain(myFirstVectorString, myFirstVector, mySecondVectorString, mySecondVector, inChain) {
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
        desc: 'MarshalRCVectorOut',
        pri: '0',
        test: function () {
            GetVector();
            verifyVectorObjectDump("myVector", myVector, myArray, vectorMembers);
        }
    });

    runner.addTest({
        id: 2,
        desc: 'RCVectorArray_Index_length',
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
        desc: 'RCVectorAsArray_Push_Pop',
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
        desc: 'RCVectorAsArray_Shift_Unshift',
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
        desc: 'RCVectorAsArray_Slice',
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
        desc: 'RCVectorAsArray_Splice',
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
        desc: 'RCVectorAsArray_Every',
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
        desc: 'RCVectorAsArray_Some',
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
        desc: 'RCVectorAsArray_Filter',
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
        desc: 'RCVectorAsArray_ForEach',
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
        desc: 'RCVectorAsArray_Map',
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
        desc: 'RCVectorAsArray_Reduce_ReduceRight',
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
        desc: 'RCVectorAsArray_Reverse_Sort',
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
        desc: 'RCVectorAsArray_Concat',
        pri: '0',
        test: function () {
            GetVector();
            myArray = [98, 66, 55, 55, 54, 44, 33, 22, 22, 14, 11, 100, 100, 100, 100, 0];
            ApplyArrayToVector(myArray);

            verify(myVector.concat([1, 2, 3]), [myVector, 1, 2, 3], "myVector.concat([1,2,3])");
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            logger.comment("var mySecondVector = new Animals.SingleIVector();");
            var mySecondVector = new Animals.SingleIVector();
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
        desc: 'RCVectorAsArray_Join',
        pri: '0',
        test: function () {
            GetVector();
            myArray = [98, 66, 55, 55, 54, 44, 33, 22, 22, 14, 11, 100, 100, 100, 100, 0];
            ApplyArrayToVector(myArray);

            verify(myVector.join(' , '), myArray.join(' , '), "myVector.join(' , ')");
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            verify(myVector.join([9, 'b', 'c']), myArray.join([9, 'b', 'c']), "myVector.join([9,'b','c'])");
            verifyVectorAndArrayItems("myVector", myVector, myArray);

            logger.comment("var mySecondVector = new Animals.SingleIVector();");
            var mySecondVector = new Animals.SingleIVector(); ;
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
        desc: 'RCVectorAsArray_IndexOf',
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
        desc: 'RCVectorAsArray_LastIndexOf',
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
        desc: 'RCVectorArray_IsArray_Apply',
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
        desc: 'RCVectorArray_PropertyOperations',
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
        desc: 'RCVectorArray_Prototype',
        pri: '0',
        test: function () {
            GetVector();
            dumpVector(myVector, "myVector");

            logger.comment("var mySecondVector = new Animals.SingleIVector();");
            var mySecondVector = new Animals.SingleIVector();
            verifyArrayPrototypeInChain("myVector", myVector, "mySecondVector", mySecondVector);

            logger.comment("var myDoubleVector = new Animals.DoubleIVector();");
            var myDoubleVector = new Animals.DoubleIVector();

            logger.comment("var mySecondDoubleVector = new Animals.DoubleIVector();");
            var mySecondDoubleVector = new Animals.DoubleIVector();
            verifyArrayPrototypeInChain("myDoubleVector", myDoubleVector, "mySecondDoubleVector", mySecondDoubleVector, false);

            logger.comment("var myMultipleVector = new Animals.MultipleIVector();");
            var myMultipleVector = new Animals.MultipleIVector();

            logger.comment("var mySecondMultipleVector = new Animals.MultipleIVector();");
            var mySecondMultipleVector = new Animals.MultipleIVector();
            verifyArrayPrototypeInChain("myMultipleVector", myMultipleVector, "mySecondMultipleVector", mySecondMultipleVector, false);

            logger.comment("var myInterfaceWithSingleIVector = new Animals.InterfaceWithSingleIVector();");
            var myInterfaceWithSingleIVector = new Animals.InterfaceWithSingleIVector();

            logger.comment("var mySecondInterfaceWithSingleIVector = new Animals.InterfaceWithSingleIVector();");
            var mySecondInterfaceWithSingleIVector = new Animals.InterfaceWithSingleIVector();
            verifyArrayPrototypeInChain("myInterfaceWithSingleIVector", myInterfaceWithSingleIVector, "mySecondInterfaceWithSingleIVector", mySecondInterfaceWithSingleIVector, false);

            logger.comment("var myInterfaceWithDoubleIVector = new Animals.InterfaceWithDoubleIVector();");
            var myInterfaceWithDoubleIVector = new Animals.InterfaceWithDoubleIVector();

            logger.comment("var mySecondInterfaceWithDoubleIVector = new Animals.InterfaceWithDoubleIVector();");
            var mySecondInterfaceWithDoubleIVector = new Animals.InterfaceWithDoubleIVector();
            verifyArrayPrototypeInChain("myInterfaceWithDoubleIVector", myInterfaceWithDoubleIVector, "mySecondInterfaceWithDoubleIVector", mySecondInterfaceWithDoubleIVector, false);

            logger.comment("var myInterfaceWithMultipleIVector = new Animals.InterfaceWithMultipleIVector();");
            var myInterfaceWithMultipleIVector = new Animals.InterfaceWithMultipleIVector();

            logger.comment("var mySecondInterfaceWithMultipleIVector = new Animals.InterfaceWithMultipleIVector();");
            var mySecondInterfaceWithMultipleIVector = new Animals.InterfaceWithMultipleIVector();
            verifyArrayPrototypeInChain("myInterfaceWithMultipleIVector", myInterfaceWithMultipleIVector, "mySecondInterfaceWithMultipleIVector", mySecondInterfaceWithMultipleIVector, false);

            logger.comment("var myRCIDoubleObservable = new Animals.RCIDoubleObservable();");
            var myRCIDoubleObservable = new Animals.RCIDoubleObservable();

            logger.comment("var mySecondRCIDoubleObservable = new Animals.RCIDoubleObservable();");
            var mySecondRCIDoubleObservable = new Animals.RCIDoubleObservable();
            verifyArrayPrototypeInChain("myRCIDoubleObservable", myRCIDoubleObservable, "mySecondRCIDoubleObservable", mySecondRCIDoubleObservable, false);
        }
    });

    runner.addTest({
        id: 21,
        desc: 'DoubleIVector properties',
        pri: '0',
        test: function () {
            logger.comment("var myDoubleVector = new Animals.DoubleIVector();");
            var myDoubleVector = new Animals.DoubleIVector();
            verifyMembers("myDoubleVector", myDoubleVector, myDoubleVectorMembers);

            // Methods invokes:
            function forEachIntMethod(x, idx) {
                verify(myDoubleVector["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](idx), x, 'myDoubleVector["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](' + idx + ')');
            }
            var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            myArray.forEach(forEachIntMethod);

            function forEachStringMethod(x, idx) {
                verify(myDoubleVector["Windows.Foundation.Collections.IVector`1<String>.getAt"](idx), x, 'myDoubleVector["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](' + idx + ')');
            }
            myArray = ["String1", "String2", "String3", "String4"];
            myArray.forEach(forEachStringMethod);

            // Property invokes
            verify(myDoubleVector["Windows.Foundation.Collections.IVector`1<Int32>.size"], 9, 'myDoubleVector["Windows.Foundation.Collections.IVector`1<Int32>.size"]');
            verify(myDoubleVector["Windows.Foundation.Collections.IVector`1<String>.size"], 4, 'myDoubleVector["Windows.Foundation.Collections.IVector`1<String>.size"]');
        }
    });

    runner.addTest({
        id: 22,
        desc: 'MultipleIVector properties',
        pri: '0',
        test: function () {
            logger.comment("var myMultipleIVector = new Animals.MultipleIVector();");
            var myMultipleIVector = new Animals.MultipleIVector();
            verifyMembers("myMultipleIVector", myMultipleIVector, myMultipleIVectorMembers);

            // Methods invokes:
            function forEachIntMethod(x, idx) {
                verify(myMultipleIVector["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](idx), x, 'myMultipleIVector["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](' + idx + ')');
            }
            var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            myArray.forEach(forEachIntMethod);

            function forEachStringMethod(x, idx) {
                verify(myMultipleIVector["Windows.Foundation.Collections.IVector`1<String>.getAt"](idx), x, 'myMultipleIVector["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](' + idx + ')');
            }
            myArray = ["String1", "String2", "String3", "String4"];
            myArray.forEach(forEachStringMethod);

            function forEachAnimalMethod(x, idx) {
                verify(myMultipleIVector["Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.getAt"](idx).getGreeting(), x, 'myMultipleIVector["Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.getAt"](' + idx + ')GetGreeting()');
            }
            myArray = ["Animal1", "Animal2", "Animal3"];
            myArray.forEach(forEachAnimalMethod);

            // Property invokes
            verify(myMultipleIVector["Windows.Foundation.Collections.IVector`1<Int32>.size"], 9, 'myMultipleIVector["Windows.Foundation.Collections.IVector`1<Int32>.size"]');
            verify(myMultipleIVector["Windows.Foundation.Collections.IVector`1<String>.size"], 4, 'myMultipleIVector["Windows.Foundation.Collections.IVector`1<String>.size"]');
            verify(myMultipleIVector["Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.size"], 3, 'myMultipleIVector["Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.size"]');
        }
    });

    runner.addTest({
        id: 23,
        desc: 'InterfaceWithSingleIVector properties',
        pri: '0',
        test: function () {
            logger.comment("var myInterfaceWithSingleIVector = new Animals.InterfaceWithSingleIVector();");
            var myInterfaceWithSingleIVector = new Animals.InterfaceWithSingleIVector();
            verifyMembers("myInterfaceWithSingleIVector", myInterfaceWithSingleIVector, myInterfaceWithSingleIVectorMembers);

            // Methods invokes:
            function forEachIntMethod(x, idx) {
                verify(myInterfaceWithSingleIVector["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](idx), x, 'myInterfaceWithSingleIVector["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](' + idx + ')');
            }
            var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            myArray.forEach(forEachIntMethod);

            function forEachFloatMethod(x, idx) {
                verify(myInterfaceWithSingleIVector["Windows.Foundation.Collections.IVectorView`1<Single>.getAt"](idx), x, 'myInterfaceWithSingleIVector["Windows.Foundation.Collections.IVectorView`1<Single>.getAt"](' + idx + ')');
            }
            myArray = [0.25, 0.50, 0.75, 1.25, 1.50];
            myArray.forEach(forEachFloatMethod);

            // Property invokes
            verify(myInterfaceWithSingleIVector["Windows.Foundation.Collections.IVector`1<Int32>.size"], 9, 'myInterfaceWithSingleIVector["Windows.Foundation.Collections.IVector`1<Int32>.size"]');
            verify(myInterfaceWithSingleIVector["Windows.Foundation.Collections.IVectorView`1<Single>.size"], 5, 'myInterfaceWithSingleIVector["Windows.Foundation.Collections.IVectorView`1<Single>.size"]');

            // Non conflicting method
            logger.comment('myInterfaceWithSingleIVector.append(999);');
            myInterfaceWithSingleIVector.append(999);
            verify(myInterfaceWithSingleIVector["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](9), 999, 'myInterfaceWithSingleIVector["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](9)');
        }
    });

    runner.addTest({
        id: 24,
        desc: 'InterfaceWithDoubleIVector properties',
        pri: '0',
        test: function () {
            logger.comment("var myInterfaceWithDoubleIVector = new Animals.InterfaceWithDoubleIVector();");
            var myInterfaceWithDoubleIVector = new Animals.InterfaceWithDoubleIVector();
            verifyMembers("myInterfaceWithDoubleIVector", myInterfaceWithDoubleIVector, myInterfaceWithDoubleIVectorMembers);

            // Methods invokes:
            function forEachIntMethod(x, idx) {
                verify(myInterfaceWithDoubleIVector["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](idx), x, 'myInterfaceWithDoubleIVector["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](' + idx + ')');
            }
            var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            myArray.forEach(forEachIntMethod);

            function forEachAnimalMethod(x, idx) {
                verify(myInterfaceWithDoubleIVector["Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.getAt"](idx).getGreeting(), x, 'myInterfaceWithDoubleIVector["Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.getAt"](' + idx + ')GetGreeting()');
            }
            myArray = ["Animal1", "Animal2", "Animal3"];
            myArray.forEach(forEachAnimalMethod);

            function forEachStringMethod(x, idx) {
                verify(myInterfaceWithDoubleIVector["Windows.Foundation.Collections.IVectorView`1<String>.getAt"](idx), x, 'myInterfaceWithDoubleIVector["Windows.Foundation.Collections.IVectorView`1<Int32>.getAt"](' + idx + ')');
            }
            myArray = ["ViewString1", "ViewString2", "ViewString3", "ViewString4", "ViewString5", "ViewString6", "ViewString7"];
            myArray.forEach(forEachStringMethod);

            // Property invokes
            verify(myInterfaceWithDoubleIVector["Windows.Foundation.Collections.IVector`1<Int32>.size"], 9, 'myInterfaceWithDoubleIVector["Windows.Foundation.Collections.IVector`1<Int32>.size"]');
            verify(myInterfaceWithDoubleIVector["Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.size"], 3, 'myInterfaceWithDoubleIVector["Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.size"]');
            verify(myInterfaceWithDoubleIVector["Windows.Foundation.Collections.IVectorView`1<String>.size"], 7, 'myInterfaceWithDoubleIVector["Windows.Foundation.Collections.IVectorView`1<String>.size"]');
        }
    });

    runner.addTest({
        id: 25,
        desc: 'InterfaceWithMultipleIVector properties',
        pri: '0',
        test: function () {
            logger.comment("var myInterfaceWithMultipleIVector = new Animals.InterfaceWithMultipleIVector();");
            var myInterfaceWithMultipleIVector = new Animals.InterfaceWithMultipleIVector();
            verifyMembers("myInterfaceWithMultipleIVector", myInterfaceWithMultipleIVector, myInterfaceWithMultipleIVectorMembers);

            // Methods invokes:
            function forEachIntMethod(x, idx) {
                verify(myInterfaceWithMultipleIVector["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](idx), x, 'myInterfaceWithMultipleIVector["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](' + idx + ')');
            }
            var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            myArray.forEach(forEachIntMethod);

            function forEachAnimalMethod(x, idx) {
                verify(myInterfaceWithMultipleIVector["Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.getAt"](idx).getGreeting(), x, 'myInterfaceWithMultipleIVector["Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.getAt"](' + idx + ')GetGreeting()');
            }
            myArray = ["Animal1", "Animal2", "Animal3"];
            myArray.forEach(forEachAnimalMethod);

            function forEachStringMethod(x, idx) {
                verify(myInterfaceWithMultipleIVector["Windows.Foundation.Collections.IVectorView`1<String>.getAt"](idx), x, 'myInterfaceWithMultipleIVector["Windows.Foundation.Collections.IVectorView`1<Int32>.getAt"](' + idx + ')');
            }
            myArray = ["ViewString1", "ViewString2", "ViewString3", "ViewString4", "ViewString5", "ViewString6", "ViewString7"];
            myArray.forEach(forEachStringMethod);

            function forEachGuidMethod(x, idx) {
                verify(myInterfaceWithMultipleIVector["Windows.Foundation.Collections.IVectorView`1<System.Guid>.getAt"](idx), x, 'myInterfaceWithMultipleIVector["Windows.Foundation.Collections.IVectorView`1<System.Guid>.getAt"](' + idx + ')');
            }
            myArray = ["8eb82cb5-03d6-49d2-80ee-8583e949b5bf", "b960a7ac-f275-43fc-b154-91e7adeee7aa", "9f1af037-baf9-473b-b8c3-183ab3f5b3ce", "c4a1cc26-eb02-435b-ae67-18a25d86a787"];
            myArray.forEach(forEachGuidMethod);

            // Property invokes
            verify(myInterfaceWithMultipleIVector["Windows.Foundation.Collections.IVector`1<Int32>.size"], 9, 'myInterfaceWithMultipleIVector["Windows.Foundation.Collections.IVector`1<Int32>.size"]');
            verify(myInterfaceWithMultipleIVector["Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.size"], 3, 'myInterfaceWithMultipleIVector["Windows.Foundation.Collections.IVector`1<Animals.IAnimal>.size"]');
            verify(myInterfaceWithMultipleIVector["Windows.Foundation.Collections.IVectorView`1<String>.size"], 7, 'myInterfaceWithMultipleIVector["Windows.Foundation.Collections.IVectorView`1<String>.size"]');
            verify(myInterfaceWithMultipleIVector["Windows.Foundation.Collections.IVectorView`1<System.Guid>.size"], 4, 'myInterfaceWithMultipleIVector["Windows.Foundation.Collections.IVectorView`1<System.Guid>.size"]');
        }
    });

    runner.addTest({
        id: 26,
        desc: 'RCIDoubleObservable properties',
        pri: '0',
        test: function () {
            logger.comment("var myRCIDoubleObservable = new Animals.RCIDoubleObservable();");
            var myRCIDoubleObservable = new Animals.RCIDoubleObservable();
            verifyMembers("myRCIDoubleObservable", myRCIDoubleObservable, myRCIDoubleObservableMembers);

            // Methods invokes:
            function forEachIntMethod(x, idx) {
                verify(myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](idx), x, 'myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](' + idx + ')');
            }
            var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            myArray.forEach(forEachIntMethod);

            function forEachStringMethod(x, idx) {
                verify(myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<String>.getAt"](idx), x, 'myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](' + idx + ')');
            }
            myArray = ["String1", "String2", "String3", "String4"];
            myArray.forEach(forEachStringMethod);

            // Property invokes
            verify(myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<Int32>.size"], 9, 'myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<Int32>.size"]');
            verify(myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<String>.size"], 4, 'myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<String>.size"]');

            // Events
            function vectorINTChanged(ev) {
                logger.comment("*** ObservableVector<int> Change invoke");

                logger.comment("    newObservableVector: " + ev.target);
                logger.comment("    vectorChangedArgs.index: " + ev.index);
                logger.comment("    vectorChangedArgs.collectionChange: " + ev.collectionChange);

                observableVectorChangedArgs = ev.detail[0];

                logger.comment("*** ObservableVector<int> Change End");
            }

            function vectorHSTRINGChanged(ev) {
                logger.comment("*** ObservableVector<HSTRING> Change invoke");

                logger.comment("    newObservableVector: " + ev.target);
                logger.comment("    vectorChangedArgs.index: " + ev.index);
                logger.comment("    vectorChangedArgs.collectionChange: " + ev.collectionChange);

                observableVectorChangedArgs = ev.detail[0];

                logger.comment("*** ObservableVector<HSTRING> Change End");
            }

            // Change item from IVector<int> 
            var observableVectorChangedArgs;
            logger.comment('myRCIDoubleObservable.addEventListener("Windows.Foundation.Collections.IObservableVector`1<Int32>.vectorchanged", vectorINTChanged);');
            myRCIDoubleObservable.addEventListener("Windows.Foundation.Collections.IObservableVector`1<Int32>.vectorchanged", vectorINTChanged);

            logger.comment('myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<Int32>.setAt"](5, 88);');
            myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<Int32>.setAt"](5, 88);

            logger.comment("Verifying results that were received in vectorINTChanged event Handler");
            verify(observableVectorChangedArgs.index, 5, "observableVectorChangedArgs.index");
            verify(observableVectorChangedArgs.collectionChange, 3, "observableVectorChangedArgs.collectionChange");
            verify(myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](5), 88, 'myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](5)');

            // Change item from IVector<String> 
            logger.comment('myRCIDoubleObservable.addEventListener("Windows.Foundation.Collections.IObservableVector`1<String>.vectorchanged", vectorHSTRINGChanged);');
            myRCIDoubleObservable.addEventListener("Windows.Foundation.Collections.IObservableVector`1<String>.vectorchanged", vectorHSTRINGChanged);

            logger.comment('myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<String>.setAt"](2, "UpdatedString");');
            myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<String>.setAt"](2, "UpdatedString");

            logger.comment("Verifying results that were received in vectorHSTRINGChanged event Handler");
            verify(observableVectorChangedArgs.index, 2, "observableVectorChangedArgs.index");
            verify(observableVectorChangedArgs.collectionChange, 3, "observableVectorChangedArgs.collectionChange");
            verify(myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<String>.getAt"](2), "UpdatedString", 'myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<String>.getAt"](2)');

            // Remove event handler from IVector<String>
            logger.comment('myRCIDoubleObservable.removeEventListener("Windows.Foundation.Collections.IObservableVector`1<String>.vectorchanged", vectorHSTRINGChanged);');
            myRCIDoubleObservable.removeEventListener("Windows.Foundation.Collections.IObservableVector`1<String>.vectorchanged", vectorHSTRINGChanged);

            // Try updating HSTRING vector
            logger.comment("observableVectorChangedArgs = null");
            observableVectorChangedArgs = null;
            logger.comment('myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<String>.append"]("NewString");');
            myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<String>.append"]("NewString");

            verify(observableVectorChangedArgs, null, "observableVectorChangedArgs");
            verify(myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<String>.getAt"](4), "NewString", 'myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<String>.getAt"](4)');

            // Try int vector update and see if we can receive the callback
            logger.comment('myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<Int32>.append"](874);');
            myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<Int32>.append"](874);

            logger.comment("Verifying results that were received in vectorINTChanged event Handler");
            verify(observableVectorChangedArgs.index, 9, "observableVectorChangedArgs.index");
            verify(observableVectorChangedArgs.collectionChange, 1, "observableVectorChangedArgs.collectionChange");
            verify(myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](9), 874, 'myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](9)');

            // Remove the event handler of IVector<int>
            logger.comment('myRCIDoubleObservable.removeEventListener("Windows.Foundation.Collections.IObservableVector`1<Int32>.vectorchanged", vectorINTChanged);');
            myRCIDoubleObservable.removeEventListener("Windows.Foundation.Collections.IObservableVector`1<Int32>.vectorchanged", vectorINTChanged);

            // Try updating Int vector
            logger.comment("observableVectorChangedArgs = null");
            observableVectorChangedArgs = null;

            logger.comment('myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<Int32>.append"](786);');
            myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<Int32>.append"](786);

            verify(observableVectorChangedArgs, null, "observableVectorChangedArgs");
            verify(myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](10), 786, 'myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](10)');

            // Try updating HSTRING vector
            logger.comment('myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<String>.append"]("AnotherNewString");');
            myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<String>.append"]("AnotherNewString");

            verify(observableVectorChangedArgs, null, "observableVectorChangedArgs");
            verify(myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<String>.getAt"](5), "AnotherNewString", 'myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<String>.getAt"](5)');

            // Common event for two types:
            function vectorChanged(ev) {
                logger.comment("*** ObservableVector<T> Change invoke");

                logger.comment("    newObservableVector: " + ev.target);
                logger.comment("    vectorChangedArgs.index: " + ev.index);
                logger.comment("    vectorChangedArgs.collectionChange: " + ev.collectionChange);

                observableVectorChangedArgs = ev.detail[0];

                logger.comment("*** ObservableVector<T> Change End");
            }

            // Change item from IVector<int> 
            logger.comment('myRCIDoubleObservable.addEventListener("Windows.Foundation.Collections.IObservableVector`1<Int32>.vectorchanged", vectorChanged);');
            myRCIDoubleObservable.addEventListener("Windows.Foundation.Collections.IObservableVector`1<Int32>.vectorchanged", vectorChanged);

            logger.comment('myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<Int32>.setAt"](2, 58);');
            myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<Int32>.setAt"](2, 58);

            logger.comment("Verifying results that were received in vectorChanged event Handler");
            verify(observableVectorChangedArgs.index, 2, "observableVectorChangedArgs.index");
            verify(observableVectorChangedArgs.collectionChange, 3, "observableVectorChangedArgs.collectionChange");
            verify(myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](2), 58, 'myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](2)');

            // Change item from IVector<String> 
            logger.comment('myRCIDoubleObservable.addEventListener("Windows.Foundation.Collections.IObservableVector`1<String>.vectorchanged", vectorChanged);');
            myRCIDoubleObservable.addEventListener("Windows.Foundation.Collections.IObservableVector`1<String>.vectorchanged", vectorChanged);

            logger.comment('myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<String>.setAt"](0, "UpdatedString4");');
            myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<String>.setAt"](0, "UpdatedString4");

            logger.comment("Verifying results that were received in vectorChanged event Handler");
            verify(observableVectorChangedArgs.index, 0, "observableVectorChangedArgs.index");
            verify(observableVectorChangedArgs.collectionChange, 3, "observableVectorChangedArgs.collectionChange");
            verify(myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<String>.getAt"](0), "UpdatedString4", 'myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<String>.getAt"](0)');

            // Remove event handler from IVector<String>
            logger.comment('myRCIDoubleObservable.removeEventListener("Windows.Foundation.Collections.IObservableVector`1<String>.vectorchanged", vectorChanged);');
            myRCIDoubleObservable.removeEventListener("Windows.Foundation.Collections.IObservableVector`1<String>.vectorchanged", vectorChanged);

            // Try updating HSTRING vector
            logger.comment("observableVectorChangedArgs = null");
            observableVectorChangedArgs = null;
            logger.comment('myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<String>.append"]("YetAnotherNewString");');
            myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<String>.append"]("YetAnotherNewString");

            verify(observableVectorChangedArgs, null, "observableVectorChangedArgs");
            verify(myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<String>.getAt"](6), "YetAnotherNewString", 'myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<String>.getAt"](6)');

            // Try int vector update and see if we can receive the callback
            logger.comment('myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<Int32>.append"](100);');
            myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<Int32>.append"](100);

            logger.comment("Verifying results that were received in vectorChanged event Handler");
            verify(observableVectorChangedArgs.index, 11, "observableVectorChangedArgs.index");
            verify(observableVectorChangedArgs.collectionChange, 1, "observableVectorChangedArgs.collectionChange");
            verify(myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](11), 100, 'myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](11)');

            // Remove the event handler of IVector<int>
            logger.comment('myRCIDoubleObservable.removeEventListener("Windows.Foundation.Collections.IObservableVector`1<Int32>.vectorchanged", vectorChanged);');
            myRCIDoubleObservable.removeEventListener("Windows.Foundation.Collections.IObservableVector`1<Int32>.vectorchanged", vectorChanged);

            // Try updating Int vector
            logger.comment("observableVectorChangedArgs = null");
            observableVectorChangedArgs = null;

            logger.comment('myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<Int32>.append"](786786);');
            myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<Int32>.append"](786786);

            verify(observableVectorChangedArgs, null, "observableVectorChangedArgs");
            verify(myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](12), 786786, 'myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<Int32>.getAt"](12)');

            // Try updating HSTRING vector
            logger.comment('myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<String>.append"]("MyString");');
            myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<String>.append"]("MyString");

            verify(observableVectorChangedArgs, null, "observableVectorChangedArgs");
            verify(myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<String>.getAt"](7), "MyString", 'myRCIDoubleObservable["Windows.Foundation.Collections.IVector`1<String>.getAt"](7)');
        }
    });

    Loader42_FileName = 'Runtime Class Collections test';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
