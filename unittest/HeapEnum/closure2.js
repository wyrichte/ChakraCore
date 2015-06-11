function outer(outerArgument) {
    var outerVar1 = "outerVar1 value";
    var outerVar2 = 2222222;
    function intermediate(intermediateArgument) {
        var intermediateVar = "intermediateVar value";
        function internal() {
            var internalVar = "internalVar value";
            return outerArgument + ' ' + outerVar1 + ' ' + outerVar2 + ' ' + intermediateArgument + ' ' + intermediateVar + ' ' + internalVar;
        }
        return internal;
    }
    return intermediate;
}

var x = outer(111111)(3333333);

WScript.Echo(x());

Debug.dumpHeap(x);
