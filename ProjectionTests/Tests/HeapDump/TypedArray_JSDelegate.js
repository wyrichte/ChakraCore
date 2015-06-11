WScript.InitializeProjection();
var propertyValueTests = new Animals.PropertyValueTests();
function delegate1() {
    return "delegate1";
}

function delegate2() {
    return "delegate2";
}

var myArray = propertyValueTests.receiveJSDelegateArray(delegate1, delegate2);
Debug.dumpHeap(myArray);