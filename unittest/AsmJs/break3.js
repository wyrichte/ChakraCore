  // do while and while loops and break
  function AsmModule() {
    "use asm";      
    var x1 = 10;
    function f3(x,y){
        x = x|0;
        y = +y;
        var m = 1000;
        var n = 10;
        var z = 1.1;

       a: while(x < 30)
        {
            x = (x+1)|0
            if( x > 10)
            {
                n = 0;
                do
                {
                    if(n > 50)
                        break a;
                    x = (x+1)|0;
                    y = +(y * z) 
                    n = (n + 1)|0
                }while(n < 100)
            }            
        }
        return +y;
    }
    
    return f3
}

var f3 = AsmModule();
print(f3  (1,1.5))  
print(f3  (1,1.5))   