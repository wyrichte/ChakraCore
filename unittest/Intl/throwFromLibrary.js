/**exception(resume_ignore):stack();**/

function testFirstChanceException() {
    var formatter = new Intl.NumberFormat("INVALID CURRENCY CODE");
}

testFirstChanceException();

WScript.Echo("pass");