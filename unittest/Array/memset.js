// Compares the value set by interpreter with the jitted code
// need to run with -mic:1 -off:simplejit

function test0(a,b,c,d,e,f,g)
{
 var i;
 for(i=0; i<100; i++)
 {
        a[i] = 0;
 }

 // reverse
 for(i=100; i>=5; i--)
 {
        b[i] = 0;
 }
 
 for(i=0; i<10; i++)
 {
        a[i] = b[i] = c[i] = 0;
 }


// unroll
 for(i=4; i<30; )
 {
        c[i] = 0;
		i++;
		c[i] = 0;
		i++;
 }

 //  missing value
 for(i=8; i<10; i++)
 {
        e[i] = 0;
 }

 f[5] = 3;
 for(i=0; i<6; i++)
 {
        f[i] = 0;
 }

 for(i=160; i<164; i++)
 {
        d[i] = 5;
 }
 
 for(i=10; i<160; i++)
 {
        d[i] = -1;
 }
 for(i=140; i< 150;i++)
 {
        g[i] = 0;
 }

 for(i=50; i< 130;i++)
 {
        g[i] = 1;
 }

 for(i=0; i< 100;i++)
 {
        g[i] = 2;
 }
 
}

var a = new Array();
var b = new Array();
var c = new Array();
var d = [1,2,3,4,5,6,7];
var e = [1,2,3,4,5,6,7];
var f = new Array();
var g = [1,2,3,4,5];

test0(a,b,c,d,e,f,g);


var a2 = new Array();
var b2 = new Array();
var c2 = new Array();
var d2 = [1,2,3,4,5,6,7];
var e2 = [1,2,3,4,5,6,7];
var f2 = new Array();
var g2 = [1,2,3,4,5];

test0(a2,b2,c2,d2,e2,f2,g2);

var i;
var passed = 0;

function compare_result(a,b) 
{
	var i;
	var passed = 1;
	for(i=0; i<a.length; i++)
		if(a[i] != b[i])
		{
			WScript.Echo(i+" "+a[i]+" "+b[i]);
			passed = 0;
		}
	return passed;
}

passed += compare_result(a, a2);
passed += compare_result(b, b2);
passed += compare_result(c, c2);
passed += compare_result(d, d2);
passed += compare_result(e, e2);
passed += compare_result(f, f2);
passed += compare_result(g, g2);

if(passed == 7)
{
	WScript.Echo("PASSED");
}
else
{
	WScript.Echo("FAILED");
}
