(0, {2:{v:"13"}})   // construct used in google maps resulting in sub expression not wrapped in StatementMap

var a1 = 10;

function foo() {
    return a1 * 5;
}

function breaking_on_sub_expression() {
    a1; /**bp: logJson('BP1'); stack(); locals(); **/
    var obj = { 
        a: a1,  
        b: a1 + 20, /**bp: logJson('BP2'); stack(); locals(); evaluate('a1 = 33'); **/
        c:  {
                ca: a1 * 2, 
                cb: foo(), /**bp:  logJson('BP3'); stack(); locals(); **/
                cc: a1
            }
        };
    WScript.Echo(JSON.stringify(obj)); /**bp:  logJson('BP4'); stack(); locals(2); **/
}

breaking_on_sub_expression();

function breaking_on_sub_expression_arguments(obj) {
    WScript.Echo(JSON.stringify(obj)); /**bp:  logJson('BP7'); stack(); locals(2); **/
}

breaking_on_sub_expression_arguments( { 
        a: a1,  
        b: a1 + 20, /**bp: logJson('BP5'); stack(); locals(); evaluate('a1 = 22'); **/
        c:  {
                ca: a1 * 2, 
                cb: foo(), /**bp: logJson('BP6'); stack(); locals(); **/
                cc: a1
            }
    }
);

var obj2 = { 
    a: a1,      /**bp: logJson('BP8'); stack(); locals(); evaluate('a1 = 11'); **/
    b: a1 + 20, 
    c:  {
            ca: a1 * 2,
            cb: foo(), 
            cc: { 
                da: a1 * 3, /**bp: logJson('BP9'); stack(); locals(); **/ 
                db: a1
            }
        }
    };
WScript.Echo(JSON.stringify(obj2)); /**bp:  logJson('BP10'); stack(); locals(2); **/

function breaking_on_sub_expression_try() {
    a1; /**bp: logJson('BP11'); stack(); locals(); **/
    try {
        var obj = { 
            a: a1,  
            b: a1 + 20, /**bp: logJson('BP12'); stack(); locals(); evaluate('a1 = 44'); **/
            c:  {
                    ca: a1 * 2, 
                    cb: foo(), /**bp:  logJson('BP13'); stack(); locals(); **/
                    cc: a1
                }
            };
        WScript.Echo(JSON.stringify(obj)); /**bp:  logJson('BP14'); stack(); locals(2); **/
        throw "Dummy Test Error";
    }
    catch(ex) {
        var obj = { 
            a: a1,  
            b: a1 + 20, /**bp: logJson('BP15'); stack(); locals(); evaluate('a1 = 55'); **/
            c:  {
                    ca: a1 * 2, 
                    cb: foo(), /**bp:  logJson('BP16'); stack(); locals(); **/
                    cc: a1
                }
            };
        WScript.Echo(JSON.stringify(obj)); /**bp:  logJson('BP17'); stack(); locals(2); **/
    }
    finally {
        var obj = { 
            a: a1,  
            b: a1 + 20, /**bp: logJson('BP18'); stack(); locals(); evaluate('a1 = 66'); **/
            c:  {
                    ca: a1 * 2, 
                    cb: foo(), /**bp:  logJson('BP19'); stack(); locals(); **/
                    cc: a1
                }
            };
        WScript.Echo(JSON.stringify(obj)); /**bp:  logJson('BP20'); stack(); locals(2); **/
    }
}

breaking_on_sub_expression_try();

var arr = [ { 
            a: a1,  
            b: a1 + 20, /**bp: logJson('BP21'); stack(); locals(); evaluate('a1 = 77'); **/
            c:  {
                    ca: a1 * 2, 
                    cb: foo(), /**bp:  logJson('BP22'); stack(); locals(); **/
                    cc: a1
                }
            },
            { 
            a: a1,  
            b: a1 + 20, /**bp: logJson('BP23'); stack(); locals(); stack(); evaluate('a1 = 88'); **/
            c:  {
                    ca: a1 * 2, 
                    cb: foo(), /**bp:  logJson('BP24'); stack(); locals(); **/
                    cc: a1
                }
            } ];
            
WScript.Echo(JSON.stringify(arr[0])); /**bp:  logJson('BP25'); stack(); locals(2); **/
WScript.Echo(JSON.stringify(arr[1])); /**bp:  logJson('BP26'); stack(); locals(2); **/
