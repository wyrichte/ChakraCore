function test0(){
  if((function () {;})) {
  }
};

// generate profile
test0(); 
test0(); 
test0(); 
test0(); 

// run JITted code
runningJITtedCode = true;
test0(); 
test0(); 
test0(); 
test0(); 



