
//Configuration: ..\simpleassignments.xml
//Testcase Number: 44543
//Bailout Testing: ON
//Switches:  -maxinterpretcount:3  
//Branch:  fbl_ie_script
//Build: 130130-2000
//Arch: AMD64
//MachineName: BPT22730

function FAILED()
{
    WScript.Echo("FAILED");
    throw(1);
}

function test0(){
  var arrObj0 = {};
  if((2 % 2)) {
  }
  else {
    b =(2 & 2);
  }
  arrObj0.length =b;
  if (arrObj0.length != 2)
	FAILED();
};

// generate profile
test0();
test0();
test0();

// run JITted code
runningJITtedCode = true;
test0();
test0();
test0();

// run code with bailouts enabled
shouldBailout = true;
test0();


WScript.Echo("Passed");