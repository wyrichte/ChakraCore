if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    var winery;

    verify.overload = function verifyOverload(obj, prop, args, expected) {
        verify.equal(obj[prop].apply(obj, args), expected, prop);
    }

    verify.members = function verifyMembers(obj, expected) {
        var expect;
        for(var mem in obj) {
            expect = expected[mem];
            verify.defined(expect, mem);
            verify.typeOf(obj[mem], expect, mem);
        }
    };

    runner.globalSetup(function () {
        winery = new Winery.RWinery(1);
    });

    runner.addTest({
        id:1,
        desc: 'Static overloading with Requires',
        pri: 0,
        test: function () {
            var c = Winery.Overloading.StaticConflictWithRequiresInterface.Access;

            var expected = {
				// doSomething() is not aliased as there is a conflict!
                'Winery.Overloading.StaticConflictWithRequiresInterface.IA.doSomething': 'function',
                'Winery.Overloading.StaticConflictWithRequiresInterface.IB.doSomething': 'function',
                'doSomethingNotOverloaded': 'function',
            }

            verify.typeOf(c, 'function');
            verify.members(c, expected);

            var retValue;

            retValue = c.doSomethingNotOverloaded(true, true);
            verify.equal(retValue, "StaticConflictWithRequiresInterface.IA.DoSomethingNotOverloaded(boolean, boolean) called", "Winery.Overloading.StaticConflictWithRequiresInterface.IA.doSomethingNotOverloaded(true, true)");

            retValue = c["doSomethingNotOverloaded"](true, true);
            verify.equal(retValue, "StaticConflictWithRequiresInterface.IA.DoSomethingNotOverloaded(boolean, boolean) called", "Winery.Overloading.StaticConflictWithRequiresInterface.IA.doSomethingNotOverloaded(true, true)#2");

            retValue = Winery.Overloading.StaticConflictWithRequiresInterface.Access.doSomethingNotOverloaded(true, true);
            verify.equal(retValue, "StaticConflictWithRequiresInterface.IA.DoSomethingNotOverloaded(boolean, boolean) called", "Winery.Overloading.StaticConflictWithRequiresInterface.IA.doSomethingNotOverloaded(true, true)");

			// interfaces from IB - as-is
			// doSomething(true)
            retValue = c["Winery.Overloading.StaticConflictWithRequiresInterface.IB.doSomething"](true);
            verify.equal(retValue, "StaticConflictWithRequiresInterface.IB.DoSomething2(boolean) called", "Winery.Overloading.StaticConflictWithRequiresInterface.IB.doSomething(true)");
			
			// doSomething(true, true, true)
            retValue = c["Winery.Overloading.StaticConflictWithRequiresInterface.IB.doSomething"](true, true, true);
            verify.equal(retValue, "StaticConflictWithRequiresInterface.IB.DoSomething4(boolean, boolean, boolean) called", "Winery.Overloading.StaticConflictWithRequiresInterface.IB.doSomething(true, true, true)");

			// doSomething(true, true) from both IA and IB
            retValue = c["Winery.Overloading.StaticConflictWithRequiresInterface.IB.doSomething"](true, true);
            verify.equal(retValue, "StaticConflictWithRequiresInterface.IB.DoSomething3(boolean, boolean) called", "Winery.Overloading.StaticConflictWithRequiresInterface.IB.doSomething(true, true)");

            retValue = c["Winery.Overloading.StaticConflictWithRequiresInterface.IA.doSomething"](true, true);
            verify.equal(retValue, "StaticConflictWithRequiresInterface.IA.DoSomething(boolean, boolean) called", "Winery.Overloading.StaticConflictWithRequiresInterface.IA.doSomething(true, true)");
        }
    });

    Loader42_FileName = "overloading.blue171007";
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
