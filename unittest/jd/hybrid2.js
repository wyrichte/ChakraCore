function h(i) {
    return i + 1;
}
function g() {
    var sum = 0;
    for (var i = 0; i < arguments.length; i = h(i)) { /**bp:locals(1);stack();resume('step_over')**/
        sum++;
    }
    return sum; /**bp:locals(1);stack();resume('step_into')**/
}
function f(obj) {
    for (var i = 0; i < 3; ++i)
        eval("obj.prop" + i + " = i; /**bp:locals(1);stack()**/"); /**bp:locals(1);stack()**/

    for (var p in obj) {
        obj[p] += g(p,p,p,p);    /**bp:locals(1);stack();resume('step_into')**/
    }
}
f({a: 1, b: 2});
f([1,2,3,4,5]);
WScript.Echo("PASSED");
