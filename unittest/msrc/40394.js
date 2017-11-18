function f(earlyRet)
{
    let arr = new Uint32Array(0x1000);
    for (let i = 0; i < 0x7fffffff;)
    {
        arr[++i] = 0x1234;
        if(earlyRet)
        {
            return;
        }
    }
}

f(true);
f(true);
f(true);

print("Pass");
