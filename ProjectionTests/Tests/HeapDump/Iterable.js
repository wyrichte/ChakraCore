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
Debug.dumpHeap(2, false, true);
var iterable = animal.sendBackSameIterable(array);
Debug.dumpHeap(0, true, true);
var iterator = iterable.first();
var manyItems = new Array(4);
var gotMany = iterator.getMany(manyItems);
verify(gotMany, array.length, 'gotMany');
for (var i = 0; i < array.length; i++) {
    verify(manyItems[i], array[i], 'manyItems[' + i + ']');
}
