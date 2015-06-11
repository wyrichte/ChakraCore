// Object Mutation Breakpoint
// Breaking on object property

/**onmbp:locals(1);**/

var b = {x:10};

function foo() {
	var a = 0;  /**bp:mbp("[Globals].b.x", "value", 'update', "b");**/ 
}
function bar() {
	b.x = 31;
}
foo();
bar();
bar(); /**bp:deleteMbp("b");**/
WScript.Echo("pass");
