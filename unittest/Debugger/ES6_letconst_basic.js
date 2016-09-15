/*
    Simple feature testing 
    Let/Const appear as part of thr global object
    Explicit setFrame in absence of a function call
*/

let a = 1;
const b = 1;
let c = 2;
const d = 4;
{
    let a = 2;
    a; /**bp:
        locals(1);
        stack();
        **/
}
WScript.Echo('PASSED');/**bp:locals(1)**/