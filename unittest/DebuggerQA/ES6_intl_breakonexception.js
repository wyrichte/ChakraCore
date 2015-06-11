/*
 When break on exception is supported, Intl internal stack should not be shown
*/

/**exception:stack()**/
function testFirstChanceException() {
    var formatter = new Intl.NumberFormat("INVALID CURRENCY CODE");/**bp:setExceptionResume('ignore')**/
}

testFirstChanceException();

WScript.Echo('PASSED');