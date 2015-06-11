// Tests that jumping out of a block across nested initialization statements
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

            ;               
            let c = 2;
            c++;            
        }
        ;                   /**loc(A)**/
        let d = 3;  
        d++;
    }

}

letSetNextStatement();

WScript.Echo('PASSED');