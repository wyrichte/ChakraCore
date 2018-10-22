
function opt(arr, start, end)
{
    for (let i = start; i < end; i++)
    {
        if (i == 10)
        {
            let j = 0;
            do
            {
                i += 0;
            }
            while(j != 0);
        }
        arr[i] = 2.3023e-320;
    }
}

let arr = new Array(100);
arr.fill(1.1);

for (i=0; i < 0x100000; i++)
{
    opt(arr, 0, 3);
}

opt(arr, 0, 100000);

WScript.Echo("Passed");