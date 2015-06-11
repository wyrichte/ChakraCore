// Fix for OS bug 778708 to make Authoring::DirectExecution::ForceCatch handle isBlockScopeFncDeclVar VarDecls correctly
// does not ensure correct fidelity.  E.g. this test case below fails for the second and third parameter listings.  They
// should work as the ES6 semantics in non-strict mode are that last executed function declaration wins out in the function
// scope, but these semantics are lost when ForceCatch moves the catch body into a force-called function expression.
// TODO: Fix this
function testcase1() {
    try {
    }
    catch (e) {
        function foo(m, n) {}
        foo(/**pl:m,n**/);
        {
            function foo(a, b) {}
        }
        foo(/**pl:a,b**/);
        {
            function foo(x, y, z) {}
        }
        foo(/**pl:x,y,z**/);
    }
}
