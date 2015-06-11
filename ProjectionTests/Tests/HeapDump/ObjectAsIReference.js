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

var propertyValueTests = new Animals.PropertyValueTests();

var myDimensions = {
    length: 10,
    width: 40
};
Debug.dumpHeap(2, false, true);
propertyValueTests.myDimensionsReference = myDimensions;
Debug.dumpHeap(0, true, true);
var outVar = propertyValueTests.myDimensionsReference;
verify(outVar.length, myDimensions.length, 'outVar.length');
verify(outVar.width, myDimensions.width, 'outVar.width');