var x = [1, 2, 2, 4, 5, +0, -0, NaN, 0, true, true , false]

for(i=-3; i < 15;i++)
{
   WScript.Echo(x.indexOf(i));
   for(j=-3; j< 15;j++)
   {
        WScript.Echo(x.indexOf(x[i],j)); 
        WScript.Echo(x.indexOf(i,j)); 
   }
}

var b = function(){};
b.prototype = Array.prototype;

var y = new b();

var z = new Object();
var a = new Object();

y[0] = "abc";
y[1] = "def";
y[2] = "efg";
y[3] = true;
y[4] = true;
y[5] = false;
y[6] = a;
y[7] = a;
y[8] = null;

y.length = 10;

WScript.Echo(y.indexOf("abc"));
WScript.Echo(y.indexOf("abc", 3));
WScript.Echo(y.indexOf("abc", 2));
WScript.Echo(y.indexOf("abc", -2));

WScript.Echo(y.indexOf("efg"));
WScript.Echo(y.indexOf("efg", 6));
WScript.Echo(y.indexOf("efg", 1));
WScript.Echo(y.indexOf("efg", -3));

WScript.Echo(y.indexOf("xyg"));
WScript.Echo(y.indexOf("esg", 2));
WScript.Echo(y.indexOf("eag", 2));
WScript.Echo(y.indexOf("", -2));

WScript.Echo(y.indexOf(true));
WScript.Echo(y.indexOf(false));
WScript.Echo(y.indexOf(new Boolean(true)));

WScript.Echo(y.indexOf(a , 6));
WScript.Echo(y.indexOf(a , 1));
WScript.Echo(y.indexOf(a ));
WScript.Echo(y.indexOf(b));

WScript.Echo(y.indexOf(null));


WScript.Echo(y.indexOf());

//implicit calls
var a ;
var arr = [10];
Object.defineProperty(Array.prototype, "4", {configurable : true, get: function(){a = true; return 30;}});
a = false;
arr.length = 6;
var f = arr.indexOf(30);
WScript.Echo(a);

//Float array with gaps
var floatarr = new Array(5.5, 5.6);
floatarr[6] =  5.6;
WScript.Echo(floatarr.indexOf(5.7));

// Cases where we do/don't have to resume after failing to find the value in the head segment.
// Run with -forcearraybtree to really stress these.
var gap = [0, 1];
WScript.Echo(gap.indexOf(4));
Array.prototype[2] = 'swine';
WScript.Echo(gap.indexOf('swine'));
gap[5] = 4;
WScript.Echo(gap.indexOf('swine'));
WScript.Echo(gap.indexOf(4));

gap = [0, 1.1];
WScript.Echo(gap.indexOf(4));
Array.prototype[2] = 'hogs';
WScript.Echo(gap.indexOf('hogs'));
gap[5] = 4;
WScript.Echo(gap.indexOf(4));
WScript.Echo(gap.indexOf('hogs'));

gap = [0, 'gooey'];
WScript.Echo(gap.indexOf(4));
Array.prototype[2] = 4;
WScript.Echo(gap.indexOf(4));
gap[5] = 4;
WScript.Echo(gap.indexOf(4));
delete Array.prototype[2]
WScript.Echo(gap.indexOf(4));
