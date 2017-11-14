if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
Loader42_FileName = 'Array Tests';

function testPassFillAndReceiveArray() {
    logger.comment("Create Animal");
    var myAnimal = new Animals.Animal(1);

    logger.comment("Call PassArray");
    var myArray = [1, 2, 3, 4];
    myAnimal.passArray(myArray);
    verify(myArray.length, 4, "myArray.length");
    for (var index = 0; index < 4; index++) {
        verify(myArray[index], index + 1, "myArray[" + index + "]");
    }

    logger.comment("Call FillArray");
    myArray = new Array(2);
    myAnimal.fillArray(myArray);
    verify(myArray.length, 2, "myArray.length");
    for (var index = 0; index < 2; index++) {
        verify(myArray[index], index + 1, "myArray[" + index + "]");
    }

    logger.comment("Call ReceiveArray");
    myArray = myAnimal.receiveArray().value;
    verify(myArray.length, 4, "myArray.length");
    for (var index = 0; index < 4; index++) {
        verify(myArray[index], index + 1, "myArray[" + index + "]");
    }

}

function testPassFillAndReceiveArrayFinalizable() {
    logger.comment("Create Animal");
    var myAnimal = new Animals.Animal(1);

    logger.comment("Call PassArray");
    var myArray = ['Friday', 'Saturday', 'Sunday'];
    myAnimal.passArrayHSTRING(myArray);

    verify(myArray.length, 3, "myArray.length");
    verify(myArray[0], 'Friday', "myArray[0]");
    verify(myArray[1], 'Saturday', "myArray[1]");
    verify(myArray[2], 'Sunday', "myArray[2]");

    logger.comment("Call FillArray");
    myArray = new Array(2);
    myAnimal.fillArrayHSTRING(myArray);

    verify(myArray.length, 2, "myArray.length");
    verify(myArray[0], 'Friday', "myArray[0]");
    verify(myArray[1], 'Saturday', "myArray[1]");

    logger.comment("Call ReceiveArray");
    myArray = myAnimal.receiveArrayHSTRING().value;
    verify(myArray.length, 3, "myArray.length");
    verify(myArray[0], 'Friday', "myArray[0]");
    verify(myArray[1], 'Saturday', "myArray[1]");
    verify(myArray[2], 'Sunday', "myArray[2]");
}

function testPassFillTypedArray() {
    logger.comment("Create Animal");
    var myAnimal = new Animals.Animal(1);

    logger.comment("Call PassArray");
    myAnimal.passArray([1, 2, 3, 4]);

    logger.comment("Call ReceiveArray");
    var myArray = myAnimal.receiveArray().value;

    for (var index = 0; index < 4; index++) {
        verify(myArray[index], index + 1, "myArray[" + index + "]");
        myArray[index] = myArray[index] * 20;
        verify(myArray[index], (index + 1) * 20, "myArray[" + index + "]");
    }

    logger.comment("PassTypedArray");
    myAnimal.passArray(myArray);
    for (var index = 0; index < 4; index++) {
        verify(myArray[index], (index + 1) * 20, "myArray[" + index + "]");
    }

    logger.comment("FillTypedArray");
    myAnimal.fillArray(myArray);
    for (var index = 0; index < 4; index++) {
        verify(myArray[index], (index + 1) * 20, "myArray[" + index + "]");
    }

    logger.comment("PassTypedArray to different type");
    verify.exception(function () {
        myAnimal.passArrayHSTRING(myArray);
    }, TypeError, "myAnimal.passArrayHSTRING(myArray)");

    logger.comment("FillTypedArray to different type");
    verify.exception(function () {
        myAnimal.fillArrayHSTRING(myArray);
    }, TypeError, "myAnimal.fillArrayHSTRING(myArray)");
}

function testPassFillCanvasPixelArray() {
    logger.comment("Create CanvasPixelArray");
    var canvasPixelArray = WScript.CreateCanvasPixelArray([1, 2, 3, 4]);

    logger.comment("Call Pass CanvasPixelArray");
    var vector = Animals.Animal.passUInt8Array(canvasPixelArray);
    verify(canvasPixelArray.length, vector.length, "canvasPixelArray.length");
    for (var index = 0; index < 4; index++) {
        verify(canvasPixelArray[index], vector[index], "canvasPixelArray[" + index + "]");
    }

    logger.comment("Fill CanvasPixelArray");
    Animals.Animal.fillUInt8Array(canvasPixelArray, [11, 22, 33, 44, 55]);
    verify(canvasPixelArray.length, 4, "canvasPixelArray.length");
    for (var index = 0; index < 4; index++) {
        verify(canvasPixelArray[index], (index + 1) * 11, "canvasPixelArray[" + index + "]");
    }
}

function testPassFillCanvasPixelArrayAsIntArray() {
    logger.comment("Create CanvasPixelArray");
    var canvasPixelArray = WScript.CreateCanvasPixelArray([1, 2, 3, 4]);

    logger.comment("Create Animal");
    var myAnimal = new Animals.Animal(1);

    logger.comment("Call PassArray");
    verify.exception(function () {
        var vector = myAnimal.passArray(canvasPixelArray);
    }, TypeError, "myAnimal.passArray(canvasPixelArray)");

    logger.comment("Call PassArray");
    myAnimal.passArray([11, 22, 33, 44, 55]);

    logger.comment("Fill CanvasPixelArray");
    verify.exception(function () {
        myAnimal.fillArray(canvasPixelArray);
    }, TypeError, "myAnimal.fillArray(canvasPixelArray)");
}

