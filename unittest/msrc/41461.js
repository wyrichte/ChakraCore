function createModule() {
    'use asm';
    const a = 1.0;
    function f() {
        var b = a;
        var a = 0;
    }

    return f;
}
var f = createModule();
for(let i=0; i<1000; ++i)
{
    f();
}
print("Pass");
