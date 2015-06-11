WScript.InitializeProjection();
var count = 0;
for (var c in Animals) {
    WScript.Echo(c + ' : ' + Animals[c]);
    count++;
}
WScript.Echo("Number of properties of Animals: " + count);
Debug.dumpHeap(Animals);