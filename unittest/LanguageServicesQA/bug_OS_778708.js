// Verify Authoring::DirectExecution::ForceCatch handles isBlockScopeFncDeclVar VarDecls correctly
// when moving a catch body into a forced function expression calll.  When the catch block declares
// a function it gets moved into a nested function.  If this is the only function with that name
// the ES6 non-strict mode VarDecl created for the parent function needs to be deleted.  If it is
// not the only one it should not be deleted.
function testcase1() {
    try {
    }
    catch (e) {
        function foo() {}/**ml:-**/
    }
}

function testcase2() {
    try {
        function foo() { }
    }
    catch (e) {
        function foo() { }/**ml:-**/
    }
}

function testcase3() {
    try {
        function foo() { }
    }
    catch (e) {
        var x;/**ml:-**/
    }
}

function testcase4() {
    function foo() { }

    try {
    }
    catch (e) {
        function foo() { }/**ml:-**/
    }
}

function testcase5() {
    function foo() { }

    try {
        function foo() { }
    }
    catch (e) {
        function foo() { }/**ml:-**/
    }
}

function testcase6() {
    try {
    }
    catch (e) {
        {
            function foo() { }
        }
        ;/**ml:-**/
    }
}

function testcase7() {
    try {
    }
    catch (e) {
        {
            function foo() { }
        }
        function foo() { }/**ml:-**/
    }
}

function testcase8() {
    try {
        function foo() { }
        function bar() { }
        function baz() { }
    }
    catch (e) {
        {
            function foo() { }
            function bar() { }
            function bus() { }
        }
        function foo() { }/**ml:-**/
    }
}
