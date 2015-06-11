// Tests that jumping into a nested inner block's non-dead zone region is
// is not allowed when performing SetNextStatement.

function letSetNextStatement()
{
    {
        let a = 0;
        a++;                /**bp:setnext('A');**/
        {
            {
                ;
                let b = 1;
                b++;        /**loc(A)**/
            }

            let c = 2;
            c++;
        }
    }
}

letSetNextStatement();

WScript.Echo('PASSED');