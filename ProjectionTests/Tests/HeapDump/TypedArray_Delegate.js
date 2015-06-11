WScript.InitializeProjection();
var propertyValueTests = new Animals.PropertyValueTests();
var myArray = propertyValueTests.receiveWinrtDelegateArray();
Debug.dumpHeap(myArray);