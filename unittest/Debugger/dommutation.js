/**ondmbp(resume_ignore):stack()**/
function f1() {
    WScript.ChangeDOMElement();
}
f1();

function f2() {
    WScript.ChangeDOMElement();
    function bar() {
        WScript.ChangeDOMElement()
    }
    bar();
}
f2();

var obj = {};
function f3() {
    try {
        throw obj;
    } catch (e) {
        WScript.ChangeDOMElement();
        if (e == obj) {
            print("PASSED");
        } else {
            print("FAILED");
        }
    }
}
f3();

function f4()  {
    var i = 0 /**bp:evaluate("WScript.ChangeDOMElement()", 1)**/
}
f4();

function f51()  {
    var i = 0 /**bp:evaluate("f52()", 1)**/
}
f51();

function f52() {
    WScript.ChangeDOMElement();
    return 10;
}
