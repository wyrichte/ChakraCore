function thisCaptured () {
    return () => this;
}

function thisNotCaptured () {
    return () => "abc";
}

var l1 = thisNotCaptured.call('hello'); 
var l2 = thisCaptured.call('hello');

Debug.dumpHeap(l1);
Debug.dumpHeap(l2);
