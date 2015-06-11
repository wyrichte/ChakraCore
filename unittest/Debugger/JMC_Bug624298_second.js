
function test1() {
    var k = 10;
    k += testInternal();
};
function testInternal() {
    return 10;                 /**bp:dumpBreak();**/
}

function test2() {
    debugger;
}
