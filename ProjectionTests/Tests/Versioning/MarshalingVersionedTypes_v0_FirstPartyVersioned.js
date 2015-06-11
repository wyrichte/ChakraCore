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

            verify.noException(function() {
                instance.win8InterfaceIn(interfaceParam);
            }, "win8InterfaceIn should succeed for this target version");

            verify.noException(function() {
                instance.win8SP1InterfaceIn(interfaceParam);
            }, "win8SP1InterfaceIn should succeed for this target version");

            verify.noException(function() {
                instance.win9InterfaceIn(interfaceParam);
            }, "win9InterfaceIn should succeed for this target version");

            verify.noException(function() {
                instance.maxVersionInterfaceIn(interfaceParam);
            }, "maxVersionInterfaceIn should succeed for this target version");
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

            verify.noException(function() {
                instance.win8ClassIn(classParam);
            }, "win8ClassIn should succeed for this target version");

            verify.noException(function() {
                instance.win8SP1ClassIn(classParam);
            }, "win8SP1ClassIn should succeed for this target version");

            verify.noException(function() {
                instance.win9ClassIn(classParam);
            }, "win9ClassIn should succeed for this target version");

            verify.noException(function() {
                instance.maxVersionClassIn(classParam);
            }, "maxVersionClassIn should succeed for this target version");
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

            verify.noException(function() {
                instance.win8StructIn(structParam);
            }, "win8StructIn should succeed for this target version");
            verify.fields(instance.win8StructOut(), structParam, null, "win8StructOut()");

            verify.noException(function() {
                instance.win8SP1StructIn(structParam);
            }, "win8SP1StructIn should succeed for this target version");
            verify.fields(instance.win8SP1StructOut(), structParam, null, "win8SP1StructOut()");

            verify.noException(function() {
                instance.win9StructIn(structParam);
            }, "win9StructIn should succeed for this target version");
            verify.fields(instance.win9StructOut(), structParam, null, "win9StructOut()");

            verify.noException(function() {
                instance.maxVersionStructIn(structParam);
            }, "maxVersionStructIn should succeed for this target version");
            verify.fields(instance.maxVersionStructOut(), structParam, null, "maxVersionStructOut()");
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

            verify.noException(function() {
                instance.win8EnumIn(DevTests.Versioning.Win8Enum.win8);
            }, "win8EnumIn should succeed for this target version");
            verify(instance.win8EnumOut(), versions.win8, "win8EnumOut()");

            verify.noException(function() {
                instance.win8SP1EnumIn(DevTests.Versioning.Win8SP1Enum.win8SP1);
            }, "win8SP1EnumIn should succeed for this target version");
            verify(instance.win8SP1EnumOut(), versions.win8sp1, "win8SP1EnumOut()");

            verify.noException(function() {
                instance.win9EnumIn(DevTests.Versioning.Win9Enum.win9);
            }, "win9EnumIn should succeed for this target version");
            verify(instance.win9EnumOut(), versions.win9, "win9EnumOut()");

            verify.noException(function() {
                instance.maxVersionEnumIn(DevTests.Versioning.MaxVersionEnum.max);
            }, "maxVersionEnumIn should succeed for this target version");
            verify(instance.maxVersionEnumOut(), DevTests.Versioning.MaxVersionEnum.max, "maxVersionEnumOut()");
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
            instance.callMinVersionDelegate(1, minDelegate);
            instance.callMinVersionDelegate(2, minDelegate);

            var expectedStruct = { a: -73, b: false };
            function win8Delegate(win8sp1Struct) {
                verify.fields(win8sp1Struct, expectedStruct, null, "Delegate Win8SP1Struct param");
            }
            
            instance.win8SP1StructIn(expectedStruct);
            instance.callWin8Delegate(win8Delegate);

            // The following cause marshaling errors when the delegate is invoked, 
            // since MaxVersionEnum is outside the target version
            function win8sp1Delegate(maxEnum) {
                verify(maxEnum, versions.max, "Delegate MaxVersionEnum param");
            }

            instance.callWin8SP1Delegate(win8sp1Delegate);

            function win9Delegate(win9Class) {
                verify.defined(win9Class.win8Version, "win9Class.win8Version");
                verify(win9Class.win8Version, versions.win8, "win9Class.win8Version");
                verify.defined(win9Class.win9Version, "win9Class.win9Version");
                verify(win9Class.win9Version, versions.win9, "win9Class.win9Version");
                // Verify this is also an instance of some class
                verify.defined(win9Class.activationValue, "win9Class.activationValue");
                verify(win9Class.activationValue, 0, "win9Class.activationValue");
            }

            instance.callWin9Delegate(win9Delegate);

            function maxDelegate(win8Interface) {
                verify.defined(win8Interface.win8Version, "win8Interface.win8Version");
                verify(win8Interface.win8Version, versions.win8, "win8Interface.win8Version");
                verify.defined(win8Interface.win9Version, "win8Interface.win9Version");
                verify(win8Interface.win9Version, versions.win9, "win8Interface.win9Version");
                // Verify this is also an instance of some class
                verify.defined(win8Interface.activationValue, "win8Interface.activationValue");
                verify(win8Interface.activationValue, 0, "win8Interface.activationValue");
            }

            instance.callMaxVersionDelegate(maxDelegate);
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

    Loader42_FileName = 'Marshaling Versioned Types Test - Version 0 (Only 1st party components)';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
