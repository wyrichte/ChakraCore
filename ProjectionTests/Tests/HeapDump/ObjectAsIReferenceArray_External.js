WScript.InitializeProjection();
function verify(actualValue, expectedValue, valueMsg) {
    if (actualValue === expectedValue) {
        WScript.Echo("Pass: " + valueMsg);
    }
    else {
        WScript.Echo("Fail: " + valueMsg);
        WScript.Echo("    Expected: " + expectedValue);
    }
    WScript.Echo("      Actual: " + actualValue + "\n");
}

var propertyValueTests = new Animals.PropertyValueTests();
function delegate1() {
    return "delegate1";
}

function delegate2() {
    return "delegate2";
}
var myArray = propertyValueTests.receiveWinrtDelegateArray(delegate1, delegate2);
Debug.dumpHeap(2, false, true);
propertyValueTests.myPropertyValue = myArray;
Debug.dumpHeap(0, true, true);
var myDelegateArray = propertyValueTests.myPropertyValue;
verify(myDelegateArray.length, myArray.length, 'myDelegateArray.length');
for (var i = 0; i < myArray.length; i++) {
    verify(myDelegateArray[i].toString(), myArray[i].toString(), 'myDelegateArray[' + i + '].toString()');
}

