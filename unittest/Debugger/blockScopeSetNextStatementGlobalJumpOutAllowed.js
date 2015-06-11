// Tests that jumping from an inner block to global scope is allowed.

var x = 0;  
{
    x;          /**bp:setnext('bp1')**/
    let a = 1;
    x;
}
x++;            /**loc(bp1)**/

WScript.Echo('PASSED');