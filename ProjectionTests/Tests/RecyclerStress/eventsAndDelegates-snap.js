if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {

    // Helpers from Delegates.js
    function verifyResult(actual, expected) {
        var i = 0;
        for (p in actual) {
            verify(p, expected[i][0], 'name');
            verify(actual[p], expected[i][1], p);
            i++;
        }

        verify(i, expected.length, 'number of members');
    }

    function verifyMultipleParam_names_newWeight_outAnimal(actual, expectedNames, expectedWeight) {
        verifyResult(actual.names, expectedNames);
        verify(actual.newWeight, expectedWeight, 'actual.newWeight');
        verifyResult(actual.outAnimal.getNames(), expectedNames);
    }

    function verifyVector(myVectorString, myVector, myArray) {
        verify(myVector.length, myArray.length, myVectorString + '.length');
        for (var iIndex = 0; iIndex < myArray.length; iIndex++) {
            verify(myVector[iIndex], myArray[iIndex], myVectorString + '[' + iIndex + ']');
        }
    }

    // name, value
    var animalGetNamesExpected = [
        ['common', 'Wolverine'],
        ['scientific', 'Gulo gulo'],
        ['alsoKnownAs', 'Skunk Bear']
    ];

    // name, value
    var animalDimensionsExpected = [
        ['length', 180],
        ['width', 360]
    ];


    var animalFactory;
    var myAnimal;
    var nativeDelegateString;

    runner.globalSetup(function () {
        animalFactory = Animals.Animal;
        myAnimal = new animalFactory(1);
        nativeDelegateString = '\nfunction DelegateWithOutParam_HSTRING() {\n    [native code]\n}\n';
    });

    // Helpers from Events-RecyclerStress.js
    var msg = "123";

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

    function verifyMembers(objectString, actualObject, expectedProperties, typeofString) {
        var objectDump = easyPrint(objectString, actualObject);
        logger.comment("typeof " + objectString + ": " + typeof actualObject);
        logger.comment("Dump of properties : " + objectDump);

        if (typeofString === undefined) {
            verify(typeof actualObject, "object", "typeof " + objectString);
        }
        else {
            verify(typeof actualObject, typeofString, "typeof " + objectString);
        }

        var propertiesIndex = 0;
        for (p in actualObject) {
            // Look in properties
            verify(p, expectedProperties[propertiesIndex][0], objectString + '["' + p + '"]');
            verify(typeof actualObject[p], expectedProperties[propertiesIndex][1], 'typeof ' + objectString + '["' + p + '"]');

            if (typeof actualObject[p] === 'function') {
                verify(actualObject[p].length, expectedProperties[propertiesIndex][2], objectString + '["' + p + '"].length');
                logger.comment('Setting length of function to be 10');
                actualObject[p].length = 10;
                verify(actualObject[p].length, expectedProperties[propertiesIndex][2], objectString + '["' + p + '"].length');
            }
            propertiesIndex++;
        }

        verify(propertiesIndex, expectedProperties.length, 'number of members of ' + objectString);
    }

    function verifyEvObjectPropertyDescriptor(evObjectString, evObject, prop, evObjectTypeString) {
        logger.comment("Verifying property descriptor for " + evObjectString + "." + prop);

        if (evObjectTypeString === undefined) {
            verify(typeof evObject, "object", "typeof " + evObjectString);
        }
        else {
            verify(typeof evObject, evObjectTypeString, "typeof " + evObjectString);
        }
        verify((prop in evObject), true, "(" + prop + " in " + evObjectString + ")");

        logger.comment("Get property descriptor from object");
        var desc = Object.getOwnPropertyDescriptor(evObject, prop);
        assert(desc !== undefined, "desc !== undefined");

        verify(desc.writable, false, "desc.writable");
        verify(desc.enumerable, true, "desc.enumerable");
        verify(desc.configurable, false, "desc.configurable");
        verify(desc.get, undefined, "desc.get");
        verify(desc.set, undefined, "desc.set");
        assert(desc.value !== undefined, "desc.value");
    }

    function createToaster() {
        var toaster = new Fabrikam.Kitchen.Toaster();
        return toaster;
    }

    var myRC1WithEventMembers = [
        ['Animals.IInterface2WithEvent.onevent2', 'function', 1],
        ['addEventListener', 'function', 2],
        ['handler1', 'function', 2],
        ['invokeDelegate', 'function', 2],
        ['invokeEvent_I1E1', 'function', 1],
        ['invokeEvent_I1E2', 'function', 1],
        ['invokeEvent_I2E1', 'function', 1],
        ['invokeEvent_I2E3', 'function', 1],
        ['onevent1', 'object'],
        ['onevent2', 'object'],
        ['onevent21', 'object'],
        ['onevent3', 'object'],
        ['removeEventListener', 'function', 2],
        ['wasHandler1Invoked', 'boolean'],
    ];

    var chefMembers = [
        ['addEventListener', 'function', 2],
        ['capabilities', 'number'],
        ['makeBreakfast', 'function', 1],
        ['name', 'string'],
        ['onmaketoastroundoff', 'object'],
        ['onmultipletoastcompletearray', 'object'],
        ['onmultipletoastcompletecollection', 'object'],
        ['removeEventListener', 'function', 2],
        ['role', 'number'],
    ];

    runner.addTest({
        id: 5,
        desc: 'Delegates: OutParamInDelegate_InOutMixed',
        pri: '0',
        test: function () {

            function DelegateWithInOutMixedParamsCallback(animal, weight) {
                logger.comment("*** In out mixed params Delegate called");
                logger.comment("Setting animal.Weight to " + weight);
                verify(animal.weight, 50, 'animal.weight');
                animal.weight = weight;
                verify(animal.weight, weight, 'animal.weight');
                var myAnimalDimensions = animal.getDimensions();
                verifyResult(myAnimalDimensions, animalDimensionsExpected);
                return myAnimalDimensions;
            }

            var myAnimalDimensions = myAnimal.callDelegateWithOutParam_InOutMixed(DelegateWithInOutMixedParamsCallback, 40);
            verifyResult(myAnimalDimensions, animalDimensionsExpected);
        }

    });


    runner.addTest({
        id: 6,
        desc: 'Delegates: OutParamInDelegate_MultipleOutParams',
        pri: '0',
        test: function () {

            function DelegateWithMultipleOutParamsCallback(animal, weight) {
                logger.comment("*** Multiple out params Delegate called");
                verify(animal.weight, 40, 'animal.weight');
                animal.weight = weight;
                verify(animal.weight, weight, 'animal.weight');
                verifyResult(animal.getNames(), animalGetNamesExpected);

                // create return object:
                var myReturnObject = new Object();
                myReturnObject.names = animal.getNames();
                myReturnObject.newWeight = animal.weight;
                myReturnObject.outAnimal = animal;

                verifyMultipleParam_names_newWeight_outAnimal(myReturnObject, animalGetNamesExpected, weight);

                return myReturnObject;
            }

            var myAnimalInfo = myAnimal.callDelegateWithMultipleOutParams(DelegateWithMultipleOutParamsCallback, 60);
            verifyMultipleParam_names_newWeight_outAnimal(myAnimalInfo, animalGetNamesExpected, 60);
        }

    });

    runner.addTest({
        id: 8,
        desc: 'Delegates: DelegateAsOutParam_JSCallback',
        pri: '0',
        test: function () {

            function DelegateAsOutParamWithJSCallback(animal) {
                logger.comment("*** JSCallback as out delegate param called");
                var animalNames = animal.getNames();
                verifyResult(animalNames, animalGetNamesExpected);
                return animalNames.common;
            }

            var myNewCallback = myAnimal.methodDelegateAsOutParam(DelegateAsOutParamWithJSCallback);
            var myDelegateStringOut = myNewCallback(myAnimal);
            verify(myDelegateStringOut, 'Wolverine', 'myDelegateStringOut');
        }

    });

    runner.addTest({
        id: 12,
        desc: 'Delegates: NativeDelegateAsOutParam',
        pri: '0',
        test: function () {

            var myNativeDelegate = myAnimal.getNativeDelegateAsOutParam();
            verify(typeof myNativeDelegate, 'function', 'typeof myNativeDelegate');
            verify(myNativeDelegate + '', nativeDelegateString, 'myNativeDelegate');
            verify(myNativeDelegate.length, 1, 'myNativeDelegate.length');
            logger.comment('Setting length of function to be 10');
            myNativeDelegate.length = 10;
            verify(myNativeDelegate.length, 1, 'myNativeDelegate.length');
            myNativeDelegate.myExpando = 42;
            verify(myNativeDelegate.myExpando, undefined, "myNativeDelegate.myExpando");
        }
    });


    runner.addTest({
        id: 13,
        desc: 'Delegates: InvokeNativeDelegate',
        pri: '0',
        test: function () {

            var myNativeDelegate = myAnimal.getNativeDelegateAsOutParam();
            verify(myNativeDelegate + '', nativeDelegateString, 'myNativeDelegate');
            verify(myNativeDelegate(myAnimal), 'Wolverine');
        }
    });

    runner.addTest({
        id: 16,
        desc: 'Delegates: ParameterisedDelegate',
        pri: '0',
        test: function () {
            logger.comment("var myObservableVector = myAnimal.getObservableVector();");
            var myObservableVector = myAnimal.getObservableVector();
            verifyVector('myObservableVector', myObservableVector, [1, 2, 3, 4, 5, 6, 7, 8, 9]);

            function vectorChanged(ev) {
                logger.comment("*** ParameterizedDelegate invoke: vectorChanged");

                vectorChanged.myObservableVector = ev.target;
                vectorChanged.vectorChangedArgs = ev.detail[0];

                logger.comment("    newObservableVector: " + ev.target);

                logger.comment("    vectorChangedArgs.index: " + ev.index);
                logger.comment("    vectorChangedArgs.collectionChange: " + ev.collectionChange);
            }

            // Assign the event handler and get no error.
            logger.comment("myObservableVector.addEventListener('vectorchanged', vectorChanged);");
            myObservableVector.addEventListener('vectorchanged', vectorChanged);

            logger.comment("myObservableVector.append(1800);");
            myObservableVector.append(1800);

            logger.comment("Verifying results that were received in delegate");
            verifyVector('vectorChanged.myObservableVector', vectorChanged.myObservableVector, [1, 2, 3, 4, 5, 6, 7, 8, 9, 1800]);
            verify(vectorChanged.vectorChangedArgs.index, 9, "vectorChanged.vectorChangedArgs.index");
            verify(vectorChanged.vectorChangedArgs.collectionChange, 1, "vectorChanged.vectorChangedArgs.collectionChange");

            logger.comment("myObservableVector.removeAtEnd()");
            myObservableVector.removeAtEnd();

            logger.comment("Verifying results that were received in delegate");
            verifyVector('vectorChanged.myObservableVector', vectorChanged.myObservableVector, [1, 2, 3, 4, 5, 6, 7, 8, 9]);
            verify(vectorChanged.vectorChangedArgs.index, 9, "vectorChanged.vectorChangedArgs.index");
            verify(vectorChanged.vectorChangedArgs.collectionChange, 2, "vectorChanged.vectorChangedArgs.collectionChange");
        }
    });

    runner.addTest({
        id: 17,
        desc: 'Delegates: Exception thrown from delegate',
        pri: '0',
        test: function () {
            var correctPathHits = 1;
            var E_FAIL = Winery.WinRTErrorTests.RestrictedErrorAccess.errorCodes.fail;

            // Throw error object
            try {
                function DelegateWithOutHSTRINGCallback1(animal) {
                    logger.comment("Throwing from delegate : DelegateWithOutHSTRINGCallback1");
                    correctPathHits = correctPathHits + 1;
                    throw new Error();
                    logger.comment("Unreachable");
                    correctPathHits = 0;
                }
                logger.comment("Call into delegate");
                var result = myAnimal.callDelegateWithOutParam_HSTRING(DelegateWithOutHSTRINGCallback1);
                logger.comment("Also unreachable");
                correctPathHits = 0;
            } catch (e) {
                correctPathHits = correctPathHits + 1;
                logger.comment("Exception caught");
                verify(e.number, E_FAIL, "e.number");
            }
            verify(correctPathHits, 3, "Check for correct path taken")

            // Throw object thats not error
            try {
                function DelegateWithOutHSTRINGCallback2(animal) {
                    logger.comment("Throwing from delegate : DelegateWithOutHSTRINGCallback2");
                    correctPathHits = correctPathHits + 1;
                    throw 1;
                    logger.comment("Unreachable");
                    correctPathHits = 0;
                }
                logger.comment("Call into delegate");
                var result = myAnimal.callDelegateWithOutParam_HSTRING(DelegateWithOutHSTRINGCallback2);
                logger.comment("Also unreachable");
                correctPathHits = 0;
            } catch (e) {
                correctPathHits = correctPathHits + 1;
                logger.comment("Exception caught");
                verify(e.number, E_FAIL, "e.number");
            }
            verify(correctPathHits, 5, "Check for correct path taken")

            // Throw runtime error
            try {
                function DelegateWithOutHSTRINGCallback3(animal) {
                    logger.comment("Throwing from delegate : DelegateWithOutHSTRINGCallback3");
                    correctPathHits = correctPathHits + 1;
                    var a = undefined;
                    a();
                    logger.comment("Unreachable");
                    correctPathHits = 0;
                }
                logger.comment("Call into delegate");
                var result = myAnimal.callDelegateWithOutParam_HSTRING(DelegateWithOutHSTRINGCallback3);
                logger.comment("Also unreachable");
                correctPathHits = 0;
            } catch (e) {
                correctPathHits = correctPathHits + 1;
                logger.comment("Exception caught");
                verify(e.toString(), "TypeError: Object expected");
            }
            verify(correctPathHits, 7, "Check for correct path taken")
        }
    });

    runner.addTest({
        id: 28,
        desc: 'Delegates: Delegate with float as in out parameter mixed with other ints',
        pri: '0',
        test: function () {
            var delegateInvokeCount = 0;
            function delegateInOutParamFloat(inValue1, inValue2, inValue3, inValue4, inValue5) {
                logger.comment("*** delegateInOutParamFloat: Invoke");
                delegateInvokeCount++;
                verify(inValue1, 20, "inValue1");
                verify(inValue2, -40, "inValue2");
                verify(inValue3, 60, "inValue3");
                verify(inValue4, 70, "inValue4");
                verify(inValue5, -107374168, "inValue5");
                logger.comment("*** delegateInOutParamFloat: Exit");
                return { outValue1: -107374176, outValue2: 107374160 };
            }
            var result = Animals.Animal.callDelegateWithInOutFloat(delegateInOutParamFloat, 20, -40, 60, 70, -107374168);
            verify(delegateInvokeCount, 1, "Delegate invoke count");
            verify(result.outValue1, -107374176, "result.outValue1");
            verify(result.outValue2, 107374160, "result.outValue2");
        }
    });

    runner.addTest({
        id: 1,
        desc: 'Cross-Apartment Delegates: Basic',
        pri: '0',
        test: function () {
            var callbackCalled = false;
            function preheatCompleteCallback() {
                callbackCalled = true;
            }

            var toaster = new Fabrikam.Kitchen.Toaster();
            logger.comment('Fabrikam.Kitchen.Toaster created successfully');
            logger.comment('Calling toaster.preheatInBackground');
            toaster.preheatInBackground(preheatCompleteCallback);
            logger.comment('toaster.preheatInBackground done');
            verify(callbackCalled, true, 'Callback was called');
        }
    });

    runner.addTest({
        id: 2,
        desc: 'Cross-Apartment Delegates: Calling Delegate on wrong thread',
        pri: '0',
        test: function () {
            var callbackCalled = false;
            function preheatCompleteCallback() {
                callbackCalled = true;
            }

            var toaster = new Fabrikam.Kitchen.Toaster();
            logger.comment('Fabrikam.Kitchen.Toaster created successfully');
            logger.comment('Calling toaster.preheatInBackgroundWithSmuggledDelegate');
            verify.exception(function () {
                toaster.preheatInBackgroundWithSmuggledDelegate(preheatCompleteCallback);
            }, Error, "Calling Delegate on wrong thread should throw exception");
            logger.comment('toaster.preheatInBackground done');
            verify(callbackCalled, false, 'Callback was called');
        }
    });

    runner.addTest({
        id: 12,
        desc: 'Events: CombinationOfEventHandlerSet',
        pri: '0',
        test: function () {
            logger.comment("Create the chef and toaster");
            var myToaster = createToaster();
            var myKitchen = new Fabrikam.Kitchen.Kitchen();
            var myChef = new Fabrikam.Kitchen.Chef('Bob', myKitchen, 42);
            verifyMembers("myChef", myChef, chefMembers);
            verify(myChef.onmultipletoastcompletecollection, null, "myChef.onmultipletoastcompletecollection");

            var eventInvokeCount = 0;
            var eventInvokeCount1 = 0;
            var eventInvokeCount2 = 0;

            function multipleToastCompleteCollectionCallback1(ev) {
                logger.comment("*** multipleToastCompleteCollectionCallback1 : Invoke");
                eventInvokeCount++;
                eventInvokeCount1++;

                // Verify that event handler is being invoked correctly
                verify(ev.type, "multipletoastcompletecollection", "ev.type");

                logger.comment("*** multipleToastCompleteCollectionCallback1 : Exit");
            }

            function multipleToastCompleteCollectionCallback2(ev) {
                logger.comment("*** multipleToastCompleteCollectionCallback2 : Invoke");
                eventInvokeCount++;
                eventInvokeCount2++;

                // Verify that event handler is being invoked correctly
                verify(ev.type, "multipletoastcompletecollection", "ev.type");

                logger.comment("*** multipleToastCompleteCollectionCallback2 : Exit");
            }

            logger.comment("Add event handler for multiple toast complete - callback1");
            myChef.onmultipletoastcompletecollection = multipleToastCompleteCollectionCallback1;
            verify(myChef.onmultipletoastcompletecollection, multipleToastCompleteCollectionCallback1, "myChef.onmultipletoastcompletecollection");

            myChef.makeBreakfast(myToaster, 5);
            verify(eventInvokeCount, 1, 'eventInvokeCount');
            verify(eventInvokeCount1, 1, 'eventInvokeCount1');
            verify(eventInvokeCount2, 0, 'eventInvokeCount2');

            logger.comment("Set event handler to callback2");
            myChef.onmultipletoastcompletecollection = multipleToastCompleteCollectionCallback2;
            verify(myChef.onmultipletoastcompletecollection, multipleToastCompleteCollectionCallback2, "myChef.onmultipletoastcompletecollection");

            myChef.makeBreakfast(myToaster, 5);
            verify(eventInvokeCount, 2, 'eventInvokeCount');
            verify(eventInvokeCount1, 1, 'eventInvokeCount1');
            verify(eventInvokeCount2, 1, 'eventInvokeCount2');

            logger.comment("Set event handler to null");
            myChef.onmultipletoastcompletecollection = null;
            verify(myChef.onmultipletoastcompletecollection, null, "myChef.onmultipletoastcompletecollection");

            myChef.makeBreakfast(myToaster, 5);
            verify(eventInvokeCount, 2, 'eventInvokeCount');
            verify(eventInvokeCount1, 1, 'eventInvokeCount1');
            verify(eventInvokeCount2, 1, 'eventInvokeCount2');

            logger.comment("Set event handler to callback 2");
            myChef.onmultipletoastcompletecollection = multipleToastCompleteCollectionCallback2;
            verify(myChef.onmultipletoastcompletecollection, multipleToastCompleteCollectionCallback2, "myChef.onmultipletoastcompletecollection");

            myChef.makeBreakfast(myToaster, 5);
            verify(eventInvokeCount, 3, 'eventInvokeCount');
            verify(eventInvokeCount1, 1, 'eventInvokeCount1');
            verify(eventInvokeCount2, 2, 'eventInvokeCount2');
        }
    });

    Loader42_FileName = 'Events and Delegates Recycler Stress tests for SNAP';
})();Run() 
