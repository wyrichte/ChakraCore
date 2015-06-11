WScript.InitializeProjection();
var animal = new Animals.Animal();
var vector = animal.getVector();
Debug.dumpHeap(vector);