a = {};
a.__defineSetter__(Symbol.toPrimitive, function tmp() { });
function replacer(arg_0, arg_1) {
    if (!({} >= a)) {
        try { } catch (e) { }
    }
    arg_0.__defineSetter__({}, (function tmp2() { }));
    return 1;
}

function test(){
for(var i = 0; i < 0x1000; i++) {
    b = replacer({}, [])
    }
}
test()

print("Pass");
