if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {

    function verifyFunctionsArePresent(testClass, arr) {
        arr.forEach(function (f) {
            verify(typeof testClass[f], "function", 'Ensuring "' + f + '" is a function of Animals.VariableProjection.TestingClass.', false);
        });
    }

    function verifyFunctionsAreMissing(testClass, arr) {
        arr.forEach(function (f) {
            verify(typeof testClass[f], "undefined", 'Ensuring "' + f + '" is not a function of Animals.VariableProjection.TestingClass.', false);
        });
    }

    function verifyPropertiesArePresent(testClass, arr) {
        arr.forEach(function (p) {
            try {
                var descriptor = Object.getOwnPropertyDescriptor(testClass.__proto__, p);
                verify(typeof (descriptor === null ? undefined : descriptor.get), "function", 'Ensuring "' + p + '" has a getter on Animals.VariableProjection.TestingClass.', false);
                verify(typeof (descriptor === null ? undefined : descriptor.set), "function", 'Ensuring "' + p + '" has a setter on Animals.VariableProjection.TestingClass.', false);
            } catch (ex) {
                verify(false, true, "Exception thrown for: '" + p + "'; ex: " + ex.toString());
            }
        });
    }

    function verifyPropertiesAreMissing(testClass, arr) {
        arr.forEach(function (p) {
            verify(typeof testClass[p], "undefined", 'Ensuring "' + p + '" is not a property on Animals.VariableProjection.TestingClass.', false);
        });
    }

    function verifyPartialInterface(obj) {
        verifyFunctionsArePresent(obj, ["add"]);
        verifyFunctionsAreMissing(obj, ["partialStructMethod"]);
        verifyPropertiesAreMissing(obj, ["missingTypeProp", "missingInterfaceProp"]);

        verify(obj.add(1, 2), 1 + 2 + 42, "Verifying that the object's add function behaves as expected.");
    }

    function verifyExtendsPartialInterface(obj) {
        verifyPartialInterface(obj);
        verifyFunctionsArePresent(obj, ["divide"]);
        verify(obj.divide(2, 1), 2 / 1 + 42, "Verifying that the object's divide function behaves as expected.");
    }

    function verifyExtendsMissingInterface(obj) {
        verifyFunctionsArePresent(obj, ["subtract"]);
        verify(obj.subtract(2, 1), 2 - 1 + 42, "Verifying that the object's subtract function behaves as expected.");
    }

    function verifyExtendsInterfaceClass(obj) {
        verifyExtendsPartialInterface(obj);
        verifyFunctionsArePresent(obj, ["subtract"]);
        verify(obj.subtract(2, 1), 2 - 1 + 42, "Verifying that the object's subtract function behaves as expected.");
    }
    function verifyBool(v) { verify(typeof v, 'boolean', "Ensuring that we have a boolean."); };


    //TEST_TYPE_MISSING(MissingStruct, MissingConstructs::MissingStruct)
    //TEST_TYPE_MISSING(PartialStruct, PartialStruct)
    //TEST_TYPE_MISSING(PartialStructWithEnum, PartialStructWithEnum)
    //TEST_TYPE_RESOLVED(MissingBool, MissingConstructs::MissingBool, *value = true)
    //TEST_TYPE_MISSING(MissingEnum, MissingConstructs::MissingEnum)
    //TEST_TYPE_RESOLVED(PartialAndMissingInterfaceClass, IPartialInterface*, Details::MakeAndInitialize<PartialAndMissingInterfaceClass>(value))
    //TEST_TYPE_MISSING(MissingAndPartialInterfaceClass, MissingConstructs::IMissingInterface*)
    //TEST_TYPE_RESOLVED(PartialInterface, IPartialInterface*, Details::MakeAndInitialize<PartialAndMissingInterfaceClass>(value))
    //TEST_TYPE_MISSING(MissingInterface, MissingConstructs::IMissingInterface*)
    //TEST_TYPE_RESOLVED(ExtendsPartialInterface, IExtendsPartialInterface*, Details::MakeAndInitialize<ExtendsInterfaceClass>(value))
    //TEST_TYPE_RESOLVED(ExtendsMissingInterface, IExtendsMissingInterface*, Details::MakeAndInitialize<ExtendsReverseInterfaceClass>(value))
    //TEST_TYPE_RESOLVED(ExtendsInterfaceClass, IExtendsPartialInterface*, Details::MakeAndInitialize<ExtendsInterfaceClass>(value))
    //TEST_TYPE_RESOLVED(ExtendsReverseInterfaceClass, IExtendsMissingInterface*, Details::MakeAndInitialize<ExtendsReverseInterfaceClass>(value))
    //TEST_TYPE_MISSING(MissingDelegate, MissingConstructs::IMissingDelegate*);
    var typesNamesThatWillResolve = ["MissingBool", "PartialAndMissingInterfaceClass", "PartialInterface", "ExtendsPartialInterface", "ExtendsMissingInterface", "ExtendsInterfaceClass", "ExtendsReverseInterfaceClass"];
    var typesNamesThatWillBeMissing = ["MissingStruct", "PartialStruct", "PartialStructWithEnum", "MissingEnum", "MissingAndPartialInterfaceClass", "MissingInterface", "MissingDelegate"];

    runner.addTest({
        id: 1,
        desc: 'Ensure all the test functions on Animals.VariableProjection.testingClass are a function object',
        pri: '0',
        test: function () {
            var testClass = new Animals.VariableProjection.TestingClass();

            var makeFirstLetterLowerCaseAndAppendProp = function (i) { return i.charAt(0).toLowerCase() + i.substr(1) + "Prop"; };

            var presentFuncs = ["testDelegateWithPartialInterfaceOutParameter", "testDelegateWithPartialInterfaceInParameter", "testDelegateWithExtendsPartialInterfaceOutParameter",
                "testDelegateWithExtendsPartialInterfaceInParameter", "testDelegateWithExtendsMissingInterfaceOutParameter", "testDelegateWithExtendsMissingInterfaceInParameter",
                "testDelegateWithPartialAndMissingInterfaceClassOutParameter", "testDelegateWithPartialAndMissingInterfaceClassInParameter", "testDelegateWithExtendsInterfaceClassOutParameter", "testDelegateWithExtendsInterfaceClassInParameter",
                "testDelegateWithExtendsReverseInterfaceClassOutParameter", "testDelegateWithExtendsReverseInterfaceClassInParameter", "testDelegateWithMissingBoolInParameter", "testDelegateWithMissingBoolOutParameter"];

            var missingFuncs = ["testDelegateWithPartialStructOutParameter", "testDelegateWithPartialStructInParameter", "testDelegateWithPartialStructInRefParameter",
                "testDelegateWithMissingEnumInParameter", "testDelegateWithMissingEnumOutParameter", "checkMissingStructByRef", "checkPartialStructByRef", "checkPartialStructWithEnumByRef",
                "testDelegateWithMissingTypeOutParameter", "testDelegateWithMissingTypeInParameter", "testDelegateWithMissingAndPartialInterfaceClassOutParameter",
                "testDelegateWithMissingAndPartialInterfaceClassInParameter"];

            typesNamesThatWillResolve.forEach(function (i) {
                presentFuncs.push("check" + i);
                presentFuncs.push("check" + i + "Out");
            });

            typesNamesThatWillBeMissing.forEach(function (i) {
                missingFuncs.push("check" + i);
                missingFuncs.push("check" + i + "Out");
            });

            verifyFunctionsArePresent(testClass, presentFuncs);

            verifyFunctionsAreMissing(testClass, missingFuncs);

            verifyPropertiesArePresent(testClass, typesNamesThatWillResolve.map(makeFirstLetterLowerCaseAndAppendProp));

            verifyPropertiesAreMissing(testClass, typesNamesThatWillBeMissing.map(makeFirstLetterLowerCaseAndAppendProp));
        }
    });

    runner.addTest({
        id: 2,
        desc: 'Attempt to construct the runtime classes.',
        pri: '0',
        test: function () {
            function constructClass(name, canConstruct) {
                try {
                    var testClass = new Animals.VariableProjection[name]();
                    verify(true, canConstruct, "The class " + name + " was constructed, verifying thats the desired behavior.");
                } catch (ex) {
                    verify(false, canConstruct, "The class " + name + " wasn't constructed, verifying thats the desired behavior. Ex was: " + ex);
                }
            }

            constructClass("TestingClass", true);
            constructClass("PartialAndMissingInterfaceClass", true);
            constructClass("ExtendsInterfaceClass", true);
            constructClass("MissingAndPartialInterfaceClass", false);
            constructClass("ExtendsReverseInterfaceClass", true);
            constructClass("MissingType", false);
        }
    });

    runner.addTest({
        id: 3,
        desc: 'Delegates with resolvable interface types, but without all interface implementations resolvable.',
        pri: '0',
        test: function () {
            var testClass = new Animals.VariableProjection.TestingClass();
            result = testClass.testDelegateWithPartialInterfaceInParameter(function (a) { verifyPartialInterface(a); });
            result.outDelegate(new Animals.VariableProjection.PartialAndMissingInterfaceClass());
            result.retDelegate(new Animals.VariableProjection.PartialAndMissingInterfaceClass());

            result = testClass.testDelegateWithExtendsPartialInterfaceOutParameter(function (a) { verifyExtendsPartialInterface(a); });
            result.outDelegate(new Animals.VariableProjection.ExtendsInterfaceClass());
            result.retDelegate(new Animals.VariableProjection.ExtendsInterfaceClass());

            result = testClass.testDelegateWithPartialInterfaceOutParameter(function () {
                return { value: new Animals.VariableProjection.PartialAndMissingInterfaceClass(), returnValue: new Animals.VariableProjection.PartialAndMissingInterfaceClass() };
            });

            var returnedObjects = result.outDelegate();
            verifyPartialInterface(returnedObjects.value);
            verifyPartialInterface(returnedObjects.returnValue);

            returnedObjects = result.retDelegate();
            verifyPartialInterface(returnedObjects.value);
            verifyPartialInterface(returnedObjects.returnValue);

            result = testClass.testDelegateWithExtendsPartialInterfaceOutParameter(function () {
                return { value: new Animals.VariableProjection.ExtendsInterfaceClass(), returnValue: new Animals.VariableProjection.ExtendsInterfaceClass() };
            });
            var returnedObjects = result.outDelegate();
            verifyExtendsPartialInterface(returnedObjects.value);
            verifyExtendsPartialInterface(returnedObjects.returnValue);

            returnedObjects = result.retDelegate();
            verifyExtendsPartialInterface(returnedObjects.value);
            verifyExtendsPartialInterface(returnedObjects.returnValue);
        }
    });

    runner.addTest({
        id: 4,
        desc: 'Delegates with resolvable runtime class types, but without all interface implementations resolvable.',
        pri: '0',
        test: function () {
            var testClass = new Animals.VariableProjection.TestingClass();
            var result = testClass.testDelegateWithPartialAndMissingInterfaceClassInParameter(function (a) { verifyPartialInterface(a); });
            result.outDelegate(new Animals.VariableProjection.PartialAndMissingInterfaceClass());
            result.retDelegate(new Animals.VariableProjection.PartialAndMissingInterfaceClass());

            result = testClass.testDelegateWithExtendsInterfaceClassInParameter(function (a) { verifyExtendsInterfaceClass(a); });
            result.outDelegate(new Animals.VariableProjection.ExtendsInterfaceClass());
            result.retDelegate(new Animals.VariableProjection.ExtendsInterfaceClass());

            result = testClass.testDelegateWithPartialAndMissingInterfaceClassOutParameter(function () {
                return { value: new Animals.VariableProjection.PartialAndMissingInterfaceClass(), returnValue: new Animals.VariableProjection.PartialAndMissingInterfaceClass() };
            });

            var returnedObjects = result.outDelegate();
            verifyPartialInterface(returnedObjects.value);
            verifyPartialInterface(returnedObjects.returnValue);

            returnedObjects = result.retDelegate();
            verifyPartialInterface(returnedObjects.value);
            verifyPartialInterface(returnedObjects.returnValue);

            result = testClass.testDelegateWithExtendsInterfaceClassOutParameter(function () {
                return { value: new Animals.VariableProjection.ExtendsInterfaceClass(), returnValue: new Animals.VariableProjection.ExtendsInterfaceClass() };
            });
            var returnedObjects = result.outDelegate();
            verifyExtendsInterfaceClass(returnedObjects.value);
            verifyExtendsInterfaceClass(returnedObjects.returnValue);

            returnedObjects = result.retDelegate();
            verifyExtendsInterfaceClass(returnedObjects.value);
            verifyExtendsInterfaceClass(returnedObjects.returnValue);
        }
    });

    runner.addTest({
        id: 5,
        desc: 'Test Events',
        pri: '0',
        test: function () {
            var hasBeenCalled = false;

            var verifyNull = function (a) {
                verify(a, null, "The object passed should have been null.");
            }

            var testingClass = new Animals.VariableProjection.TestingClass();

            function testEvent(ev, verifyFunction) {
                var expectToBeCalled = verifyFunction !== undefined;
                var o = {};
                o.token = testingClass.addEventListener(ev, function (a) {
                    hasBeenCalled = true;
                    if (verifyFunction !== undefined) {
                        verifyFunction(a.target);
                    }
                    testingClass.removeEventListener(ev, o.token);
                });
                verify(hasBeenCalled, expectToBeCalled, "Test for whether the event '" + ev + "' listener has been called or not.");
                hasBeenCalled = false;
            }

            testEvent("invalidevent");
            testEvent("missingStructEvent");
            testEvent("partialstructevent");
            testEvent("partialstructwithenumevent");
            testEvent("missingenumevent");
            testEvent("missingandpartialinterfaceclassevent");
            testEvent("missinginterfaceevent");
            testEvent("missingdelegateevent");
            testEvent("missingboolevent", verifyBool);
            testEvent("partialandmissinginterfaceclassevent", verifyPartialInterface);
            testEvent("partialinterfaceevent", verifyPartialInterface);
            testEvent("extendspartialinterfaceevent", verifyExtendsPartialInterface);
            testEvent("extendsmissinginterfaceevent", verifyExtendsMissingInterface);
            testEvent("extendsinterfaceclassevent", verifyExtendsInterfaceClass);
            testEvent("extendsreverseinterfaceclassevent", verifyExtendsInterfaceClass);
        }
    });

    runner.addTest({
        id: 6,
        desc: 'Test delegates with arguments that are of unresolvable types',
        pri: '0',
        test: function () {
            var outDelegate = function () { return { value: null, returnValue: null }; };
            var inDelegate = function (value) { verify(value, null, "Arguments with unresolved types must ne null."); };

            var testingClass = new Animals.VariableProjection.TestingClass();
            function testDelegate(delegateName, isDelegateIn) {
                var result = testingClass["test" + delegateName](isDelegateIn ? inDelegate : outDelegate);
                //result.outDelegate is expected to same as the delegate passed on, result.retValue is expected to be a delegate constructed by the server

                if (isDelegateIn) {
                    result.outDelegate(null);
                    result.retDelegate(null);
                } else {
                    var returnValue = result.outDelegate();
                    verify(returnValue.value, null, "Our delegate was returning null.");
                    verify(returnValue.returnValue, null, "Our delegate was returning null.");
                    returnValue = result.retDelegate();
                    verify(returnValue.value, null, "Server delegate must return null.");
                    verify(returnValue.returnValue, null, "Server delegate must return null.");
                }
            }

            //testDelegate("DelegateWithMissingTypeOutParameter", false);
            //testDelegate("DelegateWithMissingTypeInParameter", true);
            //testDelegate("DelegateWithMissingAndPartialInterfaceClassOutParameter", false);
            //testDelegate("DelegateWithMissingAndPartialInterfaceClassInParameter", true);
        }
    });

    runner.addTest({
        id: 7,
        desc: 'Test delegates with arguments whos types resolvable',
        pri: '0',
        test: function () {
            var testingClass = new Animals.VariableProjection.TestingClass();
            function testDelegate(delegateName, isDelegateIn, valueVerifyFunction, typeCreationFunction) {
                var result = testingClass["test" + delegateName](isDelegateIn
                    ? function (value) { valueVerifyFunction(value); }
                    : function () { return { value: typeCreationFunction(), returnValue: typeCreationFunction() }; }
                    );
                //result.outDelegate is expected to same as the delegate passed on, result.retValue is expected to be a delegate constructed by the server

                if (isDelegateIn) {
                    result.outDelegate(typeCreationFunction());
                    result.retDelegate(typeCreationFunction());
                } else {
                    var returnValue = result.outDelegate();
                    valueVerifyFunction(returnValue.value);
                    valueVerifyFunction(returnValue.returnValue);
                    returnValue = result.retDelegate();
                    valueVerifyFunction(returnValue.value);
                    valueVerifyFunction(returnValue.returnValue);
                }
            }

            var createPartialClass = function () { return new Animals.VariableProjection.PartialAndMissingInterfaceClass(); };
            var createExtendsPartialClass = function () { return new Animals.VariableProjection.ExtendsInterfaceClass(); };
            var createExtendsMissingClass = function () { return new Animals.VariableProjection.ExtendsReverseInterfaceClass(); };
            var createBool = function () { return true; };

            testDelegate("DelegateWithMissingBoolOutParameter", false, verifyBool, createBool);
            testDelegate("DelegateWithPartialInterfaceOutParameter", false, verifyPartialInterface, createPartialClass);
            testDelegate("DelegateWithPartialInterfaceInParameter", true, verifyPartialInterface, createPartialClass);
            testDelegate("DelegateWithExtendsPartialInterfaceOutParameter", false, verifyExtendsPartialInterface, createExtendsPartialClass);
            testDelegate("DelegateWithExtendsPartialInterfaceInParameter", true, verifyExtendsPartialInterface, createExtendsPartialClass);
            testDelegate("DelegateWithExtendsMissingInterfaceOutParameter", false, verifyExtendsInterfaceClass, createExtendsMissingClass);
            testDelegate("DelegateWithExtendsMissingInterfaceInParameter", true, verifyExtendsInterfaceClass, createExtendsMissingClass);
            testDelegate("DelegateWithPartialAndMissingInterfaceClassOutParameter", false, verifyPartialInterface, createPartialClass);
            testDelegate("DelegateWithPartialAndMissingInterfaceClassInParameter", true, verifyPartialInterface, createPartialClass);
            testDelegate("DelegateWithExtendsInterfaceClassOutParameter", false, verifyExtendsInterfaceClass, createExtendsPartialClass);
            testDelegate("DelegateWithExtendsInterfaceClassInParameter", true, verifyExtendsInterfaceClass, createExtendsPartialClass);
            testDelegate("DelegateWithExtendsReverseInterfaceClassOutParameter", false, verifyExtendsInterfaceClass, createExtendsMissingClass);
            testDelegate("DelegateWithExtendsReverseInterfaceClassInParameter", true, verifyExtendsInterfaceClass, createExtendsMissingClass);
        }
    });
    
    runner.addTest({
        id: 8,
        desc: 'Test delay resolving parameter type',
        pri: '0',
        test: function () {
            // musicLibrary(Windows.Storage.StorageFolder) getter introduces Windows.Storage.Search.QueryOptions resolving 
            // because StorageFolder implemented interface Windows.Storage.Search.IStorageFolderQueryOperations, 
            // which has QueryOptions as input parameter for AreQueryOptionsSupported method. In fast path case, the QueryOptions 
            // rumtime class is created before calling the musicLibrary getter, at that time we should allow the metadata resolving
            Windows.Storage.KnownFolders.musicLibrary;
        }
    });

    Loader42_FileName = "WinRT VariableProjection Tests";
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
