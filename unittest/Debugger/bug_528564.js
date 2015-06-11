//Switches: -maxinterpretcount:2 -debuglaunch -targeted -dbgbaseline 
//Baseline Switches: -nonative 
function test0(){
  var a = 1;
  var e = 1;
  switch(1) {
    case 1: 
      (function () {
          function foo() {
              eval("");
              function bar() {
                  foo();
              }
          };
      })();
      break;
  }
};
test0(); 
test0(); 
test0(); 

WScript.Echo("PASS");
