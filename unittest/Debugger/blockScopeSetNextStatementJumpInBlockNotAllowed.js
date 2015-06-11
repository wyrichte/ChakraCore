// Tests that jumping into an inner block's non-dead zone region is
// not allowed when performing SetNextStatement.

function letSetNextStatement()
{
    {
        let a = 0;
        a++;            /**bp:setnext('A');**/
        {
            ;
            let b = 1;
            b++;        /**loc(A)**/
        }
    }
}

letSetNextStatement();

WScript.Echo('PASSED');