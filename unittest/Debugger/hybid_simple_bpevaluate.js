let x = 1;
var y = 1; /**bp:evaluate('x')**/;
let z = {
    m: {
        n: 1
    },
    p: 2,
    set value(val){return 1}, 
    get value(){return 1}
}
WScript.Echo('PASSED');/**bp:evaluate('z');evaluate('z.m');evaluate('z.m.n');evaluate('z.p');evaluate('a.value')**/