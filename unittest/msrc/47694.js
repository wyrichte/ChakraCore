var a0 = new Float64Array(0xfff);
function f0(a, start, end, offset) {
    if (start < 0 || offset < 0) return;
    var i = start;
    var j = start + offset;
    while (i < end) {
        i++;
        j++;
        a[i] = 5;
    }
    a[j];
}
f0(a0, 0x100, 0x200, 0x10);
f0(a0, 0, 1, 0x7fffffff);

var a1 = new Float64Array(0xfff);
function f1(a, start, end, offset, magic) {
    if (start < 0 || offset < 0 || magic < 0) return;
    var i = start;
    var j = start + offset;
    while (i < end) {
        a[i] = 5;
        i++;
        j++;
    }
    magic = j + magic;
    return [magic];
}
f1(a1, 0, 0x100, 0x200, 0x10);
f1(a1, 0, 1, 0x7fffffff, 0x7FF80002)[0];

var a2 = new Float64Array(0xfff);
function f2(a, start, end, offset) {
    if (start < 0 || offset < 0) return;
    for (var i = start, j = start + offset; i < end; i++ , j++) a[i] = 5;
    a[j];
}
f2(a2, 0x100, 0x200, 0x10);
f2(a2, 0, 1, 0x7fffffff);

WScript.Echo("PASSED");

