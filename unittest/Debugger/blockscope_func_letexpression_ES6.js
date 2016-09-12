/*
    Function hoisting for let function expression
 */



var x = 1; /**bp:locals(1)**/
WScript.Echo(baz);/**bp:setExceptionResume('ignore')**/
//baz in undefined here
{
    //function expression
    let baz = function() { return 'baz';} 
    WScript.Echo(baz);/**bp:locals(1)**/
}
WScript.Echo(baz);
WScript.Echo('PASSED')/**bp:locals(1)**/
