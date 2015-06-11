function write(v) { WScript.Echo(v + ""); }

var a = new Array();
a[0] = 20.78925678;

var values = [ 0, -0, +0, -0.0, +0.0, 0.0,
               1, -1, +1, -1.0, +1.0, 1.0,
               0.25, -0.25, 0.5, -0.5, +0.99, 
               10, -10, 10.5, -10.5,
               NaN, new Number(NaN), Number.NaN, new Number(Number.NaN),
               Number.MAX_VALUE, new Number(Number.MAX_VALUE),
               Number.MIN_VALUE, Number(Number.MIN_VALUE),
               Number.NEGATIVE_INFINITY, Number(Number.NEGATIVE_INFINITY),
               Number.POSITIVE_INFINITY, new Number(Number.POSITIVE_INFINITY),
               undefined, null,
               Math.E, Math.LN10, Math.LN2, Math.LOG2E, Math.LOG10E,
               Math.PI, Math.PI/2, Math.PI/4,
               Math.SQRT1_2, Math.SQRT2,
               "1.0", "+1.23", "-24.32", "2e-3", "-2e-3243", "1000",
               a[0]
            ];


var funcs = [ "abs", "acos", "asin", "atan", "atan2",
              "ceil", "cos", "exp", "floor", "log",
              "max", "min", "pow", "round", "sin",
              "sqrt", "tan"
            ];
 
var twoArgFuncs = ["atan2", "max", "min", "pow" ];
var str = "";
var i,j,k;
           
for (i=0; i<funcs.length; i++) {
    for (j=0; j<values.length; j++) {
        str = "Math." + funcs[i] + "(" + values[j] + ")";
        write(str + " : " + eval(str));
    }
}
               
for (i=0; i<twoArgFuncs.length; i++) {
    for (j=0; j<values.length; j++) {
        for (k=0; k<values.length; k++) {
            str = "Math." + twoArgFuncs[i] + "(" + values[j] + ", " + values[k] + ")";
            write(str + " : " + eval(str));
        }
    }
}