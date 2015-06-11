// Object Mutation Breakpoint
// Multiple Set on same object, with search in [Scope] objects

/**onmbp:locals(1)**/

function noop(x)
{}

function test4()
{
	var obj = {a : 0};
	(function() {
		noop("Set a bp for update on a, should break once"); /**bp:mbp("obj.a", "value", "update", "obj");**/
		obj.a = 10;
		obj.b = 20; 		

		noop("Set it to none, next two should not break"); /**bp:mbp("obj.a", "value", "none", "obj");**/
		obj.a = "x";
		obj.b = {};		

		noop("Set it back to update, should break twice from below."); /**bp:mbp("obj", "properties", "update", "obj");**/
		obj.a = 65;
		obj.b = 100;
	})();
}

test4();

var a = 0;
WScript.Echo("pass");
