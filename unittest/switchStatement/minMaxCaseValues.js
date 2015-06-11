function test0(){
    switch(1) {
      case 2147483647: 
        break;
      case 2: 
        break;
      case 1: 
        break;
      case -2147483648: 
        break;
    }
  
};

// generate profile
test0(); 
test0(); 
WScript.Echo("PASSED");

