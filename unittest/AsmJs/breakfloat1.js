  // nested for loop with break
  function AsmModule(stdlib) {
    "use asm";      
    var x1 = 10;
    var fr = stdlib.Math.fround;
    function f3(x,y){
        x = fr(x);
        y = fr(y);
        var m = 1000;
        var n = 20;
        var z = 11;

       a: for( m = 0; m < 50 ; m = m+1)
        {
            x = fr(x + y);
            for( n = 0; n < 100 ; n = n+1)
            {
                if(n >  50)
                    break a;
                x = fr(x + y);
                z = (z+1)|0;
            }           
        }
        return fr(x);
    }
    
    return f3
}
var stdlib = {Math:Math}
var f3 = AsmModule(stdlib);
print(f3  (1,1.5))  
print(f3  (1,1.5))   