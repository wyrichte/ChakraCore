var n = 1000000;
var my_heap = new ArrayBuffer(n>>2);
var a = new Int32Array(my_heap);
var  heap = new ArrayBuffer(n>>2);
var b = new Int32Array(heap);

function test(start)
{
	for (var i=start;i<start+n/2;i++) {
		b[i] = a[i];
	}
}


var i;
for(i=0;i<n;i++)
{
	a[i] = i;		
}

test(0);
test(n/2);
var passed = 1;

for(i=0;i<n;i++) if(a[i] != b[i]) passed = 0;

if(passed == 1)
{
	WScript.Echo("PASSED");
}
else
{
	WScript.Echo("FAILED");
}
