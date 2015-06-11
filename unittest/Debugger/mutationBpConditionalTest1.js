// Conditional mutation breakpoins variables should only be available at mutation breakpoint

/**onmbp:evaluate('$newValue$');evaluate('$propertyName$');evaluate('$mutationType$');**/

obj = {
    a : 1
};

// At this break conditional OMBP variables should not be available
var x = 1; /**bp:evaluate('$newValue$');evaluate('$propertyName$');evaluate('$mutationType$');**/

x = 2; /**bp:mbp("obj", 'properties', 'all', "MB1");**/
obj.b = 2; // OMP - add
obj.a = 3; // OMP - change
delete obj.b; // OMP - delete
WScript.Echo("pass");