// Bug : 291582
// Validation that activation object scope, due to eval, will not mess up the variable visibility.

var a = 1;
{
    let a = 2;
    eval("");
}
var x = 10; /**bp:locals()**/

WScript.Echo("Pass");
