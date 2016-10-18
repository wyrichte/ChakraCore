if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {

    // This test is similar to NamespaceEnumerability.js, except that any runtime class that doesn't have the AllowForWeb
    // attribute is expected to be hidden

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
            if (obj.hasOwnProperty(prop) === false) {
                logger.comment('addAndCheckProperty(' + prop + '): adding empty property');
                obj[prop] = undefined;
            }

            logger.comment('addAndCheckProperty(' + prop + '): ' + JSON.stringify(Object.getOwnPropertyDescriptor(obj, prop)));
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

    var expectedWineryChildren = {
        AllowForWebCustomAsyncInfo : 'function',
        IEnumerable : 'object',
        Overloading : 'object',
        RWinery : 'function',
        WinRTErrorTests : 'object',
        reds    : 'object',
        sweets  : 'object',
        whites  : 'object'
    };

    runner.addTest({
        id: 1,
        desc: 'Namespace Enumerability -- own Winmd only (Winery)',
        pri: '0',
        test: function () {
            var wineryOri = Winery.prototype;
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

            // revert changes
            Winery.prototype = wineryOri;
        }
    });

    // BakeOperation, Chef, Cookie, Kitchen, Oven, TimerOperation, Toast, and Toaster are all hidden
    var expectedFabrikamKitchenChildren = {
        ChefCapabilities    : 'object',
        ChefRole    : 'object',
        CookieDoneness  : 'object'
    };

    runner.addTest({
        id: 2,
        desc: 'Namespace Enumerability -- no metadata of exact name (Fabrikam.Kitchen)',
        pri: '0',
        test: function () {
            var fabrikamKitchenOri = Fabrikam.Kitchen.prototype;
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

            // revert changes
            Fabrikam.Kitchen.prototype = fabrikamKitchenOri;
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
        GCPressure  : 'object',
        Repros  : 'object',
        SimpleTestNamespace : 'object',
        Versioning  : 'object',
        constantValue   : 'number',
        lastUsedNamespace   : 'object',
        last    : 'object'
    };

    runner.addTest({
        id: 3,
        desc: 'Namespace Enumerability -- both own and child namespace Winmds (DevTests)',
        pri: '0',
        test: function () {
            var ns = DevTests;
            var expectedChildren;
            if (typeof WScript !== 'undefined') {
                expectedChildren = expectedDevTestsChildren;
            } else {
                expectedChildren = expectedDevTestsChildrenNoExpandos;
            }

            // setup
            // non-configurable, enumerable, accessor property
            addAndCheckProperty(DevTests, 'lastUsedNamespace', true, {
                get: function () {
                    return this.last;
                },
                set: function (val) {
                    this.last = val;
                },
                configurable: false,
                enumerable: true
            });
            setAndCheckProperty(DevTests, 'lastUsedNamespace', DevTests.CamelCasing, true);
            // /setup

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
        id: 4,
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

    Loader42_FileName = "AllowForWeb - Namespace Enumerability Tests";
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
