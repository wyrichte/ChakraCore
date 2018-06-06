for (var x = 0 ; x < 1000; x++) {
    try {
        new f()
    } catch (e) {
    }
}
function f() {
  eval('(function(a,b,c){"use asm";var bug=new a.Uint8Array(c);function foo(x,y){x=x|0;y=+y;} return {foo: foo}})()');
}

print("passed");