function test() {
    var Obj = { x: 'wrong' };
    Obj.x;
    Obj = 1;
    WScript.Echo(Obj.x + '');
}
test();

(function () {
    var obj0 = 1;
    (function (p0) {
        p0 -= Math.abs(Math.atan2(1, (obj0.length <<= 1)));
        WScript.Echo(p0);
    })(0);
})();
