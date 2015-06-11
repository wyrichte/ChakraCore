// test

function foo() {

    (function bar(){

        function blah() {
            for (var i = 0; i < 10; i++) {
                if (i == 5) {
                    Debug.sourceDebugBreak();
                }
            }
            for (var i = 0; i < 10; i++) {
                if (i == 9) {
                    Debug.sourceDebugBreak();
                }
            }
        }

        blah();

    })();
}

foo();
