let o = {
    get a() {},
    0: 0, // Deoptimizing object header inlining
    a: 0x1234
};

if (o.a === 0x1234)
    WScript.Echo('pass');
