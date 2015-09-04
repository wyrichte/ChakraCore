var a = 1;
function foo1() {
    var b = 2; a, a, b, a;b++;
}

function foo2() {
    a++; a, a, b, a;
}

foo1(); /**bp:resume('step_into');dumpBreak();resume('step_into');dumpBreak();**/
foo2();