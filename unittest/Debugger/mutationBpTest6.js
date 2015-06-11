// Object mutation breakpoint
// Delete property - simple

/**onmbp:locals(1);**/

(function(){
	var obj = {};
	obj.a = ":)"; /**bp:mbp("obj", "properties", "delete", "dela");**/
	delete obj.a; // should dump
})();

WScript.Echo("pass");