// Tests that jumping out of a block into a dead zone forward region
// is allowed when performing SetNextStatement.

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

            ;               /**loc(A)**/
            let c = 2;
            c++;
        }
    }
}

letSetNextStatement();

WScript.Echo('PASSED');