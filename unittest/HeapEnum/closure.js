function Dump(x, dump)
{
    //dump = true;
    if (dump != undefined)
    {
        WScript.Echo(x);
    }
}

var HETest = {};

HETest.producer = function () {
   var x=3;
   var z=function() {
      Dump(x);
   }
   return z;
}

HETest.consumer = function (f) {
    f();
}

HETest.clo=HETest.producer();
HETest.consumer(HETest.clo);

Debug.dumpHeap(HETest, true);