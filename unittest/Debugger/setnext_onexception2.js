// setnextstatement backward on native frame (exception on top frame)

/**exception(resume_ignore): locals();enableBp('A');enableBp('B');removeExpr();setnext('A');setnext('B');**/

function foo()
{
        var j = 10;
        j++;
        var s1 = [1,2,3];
        var sum = 0;            
        for (var i in s1)
        {
            sum = s1[i];                      /**loc(A): locals()**/    
        }

        for (var i1 = 0; i1 < s1.length; i1++)
        {
            sum += s1[i1];                    /**loc(B): locals()**/    
        }
        
        WScript.Echo("foo : isInJit : " +Debug.isInJit());
		abc.cause.exception = 10;
		
        var c = 20;
        c++;
        WScript.Echo("foo : isInJit : " +Debug.isInJit());
}

function test1()
{
	/**disableBp('A');disableBp('B');**/

}

test1();
foo();
WScript.Echo("pass");