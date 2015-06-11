// Tests that jumping between grandchildren is not allowed
// when performing SetNextStatement and jumping across
// an initialization.

function letSetNextStatement()
{
    {
        let a = 0;
        a++;                
        {
            {
                ;           /**bp:setnext('A')**/
                let g = 1;
                g++;
            }

            ;           
            let b = 1;
            b++;
        }

        let c = 2;
        c++;

        {
            {
                ;           /**loc(A)**/
                let g = 1;
                g++;
            }

            ;
            let b = 1;
            b++;
        }
    }
}

letSetNextStatement();

WScript.Echo('PASSED');