var child = WScript.LoadScriptFile("arraysortchild.js", "samethread");

var arrayToSort = ["one", "five", "two", "eight", "x"];
child.mySort(arrayToSort);
var newArray = [];
for (i = 0; i < arrayToSort.length; i++) {
  newArray[i] = arrayToSort[i];
}
for (i = 0; i < newArray.length; i++) {
  WScript.Echo(newArray[i]);
}

