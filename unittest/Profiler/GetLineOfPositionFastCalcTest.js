var g = 0;

function firstFunc() {
    g++;
}
firstFunc();

function secondFunc() {
    var a = 0;
    function innerFunc() {
        g++;
    }
    g++;
}
secondFunc();

function thirdFunc() {
    g++;
}
thirdFunc();

var first = Debug.getLineOfPosition(firstFunc)
var second = Debug.getLineOfPosition(secondFunc)
var third = Debug.getLineOfPosition(thirdFunc)

WScript.Echo(first.line + ',' + first.column)
WScript.Echo(second.line + ',' + second.column)
WScript.Echo(third.line + ',' + third.column)