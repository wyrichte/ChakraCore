// The branch on 'i' should get const-folded. Ideally, the branch should go away altogether (since after const-folding the
// branch, it becomes an unconditional branch to the next instruction), but that's not yet implemented.
function test0() {
    var i = 0;
    if(i)
        return 1;
    else
        return 2;
}
test0();

// 'i < n' and '++i' should get int-specialized with AggressiveIntTypeSpec enabled and with profile data, both in the loop
// prepass and in the optimization pass
function test1(n) {
    for(var i = 0; i < n; ++i);
}
test1(1);
