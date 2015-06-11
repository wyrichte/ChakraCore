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
function vectorChangedHandler(ev) {
    vectorChangedCount++;
}
vector.addEventListener('vectorchanged', vectorChanged);
vector.onvectorchanged = vectorChangedHandler;
vector[3] = 55;
verify(vectorChangedCount, 2, "vectorChangedCount");
Debug.dumpHeap(vector);