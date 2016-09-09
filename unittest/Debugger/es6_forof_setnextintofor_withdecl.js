/* negative case of jumping into loop - as declaration is present */
/**exception(resume_ignore)**/
var m = 0;
m;/**bp:setnext('bp1')**/
for(let i of [0,1]){
    if(i<1){} /**loc(bp1)**/
}


/* negative case of jumping into loop - as declaration is present */
var m1 = 0;
m1;/**bp:locals();setnext('bp2')**/
for(var i1 of [0,1]){
    i1;/**loc(bp2)**/
    if(i1<1){ i1 ; /**bp:evaluate('i1')**/ } 
    
}
WScript.Echo('PASSED');