


WScript.Echo('This should be skipped'); /**bp:setnext('bp1')**/
try{
	var a = 1;
	WScript.Echo('jump to try')/**bp(bp1):locals();setnext('bp2');**/
}catch(e){
	WScript.Echo('jump to catch');/**bp(bp2):locals()**/	
}
WScript.Echo('Test 2')

WScript.Echo('This should be skipped'); /**bp:setnext('bp11')**/
try{
	let a = 1;
	WScript.Echo('jump to try')/**bp(bp11):locals();setnext('bp21');**/
}catch(e){
	WScript.Echo('jump to catch');/**bp(bp21):locals()**/	
}