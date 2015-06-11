
var testFoo = "test foo";

function testFunction(fn) { 
    var testString = "test string"
    var m = 10;
    m++;
    m++;                             /**bp:stack();locals()**/
    fn();
}
