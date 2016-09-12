
var a = {
    demo: function() {
    },
    sample: 1,
    get e() {},
    set e(value) {}
}

a;
a; /**bp:evaluate('a', 2, LOCALS_FULLNAME)**/
a;
a; /**bp:evaluate('a', 1, LOCALS_FULLNAME)**/
a;

function foo(){
    this.x = 1;
    this; /**bp:evaluate('arguments', 1, LOCALS_FULLNAME)**/
    this;
}
foo.x = 2;
foo.prototype.x = 3;


var __foo  = new foo();

__foo;
__foo; /**bp:evaluate('foo', 1, LOCALS_FULLNAME);evaluate('__foo', 1, LOCALS_FULLNAME)**/


var arr = new Array();
arr; /**bp:evaluate('arr', 2, LOCALS_FULLNAME)**/
arr;
arr; /**bp:evaluate('Array', 2, LOCALS_FULLNAME)**/

var a1 = [];
var b1 = function () { };
var c1 = {
    a: function () { },
    b: new Array(),
    get e() { },
    set e(value) { }            
}

c1.a["."] = 1,
c1.a["-"] = 2;
c1.a["-1"] = 3;
c1.a[""] = 4;

this[""] = {};
this[""]["a"] = 1;  
this["a"] = {};
this["a"][""] = 1 
this[null] = {};
this[null][null] = 1;
this;  /**bp:evaluate('this', 2, LOCALS_FULLNAME);**/
this;

WScript.Echo('pass');
