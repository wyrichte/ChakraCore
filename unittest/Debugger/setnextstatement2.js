function f()
{
    var x = "1";
    x += " 2";   /**loc(first)**/
    x += " 3";   /**loc(second)**/
    x += " 4";   /**bp(third):setnext("first");disableBp("third")**/
    x += " 5";   /**bp:evaluate("x");**/
}
f();

// Validating setnext jump to block scope.
function f1()
{
    var a = 10;
    a++                      /**bp:setnext('test1');resume('continue')**/
    a++;
    
    {
        var c = 20;
        c++;                   /**loc(test1)**/
        c++;                   /**bp:locals()**/
    }
    
    c++;
    a++;                      /**bp:locals()**/

}

f1();

WScript.Echo("pass");
