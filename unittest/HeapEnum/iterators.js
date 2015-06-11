var array_iterator = [ 1, 2, 3 ].entries();
var array_iterator_result = array_iterator.next();

var map_iterator = (new Map([ [1,1], [2,2], [3,3] ])).entries();
var map_iterator_result = map_iterator.next();

var set_iterator = (new Set([1, 2, 3])).entries();
var set_iterator_result = set_iterator.next();

var string_iterator = "hello"[Symbol.iterator]();
var string_iterator_result = string_iterator.next();

// Sanity check that iterator result objects dump correctly (they do not have any internal properties)
var iteratorResultObjects = {
    array_iterator_result: array_iterator_result,
    map_iterator_result: map_iterator_result,
    set_iterator_result: set_iterator_result,
    string_iterator_result: string_iterator_result,
};

Debug.dumpHeap(iteratorResultObjects, /*dump log*/ true, /*baselineCompare*/ true);

// Check that the built-in iterator objects hold reference to their underyling collection via internal property
var iteratorObjects = {
    array_iterator: array_iterator,
    map_iterator: map_iterator,
    set_iterator: set_iterator,
    string_iterator: string_iterator,
};

Debug.dumpHeap(iteratorObjects, /*dump log*/ true, /*baselineCompare*/ true);

// Exhaust the iterators and verify that they no longer hold reference to these members
for (var x of array_iterator) { }
for (var x of map_iterator) { }
for (var x of set_iterator) { }
for (var x of string_iterator) { }

Debug.dumpHeap(iteratorObjects, /*dump log*/ true, /*baselineCompare*/ true);
