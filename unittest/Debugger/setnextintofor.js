var i = 0;
WScript.Echo();/**bp:setnext('bp1')**/
WScript.Echo('failed');
for(i = 0; i<10; i++){
	if(i<1){} /**loc(bp1)**/
}
WScript.Echo('passed');