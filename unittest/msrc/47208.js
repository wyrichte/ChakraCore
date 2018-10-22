let arr = [1.1, 2.2, {}];
function jit(arg)
{
    let t = arg[2];
    t.x = 1;
}

for(let i = 0; i<10000;i++)
{
    jit(arr);
}

let tt = new Array(1,2,-524286, {});

Array.isArray([]);
jit(tt);
print("Passed");