  // return double 
  // do while and while loops
  function AsmModule() {
    "use asm";      
    var x1 = 10;
    function f3(x,y){
        x = x|0;
        y = +y;
        var m = 1000;
        var n = 10;
        var z = 1.1;

       a: while( x < 30)
        {
            x = (x+1)|0
            if( x > 10)
            {
                do
                {
                    if(n > 50)
                        return +y;
                    x = (x+1)|0;
                    y = +(y * z) 
                    n = (n + 1)|0;
                }while(n < 100)
            }            
        }
        return +y;
    }
    
    function bar(k)
    {
        k = k|0;
        if( k > 5) 
            return +f3(1,1.5);  
        else 
            return 1.5;
    }
    
    return bar
}

var bar = AsmModule();
print(bar(1))  
print(bar(1))   
print(bar(10))   
print(bar(10))   