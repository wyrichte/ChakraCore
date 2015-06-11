// Conditional mutation breakpoins complex expression

/**onmbp:evaluate('$propertyName$=="c" && $mutationType$=="update" && $newValue$==3');**/

obj = {
    a : 1
};
x = 2; /**bp:mbp("obj", 'properties', 'all', "MB1");**/
obj.b = 2; // Condition should evaluate to false as property is not c
obj.c = 1; // Condition should evaluate to false as mutationType is not update
obj.c = 2; // Condition should evaluate to false as newValue is not 3
obj.c = 3; // Condition should evaluate to true
WScript.Echo("pass");