// Setting OMBP on global object for value change, can't set as there is no parent - Support this as part of another WI

/**onmbp:locals(1);stack()**/

obj1 = { foo : {} };
var x = 1; /**bp:mbp("obj1", 'value', 'all', "MB1");**/
obj1 = { a : 1 };

var obj2 = { foo : {} };
x = 1; /**bp:mbp("obj2", 'value', 'all', "MB2");**/
obj2 = 1;

function foo1() {
    obj3 = { foo : {} };
    var x = 1; /**bp:mbp("obj3", 'value', 'all', "MB3");**/
    obj3 = Math;
}
foo1();

function foo2() {
    var obj4 = { foo : {} };
    var x = 1; /**bp:mbp("obj4", 'value', 'all', "MB4");**/
    obj4 = Math;
}
foo2();

WScript.Echo("pass");