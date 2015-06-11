function demo(){};
var i=0;
WScript.Echo('Intializing'); /**bp:setnext('bp1')**/

switch(i){	
	case 0: break;
	case 1: break; /**loc(bp1)**/ 
}


i = 0; /**bp:setnext('bp3')**/
switch(i){
	case 0: var a = 1; 
			WScript.Echo(a);
			break;
	case 1: var b = 2; /**loc(bp3)**/
			WScript.Echo(b);/**bp:locals()**/
			break; 
}

i = 0; /**bp:setnext('bp2')**/
switch(i){
	case 0: let a = 1; 
			WScript.Echo(a);
			break;
	case 1: let b = 2; /**loc(bp2)**/
			WScript.Echo(b);/**bp:locals()**/
			break; 
}

i = 0;
function foo(){return i;}
i ; /**bp:setnext('bp4')**/
switch(foo()){
	case 0: var a = 1; 
			WScript.Echo(a);
			break;
	case 1: var b = 2; /**loc(bp4)**/
			WScript.Echo(b); /**bp:locals()**/
			break; 
}



