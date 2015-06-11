
function generateEval(n)
{
      eval("var oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo = " + n); 
}

WScript.Echo("Baseline Working set:" + Debug.getWorkingSet().workingSet);

var numIters = 110000;
// var numIters = 330;

for(var i = 0; i < numIters; i++)
{
    generateEval(i); 
}
WScript.Echo("Working set:" + Debug.getWorkingSet().workingSet);
CollectGarbage();
CollectGarbage();
WScript.Echo("Working set after GC:" + Debug.getWorkingSet().workingSet);
WScript.Echo("Peak Working set:" + Debug.getWorkingSet().maxWorkingSet);
