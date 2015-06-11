// Tests that jumping between grandchildren is allowed
// when performing SetNextStatement and the source scope
// is deeply nested.

function letSetNextStatement()
{
    {
        let a = 0;
        a++;                
        {
            {
                let x = 1;
                {
                    let y = 1;
                    {
                        let z = 1;
                        {
                            ;           /**bp:setnext('A')**/
                            let g = 1;
                            g++;
                        }
                    }
                }
            }

            ;           
            let b = 1;
            b++;
        }

        {
            {
                ;                       /**loc(A)**/
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