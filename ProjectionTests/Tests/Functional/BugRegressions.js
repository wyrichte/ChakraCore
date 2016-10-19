if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    runner.addTest({
        id: 164485,
        desc: 'Inheritted generic interface crashes (see Windows.Data.Json.JsonArray)',
        pri: '0',
        test: function () {
            Windows.Data.Json.JsonArray
        }
    });

    runner.addTest({
        id: 164448,
        desc: 'Windows.Foundation.Collections["MapChangedEventHandler`2"]',
        pri: '0',
        test: function () {
            Windows.Foundation.Collections["MapChangedEventHandler`2"]
        }
    });

    runner.addTest({
        id: 164458,
        desc: 'Windows.Foundation.Collections.PropertySet',
        pri: '0',
        test: function () {
            Windows.Foundation.Collections.PropertySet
        }
    });

    runner.addTest({
        id: 164470,
        desc: 'Windows.System.ApplicationLifetime',
        pri: '0',
        test: function () {
            Windows.System.ApplicationLifetime
        }
    });

    runner.addTest({
        id: 164474,
        desc: 'Windows.Networking.NetworkOperators.ProvisioningAgent',
        pri: '0',
        test: function () {
            Windows.Networking.NetworkOperators.ProvisioningAgent
        }
    });

    runner.addTest({
        id: 164434,
        desc: 'Windows.Devices.Sensors.AccelerometerReadingChangedEventHandler',
        pri: '0',
        test: function () {
            verify(Windows.Devices.Sensors.AccelerometerReadingChangedEventHandler, undefined, "Windows.Devices.Sensors.AccelerometerReadingChangedEventHandler");
        }
    });

    runner.addTest({
        id: 177062,
        desc: 'Windows.Media.MediaProperties.AudioEncodingProperties',
        pri: '0',
        test: function () {
            logger.comment("Windows.Media.MediaProperties.AudioEncodingProperties: " + Windows.Media.MediaProperties.AudioEncodingProperties);
            verify(typeof Windows.Media.MediaProperties.AudioEncodingProperties, "function", "typeof Windows.Media.MediaProperties.AudioEncodingProperties");
        }
    });

    runner.addTest({
        id: 357538,
        desc: 'Windows.UI.Xaml.ResourceDictionary should be hidden as the runtime class is hidden',
        pri: '0',
        test: function () {
            verify(Windows.UI.Xaml.ResourceDictionary, undefined, "Windows.UI.Xaml.ResourceDictionary");
            verify(Windows.ApplicationModel.Core.DeviceActivationEvents, undefined, "Windows.ApplicationModel.Core.DeviceActivationEvents");
        }
    });

    runner.addTest({
        id: 167042,
        desc: 'Initialize [out] pointers to nullptr',
        pri: '0',
        test: function () {
            var turkey = new Animals.Turkey();
            verify(turkey.getNumFeathers(), 100, "turkey.getNumFeathers()");

            verify(Animals.Animal.getCLSID(), "eb561c4d-2526-4a9e-94d3-4743a5eb658b", "Animals.Animal.getCLSID()");

            var myAnimal = new Animals.Animal(1, 1, 2);
            verify(myAnimal.getNumLegs(), 4, "myAnimal.getNumLegs()");
            var dimensions = myAnimal.getDimensions();
            verify(dimensions.length, 180, "dimensions.length");
            verify(dimensions.width, 360, "dimensions.width");

            var mother = new Animals.Animal(100);
            var child = new Animals.Animal(mother, 4);
            verify(child.mother.getNumLegs(), 100, "child.mother.getNumLegs()");
        }
    });

    runner.addTest({
        id: 183112,
        desc: 'Windows.Foundation.Collections.PropertySet',
        pri: '0',
        test: function () {
            var propSet = new Windows.Foundation.Collections.PropertySet();

            try {
                var n = propSet.size;
                verify(n, 0, "propSet.size");
            } catch (e) {
                verify(e.number, -2146827858, "Error Number");
                fail("This error occurs due to bug 182500. This test should begin passing once the fix makes it to our branch.");
            }
        }
    });

    runner.addTest({
        id: 186241,
        desc: 'Windows.Foundation.Uri.length',
        pri: '0',
        test: function () {
            verify(Windows.Foundation.Uri.length, 1, 'Windows.Foundation.Uri.length');
        }
    });

    runner.addTest({
        id: 'buddy-test-1',
        desc: 'Single byte struct',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);
            myAnimal.testPackedByte12({ field0: 188 });
        }
    });

    runner.addTest({
        id: 'buddy-test-2',
        desc: 'Struct of bools',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);
            myAnimal.testPackedBoolean1({ field0: true, field1: false, field2: true, field3: false });
        }
    });

    runner.addTest({
        id: 'buddy-test-3',
        desc: 'Send and get vector of structs',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);
            myAnimal.sendAndGetIVectorStructs([{ a: 100}]);
        }
    });

    runner.addTest({
        id: '222538 1',
        desc: 'Marshal HRESULT',
        pri: '0',
        preReq: function () {
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            var animal = new Animals.Animal(1);
            var hr = animal.marshalHRESULT(52);
            verify(hr, 52, 'Marshal [out] HRESULT');
        }
    });

    runner.addTest({
        id: '222538 2',
        desc: 'Marshal HRESULT as Property ',
        pri: '0',
        preReq: function () {
            return (typeof Animals._CLROnly === 'undefined');
        },
        test: function () {
            var animal = new Animals.Animal(1);
            var hr = animal.errorCode;
            verify(hr, 192, 'Marshal HRESULT as Property');
        }
    });

    runner.addTest({
        id: '220655',
        desc: 'Windows.Devices.Proximity.SessionAdvertisement',
        pri: '0',
        test: function () {
            var foundException = false;
            try {
                var obj = new Windows.Devices.Proximity.SessionAdvertisement();
            } catch (e) {
                foundException = true;
                verify(e.toString(), "TypeError: Unable to get property 'SessionAdvertisement' of undefined or null reference", "Exception while calling Windows.Devices.Proximity.SessionAdvertisement()");
            }
            assert(foundException, "Exception Caught");
        }
    });

    runner.addTest({
        id: 'int64-roundtrip-1',
        desc: 'An int64 can roundtrip from WinRT->JS->WinRT without modification (not converted to double)',
        pri: '0',
        test: function () {
            var dateTimeTests = new DevTests.DateTimeAndTimeSpan.Tests();

            var int64max = dateTimeTests.getInt64Max();
            var int64maxDouble = 0x7FFFFFFFFFFFFFFF;
            verify.notEqual(dateTimeTests.int64Cmp(int64max, int64maxDouble), 0, "Projected max int64 not equal to double approximation: dateTimeTests.int64Cmp(int64max, int64maxDouble)");
            verify(dateTimeTests.verifyInt64Max(int64max), true, "dateTimeTests.verifyInt64Max(int64max)");
            verify((int64max = int64max + 0), int64max, "Mathematical operation with int64 value: (int64max = int64max + 0)");
            verify(dateTimeTests.verifyInt64Max(int64max), false, "dateTimeTests.verifyInt64Max(int64max)");
            verify(dateTimeTests.int64Cmp(int64max, int64maxDouble), 0, "Projected max int64 now equal to double approximation: dateTimeTests.int64Cmp(int64max, int64maxDouble)");
        }
    });

    runner.addTest({
        id: '231314',
        desc: 'QueryObjectInterface on IJavascriptOperations',
        pri: '0',
        preReq: function () {
            return (typeof TestUtilities !== 'undefined');
        },
        test: function () {
            var simpleClassOriginal = new DevTests.SimpleTestNamespace.SimpleClass();
            verify(simpleClassOriginal.getMessage(), "Hello", 'simpleClassOriginal.getMessage()');
            verify(simpleClassOriginal.value, 50, 'simpleClassOriginal.value');

            logger.comment("QueryObjectInterface and modify underlying native class");
            var simpleClassReceived = TestUtilities.UpdateSimpleClassAndReturnAsVar(simpleClassOriginal);

            verify(simpleClassOriginal.getMessage(), "Goodbye", 'simpleClassOriginal.getMessage()');
            verify(simpleClassOriginal.value, 217, 'simpleClassOriginal.value');

            verify.strictEqual(simpleClassReceived, simpleClassOriginal, "simpleClassReceived === simpleClassOriginal");
        }
    });

    runner.addTest({
        id: 357908,
        desc: 'Windows.Foundation.HResult',
        pri: '0',
        test: function () {
            var exceptionCaught = false;
            try {
                new Windows.Foundation.HResult();
            } catch (e) {
                exceptionCaught = true;
                verify(e.toString(), "TypeError: Object doesn't support this action", "Exception caught trying to create a Windows.Foundation.HReslt");
            }
            verify(exceptionCaught, true, "Exception caught");
        }
    });

    runner.addTest({
        id: 376072,
        desc: 'Error strings with arguments',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal();
            // STG_E_FILEALREADYEXISTS (0x80030050)
            logger.comment("Test marshaling of HRESULT: STG_E_FILEALREADYEXISTS");
            try {
                myAnimal.testError(0x80030050);
            }
            catch (e) {
                verify.instanceOf(e, WinRTError);
                verify(e.number >>> 0, 0x80030050, 'Error number');
                verify(e.name, 'WinRTError', 'Error name');
                verify.notEqual(e.message, "Unknown runtime error", "Error message");
                if (typeof TestUtilities !== 'undefined') {
                    verify(e.message, TestUtilities.GetSystemStringFromHr(0x80030050), 'Error message');
                }
            }
        }
    });

    runner.addTest({
        id: 231298,
        desc: 'VarToDispEx should fail for WinRT objects',
        pri: '0',
        preReq: function () {
            return (typeof TestUtilities !== 'undefined');
        },
        test: function () {
            verify(TestUtilities.VarToDispExTest({}), "Call succeeded", "TestUtilities.VarToDispExTest({})");
            verify(TestUtilities.VarToDispExTest(new Animals.Animal()), "Call failed with hr of E_INVALIDARG", "TestUtilities.VarToDispExTest(new Animals.Animal())");
        }
    });

    runner.addTest({
        id: 397120,
        desc: 'Ensure that we can create a XmlDocument and call loadXml',
        pri: '0',
        test: function () {
            var doc = new Windows.Data.Xml.Dom.XmlDocument;
            doc.loadXml("<root/>");
        }
    });


    runner.addTest({
        id: '405211-repro1',
        desc: 'Ensure that we can create a XmlDocument and call createCDataSection',
        pri: '0',
        test: function () {
            var doc = new Windows.Data.Xml.Dom.XmlDocument;
            doc.createCDataSection("Hello");
        }
    });

    runner.addTest({
        id: '405211-repro2',
        desc: 'Ensure that we can create a XmlDocument, call loadXml and use doc.documentElement property\'s methods',
        pri: '0',
        test: function () {
            var doc = new Windows.Data.Xml.Dom.XmlDocument;
            doc.loadXml("<root id='123'>content</root>");
            var attr = doc.documentElement.attributes.item(0);
        }
    });

    runner.addTest({
        id: '426277',
        desc: 'instance of',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(4);
            verify(myAnimal instanceof Object, true, "myAnimal instanceof Object");
            verify(myAnimal instanceof Animals.Animal, true, "myAnimal instanceof Animals.Animal");

            verify(1 instanceof Animals.Animal, false, "1 instanceof Animals.Animal");

            verify.exception(function () {
                var result = 1 instanceof myAnimal;
            }, TypeError, "1 instanceof myAnimal");

            verify.exception(function () {
                var result = myAnimal instanceof myAnimal;
            }, TypeError, "myAnimal instanceof myAnimal");
        }
    });

    runner.addTest({
        id: '639242',
        desc: 'Interface [out] fast path when runtimeclass name is not found in metadata',
        pri: '0',
        test: function () {
            var interface = DevTests.Repros.InterfaceOutFastPath.Tests.interfaceOutTest();
            verify.defined(interface, "result of interfaceOutTest");
            assert(interface.testMethod(), "interface.testMethod()");
        }
    });

    runner.addTest({
        id: '897014',
        desc: 'Object identity when calling InspectableUnknownToVar with non-default interface pointer',
        pri: '0',
        preReq: function () {
            return (typeof TestUtilities !== 'undefined');
        },
        test: function () {
            var simpleClassOriginal = new DevTests.SimpleTestNamespace.SimpleClass();
            verify(simpleClassOriginal.getMessage(), "Hello", 'simpleClassOriginal.getMessage()');
            verify(simpleClassOriginal.value, 50, 'simpleClassOriginal.value');

            logger.comment("QueryObjectInterface and modify underlying native class");
            var simpleClassReceived = TestUtilities.UpdateSimpleClassAndReturnAsVarByAlternateInterface(simpleClassOriginal);

            verify(simpleClassOriginal.getMessage(), "Goodbye", 'simpleClassOriginal.getMessage()');
            verify(simpleClassOriginal.value, 217, 'simpleClassOriginal.value');

            verify.strictEqual(simpleClassReceived, simpleClassOriginal, "simpleClassReceived === simpleClassOriginal");
        }
    });

    Loader42_FileName = "Bug Regressions";
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
