function etest(t, s) {
    try {
        eval(s);
        write(t + " parsed");
    }
    catch (e) {
        write(t + ":" + e.message);
    }
}

function write(a) {
    if (this.WScript == undefined) {
        document.write(a);
        document.write("</br>");
    }
    else
        WScript.Echo(a)
}


var expected;
var n = 10;                 var n_e = ["constructor", "hasOwnProperty", "isPrototypeOf", "propertyIsEnumerable", "toExpotential", "toFixed", "toLocalString", "toPercision", "toString", "valueOf"];
var s = "a";                var s_e = ["anchor", "big", "blink", "bold", "charAt", "charCodeAt", "concat", "constructor", "fixed", "fontcolor", "fontsize", "hasOwnProperty", "indexOf", "isPrototypeOf", "italics", "lastIndexOf", "length", "link", "localCompare", "match", "propertyIsEnumerable", "replace", "search", "slice", "small", "split", "strike", "sub", "substr", "substring", "sup", "toLocalLowerCase", "toLocalString", "toLocalUpperCase", "toLowerCase", "toString", "toUpperCase", "valueOf"];
var o = { a: 0, b: 1 };     var o_e = ["a", "b", "constructor", "hasOwnProperty", "isPrototypeOf", "propertyIsEnumerable", "toLocalString", "toString", "valueOf"];
var a = [1, 2, 3];          var a_e = ["concat", "constructor", "hasOwnProperty", "isPrototypeOf", "join", "length", "pop", "propertyIsEnumerable", "push", "reverse", "shift", "slice", "sort", "splice", "toLocalString", "toString", "unshift", "valueOf"];
function f() { }            var f_e = ["apply", "call", "constructor", "hasOwnProperty", "isPrototypeOf", "length", "propertyIsEnumerable", "prototype", "toLocalString", "toString", "valueOf"];
var r = /^\d$/;             var r_e = ["compile", "constructor", "exec", "global", "ignoreCase", "lastIndex", "multiline", "source", "test"];
var b = true;               var b_e = ["constructor", "hasOwnProperty", "isPrototypeOf", "propertyIsEnumerable", "toLocaleString", "toString", "valueOf"];


expected = s_e; (s. ab|ccd );

expected = n_e; (n.| ); 
expected = s_e; (s.| );
expected = o_e; (o.| );
expected = a_e; (a.| );
expected = f_e; (f.| );
expected = r_e; (r.| );
expected = b_e; (b.| );

expected = n_e; (n.a| ); 
expected = s_e; (s. ab|ccd );
expected = o_e; (o.d9Ad|A );
expected = a_e; (a.dd| );
expected = f_e; (f.dd|d );
expected = r_e; (r.dd|d );
expected = b_e; (b.dd|d );


// literals
expected = s_e; ("string".|);
expected = s_e; ("string". ab|ccd );

expected = r_e; (/^\d$/.|);
expected = r_e; (/^\d$/. a454|ccd );

expected = b_e; (true.| );
expected = b_e; (false. a4| );

expected = n_e; (NaN.| );
expected = n_e; (NaN. a4| );

expected = []; (null.| );
expected = []; (null. a4| );

expected = []; (undefined.| );
expected = []; (undefined. ad| );


// undeclared variables
U_a = 0;
U_b.b1.b2.b3 = "string";
U_c["c1"] = true;
this.U_d = 2.3;
this["U_e"] = 3;

function U_func()
{
  U_g = 0;|
  U_h = 0;  
}
U_j = 0;|