function etest(t, s) {
    try {
        eval(s);
        write(t + " parsed");
    }
    catch (e) {
        write(t + ":" + e.message);
    }
}

/* This is a multi line comment
 *
 */

function write(a) {
    if (this.WScript == undefined) {
        document.write(a);
        document.write("</br>");
    }
    else
        WScript.Echo(a)
}


// This is a single line comment


var expected;
var n = 10;                 var n_e = ["constructor", "hasOwnProperty", "isPrototypeOf", "propertyIsEnumerable", "toExpotential", "toFixed", "toLocalString", "toPercision", "toString", "valueOf"];
var s = "a";                var s_e = ["anchor", "big", "blink", "bold", "charAt", "charCodeAt", "concat", "constructor", "fixed", "fontcolor", "fontsize", "hasOwnProperty", "indexOf", "isPrototypeOf", "italics", "lastIndexOf", "length", "link", "localCompare", "match", "propertyIsEnumerable", "replace", "search", "slice", "small", "split", "strike", "sub", "substr", "substring", "sup", "toLocalLowerCase", "toLocalString", "toLocalUpperCase", "toLowerCase", "toString", "toUpperCase", "valueOf"];
var o = { a: 0, b: 1 };     var o_e = ["a", "b", "constructor", "hasOwnProperty", "isPrototypeOf", "propertyIsEnumerable", "toLocalString", "toString", "valueOf"];
var a = [1, 2, 3];          var a_e = ["concat", "constructor", "hasOwnProperty", "isPrototypeOf", "join", "length", "pop", "propertyIsEnumerable", "push", "reverse", "shift", "slice", "sort", "splice", "toLocalString", "toString", "unshift", "valueOf"];
function f() { }            var f_e = ["apply", "call", "constructor", "hasOwnProperty", "isPrototypeOf", "length", "propertyIsEnumerable", "prototype", "toLocalString", "toString", "valueOf"];


expected = s_e; (s. ab|ccd );

expected = n_e; (n.| ); 
expected = s_e; (s.| );
expected = o_e; (o.| );
expected = a_e; (a.| );
expected = f_e; (f.| );

expected = n_e; (n.a| ); 
expected = s_e; (s. ab|ccd );
expected = o_e; (o.d9Ad|A );
expected = a_e; (a.dd| );
expected = f_e; (f.dd|d );
