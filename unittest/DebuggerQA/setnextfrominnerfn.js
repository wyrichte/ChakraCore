/* Negastive test case - ignore positive assertion text in WScript.Echo */

var a = 1;
var count = 0;
function foo(){
	WScript.Echo('Marker1');/**bp(bp3):locals()**/
	bar();
	function bar(){
		let a = 1;
		count++;		
		if(count > 1) return; /**bp:setnext('bp4')**/
		WScript.Echo(); /**bp:setnext('bp1')**/
		WScript.Echo('Should not print this - innerfn- bar');
	}
	WScript.Echo('Marker2'); /**loc(bp1):setnext('bp3')**/
}
foo();

WScript.Echo('Marker3 should be skipped');
var b = 2; /**loc(bp4)**/
WScript.Echo('Passed');/**bp:locals()**/


function Run(){
	function inner(){
		WScript.Echo();/**bp:setnext('bp1')**/
	}
	inner();
	WScript.Echo('Check') /**bp(bp1):stack()**/
}
Run();
WScript.Echo('Passed2');