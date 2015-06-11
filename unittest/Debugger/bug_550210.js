// Validate the bug : 550210. Empty string and escape sequence in the string.
var obj = {}
obj["\n"] = {}
obj["\n"]["b"] = function () {
    obj; /**bp:stack()**/
}

obj[""] = {}
obj[""]["a"] = function () {
    obj; /**bp:stack()**/
}
obj["2"] = {}
obj["2"]["a"] = function () {
    obj; /**bp:stack()**/
}
obj;                  /**bp:evaluate('obj',2, LOCALS_FULLNAME)**/
obj["\n"]["b"]();
obj[""]["a"]();
obj["2"]["a"]();

WScript.Echo("Pass");