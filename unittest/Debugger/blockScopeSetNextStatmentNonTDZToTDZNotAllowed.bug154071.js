// Tests that jumping from a dead zone to a non dead zone region in a block is not
// allowed when performing SetNextStatement.
// Bug #154071.

function letSetNextStatement()
{
    {
        var x ; /**bp:setnext('A');**/
        let y = 1 ;
        y++;    /**loc(A)**/
    }
}

letSetNextStatement();

WScript.Echo('PASSED');