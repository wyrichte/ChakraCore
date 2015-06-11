// Conditional mutation breakpoins variables $propertyName$ and $mutationType$ can't be changed

/**onmbp:evaluate('$newValue$=1');evaluate('$propertyName$="c"');evaluate('$mutationType$="delete"');**/

obj = {
    a : 1
};
x = 2; /**bp:mbp("obj", 'properties', 'all', "MB1");**/
obj.b = 2; // OMP - add
WScript.Echo("pass");