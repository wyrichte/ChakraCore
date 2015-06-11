/*
    Local Insepction Testing
*/
var num = 1;
var _num = new Number(1);

var bool = false;
var _bool = new Boolean(false);

var str = "test";
var _str = new String("test");

var arr = [];
var _arr = new Array();

function fn() { };
var _fn = function () { };
var __fn = new Function();

var intl = this.Intl;

//not present in jshost
//var webWorker = new Worker();

//var map = Map();
var _map = new Map();

//var set = Set();
var _set = new Set();

//var weakmap = WeakMap();
var _weakmap = new WeakMap();


var obj = {};
var _obj = new Object();

var math = this.Math;

var dt = new Date();

try{
    throw new Error();
} catch (e) {
    e != null; /**bp:evaluate('e',2)**/
}

try {
    throw new EvalError();
} catch (e) {
    e != null; /**bp:evaluate('e',2)**/
}

try {
    throw new RangeError();
} catch (e) {
    e != null; /**bp:evaluate('e',2)**/
}

try {
    throw new ReferenceError();
} catch (e) {
    e != null; /**bp:evaluate('e',2)**/
}

try {
    throw new SyntaxError();
} catch (e) {
    e != null; /**bp:evaluate('e',2)**/
}

try {
    throw new TypeError();
} catch (e) {
    e != null; /**bp:evaluate('e',2)**/
}

try {
    throw new URIError();
} catch (e) {
    e != null; /**bp:evaluate('e',2)**/
}

WScript.Echo('PASSED'); /**bp:locals(3)**/


