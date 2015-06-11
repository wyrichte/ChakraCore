function producer() {
   var x=3;
   var z=function() {
      WScript.Echo(x);
   }
   return z;
}

function consumer(f) {
    f();
}

var clo=producer();
consumer(clo);
