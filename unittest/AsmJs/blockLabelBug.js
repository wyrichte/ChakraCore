function module()
{
	"use asm"
	function foo()
	{
 	  h:{
			switch (1) {
			case 1:
			{  
				break h
			}

			}
                
        }		
	}
	return foo;
}
var obj = module();
obj(); 
WScript.Echo("Pass");