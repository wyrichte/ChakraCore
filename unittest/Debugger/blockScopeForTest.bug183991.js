// Tests that a let iterator at global scope reflects
// properly in the debugger when the loop is at global
// scope.
// Bug #183991.

var a = {x:2, y:1};
for(let y in a){
   y; /**bp:locals(1)**/
}

WScript.Echo('PASSED');