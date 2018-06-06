var reg = /wwww/g;
var g = [{}];
function f(){
	var n = [1.1,2.2];
	n[0] = 1.1;
	g[0] = n;
	reg.exec("www");
	n[1] = 6.17651672645e-312;
}

f();
f();

z = {};
z.toString = function(){
	[].slice();
	g[0][0] = {};
}
reg.lastIndex = z;
f();
g[0][1];
print("passed")