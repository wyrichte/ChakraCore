WScript.LoadScriptFile("Utils.js")

var HETest = {};
    
var HD = Debug.dumpHeap(0, /*dump log*/ false, /*forbaselineCompare*/ true, /*rootsOnly*/ true, /*returnArray*/ true);
HD = undefined;

microRequestedSize = 1;
mediumRequestedSize = 1000;
maxRequestedSize = 0X7FFFFFFF;
HETest.ab_micro = new ArrayBuffer(1);
HETest.ab_medium = new ArrayBuffer(mediumRequestedSize);
try
{
    // sometimes we might OOM and fail to generate the array, so ignore failue
    HETest.ab_max = new ArrayBuffer(maxRequestedSize);
}
catch(e)
{
    HETest.ab_max = null;
}

try
{
    HETest.ab_max_plus_one = new ArrayBuffer(maxRequestedSize+1);
}
catch(e)
{
    // expect this to fail    
}
testExpectation("HETest.ab_max_plus_one == undefined") ;
HD = Debug.dumpHeap(0, /*dump log*/ false, /*forbaselineCompare*/ undefined, /*rootsOnly*/ undefined, /*returnArray*/ true);

var microSize;
var mediumSize;
var maxSize;
var o;
if (o=findObjectInHeap(HETest.ab_micro)) microSize = o.size;
if (o=findObjectInHeap(HETest.ab_medium)) mediumSize = o.size; 
if (o=findObjectInHeap(HETest.ab_max)) maxSize = o.size; 

testExpectation("microSize - microRequestedSize == mediumSize - mediumRequestedSize");
var objectSize = microSize - microRequestedSize;
testExpectation("microSize-objectSize == microRequestedSize");
testExpectation("mediumSize-objectSize == mediumRequestedSize");
testExpectation("! HETest.ab_max || maxSize-objectSize == maxRequestedSize");
displayResult();
