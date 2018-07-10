var cc = WScript.LoadScript("function foo() { return new.target; }", "samethread");

function bar() {
}

if (Reflect.construct(cc.foo, [], bar) == bar) {
    print("PASSED");
} else {
    print("FAILED");
}