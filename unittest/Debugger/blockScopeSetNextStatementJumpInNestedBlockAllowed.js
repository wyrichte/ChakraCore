// Tests that jumping into a nested inner block's dead zone region is
// is allowed when performing SetNextStatement.

function letSetNextStatement()
{
    {
        let a = 0;
        a++;            /**bp:setnext('A');**/
        {
            {
                ;       /**loc(A)**/
                let b = 1;
                b++;        
            }

            let c = 2;
            c++;
        }
    }
}

letSetNextStatement();

WScript.Echo('PASSED');