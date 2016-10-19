if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    var root = this;
    var msg = "123";
    function createToaster() {
        var toaster = new Fabrikam.Kitchen.Toaster();
        return toaster;
    }

    var findData = true;
    var eventCallsCount = 0;

    function toastCompleteCallback(ev) {
        verify(ev.message, msg, 'ev.message');
        var weakProperty = msGetWeakWinRTProperty(ev.target, "weakProperty");
        if (findData) {
            assert(weakProperty !== null, 'msGetWeakWinRTProperty(ev.target, "weakProperty") !== null');
            assert(weakProperty !== undefined, 'msGetWeakWinRTProperty(ev.target, "weakProperty") !== undefined');
            verify(weakProperty._data, "Data", 'msGetWeakWinRTProperty(ev.target, "weakProperty")._data');
        }
        else {
            verify(weakProperty, null, 'msGetWeakWinRTProperty(ev.target, "weakProperty")');
        }
        eventCallsCount++;
    }

    function setPropertyOnToaster(toaster) {
        var newObject = {
            _toaster: toaster,
            _data: "Data"
        };

        toaster.addEventListener("toastcompleteevent", toastCompleteCallback);
        msSetWeakWinRTProperty(toaster, "weakProperty", newObject);
        var toast = toaster.makeToast(msg);
        verify(eventCallsCount, 1, 'eventCallsCount');
        newObject = null;
        findData = false;
    }

    runner.addTest({
        id: 1,
        desc: 'Verify msGetWeakWinRTProperty function exists on the global object',
        pri: '0',
        test: function () {
            verify.defined(msGetWeakWinRTProperty, "msGetWeakWinRTProperty");
            verify.typeOf(msGetWeakWinRTProperty, 'function');

            var desc = Object.getOwnPropertyDescriptor(root, "msGetWeakWinRTProperty");
            var attributesExpected = {
                writable: true,
                enumerable: false,
                configurable: true
            };
            logger.comment("Verify attributes of msGetWeakWinRTProperty");
            for (var attrib in attributesExpected) {
                verify(desc[attrib], attributesExpected[attrib], attrib);
            }
            verify(msGetWeakWinRTProperty.length, 2, "msGetWeakWinRTProperty.length");
        }
    });

    runner.addTest({
        id: 2,
        desc: 'Verify msSetWeakWinRTProperty function exists on the global object',
        pri: '0',
        test: function () {
            verify.defined(msSetWeakWinRTProperty, "msSetWeakWinRTProperty");
            verify.typeOf(msSetWeakWinRTProperty, 'function');

            var desc = Object.getOwnPropertyDescriptor(root, "msSetWeakWinRTProperty");
            var attributesExpected = {
                writable: true,
                enumerable: false,
                configurable: true
            };
            logger.comment("Verify attributes of msSetWeakWinRTProperty");
            for (var attrib in attributesExpected) {
                verify(desc[attrib], attributesExpected[attrib], attrib);
            }
            verify(msSetWeakWinRTProperty.length, 3, "msSetWeakWinRTProperty.length");
        }
    });

    runner.addTest({
        id: 3,
        desc: 'Verify that the msGetWeakWinRTProperty fails if not set already',
        pri: '0',
        test: function () {
            var toaster = createToaster();
            var toastCompleteCount = 0;

            function toastCompleteCallback(ev) {
                verify(ev.message, msg, 'ev.message');
                verify(msGetWeakWinRTProperty(ev.target, "weakProperty"), null, 'msGetWeakWinRTProperty(ev.target, "weakProperty")');
                toastCompleteCount++;
            }

            toaster.addEventListener("toastcompleteevent", toastCompleteCallback);
            var requiredToastCount = 3;
            for (var i = 0; i < requiredToastCount; i++) {
                var toast = toaster.makeToast(msg);
            }
            verify(toastCompleteCount, requiredToastCount, 'toastCompleteCount');
        }
    });

    runner.addTest({
        id: 4,
        desc: 'Verify that the msGetWeakWinRTProperty succceeds after setting the property',
        pri: '0',
        test: function () {
            var toaster = createToaster();
            var toastCompleteCount = 0;

            var expectedToast = toaster.makeToast(msg);
            msSetWeakWinRTProperty(toaster, "expectedToast", expectedToast);

            function toastCompleteCallback(ev) {
                verify(ev.message, msg, 'ev.message');
                verify(msGetWeakWinRTProperty(ev.target, "expectedToast"), expectedToast, 'msGetWeakWinRTProperty(ev.target, "expectedToast")');
                toastCompleteCount++;
            }

            toaster.addEventListener("toastcompleteevent", toastCompleteCallback);
            var requiredToastCount = 3;
            for (var i = 0; i < requiredToastCount; i++) {
                var toast = toaster.makeToast(msg);
            }
            verify(toastCompleteCount, requiredToastCount, 'toastCompleteCount');
        }
    });

    runner.addTest({
        id: 5,
        desc: 'Verify that the cycle can be broken using msGetWeakWinRTProperty and msSetWeakWinRTProperty',
        pri: '0',
        test: function () {
            var toaster = createToaster();
            setPropertyOnToaster(toaster);
            CollectGarbage();

            var requiredToastCount = 3;
            for (var i = 1; i < requiredToastCount; i++) {
                var toast = toaster.makeToast(msg);
            }
            verify(eventCallsCount, requiredToastCount, 'eventCallsCount');
        }
    });

    runner.addTest({
        id: 6,
        desc: 'Verify that the msSetWeakWinRTProperty can override existing property',
        pri: '0',
        test: function () {
            var toaster = createToaster();
            var toastCompleteCount = 0;

            var newObject = {
                _toaster: toaster,
                _data: "Data1"
            };

            var newObject2 = {
                _toaster: toaster,
                _data: "Data2"
            };

            var shouldFindData2 = false;

            function toastCompleteCallback(ev) {
                verify(ev.message, msg, 'ev.message');
                var weakProperty = msGetWeakWinRTProperty(ev.target, "weakProperty");
                assert(weakProperty !== null, 'msGetWeakWinRTProperty(ev.target, "weakProperty") !== null');
                assert(weakProperty !== undefined, 'msGetWeakWinRTProperty(ev.target, "weakProperty") !== undefined');
                if (shouldFindData2) {
                    verify(weakProperty._data, "Data2", 'msGetWeakWinRTProperty(ev.target, "weakProperty")._data');
                }
                else {
                    verify(weakProperty._data, "Data1", 'msGetWeakWinRTProperty(ev.target, "weakProperty")._data');
                }
                toastCompleteCount++;
            }

            toaster.addEventListener("toastcompleteevent", toastCompleteCallback);
            msSetWeakWinRTProperty(toaster, "weakProperty", newObject);
            var toast = toaster.makeToast(msg);
            verify(toastCompleteCount, 1, 'toastCompleteCount');

            msSetWeakWinRTProperty(toaster, "weakProperty", newObject2);
            shouldFindData2 = true;
            var requiredToastCount = 3;
            for (var i = 1; i < requiredToastCount; i++) {
                var toast = toaster.makeToast(msg);
            }
            verify(toastCompleteCount, requiredToastCount, 'toastCompleteCount');

            // Keep weakProperty alive
            verify(newObject._data, "Data1", "newObject._data");
            verify(newObject2._data, "Data2", "newObject2._data");
        }
    });

    runner.addTest({
        id: 7,
        desc: 'Parameters of msSetWeakWinRTProperty',
        pri: '0',
        test: function () {
            verify.exception(function () {
                msSetWeakWinRTProperty();
            }, Error, "msSetWeakWinRTProperty();");
            verify.exception(function () {
                msSetWeakWinRTProperty(new Animals.Animal());
            }, Error, "msSetWeakWinRTProperty(new Animals.Animal());");
            verify.exception(function () {
                msSetWeakWinRTProperty(new Animals.Animal(), "weakProp");
            }, Error, 'msSetWeakWinRTProperty(new Animals.Animal(), "weakProp");');

            verify.exception(function () {
                msSetWeakWinRTProperty(32, "weakProp", "32");
            }, TypeError, 'msSetWeakWinRTProperty(32, "weakProp", "32");');

            verify.noException(function () {
                var newObject = new Object();
                var myAnimal = new Animals.Animal();
                msSetWeakWinRTProperty(myAnimal, null, newObject);
                verify(msGetWeakWinRTProperty(myAnimal, null), newObject, 'msGetWeakWinRTProperty(myAnimal, null)');
            }, 'msSetWeakWinRTProperty(myAnimal, null, newObject);');
        }
    });

    runner.addTest({
        id: 8,
        desc: 'Parameters of msGetWeakWinRTProperty',
        pri: '0',
        test: function () {
            verify.exception(function () {
                msGetWeakWinRTProperty();
            }, Error, "msGetWeakWinRTProperty();");
            verify.exception(function () {
                msGetWeakWinRTProperty(new Animals.Animal());
            }, Error, "msGetWeakWinRTProperty(new Animals.Animal());");

            verify.exception(function () {
                msGetWeakWinRTProperty(32, "weakProp");
            }, TypeError, 'msGetWeakWinRTProperty(32, "weakProp");');

            verify.noException(function () {
                msGetWeakWinRTProperty(new Animals.Animal(), null);
            }, 'msGetWeakWinRTProperty(new Animals.Animal(), null);');
        }
    });

    runner.addTest({
        id: 9,
        desc: 'Verify Multiple WeakProperties can be set and get',
        pri: '0',
        test: function () {
            var toaster = createToaster();
            var toastCompleteCount = 0;

            var newObject = {
                _toaster: toaster,
                _data: "Data1"
            };

            var newObject2 = {
                _toaster: toaster,
                _data: "Data2"
            };

            var newObject3 = {
                _toaster: toaster,
                _data: "Data3"
            };

            function verifyWeakProperty(obj, weakPropertyValue) {
                var weakProperty = msGetWeakWinRTProperty(obj, weakPropertyValue);
                assert(weakProperty !== null, 'msGetWeakWinRTProperty(ev.target, "' + weakPropertyValue + '") !== null');
                assert(weakProperty !== undefined, 'msGetWeakWinRTProperty(ev.target, "' + weakPropertyValue + '") !== undefined');
                verify(weakProperty._data, weakPropertyValue, 'msGetWeakWinRTProperty(ev.target, "' + weakPropertyValue + '")._data');
            }

            function toastCompleteCallback(ev) {
                verify(ev.message, msg, 'ev.message');

                verifyWeakProperty(ev.target, "Data1");
                verifyWeakProperty(ev.target, "Data2");
                verifyWeakProperty(ev.target, "Data3");

                toastCompleteCount++;
            }

            toaster.addEventListener("toastcompleteevent", toastCompleteCallback);
            msSetWeakWinRTProperty(toaster, "Data1", newObject);
            msSetWeakWinRTProperty(toaster, "Data2", newObject2);
            msSetWeakWinRTProperty(toaster, "Data3", newObject3);
            var requiredToastCount = 3;
            for (var i = 0; i < requiredToastCount; i++) {
                var toast = toaster.makeToast(msg);
            }
            verify(toastCompleteCount, requiredToastCount, 'toastCompleteCount');

            // Keep weakProperty alive
            verify(newObject._data, "Data1", "newObject._data");
            verify(newObject2._data, "Data2", "newObject2._data");
            verify(newObject3._data, "Data3", "newObject3._data");
        }
    });

    runner.addTest({
        id: 10,
        desc: 'Verify weak properties can be set on non event handling projection object instance',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal();
            var myFish = new Animals.Fish();
            msSetWeakWinRTProperty(myAnimal, "WeakProperty", myFish);
            var weakProperty = msGetWeakWinRTProperty(myAnimal, "WeakProperty");
            verify(weakProperty, myFish, 'msGetWeakWinRTProperty(myAnimal, "WeakProperty")');
        }
    });

    Loader42_FileName = 'WeakWinRTProperty Tests'
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
