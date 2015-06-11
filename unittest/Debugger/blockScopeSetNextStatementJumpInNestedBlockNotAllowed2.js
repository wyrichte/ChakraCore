// Tests that jumping into a nested inner block's dead zone region is
// is not allowed when performing SetNextStatement and one of its parents
// initializations is being leapt over.

function letSetNextStatement()
{
    {
        let a = 0;
        a++;                /**bp:setnext('A');**/
        {
            let c = 2;
            c++;
            {
                ;           /**loc(A)**/
                let b = 1;
                b++;
            }
        }
    }
}

letSetNextStatement();

WScript.Echo('PASSED');