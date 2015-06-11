var map = new Map();
map.set(1,2) ;
map.set({}, function(){})
map; /**bp:evaluate('map', 2, LOCALS_FULLNAME)**/

var set = new Set();
set.add(1);
set.add([]);
set.add({});

var wm = new WeakMap(); 
function demo() {
    var k1 = [];
    wm.set(k1, map);
    wm;
    wm; /**bp:evaluate('wm', 2, LOCALS_FULLNAME)**/
}

var arr = new Array();
arr[-1] = this;


var l1 = () => {
 (() => {
    var l2 = 1;
    (() => {
        var l3 = 3;
    l3; /**bp:evaluate('l1', 2, LOCALS_FULLNAME); evaluate('l2', 1, LOCALS_FULLNAME); evaluate('l3', 1, LOCALS_FULLNAME)**/        
    })()
 
 })();    
}
l1();

l1; /**bp:evaluate('l1', 2, LOCALS_FULLNAME)**/

class A {
    sample() {
        this; /**bp:evaluate('A', 2, LOCALS_FULLNAME)**/
    }
}

var _classA = new A();
_classA.sample();


function bar() {
    this;
    arguments; /**bp:evaluate('arguments', 2, LOCALS_FULLNAME)**/
    this;
}
bar();

var obj = {
    a: () => {},
    b: function() {},
    c: new Map(),
    d: new Set(),
    e: new WeakMap(),
    f: function() {
        eval("this"); 
    }    
}
obj;
obj; /**bp:evaluate('obj', 2, LOCALS_FULLNAME)**/
obj;


WScript.Echo('pass');

