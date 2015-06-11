function TestAgrumentsAccess()
{           
    function f1()
    {
        var temp = arguments.length;
        for (var i in arguments)
        {
            temp = arguments[i];
        }
        return temp;
    }

    f1(1);
    f1(1, 2);
    f1(1, 2, 3);
    f1(1, 2, 3, 4);
    f1(1, 2, 3, 4, "a");
    f1(1, 2, 3, 4, "a", {});
    f1(1, 2, 3, 4, "a", {});
    f1(1, 2, 3, 4, "a", {});
    f1(1, 2, 3, 4, "a", {});
    f1(1, 2, 3, 4, "a", {});
    f1(1, 2, 3, 4, "a", {});
    f1(1, 2, 3, 4, "a", {});
    f1(1, 2, 3, 4, "a", {});
    f1(1, 2, 3, 4, "a", {});
    f1(1, 2, 3, 4, "a", {});
    f1(1, 2, 3, 4, "a", {});

    function f2(arg1, arg2, arg3, arg4)
    {
        var temp = arguments.length;
        for (var i in arguments)
        {
            temp = arguments[i];
        }
        return temp;
    }

    f2(1);
    f2(1, 2);
    f2(1, 2, 3);
    f2(1, 2, 3, 4);
    f2(1, 2, 3, 4, "a");
    f2(1, 2, 3, 4, "a", {});
    f2(1, 2, 3, 4, "a", {});
    f2(1, 2, 3, 4, "a", {});
    f2(1, 2, 3, 4, "a", {});
    f2(1, 2, 3, 4, "a", {});
    f2(1, 2, 3, 4, "a", {});
    f2(1, 2, 3, 4, "a", {});
    f2(1, 2, 3, 4, "a", {});
    f2(1, 2, 3, 4, "a", {});
    f2(1, 2, 3, 4, "a", {});
    f2(1, 2, 3, 4, "a", {});
}

TestAgrumentsAccess.testId = "Object.new.1";
TestAgrumentsAccess.description = "new Object()";
TestAgrumentsAccess.iterations = 25000;
TestAgrumentsAccess.quantifier = 100;
Register(TestAgrumentsAccess);

