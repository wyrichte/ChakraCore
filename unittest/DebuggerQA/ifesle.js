
var a = 1; /**bp:setnext('bp1')**/

if(true){
	WScript.Echo('this should be skipped - if');
	var b = 1;
	WScript.Echo('jumping to if'); /**bp(bp1):locals();setnext('bp2')**/
}else{
	WScript.Echo('this should be skipped - else');
	var c = 1;
	WScript.Echo('jumping to else');/**bp(bp2):locals();setnext('bp3')**/
}

WScript.Echo('PASSED'); /**bp(bp3):locals()**/




var a1 = 1; /**bp:setnext('bp11')**/

if(true){
	WScript.Echo('this should be printed - if1');
	let b1 = 1;
	WScript.Echo('jumping to if1'); /**bp(bp11):locals();setnext('bp21')**/
}else{	
	let c1 = 1;
	WScript.Echo('jumping to else1');/**bp(bp21):locals();setnext('bp31')**/
	WScript.Echo('this should be skipped - else1');
}

WScript.Echo('PASSED'); /**bp(bp31):locals()**/