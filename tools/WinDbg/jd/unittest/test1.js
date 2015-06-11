// test

function foo() {

    (function bar(){

        function blah() {
            for (var i = 0; i < 10; i++) {
                if (i == 5) {
                    WScript.Echo();
                }
            }
            for (var i = 0; i < 10; i++) {
                if (i == 9) {
                    WScript.Echo();
                }
            }
        }

        blah();

    })();
}

foo();
