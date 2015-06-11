// bug : 355097, functional issue when the forcedeferparse of the let vars.

function Run(){
    function foo() {  
        let a = "a";
        function bar() {}  
        a; /**bp:evaluate('a')**/ 
    }
    foo();
};
Run();
WScript.Attach(Run);  
WScript.Echo("Pass");  
