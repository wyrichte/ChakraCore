//
// Test array, where an item has toString primitive
//
var x = [ {toString: function(){}} ];

//
// Test getter
//
var x2 = {a: 1, b: 2, get c() { WScript.Echo("hello"); return 100; } };
