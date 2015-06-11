// Run with jc -maxinterpretcount:1

// Create a path type with fixed method 'o.inspect'.
var o = {};
o.inspect = function () { WScript.Echo("original"); };

// JIT a function that uses the fixed method
function useMethod(obj) {
    obj.inspect();
}
useMethod(o);
useMethod(o);

// Use the property cache to overwrite the fixed method
function test(obj, overwrite) {
    for (var prop in obj) {
        if (overwrite)
            obj[prop] = function () { WScript.Echo("new"); }
    }
}
test(o,false);
test(o,true);

// Verify that the new function is called.
useMethod(o);
