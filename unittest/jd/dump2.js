// test WER JS dump

function foo() {
    try {
        throw new Error();
    } catch(e) {
        //WScript.Echo(e.stack);
    }
}

function test()
{
    try {
        foo(); // 1st call

        foo(); // 2nd call. Amd64 should use saved EH frame ip to look up script location.

    } catch(e) {
        WScript.Echo(e);
    } finally {
        this.not_used = 0;
    }
}

test();
