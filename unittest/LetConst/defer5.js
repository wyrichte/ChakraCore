// Flags: -version:5 -ForceDeferParse
function test()
{
    function foo() {
        // Bar is a let variable being used here before it's been assigned to
        bar;
    }

    foo();
    let bar = this;
}

test();