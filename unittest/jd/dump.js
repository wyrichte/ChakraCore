// test WER JS dump

var g_count = 0;

function foo() {
    if (g_count++ % 5 == 4) {
        Debug.sourceDebugBreak();
    }
}

function bar() {
    foo();
}

function bung() {
    bar();
}

function test()
{
    var f = function() {
        try {
            bung();
        } catch(e) {
            WScript.Echo(e);
        } finally {
            this.not_used = 0;
        }
    }
    
    // loop 1
    for (var i = 0; i < 5; i++) {
        f();
    }

    // loop 2
    for (var i = 0; i < 5; i++) {
        f();
    }
}

test();
