
function loopfunc(obj, i8, b) {
    // JIT loop body
    for (var a in i8) {
        for (var i = 1; i <= 1; i++) {
            a = b;
        }
        obj.p = a;
        a = ++b; // b gets stack allocated
    }
}

function foo(counter)
{
    if(counter == 0) return;
    foo(counter - 1);
}

function test0() {
    var obj = {};
    var i8 = new Int8Array(0x10000);
    var b = 0x82468246;
    loopfunc(obj, i8, b);
    // call recursive function to overwrite stack
    foo(100);
    obj.p.crash; // access stack allocated property
}
test0();

WScript.Echo("Pass");
