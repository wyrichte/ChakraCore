// Tests that jumping from a non dead zone to a non dead zone region in a block is
// allowed when performing SetNextStatement.
// Bug #154071.

function letSetNextStatement()
{
    {
        var x = 0; 
        x++;        /**loc(A)**/
        let y = 2 ;
        y++;        /**bp:locals();setnext('A');disableBp()**/
        y;          /**bp:locals()**/
    }
}

letSetNextStatement();

WScript.Echo('PASSED');