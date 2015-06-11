// Tests that jumping from a dead zone to a non dead zone region in a local block is
// not allowed when performing SetNextStatement.

function letSetNextStatement()
{
    var a = 0;
    {
        ;           /**bp:setnext('A');**/
        let b = 1 ;
        b++;        /**loc(A)**/
    }
}

letSetNextStatement();

WScript.Echo('PASSED');