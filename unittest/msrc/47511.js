	let bigarr = [1.1,2.2];
	bigarr.length = 3;

	let victim = [1.1,2.2,3.3];

	let ut8 = new Uint8Array(0x40000000);   //need to bigger than 0x40000000 on x86 and x64 need to < 0(bigger than 0x7fffffff)
	ut8.__defineGetter__("length", Object.prototype.valueOf);   

function jit(arr){
	let bb = arr.length;  //disable |LdLenIntSpec|

	victim[1] = 1.1;
	let aa = ut8.length;
	victim[0] = 6.18764541553e-312;  //0x12398765432;
	
}


bigarr.length = 0x80000001;
for(let i = 0;i < 0x80001; i++){
	jit(bigarr);
}


ut8.__defineGetter__("length", function(){
	Array.isArray([]);
	victim[0] = {};
});

jit(bigarr);

if (victim[0] === 6.18764541553e-312)
{
    WScript.Echo('pass');
}

