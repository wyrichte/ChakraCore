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

var array = [1, 2, 3];
var animal = new Animals.Animal();
Debug.dumpHeap(0, false, true);
var vector = animal.sendBackSameVectorView(array);
Debug.dumpHeap(0, true, true);
verify(vector.length, array.length, 'vector.length');
for (var i = 0; i < array.length; i++) {
    verify(vector[i], array[i], 'vector[' + i + ']');
}
