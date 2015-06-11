/**exception(resume_ignore):stack()**/
function* gf1() {
    yield 1;
    yield 2;
    yield 3;
}
function* gf2() {
    yield* gf1();
}

g = gf1();
g.next();
g.throw(1);

g = gf2();
g.next(1);
g.throw(2);

WScript.Echo("PASS");
