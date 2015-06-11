let x;
WScript.Echo(x);
{
    let x = 5;
    WScript.Echo(x);
}
WScript.Echo(x);

for (var a in this) {
    WScript.Echo(a);
}
WScript.Echo();


function f() {
    const x = 'a';
    WScript.Echo(x);
    if (1 > 0)
    {
        let x;
        WScript.Echo(x);
    }
    WScript.Echo(x);
    WScript.Echo(f);
}f();

WScript.Echo(x);