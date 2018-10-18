//
// Test array, where an item has toString primitive
//
var x = [ {toString: function(){}} ];

//
// Test getter
//
var x2 = {a: 1, b: 2, get c() { WScript.Echo("hello"); return 100; }, d: Math.cos(0) }; 

// Adding Math.cos(0) to initialize the Math object fully, otherwise during enumerating the objects the main script context would close
// and the enumeration would trigger the deferred type handler to undefer for the math object.
