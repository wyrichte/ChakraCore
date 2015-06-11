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

    function verifyStringVectorAndArrayItems(myVectorString, myVector, myArrayString, myArray) {
        verify(myVector.length, myArray.length, "Number of array Items and Vector Items")

        for (var iIndex = 0; iIndex < myVector.length; iIndex++) {
            verify(myVector[iIndex].toString(), myArray[iIndex], myVectorString + '[' + iIndex + '].toString() and ' + myArrayString + '[' + iIndex + ']');
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
            var myArray = ['white', 'Red', 'Blue', 'Yellow', 'Pink', 'Black', 'Orange', 'Purple', 'Green'];
            var myAnimal = new Animals.Animal(1);

            var myVector = myAnimal.passArrayHSTRING(myArray);
            verifyVectorAndArrayItems("myVector", myVector, "myArray", myArray)

            myVector = myAnimal.fillArrayHSTRING(myArray);
            verifyVectorAndArrayItems("myVector", myVector, "myArray", myArray)

            var myResult = myAnimal.receiveArrayHSTRING();
            var expectedResult = [["value", "object"], ["outVector", "object"]];
            verifyResultObject("myResult", myResult, expectedResult, "object");
            verifyVectorAndArrayItems("myResult.value", myResult.value, "myResult.outVector", myResult.outVector);

            var myArrayProjection = myResult.value;

            // Check if we can marshal in the projected array as inout parameter:
            myVector = myAnimal.fillArrayHSTRING(myArrayProjection);
            verifyVectorAndArrayItems("myVector", myVector, "myArrayProjection", myArrayProjection)

            // Check if we can marshal in the projected array as in parameter:
            myArrayProjection[4] = 'Gold'
            myVector = myAnimal.passArrayHSTRING(myArrayProjection);
            verifyVectorAndArrayItems("myVector", myVector, "myArrayProjection", myArrayProjection)

            // Verify marshalling into different type throws exception - in param
            verify.exception(function () {
                myVector = myAnimal.passArray(myArrayProjection);
            }, TypeError, "myVector = myAnimal.passArray(myArrayProjection);");

            // Verify marshalling into different type throws exception - inout param
            verify.exception(function () {
                myVector = myAnimal.fillArray(myArrayProjection);
            }, TypeError, "myVector = myAnimal.fillArray(myArrayProjection);");
        }
    });

    runner.addTest({
        id: 2,
        desc: 'ProjectOutByRefArrayNull_Basic',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);

            // Null as array
            myResult = myAnimal.passArrayHSTRING(null);
            myResult = myAnimal.receiveArrayHSTRING();

            expectedResult = [["value", "object"], ["outVector", "object"]];
            verifyVectorAndArrayItems("myResult.value", myResult.value, "null", null);
        }
    });

    runner.addTest({
        id: 3,
        desc: 'Delegate_MarshalInArray_Basic',
        pri: '0',
        test: function () {
            function delegatePassArray(animal, stringArray) {
                logger.comment("*** delegatePassArray Delegate");

                var stringVector = delegatePassArray.outVector;
                verifyVectorAndArrayItems("stringArray", stringArray, "stringVector", stringVector)

                // Check if we can marshal in the projected array as inout parameter:
                myVector = myAnimal.fillArrayHSTRING(stringArray);
                verifyVectorAndArrayItems("myVector", myVector, "stringArray", stringArray)

                // Check if we can marshal in the projected array as in parameter:
                stringArray[4] = 'Gold'
                var myVector = animal.passArrayHSTRING(stringArray);
                verifyVectorAndArrayItems("myVector", myVector, "stringArray", stringArray)

                // Verify marshalling into different type throws exception - in param
                verify.exception(function () {
                    myVector = myAnimal.passArray(stringArray);
                }, TypeError, "myVector = myAnimal.passArray(stringArray);");

                // Verify marshalling into different type throws exception - inout param
                verify.exception(function () {
                    myVector = myAnimal.fillArray(stringArray);
                }, TypeError, "myVector = myAnimal.fillArray(stringArray);");
            }

            var myArray = ['white', 'Red', 'Blue', 'Yellow', 'Pink', 'Black', 'Orange', 'Purple', 'Green'];
            var myAnimal = new Animals.Animal(1);

            delegatePassArray.outVector = myAnimal.passArrayHSTRING(myArray);
            myAnimal.callDelegatePassArrayHSTRING(delegatePassArray);

            verify(myArray, ['white', 'Red', 'Blue', 'Yellow', 'Pink', 'Black', 'Orange', 'Purple', 'Green'], "myArray");
        }
    });

    runner.addTest({
        id: 4,
        desc: 'Delegate_ProjectOutArrayByValue',
        pri: '0',
        test: function () {
            function delegateFillArray(animal, stringArray) {
                logger.comment("*** delegateFillArray Delegate");

                // Check if we can marshal in the projected array as inout parameter:
                myVector = myAnimal.fillArrayHSTRING(stringArray);
                verifyVectorAndArrayItems("myVector", myVector, "stringArray", stringArray)

                // Check if we can marshal in the projected array as in parameter:
                stringArray[4] = 'Gold'
                var myVector = animal.passArrayHSTRING(stringArray);
                verifyVectorAndArrayItems("myVector", myVector, "stringArray", stringArray)

                // Verify marshalling into different type throws exception - in param
                verify.exception(function () {
                    myVector = myAnimal.passArray(stringArray);
                }, TypeError, "myVector = myAnimal.passArray(stringArray);");

                // Verify marshalling into different type throws exception - inout param
                verify.exception(function () {
                    myVector = myAnimal.fillArray(stringArray);
                }, TypeError, "myVector = myAnimal.fillArray(stringArray);");

                stringArray[7] = "Voilet";

                // Save the stringArray to see lifetime after this function exit
                delegateFillArray.stringArray = stringArray;
            }

            var myArray = ['white', 'Red', 'Blue', 'Yellow', 'Pink', 'Black', 'Orange', 'Purple', 'Green'];
            var myAnimal = new Animals.Animal(1);

            myAnimal.passArrayHSTRING(myArray);
            delegateFillArray.arrayLength = myArray.length;
            myAnimal.callDelegateFillArrayHSTRING(delegateFillArray);
            verify(myArray, ['white', 'Red', 'Blue', 'Yellow', 'Pink', 'Black', 'Orange', 'Purple', 'Green'], "myArray");

            myResult = myAnimal.receiveArrayHSTRING();
            verifyVectorAndArrayItems("delegateFillArray.stringArray", delegateFillArray.stringArray, "myResult.outVector", myResult.outVector);

            // Try changing the fields outside function - shouldnt affect either abi or existing vector or projected array
            delegateFillArray.stringArray[5] = "Brown";
            verify(myResult.value[5] != 'Brown', true, "myResult.value[5] != 'Brown'");
            myResult = myAnimal.receiveArrayHSTRING();
            verify(myResult.value[5] != 'Brown', true, "myResult.value[5] != 'Brown'");
        }
    });

    runner.addTest({
        id: 5,
        desc: 'Delegate_ProjectOutArrayByRef_Array',
        pri: '0',
        test: function () {
            function delegateReceiveArray(animal) {
                logger.comment("*** delegateReceiveArray Delegate");
                delegateReceiveArray.myArray = ['white', 'Red', 'Blue', 'Yellow', 'Pink', 'Black', 'Orange', 'Purple', 'Green', 'Magenta'];
                return delegateReceiveArray.myArray;
            }

            var myAnimal = new Animals.Animal(1);
            myAnimal.callDelegateReceiveArrayHSTRING(delegateReceiveArray);
            verify(delegateReceiveArray.myArray, ['white', 'Red', 'Blue', 'Yellow', 'Pink', 'Black', 'Orange', 'Purple', 'Green', 'Magenta'], "delegateReceiveArray.myArray");

            myResult = myAnimal.receiveArrayHSTRING();
            verifyVectorAndArrayItems("myResult.outVector", myResult.outVector, "delegateReceiveArray.myArray", delegateReceiveArray.myArray);

            // Try changing the fields outside function - shouldnt affect either abi or existing vector or projected array
            delegateReceiveArray.myArray[5] = 'Brown';
            verify(myResult.value[5] != 'Brown', true, "myResult.value[5] != 'Brown'");

            myResult = myAnimal.receiveArrayHSTRING();
            verify(myResult.value[5] != 'Brown', true, "myResult.value[5] != 'Brown'");
        }
    });

    runner.addTest({
        id: 6,
        desc: 'Delegate_ProjectOutArrayByRef_ArrayProjection',
        pri: '0',
        test: function () {
            function delegateReceiveArray(animal) {
                logger.comment("*** delegateReceiveArray Delegate");

                delegateReceiveArray.myArray = ['white', 'Red', 'Blue', 'Yellow', 'Pink', 'Black', 'Orange', 'Purple', 'Green'];
                animal.passArrayHSTRING(delegateReceiveArray.myArray);

                var myResult = animal.receiveArrayHSTRING();
                delegateReceiveArray.myArray[3] = "Brown";

                myResult.value[3] = "Brown";
                delegateReceiveArray.myArrayProjection = myResult.value;

                return delegateReceiveArray.myArrayProjection;
            }

            var myAnimal = new Animals.Animal(1);
            myAnimal.callDelegateReceiveArrayHSTRING(delegateReceiveArray);
            verifyVectorAndArrayItems("delegateReceiveArray.myArray", delegateReceiveArray.myArray, "delegateReceiveArray.myArrayProjection", delegateReceiveArray.myArrayProjection);

            myResult = myAnimal.receiveArrayHSTRING();
            verifyVectorAndArrayItems("myResult.outVector", myResult.outVector, "delegateReceiveArray.myArrayProjection", delegateReceiveArray.myArrayProjection);

            // Try changing the fields outside function - shouldnt affect either abi or existing vector or projected array
            delegateReceiveArray.myArrayProjection[5] = 'Silver';
            verify(myResult.value[5] != 'Silver', true, "myResult.value[5] != 'Silver'");
            verify(delegateReceiveArray.myArray[5] != 'Silver', true, "delegateReceiveArray.myArray[5] != 'Silver'");

            myResult = myAnimal.receiveArrayHSTRING();
            verify(myResult.value[5] != 'Silver', true, "myResult.value[5] != 'Silver'");
        }
    });

    Loader42_FileName = 'Recycler Stress - ArrayString.js';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