function testPassFillArrayProjection() {
    logger.comment("Create Animal");
    var myAnimal = new Animals.Animal(1);

    logger.comment("Call PassArray");
    var myArray = ['Friday', 'Saturday', 'Sunday'];
    myAnimal.passArrayHSTRING(myArray);

    logger.comment("Call ReceiveArray");
    myArray = myAnimal.receiveArrayHSTRING().value;

    logger.comment("PassArrayProjection");
    myAnimal.passArrayHSTRING(myArray);
    verify(myArray.length, 3, "myArray.length");
    verify(myArray[0], 'Friday', "myArray[0]");
    verify(myArray[1], 'Saturday', "myArray[1]");
    verify(myArray[2], 'Sunday', "myArray[2]");

    logger.comment("FillArrayProjection");
    myAnimal.fillArrayHSTRING(myArray);
    verify(myArray.length, 3, "myArray.length");
    verify(myArray[0], 'Friday', "myArray[0]");
    verify(myArray[1], 'Saturday', "myArray[1]");
    verify(myArray[2], 'Sunday', "myArray[2]");

    logger.comment("PassArrayProjection to different type");
    verify.exception(function () {
        myAnimal.passArray(myArray);
    }, TypeError, "myAnimal.passArray(myArray)");

    logger.comment("FillArrayProjection to different type");
    verify.exception(function () {
        myAnimal.fillArray(myArray);
    }, TypeError, "myAnimal.fillArray(myArray)");
}

function delegatePassFillAndReceiveArray() {
    logger.comment("Create Animal");
    var myAnimal = new Animals.Animal(1);

    logger.comment("Call PassArray");
    myAnimal.passArray([1, 2, 3, 4]);

    function delegatePassArray(animal, myArray) {
        for (var index = 0; index < 4; index++) {
            verify(myArray[index], index + 1, "myArray[" + index + "]");
        }
    }
    logger.comment("Call Delegate PassArray");
    myAnimal.callDelegatePassArray(delegatePassArray);

    function delegateFillArray(animal, myArray) {
        for (var index = 0; index < 4; index++) {
            verify(myArray[index], 0, "myArray[" + index + "]");
        }
    }
    logger.comment("Call Delegate FillArray");
    myAnimal.callDelegateFillArray(delegateFillArray);

    function delegateReceiveJsArray(animal) {
        logger.comment("Returning Js Array");
        return [11, 22, 33];
    }
    logger.comment("Call Delegate ReceiveJsArray");
    myAnimal.callDelegateReceiveArray(delegateReceiveJsArray);

    function delegateReceiveTypedArray(animal) {
        logger.comment("Returning TypedArray");
        return animal.receiveArray().value;
    }
    logger.comment("Call Delegate ReceiveTypedArray");
    myAnimal.callDelegateReceiveArray(delegateReceiveTypedArray);

    function delegateReceiveArrayProjection(animal) {
        logger.comment("Returning ArrayProjection");
        animal.passArrayHSTRING([1, 2, 3]);
        return animal.receiveArrayHSTRING().value;
    }
    logger.comment("Call Delegate ReceiveArrayProjection");
    myAnimal.callDelegateReceiveArray(delegateReceiveArrayProjection);
}

function delegatePassFillAndReceiveArrayFinalizable() {
    logger.comment("Create Animal");
    var myAnimal = new Animals.Animal(1);

    logger.comment("Call PassArray");
    myAnimal.passArrayHSTRING(['Friday', 'Saturday', 'Sunday']);

    function delegatePassArray(animal, myArray) {
        verify(myArray.length, 3, "myArray.length");
        verify(myArray[0], 'Friday', "myArray[0]");
        verify(myArray[1], 'Saturday', "myArray[1]");
        verify(myArray[2], 'Sunday', "myArray[2]");
    }
    logger.comment("Call Delegate PassArray");
    myAnimal.callDelegatePassArrayHSTRING(delegatePassArray);

    function delegateFillArray(animal, myArray) {
        verify(myArray.length, 3, "myArray.length");
        verify(myArray[0], '', "myArray[0]");
        verify(myArray[1], '', "myArray[1]");
        verify(myArray[2], '', "myArray[2]");
    }
    logger.comment("Call Delegate FillArray");
    myAnimal.callDelegateFillArrayHSTRING(delegateFillArray);

    function delegateReceiveJsArray(animal) {
        logger.comment("Returning Js Array");
        return ['11', '22', '33'];
    }
    logger.comment("Call Delegate ReceiveJsArray");
    myAnimal.callDelegateReceiveArrayHSTRING(delegateReceiveJsArray);

    function delegateReceiveTypedArray(animal) {
        logger.comment("Returning TypedArray");
        animal.passArray([1, 2, 3]);
        return animal.receiveArray().value;
    }
    logger.comment("Call Delegate ReceiveTypedArray");
    myAnimal.callDelegateReceiveArrayHSTRING(delegateReceiveTypedArray);

    function delegateReceiveArrayProjection(animal) {
        logger.comment("Returning ArrayProjection");
        return animal.receiveArrayHSTRING().value;
    }
    logger.comment("Call Delegate ReceiveArrayProjection");
    myAnimal.callDelegateReceiveArrayHSTRING(delegateReceiveArrayProjection);
}

