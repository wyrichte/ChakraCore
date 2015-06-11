// EnC: ScopeSlots and ScopeObject may have same closure content layout, but the slots are at
// different location. ScopeSlots starts from SlotArr[2], while ScopeObject from SlotArr[0].
// We need to reject update if one is ScopeSlot and the other is ScopeObject.

var x = (function foo() {
    /**edit(test)**/
    /// var foo = 1;
    /**endedit(test)**/
    return function f() { return foo; }
});

var f = x();

var stages = ["=== Before change ===", "=== After change ==="], curStage = 0;
function test() {
    WScript.Echo(stages[curStage++]);
    WScript.Echo(f());
}

test();
WScript.Edit("test", test);
