if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
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

    runner.addTest({
        id: 1,
        desc: 'OutParamInDelegate_HSTRING',
        pri: '0',
        test: function () {
            function DelegateWithOutHSTRINGCallback(animal) {
                logger.comment("***HSTRING as out parameter Delegate called");
                var animalNames = animal.getNames();
                verifyResult(animalNames, animalGetNamesExpected);
                return animalNames.common;
            }

            var myAnimalDelegateStringOut = myAnimal.callDelegateWithOutParam_HSTRING(DelegateWithOutHSTRINGCallback);
            verify(myAnimalDelegateStringOut, 'Wolverine');
        }
    });

    runner.addTest({
        id: 2,
        desc: 'OutParamInDelegate_int',
        pri: '0',
        test: function () {

            function DelegateWithOutintCallback(animal) {
                logger.comment("***int as out parameter Delegate called");
                verify(animal.weight, 50, 'animal.weight');
                return animal.weight;
            }

            var myAnimalWeight = myAnimal.callDelegateWithOutParam_int(DelegateWithOutintCallback);
            verify(myAnimalWeight, 50, 'myAnimalWeight');
        }
    });

    runner.addTest({
        id: 3,
        desc: 'OutParamInDelegate_Interface',
        pri: '0',
        test: function () {

            function DelegateWithOutInterfaceCallback(animal) {
                logger.comment("***Interface as out parameter Delegate called");
                verifyResult(animal.getNames(), animalGetNamesExpected);
                return animal;
            }

            var myNewAnimal = myAnimal.callDelegateWithOutParam_Interface(DelegateWithOutInterfaceCallback);
            verifyResult(myNewAnimal.getNames(), animalGetNamesExpected);
        }

    });

    runner.addTest({
        id: 4,
        desc: 'OutParamInDelegate_Struct',
        pri: '0',
        test: function () {

            function DelegateWithOutStructCallback(animal) {
                logger.comment("*** Struct as out parameter Delegate called");
                var myAnimalDimensions = animal.getDimensions();
                verifyResult(myAnimalDimensions, animalDimensionsExpected);
                return myAnimalDimensions;
            }

            var myAnimalDimensions = myAnimal.callDelegateWithOutParam_Struct(DelegateWithOutStructCallback);
            verifyResult(myAnimalDimensions, animalDimensionsExpected);

        }

    });

    runner.addTest({
        id: 5,
        desc: 'OutParamInDelegate_InOutMixed',
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
        desc: 'OutParamInDelegate_MultipleOutParams',
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
        id: 7,
        desc: 'MarshalParamAsDelegate_Null',
        pri: '0',
        test: function () {

            var myMessage = myAnimal.marshalNullAsDelegate(null);
            verify(myMessage, 'Success', 'myMessage');
        }

    });

    runner.addTest({
        id: 8,
        desc: 'DelegateAsOutParam_JSCallback',
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
        id: 9,
        desc: 'DelegateAsOutParam_Null',
        pri: '0',
        test: function () {

            var outDelegate = myAnimal.methodDelegateAsOutParam(null);
            verify(outDelegate, null, 'outDelegate');
        }

    });


    runner.addTest({
        id: 10,
        desc: 'DelegateAsInParam_undefined',
        pri: '0',
        test: function () {
            var outDelegate = myAnimal.methodDelegateAsOutParam(undefined);
            verify(outDelegate, null, 'outDelegate');
        }

    });

    runner.addTest({
        id: 11,
        desc: 'DelegateAsInParam_undefinedVarIn',
        pri: '0',
        test: function () {
            var undefinedVar;
            var outDelegate = myAnimal.methodDelegateAsOutParam(undefinedVar);
            verify(outDelegate, null, 'outDelegate');
        }
    });

    runner.addTest({
        id: 12,
        desc: 'NativeDelegateAsOutParam',
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
        desc: 'InvokeNativeDelegate',
        pri: '0',
        test: function () {

            var myNativeDelegate = myAnimal.getNativeDelegateAsOutParam();
            verify(myNativeDelegate + '', nativeDelegateString, 'myNativeDelegate');
            verify(myNativeDelegate(myAnimal), 'Wolverine');
        }
    });

    runner.addTest({
        id: 14,
        desc: 'NativeDelegateAsInParam',
        pri: '0',
        test: function () {

            var myNativeDelegate = myAnimal.getNativeDelegateAsOutParam();
            verify(myNativeDelegate + '', nativeDelegateString, 'myNativeDelegate');

            var myNewCallback = myAnimal.methodDelegateAsOutParam(myNativeDelegate);
            verify(myNewCallback + '', nativeDelegateString, 'myNewCallback');
            verify(myNewCallback(myAnimal), 'Wolverine', 'myNewCallback(myAnimal)');
        }
    });

    runner.addTest({
        id: 15,
        desc: 'NativeDelegateAsInParam',
        pri: '0',
        test: function () {

            var myNativeDelegate = myAnimal.getNativeDelegateAsOutParam();
            verify(myNativeDelegate + '', nativeDelegateString, 'myNativeDelegate');

            var foundException = false;
            try {
                var myReturnValue = myAnimal.callDelegateWithOutParam_int(myNativeDelegate);
            }
            catch (e) {
                verify(e.description, 'Type mismatch', 'e.Description');
                foundException = true;
            }

            assert(foundException, 'Expected exception was caught');
        }
    });

    runner.addTest({
        id: 16,
        desc: 'ParameterisedDelegate',
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

            logger.comment("myObservableVector[5] = 88;");
            myObservableVector[5] = 88;

            logger.comment("Verifying results that were received in delegate");
            verifyVector('vectorChanged.myObservableVector', vectorChanged.myObservableVector, [1, 2, 3, 4, 5, 88, 7, 8, 9]);
            verify(vectorChanged.vectorChangedArgs.index, 5, "vectorChanged.vectorChangedArgs.index");
            verify(vectorChanged.vectorChangedArgs.collectionChange, 3, "vectorChanged.vectorChangedArgs.collectionChange");

            logger.comment("myObservableVector.insertAt(5, 1800);");
            myObservableVector.insertAt(5, 1800);

            logger.comment("Verifying results that were received in delegate");
            verifyVector('vectorChanged.myObservableVector', vectorChanged.myObservableVector, [1, 2, 3, 4, 5, 1800, 88, 7, 8, 9]);
            verify(vectorChanged.vectorChangedArgs.index, 5, "vectorChanged.vectorChangedArgs.index");
            verify(vectorChanged.vectorChangedArgs.collectionChange, 1, "vectorChanged.vectorChangedArgs.collectionChange");

            logger.comment("myObservableVector.removeAt(5)");
            myObservableVector.removeAt(5);

            logger.comment("Verifying results that were received in delegate");
            verifyVector('vectorChanged.myObservableVector', vectorChanged.myObservableVector, [1, 2, 3, 4, 5, 88, 7, 8, 9]);
            verify(vectorChanged.vectorChangedArgs.index, 5, "vectorChanged.vectorChangedArgs.index");
            verify(vectorChanged.vectorChangedArgs.collectionChange, 2, "vectorChanged.vectorChangedArgs.collectionChange");

            logger.comment("myObservableVector.append(1800);");
            myObservableVector.append(1800);

            logger.comment("Verifying results that were received in delegate");
            verifyVector('vectorChanged.myObservableVector', vectorChanged.myObservableVector, [1, 2, 3, 4, 5, 88, 7, 8, 9, 1800]);
            verify(vectorChanged.vectorChangedArgs.index, 9, "vectorChanged.vectorChangedArgs.index");
            verify(vectorChanged.vectorChangedArgs.collectionChange, 1, "vectorChanged.vectorChangedArgs.collectionChange");

            logger.comment("myObservableVector.removeAtEnd()");
            myObservableVector.removeAtEnd();

            logger.comment("Verifying results that were received in delegate");
            verifyVector('vectorChanged.myObservableVector', vectorChanged.myObservableVector, [1, 2, 3, 4, 5, 88, 7, 8, 9]);
            verify(vectorChanged.vectorChangedArgs.index, 9, "vectorChanged.vectorChangedArgs.index");
            verify(vectorChanged.vectorChangedArgs.collectionChange, 2, "vectorChanged.vectorChangedArgs.collectionChange");

            logger.comment("myObservableVector.clear()");
            myObservableVector.clear();

            logger.comment("Verifying results that were received in delegate");
            verifyVector('vectorChanged.myObservableVector', vectorChanged.myObservableVector, []);
            verify(vectorChanged.vectorChangedArgs.index, 0, "vectorChanged.vectorChangedArgs.index");
            verify(vectorChanged.vectorChangedArgs.collectionChange, 0, "vectorChanged.vectorChangedArgs.collectionChange");

            logger.comment("myObservableVector.replaceAll([50, 100, 150])");
            myObservableVector.replaceAll([50, 100, 150]);

            logger.comment("Verifying results that were received in delegate");
            verifyVector('vectorChanged.myObservableVector', vectorChanged.myObservableVector, [50, 100, 150]);

            if (typeof Animals._CLROnly === 'undefined') // native implementation
            {
                verify(vectorChanged.vectorChangedArgs.index, 0, "vectorChanged.vectorChangedArgs.index");
                verify(vectorChanged.vectorChangedArgs.collectionChange, 0, "vectorChanged.vectorChangedArgs.collectionChange");
            }
            else {
                verify(vectorChanged.vectorChangedArgs.index, 2, "vectorChanged.vectorChangedArgs.index");
                verify(vectorChanged.vectorChangedArgs.collectionChange, 1, "vectorChanged.vectorChangedArgs.collectionChange");
            }
            verifyVector('myObservableVector', myObservableVector, [50, 100, 150]);
        }
    });

    runner.addTest({
        id: 18,
        desc: 'Delegate returning object',
        pri: '0',
        test: function () {
            logger.comment('Start');
            function BooleanOut2Impl(p0, p1) {
                logger.comment('In delegate');
                return { p0: true, p1: false };
            }
            logger.comment('Call');
            var result = myAnimal.delIn_BooleanOut2(BooleanOut2Impl);
            logger.comment('Done');
        }
    });

    runner.addTest({
        id: 19,
        desc: 'Delegate outputing big struct',
        pri: '0',
        test: function () {
            var delegateInvokeCount = 0;
            function delegateWithBigStruct_Out() {
                logger.comment('*** delegateWithBigStruct_Out: Invoke');
                delegateInvokeCount++;

                var myOutputObject = new Object();
                myOutputObject.args = {
                    eType: Animals.CollectionChangeType.itemRemoved,
                    index: 38,
                    previousIndex: 49,
                    objectId: "delegateWithOutParamTestCase"
                };
                myOutputObject.objectId = "delegateWithOutParamTestCase";
                myOutputObject.eType = Animals.CollectionChangeType.itemRemoved;
                myOutputObject.index = 38;
                myOutputObject.previousIndex = 49;

                logger.comment('*** delegateWithBigStruct_Out: Exit');

                return myOutputObject;
            }
            var result = Animals.Animal.callDelegateWithOutParam_BigStruct(delegateWithBigStruct_Out);
            verify(delegateInvokeCount, 1, "delegateInvokeCount");

            verify(result.eTypeFromStruct, result.eType, "result.eTypeFromStruct");
            verify(result.indexFromStruct, result.index, "result.indexFromStruct");
            verify(result.previousIndexFromStruct, result.previousIndex, "result.previousIndexFromStruct");
            verify(result.objectIdFromStruct, result.objectId, "result.objectIdFromStruct");
        }
    });

    runner.addTest({
        id: 20,
        desc: 'Delegate taking in a big struct',
        pri: '0',
        test: function () {
            var delegateInvokeCount = 0;
            function delegateWithBigStruct_In(eventArgs, objectId, eType, index, previousIndex) {
                logger.comment('*** delegateWithBigStruct_In: Invoke');
                delegateInvokeCount++;

                assert(objectId === myObjectId, "objectId === myObjectId");
                assert(eType === myEventArgsType, "eType === myEventArgsType");
                assert(index === myIndex, "index === myIndex");
                assert(previousIndex === myPreviousIndex, "previousIndex == myPreviousIndex");

                verify(eventArgs.eType, myEventArgsType, "eventArgs.eType");
                verify(eventArgs.index, myIndex, "eventArgs.index");
                verify(eventArgs.previousIndex, myPreviousIndex, "eventArgs.previousIndex");
                verify(eventArgs.objectId, myObjectId, "eventArgs.objectId");

                logger.comment('*** delegateWithBigStruct_In: Exit');
            }
            var myEventArgsType = Animals.CollectionChangeType.itemRemoved;
            var myObjectId = "DelegateWithInParamTestCase";
            var myIndex = 38;
            var myPreviousIndex = 49;
            var result = Animals.Animal.callDelegateWithInParam_BigStruct(delegateWithBigStruct_In, myObjectId, myEventArgsType, myIndex, myPreviousIndex);
            verify(delegateInvokeCount, 1, "delegateInvokeCount");
        }
    });

    runner.addTest({
        id: 21,
        desc: 'Delegate with in and out Packed Byte',
        pri: '0',
        test: function () {
            var inValue = { field0: 80 };
            var delegateInvokeCount = 0;
            function delegatePackedByte(delegateInValue) {
                logger.comment("*** delegatePackedByte: Invoke");
                delegateInvokeCount++;
                verify(delegateInValue.toString(), "[object Animals.PackedByte]", "delegateInValue.toString()");
                verify(delegateInValue.field0, 80, "delegateInValue.field0");
                assert(delegateInValue.field0 === inValue.field0, "delegateInValue.field0 == inValue.field0");
                logger.comment("*** delegatePackedByte: Exit");
                return delegateInValue;
            }
            var outValue = Animals.Animal.callDelegateWithInOutPackedByte(inValue, delegatePackedByte);
            verify(delegateInvokeCount, 1, "delegateInvokeCount");
            verify(outValue.toString(), "[object Animals.PackedByte]", "outValue.toString()");
            verify(outValue.field0, 80, "outValue.field0");
            assert(outValue.field0 === inValue.field0, "outValue.field0 == inValue.field0");
        }
    });

    runner.addTest({
        id: 22,
        desc: 'Delegate with  in and out Packed Booleans',
        pri: '0',
        test: function () {
            var inValue = {
                field0: false,
                field1: true,
                field2: false,
                field3: false
            }
            var delegateInvokeCount = 0;
            function delegatePackedBoolean(delegateInValue) {
                logger.comment("*** delegatePackedBoolean: Invoke");
                delegateInvokeCount++;
                verify(delegateInValue.toString(), "[object Animals.PackedBoolean4]", "delegateInValue.toString()");
                verify(delegateInValue.field0, false, "delegateInValue.field0");
                verify(delegateInValue.field1, true, "delegateInValue.field1");
                verify(delegateInValue.field2, false, "delegateInValue.field2");
                verify(delegateInValue.field3, false, "delegateInValue.field3");

                assert(delegateInValue.field0 === inValue.field0, "delegateInValue.field0 === inValue.field0");
                assert(delegateInValue.field1 === inValue.field1, "delegateInValue.field1 === inValue.field1");
                assert(delegateInValue.field2 === inValue.field2, "delegateInValue.field2 === inValue.field2");
                assert(delegateInValue.field3 === inValue.field3, "delegateInValue.field3 === inValue.field3");
                logger.comment("*** delegatePackedBoolean: Exit");
                return delegateInValue;
            }
            var outValue = Animals.Animal.callDelegateWithInOutPackedBoolean(inValue, delegatePackedBoolean);
            verify(delegateInvokeCount, 1, "delegateInvokeCount");
            verify(outValue.toString(), "[object Animals.PackedBoolean4]", "outValue.toString()");
            verify(outValue.field0, false, "outValue.field0");
            verify(outValue.field1, true, "outValue.field1");
            verify(outValue.field2, false, "outValue.field2");
            verify(outValue.field3, false, "outValue.field3");

            assert(outValue.field0 === inValue.field0, "outValue.field0 === inValue.field0");
            assert(outValue.field1 === inValue.field1, "outValue.field1 === inValue.field1");
            assert(outValue.field2 === inValue.field2, "outValue.field2 === inValue.field2");
            assert(outValue.field3 === inValue.field3, "outValue.field3 === inValue.field3");
        }
    });

    runner.addTest({
        id: 23,
        desc: 'Delegate with in and out odd sized struct',
        pri: '0',
        test: function () {
            var inValue = {
                field0: 50,
                field1: 100,
                field2: 150
            }
            var delegateInvokeCount = 0;
            function delegateOddSizedStruct(delegateInValue) {
                logger.comment("*** delegateOddSizedStruct: Invoke");
                delegateInvokeCount++;
                verify(delegateInValue.toString(), "[object Animals.OddSizedStruct]", "delegateInValue.toString()");
                verify(delegateInValue.field0, 50, "delegateInValue.field0");
                verify(delegateInValue.field1, 100, "delegateInValue.field1");
                verify(delegateInValue.field2, 150, "delegateInValue.field2");

                assert(delegateInValue.field0 === inValue.field0, "delegateInValue.field0 === inValue.field0");
                assert(delegateInValue.field1 === inValue.field1, "delegateInValue.field1 === inValue.field1");
                assert(delegateInValue.field2 === inValue.field2, "delegateInValue.field2 === inValue.field2");
                logger.comment("*** delegateOddSizedStruct: Exit");
                return delegateInValue;
            }
            var outValue = Animals.Animal.callDelegateWithInOutOddSizedStruct(inValue, delegateOddSizedStruct);
            verify(delegateInvokeCount, 1, "delegateInvokeCount");
            verify(outValue.toString(), "[object Animals.OddSizedStruct]", "outValue.toString()");
            verify(outValue.field0, 50, "outValue.field0");
            verify(outValue.field1, 100, "outValue.field1");
            verify(outValue.field2, 150, "outValue.field2");

            assert(outValue.field0 === inValue.field0, "outValue.field0 === inValue.field0");
            assert(outValue.field1 === inValue.field1, "outValue.field1 === inValue.field1");
            assert(outValue.field2 === inValue.field2, "outValue.field2 === inValue.field2");
        }
    });

    runner.addTest({
        id: 24,
        desc: 'Delegate with in and out small complex struct',
        pri: '0',
        test: function () {
            var inValue = {
                field0: 50,
                field1: {
                    field0: 100
                },
                field2: 150
            }
            var delegateInvokeCount = 0;
            function delegateSmallComplexStruct(delegateInValue) {
                logger.comment("*** delegateSmallComplexStruct: Invoke");
                delegateInvokeCount++;
                verify(delegateInValue.toString(), "[object Animals.SmallComplexStruct]", "delegateInValue.toString()");
                verify(delegateInValue.field0, 50, "delegateInValue.field0");
                verify(delegateInValue.field1.toString(), "[object Animals.PackedByte]", "delegateInValue.field1.toString()");
                verify(delegateInValue.field1.field0, 100, "delegateInValue.field1.field0");
                verify(delegateInValue.field2, 150, "delegateInValue.field2");

                assert(delegateInValue.field0 === inValue.field0, "delegateInValue.field0 === inValue.field0");
                assert(delegateInValue.field1.field0 === inValue.field1.field0, "delegateInValue.field1.field0 === inValue.field1.field0");
                assert(delegateInValue.field2 === inValue.field2, "delegateInValue.field2 === inValue.field2");
                logger.comment("*** delegateSmallComplexStruct: Exit");
                return delegateInValue;
            }
            var outValue = Animals.Animal.callDelegateWithInOutSmallComplexStruct(inValue, delegateSmallComplexStruct);
            verify(delegateInvokeCount, 1, "delegateInvokeCount");
            verify(outValue.toString(), "[object Animals.SmallComplexStruct]", "outValue.toString()");
            verify(outValue.field0, 50, "outValue.field0");
            verify(outValue.field1.toString(), "[object Animals.PackedByte]", "outValue.field1.toString()");
            verify(outValue.field1.field0, 100, "outValue.field1.field0");
            verify(outValue.field2, 150, "outValue.field2");

            assert(outValue.field0 === inValue.field0, "outValue.field0 === inValue.field0");
            assert(outValue.field1.field0 === inValue.field1.field0, "outValue.field1.field0 === inValue.field1.field0");
            assert(outValue.field2 === inValue.field2, "outValue.field2 === inValue.field2");
        }
    });


    runner.addTest({
        id: 25,
        desc: 'Delegate with in and out big complex struct',
        pri: '0',
        test: function () {
            var inValue = {
                field0: 5,
                field1: {
                    field0: 10
                },
                field2: 15,
                field3: {
                    field0: true,
                    field1: true,
                    field2: false,
                    field3: false
                },
                field4: {
                    field0: 20,
                    field1: {
                        field0: 25
                    },
                    field2: 30
                },
                field5: {
                    field0: 35,
                    field1: {
                        field0: 40
                    },
                    field2: 45
                },
                field6: 50,
                field7: 55
            }
            var delegateInvokeCount = 0;
            function delegateBigComplexStruct(delegateInValue) {
                logger.comment("*** delegateBigComplexStruct: Invoke");
                delegateInvokeCount++;
                verify(delegateInValue.toString(), "[object Animals.BigComplexStruct]", "delegateInValue.toString()");
                verify(delegateInValue.field0, 5, "delegateInValue.field0");
                verify(delegateInValue.field1.toString(), "[object Animals.PackedByte]", "delegateInValue.field1.toString()");
                verify(delegateInValue.field1.field0, 10, "delegateInValue.field1.field0");
                verify(delegateInValue.field2, 15, "delegateInValue.field2");
                verify(delegateInValue.field3.toString(), "[object Animals.PackedBoolean4]", "delegateInValue.field3.toString()");
                verify(delegateInValue.field3.field0, true, "delegateInValue.field3.field0");
                verify(delegateInValue.field3.field1, true, "delegateInValue.field3.field1");
                verify(delegateInValue.field3.field2, false, "delegateInValue.field3.field2");
                verify(delegateInValue.field3.field3, false, "delegateInValue.field3.field3");
                verify(delegateInValue.field4.toString(), "[object Animals.SmallComplexStruct]", "delegateInValue.field3.toString()");
                verify(delegateInValue.field4.field0, 20, "delegateInValue.field4.field0");
                verify(delegateInValue.field4.field1.toString(), "[object Animals.PackedByte]", "delegateInValue.field4.field1.toString()");
                verify(delegateInValue.field4.field1.field0, 25, "delegateInValue.field4.field1.field0");
                verify(delegateInValue.field4.field2, 30, "delegateInValue.field4.field2");
                verify(delegateInValue.field5.toString(), "[object Animals.SmallComplexStruct]", "delegateInValue.field5.toString()");
                verify(delegateInValue.field5.field0, 35, "delegateInValue.field5.field0");
                verify(delegateInValue.field5.field1.toString(), "[object Animals.PackedByte]", "delegateInValue.field5.field1.toString()");
                verify(delegateInValue.field5.field1.field0, 40, "delegateInValue.field5.field1.field0");
                verify(delegateInValue.field5.field2, 45, "delegateInValue.field5.field2");
                verify(delegateInValue.field6, 50, "delegateInValue.field6");
                verify(delegateInValue.field7, 55, "delegateInValue.field7");

                assert(delegateInValue.field0 === inValue.field0, "delegateInValue.field0 === inValue.field0");
                assert(delegateInValue.field1.field0 === inValue.field1.field0, "delegateInValue.field1.field0 === inValue.field1.field0");
                assert(delegateInValue.field2 === inValue.field2, "delegateInValue.field2 === inValue.field2");
                assert(delegateInValue.field3.field0 === inValue.field3.field0, "delegateInValue.field3.field0 === inValue.field3.field0");
                assert(delegateInValue.field3.field1 === inValue.field3.field1, "delegateInValue.field3.field1 === inValue.field3.field1");
                assert(delegateInValue.field3.field2 === inValue.field3.field2, "delegateInValue.field3.field2 === inValue.field3.field2");
                assert(delegateInValue.field3.field3 === inValue.field3.field3, "delegateInValue.field3.field3 === inValue.field3.field3");
                assert(delegateInValue.field4.field0 === inValue.field4.field0, "delegateInValue.field4.field0 === inValue.field4.field0");
                assert(delegateInValue.field4.field1.field0 === inValue.field4.field1.field0, "delegateInValue.field4.field1.field0 === inValue.field4.field1.field0");
                assert(delegateInValue.field4.field2 === inValue.field4.field2, "delegateInValue.field4.field2 === inValue.field4.field2");
                assert(delegateInValue.field5.field0 === inValue.field5.field0, "delegateInValue.field5.field0 === inValue.field5.field0");
                assert(delegateInValue.field5.field1.field0 === inValue.field5.field1.field0, "delegateInValue.field5.field1.field0 === inValue.field5.field1.field0");
                assert(delegateInValue.field5.field2 === inValue.field5.field2, "delegateInValue.field5.field2 === inValue.field5.field2");
                assert(delegateInValue.field6 === inValue.field6, "delegateInValue.field6 === inValue.field6");
                assert(delegateInValue.field7 === inValue.field7, "delegateInValue.field7 === inValue.field7");
                logger.comment("*** delegateBigComplexStruct: Exit");
                return delegateInValue;
            }
            var outValue = Animals.Animal.callDelegateWithInOutBigComplexStruct(inValue, delegateBigComplexStruct);
            verify(delegateInvokeCount, 1, "delegateInvokeCount");
            verify(outValue.toString(), "[object Animals.BigComplexStruct]", "outValue.toString()");
            verify(outValue.field0, 5, "outValue.field0");
            verify(outValue.field1.toString(), "[object Animals.PackedByte]", "outValue.field1.toString()");
            verify(outValue.field1.field0, 10, "outValue.field1.field0");
            verify(outValue.field2, 15, "outValue.field2");
            verify(outValue.field3.toString(), "[object Animals.PackedBoolean4]", "outValue.field3.toString()");
            verify(outValue.field3.field0, true, "outValue.field3.field0");
            verify(outValue.field3.field1, true, "outValue.field3.field1");
            verify(outValue.field3.field2, false, "outValue.field3.field2");
            verify(outValue.field3.field3, false, "outValue.field3.field3");
            verify(outValue.field4.toString(), "[object Animals.SmallComplexStruct]", "outValue.field3.toString()");
            verify(outValue.field4.field0, 20, "outValue.field4.field0");
            verify(outValue.field4.field1.toString(), "[object Animals.PackedByte]", "outValue.field4.field1.toString()");
            verify(outValue.field4.field1.field0, 25, "outValue.field4.field1.field0");
            verify(outValue.field4.field2, 30, "outValue.field4.field2");
            verify(outValue.field5.toString(), "[object Animals.SmallComplexStruct]", "outValue.field5.toString()");
            verify(outValue.field5.field0, 35, "outValue.field5.field0");
            verify(outValue.field5.field1.toString(), "[object Animals.PackedByte]", "outValue.field5.field1.toString()");
            verify(outValue.field5.field1.field0, 40, "outValue.field5.field1.field0");
            verify(outValue.field5.field2, 45, "outValue.field5.field2");
            verify(outValue.field6, 50, "outValue.field6");
            verify(outValue.field7, 55, "outValue.field7");

            assert(outValue.field0 === inValue.field0, "outValue.field0 === inValue.field0");
            assert(outValue.field1.field0 === inValue.field1.field0, "outValue.field1.field0 === inValue.field1.field0");
            assert(outValue.field2 === inValue.field2, "outValue.field2 === inValue.field2");
            assert(outValue.field3.field0 === inValue.field3.field0, "outValue.field3.field0 === inValue.field3.field0");
            assert(outValue.field3.field1 === inValue.field3.field1, "outValue.field3.field1 === inValue.field3.field1");
            assert(outValue.field3.field2 === inValue.field3.field2, "outValue.field3.field2 === inValue.field3.field2");
            assert(outValue.field3.field3 === inValue.field3.field3, "outValue.field3.field3 === inValue.field3.field3");
            assert(outValue.field4.field0 === inValue.field4.field0, "outValue.field4.field0 === inValue.field4.field0");
            assert(outValue.field4.field1.field0 === inValue.field4.field1.field0, "outValue.field4.field1.field0 === inValue.field4.field1.field0");
            assert(outValue.field4.field2 === inValue.field4.field2, "outValue.field4.field2 === inValue.field4.field2");
            assert(outValue.field5.field0 === inValue.field5.field0, "outValue.field5.field0 === inValue.field5.field0");
            assert(outValue.field5.field1.field0 === inValue.field5.field1.field0, "outValue.field5.field1.field0 === inValue.field5.field1.field0");
            assert(outValue.field5.field2 === inValue.field5.field2, "outValue.field5.field2 === inValue.field5.field2");
            assert(outValue.field6 === inValue.field6, "outValue.field6 === inValue.field6");
            assert(outValue.field7 === inValue.field7, "outValue.field7 === inValue.field7");
        }
    });

    runner.addTest({
        id: 26,
        desc: 'Delegate with float as inParam',
        pri: '0',
        test: function () {
            var delegateInvokeCount = 0;
            function delegateInParamFloat(inValue) {
                logger.comment("*** delegateInParamFloat: Invoke");
                delegateInvokeCount++;
                verify(inValue, -107374176, "inValue");
                logger.comment("*** delegateInParamFloat: Exit");
            }
            Animals.Animal.callDelegateWithInFloat(delegateInParamFloat, -107374176);
            verify(delegateInvokeCount, 1, "Delegate invoke count");
        }
    });

    runner.addTest({
        id: 27,
        desc: 'Delegate with float as outParam',
        pri: '0',
        test: function () {
            var delegateInvokeCount = 0;
            function delegateOutParamFloat() {
                logger.comment("*** delegateOutParamFloat: Invoke");
                delegateInvokeCount++;
                logger.comment("*** delegateOutParamFloat: Exit");
                return -107374176;
            }
            var outValue = Animals.Animal.callDelegateWithOutFloat(delegateOutParamFloat);
            verify(delegateInvokeCount, 1, "Delegate invoke count");
            verify(outValue, -107374176, "outValue");
        }
    });

    runner.addTest({
        id: 28,
        desc: 'Delegate with float as in out parameter mixed with other ints',
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
        id: 29,
        desc: 'Delegate stack overflow',
        pri: '0',
        test: function () {
            var invokeCount = 0;
            var passedInFloatVal = 10;
            function validDelegate(floatVal) {
                logger.comment("*** Valid Delegate : Invoke");
                invokeCount++;
                verify(floatVal, passedInFloatVal, "Received Float Val");
                logger.comment("*** Valid Delegate : Exit");
            }
            Animals.Animal.callDelegateWithInFloat(validDelegate, passedInFloatVal);
            verify(invokeCount, 1, "ValidDelegate was called");

            invokeCount = 0;
            function validRecursiveDelegate(floatVal) {
                logger.comment("*** validRecursiveDelegate " + (invokeCount + 1) + " : Invoke");
                invokeCount++;
                verify(floatVal, passedInFloatVal, "Received Float Val");
                if (invokeCount < 4) {
                    Animals.Animal.callDelegateWithInFloat(validRecursiveDelegate, passedInFloatVal);
                }
                Animals.Animal.callDelegateWithInFloat(validDelegate, passedInFloatVal);
                logger.comment("*** validRecursiveDelegate " + invokeCount + " : Exit");
            }
            Animals.Animal.callDelegateWithInFloat(validRecursiveDelegate, passedInFloatVal);
            verify(invokeCount, 8, "ValidDelegate was called");
        }
    });

    runner.addTest({
        id: "Win8: 874118 - 1",
        desc: 'RCConstructorFunctionAsDelegateInParam',
        pri: '0',
        test: function () {

            var foundException = false;
            try {
                var myReturnValue = myAnimal.callDelegateWithOutParam_HSTRING(Windows.Foundation.Uri);
            }
            catch (e) {
                verify(e.description, 'Type mismatch', 'e.Description');
                foundException = true;
            }

            assert(foundException, 'Expected exception was caught');
        }
    });

    runner.addTest({
        id: "Win8: 874118 - 2",
        desc: 'RCInstanceMethodAsDelegateInParam',
        pri: '0',
        test: function () {

            var foundException = false;
            try {
                var myReturnValue = myAnimal.callDelegateWithOutParam_HSTRING((new Animals.Animal()).getDimensions);
            }
            catch (e) {
                verify(e.description, 'Type mismatch', 'e.Description');
                foundException = true;
            }

            assert(foundException, 'Expected exception was caught');
        }
    });

    runner.addTest({
        id: "Win8: 874118 - 3",
        desc: 'RCStaticMethodAsDelegateInParam',
        pri: '0',
        test: function () {

            var foundException = false;
            try {
                var myReturnValue = myAnimal.callDelegateWithOutParam_int(Animals.Animal.marshalInAndOutPackedByte);
            }
            catch (e) {
                verify(e.description, 'Type mismatch', 'e.Description');
                foundException = true;
            }

            assert(foundException, 'Expected exception was caught');
        }
    });

    runner.addTest({
        id: "Win8: 891483 - 1",
        desc: 'RCOverloadMethodAsDelegateInParam',
        pri: '0',
        test: function () {

            var foundException = false;
            try {
                var myReturnValue = myAnimal.callDelegateWithOutParam_int((new Animals.Turkey()).toSandwich);
            }
            catch (e) {
                verify(e.description, 'Type mismatch', 'e.Description');
                foundException = true;
            }

            assert(foundException, 'Expected exception was caught');
        }
    });

    runner.addTest({
        id: "Win8: 891483 - 2",
        desc: 'RCFastPathMethodAsDelegateInParam',
        pri: '0',
        test: function () {

            var foundException = false;
            try {
                var myReturnValue = myAnimal.callDelegateWithOutParam_HSTRING((new Windows.Foundation.Uri("http://a")).combineUri);
            }
            catch (e) {
                verify(e.description, 'Type mismatch', 'e.Description');
                foundException = true;
            }

            assert(foundException, 'Expected exception was caught');
        }
    });

    runner.addTest({
        id: "Win8: 891483 - 3",
        desc: 'RCStaticFastPathMethodAsDelegateInParam',
        pri: '0',
        test: function () {

            var foundException = false;
            try {
                var myReturnValue = myAnimal.callDelegateWithOutParam_int(Animals.Animal.getAnswer);
            }
            catch (e) {
                verify(e.description, 'Type mismatch', 'e.Description');
                foundException = true;
            }

            assert(foundException, 'Expected exception was caught');
        }
    });

    runner.addTest({
        id: "Win8: 866210",
        desc: 'Verify undefined/null return value behavior',
        pri: '0',
        test: function () {
            var params = DevTests.Delegates.Params;
            var testClass = new DevTests.Delegates.TestClass();

            var returnObject;
            function delegateFunc(inspectable) {
                verify.strictEqual(inspectable, testClass, "Verify correct inspectable received");
                return returnObject;
            }

            verify.noException(function () {
                testClass.verifyTestDelegateOutParams((params.str | params.num | params.class | params.iface), params.none, delegateFunc);
            }, "Verify out parameters with return object: " + returnObject);

            returnObject = null;
            verify.noException(function () {
                testClass.verifyTestDelegateOutParams(params.none, (params.str | params.num | params.class | params.iface), delegateFunc);
            }, "Verify out parameters with return object: " + returnObject);

            returnObject = "abc";
            verify.noException(function () {
                testClass.verifyTestDelegateOutParams((params.str | params.num | params.class | params.iface), params.none, delegateFunc);
            }, "Verify out parameters with return object: " + returnObject.toString());

            returnObject = { str: null, rc: testClass };
            verify.noException(function () {
                testClass.verifyTestDelegateOutParams((params.num | params.iface), params.str, delegateFunc);
            }, "Verify out parameters with return object: " + returnObject.toString());

            returnObject = { str: "hello", rc: testClass, num: 42, iface: testClass };
            verify.noException(function () {
                testClass.verifyTestDelegateOutParams(params.none, params.none, delegateFunc);
            }, "Verify out parameters with return object: " + returnObject.toString());
        }
    });

    Loader42_FileName = 'Delegates tests';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
