function opt(a, b){
    a.a = 1
    a.b = 1
    for(let i=0;i<0x1000;i++) {
//        b[b.length] = i;
//        b.length++;
        b.push(i);
    }
    
    a.c = 1  // AdjustSlots(0,8)
    a.d = 1
}

var b = {}
b.push = [].push

for(var i=0;i<100;i++){
    var a = {}
    a.push = [].push
    a.length = 0xffffffff
    opt(a,b)
}

// Object.isSealed(opt)
var a = {}
a.push = [].push
a.length = 0xffffffff
opt(a,a)  

if (a.d === 1)
    WScript.Echo('pass');
