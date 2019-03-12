function jit1(arg, index)
{
    let tmp = {"0":0x1234,"1":0x1234};   
    
	if(flag){
		arg = tmp;
    }

	arg[1] = 1.1;
	tmp[index] = 2;	
	arr2[0] = arg[0];	//give me a missingValue
}

function jit2(arg, index)
{
    let tmp = {};
    tmp[0] = 0x1234;
    tmp[1] = 0x1234
 
	if(flag){
		arg = tmp;
    }

	arg[1] = 1.1;
	tmp[index] = 2;	
	arr2[0] = arg[0];	//give me a missingValue
}

function jit3(arg, index){
	
    let tmp = {a:1, b:2}
    tmp[0] = 0x1234;
    tmp[1] = 0x1234;
    
	if(flag){
		arg = tmp;
    }

	arg[1] = 1.1;
	tmp[index] = 2;	
	arr2[0] = arg[0];	//give me a missingValue
}

flag = 1; 
let funcs = [jit1, jit2, jit3]

let arr = {};
arr[0] = 1.1;
arr[1] = 2.2;

let arr2 = [1.1,2.2,{}];

for(var i = 0; i < 3; i++)
{  
    funcs[i](arr, 0);
    funcs[i](arr, 0);
    funcs[i](arr, 0x14);
    
    print(arr2[0]);

    arr = {};
    arr[0] = 1.1;
    arr[1] = 2.2;

    arr2 = [1.1,2.2,{}];
}