function output(data)
{
WScript.Echo(data);
}
function printObj(obj)
{
output(obj);
for (i in obj) {
  output(i + " = " + obj[i]);
}
}
function func () { return this };
label = "win1";

win2 = WScript.LoadScriptFile("empty.js", "crossthread");
//win2 = window.open();

win2.label = "win2";
function test(){
 win2.func = func;
 output(win2.func().label);  // win1
 output(func.call(win2).label); // win2
}


function testfunc() {
this.nested = function() { return this; }
this.nested.label = "nested";
this.label = "testfunc";
return this;
}

function test1() {
  testfunc();
  win2.testfunc = new testfunc();
  win2.testfunc.nested.label = "win2 nested";
  win2.testfunc.label = "win2 testfunc";
  win2testfunc = win2.testfunc; 
  output(win2testfunc.nested().label);
  output(win2testfunc.nested.call(win2testfunc.nested).label);
  output(win2testfunc.nested.call(testfunc.nested).label);
  output(testfunc.call(testfunc).label);
  output(testfunc.nested().label);
}

test();
test1();