//FileName: \\ezeqasrv1\users\runtimetoolsbugs\casanalyzed\exprgen\fbl_ie_script_dev\130420-2030\x86\nebularun_x86\201304220152\ddltd2_1c85c885-2ee3-43ce-8fb7-39780ac4e20a\bug2.js
//Configuration: inline.xml
//Testcase Number: 55168
//Switches:  -maxinterpretcount:3  
//Baseline Switches: -nonative 
//Branch:  fbl_ie_script_dev
//Build: 130420-2030
//Arch: X86
//MachineName: BPT02679
function test0(){
  var obj0 = {};
  var arrObj0 = {};
  var func0 = function(){
    eval("");
  }
  var func1 = function(){
    var obj4 = {nd0: {nd0: {lf0: {prop0: -46, prop1: 3, prop2: -2147483648, length: -6.02625054824609E+18 , method0: func0}}}};
    d ^=obj4.nd0.nd0.lf0.method0();
    obj4.nd0.nd0.lf0 = 1;
    this.prop1 |=obj4.nd0.nd0.lf0.method0.call(obj0 );
  }
  Object.prototype.method0 = func1; 
  var d = 1;
  arrObj0.method0();
};

var testOutcome = false;

try
{
// generate profile
test0(); 
test0(); 
test0(); 

// run JITted code
runningJITtedCode = true;
test0(); 
test0(); 
test0(); 
}
catch(e)
{
  WScript.Echo("Caught expected exception. Type of exception: " + e);
  if (e == "Error: Out of stack space") {
    testOutcome = true;
  }
}

if (testOutcome) {
  WScript.Echo("Passed");
}
else {
  WScript.Echo("Failed");
}
