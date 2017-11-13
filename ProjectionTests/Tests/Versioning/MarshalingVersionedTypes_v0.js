if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    var versions = {
        min     : 0,
        win8    : 0x06020000,
        win8sp1 : 0x06020100,
        win9    : 0x06030000,
        max     : 0xFFFFFFFE
    }

    verify.members = function verifyMembers(obj, expected, msg) {
        logger.comment("Verifying members of " + msg);

        var expect;
        var numMembers = 0;
        for (var mem in obj) {
            numMembers++;
            expect = expected[mem];
            verify.defined(expect, mem);
            verify.typeOf(obj[mem], expect, mem);
        }
        verify(numMembers, Object.keys(expected).length, "Number of members == number of members expected");
    };

    verify.fields = function verifyEnum(obj, expectedFields, unexpectedFields, msg) {
        logger.comment("Verifying fields of " + msg);
        for (var field in expectedFields) {
            verify.defined(obj[field], field);
            verify(obj[field], expectedFields[field], field);
        }

        for (var field in unexpectedFields) {
            verify.typeOf(obj[field], unexpectedFields[field]);
        }
    }

    var rtc = DevTests.Versioning.MarshalVersionedTypes;
    var rtcName = "DevTests.Versioning.MarshalVersionedTypes";

    runner.addTest({
        id: 0,
        desc: 'DevTests.Versioning.MarshalVersionedTypes Members',
        pri: '0',
        test: function () {
            // Verify class exists
            verify.defined(rtc, rtcName);

            // Verify can construct an instance
            var instance = new rtc();
            verify.defined(instance, rtcName + " instance");

            var membersExpected = {
                // Version 0 types
                minVersionInterfaceIn       : 'function',
                minVersionClassIn           : 'function',
                minVersionStructIn          : 'function',
                minVersionStructOut         : 'function',
                minVersionEnumIn            : 'function',
                minVersionEnumOut           : 'function',
                callMinVersionDelegate      : 'function',
                minVersionInterfaceVectorIn : 'function',
                // Win8 types
                win8InterfaceIn         : 'function',
                win8ClassIn             : 'function',
                win8StructIn            : 'function',
                win8StructOut           : 'function',
                win8EnumIn              : 'function',
                win8EnumOut             : 'function',
                callWin8Delegate        : 'function',
                win8InterfaceVectorIn   : 'function',
                // Win8SP1 types
                win8SP1InterfaceIn      : 'function',
                win8SP1ClassIn          : 'function',
                win8SP1StructIn         : 'function',
                win8SP1StructOut        : 'function',
                win8SP1EnumIn           : 'function',
                win8SP1EnumOut          : 'function',
                callWin8SP1Delegate     : 'function',
                win8SP1InterfaceVectorIn: 'function',
                // Win9 types
                win9InterfaceIn         : 'function',
                win9ClassIn             : 'function',
                win9StructIn            : 'function',
                win9StructOut           : 'function',
                win9EnumIn              : 'function',
                win9EnumOut             : 'function',
                callWin9Delegate        : 'function',
                win9InterfaceVectorIn   : 'function',
                // Max version types
                maxVersionInterfaceIn       : 'function',
                maxVersionClassIn           : 'function',
                maxVersionStructIn          : 'function',
                maxVersionStructOut         : 'function',
                maxVersionEnumIn            : 'function',
                maxVersionEnumOut           : 'function',
                callMaxVersionDelegate      : 'function',
                maxVersionInterfaceVectorIn : 'function',
            };
            verify.members(instance, membersExpected, instance);
        }
    });

    runner.addTest({
        id: 1,
        desc: 'Marshaling of Versioned Interfaces',
        pri: '0',
        test: function () {
            var instance = new rtc();

            // IMinVersionInterface is within the target version,
            // but there is no way to construct such an object within the target version.
            // Pass undefined for now.
            var interfaceParam;
            verify.noException(function() {
                instance.minVersionInterfaceIn(interfaceParam);
            }, "minVersionInterfaceIn should succeed for this target version");

            // All other *InterfaceIn methods will have invalid signatures, because the 
            // parameter type is outside the target version
            verify.exception(function() {
                instance.win8InterfaceIn(interfaceParam);
            }, TypeError, "win8InterfaceIn has an invalid signature for this target version");

            verify.exception(function() {
                instance.win8SP1InterfaceIn(interfaceParam);
            }, TypeError, "win8SP1InterfaceIn has an invalid signature for this target version");

            verify.exception(function() {
                instance.win9InterfaceIn(interfaceParam);
            }, TypeError, "win9InterfaceIn has an invalid signature for this target version");

            verify.exception(function() {
                instance.maxVersionInterfaceIn(interfaceParam);
            }, TypeError, "maxVersionInterfaceIn has an invalid signature for this target version");
        }
    });

    runner.addTest({
        id: 2,
        desc: 'Marshaling of Versioned RuntimeClasses',
        pri: '0',
        test: function () {
            var instance = new rtc();

            // MinVersionClass is within the target version,
            // but there is no way to construct such an object within the target version.
            // Pass undefined for now.
            var classParam;
            verify.noException(function() {
                instance.minVersionClassIn(classParam);
            }, "minVersionClassIn should succeed for this target version");

            // All other *ClassIn methods will have invalid signatures, because the 
            // parameter type is outside the target version
            verify.exception(function() {
                instance.win8ClassIn(classParam);
            }, TypeError, "win8ClassIn has an invalid signature for this target version");

            verify.exception(function() {
                instance.win8SP1ClassIn(classParam);
            }, TypeError, "win8SP1ClassIn has an invalid signature for this target version");

            verify.exception(function() {
                instance.win9ClassIn(classParam);
            }, TypeError, "win9ClassIn has an invalid signature for this target version");

            verify.exception(function() {
                instance.maxVersionClassIn(classParam);
            }, TypeError, "maxVersionClassIn has an invalid signature for this target version");
        }
    });

    runner.addTest({
        id: 3,
        desc: 'Marshaling of Versioned Structs',
        pri: '0',
        test: function () {
            var instance = new rtc();

            var structParam = { a: 12, b: true };
            var result;
            verify.noException(function() {
                instance.minVersionStructIn(structParam);
            }, "minVersionStructIn should succeed for this target version");
            verify.fields(instance.minVersionStructOut(), structParam, null, "minVersionStructOut()");

            // All other *StructIn/Out methods will have invalid signatures, because the 
            // parameter type is outside the target version
            verify.exception(function() {
                instance.win8StructIn(structParam);
            }, TypeError, "win8StructIn has an invalid signature for this target version");

            verify.exception(function() {
                result = instance.win8StructOut();
            }, TypeError, "win8StructOut has an invalid signature for this target version");

            verify.exception(function() {
                instance.win8SP1StructIn(structParam);
            }, TypeError, "win8SP1StructIn has an invalid signature for this target version");

            verify.exception(function() {
                result = instance.win8SP1StructOut();
            }, TypeError, "win8SP1StructOut has an invalid signature for this target version");

            verify.exception(function() {
                instance.win9StructIn(structParam);
            }, TypeError, "win9StructIn has an invalid signature for this target version");

            verify.exception(function() {
                result = instance.win9StructOut();
            }, TypeError, "win9StructOut has an invalid signature for this target version");

            verify.exception(function() {
                instance.maxVersionStructIn(structParam);
            }, TypeError, "maxVersionStructIn has an invalid signature for this target version");

            verify.exception(function() {
                result = instance.maxVersionStructOut();
            }, TypeError, "maxVersionStructOut has an invalid signature for this target version");
        }
    });

    runner.addTest({
        id: 4,
        desc: 'Marshaling of Versioned Enums',
        pri: '0',
        test: function () {
            var instance = new rtc();

            var result;
            verify.noException(function() {
                instance.minVersionEnumIn(DevTests.Versioning.MinVersionEnum.min);
            }, "minVersionEnumIn should succeed for this target version");
            verify(instance.minVersionEnumOut(), versions.min, "minVersionEnumOut()");

            // All other *EnumIn/Out methods will have invalid signatures, because the 
            // parameter type is outside the target version
            verify.exception(function() {
                instance.win8EnumIn(0);
            }, TypeError, "win8EnumIn has an invalid signature for this target version");

            verify.exception(function() {
                result = instance.win8EnumOut();
            }, TypeError, "win8EnumOut has an invalid signature for this target version");

            verify.exception(function() {
                instance.win8SP1EnumIn(0);
            }, TypeError, "win8SP1EnumIn has an invalid signature for this target version");

            verify.exception(function() {
                result = instance.win8SP1EnumOut();
            }, TypeError, "win8SP1EnumOut has an invalid signature for this target version");

            verify.exception(function() {
                instance.win9EnumIn(0);
            }, TypeError, "win9EnumIn has an invalid signature for this target version");

            verify.exception(function() {
                result = instance.win9EnumOut();
            }, TypeError, "win9EnumOut has an invalid signature for this target version");

            verify.exception(function() {
                instance.maxVersionEnumIn(0);
            }, TypeError, "maxVersionEnumIn has an invalid signature for this target version");

            verify.exception(function() {
                result = instance.maxVersionEnumOut();
            }, TypeError, "maxVersionEnumOut has an invalid signature for this target version");
        }
    });

    runner.addTest({
        id: 5,
        desc: 'Marshaling of Versioned Delegates',
        pri: '0',
        test: function () {
            var instance = new rtc();

            function minDelegate(minInterface) {
                verify.defined(minInterface.minVersion, "minInterface.minVersion");
                verify(minInterface.minVersion, versions.min, "minInterface.minVersion");
                // Verify this is also an instance of some class
                verify.defined(minInterface.activationValue, "minInterface.activationValue");
                verify(minInterface.activationValue, 0, "minInterface.activationValue");
            }
            instance.callMinVersionDelegate(0, minDelegate);

            // The following cause marshaling errors when the delegate is invoked, 
            // since Win8Class and Win8SP1Class are outside the target version.
            // Skipping these cases, because the delegate unhandled exception behavior causes
            // the errors to be reported directly to OnScriptError
            /*
            instance.callMinVersionDelegate(1, minDelegate);
            instance.callMinVersionDelegate(2, minDelegate);
            */

            // All other Call*Delegate methods will have invalid signatures, because the 
            // parameter type is outside the target version
            function win8Delegate(win8sp1Struct) {}
            
            verify.exception(function() {
                instance.callWin8Delegate(win8Delegate);
            }, TypeError, "callWin8Delegate has an invalid signature for this target version");

            function win8sp1Delegate(maxEnum) {}

            verify.exception(function() {
                instance.callWin8SP1Delegate(win8sp1Delegate);
            }, TypeError, "callWin8SP1Delegate has an invalid signature for this target version");

            function win9Delegate(win9Class) {}

            verify.exception(function() {
                instance.callWin9Delegate(win9Delegate);
            }, TypeError, "callWin9Delegate has an invalid signature for this target version");

            function maxDelegate(win8Interface) {}

            verify.exception(function() {
                instance.callMaxVersionDelegate(maxDelegate);
            }, TypeError, "callMaxVersionDelegate has an invalid signature for this target version");
        }
    });

    runner.addTest({
        id: 6,
        desc: 'Marshaling of IVector with Versioned T',
        pri: '0',
        test: function () {
            var instance = new rtc();

            // IMinVersionInterface is within the target version,
            // but there is no way to construct such an object within the target version.
            // Pass undefined for now.
            var interfaceVector = [];
            // All *InterfaceVectorIn methods will have invalid signatures, because the 
            // parameter type is outside the target version
            verify.exception(function() {
                instance.minVersionInterfaceVectorIn(interfaceVector);
            }, TypeError, "minVersionInterfaceVectorIn has an invalid signature for this target version");

            verify.exception(function() {
                instance.win8InterfaceVectorIn(interfaceVector);
            }, TypeError, "win8InterfaceVectorIn has an invalid signature for this target version");

            verify.exception(function() {
                instance.win8SP1InterfaceVectorIn(interfaceVector);
            }, TypeError, "win8SP1InterfaceVectorIn has an invalid signature for this target version");

            verify.exception(function() {
                instance.win9InterfaceVectorIn(interfaceVector);
            }, TypeError, "win9InterfaceVectorIn has an invalid signature for this target version");

            verify.exception(function() {
                instance.maxVersionInterfaceVectorIn(interfaceVector);
            }, TypeError, "maxVersionInterfaceVectorIn has an invalid signature for this target version");
        }
    });

    Loader42_FileName = 'Marshaling Versioned Types Test - Version 0';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
