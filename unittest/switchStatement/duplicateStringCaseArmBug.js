//-maxinterpretcount:1 D:\fbl_ie_script_dev\testFiles\bug8.js -loopinterpretcount:1 -bgjit- -MaxLinearStringCaseCount:2

function test0(){
  var b = 5;
  var __loopvar1 = 3;
    
    for(; __loopvar1 < 4; __loopvar1++) {
      switch('m') {
        case 'n': 
          break;
		case 'a':
			break;
        case 'n': 	
		WScript.Echo("m");
          b|2;
          break;      
		 default:
		 break;
      }
    }
  
};

// generate profile
test0(); 
test0(); 
WScript.Echo("PASSED");

