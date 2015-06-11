var tests = {
    // msg from host (private hr)
    "Property type is unsupported by SCA": {
        foo: function() {}
    },

    // msg from engine (recorded exception)
    "getter throws exception": {
        get foo() {
            return no_such_var;
        }
    },

    // msg from engine (recorded exception)
    "getter stack overflow": {
        get foo() {
            return this.foo;
        }
    }
};

for (var t in tests) {
    try {
        WScript.Echo(t);
        SCA.serialize(tests[t]);
    } catch (e) {
        WScript.Echo(TrimStackTracePath(e.stack));
    }
    WScript.Echo();
}

function TrimStackTracePath(line) {
    return line && line.replace(/\(.+unittest.sca./ig, "(");
}
