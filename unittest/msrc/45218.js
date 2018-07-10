
try {
    baseObj = {
        get val() {
                return 30;
            },
        val: 61849
    };
    function F(v, p) {
        baseObj.val('this.prop0 = ' + (this | 0));
        c = [
                1,
                2,
                3,
                4
            ];
    }
    F.prototype = baseObj;
    a = new F(20, null);
    b = new F(22, a);
    c = new F(24, b);
    this.baseObj = b;
    b.val = 13;
    WScript.Echo("Failed");
} catch (e) {
    if (e instanceof TypeError){
        WScript.Echo("passed");
    } else {
	WScript.Echo(e);
        WScript.Echo("Failed")
    }
    
}
