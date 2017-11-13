if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    function verifyVectorAndArrayItems(myVectorString, myVector, myArrayString, myArray) {
        if (myArray == null) {
            verify(myVector, myArray, myVectorString);
        }
        else {
            verify(myVector.length, myArray.length, "Number of array Items and Vector Items")

            for (var iIndex = 0; iIndex < myVector.length; iIndex++) {
                verify(myVector[iIndex], myArray[iIndex], myVectorString + '[' + iIndex + '] and ' + myArrayString + '[' + iIndex + ']');
            }
        }
    }

    function verifyVectorAndStringArrayItems(myVectorString, myVector, myArrayString, myArray) {
        verify(myVector.length, myArray.length, "Number of array Items and Vector Items")

        for (var iIndex = 0; iIndex < myVector.length; iIndex++) {
            verify(myVector[iIndex], myArray[iIndex].toString(), myVectorString + '[' + iIndex + '] and ' + myArrayString + '[' + iIndex + '].toString()');
        }
    }

    function verifyResultObject(actualString, actual, expected, expectedType) {
        verify(typeof actual, expectedType, 'typeof ' + actualString);

        var i = 0;
        for (p in actual) {
            verify(p, expected[i][0], actualString + '.' + p);
            verify(typeof actual[p], expected[i][1], 'typeof ' + actualString + '.' + p);
            if (typeof actual[p] == 'function') {
                verify(actual[p].length, expected[i][2], actualString + '.' + p + ".length");
                logger.comment('Setting length of function to be 10');
                actual[p].length = 10;
                verify(actual[p].length, expected[i][2], actualString + '.' + p + ".length");
            }
            i++;
        }

        verify(i, expected.length, 'number of members');
    }

    runner.addTest({
        id: 1,
        desc: 'Pass, Fill, ReceiveArray',
        pri: '0',
        test: function () {
            logger.comment("var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];");
            var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            var myAnimal = new Animals.Animal(1);

            var myVector = myAnimal.passArray(myArray);
            verifyVectorAndArrayItems("myVector", myVector, "myArray", myArray)

            myVector = myAnimal.fillArray(myArray);
            verifyVectorAndArrayItems("myVector", myVector, "myArray", myArray)

            var myResult = myAnimal.receiveArray();
            var expectedResult = [["value", "object"], ["outVector", "object"]];
            verifyResultObject("myResult", myResult, expectedResult, "object");
            verifyVectorAndArrayItems("myResult.value", myResult.value, "myResult.outVector", myResult.outVector);

            var myArrayProjection = myResult.value;

            // Check if we can marshal in the projected array as inout parameter:
            myVector = myAnimal.fillArray(myArrayProjection);
            verifyVectorAndArrayItems("myVector", myVector, "myArrayProjection", myArrayProjection)

            // Check if we can marshal in the projected array as in parameter:
            myArrayProjection[4] = 300
            myVector = myAnimal.passArray(myArrayProjection);
            verifyVectorAndArrayItems("myVector", myVector, "myArrayProjection", myArrayProjection)

            // Verify marshalling into different type throws exception - in param
            verify.exception(function () {
                myVector = myAnimal.passArrayHSTRING(myArrayProjection);
            }, TypeError, "myVector = myAnimal.passArrayHSTRING(myArrayProjection);");

            // Verify marshalling into different type throws exception - inout param
            verify.exception(function () {
                myVector = myAnimal.fillArrayHSTRING(myArrayProjection);
            }, TypeError, "myVector = myAnimal.fillArrayHSTRING(myArrayProjection);");
        }
    });

    runner.addTest({
        id: 2,
        desc: 'ProjectOutByRefArrayNull_Basic',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);

            // Null as array
            myResult = myAnimal.passArray(null);
            myResult = myAnimal.receiveArray();
            expectedResult = [["value", "object"], ["outVector", "object"]];
            verifyVectorAndArrayItems("myResult.value", myResult.value, "null", null);
        }
    });

    runner.addTest({
        id: 3,
        desc: 'Delegate_MarshalInArray_Basic',
        pri: '0',
        test: function () {
            function delegatePassArray(animal, intArray) {
                logger.comment("*** delegatePassArray Delegate");
                var intVector = delegatePassArray.outVector;
                verifyVectorAndArrayItems("intArray", intArray, "intVector", intVector)

                // Check if we can marshal in the projected array as inout parameter:
                myVector = myAnimal.fillArray(intArray);
                verifyVectorAndArrayItems("myVector", myVector, "intArray", intArray)

                // Check if we can marshal in the projected array as in parameter:
                intArray[4] = 300
                var myVector = animal.passArray(intArray);
                verifyVectorAndArrayItems("myVector", myVector, "intArray", intArray)

                // Verify marshalling into different type throws exception - in param
                verify.exception(function () {
                    myVector = myAnimal.passArrayHSTRING(intArray);
                }, TypeError, "myVector = myAnimal.passArrayHSTRING(intArray);");

                // Verify marshalling into different type throws exception - inout param
                verify.exception(function () {
                    myVector = myAnimal.fillArrayHSTRING(intArray);
                }, TypeError, "myVector = myAnimal.fillArrayHSTRING(intArray);");
            }

            var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            var myAnimal = new Animals.Animal(1);

            delegatePassArray.outVector = myAnimal.passArray(myArray);
            myAnimal.callDelegatePassArray(delegatePassArray);
            verify(myArray, [1, 2, 3, 4, 5, 6, 7, 8, 9], "myArray");
        }
    });

    runner.addTest({
        id: 4,
        desc: 'Delegate_ProjectOutArrayByValue',
        pri: '0',
        test: function () {
            function delegateFillArray(animal, intArray) {
                logger.comment("*** delegateFillArray Delegate");

                // Check if we can marshal in the projected array as inout parameter:
                myVector = myAnimal.fillArray(intArray);
                verifyVectorAndArrayItems("myVector", myVector, "intArray", intArray)

                // Check if we can marshal in the projected array as in parameter:
                intArray[4] = 300
                var myVector = animal.passArray(intArray);
                verifyVectorAndArrayItems("myVector", myVector, "intArray", intArray)

                // Verify marshalling into different type throws exception - in param
                verify.exception(function () {
                    myVector = myAnimal.passArrayHSTRING(intArray);
                }, TypeError, "myVector = myAnimal.passArrayHSTRING(intArray);");

                // Verify marshalling into different type throws exception - inout param
                verify.exception(function () {
                    myVector = myAnimal.fillArrayHSTRING(intArray);
                }, TypeError, "myVector = myAnimal.fillArrayHSTRING(intArray);");

                logger.comment('intArray[7] = 45;');
                intArray[7] = 45;

                // Save the intArray to see lifetime after this function exit
                delegateFillArray.intArray = intArray;
            }

            var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
            var myAnimal = new Animals.Animal(1);

            myAnimal.passArray(myArray);
            delegateFillArray.arrayLength = myArray.length;
            myAnimal.callDelegateFillArray(delegateFillArray);

            verify(myArray, [1, 2, 3, 4, 5, 6, 7, 8, 9], "myArray");

            myResult = myAnimal.receiveArray();
            verifyVectorAndArrayItems("delegateFillArray.intArray", delegateFillArray.intArray, "myResult.outVector", myResult.outVector);
        }
    });

    runner.addTest({
        id: 5,
        desc: 'Delegate_ProjectOutArrayByRef_Array',
        pri: '0',
        test: function () {
            function delegateReceiveArray(animal) {
                logger.comment("*** delegateReceiveArray Delegate");
                delegateReceiveArray.myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
                return delegateReceiveArray.myArray;
            }

            var myAnimal = new Animals.Animal(1);
            myAnimal.callDelegateReceiveArray(delegateReceiveArray);

            verify(delegateReceiveArray.myArray, [1, 2, 3, 4, 5, 6, 7, 8, 9, 10], "delegateReceiveArray.myArray");
            myResult = myAnimal.receiveArray();
            verifyVectorAndArrayItems("myResult.outVector", myResult.outVector, "delegateReceiveArray.myArray", delegateReceiveArray.myArray);

            // Try changing the fields outside function - shouldnt affect either abi or existing vector or projected array
            delegateReceiveArray.myArray[5] = 1234;
            verify(myResult.value[5] != 1234, true, "myResult.value[5] != 1234");

            myResult = myAnimal.receiveArray();
            verify(myResult.value[5] != 1234, true, "myResult.value[5] != 1234");
        }
    });

    runner.addTest({
        id: 6,
        desc: 'Delegate_ProjectOutArrayByRef_ArrayProjection',
        pri: '0',
        test: function () {
            function delegateReceiveArray(animal) {
                logger.comment("*** delegateReceiveArray Delegate");

                delegateReceiveArray.myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
                animal.passArray(delegateReceiveArray.myArray);
                var myResult = animal.receiveArray();
                delegateReceiveArray.myArray[3] = 999;

                myResult.value[3] = 999;
                delegateReceiveArray.myArrayProjection = myResult.value;
                return delegateReceiveArray.myArrayProjection;
            }

            var myAnimal = new Animals.Animal(1);
            myAnimal.callDelegateReceiveArray(delegateReceiveArray);
            verifyVectorAndArrayItems("delegateReceiveArray.myArray", delegateReceiveArray.myArray, "delegateReceiveArray.myArrayProjection", delegateReceiveArray.myArrayProjection);

            myResult = myAnimal.receiveArray();
            verifyVectorAndArrayItems("myResult.outVector", myResult.outVector, "delegateReceiveArray.myArrayProjection", delegateReceiveArray.myArrayProjection);

            // Try changing the fields outside function - shouldnt affect either abi or existing vector or projected array
            delegateReceiveArray.myArrayProjection[5] = 1234;
            verify(myResult.value[5] != 1234, true, "myResult.value[5] != 1234");
            verify(delegateReceiveArray.myArray[5] != 1234, true, "delegateReceiveArray.myArray[5] != 1234");

            myResult = myAnimal.receiveArray();
            verify(myResult.value[5] != 1234, true, "myResult.value[5] != 1234");
        }
    });

    runner.addTest({
        id: 7,
        desc: 'Receive a big complex struct',
        pri: '0',
        test: function () {
            var bigComplexStructArray = Animals.Animal.getBigComplexStructArray();
            verify(bigComplexStructArray.length, 5, "bigComplexStructArray.length");
            for (var arrayIndex = 0; arrayIndex < 5; arrayIndex++) {
                verify(bigComplexStructArray[arrayIndex].toString(), "[object Animals.BigComplexStruct]", "bigComplexStructArray[" + arrayIndex + "].toString()");

                verify(bigComplexStructArray[arrayIndex].field0, arrayIndex, "bigComplexStructArray[" + arrayIndex + "].field0");

                verify(bigComplexStructArray[arrayIndex].field1.toString(), "[object Animals.PackedByte]", "bigComplexStructArray[" + arrayIndex + "].field1.toString()");
                verify(bigComplexStructArray[arrayIndex].field1.field0, arrayIndex + 50, "bigComplexStructArray[" + arrayIndex + "].field1.field0");

                verify(bigComplexStructArray[arrayIndex].field2, arrayIndex + 200, "bigComplexStructArray[" + arrayIndex + "].field2");

                verify(bigComplexStructArray[arrayIndex].field3.toString(), "[object Animals.PackedBoolean4]", "bigComplexStructArray[" + arrayIndex + "].field3.toString()");
                verify(bigComplexStructArray[arrayIndex].field3.field0, false, "bigComplexStructArray[" + arrayIndex + "].field3.field0");
                verify(bigComplexStructArray[arrayIndex].field3.field1, true, "bigComplexStructArray[" + arrayIndex + "].field3.field1");
                verify(bigComplexStructArray[arrayIndex].field3.field2, false, "bigComplexStructArray[" + arrayIndex + "].field3.field2");
                verify(bigComplexStructArray[arrayIndex].field3.field3, true, "bigComplexStructArray[" + arrayIndex + "].field3.field3");

                verify(bigComplexStructArray[arrayIndex].field4.toString(), "[object Animals.SmallComplexStruct]", "bigComplexStructArray[" + arrayIndex + "].field3.toString()");
                verify(bigComplexStructArray[arrayIndex].field4.field0, arrayIndex + 180, "bigComplexStructArray[" + arrayIndex + "].field4.field0");
                verify(bigComplexStructArray[arrayIndex].field4.field1.toString(), "[object Animals.PackedByte]", "bigComplexStructArray[" + arrayIndex + "].field4.field1.toString()");
                verify(bigComplexStructArray[arrayIndex].field4.field1.field0, arrayIndex + 150, "bigComplexStructArray[" + arrayIndex + "].field4.field1.field0");
                verify(bigComplexStructArray[arrayIndex].field4.field2, arrayIndex + 190, "bigComplexStructArray[" + arrayIndex + "].field4.field2");

                verify(bigComplexStructArray[arrayIndex].field5.toString(), "[object Animals.SmallComplexStruct]", "bigComplexStructArray[" + arrayIndex + "].field5.toString()");
                verify(bigComplexStructArray[arrayIndex].field5.field0, arrayIndex + 80, "bigComplexStructArray[" + arrayIndex + "].field5.field0");
                verify(bigComplexStructArray[arrayIndex].field5.field1.toString(), "[object Animals.PackedByte]", "bigComplexStructArray[" + arrayIndex + "].field5.field1.toString()");
                verify(bigComplexStructArray[arrayIndex].field5.field1.field0, arrayIndex + 50, "bigComplexStructArray[" + arrayIndex + "].field5.field1.field0");
                verify(bigComplexStructArray[arrayIndex].field5.field2, arrayIndex + 90, "bigComplexStructArray[" + arrayIndex + "].field5.field2");

                verify(bigComplexStructArray[arrayIndex].field6, arrayIndex + 7, "bigComplexStructArray[" + arrayIndex + "].field6");

                verify(bigComplexStructArray[arrayIndex].field7, arrayIndex + 2000, "bigComplexStructArray[" + arrayIndex + "].field7");
            }
        }
    });

    Loader42_FileName = 'Recycler Stress - Array.js';
})();

if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
