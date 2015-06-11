//Configuration: TrackIntUsage.xml
//Testcase Number: 5965
//Switches:   -maxinterpretcount:1 -maxsimplejitruncount:2  -MaxLinearStringCaseCount:2 -MaxLinearIntCaseCount:2 -forceserialized -MinSwitchJumpTableSize:3 -bgjit- -loopinterpretcount:1 -force:fieldhoist -force:polymorphicinlinecache -force:fieldcopyprop -sse:2 -force:interpreterautoprofile
//Baseline Switches: -nonative 
//Arch: AMD64
//Flavor: chk
//Branch:  fbl_ie_stage_dev3
//Build: 140612-2030
//FullBuild: 9769.0.140612
//MachineName: VSP53702
//InstructionSet: 
function test0(){
  var obj0 = {};
  var arrObj0 = {};
  var func0 = function(){
    l -=((-733600173 * arrObj0.prop0) & (-731419186 * (-733600173 * arrObj0.prop0)));
  };
  var func1 = function(argArr0){
    k = func0.call(arrObj0 );
  };
  obj0.method0 = func1;
  var l = 1;
  arrObj0.prop0 = -38;
  m = obj0.method0.call(obj0 , 1);
  WScript.Echo('l = ' + (l|0));
};

// generate profile
test0();
// Run Simple JIT
test0();
test0();

// run JITted code
runningJITtedCode = true;
test0();


// Baseline output:
// l = -897712127
// l = -897712127
// l = -897712127
// l = -897712127
// 
// 
// Test output:
// l = -897712127
// l = -897712127
// l = -897712127
// l = -897713151
// 
