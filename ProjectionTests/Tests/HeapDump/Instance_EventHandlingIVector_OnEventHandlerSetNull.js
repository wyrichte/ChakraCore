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

var animal = new Animals.Animal();
var vector = animal.getObservableVector();
var vectorChangedCount = 0;
function vectorChanged(ev) {
    vectorChangedCount++;
}
vector.onvectorchanged = vectorChanged;
vector[3] = 55;
verify(vectorChangedCount, 1, "vectorChangedCount");
vector.onvectorchanged = null;
vector[3] = 58;
verify(vectorChangedCount, 1, "vectorChangedCount");
Debug.dumpHeap(vector);