/*
    Set Next statement to a closure function - negative case
*/


function Run(){
    {
        var x = 1; 
        let a = 1;
        let b = 2;/**bp:setnext("bp1");locals(1)**/
        a++; 
        a; /**loc(bp1)**/
    }

    function foo(){
        let a = 1;
        let b = 2;/**bp:setnext("bp2");locals(1)**/
        return function(){
            var x = 1; /**bp:setnext("bp2")**/       
            a++; 
            a; /**loc(bp2)**/
            a++;
            a++;/**bp:locals(1)**/
        }
    }

    foo()();

    WScript.Echo('PASSED');
}
WScript.Attach(Run);

