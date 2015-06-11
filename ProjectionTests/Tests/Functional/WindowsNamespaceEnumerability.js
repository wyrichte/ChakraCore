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

    var expectedWindowsFoundationChildren = {
        AsyncStatus : 'object',
        Collections : 'object',
        Diagnostics : 'object',
        Metadata    : 'object',
        PropertyType    : 'object',
        Uri : 'function',
        WwwFormUrlDecoder   : 'function'
    };

    runner.addTest({
        id: 1,
        desc: 'Namespace Enumerability -- own Winmd only (Windows.Foundation)',
        pri: '0',
        test: function () {
            var ns = Windows.Foundation;
            var expectedChildren = expectedWindowsFoundationChildren;

            verify.forIn(ns, expectedChildren, "Windows.Foundation");
            verify.keys(ns, expectedChildren, "Windows.Foundation");
            verify.ownProperties(ns, expectedChildren, "Windows.Foundation");

            var keys1 = Object.keys(ns);
            verify(keys1, Object.keys(expectedChildren), "Object.Keys(Windows.Foundation)");

            keys1.reverse().forEach(function (value) {
                verify.typeOf(ns[value], expectedChildren[value]);
            });
            
            verify.enumerationOrder(ns, expectedChildren, "Windows.Foundation");
        }
    });

    var expectedWindowsFoundationMetadataChildren = {
        ActivatableAttribute    : 'function',
        AllowMultipleAttribute  : 'function',
        AttributeTargets    : 'object',
        AttributeUsageAttribute : 'function',
        ComposableAttribute : 'function',
        CompositionType : 'object',
        DefaultAttribute    : 'function',
        DefaultOverloadAttribute    : 'function',
        DualApiPartitionAttribute   : 'function',
        ExclusiveToAttribute    : 'function',
        GCPressureAmount    : 'object',
        GCPressureAttribute : 'function',
        GuidAttribute   : 'function',
        HasVariantAttribute : 'function',
        LengthIsAttribute   : 'function',
        MarshalingBehaviorAttribute : 'function',
        MarshalingType  : 'object',
        MuseAttribute : 'function',
        OverloadAttribute   : 'function',
        OverridableAttribute    : 'function',
        ProtectedAttribute  : 'function',
        RangeAttribute  : 'function',
        StaticAttribute : 'function',
        ThreadingAttribute  : 'function',
        ThreadingModel  : 'object',
        VariantAttribute    : 'function',
        VersionAttribute    : 'function',
        WebHostHiddenAttribute  : 'function',
    };

    // in windows blue there are new attributes
    // seems unit test is still running in win8 host type
    // following is for running in wwahost only
    if (Utils.isWWAHost()) {
        var newInBlue = {
            DeprecatedAttribute: 'function',
            DeprecationType: 'object',
            Platform: 'object',
            PlatformAttribute: 'function',
        };
        for (var k in newInBlue) {
            expectedWindowsFoundationMetadataChildren[k] = newInBlue[k];
        }
        var o = {};
        Object.keys(expectedWindowsFoundationMetadataChildren).sort().forEach(function (k) {
            o[k] = expectedWindowsFoundationMetadataChildren[k];
        });
        expectedWindowsFoundationMetadataChildren = o;
    }

    runner.addTest({
        id: 2,
        desc: 'Namespace Enumerability -- no metadata of exact name (Windows.Foundation.Metadata)',
        pri: '0',
        test: function () {
            var ns = Windows.Foundation.Metadata;
            var expectedChildren = expectedWindowsFoundationMetadataChildren;

            verify.forIn(ns, expectedChildren, "Windows.Foundation.Metadata");
            verify.keys(ns, expectedChildren, "Windows.Foundation.Metadata");
            verify.ownProperties(ns, expectedChildren, "Windows.Foundation.Metadata");

            var keys1 = Object.keys(ns);
            verify(keys1, Object.keys(expectedChildren), "Object.Keys(Windows.Foundation.Metadata)");

            keys1.reverse().forEach(function (value) {
                verify.typeOf(ns[value], expectedChildren[value]);
            });

            verify.enumerationOrder(ns, expectedChildren, "Windows.Foundation.Metadata");
        }
    });

    runner.addTest({
        id: 3,
        desc: 'Namespace Enumerability -- only child namespace Winmds (Windows)',
        pri: '0',
        test: function () {
            logger.comment("NOTE: Only checking for enumerability of some (not all) members due to high churn.");
            enumerateOver(Windows, "Windows", true, "Storage");
            enumerateOver(Windows, "Windows", true, "Foundation");
            enumerateOver(Windows, "Windows", true, "Devices");
            enumerateOver(Windows, "Windows", true, "UI");

            // Get baseline enumeration order 
            var keys1 = Object.keys(Windows);

            // Populate some namespace children
            verify.typeOf(Windows.Storage, 'object');
            verify.typeOf(Windows.Foundation, 'object');
            verify.typeOf(Windows.UI, 'object');

            // Check order is the same
            verify.enumerationOrder(Windows, keys1, "Windows");
            
            verify.typeOf(Windows.Devices, 'object');

            verify(Object.keys(Windows), keys1, "Object.keys(Windows)");
        }
    });

    runner.addTest({
        id: 4,
        desc: 'Namespace Enumerability -- all namespace children are enumerable true',
        pri: '0',
        test: function() {
            var p;

            // No metadata of exact name
            logger.comment("Verify all children of first party namespace (Windows.Foundation.Metadata) are enumerable true");
            var ns = Windows.Foundation.Metadata;
            for (p in ns) {
                verify(verifyDescriptor(ns, p, {enumerable : true}), true, 'Enumerable: '+p);
            }

            // Own metadata only
            logger.comment("Verify all children of first party namespace (Windows.Foundation) are enumerable true");
            ns = Windows.Foundation;
            for (p in ns) {
                verify(verifyDescriptor(ns, p, {enumerable : true}), true, 'Enumerable: '+p);
            }

            // Child namespace metadata only
            logger.comment("Verify all children of first party namespace (Windows) are enumerable true");
            ns = Windows;
            for (p in ns) {
                verify(verifyDescriptor(ns, p, {enumerable : true}), true, 'Enumerable: '+p);
            }
        }
    });


    Loader42_FileName = "Enumeration Tests over Windows Namespaces";
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
