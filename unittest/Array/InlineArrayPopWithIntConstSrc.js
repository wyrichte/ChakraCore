function test0(){
  
  var func0 = function(argArr2){
    if((check ? (argArr2.pop()) : WScript.Echo("false"))) {
    }
  }
 
  var check = true; 
  func0([1]); // func0 will be inlined while jitting test0()
  check = false;
  func0(1); // func0 will be inlined while jitting test0()
};

// generate profile
test0(); 
// Run Simple JIT
test0(); 

