/*
    Simple feature testing 
*/

function write (x) { WScript.Echo(x); }
function Run ()
{
    {
        let x = '2';
        var x = 'var x';
        write(x); /**bp:locals();stack()**/
    }
    write(x); /**bp:locals();stack()**/
}

WScript.Attach(Run);


