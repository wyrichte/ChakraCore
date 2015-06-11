WScript.InitializeProjection();
var animal = new Animals.Animal();
var vector = animal.getObservableVector();
Debug.dumpHeap(vector);