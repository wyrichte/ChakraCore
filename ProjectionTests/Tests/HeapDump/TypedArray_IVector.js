WScript.InitializeProjection();
var propertyValueTests = new Animals.PropertyValueTests();
var myArray = propertyValueTests.receiveVectorArray();
Debug.dumpHeap(myArray);