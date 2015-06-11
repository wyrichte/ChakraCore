// Setting OMBP on arguments formal object

/**onmbp:locals(1);stack()**/

function foo1(obj) {
    // This will not be set as there is no parent for value change
    var x = 1; /**bp:mbp("obj", 'value', 'delete', "MB1");**/
    delete obj.a;
}
foo1({ a : 1 });

function foo2(obj) {
    var x = 1; /**bp:mbp("obj", 'properties', 'add', "MB2");**/
    obj.b = 1;
}
foo2({ a : 1 });

function foo3(obj) {
    // This will not be set as there is no parent for value change
    var x = 1; /**bp:mbp("obj", 'value', 'add', "MB3");**/
    arguments[0] = 1;
}
foo3({ a : 1 });

function foo4(obj) {
    var x = 1; /**bp:mbp("obj", 'properties', 'delete', "MB4");**/
    delete arguments[0].b;
}
foo4({ a : 1, b : {} });

WScript.Echo("pass");