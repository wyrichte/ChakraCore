WScript.RegisterCrossThreadInterfacePS();

var testUndefined;
var testNull = null;
var testObject = { p1: "blah" };
var testBoolean = true;
var testBooleanObject = new Boolean(true);
var testDate = new Date(2011, 3, 24, 12, 49, 15, 478);
var testNumber1 = 3.14;
var testNumber2 = 8;
var testNumber3 = 0xFFFFFFFFFFFFFFFF;
var testNumberObject1 = new Number(3.14);
var testNumberObject2 = new Number(8);
var testNumberObject3 = new Number(0xFFFFFFFFFFFFFFFF);
var testRegExp = new RegExp();
var testArray = [ 1, 2, 3 ];
function testFunction() { WScript.Echo("Test"); }
var testString = "Blah blah blah";
var testStringObject = new String("Blah blah blah");
var getterSetterValue = "inital getter setter value";
function getter() { WScript.Echo(">>>In getter"); return getterSetterValue; }
function setter(value) { WScript.Echo(">>>In setter"); getterSetterValue = value; }
function TestObject(p2, p3) {
    this.p2 = p2;
    this.p3 = p3;
}
var testObject2 = new TestObject("bar", 42);
var numObj = {a: 1, b: 2, c:3};
var testBigObject =
{
    testUndefined: testUndefined,
    testNull: testNull,
    testObject: testObject,
    testBoolean: testBoolean,
    testBooleanObject: testBooleanObject,
    testDate: testDate,
    testNumber1: testNumber1,
    testNumber2: testNumber2,
    testNumber3: testNumber3,
    testNumberObject1: testNumberObject1,
    testNumberObject2: testNumberObject2,
    testNumberObject3: testNumberObject3,
    testRegExp: testRegExp,
    testArray: testArray,
    testFunction: testFunction,
    testString: testString,
    testStringObject: testStringObject
};
