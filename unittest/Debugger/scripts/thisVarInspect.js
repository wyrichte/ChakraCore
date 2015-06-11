function test1() {
    return 10;
}

function test2() {
    this.x = 10;
    return this.x;
}

function test3() {
    return "test3";
}

var a1 = test1();
var a2 = new test2();

var testObj = {t1:1, t2:"t2"};
var a3 = test3.apply(testObj);

var a4 = test3.apply(this);

