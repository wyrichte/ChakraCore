var child = WScript.LoadScriptFile("bug705618child.js","samethread");
var obj= {};
  var myShift = [].shift;
  obj.loader = child.myArr;
  var first = myShift.apply(obj.loader);
  WScript.Echo("PASSED");
