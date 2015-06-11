// This tests that sibling scopes are tracked when in non-debug mode
// and properly reset during reparse and bytecode regeneration.
// Bug #321751

function Run() {
    // Slot array scope
    {
        let a = 0;          // Slot array
        function inner() {  // Register slot
            a++;
        }
        inner();
    }

    // Slot array scope
    {
        let c = 2;          // Slot array
        function inner2() { // Register slot
            c++;
        }
        inner2();
    }
};
Run();

WScript.Attach(function(){Run();});
WScript.Echo("PASSED");