function mySort(myArray) {
  Array.prototype.sort.call(myArray, sortFunc);
}

var localArray = [];
function sortFunc(a, b) {
  localArray[a.toString()] = a;
  localArray[b.toString()] = b;
}