WScript.InitializeProjection();
var propertyValueTests = new Animals.PropertyValueTests();
var myArray = propertyValueTests.receiveAnimalArray();
Debug.dumpHeap(myArray);