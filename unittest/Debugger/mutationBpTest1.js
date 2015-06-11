// Object Mutation Breakpoint
// Simple setting and deleting bp on a valid Javascript object

/**onmbp:locals(1);**/

var obj = new Object();

function doNothing() {
	return true;  /**bp:mbp("[Globals].obj", "properties", 'add', "greet");**/ 
}
function setGreet() {
	obj.greetStr = "Welcome! :)";
}
function setName() {
	obj.myName = "greeter";
}
doNothing();
setGreet();
setName(); /**bp:deleteMbp("greet");**/

WScript.Echo("pass");