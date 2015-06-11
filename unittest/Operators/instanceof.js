function write(v) { WScript.Echo(v + ""); }

function foo() {} 
function bar() {} ;

var fncs = [ "Object", "Function", "Array", "String", "Number", "Boolean", "Date", "RegExp", "foo", "bar"] ;

var f = new foo();
var b = new bar();

var objs = [ "new Object()",
            "f", "b", "foo", "String.fromCharCode", "Array.prototype.concat",            
            "[1,2,3]", "new Array()", "fncs",
            "'hello'", "new String('world')",
            "10", "10.2", "NaN", "new Number(3)", 
            "true", "false", "new Boolean(true)", "new Boolean(false)",
            "new Date()",
            "/a+/"
           ];

function check(str)
{
    try {
        write(str + " : " + eval(str));
    } catch (e) {
        write(" Exception: " + str + ". " + e.message);
    }
}

for (var i=0; i<objs.length ; i++) {
    for (var j=0; j<fncs.length; j++) {
        check(objs[i] + " instanceof " + fncs[j]);
    }
}

var count = 0;

for (var i=0; i<objs.length ; i++) {
    for (var j=0; j<objs.length; j++) {
        check(objs[i] + " instanceof " + objs[j]);
        /* TODO: Enable this test later once flags stuff is sorted out
        check("Array.prototype.join.prototype = " + objs[j] + "; count++;");
        check(objs[i] + " instanceof Array.prototype.join");
        */
    }
}