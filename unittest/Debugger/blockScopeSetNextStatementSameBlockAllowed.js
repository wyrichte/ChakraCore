// Tests that jumping from a non dead zone to a dead zone region in a local block is
// allowed when performing SetNextStatement.

function letSetNextStatement()
{
    var a = 0;
    {
        ;           /**loc(A)**/
        let b = 1 ;
        a++;        /**bp:setnext('A');disableBp()**/
        b;          /**bp:locals()**/
    }
}

letSetNextStatement();

WScript.Echo('PASSED');