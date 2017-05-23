var a = Debug.createTypedObject(2000, "mytype", 32);
var b = Debug.addFTLProperty(a, "test", 1, 20);
var c = a.test;
var c = a.test;
WScript.Echo(a);
for (i in a) {
WScript.Echo(i + " = " + a[i]) }
WScript.Echo(c);



function oneTest(index)
{
  var a = Debug.createTypedObject(3000+index, "mytype", (index+1)*8);
  var proto = Debug.createTypedObject(3000+index, "mytype", 32);
  a.__proto__ = proto;
  var b = Debug.addFTLProperty(a, "test"+index, index, 20+index);
  function test(a) {
    var c = a["test"+index];
    var c = a["test"+index];
    WScript.Echo(c);
  };
  test(a);
  test(a);
}

for (i = 1; i < 20; i++) {
oneTest(i);
}

