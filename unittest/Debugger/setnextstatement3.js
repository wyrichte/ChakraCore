// Validating setnextstatement after emulating step out.

function foo2()
{
    var k = 10;               /**bp:enableBp('E')**/
    k++;
}

// Testing setnext backward (for in)
function bar2()
{
    var m = 101;
    var s1 = [1,2,3];
    var sum = 0;            
    for(var i in s1)
    {
        sum = s1[i];                      /**loc(F): locals()**/    
    }
    m++;                    
    for (var i1 = 0; i1 < s1.length; i1++)
    {
        sum += s1[i1];                    /**loc(G): locals()**/    
    }
    
    WScript.Echo(Debug.isInJit());
    foo2();
    m--;                                 /**loc(E): locals();enableBp('F');removeExpr();enableBp('G');setnext('F');setnext('G')**/
    WScript.Echo(Debug.isInJit());
    var s2;
    s1++;                   
    s1--;
}

function foo3()
{
    var k = 10;                         /**bp:enableBp('H')**/
    k++;
}

// Testing setnext forward (for in)
function bar3()
{
    var m = 201;
    m++;                            
    WScript.Echo(Debug.isInJit());
    foo3();
    m--;                                /**loc(H): locals();enableBp('I');enableBp('J');setnext('I');setnext('J')**/
    WScript.Echo(Debug.isInJit());
    var s1 = [11,22,33];
    var sum = 0;
    for(var i in s1)
    {
        sum = s1[i];                    /**loc(I): locals()**/    
    }
    var s2;
    s1++;                               /**loc(J): locals()**/
    s1--;
}

function test1()
{
    /**disableBp('E');disableBp('F');disableBp('G');disableBp('H');disableBp('I');disableBp('J');**/
}

test1();
bar2();
bar3();
