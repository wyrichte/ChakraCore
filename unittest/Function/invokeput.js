var count = 0;
var obj = { x: function(a) {
        WScript.Echo("In x(), count = " + count);
        count++;
        return count;
    }
};
function w() {
    WScript.Echo("In w(), count = " + count);
    count++;
    return obj;
}
function y() {
    WScript.Echo("In y(), count = " + count);
    count++;
}
function z() {
    WScript.Echo("In z(), count = " + count);
    count++;
    return count;
}

function TestInvokePut(expr, desc) {
    try {
        WScript.Echo("");
        WScript.Echo("Test: " + desc);
        expr();
    }
    catch (ex) {
        WScript.Echo("In catch, count = " + count + " " + ex);
        count++;
    }
}

function test() {

    try {
        // CallFld with simple assign
        w().x(y()) = z();
    }
    catch (e) {
        WScript.Echo("In catch1, count = " + count);
        count++;
    }

    try {
        // CallElem with simple assign
        w()["x"](y()) = z();
    }
    catch (e) {
        WScript.Echo("In catch2, count = " + count);
        count++;
    }

    try {
        // CallI with simple assign
        var foo = w().x;
        foo(y()) = z();
    }
    catch (e) {
        WScript.Echo("In catch3, count = " + count);
        count++;
    }

    try {
        // CallFld with compound assign
        w().x(y()) += z();
    }
    catch (e) {
        WScript.Echo("In catch1, count = " + count);
        count++;
    }

    try {
        // CallElem with compound assign
        w()["x"](y()) += z();
    }
    catch (e) {
        WScript.Echo("In catch2, count = " + count);
        count++;
    }

    try {
        // CallI with compound assign
        var foo = w().x;
        foo = new foo(y()) += z();
    }
    catch (e) {
        WScript.Echo("In catch3, count = " + count );
        count++;
    }
    // Compound Assignment
    TestInvokePut(function () { w().x(y()) -= ([11, 12].length); }, "CallFld with compound assignment and complex RHS");
    TestInvokePut(function () { w()["x"](y()) -= ([11, 12].length); }, "CallElem with compound assignment and complex RHS");
    TestInvokePut(function () { w(y()) -= ([11, 12].length); }, "CallFld with compound assignment and complex RHS");
    TestInvokePut(function () { eval(); w(y()) -= ([11, 12].length); }, "CallFld with eval with compound assignment and complex RHS");
    TestInvokePut(function () { w() -= 2; }, "CallFld with simple RHS");
    TestInvokePut(function () { w() *= ([11, 12].length); }, "CallFld with complex RHS");
    TestInvokePut(function () { eval(); w() -= 2; }, "CallFld with eval with simple RHS");
    TestInvokePut(function () { eval(); w() *= ([11, 12].length); }, "CallFld with eval with complext RHS");
    
    // Post Increment
    TestInvokePut(function () { w().x(y())++; }, "CallFld Post Incr");
    TestInvokePut(function () { w()["x"](y())++; }, "CallElem Post Incr");
    TestInvokePut(function () { w(y())++; }, "CallFld Post Incr");
    TestInvokePut(function () { w()++; }, "CallI with Post Incr");

    // Pre Increment
    TestInvokePut(function () { ++w().x(y()); }, "CallFld Pre Incr");
    TestInvokePut(function () { ++w()["x"](y()); }, "CallElem Pre Incr");
    TestInvokePut(function () { ++w(y()); }, "CallFld Pre Incr");
    TestInvokePut(function () { --w(); }, "CallI with Pre Decr");

    // Multiple Arguments
    TestInvokePut(function () { w().x(y(), "test") -= ([11, 12].length); }, "CallFld with multiple args and compound assignment and complex RHS");
    TestInvokePut(function () { w().x(y(), "test", "foo", 3 > 10) -= ([11, 12].length); }, "CallFld with multiple args and compound assignment and complex RHS");

    // Eval
    TestInvokePut(function () { eval("10;") += ([11, 12].length); }, "Eval with compound assingment");
    TestInvokePut(function () { eval("10;")++; }, "Eval with post increment");
    TestInvokePut(function () { --eval("10;"); }, "Eval with pre increment");

    //CallIPut
    TestInvokePut(function () { var foo = w().x; foo(y()) -= ([11, 12].length); }, "CallFld with compound assignment and complex RHS");
    TestInvokePut(function () { var foo = w(); foo() *= 23; }, "Compund assignment of local");
    TestInvokePut(function () { var foo = w(); foo() = 23; }, "Simple assignment of local");
    TestInvokePut(function () { function foo() { WScript.Echo("In foo(), count = " + count++); }; function bar() { foo = 2; } bar(); }, "Simple assignment of closure variable");
    TestInvokePut(function () { function foo() { WScript.Echo("In foo(), count = " + count++); }; function bar() { foo() *= 2; } bar(); }, "Compund assignment of closure variable");

    // In a with scope
    TestInvokePut(function () { function foo() { WScript.Echo("In foo(), count = " + count++); } function bar() { with (obj) { foo() = 2; } } bar(); }, "Simple assignment in a with scope.");
    TestInvokePut(function () { function foo() { WScript.Echo("In foo(), count = " + count++); } function bar() { with (obj) { foo() *= 2; } } bar(); }, "Compound assignment in a with scope.");

    // Function passed as an argument
    TestInvokePut(function () { function foo() { WScript.Echo("In foo(), count = " + count++); } function bar(func) { arguments; func() = 2; } bar(foo); }, "Simple assignment with function as argument");
    TestInvokePut(function () { function foo() { WScript.Echo("In foo(), count = " + count++); } function bar(func) { arguments; func() *= 2; } bar(foo); }, "Compound assignment with function as argument");

    // Function expression
    TestInvokePut(function () { var foo = function() { WScript.Echo("In foo(), count = " + count++); }; foo() = 20; }, "Simple assignment with function expression");
    TestInvokePut(function () { var foo = function() { WScript.Echo("In foo(), count = " + count++); }; foo() *= 20; } , "Compound assignment with function expression");
    TestInvokePut(function () { var foo = function () { WScript.Echo("In foo(), count = " + count++); }; foo() *= [3, 4].length; }, "Compound assignment with function expression with Complext RHS");

}

test();