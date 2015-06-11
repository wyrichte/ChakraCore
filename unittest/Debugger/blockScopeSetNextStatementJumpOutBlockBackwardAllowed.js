// Tests that jumping out of a block backwards
// is allowed when performing SetNextStatement.

function letSetNextStatement()
{
    {
        let a = 0;
        a++;                /**loc(A)**/
        {
            {
                ;           /**bp:setnext('A');disableBp()**/
                let b = 1;
                b++;        
            }

            ;
            let c = 2;
            c++;
        }
    }
}

letSetNextStatement();

WScript.Echo('PASSED');