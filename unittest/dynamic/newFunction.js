function write(v) { WScript.Echo(v + ""); }

WScript.Echo("Baseline Working set:" + Debug.getWorkingSet().workingSet);

var numIters = 100000;

for (var i = 0; i < numIters; i++)
{
    var f0 = new Function("return " + i);
    if (i % 5000 == 0) { CollectGarbage(); write(f0()); }
}

WScript.Echo("Working set:" + Debug.getWorkingSet().workingSet);
CollectGarbage();
CollectGarbage();
WScript.Echo("Working set after GC:" + Debug.getWorkingSet().workingSet);
WScript.Echo("Peak Working set:" + Debug.getWorkingSet().maxWorkingSet);

while (1);
