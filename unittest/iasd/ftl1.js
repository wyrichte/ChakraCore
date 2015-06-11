  var obj = Debug.createTypedObject(2000, "mytype", 32);
  var proto = Debug.createTypedObject(2000, "mytype", 32);
  obj.__proto__ = proto;
  var b = Debug.addFTLProperty(obj, "fld", 1, 20);
  var c = Debug.addFTLProperty(obj, "fld1", 2, 30);
   var sum = 0;
   Object.defineProperty(obj, "tmp", {get: function() {return 12;}});
  var normalObj = {tmp: 1, fld: 2, fld1: 3};

function test(testObj, testObj1) {
  var sum = 0;
  for (var i = 0; i < 100; i++ ) {
    sum += testObj.fld + testObj1.fld1;
  }
  return sum;
}

function test1(testObj, testObj1) {
  var sum = 0;
  for (var i = 0; i < 100; i++ ) {
    sum += testObj.fld + testObj1.tmp;
  }
  return sum;
}

var total = 0;
total = test(obj, obj);
WScript.Echo(total);
total = test(obj, obj);
WScript.Echo(total);

total = test1(obj, obj);
WScript.Echo(total);
total = test1(obj, obj);
WScript.Echo(total);

total = test(obj, normalObj);
WScript.Echo(total);
total = test1(obj, normalObj);
WScript.Echo(total);

