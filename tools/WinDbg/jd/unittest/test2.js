// test function name

try {
    var foo = function() {
        throw new Error("My Error!");
    };

    function func(){
        foo();
    }

    var constructed = new Function("func();");

    function bar(){
        (function(){
            eval("constructed();");
        })();
    }

    bar();
} catch(e) {
    WScript.Echo(e.stack);
}
