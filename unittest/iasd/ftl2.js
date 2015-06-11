  var obj = Debug.createTypedObject(2000, "mytype", 32);
  var proto = Debug.createTypedObject(2000, "mytype", 32);
  obj.__proto__ = proto;
  var b = Debug.addFTLProperty(obj, "fld", 1, 20);
  var c = Debug.addFTLProperty(obj, "fld1", 2, 30);
   var sum = 0;
   Object.defineProperty(obj, "tmp", {get: function() {return 12;}});
function test(testObj)
{
  for (i =0; i < 1000; i++)
  {
      sum += testObj.fld + testObj.fld1;
  }
  return sum;
}

function test1(testObj)
{
  for (i =0; i < 1000; i++)
  {
      sum += testObj.fld + testObj.tmp;
  }
  return sum;
}

var start = new Date();
var total = 0;
for (j = 0 ; j < 1000; j++) 
{
total += test(obj);
}
var end = new Date();
WScript.Echo(obj.test);
WScript.Echo("time without implicit call: " + (end-start));
start = new Date();
total = 0;
for (j = 0 ; j < 1000; j++) 
{
total += test1(obj);
}

var end = new Date();
WScript.Echo(obj.test);
WScript.Echo("time with implicit call: " + (end-start));
