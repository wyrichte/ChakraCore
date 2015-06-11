WScript.Echo();/**bp:setnext('bp1')**/
WScript.Echo('failed');
for (i of [0,1,2,3]) {
    if(i<1){} /**loc(bp1)**/
}
WScript.Echo('passed');