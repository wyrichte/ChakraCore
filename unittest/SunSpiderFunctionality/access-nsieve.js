//var _sunSpiderStartDate = new Date();

// The Great Computer Language Shootout
// http://shootout.alioth.debian.org/
//
// modified by Isaac Gouy

function pad(number,width){
   var s = number.toString();
   var prefixWidth = width - s.length;
   if (prefixWidth>0){
      for (var i=1; i<=prefixWidth; i++) s = " " + s;
   }
   return s;
}

function nsieve(m, isPrime){
   var i, k, count;

   for (i=2; i<=m; i++) { isPrime[i] = true; }
   count = 0;

   for (i=2; i<=m; i++){
      if (isPrime[i]) {
         for (k=i+i; k<=m; k+=i) { isPrime[k] = false; /*WScript.Echo(k);*/ }
         count++;
      }
   }
   for (i=2; i<=m; i++){
		if (isPrime[i]) {
			WScript.Echo(i);
		}
	}
	//To remove the for loop when the below line can be printed...
	//WScript.Echo(isPrime.toString());
   return count;
}

function sieve() {
    for (var i = 1; i <= 3; i++ ) {
        var m = (1<<i)*10000;
        var flags = Array(m+1);
        nsieve(m, flags);
    }
}

sieve();


//var _sunSpiderInterval = new Date() - _sunSpiderStartDate;
//WScript.Echo("### TIME:", _sunSpiderInterval, "ms");
