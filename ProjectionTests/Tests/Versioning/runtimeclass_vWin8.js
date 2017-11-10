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

    runner.addTest({
        id: 0,
        desc: 'DevTests.Versioning Members',
        pri: '0',
        test: function () {
            var ns = DevTests.Versioning;
            var nsName = "DevTests.Versioning";

            // Verify namespace exists
            verify.defined(ns, nsName);
            var membersExpected = {
                // Version 0 types
                VersionedEnumFields     : "object",
                MinVersionEnum          : "object",
                MinVersionClass         : "function",
                MarshalVersionedTypes   : "function",
                VectorInt               : "function",
                ObservableVectorInt     : "function",
                RequiresVectorInt           : "function",
                RequiresObservableVectorInt : "function",
                // Win8 types
                Win8Enum                : "object",
                Win8Class               : "function",
                VersionedVectorInt          : "function",
                VersionedObservableVectorInt: "function",
                VectorVersionedT            : "function",
                ObservableVectorVersionedT  : "function",
                /* -- Excluded from this version --
                // Win8SP1 types
                Win8SP1Enum             : "object",
                Win8SP1Class            : "function",
                // Win9 types
                Win9Enum                : "object",
                Win9Class               : "function",
                // Max version types
                MaxVersionEnum          : "object",
                MaxVersionClass         : "function",
                */
            };
            verify.members(ns, membersExpected, nsName);
            
            var fieldsExpected = {
                min     : 0,
                win8    : 1,
            };

            var fieldsUnexpected = {
                win8SP1 : 'undefined',
                win9    : 'undefined',
                max     : 'undefined'
            };

            verify.fields(ns.VersionedEnumFields, fieldsExpected, fieldsUnexpected, nsName + ".VersionedEnumFields");

            fieldsExpected = {
                min     : 0
            };
            verify.fields(ns.MinVersionEnum, fieldsExpected, null, nsName + ".MinVersionEnum");

            fieldsExpected = {
                win8     : versions.win8
            };
            verify.fields(ns.Win8Enum, fieldsExpected, null, nsName + ".Win8Enum");
        }
    });

    runner.addTest({
        id: 1,
        desc: 'Versioning behavior of MinVersionClass',
        pri: '0',
        test: function () {
            var rtc = DevTests.Versioning.MinVersionClass;
            var rtcName = "DevTests.Versioning.MinVersionClass";

            // Verify class exists
            verify.defined(rtc, rtcName);

            // Verify static members
            var staticExpected = {};
            verify.members(rtc, staticExpected, rtcName);
            
            // Verify Activation
            var instance;
            verify.exception(function () {
                instance = new rtc(1);
            }, Error, "Type is not simple or factory activatable");

            verify.notDefined(instance, rtcName + " instance");
        }
    });

    runner.addTest({
        id: 2,
        desc: 'Versioning behavior of Win8Class',
        pri: '0',
        test: function () {
            var rtc = DevTests.Versioning.Win8Class;
            var rtcName = "DevTests.Versioning.Win8Class";

            // Verify class exists
            verify.defined(rtc, rtcName);

            // Verify static members
            var staticExpected = {
                win8Version : 'number'
            };
            verify.members(rtc, staticExpected, rtcName);

            verify(rtc.win8Version, versions.win8, rtcName + ".win8Version");
            
            // Verify Activation
            var instance;
            verify.exception(function () {
                instance = new rtc();
            }, Error, "Type is not simple activatable");
            
            instance = new rtc(42);
            verify.defined(instance, rtcName + " instance");
            verify(instance.activationValue, 42, "Instance was created with a factory constructor");

            // Verify instance
            var instanceExpected = {
                activationValue : 'number',
                minVersion  : 'number',
            };
            verify.members(instance, instanceExpected, instance.toString());

            verify(instance.minVersion, versions.min, "instance.minVersion");
        }
    });

    runner.addTest({
        id: 3,
        desc: 'Versioning behavior of Win8SP1Class',
        pri: '0',
        test: function () {
            var rtc = DevTests.Versioning.Win8SP1Class;
            var rtcName = "DevTests.Versioning.Win8SP1Class";

            // Verify class does not exist
            verify.notDefined(rtc, rtcName);
        }
    });

    runner.addTest({
        id: 4,
        desc: 'Versioning behavior of Win9Class',
        pri: '0',
        test: function () {
            var rtc = DevTests.Versioning.Win9Class;
            var rtcName = "DevTests.Versioning.Win9Class";

            // Verify class does not exist
            verify.notDefined(rtc, rtcName);
        }
    });

    runner.addTest({
        id: 5,
        desc: 'Versioning behavior of MaxVersionClass',
        pri: '0',
        test: function () {
            var rtc = DevTests.Versioning.MaxVersionClass;
            var rtcName = "DevTests.Versioning.MaxVersionClass";

            // Verify class does not exist
            verify.notDefined(rtc, rtcName);
        }
    });

        runner.addTest({
        id: 6,
        desc: 'Versioning behavior of VectorInt',
        pri: '0',
        test: function () {
            var rtc = DevTests.Versioning.VectorInt;
            var rtcName = "DevTests.Versioning.VectorInt";

            // Verify class exists
            verify.defined(rtc, rtcName);

            // Verify Activation
            var instance = rtc();
            verify.defined(instance, rtcName + " instance");

            var instanceExpected = {
                hasDefault  : 'function',
                append      : 'function',
                clear       : 'function',
                first       : 'function',
                getAt       : 'function',
                getMany     : 'function',
                getView     : 'function',
                indexOf     : 'function',
                insertAt    : 'function',
                removeAt    : 'function',
                removeAtEnd : 'function',
                replaceAll  : 'function',
                setAt       : 'function',
                size        : 'number',
                '0'         : 'number',
                '1'         : 'number',
                '2'         : 'number',
                '3'         : 'number',
                '4'         : 'number',
                '5'         : 'number',
                '6'         : 'number',
                '7'         : 'number',
                '8'         : 'number',
            }

            verify.members(instance, instanceExpected, instance.toString());

            assert(instance.hasDefault(), "instance.hasDefault()");
            verify(instance.size, 9, "instance.size");
            verify(instance.getAt(6), 7, "instance.getAt(6)");
        }
    });

    runner.addTest({
        id: 7,
        desc: 'Versioning behavior of ObservableVectorInt',
        pri: '0',
        test: function () {
            var rtc = DevTests.Versioning.ObservableVectorInt;
            var rtcName = "DevTests.Versioning.ObservableVectorInt";

            // Verify class exists
            verify.defined(rtc, rtcName);

            // Verify Activation
            var instance = rtc();
            verify.defined(instance, rtcName + " instance");

            var instanceExpected = {
                hasDefault  : 'function',
                append      : 'function',
                clear       : 'function',
                first       : 'function',
                getAt       : 'function',
                getMany     : 'function',
                getView     : 'function',
                indexOf     : 'function',
                insertAt    : 'function',
                removeAt    : 'function',
                removeAtEnd : 'function',
                replaceAll  : 'function',
                setAt       : 'function',
                size        : 'number',
                addEventListener    : 'function',
                removeEventListener : 'function',
                onvectorchanged     : 'object',
                '0'         : 'number',
                '1'         : 'number',
                '2'         : 'number',
                '3'         : 'number',
                '4'         : 'number',
                '5'         : 'number',
                '6'         : 'number',
                '7'         : 'number',
                '8'         : 'number',
            }

            verify.members(instance, instanceExpected, instance.toString());

            assert(instance.hasDefault(), "instance.hasDefault()");
            verify(instance.size, 9, "instance.size");
            verify(instance.getAt(6), 7, "instance.getAt(6)");
            verify.noException(function (){
                instance.onvectorchanged = function (ev) { };
            }, "Can access IObservableVector members");
        }
    });

    runner.addTest({
        id: 8,
        desc: 'Versioning behavior of RequiresVectorInt',
        pri: '0',
        test: function () {
            var rtc = DevTests.Versioning.RequiresVectorInt;
            var rtcName = "DevTests.Versioning.RequiresVectorInt";

            // Verify class exists
            verify.defined(rtc, rtcName);

            // Verify Activation
            var instance = rtc();
            verify.defined(instance, rtcName + " instance");

            var instanceExpected = {
                hasIRequiresVector  : 'function',
                append      : 'function',
                clear       : 'function',
                first       : 'function',
                getAt       : 'function',
                getMany     : 'function',
                getView     : 'function',
                indexOf     : 'function',
                insertAt    : 'function',
                removeAt    : 'function',
                removeAtEnd : 'function',
                replaceAll  : 'function',
                setAt       : 'function',
                size        : 'number',
                '0'         : 'number',
                '1'         : 'number',
                '2'         : 'number',
                '3'         : 'number',
                '4'         : 'number',
                '5'         : 'number',
                '6'         : 'number',
                '7'         : 'number',
                '8'         : 'number',
            }

            verify.members(instance, instanceExpected, instance.toString());

            assert(instance.hasIRequiresVector(), "instance.hasIRequiresVector()");
            verify(instance.size, 9, "instance.size");
            verify(instance.getAt(6), 7, "instance.getAt(6)");
        }
    });

    runner.addTest({
        id: 9,
        desc: 'Versioning behavior of RequiresObservableVectorInt',
        pri: '0',
        test: function () {
            var rtc = DevTests.Versioning.RequiresObservableVectorInt;
            var rtcName = "DevTests.Versioning.RequiresObservableVectorInt";

            // Verify class exists
            verify.defined(rtc, rtcName);

            // Verify Activation
            var instance = rtc();
            verify.defined(instance, rtcName + " instance");

            var instanceExpected = {
                hasIRequiresObservableVector  : 'function',
                append      : 'function',
                clear       : 'function',
                first       : 'function',
                getAt       : 'function',
                getMany     : 'function',
                getView     : 'function',
                indexOf     : 'function',
                insertAt    : 'function',
                removeAt    : 'function',
                removeAtEnd : 'function',
                replaceAll  : 'function',
                setAt       : 'function',
                size        : 'number',
                addEventListener    : 'function',
                removeEventListener : 'function',
                onvectorchanged     : 'object',
                '0'         : 'number',
                '1'         : 'number',
                '2'         : 'number',
                '3'         : 'number',
                '4'         : 'number',
                '5'         : 'number',
                '6'         : 'number',
                '7'         : 'number',
                '8'         : 'number',
            }

            verify.members(instance, instanceExpected, instance.toString());

            assert(instance.hasIRequiresObservableVector(), "instance.hasIRequiresObservableVector()");
            verify(instance.size, 9, "instance.size");
            verify(instance.getAt(6), 7, "instance.getAt(6)");
            verify.noException(function (){
                instance.onvectorchanged = function (ev) { };
            }, "Can access IObservableVector members");
        }
    });

    runner.addTest({
        id: 10,
        desc: 'Versioning behavior of VersionedVectorInt',
        pri: '0',
        test: function () {
            var rtc = DevTests.Versioning.VersionedVectorInt;
            var rtcName = "DevTests.Versioning.VersionedVectorInt";

            // Verify class exists
            verify.defined(rtc, rtcName);

            // Verify Activation
            var instance = rtc();
            verify.defined(instance, rtcName + " instance");

            var instanceExpected = {
                hasDefault  : 'function'
            }

            verify.members(instance, instanceExpected, instance.toString());

            assert(instance.hasDefault(), "instance.hasDefault()");
        }
    });

    runner.addTest({
        id: 11,
        desc: 'Versioning behavior of VersionedObservableVectorInt',
        pri: '0',
        test: function () {
            var rtc = DevTests.Versioning.VersionedObservableVectorInt;
            var rtcName = "DevTests.Versioning.VersionedObservableVectorInt";

            // Verify class exists
            verify.defined(rtc, rtcName);

            // Verify Activation
            var instance = rtc();
            verify.defined(instance, rtcName + " instance");

            var instanceExpected = {
                hasDefault  : 'function'
            }

            verify.members(instance, instanceExpected, instance.toString());

            assert(instance.hasDefault(), "instance.hasDefault()");
        }
    });

    runner.addTest({
        id: 12,
        desc: 'Versioning behavior of VectorVersionedT',
        pri: '0',
        test: function () {
            var rtc = DevTests.Versioning.VectorVersionedT;
            var rtcName = "DevTests.Versioning.VectorVersionedT";

            // Verify class exists
            verify.defined(rtc, rtcName);

            // Verify Activation
            var instance = rtc();
            verify.defined(instance, rtcName + " instance");

            var instanceExpected = {
                hasDefault  : 'function'
            }

            verify.members(instance, instanceExpected, instance.toString());

            assert(instance.hasDefault(), "instance.hasDefault()");
        }
    });

    runner.addTest({
        id: 13,
        desc: 'Versioning behavior of ObservableVectorVersionedT',
        pri: '0',
        test: function () {
            var rtc = DevTests.Versioning.ObservableVectorVersionedT;
            var rtcName = "DevTests.Versioning.ObservableVectorVersionedT";

            // Verify class exists
            verify.defined(rtc, rtcName);

            // Verify Activation
            var instance = rtc();
            verify.defined(instance, rtcName + " instance");

            var instanceExpected = {
                hasDefault  : 'function'
            }

            verify.members(instance, instanceExpected, instance.toString());

            assert(instance.hasDefault(), "instance.hasDefault()");
        }
    });


    Loader42_FileName = 'RuntimeClass Versioning Tests - Version NTDDI_WIN8';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
