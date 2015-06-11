if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    var root = this;

    verify.forIn = function verifyMembers(obj, expected, msg) {
        logger.comment("for...in enumeration of " + msg);

        var expect;
        for (var mem in obj) {
            expect = expected[mem];
            verify.defined(expect, mem);
        }
    };

    verify.keys = function verifyKeys(obj, expected, msg) {
        logger.comment("Object.keys enumeration of " + msg);

        var members = Object.keys(obj);
        for (var mem in members) {
            var key = members[mem];
            verify.defined(expected[key], key);
        }
    }

    verify.ownProperties = function verifyOwnProperties(obj, expected, msg) {
        logger.comment("Object.getOwnPropertyNames enumeration of " + msg);

        var members = Object.getOwnPropertyNames(obj);
        for (var mem in members) {
            var propName = members[mem];
            verify.defined(expected[propName], propName);
        }
    }

    verify.enumerationOrder = function verifyEnumerationOrder(obj, expected, msg) {
        logger.comment("Verify order of enumeration over " + msg);

        var expectedArray;

        if (expected instanceof Array) { 
            expectedArray = expected;    
        } else {
            expectedArray = Object.keys(expected); 
        }
        
        var index = 0;
        var p;
        for (p in obj) {
            verify(p, expectedArray[index], "property: "+index)
            index++;
        }
        verify(expectedArray.length, index, "Total number of properties enumerated");
    }

    function verifyDescriptor(obj, prop, attributes) {
        if (((typeof obj !== "object") && (typeof obj !== "function")) || (typeof attributes !== "object")) {
            return false;
        }
        if (!(prop in obj)) {
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
            if (attributes.writable !== undefined) {
                if (attributes.writable && (desc.set === undefined)) { return false; }
                if (!attributes.writable && (desc.set !== undefined)) { return false; }
            }
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
        verify(prototype.isPrototypeOf(obj), true, "prototype.isPrototypeOf(obj)");
    }

    function enumerateOver(obj, name, enumerable, child, childEnumerable) {
        var found = false;
        var propList = [];
        try {
            for (var prop in obj) {
                if (prop == child) { found = true; }
                propList.push(prop);
            }
        }
        catch (e) {
            logger.comment("got exception when calling enumerateOver: " + e);
        }

        verify(propList.length > 0, enumerable, "Object (" + name + ") has enumerable properties");
        if (child !== undefined) {
            if (childEnumerable !== undefined) {
                verify(found, childEnumerable, "Found expected object property (" + child + ") when enumerating over " + name);
            } else {
                verify(found, enumerable, "Found expected object property (" + child + ") when enumerating over " + name);
            }
        }
    }

    function checkHasProperty(obj, prop, defined) {
        var succeeded;

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

    runner.addTest({
        id: 1,
        desc: 'RuntimeClass Constructor Attributes',
        pri: '0',
        test: function () {
            var animalConstructor = Animals.Animal;

            checkPrototype(Object.prototype, animalConstructor);

            // Verify the static properties and methods on the constructor are enumerable
            enumerateOver(animalConstructor, "Animals.Animal", true);

            // Verify the prototype is not writable
            setAndCheckProperty(animalConstructor, "prototype", 3, false);
            // Verify the prototype is not configurable
            deleteAndCheckProperty(animalConstructor, "prototype", false);

            // Verify the length is not writable
            setAndCheckProperty(animalConstructor, "length", 3, false);
            // Verify the length is not configurable
            deleteAndCheckProperty(animalConstructor, "length", false);

            // Verify a static method is not writable
            setAndCheckProperty(animalConstructor, "takeANap", "Take a nap.", false);
            // Verify a property with a setter is writable (with the correct type)
            setAndCheckProperty(animalConstructor, "isLovable", false, true);
            setAndCheckProperty(animalConstructor, "isLovable", true, true);
            // Verify a property with a setter is not writable (with an incorrect type)
            setAndCheckProperty(animalConstructor, "isLovable", "blah", false);
            // Verify a property with no setter is not writable
            setAndCheckProperty(animalConstructor, "myFishRefCount", 42, false);

            // Verify cannot define a property that already exists
            addAndCheckProperty(animalConstructor, "getAnswer", false);
            // Verify cannot define additional properties on constructor
            addAndCheckProperty(animalConstructor, "myFavoriteAnimal", false);
            // Verify cannot set additional properties on constructor
            setAndCheckProperty(animalConstructor, "myProperty", "hello", false);

            // Verify cannot delete properties from constructor
            deleteAndCheckProperty(animalConstructor, "multiplyNumbers", false);
        }
    });

    runner.addTest({
        id: 2,
        desc: 'Non-activatable RuntimeClass Constructor Attributes',
        pri: '0',
        test: function () {
            var puppyConstructor = Animals.Pomapoodle;

            checkPrototype(Object.prototype, puppyConstructor);

            // Verify the static properties and methods on the constructor are enumerable
            enumerateOver(puppyConstructor, "Animals.Pomapoodle", true);

            // Verify the prototype is not writable
            setAndCheckProperty(puppyConstructor, "prototype", 3, false);
            // Verify the prototype is not configurable
            deleteAndCheckProperty(puppyConstructor, "prototype", false);

            // Verify the length is not writable
            setAndCheckProperty(puppyConstructor, "length", 3, false);
            // Verify the length is not configurable
            deleteAndCheckProperty(puppyConstructor, "length", false);

            // Verify a static method is not writable
            setAndCheckProperty(puppyConstructor, "eatCookies", "Eat cookies.", false);

            // Verify cannot define a property that already exists
            addAndCheckProperty(puppyConstructor, "eatCookies", false);
            // Verify cannot define additional properties on constructor
            addAndCheckProperty(puppyConstructor, "myFavoriteToy", false);
            // Verify cannot set additional properties on constructor
            setAndCheckProperty(puppyConstructor, "myProperty", "hello", false);

            // Verify cannot delete properties from constructor
            deleteAndCheckProperty(puppyConstructor, "eatCookies", false);
        }
    });

    runner.addTest({
        id: 3,
        desc: 'RuntimeClass Prototype Attributes -- Prototype from constructor',
        pri: '0',
        test: function () {
            var animalPrototype = Animals.Animal.prototype;

            checkPrototype(Object.prototype, animalPrototype);

            // Verify the properties and methods on the prototype are enumerable
            enumerateOver(animalPrototype, "Animals.Animal.prototype", true);

            // Verify a method is not writable
            setAndCheckProperty(animalPrototype, "getNumLegs", "Get number of legs.", false);
            // Verify a property with a setter is not writable (with the correct type)
            setAndCheckProperty(animalPrototype, "weight", 102, false);
            setAndCheckProperty(animalPrototype, "weight", 81, false);
            // Verify a property with a setter is not writable (with an incorrect type)
            setAndCheckProperty(animalPrototype, "mother", "blah", false);

            // Verify cannot define a property that already exists
            addAndCheckProperty(animalPrototype, "getDimensions", false);
            // Verify cannot define additional properties on prototype
            addAndCheckProperty(animalPrototype, "myFavoriteAnimal", false);
            // Verify cannot set additional properties on prototype
            setAndCheckProperty(animalPrototype, "myProperty", "hello", false);

            // Verify cannot delete properties from prototype
            deleteAndCheckProperty(animalPrototype, "myPhylum", false);
        }
    });

    runner.addTest({
        id: 4,
        desc: 'RuntimeClass Prototype Attributes -- Prototype from instance',
        pri: '0',
        test: function () {
            var animalInstance = new Animals.Animal();
            var animalPrototype = Object.getPrototypeOf(animalInstance);

            checkPrototype(Object.prototype, animalPrototype);

            // Verify the properties and methods on the prototype are enumerable
            enumerateOver(animalPrototype, "Object.getPrototypeOf(animalInstance)", true);

            // Verify a method is not writable
            setAndCheckProperty(animalPrototype, "getNumLegs", "Get number of legs.", false);
            // Verify a property with a setter is not writable (with the correct type)
            setAndCheckProperty(animalPrototype, "weight", 102, false);
            setAndCheckProperty(animalPrototype, "weight", 81, false);
            // Verify a property with a setter is not writable (with an incorrect type)
            setAndCheckProperty(animalPrototype, "mother", "blah", false);

            // Verify cannot define a property that already exists
            addAndCheckProperty(animalPrototype, "getDimensions", false);
            // Verify cannot define additional properties on prototype
            addAndCheckProperty(animalPrototype, "myFavoriteAnimal", false);
            // Verify cannot set additional properties on prototype
            setAndCheckProperty(animalPrototype, "myProperty", "hello", false);

            // Verify cannot delete properties from prototype
            deleteAndCheckProperty(animalPrototype, "myPhylum", false);
        }
    });

    runner.addTest({
        id: 5,
        desc: 'RuntimeClass Instance Attributes',
        pri: '0',
        test: function () {
            var animalInstance = new Animals.Animal();

            checkPrototype(Animals.Animal.prototype, animalInstance);

            // Verify the properties and methods on the instance are enumerable
            enumerateOver(animalInstance, animalInstance.toString(), true);

            // Verify a method is not writable
            setAndCheckProperty(animalInstance, "getNumLegs", "Get number of legs.", false);
            // Verify a property with a setter is writable (with the correct type)
            setAndCheckProperty(animalInstance, "weight", 102, true);
            setAndCheckProperty(animalInstance, "weight", 81, true);
            // Verify a property with a setter is not writable (with an incorrect type)
            setAndCheckProperty(animalInstance, "mother", "blah", false);

            // Verify cannot define a property that already exists
            addAndCheckProperty(animalInstance, "getDimensions", false);
            // Verify cannot define additional properties on instance
            addAndCheckProperty(animalInstance, "myFavoriteAnimal", false);
            // Verify cannot set additional properties on instance
            setAndCheckProperty(animalInstance, "myProperty", "hello", false);

            // Verify cannot delete properties from instance
            deleteAndCheckProperty(animalInstance, "myPhylum", false);
        }
    });

    runner.addTest({
        id: 6,
        desc: 'Enum Object Attributes',
        pri: '0',
        test: function () {
            var phylumEnum = Animals.Phylum;

            checkPrototype(Object.prototype, phylumEnum);

            // Verify the fields of the enum are enumerable
            enumerateOver(phylumEnum, phylumEnum.toString(), true);

            // Verify an existing enum field is not writable (with correct type)
            setAndCheckProperty(phylumEnum, "nematomorpha", 56, false);
            // Verify an existing enum field is not writable (with an incorrect type)
            setAndCheckProperty(phylumEnum, "echinodermata", "blah", false);

            // Verify cannot define a field that already exists
            addAndCheckProperty(phylumEnum, "brachiopoda", false);
            // Verify cannot define additional properties on enum
            addAndCheckProperty(phylumEnum, "myFavoriteAnimal", false);
            // Verify cannot set additional properties on enum
            setAndCheckProperty(phylumEnum, "myField", 71, false);

            // Verify cannot delete properties from enum
            deleteAndCheckProperty(phylumEnum, "tardigrada", false);
        }
    });

    runner.addTest({
        id: 7,
        desc: 'Struct Instance Attributes -- simple struct',
        pri: '0',
        test: function () {
            var animalInstance = new Animals.Animal();
            var dimensionsStruct = animalInstance.getDimensions();

            checkPrototype(Object.prototype, dimensionsStruct);

            // Verify the fields of the struct are enumerable
            enumerateOver(dimensionsStruct, dimensionsStruct.toString(), true);

            // Verify an existing struct field is writable (with correct type)
            setAndCheckProperty(dimensionsStruct, "width", 56, true);
            // Verify an existing struct field is writable (with an incorrect type)
            setAndCheckProperty(dimensionsStruct, "length", "blah", true);

            // Verify can define a field that already exists (properties are configurable)
            addAndCheckProperty(dimensionsStruct, "width", true);
            // Verify cannot define additional properties on struct
            addAndCheckProperty(dimensionsStruct, "myFavoriteAnimal", false);
            // Verify cannot set additional properties on struct
            setAndCheckProperty(dimensionsStruct, "myField", 71, false);

            // Verify can delete properties from struct (properties are configurable)
            deleteAndCheckProperty(dimensionsStruct, "length", true);
            // Verify cannot set a property after it's been deleted (non-extensible)
            setAndCheckProperty(dimensionsStruct, "length", 50, false);
        }
    });

    runner.addTest({
        id: 8,
        desc: 'Struct Instance Attributes -- nested struct',
        pri: '0',
        test: function () {
            var animalConstructor = Animals.Animal;
            var bigComplexStructIn = {
               field0: 16,
               field1: { field0: 255 },
               field2: 55,
               field3: { field0: true, field1: false, field2: false, field3: true },
               field4: { field0: 129, field1: { field0: 3 }, field2: 82 },
               field5: { field0: 24, field1: { field0: 198 }, field2: 222 },
               field6: 73,
               field7: 1000,
            };

            var bigComplexStructOut = animalConstructor.marshalInAndOutBigComplexStruct(bigComplexStructIn);

            checkPrototype(Object.prototype, bigComplexStructOut);

            // Verify the fields of the struct are enumerable
            enumerateOver(bigComplexStructOut, bigComplexStructOut.toString(), true);

            // Verify an existing struct field is writable (with correct type)
            setAndCheckProperty(bigComplexStructOut, "field0", 56, true);
            // Verify an existing struct field is writable (with an incorrect type)
            setAndCheckProperty(bigComplexStructOut, "field1", "blah", true);

            // Verify can define a field that already exists (properties are configurable)
            addAndCheckProperty(bigComplexStructOut, "field3", true);
            // Verify cannot define additional properties on struct
            addAndCheckProperty(bigComplexStructOut, "myFavoriteAnimal", false);
            // Verify cannot set additional properties on struct
            setAndCheckProperty(bigComplexStructOut, "myField", 71, false);

            // Verify can delete properties from struct (properties are configurable)
            deleteAndCheckProperty(bigComplexStructOut, "field4", true);
            // Verify cannot set a property after it's been deleted (non-extensible)
            setAndCheckProperty(bigComplexStructOut, "field4", 50, false);

            var nestedStruct = bigComplexStructOut.field5;

            checkPrototype(Object.prototype, nestedStruct);

            // Verify the fields of the struct are enumerable
            enumerateOver(nestedStruct, nestedStruct.toString(), true);

            // Verify an existing struct field is writable (with correct type)
            setAndCheckProperty(nestedStruct, "field0", 56, true);
            // Verify an existing struct field is writable (with an incorrect type)
            setAndCheckProperty(nestedStruct, "field1", "blah", true);

            // Verify can define a field that already exists (properties are configurable)
            addAndCheckProperty(nestedStruct, "field2", true);
            // Verify cannot define additional properties on struct
            addAndCheckProperty(nestedStruct, "myFavoriteAnimal", false);
            // Verify cannot set additional properties on struct
            setAndCheckProperty(nestedStruct, "myField", 71, false);

            // Verify can delete properties from struct (properties are configurable)
            deleteAndCheckProperty(nestedStruct, "field0", true);
            // Verify cannot set a property after it's been deleted (non-extensible)
            setAndCheckProperty(nestedStruct, "field0", 50, false);
        }
    });

    runner.addTest({
        id: 9,
        desc: 'Return Object Attributes -- multiple [out] parameters',
        pri: '0',
        test: function () {
            var animalInstance = new Animals.Animal();
            var outputObject = animalInstance.layoutOfManyMembers(12, 2044, 134, 54.3, 79, 237, 9.4082, -423, 604.35);

            checkPrototype(Object.prototype, outputObject);

            // Verify the properties of the object are enumerable
            enumerateOver(outputObject, outputObject.toString(), true);

            // Verify an existing object property is writable (with correct type)
            setAndCheckProperty(outputObject, "reta", 56, true);
            // Verify an existing object property is writable (with an incorrect type)
            setAndCheckProperty(outputObject, "retb", "blah", true);

            // Verify can define a field that already exists (properties are configurable)
            addAndCheckProperty(outputObject, "retd", true);
            // Verify can define additional properties on object
            addAndCheckProperty(outputObject, "myFavoriteAnimal", true);
            // Verify can set additional properties on object
            setAndCheckProperty(outputObject, "myField", 71, true);

            // Verify can delete properties from object (properties are configurable)
            deleteAndCheckProperty(outputObject, "reth", true);
            // Verify can set a property after it's been deleted (is extensible)
            setAndCheckProperty(outputObject, "reth", 50, true);
        }
    });

    runner.addTest({
        id: 10,
        desc: 'Promises Attributes -- promises prototypes',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var promise = winery.asyncOperationOut();

            checkPrototype(Object.prototype, promise);
            checkPrototype(Object.prototype, promise.then);
            checkPrototype(Object.prototype, promise.cancel);
            checkPrototype(Object.prototype, promise.operation);

            var promisePrototype = Object.getPrototypeOf(promise);
            var thenPrototype = Object.getPrototypeOf(promise.then);
            var cancelPrototype = Object.getPrototypeOf(promise.cancel);
            var operationPrototype = Object.getPrototypeOf(promise.operation);
            var thenFunctionPrototype = Object.getPrototypeOf(promise.then());

            verify(thenPrototype === Function.prototype, true, "Object.getPrototypeOf(promise.then) === Function.prototype");
            verify(cancelPrototype === Function.prototype, true, "Object.getPrototypeOf(promise.cancel) === Function.prototype");
            verify(thenFunctionPrototype === Function.prototype, false, "Object.getPrototypeOf(promise.then()) === Function.prototype");
            verify(thenFunctionPrototype === Object.prototype, false, "Object.getPrototypeOf(promise.then()) === Object.prototype");
            verify(promisePrototype === Object.prototype, false, "Object.getPrototypeOf(promise) === Object.prototype");

            verify(operationPrototype, Winery.CustomAsyncInfo.prototype, "Object.getPrototypeOf(promise.operation)");

            // TEST ATTRIBUTES OF THE OPERATION PROTOTYPE
            // Verify the properties and methods on the prototype are enumerable
            enumerateOver(operationPrototype, "Object.getPrototypeOf(promise.operation)", true);

            // Verify a method is not writable
            setAndCheckProperty(operationPrototype, "cancel", "Cancel.", false);
            // Verify a property with a getter only is not writable
            setAndCheckProperty(operationPrototype, "id", 102, false);
            // Verify a property with a setter is not writable (with the correct type)
            setAndCheckProperty(operationPrototype, "progress", function (async, percent) {
                logger.comment("Progress made: " + percent);
            }, false);
            // Verify a property with a setter is not writable (with an incorrect type)
            setAndCheckProperty(operationPrototype, "completed", "blah", false);

            // Verify cannot define a property that already exists
            addAndCheckProperty(operationPrototype, "getResults", false);
            // Verify cannot define additional properties on prototype
            addAndCheckProperty(operationPrototype, "myOperationMethod", false);
            // Verify cannot set additional properties on prototype
            setAndCheckProperty(operationPrototype, "myProperty", "hello", false);

            // Verify cannot delete properties from prototype
            deleteAndCheckProperty(operationPrototype, "status", false);

            // TEST ATTRIBUTES OF THE PROMISE PROTOTYPE
            // Verify the properties and methods on the prototype are enumerable
            enumerateOver(promisePrototype, "Object.getPrototypeOf(promise)", false);

            // Verify cannot define additional properties on prototype
            addAndCheckProperty(promisePrototype, "myPromiseMethod", false);
            // Verify cannot set additional properties on prototype
            setAndCheckProperty(promisePrototype, "myProperty", "hello", false);
        
            // TEST ATTRIBUTES OF THE THEN FUNCTION PROTOTYPE
            // Verify the properties and methods on the prototype are enumerable
            enumerateOver(thenFunctionPrototype, "Object.getPrototypeOf(promise.then())", false);

            // Verify cannot define additional properties on prototype
            addAndCheckProperty(thenFunctionPrototype, "myPromiseMethod", false);
            // Verify cannot set additional properties on prototype
            setAndCheckProperty(thenFunctionPrototype, "myProperty", "hello", false);
        }
    });

    runner.addTest({
        id: 11,
        desc: 'Promises Attributes -- promises instance',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var promise = winery.asyncOperationOut();

            var thenProperty = promise.then;
            var thenFunction = promise.then();
            var cancelProperty = promise.cancel;
            var asyncOperation = promise.operation;

            // TEST ATTRIBUTES OF PROMISES INSTANCE OBJECT
            // Verify the properties and methods on the object are enumerable
            enumerateOver(promise, "promise", true);

            // Verify a method is not writable
            setAndCheckProperty(promise, "cancel", "Cancel.", false);

            // Verify cannot define a property that already exists
            addAndCheckProperty(promise, "then", false);
            // Verify cannot define additional properties on object
            addAndCheckProperty(promise, "myPromisesMethod", false);
            // Verify cannot set additional properties on object
            setAndCheckProperty(promise, "myProperty", "hello", false);

            // Verify cannot delete properties from object
            deleteAndCheckProperty(promise, "operation", false);

            // TEST ATTRIBUTES OF OPERATION PROPERTY
            // Verify the properties and methods on the object are enumerable
            enumerateOver(asyncOperation, "promise.operation", true);

            // Verify a method is not writable
            setAndCheckProperty(asyncOperation, "cancel", "Cancel.", false);
            // Verify a property with a getter only is not writable
            setAndCheckProperty(asyncOperation, "id", 102, false);
            // Verify a property with a setter is writable (with the correct type)
            setAndCheckProperty(asyncOperation, "progress", function (async, percent) {
                logger.comment("Progress made: " + percent);
            }, true);
            // Verify a property with a setter is not writable (with an incorrect type)
            setAndCheckProperty(asyncOperation, "completed", "blah", false);

            // Verify cannot define a property that already exists
            addAndCheckProperty(asyncOperation, "getResults", false);
            // Verify cannot define additional properties on object
            addAndCheckProperty(asyncOperation, "myOperationMethod", false);
            // Verify cannot set additional properties on object
            setAndCheckProperty(asyncOperation, "myProperty", "hello", false);

            // Verify cannot delete properties from object
            deleteAndCheckProperty(asyncOperation, "status", false);

            // TEST ATTRIBUTES OF THEN PROPERTY
            // Verify the properties and methods on the object are enumerable
            enumerateOver(thenFunction, "promise.then()", true);

            // Verify a method is not writable
            setAndCheckProperty(thenFunction, "cancel", "Cancel.", false);

            // Verify cannot define a property that already exists
            addAndCheckProperty(thenFunction, "then", false);
            // Verify cannot define additional properties on object
            addAndCheckProperty(thenFunction, "myPromisesMethod", false);
            // Verify cannot set additional properties on object
            setAndCheckProperty(thenFunction, "myProperty", "hello", false);

            // Verify cannot delete properties from object
            deleteAndCheckProperty(thenFunction, "cancel", false);

            // TEST ATTRIBUTES OF THEN PROPERTY
            // Verify the properties and methods on the object are enumerable
            enumerateOver(thenProperty, "promise.then", false);

            // Verify cannot define additional properties on object
            addAndCheckProperty(promise, "myPromisesMethod", false);
            // Verify cannot set additional properties on object
            setAndCheckProperty(promise, "myProperty", "hello", false);

            // TEST ATTRIBUTES OF CANCEL PROPERTY
            // Verify the properties and methods on the object are enumerable
            enumerateOver(cancelProperty, "promise.cancel", false);

            // Verify cannot define additional properties on object
            addAndCheckProperty(cancelProperty, "myPromisesMethod", false);
            // Verify cannot set additional properties on object
            setAndCheckProperty(cancelProperty, "myProperty", "hello", false);
        }
    });

    var expectedFabrikamChildrenNoExpandos = {
        Kitchen : 'object',
    };

    runner.addTest({
        id: 12,
        desc: 'Namespace Attributes -- non-extensible',
        pri: '0',
        test: function () {
            checkPrototype(Object.prototype, Fabrikam);

            // Verify the namespace is enumerable on the global object
            enumerateOver(root, "Global object", true, "Fabrikam");
            // Verify the order of the enumerable namespace properties
            verify.enumerationOrder(Fabrikam, expectedFabrikamChildrenNoExpandos, "Fabrikam");
            // Verify the namespace has enumerable properties
            enumerateOver(Fabrikam, "Fabrikam", true, "Kitchen");

            // Verify Fabrikam has subnamespace Kitchen (will cause property "Kitchen to be added to Fabrikam)
            checkHasProperty(Fabrikam, "Kitchen", true);
            // Verify cannot set property that we know already exists on the namespace
            setAndCheckProperty(Fabrikam, "Kitchen", "my Kitchen namespace", false);

            // Verify cannot set property that has not been populated on the namespace
            setAndCheckProperty(Fabrikam.Kitchen, "Kitchen", "my Kitchen class", false);

            // Populate Fabrikam.Kitchen members
            checkHasProperty(Fabrikam.Kitchen, "Chef", true);
            checkHasProperty(Fabrikam.Kitchen, "Toaster", true);

            // Verify enumerable properties on Fabrikam after populating at least one child
            enumerateOver(Fabrikam, "Fabrikam", true);
            //Verify enumerable properties on subnamespace Kitchen after populating children
            enumerateOver(Fabrikam.Kitchen, "Fabrikam.Kitchen", true, "CookieDoneness");
            enumerateOver(Fabrikam.Kitchen, "Fabrikam.Kitchen", true, "Dimensions", false);

            // Verify cannot re-define projected namespace property
            addAndCheckProperty(root, 'Fabrikam', false, { writable: true, configurable: true, extensible: true, value: Fabrikam });
            // Verify cannot re-define projected property on namespace
            addAndCheckProperty(Fabrikam, 'Kitchen', false, { writable: true, configurable: true, extensible: true, value: Fabrikam.Kitchen });
            addAndCheckProperty(Fabrikam.Kitchen, 'Chef', false, { writable: true, configurable: true, extensible: true, value: Fabrikam.Kitchen.Chef });

            // currently wwahost always mark 3rd WinRT component as extensible. 
            // so no way to test non-extensible behavior in wwahost for 3rd WinRT anymore
            if (!Utils.isWWAHost()) {
                logger.comment("Running in jshost");
                // Verify cannot define additional properties on namespace
                addAndCheckProperty(Fabrikam, 'Bedroom', false);
                addAndCheckProperty(Fabrikam.Kitchen, 'Utensils', false);
                // Verify cannot set additional properties on namespace
                setAndCheckProperty(Fabrikam, "LivingRoom", { furniture: ["couch", "tv", "coffee table", "lamp"], lighting: "moderate" }, false);
                setAndCheckProperty(Fabrikam.Kitchen, "size", 200, false);
            } else { // wwahost
                logger.comment("Running in wwahost, the 3rd WinRT objects are always extensible in win Blue");
                // Verify cannot define additional properties on namespace
                addAndCheckProperty(Fabrikam, 'Bedroom', true);
                addAndCheckProperty(Fabrikam.Kitchen, 'Utensils', true);
                // Verify cannot set additional properties on namespace
                setAndCheckProperty(Fabrikam, "LivingRoom", { furniture: ["couch", "tv", "coffee table", "lamp"], lighting: "moderate" }, true);
                setAndCheckProperty(Fabrikam.Kitchen, "size", 200, true);

                //cleanup for test #15 when testing in wwahost
                delete Fabrikam.Bedroom;
                delete Fabrikam.Kitchen.Utensils;
                delete Fabrikam.LivingRoom;
                delete Fabrikam.Kitchen.size;
            }


            // Verify cannot delete namespace properties
            deleteAndCheckProperty(root, 'Fabrikam', false);
            // Verify cannot delete projected property on namespace
            deleteAndCheckProperty(Fabrikam, 'Kitchen', false);
            deleteAndCheckProperty(Fabrikam.Kitchen, 'Toaster', false);
            // Verify cannot delete projected property (not yet populated)
            delete Fabrikam.Kitchen.Cookie;
            checkHasProperty(Fabrikam.Kitchen, 'Cookie', true);
        }
    });

    var expectedDevTestsChildrenNoExpandos = {
        Arm : 'object',
        CamelCasing : 'object',
        DateTimeAndTimeSpan : 'object',
        Delegates : 'object',
        GCPressure : 'object',
        Repros  : 'object',
        SimpleTestNamespace : 'object',
        Versioning  : 'object'
    };

    runner.addTest({
        id: 13,
        desc: 'Namespace Attributes -- extensible',
        pri: '0',
        preReq: function () {
            return (typeof TestUtilities !== 'undefined');
        },
        test: function () {
            checkPrototype(Object.prototype, DevTests);

            // Verify the namespace is enumerable on the global object
            enumerateOver(root, "Global object", true, "DevTests");
            // Verify the order of the enumerable namespace properties before expandos
            verify.enumerationOrder(DevTests, expectedDevTestsChildrenNoExpandos, "DevTests");
            // Verify the namespace has enumerable properties
            enumerateOver(DevTests, "DevTests", true, "Arm");

            // Verify Fabrikam has subnamespace CamelCasing (will cause property "CamelCasing" to be added to DevTests)
            checkHasProperty(DevTests, "CamelCasing", true);
            // Verify cannot set property that we know already exists on the namespace
            setAndCheckProperty(DevTests, "CamelCasing", "camelCase tests", false);

            // Verify cannot set property that has not been populated on the namespace
            setAndCheckProperty(DevTests, "DateTimeAndTimeSpan", "DateTime and TimeSpan tests", false);

            // Populate DevTests.CamelCasing members
            checkHasProperty(DevTests.CamelCasing, "StringVariations", true);
            checkHasProperty(DevTests.CamelCasing, "CrossMemberCollisions", true);
            checkHasProperty(DevTests.CamelCasing, "SimpleNameCollisions", true);

            // Verify enumerable properties on DevTests after populating only projected children
            enumerateOver(DevTests, "DevTests", true);
            //Verify enumerable properties on subnamespace CamelCasing after populating only projected children
            enumerateOver(DevTests.CamelCasing, "DevTests.CamelCasing", true, "StringVariations");
            enumerateOver(DevTests.CamelCasing, "DevTests.CamelCasing", true, "CamelCasingHandler", false);


            // Verify can define additional properties on namespace
            // non-writable, non-configurable, non-enumerable, data property
            addAndCheckProperty(DevTests, 'constantValue', true, { writable: false, configurable: false, enumerable: false, value: 42 });
            // writable, configurable, enumerable, data property
            addAndCheckProperty(DevTests.DateTimeAndTimeSpan, 'samples', true, {writable: true, configurable: true, enumerable: true, value: { sample1: function() { return true; }, sample2: function () { return false; }}});
            // non-configurable, enumerable, accessor property
            addAndCheckProperty(DevTests, 'lastUsedNamespace', true, { 
                get: function () { 
                    return this.last; 
                }, 
                set: function (val) { 
                    this.last = val; 
                },
                configurable: false,
                enumerable: true});
            // non-writable, configurable non-enumerable array property
            addAndCheckProperty(DevTests.CamelCasing, 'colors', true, {
                writable: false,
                configurable: true,
                enumerable: false,
                value: ['blue', 'green', 'red', 'aqua', 'purple', 'yellow'] });
            //writable, non-configurable, enumerable data property
            addAndCheckProperty(DevTests.CamelCasing, 'storedData', true, { writable: true, configurable: false, enumerable: true, value: "data" });

            // Verify can set additional properties on namespace
            setAndCheckProperty(DevTests, "testHelpers", { createTest: function() { }}, true);
            setAndCheckProperty(DevTests.CamelCasing, "numTests", 200, true);

            // Verify can enumerate enumerable expandos and projected objects
            enumerateOver(DevTests, "DevTests", true, "lastUsedNamespace", true);
            enumerateOver(DevTests.CamelCasing, "DevTests.CamelCasing", true, "numTests", true);
            enumerateOver(DevTests.DateTimeAndTimeSpan, "DevTests.DateTimeAndTimeSpan", true, "samples", true);
            enumerateOver(DevTests, "DevTests", true, "CamelCasing");
            enumerateOver(DevTests.DateTimeAndTimeSpan, "DevTests.DateTimeAndTimeSpan", true, "Tests");
            // Verify cannot enumerate non-enumerable expandos
            enumerateOver(DevTests, "DevTests", true, "constantValue", false);
            enumerateOver(DevTests.CamelCasing, "DevTests.CamelCasing", true, "colors", false);

            // Verify cannot re-define projected namespace property
            addAndCheckProperty(root, 'DevTests', false, { writable: true, configurable: true, extensible: true, value: DevTests });
            // Verify cannot re-define projected property on namespace
            addAndCheckProperty(DevTests, 'DateTimeAndTimeSpan', false, { writable: true, configurable: true, extensible: true, value: DevTests.DateTimeAndTimeSpan });
            addAndCheckProperty(DevTests.CamelCasing, 'SimpleNameCollisions', false, { writable: true, configurable: true, extensible: true, value: DevTests.CamelCasing.SimpleNameCollisions });


            // Verify can set writable expandos
            setAndCheckProperty(DevTests, 'lastUsedNamespace', DevTests.CamelCasing, true);
            setAndCheckProperty(DevTests.DateTimeAndTimeSpan, 'samples', undefined, true);
            // Verify cannot set non-writable expandos
            setAndCheckProperty(DevTests, 'constantValue', 0, false);
            setAndCheckProperty(DevTests.CamelCasing, 'colors', ['black', 'white'], false);

            // Verify can re-define configurable expandos
            addAndCheckProperty(DevTests, "testHelpers", true);
            addAndCheckProperty(DevTests.DateTimeAndTimeSpan, "samples", true, { value: {} });
            // Verify cannot re-define non-configurable expandos
            addAndCheckProperty(DevTests, "constantValue", false);
            addAndCheckProperty(DevTests.CamelCasing, "storedData", false);

            // Verify can delete configurable expandos
            deleteAndCheckProperty(DevTests, 'testHelpers', true);
            deleteAndCheckProperty(DevTests.CamelCasing, 'colors', true);
            // Verify cannot delete non-configurable expandos
            deleteAndCheckProperty(DevTests, 'lastUsedNamespace', false);
            deleteAndCheckProperty(DevTests.CamelCasing, 'storedData', false);

            // Verify cannot delete namespace properties
            deleteAndCheckProperty(root, 'DevTests', false);
            // Verify cannot delete projected property on namespace
            deleteAndCheckProperty(DevTests, 'DateTimeAndTimeSpan', false);
            deleteAndCheckProperty(DevTests.CamelCasing, 'StringVariations', false);
            // Verify cannot delete projected property (not yet populated)
            delete DevTests.CamelCasing.OverloadStringVariations;
            checkHasProperty(DevTests.CamelCasing, 'OverloadStringVariations', true);

        }
    });

    var expectedWineryChildren = {
        AllowForWebCustomAsyncInfo: 'function',
        CustomAsyncInfo : 'function',
        IEnumerable : 'object',
        Overloading : 'object',
        RWinery : 'function',
        WinRTErrorTests : 'object',
        reds    : 'object',
        sweets  : 'object',
        whites  : 'object'
    };

    runner.addTest({
        id: 14,
        desc: 'Namespace Enumerability -- own Winmd only (Winery)',
        pri: '0',
        test: function () {
            var ns = Winery;
            var expectedChildren = expectedWineryChildren;

            verify.forIn(ns, expectedChildren, "Winery");
            verify.keys(ns, expectedChildren, "Winery");
            verify.ownProperties(ns, expectedChildren, "Winery");

            var keys1 = Object.keys(ns);
            verify(keys1, Object.keys(expectedChildren), "Object.Keys(Winery)");

            keys1.reverse().forEach(function (value) {
                verify.typeOf(ns[value], expectedChildren[value]);
            });

            verify.enumerationOrder(ns, expectedChildren, "Winery");
       }
    });

    var expectedFabrikamKitchenChildren = {
        BakeOperation   : 'function',
        Chef    : 'function',
        ChefCapabilities    : 'object',
        ChefRole    : 'object',
        Cookie  : 'function',
        CookieDoneness  : 'object',
        Kitchen : 'function',
        Oven    : 'function',
        TimerOperation  : 'function',
        Toast   : 'function',
        Toaster : 'function'
    };

    runner.addTest({
        id: 15,
        desc: 'Namespace Enumerability -- no metadata of exact name (Fabrikam.Kitchen)',
        pri: '0',
        test: function () {
            var ns = Fabrikam.Kitchen;
            var expectedChildren = expectedFabrikamKitchenChildren;

            verify.forIn(ns, expectedChildren, "Fabrikam.Kitchen");
            verify.keys(ns, expectedChildren, "Fabrikam.Kitchen");
            verify.ownProperties(ns, expectedChildren, "Fabrikam.Kitchen");

            var keys1 = Object.keys(ns);
            verify(keys1, Object.keys(expectedChildren), "Object.Keys(Fabrikam.Kitchen)");

            keys1.reverse().forEach(function (value) {
                verify.typeOf(ns[value], expectedChildren[value]);
            });

            verify.enumerationOrder(ns, expectedChildren, "Fabrikam.Kitchen");
        }
    });

    var expectedDevTestsChildren = {
        Arm : 'object',
        CamelCasing : 'object',
        DateTimeAndTimeSpan : 'object',
        Delegates  : 'object',
        GCPressure  : 'object',
        Repros  : 'object',
        SimpleTestNamespace : 'object',
        Versioning  : 'object',
        lastUsedNamespace   : 'object',
        last    : 'object'
    };

    var allDevTestsChildren = {
        Arm : 'object',
        CamelCasing : 'object',
        DateTimeAndTimeSpan : 'object',
        Delegates  : 'object',
        GCPressure : 'object',
        Repros  : 'object',
        SimpleTestNamespace : 'object',
        Versioning  : 'object',
        constantValue   : 'number',
        lastUsedNamespace   : 'object',
        last    : 'object'
    };

    runner.addTest({
        id: 16,
        desc: 'Namespace Enumerability -- both own and child namespace Winmds (DevTests)',
        pri: '0',
        test: function () {
            var ns = DevTests;
            var expectedChildren;
            if (typeof TestUtilities !== 'undefined') {
                expectedChildren = expectedDevTestsChildren;
            } else {
                expectedChildren = expectedDevTestsChildrenNoExpandos;
            }

            verify.forIn(ns, expectedChildren, "DevTests");
            verify.keys(ns, expectedChildren, "DevTests");
            verify.ownProperties(ns, allDevTestsChildren, "DevTests");

            var keys1 = Object.keys(ns);
            verify(keys1, Object.keys(expectedChildren), "Object.Keys(DevTests)");

            keys1.reverse().forEach(function (value) {
                verify.typeOf(ns[value], expectedChildren[value]);
            });

            verify.enumerationOrder(ns, expectedChildren, "DevTests");
        }
    });

    runner.addTest({
        id: 17,
        desc: 'Namespace Enumerability -- all namespace children are enumerable true',
        pri: '0',
        test: function() {
            var p;

            // Own metadata only
            logger.comment("Verify all children of third party namespace (Winery) are enumerable true");
            var ns = Winery;
            for (p in ns) {
                verify(verifyDescriptor(ns, p, {enumerable : true}), true, 'Enumerable: '+p);
            }

            // No metadata of exact name
            logger.comment("Verify all children of third party namespace (Fabrikam.Kitchen) are enumerable true");
            ns = Fabrikam.Kitchen;
            for (p in ns) {
                verify(verifyDescriptor(ns, p, {enumerable : true}), true, 'Enumerable: '+p);
            }

            // Own metadata only
            logger.comment("Verify all children of third party namespace (Animals) are enumerable true");
            ns = Animals;
            for (p in ns) {
                verify(verifyDescriptor(ns, p, {enumerable : true}), true, 'Enumerable: '+p);
            }
        }
    });


    Loader42_FileName = "Projected Object Attributes Tests";
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
