function withStrict(a) {
    "use strict"
    function foo() {
        var j = 10;
        j++;
        return j;
    }
    foo();
    foo.apply({ x1: 20 });
}
withStrict(10);
