// Tests that jumping into a block from global scope is not allowed.

var x = 0;      /**bp:setnext('bp1')**/
{
    x;
    let a = 1;
    x;          /**loc(bp1)**/
}

WScript.Echo('PASSED');