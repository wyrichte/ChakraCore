function Dump(x, dump)
{
    dump = true;
    if (dump != undefined)
    {
        WScript.Echo(x);
    }
}

var HETest = {};

HETest.x = {
    x_data: "x local",
    get x_accessor() { return this.x_data; },
    set x_accessor(arg) { this.x_data = arg; }
}

HETest.x2 = {
    x_data: "x local",
    set x_accessor(arg) { this.x_data = arg; }
}

HETest.x3 = {
    x_data: "x local",
    get x_accessor() { return this.x_data; },
}

HETest.y = {
    body: function() {
        var y_data = "y local";
        var y_inner = {
            get y_accessor() { return this.y_data; },
            set y_accessor(arg) { this.y_data = arg; }
        }
        return {
            get_direct: function() { return y_data; },
            set_via_prop: function(arg) { y_inner.y_accessor = arg; }
        };
    }
}

Dump(HETest.x.x_data);
HETest.x.x_accessor = 5;
Dump(HETest.x.x_data);
HETest.yy = HETest.y.body();
Dump(HETest.yy.get_direct());
HETest.yy.set_via_prop(6);
Dump(HETest.yy.get_direct());

Debug.dumpHeap(HETest, true);