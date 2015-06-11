
function demo() {    
    "use strict";    
    function sample() {}    
    sample;
    sample; /**bp:evaluate('sample',1)**/
    sample;
}

demo();

function baz() {}

var foo = (a) => {
    a; 
    a; 
    a;
}

var bar = (arg) => {
    arg;
    foo(arg); /**bp:stack();resume('step_into');locals();stack();**/
    arg;
    this; /**bp:evaluate('this', 1)**/
    arg;    
}

bar(1);
baz;
baz; /**bp:evaluate('baz',1)**/
baz;

WScript.Echo('pass');/**bp:evaluate('baz',2);evaluate('foo',2);evaluate('bar',2)**/