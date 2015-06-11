// Tests that jumping out of a block into a non-dead zone forward region
// is not allowed when performing SetNextStatement.

function letSetNextStatement()
{
    {
        let a = 0;
        a++;
        {
            {
                ;           /**bp:setnext('A')**/
                let b = 1;
                b++;
            }

            ;
            let c = 2;
            c++;            /**loc(A)**/
        }
    }
}

letSetNextStatement();

WScript.Echo('PASSED');