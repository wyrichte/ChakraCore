WScript.InitializeProjection();
function verify(actualValue, expectedValue, valueMsg) {
    if (actualValue === expectedValue) {
        WScript.Echo("Pass: " + valueMsg);
    }
    else {
        WScript.Echo("Fail: " + valueMsg);
        WScript.Echo("    Expected: " + expectedValue);
        runner.currentTestSucceeded = false;
    }
    WScript.Echo("      Actual: " + actualValue + "\n");
}

var myAnimal = new Animals.Animal(1);
var array = ["SnowWhite", "PrinceCharming", "EvilQueen"];
myAnimal.passArrayHSTRING(array);
var myTypedArray = myAnimal.receiveArrayHSTRING().value;
verify(myTypedArray.length, array.length, 'myTypedArray.length');
for (var i = 0; i < array.length; i++) {
    verify(myTypedArray[i], array[i], 'myTypedArray[' + i + ']');
}
Debug.dumpHeap(myTypedArray);