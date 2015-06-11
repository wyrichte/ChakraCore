/**exception(resume_ignore):logJson('Hit Exception'); stack();locals(1)**/
var a1 = 10;

function foo() {
    throw "Foo Error";
    return a1 * 5;
}

function bad_sub_expression() {
    a1; 
    var obj = { 
        a: a1,  
        b: a1 + 20, 
        c:  {
                ca: a1 * 2, 
                cb: foo1(),      // Calling foo1() that is undefined
                cc: a1
            }
        };
    WScript.Echo(JSON.stringify(obj)); /**bp:  logJson('BP1'); stack(); locals(); **/
}

function sub_expression_exception() {
    a1; 
    var obj = { 
        a: a1,  
        b: a1 + 20, 
        c:  {
                ca: a1 * 2, 
                cb: foo(),      // Calling foo() that throws
                cc: a1
            }
        };
    WScript.Echo(JSON.stringify(obj)); /**bp:  logJson('BP2'); stack(); locals(); **/
}

bad_sub_expression();
sub_expression_exception();
