// Tests that jumping out of a block across nested initialization statements
// is not allowed when performing SetNextStatement and entering a non-dead
// zone region at the top scope.

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
            c++;
        }
        ;
        let d = 3;
        d++;                /**loc(A)**/
    }

}

letSetNextStatement();

WScript.Echo('PASSED');