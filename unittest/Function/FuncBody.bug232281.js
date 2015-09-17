//FileName: \\ezeqasrv1\users\runtimetoolsbugs\casanalyzed\exprgen\fbl_ie_script_dev\130421-2030\x86\nebularun_x86\201304231347\ddltd2_269a5520-bce0-4753-a54d-6ac358f9cdd4\bug0.js
//Configuration: inline.xml
//Testcase Number: 3445
//Bailout Testing: ON
//Switches:  -maxinterpretcount:1  
//Baseline Switches: -nonative 
//Branch:  fbl_ie_script_dev
//Build: 130421-2030
//Arch: X86
//MachineName: BPT16228
var shouldBailout = false;
function test0(){
  var obj0 = {};
  var obj1 = {};
  var func0 = function(){
  }
  var func1 = function(argObj0,argArr1,argMath2){
    eval("");
    func0();
  }
  Object.prototype.method0 = func1; 
  obj1.length = ((shouldBailout ? func0 = obj0.method0 : 1), func0()); 
};

var testOutcome = false;

try
{
// generate profile
test0(); 

// run JITted code
runningJITtedCode = true;
test0(); 

// run code with bailouts enabled
shouldBailout = true;
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
