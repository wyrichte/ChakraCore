function foo() {
    "use strict";
    var a;
    delete (a); // Win8 776066: delete (identifier) should result in SyntaxError in strict mode.
}

foo();
