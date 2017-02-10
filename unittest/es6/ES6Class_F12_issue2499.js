var e = WScript.LoadScript("var A = class A {}");

var count = 0;

function foo() {
    var a = new e.A();
    count++;
    if (!(a instanceof e.A)) {
        console.log("Failed");
    } else if (count < 1000) {
        WScript.SetTimeout(bar, 0);
    } else {
        console.log("Pass");
    }
}

function bar() {
    if (count < 500) {
        foo();
    } else {
        WScript.Attach(foo);
    }
}

bar();
