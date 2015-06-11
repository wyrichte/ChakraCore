// Conditional mutation breakpoins $newValue$ property lookup

/**onmbp:evaluate('$newValue$.x.y == 1');evaluate('$newValue$[0] == 1');**/

obj = {
    a : 1
};
x = 2; /**bp:mbp("obj", 'properties', 'all', "MB1");**/
var obj2 = {};
obj2.x = {};
obj2.x.y = 1;
obj.b = obj2;
obj.c = [1];
WScript.Echo("pass");
