function f() {
    var sum = 0;
    var k = 10;
    for (var i = 0; i < 15; i++) {
        sum += (i + 1);
        sum;                    /**bp:locals();removeExpr();setnext(12,1);resume('continue')**/
    }
    k += 20;                    /**bp:locals()**/
    k += 20;
    var s = "s"
    k += sum;                   /**bp:locals()**/

    var m = "m";
    m;                          /**bp:locals();removeExpr();setnext(22,1);setnext(3,1);resume('step_over');**/
}
f();

function f2(a) {
    var j = 10;
    j++;
    return j + a;
}

f2();

function f3() {
    var j = "message";
    j;                             /**bp:locals();removeExpr();setnext(40,1);resume('step_over')**/
    var m = 10;
    try {
        abc.def = 20;             /**bp:locals();**/
        j = "succeed";
    }
    catch (ex) {
        j = ex.message;
        ex;
    }

    j;

    return j;
}

f3();

var ret = 0;
function f4() {
    var input = 10;
    input++;
    ret += f2(input);
    ret;                         /**bp:locals(1);removeExpr();setnext(56,1);resume('step_over')**/
    return ret;                 /**bp:locals(1);**/
}
f4();
WScript.Echo("pass");