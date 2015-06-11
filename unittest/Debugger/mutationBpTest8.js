// Setting OMBP on global object for properties change

/**onmbp:locals(1);stack()**/

obj1 = { foo : {} };
var x = 1; /**bp:mbp("obj1", 'properties', 'add', "MB1");**/
obj1.bar = { a : 1 };

var obj2 = { foo : {} };
x = 1; /**bp:mbp("obj2", 'properties', 'update', "MB2");**/
obj2.foo = 1;

function foo1() {
    obj3 = { foo : {} };
    var x = 1; /**bp:mbp("obj3", 'properties', 'delete', "MB3");**/
    delete obj3.foo;
}
foo1();

function foo2() {
    var obj4 = { foo : {} };
    var x = 1; /**bp:mbp("obj4", 'properties', 'all', "MB4");**/
    obj4.foo = Math.sin;
}
foo2();

WScript.Echo("pass");