function testPassArrayOfBigStructs() {
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

function testPassFillAndReceiveArrayNull() {
    logger.comment("Create Animal");
    var myAnimal = new Animals.Animal(1);

    logger.comment("Call PassArray");
    var myVector = myAnimal.passArray(null);
    verify(myVector.length, 0, "myVector.length");

    logger.comment("Call FillArray");
    myAnimal.fillArray(null);
    verify(myVector.length, 0, "myVector.length");

    logger.comment("Call ReceiveArray");
    myArray = myAnimal.receiveArray().value;
    verify(myArray, null, "myArray");
}


function testPassAndFillArray0Elements() {
    logger.comment("Create Animal");
    var myAnimal = new Animals.Animal(1);

    var myArray = new Array();
    verify(myArray.length, 0, "myArray.length");

    logger.comment("Call PassArray");
    var myVector = myAnimal.passArray(myArray);
    verify(myVector.length, 0, "myVector.length");

    logger.comment("Call FillArray");
    myAnimal.fillArray(myArray);
    verify(myVector.length, 0, "myVector.length");
}

function delegatePassFillAndReceiveArrayNull() {
    logger.comment("Create Animal");
    var myAnimal = new Animals.Animal(1);

    function delegatePassArray(animal, myArray) {
        verify(myArray, null, "myArray");
    }
    logger.comment("Call Delegate PassArray");
    myAnimal.callDelegatePassArray(delegatePassArray);

    function delegateFillArray(animal, myArray) {
        verify(myArray, null, "myArray");
    }
    logger.comment("Call Delegate FillArray");
    myAnimal.callDelegateFillArray(delegateFillArray);

    function delegateReceiveJsArray(animal) {
        logger.comment("Returning null");
        return null;
    }
    logger.comment("Call Delegate ReceiveJsArray");
    myAnimal.callDelegateReceiveArray(delegateReceiveJsArray);
    verify(myAnimal.receiveArray().value, null, "myAnimal.receiveArray().value");
}

function delegateReceiveArray0Elelements() {
    logger.comment("Create Animal");
    var myAnimal = new Animals.Animal(1);

    function delegateReceiveJsArray(animal) {
        logger.comment("Returning JsArray of 0 elements");
        return new Array();
    }
    logger.comment("Call Delegate ReceiveJsArray");
    myAnimal.callDelegateReceiveArray(delegateReceiveJsArray);

    verify(myAnimal.receiveArray().value, null, "myAnimal.receiveArray().value");
}

function verifyArrayItems(myArrayString, myArray, myExpectedArray) {
    verify(myArray.length, myExpectedArray.length, myArrayString + ".length")

    for (var iIndex = 0; iIndex < myExpectedArray.length; iIndex++) {
        verify(myArray[iIndex], myExpectedArray[iIndex], myArrayString + '[' + iIndex + ']');
    }
}

function dumpArrayItems(myArrayString, myArray) {
    if (myArray === null) {
        logger.comment(myArrayString + " : null");
        return;
    }

    var arrayPrint = myArrayString + " : [ length = " + myArray.length + " ] : [ contents = ";
    for (var i = 0; i < myArray.length; i++) {
        arrayPrint = arrayPrint + " " + myArray[i];
    }
    arrayPrint = arrayPrint + " ]";
    logger.comment(arrayPrint);
}

function verifyAllZeroItems(myArrayString, myArray, expectedLength, expectedNull) {
    if (expectedNull === true) {
        verify(myArray, null, myArrayString);
        return;
    }

    verify(myArray.length, expectedLength, myArrayString + ".length")
    for (var iIndex = 0; iIndex < expectedLength; iIndex++) {
        verify(myArray[iIndex], 0, myArrayString + '[' + iIndex + ']');
    }
}

function verifyAllZeroItemsHSTRING(myArrayString, myArray, expectedLength, expectedNull) {
    if (expectedNull === true) {
        verify(myArray, null, myArrayString);
        return;
    }

    verify(myArray.length, expectedLength, myArrayString + ".length")
    for (var iIndex = 0; iIndex < expectedLength; iIndex++) {
        verify(myArray[iIndex], "", myArrayString + '[' + iIndex + ']');
    }
}

function testPassArrayWithLengthAttribute() {
    var myAnimal = new Animals.Animal(1);

    logger.comment("Call PassArray with 4 elements on JsArray of 9 elements");
    myAnimal.passArrayWithInLength([1, 2, 3, 4, 5, 6, 7, 8, 9], 4);
    var resultArray = myAnimal.pureReceiveArray();
    verifyArrayItems("resultArray", resultArray, [1, 2, 3, 4, 0, 0, 0, 0, 0]);

    logger.comment("Call PassArray with 4 elements on Int32Array of 9 elements");
    myAnimal.passArrayWithInLength(new Int32Array([1, 2, 3, 4, 5, 6, 7, 8, 9]), 4);
    var resultArray = myAnimal.pureReceiveArray();
    verifyArrayItems("resultArray", resultArray, [1, 2, 3, 4, 0, 0, 0, 0, 0]);

    logger.comment("Call PassArray with 0 elements on null");
    myAnimal.passArrayWithInLength(null, 0);
    var resultArray = myAnimal.pureReceiveArray();
    verify(resultArray, null, "resultArray");

    logger.comment("Call PassArray JsArray");
    var elementsRead = myAnimal.passArrayWithOutLength([1, 2, 3, 4, 5, 6, 7, 8, 9]);
    var resultArray = myAnimal.pureReceiveArray();
    verify(elementsRead, 9, "elementsRead");
    verifyArrayItems("resultArray", resultArray, [1, 2, 3, 4, 5, 6, 7, 8, 9]);

    logger.comment("Call PassArray Int32Array");
    elementsRead = myAnimal.passArrayWithOutLength(new Int32Array([11, 22, 33, 44]));
    resultArray = myAnimal.pureReceiveArray();
    verify(elementsRead, 4, "elementsRead");
    verifyArrayItems("resultArray", resultArray, [11, 22, 33, 44]);

    logger.comment("Call PassArray with 0 elements on null");
    elementsRead = myAnimal.passArrayWithOutLength(null);
    var resultArray = myAnimal.pureReceiveArray();
    verify(elementsRead, 0, "elementsRead");
    verify(resultArray, null, "resultArray");
}

function testFillArrayWithLengthAttribute() {
    var myAnimal = new Animals.Animal(1);
    myAnimal.purePassArray([12, 23, 34, 45]);

    logger.comment("Call FillArray with 2 elements on JsArray of 4 elements");
    var myArray = [1, 2, 3, 4];
    myAnimal.fillArrayWithInLength(myArray, 2);
    verifyArrayItems("myArray", myArray, [12, 23, 0, 0]);

    logger.comment("Call FillArray with 3 elements on Int32Array of 5 elements");
    var myArray = new Int32Array([1, 2, 3, 4, 5]);
    myAnimal.fillArrayWithInLength(myArray, 3);
    verifyArrayItems("myArray", myArray, [12, 23, 34, 0, 0]);

    logger.comment("Call FillArray with 0 elements on null");
    myAnimal.fillArrayWithInLength(null, 0);

    logger.comment("Call FillArray with 4 elements on JsArray of 9 elements");
    var myArray = [1, 2, 3, 4, 5, 6, 7, 8, 9];
    var elementsRead = myAnimal.fillArrayWithOutLength(myArray);
    verify(elementsRead, 4, "elementsRead");
    verifyArrayItems("myArray", myArray, [12, 23, 34, 45, 0, 0, 0, 0, 0]);

    logger.comment("Call FillArray with 2 elements on JsArray of 2 elements");
    var myArray = [1, 2];
    var elementsRead = myAnimal.fillArrayWithOutLength(myArray);
    verify(elementsRead, 2, "elementsRead");
    verifyArrayItems("myArray", myArray, [12, 23]);

    logger.comment("Call FillArray with 4 elements on Int32Array of 9 elements");
    var myArray = new Int32Array([1, 2, 3, 4, 5, 6, 7, 8, 9]);
    elementsRead = myAnimal.fillArrayWithOutLength(myArray);
    verify(elementsRead, 4, "elementsRead");
    verifyArrayItems("myArray", myArray, [12, 23, 34, 45, 0, 0, 0, 0, 0]);

    logger.comment("Call FillArray with 2 elements on Int32Array of 2 elements");
    var myArray = new Int32Array([1, 2]);
    var elementsRead = myAnimal.fillArrayWithOutLength(myArray);
    verify(elementsRead, 2, "elementsRead");
    verifyArrayItems("myArray", myArray, [12, 23]);

    logger.comment("Call FillArray with 0 elements on null");
    elementsRead = myAnimal.fillArrayWithOutLength(null);
    verify(elementsRead, 0, "elementsRead");
}

function testReceiveArrayWithLengthAttribute() {
    var myAnimal = new Animals.Animal(1);
    myAnimal.purePassArray([12, 23, 34, 45]);

    logger.comment("Call ReceiveArray with 2 elements");
    var myArray = myAnimal.receiveArrayWithInLength(2);
    verifyArrayItems("myArray", myArray, [12, 23, 0, 0]);

    logger.comment("Call ReceiveArray with 0 elements");
    myAnimal.purePassArray([]);
    var myArray = myAnimal.receiveArrayWithInLength(0);
    verify(myArray, null, "myArray");

    logger.comment("Call ReceiveArray with size = 4  length = 2");
    myAnimal.passArrayWithInLength([12, 23, 34, 45], 2);
    var result = myAnimal.receiveArrayWithOutLength();
    verify(result.lengthValue, 2, "elementsRead");
    verifyArrayItems("myArray", result.value, [12, 23, 0, 0]);

    logger.comment("Call ReceiveArray with size = 0  length = 0");
    myAnimal.purePassArray([]);
    var result = myAnimal.receiveArrayWithOutLength();
    verify(result.lengthValue, 0, "elementsRead");
    verify(result.value, null, "myArray");
}

function testDelegatePassArrayWithLengthAttribute() {
    var actualLength = 0;
    var actualArray = 0

    function delegatePassArrayWithInLength(arrayInDelegate, lengthAttribute) {
        logger.comment("*** delegatePassArrayWithInLength - Invoke ***");

        dumpArrayItems("arrayInDelegate", arrayInDelegate);
        logger.comment("lengthAttribute : " + lengthAttribute);

        actualLength = lengthAttribute;
        actualArray = arrayInDelegate;

        logger.comment("*** delegatePassArrayWithInLength - Exit ***");
    }

    logger.comment("Call PassArray with size = 4  length = 2");
    var myAnimal = new Animals.Animal(1);
    myAnimal.passArrayWithInLength([12, 23, 34, 45], 2);
    myAnimal.callDelegatePassArrayWithInLength(delegatePassArrayWithInLength);
    verify(actualLength, 2, "elementsRead");
    verifyArrayItems("myArray", actualArray, [12, 23, 0, 0]);

    var actualArray = 0
    var sendLength;
    function delegatePassArrayWithOutLength(arrayInDelegate) {
        logger.comment("*** delegatePassArrayWithOutLength - Invoke ***");
        dumpArrayItems("arrayInDelegate", arrayInDelegate);

        actualArray = arrayInDelegate;

        logger.comment("*** delegatePassArrayWithOutLength - Exit ***");
        return sendLength;
    }

    logger.comment("Call PassArray with size = 4  length = 2");
    var myAnimal = new Animals.Animal(1);
    myAnimal.passArrayWithInLength([12, 23, 34, 45], 2);
    sendLength = 2;
    myAnimal.callDelegatePassArrayWithOutLength(delegatePassArrayWithOutLength);
    verifyArrayItems("myArray", actualArray, [12, 23, 0, 0]);
}

function testDelegateFillArrayWithLengthAttribute() {
    var actualArray = 0
    var actualLength = 0;
    var excpectedNull = false;
    function delegateFillArrayWithInLength(arrayInDelegate, lengthAttribute) {
        logger.comment("*** delegateFillArrayWithInLength - Invoke ***");

        verifyAllZeroItems("arrayInDelegate", arrayInDelegate, excpectedNull ? 0 : actualArray.length, excpectedNull);
        verify(lengthAttribute, actualLength, "lengthAttribute");

        for (var i = 0; i < actualLength; i++) {
            arrayInDelegate[i] = actualArray[i];
        }

        logger.comment("*** delegateFillArrayWithInLength - Exit ***");
    }

    logger.comment("Call FillArray with size = 4  length = 2");
    var myAnimal = new Animals.Animal(1);
    actualArray = [12, 23, 34, 45];
    actualLength = 2;
    myAnimal.passArrayWithInLength(actualArray, actualLength);
    myAnimal.callDelegateFillArrayWithInLength(delegateFillArrayWithInLength);
    var resultArray = myAnimal.pureReceiveArray();
    verifyArrayItems("resultArray", resultArray, [12, 23, 0, 0]);

    var actualArray = 0
    var actualLength = 0;
    var excpectedNull = false;
    function delegateFillArrayWithOutLength(arrayInDelegate) {
        logger.comment("*** delegateFillArrayWithOutLength - Invoke ***");

        verifyAllZeroItems("arrayInDelegate", arrayInDelegate, excpectedNull ? 0 : actualArray.length, excpectedNull);
        for (var i = 0; i < actualLength; i++) {
            arrayInDelegate[i] = actualArray[i];
        }
        logger.comment("returnLength", actualLength);

        logger.comment("*** delegateFillArrayWithOutLength - Exit ***");

        return actualLength;
    }

    var myAnimal = new Animals.Animal(1);
    myAnimal.purePassArray([11, 22, 33, 44]);

    logger.comment("Call FillArray with size = 4  length = 2");
    actualArray = [12, 23, 34, 45];
    actualLength = 2;
    myAnimal.callDelegateFillArrayWithOutLength(delegateFillArrayWithOutLength);
    var resultArray = myAnimal.pureReceiveArray();
    verifyArrayItems("resultArray", resultArray, [12, 23, 0, 0]);
}

function testDelegateReceiveArrayWithLengthAttribute() {
    var actualArray = 0
    var actualLength = 0;
    function delegateReceiveArrayWithInLength(lengthAttribute) {
        logger.comment("*** delegateReceiveArrayWithInLength - Invoke ***");

        verify(lengthAttribute, actualLength, "lengthAttribute");
        dumpArrayItems("returnArray", actualArray);
        logger.comment("*** delegateReceiveArrayWithInLength - Exit ***");
        return actualArray;
    }

    logger.comment("Call ReceiveArray with size = 4  length = 2");
    var myAnimal = new Animals.Animal(1);
    actualArray = [12, 23, 34, 45];
    actualLength = 2;
    myAnimal.passArrayWithInLength([99, 88, 77, 66], actualLength);
    myAnimal.callDelegateReceiveArrayWithInLength(delegateReceiveArrayWithInLength);
    var resultArray = myAnimal.pureReceiveArray();
    verifyArrayItems("resultArray", resultArray, [12, 23, 0, 0]);

    var actualArray = 0
    var actualLength = 0;
    function delegateReceiveArrayWithOutLength() {
        logger.comment("*** delegateReceiveArrayWithOutLength - Invoke ***");

        dumpArrayItems("returnArray", actualArray);
        logger.comment("returnLength", actualLength);

        logger.comment("*** delegateReceiveArrayWithOutLength - Exit ***");

        var outVar = new Object();
        outVar.value = actualArray;
        outVar.lengthValue = actualLength;

        return outVar;
    }

    logger.comment("Call ReceiveArray with size = 4  length = 2");
    var myAnimal = new Animals.Animal(1);
    actualArray = [12, 23, 34, 45];
    actualLength = 2;
    myAnimal.callDelegateReceiveArrayWithOutLength(delegateReceiveArrayWithOutLength);
    var resultArray = myAnimal.pureReceiveArray();
    verifyArrayItems("resultArray", resultArray, [12, 23, 0, 0]);
}

function testPassArrayWithLengthAttributeHSTRING() {
    var myAnimal = new Animals.Animal(1);

    logger.comment("Call PassArray with 2 elements on JsArray of 4 elements");
    myAnimal.passArrayWithInLengthHSTRING(["Once", "Upon", "a", "Time"], 2);
    var resultArray = myAnimal.receiveArrayHSTRING().value;
    verifyArrayItems("resultArray", resultArray, ["Once", "Upon", "", ""]);

    logger.comment("Call PassArray with 2 elements on StringArray of 4 elements");
    var myArray = resultArray;
    myAnimal.passArrayWithInLengthHSTRING(myArray, 2);
    var resultArray = myAnimal.receiveArrayHSTRING().value;
    verifyArrayItems("resultArray", resultArray, ["Once", "Upon", "", ""]);

    logger.comment("Call PassArray with 0 elements on null");
    myAnimal.passArrayWithInLengthHSTRING(null, 0);
    var resultArray = myAnimal.receiveArrayHSTRING().value;
    verify(resultArray, null, "resultArray");

    logger.comment("Call PassArray JsArray");
    var elementsRead = myAnimal.passArrayWithOutLengthHSTRING(["Once", "Upon", "a", "Time"]);
    var resultArray = myAnimal.receiveArrayHSTRING().value;
    verify(elementsRead, 4, "elementsRead");
    verifyArrayItems("resultArray", resultArray, ["Once", "Upon", "a", "Time"]);

    logger.comment("Call PassArray StringArray");
    elementsRead = myAnimal.passArrayWithOutLengthHSTRING(resultArray);
    resultArray = myAnimal.receiveArrayHSTRING().value;
    verify(elementsRead, 4, "elementsRead");
    verifyArrayItems("resultArray", resultArray, ["Once", "Upon", "a", "Time"]);

    logger.comment("Call PassArray with 0 elements on null");
    elementsRead = myAnimal.passArrayWithOutLengthHSTRING(null);
    var resultArray = myAnimal.receiveArrayHSTRING().value;
    verify(elementsRead, 0, "elementsRead");
    verify(resultArray, null, "resultArray");
}

function testFillArrayWithLengthAttributeHSTRING() {
    var myAnimal = new Animals.Animal(1);
    myAnimal.passArrayHSTRING(["Mr. Gold", "Regina", "Mary Margaret", "David Nolan"]);

    logger.comment("Call FillArray with 2 elements on JsArray of 4 elements");
    var myArray = ["Rumplestiltskin", "The Evil Queen", "Snow White", "Prince Charming"];
    myAnimal.fillArrayWithInLengthHSTRING(myArray, 2);
    verifyArrayItems("myArray", myArray, ["Mr. Gold", "Regina", "", ""]);

    logger.comment("Call FillArray with 2 elements on StringArray of 4 elements");
    var myArray = myAnimal.receiveArrayHSTRING().value;
    myAnimal.fillArrayWithInLengthHSTRING(myArray, 2);
    verifyArrayItems("myArray", myArray, ["Mr. Gold", "Regina", "", ""]);

    logger.comment("Call FillArray with 0 elements on null");
    myAnimal.fillArrayWithInLengthHSTRING(null, 0);

    myAnimal.passArrayHSTRING(["Rumplestiltskin", "The Evil Queen", "Snow White", "Prince Charming"]);

    logger.comment("Call FillArray with 4 elements on JsArray of 7 elements");
    var myArray = ["Mr. Gold", "Regina", "Mary Margaret", "David Nolan", "Emma Swan", "Henry Mills", "Sheriff Graham"];
    var elementsRead = myAnimal.fillArrayWithOutLengthHSTRING(myArray);
    verify(elementsRead, 4, "elementsRead");
    verifyArrayItems("myArray", myArray, ["Rumplestiltskin", "The Evil Queen", "Snow White", "Prince Charming", "", "", ""]);

    logger.comment("Call FillArray with 2 elements on JsArray of 2 elements");
    var myArray = ["Mr. Gold", "Regina"];
    var elementsRead = myAnimal.fillArrayWithOutLengthHSTRING(myArray);
    verify(elementsRead, 2, "elementsRead");
    verifyArrayItems("myArray", myArray, ["Rumplestiltskin", "The Evil Queen"]);

    logger.comment("Call FillArray with 4 elements on StringArray of 4 elements");
    var myArray = myAnimal.receiveArrayHSTRING().value;
    myArray[0] = "Mr. Gold";
    myArray[1] = "Regina";
    myArray[2] = "Mary Margaret";
    myArray[3] = "David Nolan";
    elementsRead = myAnimal.fillArrayWithOutLengthHSTRING(myArray);
    verify(elementsRead, 4, "elementsRead");
    verifyArrayItems("myArray", myArray, ["Rumplestiltskin", "The Evil Queen", "Snow White", "Prince Charming"]);

    logger.comment("Call FillArray with 4 elements on StringArray of 7 elements");
    myAnimal.passArrayHSTRING(["Mr. Gold", "Regina", "Mary Margaret", "David Nolan", "Emma Swan", "Henry Mills", "Sheriff Graham"]);
    myArray = myAnimal.receiveArrayHSTRING().value;
    myAnimal.passArrayHSTRING(["Rumplestiltskin", "The Evil Queen", "Snow White", "Prince Charming"]);
    var elementsRead = myAnimal.fillArrayWithOutLengthHSTRING(myArray);
    verify(elementsRead, 4, "elementsRead");
    verifyArrayItems("myArray", myArray, ["Rumplestiltskin", "The Evil Queen", "Snow White", "Prince Charming", "", "", ""]);

    logger.comment("Call FillArray with 0 elements on null");
    elementsRead = myAnimal.fillArrayWithOutLengthHSTRING(null);
    verify(elementsRead, 0, "elementsRead");
}

function testReceiveArrayWithLengthAttributeHSTRING() {
    var myAnimal = new Animals.Animal(1);
    myAnimal.passArrayHSTRING(["Mr. Gold", "Regina", "Emma Swan", "Henry Mills"]);

    logger.comment("Call ReceiveArray with 4 elements");
    var myArray = myAnimal.receiveArrayWithInLengthHSTRING(2);
    verifyArrayItems("myArray", myArray, ["Mr. Gold", "Regina", "", ""]);

    logger.comment("Call ReceiveArray with 0 elements");
    myAnimal.passArrayHSTRING([]);
    var myArray = myAnimal.receiveArrayWithInLengthHSTRING(0);
    verify(myArray, null, "myArray");

    logger.comment("Call ReceiveArray with size = 4  length = 2");
    myAnimal.passArrayWithInLengthHSTRING(["Mr. Gold", "Regina", "Emma", "Henry"], 2);
    var result = myAnimal.receiveArrayWithOutLengthHSTRING();
    verify(result.lengthValue, 2, "elementsRead");
    verifyArrayItems("myArray", result.value, ["Mr. Gold", "Regina", "", ""]);

    logger.comment("Call ReceiveArray with size = 0  length = 0");
    myAnimal.passArrayHSTRING([]);
    var result = myAnimal.receiveArrayWithOutLengthHSTRING();
    verify(result.lengthValue, 0, "elementsRead");
    verify(result.value, null, "myArray");
}

function testDelegatePassArrayWithLengthAttributeHSTRING() {
    var actualLength = 0;
    var actualArray = 0

    function delegatePassArrayWithInLength(arrayInDelegate, lengthAttribute) {
        logger.comment("*** delegatePassArrayWithInLength - Invoke ***");

        dumpArrayItems("arrayInDelegate", arrayInDelegate);
        logger.comment("lengthAttribute : " + lengthAttribute);

        actualLength = lengthAttribute;
        actualArray = arrayInDelegate;

        logger.comment("*** delegatePassArrayWithInLength - Exit ***");
    }

    logger.comment("Call PassArray with size = 4  length = 2");
    var myAnimal = new Animals.Animal(1);
    myAnimal.passArrayWithInLengthHSTRING(["Sheldon", "Leonard", "Howard", "Rajesh"], 2);
    myAnimal.callDelegatePassArrayWithInLengthHSTRING(delegatePassArrayWithInLength);
    verify(actualLength, 2, "elementsRead");
    verifyArrayItems("myArray", actualArray, ["Sheldon", "Leonard", "", ""]);

    var actualArray = 0
    var sendLength;
    function delegatePassArrayWithOutLength(arrayInDelegate) {
        logger.comment("*** delegatePassArrayWithOutLength - Invoke ***");
        dumpArrayItems("arrayInDelegate", arrayInDelegate);

        actualArray = arrayInDelegate;

        logger.comment("*** delegatePassArrayWithOutLength - Exit ***");
        return sendLength;
    }

    logger.comment("Call PassArray with size = 4  length = 2");
    var myAnimal = new Animals.Animal(1);
    myAnimal.passArrayWithInLengthHSTRING(["Sheldon", "Leonard", "Howard", "Rajesh"], 2);
    sendLength = 2;
    myAnimal.callDelegatePassArrayWithOutLengthHSTRING(delegatePassArrayWithOutLength);
    verifyArrayItems("myArray", actualArray, ["Sheldon", "Leonard", "", ""]);
}

function testDelegateFillArrayWithLengthAttributeHSTRING() {
    var actualArray = 0
    var actualLength = 0;
    var excpectedNull = false;
    function delegateFillArrayWithInLength(arrayInDelegate, lengthAttribute) {
        logger.comment("*** delegateFillArrayWithInLength - Invoke ***");

        verifyAllZeroItemsHSTRING("arrayInDelegate", arrayInDelegate, excpectedNull ? 0 : actualArray.length, excpectedNull);
        verify(lengthAttribute, actualLength, "lengthAttribute");

        for (var i = 0; i < actualLength; i++) {
            arrayInDelegate[i] = actualArray[i];
        }

        logger.comment("*** delegateFillArrayWithInLength - Exit ***");
    }

    logger.comment("Call FillArray with size = 4  length = 2");
    var myAnimal = new Animals.Animal(1);
    actualArray = ["Sheldon", "Leonard", "Howard", "Rajesh"];
    actualLength = 2;
    myAnimal.passArrayWithInLengthHSTRING(actualArray, actualLength);
    myAnimal.callDelegateFillArrayWithInLengthHSTRING(delegateFillArrayWithInLength);
    var resultArray = myAnimal.receiveArrayHSTRING().value;
    verifyArrayItems("resultArray", resultArray, ["Sheldon", "Leonard", "", ""]);

    var actualArray = 0
    var actualLength = 0;
    var excpectedNull = false;
    function delegateFillArrayWithOutLength(arrayInDelegate) {
        logger.comment("*** delegateFillArrayWithOutLength - Invoke ***");

        verifyAllZeroItemsHSTRING("arrayInDelegate", arrayInDelegate, excpectedNull ? 0 : actualArray.length, excpectedNull);
        for (var i = 0; i < actualLength; i++) {
            arrayInDelegate[i] = actualArray[i];
        }
        logger.comment("returnLength : " + actualLength);

        logger.comment("*** delegateFillArrayWithOutLength - Exit ***");

        return actualLength;
    }

    var myAnimal = new Animals.Animal(1);
    myAnimal.passArrayHSTRING(["Sheldon", "Leonard", "Howard", "Rajesh"]);

    logger.comment("Call FillArray with size = 4  length = 2");
    actualArray = ["Sheldon", "Leonard", "Howard", "Rajesh"];
    actualLength = 2;
    myAnimal.callDelegateFillArrayWithOutLengthHSTRING(delegateFillArrayWithOutLength);
    var resultArray = myAnimal.receiveArrayHSTRING().value;
    verifyArrayItems("resultArray", resultArray, ["Sheldon", "Leonard", "", ""]);
}

function testDelegateReceiveArrayWithLengthAttributeHSTRING() {
    var actualArray = 0
    var actualLength = 0;
    function delegateReceiveArrayWithInLength(lengthAttribute) {
        logger.comment("*** delegateReceiveArrayWithInLength - Invoke ***");

        verify(lengthAttribute, actualLength, "lengthAttribute");
        dumpArrayItems("returnArray", actualArray);
        logger.comment("*** delegateReceiveArrayWithInLength - Exit ***");
        return actualArray;
    }

    logger.comment("Call ReceiveArray with size = 4  length = 2");
    var myAnimal = new Animals.Animal(1);
    actualArray = ["Sheldon", "Leonard", "Howard", "Rajesh"];
    actualLength = 2;
    myAnimal.passArrayWithInLengthHSTRING(["Amy", "Priya", "Bernadette", "Penny"], actualLength);
    myAnimal.callDelegateReceiveArrayWithInLengthHSTRING(delegateReceiveArrayWithInLength);
    var resultArray = myAnimal.receiveArrayHSTRING().value;
    verifyArrayItems("resultArray", resultArray, ["Sheldon", "Leonard", "", ""]);

    var actualArray = 0
    var actualLength = 0;
    function delegateReceiveArrayWithOutLength() {
        logger.comment("*** delegateReceiveArrayWithOutLength - Invoke ***");

        dumpArrayItems("returnArray", actualArray);
        logger.comment("returnLength: " + actualLength);

        logger.comment("*** delegateReceiveArrayWithOutLength - Exit ***");

        var outVar = new Object();
        outVar.value = actualArray;
        outVar.lengthValue = actualLength;

        return outVar;
    }

    logger.comment("Call ReceiveArray with size = 4  length = 2");
    var myAnimal = new Animals.Animal(1);
    actualArray = ["Amy", "Priya", "Bernadette", "Penny"];
    actualLength = 2;
    myAnimal.callDelegateReceiveArrayWithOutLengthHSTRING(delegateReceiveArrayWithOutLength);
    var resultArray = myAnimal.receiveArrayHSTRING().value;
    verifyArrayItems("resultArray", resultArray, ["Amy", "Priya", "", ""]);
}

runner.addTest({
    id: 1,
    desc: 'PassArray FillArray And ReceiveArray : methodCall',
    pri: '0',
    test: function () {
        testPassFillAndReceiveArray();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 2,
    desc: 'PassArray FillArray And ReceiveArray on finalizable element type : methodCall',
    pri: '0',
    test: function () {
        testPassFillAndReceiveArrayFinalizable();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 3,
    desc: 'PassArray FillArray on TypedArray : methodCall',
    pri: '0',
    test: function () {
        testPassFillTypedArray();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 4,
    desc: 'PassArray FillArray And ReceiveArray on ArrayProjection : methodCall',
    pri: '0',
    test: function () {
        testPassFillArrayProjection();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 5,
    desc: 'PassArray FillArray And ReceiveArray : delegate',
    pri: '0',
    test: function () {
        delegatePassFillAndReceiveArray();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 6,
    desc: 'PassArray FillArray And ReceiveArray on finalizable: delegate',
    pri: '0',
    test: function () {
        delegatePassFillAndReceiveArrayFinalizable();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 7,
    desc: 'PassArray FillArray on CanvasPixelArray as Uint8Array : methodCall',
    pri: '0',
    test: function () {
        testPassFillCanvasPixelArray();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 8,
    desc: 'PassArray FillArray on CanvasPixelArray as IntArray : methodCall',
    pri: '0',
    test: function () {
        testPassFillCanvasPixelArrayAsIntArray();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 9,
    desc: 'Pass array of big structs : methodCall',
    pri: '0',
    test: function () {
        testPassArrayOfBigStructs();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 10,
    desc: 'PassArray FillArray And ReceiveArray - null : method call',
    pri: '0',
    test: function () {
        testPassFillAndReceiveArrayNull();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 11,
    desc: 'PassArray FillArray jsArray with 0 elements : method Call',
    pri: '0',
    test: function () {
        testPassAndFillArray0Elements();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 12,
    desc: 'PassArray FillArray And ReceiveArray null : delegate',
    pri: '0',
    test: function () {
        delegatePassFillAndReceiveArrayNull();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 13,
    desc: 'ReceiveArray jsArray with 0 elements : delegate',
    pri: '0',
    test: function () {
        delegateReceiveArray0Elelements();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 14,
    desc: 'PassArray With length attribute',
    pri: '0',
    test: function () {
        testPassArrayWithLengthAttribute();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 15,
    desc: 'FillArray With length attribute',
    pri: '0',
    test: function () {
        testFillArrayWithLengthAttribute();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 16,
    desc: 'ReceiveArray With length attribute',
    pri: '0',
    test: function () {
        testReceiveArrayWithLengthAttribute();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 17,
    desc: 'Delegate PassArray With length attribute',
    pri: '0',
    test: function () {
        testDelegatePassArrayWithLengthAttribute();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 18,
    desc: 'Delegate FillArray With length attribute',
    pri: '0',
    test: function () {
        testDelegateFillArrayWithLengthAttribute();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 19,
    desc: 'Delegate ReceiveArray With length attribute',
    pri: '0',
    test: function () {
        testDelegateReceiveArrayWithLengthAttribute();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 20,
    desc: 'PassArrayHSTRING With length attribute',
    pri: '0',
    test: function () {
        testPassArrayWithLengthAttributeHSTRING();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 21,
    desc: 'FillArrayHSTRING With length attribute',
    pri: '0',
    test: function () {
        testFillArrayWithLengthAttributeHSTRING();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 22,
    desc: 'ReceiveArrayHSTRING With length attribute',
    pri: '0',
    test: function () {
        testReceiveArrayWithLengthAttributeHSTRING();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 23,
    desc: 'Delegate PassArrayHSTRING With length attribute',
    pri: '0',
    test: function () {
        testDelegatePassArrayWithLengthAttributeHSTRING();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 24,
    desc: 'Delegate FillArrayHSTRING With length attribute',
    pri: '0',
    test: function () {
        testDelegateFillArrayWithLengthAttributeHSTRING();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 25,
    desc: 'Delegate ReceiveArrayHSTRING With length attribute',
    pri: '0',
    test: function () {
        testDelegateReceiveArrayWithLengthAttributeHSTRING();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
