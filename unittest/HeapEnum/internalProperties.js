// Tests that we don't expose internal properties via the Heap Enumerator

function Dump(x, dump)
{
    //dump = true;
    if (dump != undefined)
    {
        WScript.Echo(x);
    }
}

var HETest = {};

HETest.testForInWithPrototype = function()
{
    function LoopResultsOfObject(o, methodName)
    {
        Dump(methodName);
        var method = eval("Object." + methodName);
        var r = method(o);
        Dump("Length: " + r.length);
        for (var i in r) {
           Dump(i + " => " + r[i]);
        }
    }

    var protoObject = {};
    protoObject.prop1 = "p1";

    function MyClass()
    {
        this.prop2 = "p2" + "3" + "tell";
    }

    MyClass.prototype = protoObject;

    HETest.instance = new MyClass;

    LoopResultsOfObject(HETest.instance, "getOwnPropertyNames");
    LoopResultsOfObject(HETest.instance, "keys");
}

HETest.testForInWithPrototype();

Debug.dumpHeap(HETest, true);
