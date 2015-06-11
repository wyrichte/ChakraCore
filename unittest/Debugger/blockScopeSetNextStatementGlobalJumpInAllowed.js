// Tests that jumping into a block from global scope is allowed.

var x = 0;      /**bp:setnext('bp1')**/
{
    x;          /**loc(bp1)**/
    let a = 1;
    x;
}

WScript.Echo('PASSED');