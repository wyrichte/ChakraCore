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
var myArray = propertyValueTests.receiveStructArray();
Debug.dumpHeap(2, false, true);
propertyValueTests.myPropertyValue = myArray;
Debug.dumpHeap(0, true, true);
var myStructArray = propertyValueTests.myPropertyValue;
verify(myStructArray.length, myArray.length, 'myStructArray.length');
for (var i = 0; i < myArray.length; i++) {
    verify(myStructArray[i].length, myArray[i].length, 'myStructArray[' + i + '].length');
    verify(myStructArray[i].width, myArray[i].width, 'myStructArray[' + i + '].width');
}
