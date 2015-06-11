// Tests that jumping into an inner block's dead zone region is
// allowed when performing SetNextStatement.

function letSetNextStatement()
{
    {
        let a = 0;
        a++;            /**bp:setnext('A');**/
        {
            ;           /**loc(A)**/
            let b = 1;
            b++;
        }
    }
}

letSetNextStatement();

WScript.Echo('PASSED');