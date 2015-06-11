//Configuration: constructors.xml
//Testcase Number: 34970
//Switches: -maxinterpretcount:5 -maxsimplejitruncount:6
//Baseline Switches: -nonative
//Arch: X86
//Flavor: fre
//Branch: fbl_ie_stage_dev3
//Build: 140623-2030
//FullBuild: 9773.0.140623
//MachineName: VSP34681
//InstructionSet: 
//reduced switches: -maxinterpretcount:5 -maxsimplejitruncount:6
//noRepro switches1: -maxinterpretcount:5 -maxsimplejitruncount:6 -off:DynamicProfile
//noRepro switches2: -maxinterpretcount:5 -maxsimplejitruncount:6 -off:EliminateArrayAccessHelperCallNativeArray
//noRepro switches3: -maxinterpretcount:5 -maxsimplejitruncount:6 -off:LossyIntTypeSpec
//noRepro switches4: -maxinterpretcount:5 -maxsimplejitruncount:6 -off:TypeSpec
//noRepro switches5: -maxinterpretcount:5 -maxsimplejitruncount:6 -off:TrackIntOverflow
//noRepro switches6: -maxinterpretcount:5 -maxsimplejitruncount:6 -off:Inline
function test0() {
  function makeArrayLength(x) {
    
    if (!(x < 1)) {
      //return Math.floor();
      return Math.floor();
    }
    
  }
  
  var obj2 = {};
  
  var func1 = function () {
    y = ~(++this.prop5 >>> protoObj1.prop3)
    protoObj1.length = makeArrayLength(y);

  };
  
  obj2.method0 = func1;
  protoObj1 = Object();
  Object.prototype.prop5 = -1921245026.9;
  obj2.method0();
  WScript.Echo(protoObj1.length);
}
test0();
test0();
test0();
test0();
test0();
test0();
test0();
test0();
test0();
test0();
test0();
test0();

test0();
test0();
test0();
test0();
test0();
test0();
test0();
test0();
test0();
test0();
test0();
test0();