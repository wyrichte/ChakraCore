//
// Allocate an initial array and set some fields.
//

var a1 = new Array();
a1[2] = "B";
a1[3] = "C";


//
// Cause the array to grow in storage.
//

a1[20] = "T";


//
// Dump the contents of the array, ensuring that uninitialized fields are properly set to
// 'undefined'.
//

for (var idx = 0; idx < a1.length; idx++)
{
    var val = a1[idx];
    if (val == undefined)
    {
        WScript.Echo("undefined");
    }
    else if (val == null)
    {
        WScript.Echo("null");
    }
    else
    {
        WScript.Echo(val);
    }
}
