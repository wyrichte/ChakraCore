
var testFoo = "test foo";

function testFunction(fn) { 
    var testString = "test string"
    var m = 10;
    m++;
    m++;                             
    fn();
}
var testObject = { p1: "blah" };
var testBoolean = true;
var testNumber1 = 3.14;
var testArray = [ 1, 2, 3 ];

function TestObject(p2, p3) {
    this.p2 = p2;
    this.p3 = p3;               
}

TestObject(1,3);
