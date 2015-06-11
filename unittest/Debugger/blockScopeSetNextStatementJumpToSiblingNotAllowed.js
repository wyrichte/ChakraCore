// Tests that jumping between siblings is not allowed
// when performing SetNextStatement and jumping across
// an initialization.

function letSetNextStatement()
{
    {
        let a = 0;
        a++;
        {
            ;           /**bp:setnext('A')**/
            let b = 1;
            b++;
        }

        let c = 2;
        c++;

        {
            ;
            let b = 1;  /**loc(A)**/
            b++;
        }
    }
}

letSetNextStatement();

WScript.Echo('PASSED');