// Tests that jumping between siblings is allowed
// when performing SetNextStatement and jumping backwards
// over an initialization.

function letSetNextStatement()
{
    {
        let a = 0;
        a++;
        {
            ;           /**loc(A)**/
            let b = 1;
            b++;
        }

        let c = 2;
        c++;

        {
            ;           /**bp:setnext('A');disableBp()**/
            let b = 1;
            b++;
        }
    }
}

letSetNextStatement();

WScript.Echo('PASSED');