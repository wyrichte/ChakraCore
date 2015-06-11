function test1(){
	var obj = {};
	obj.k = 10;
	obj.f3 = function () {/*temp function definition*/};
	obj.b3 = function () {/*temp function definition*/};
	obj.a = "a";
	obj.c = 3;
	obj[2] = 1;
	obj[0] = 5;
	obj.abc = "abc";
	obj.abcd = "abcde";
	obj.xyz = "xyz";
	obj[3] = "3"
	obj.x = "xyz";
	obj.x3 = function () {/*temp function definition*/};
	
	return obj;
}
test1();

function test2(){
	var obj = {};
	obj.ob = {d:10,c:20};
	obj.a2 = "a2";
	obj.a = "a";
	obj.cc = {x:10,z:20};

	return obj;
}

test2();
