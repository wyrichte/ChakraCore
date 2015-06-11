//Switches:  -maxinterpretcount:2 -maxsimplejitruncount:5  -off:inline
function test0(){
  var d = 1;
  var __loopvar0 = 0;
  do {
    __loopvar0++;
    if((this.prop0 <= d)) {
      var __loopvar3 = 0;
      do {
        __loopvar3++;
1        // Snippet switch1
        switch(Object.keys(arrObj0).length){
          case 1:
            GiantPrintArray.push(1);
            break;
          case 2:
            GiantPrintArray.push(2);
            break;
          case 3:
            GiantPrintArray.push(3);
            break;
          case 4:
            
            GiantPrintArray.push(4);
          case arrObj0:
            GiantPrintArray.push(arrObj0);
          case 5:
            d *=1;
            GiantPrintArray.push(5);
            break;
          case 6:
            GiantPrintArray.push(6);
            break;
          case 7:
            GiantPrintArray.push(7);
          case 8:
            
            GiantPrintArray.push(8);
            break;
          default:
            GiantPrintArray.push("Default");
            break;
        }    
      } while(((d = 1)) && __loopvar3 < 3)
      d ^=IntArr1[(1)];
      //Code Snippet: NegativeArgumentBug.ecs (Blue15423)
      for (var _i in arguments[1]) { 
        d =2.62877767046713E+18;
      };
      
      
    }
  } while((1) && __loopvar0 < 3)
};

// generate profile
test0(); 
test0(); 
// Run Simple JIT
test0(); 
test0(); 
test0(); 
test0(); 
test0(); 

// run JITted code
runningJITtedCode = true;
test0(); 
test0(); 

// run code with bailouts enabled
shouldBailout = true;
test0(); 
WScript.Echo("PASSED");
