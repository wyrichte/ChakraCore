
let d = 2.3023e-320;
let o = { x: 1, length: 2 };



function opt(o, o2) {
    o.x = 0.1;
    o2.reverse();
    o.x = d;
}
Object.prototype.reverse = Array.prototype.reverse;


for (let i = 0; i < 3; ++i) {
    let o2 = { x: 1, y: 2 };
    opt(o, o2);
}


o.__proto__[0] = 0;
o.__proto__[1] = 1;
opt(o, o);

if (o.x === 2.3023e-320)
    WScript.Echo('pass');
