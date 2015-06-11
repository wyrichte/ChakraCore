function test() {
    function nested() {
        eval(""); // eval here makes test's scope a non-dynamic object scope
    }
    /**bp:locals()**/
    let l = 10;
    /**bp:locals()**/
    const c = 20;
    /**bp:locals(),evaluate('c=30'),locals()**/
}
test.apply({});
WScript.Echo("PASSED");
