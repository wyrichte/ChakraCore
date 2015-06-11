/** setnext into foreach statement **/

var  a = {
	test: 1
};
a ; /**bp:setnext('bp1')**/
for(var b in a){	
	b; /**loc(bp1)**/
}


a ; /**bp:setnext('bp2')**/
for(let c in a){
	c; /**loc(bp2)**/
}

WScript.Echo('PASSED');
