//var _sunSpiderStartDate = new Date();

// The Computer Language Shootout
// http://shootout.alioth.debian.org/
// contributed by Isaac Gouy

function partial(n)
{
    var a1 = a2 = a3 = a4 = a5 = a6 = a7 = a8 = a9 = 0.0;
    
    var twothirds = 2.0 / 3.0;
    var alt = -1.0;

    var k2 = k3 = sk = ck = 0.0;

    for (var k = 1; k <= n; k++)
    {
        k2 = k * k;
        k3 = k2 * k;
        sk = Math.sin(k);
        ck = Math.cos(k);
        alt = -alt;

        a1 += Math.pow(twothirds, k - 1);
        a2 += Math.pow(k, -0.5);
        a3 += 1.0 / (k * (k + 1.0));
        a4 += 1.0 / (k3 * sk * sk);
        a5 += 1.0 / (k3 * ck * ck);
        a6 += 1.0 / k;
        a7 += 1.0 / k2;
        a8 += alt / k;
        a9 += alt / (2 * k - 1);
    }
    WScript.Echo(a1);
    WScript.Echo(a2);
    WScript.Echo(a3);
    WScript.Echo(a4);
    WScript.Echo(a5);
    WScript.Echo(a6);
    WScript.Echo(a7);
    WScript.Echo(a8);
    WScript.Echo(a9);
}

for (var i = 1024; i <= 16384; i *= 2)
{
    partial(i);
}


//var _sunSpiderInterval = new Date() - _sunSpiderStartDate;
//WScript.Echo("### TIME:", _sunSpiderInterval, "ms");
