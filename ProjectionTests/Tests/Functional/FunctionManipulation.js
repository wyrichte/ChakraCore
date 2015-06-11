if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    var root = this;

    runner.addTest({
        id: 1,
        desc: "Manipulation of static methods",
        pri: '0',
        test: function () {
            // Calling 'bind' on static method with no overloads 
            var func = Animals.Animal.getAnswer.bind(Animals.Animal);
            verify(func(), 42, "Animals.Animal.getAnswer.bind(Animals.Animal)");
            func = Animals.Animal.getAnswer.bind();
            verify(func(), 42, "Animals.Animal.getAnswer.bind()");
            func = Animals.Animal.getAnswer.bind(root, 'hello');
            verify(func(), 42, "Animals.Animal.getAnswer.bind(root, 'hello')");
            func = Animals.Animal.getAnswer.bind({});
            verify(func(), 42, "Animals.Animal.getAnswer.bind({})");
            func = Animals.Animal.getAnswer.bind(null);
            verify(func(), 42, "Animals.Animal.getAnswer.bind(null)");
            func = Animals.Animal.getAnswer.bind(undefined);
            verify(func(), 42, "Animals.Animal.getAnswer.bind(undefined)");

            // Calling 'bind' on static method with overloads 
            func = DevTests.CamelCasing.OverloadStringVariations.pascalStaticOverload.bind(DevTests.CamelCasing.OverloadStringVariations);
            verify(func(1), "DevTests.CamelCasing.IOverloadCasingStatic.PascalStaticOverload(int) Called", "DevTests.CamelCasing.OverloadStringVariations.pascalStaticOverload.bind(DevTests.CamelCasing.OverloadStringVariations)");
            func = DevTests.CamelCasing.OverloadStringVariations.pascalStaticOverload.bind();
            verify(func(), "DevTests.CamelCasing.IOverloadCasingStatic.PascalStaticOverload() Called", "DevTests.CamelCasing.OverloadStringVariations.pascalStaticOverload.bind()");
            func = DevTests.CamelCasing.OverloadStringVariations.pascalStaticOverload.bind(root);
            verify(func(12), "DevTests.CamelCasing.IOverloadCasingStatic.PascalStaticOverload(int) Called", "DevTests.CamelCasing.OverloadStringVariations.pascalStaticOverload.bind(root)");
            func = DevTests.CamelCasing.OverloadStringVariations.pascalStaticOverload.bind({}, 12);
            verify(func(), "DevTests.CamelCasing.IOverloadCasingStatic.PascalStaticOverload(int) Called", "DevTests.CamelCasing.OverloadStringVariations.pascalStaticOverload.bind({}, 12)");
            func = DevTests.CamelCasing.OverloadStringVariations.pascalStaticOverload.bind(null);
            verify(func(), "DevTests.CamelCasing.IOverloadCasingStatic.PascalStaticOverload() Called", "DevTests.CamelCasing.OverloadStringVariations.pascalStaticOverload.bind(null)");
            func = DevTests.CamelCasing.OverloadStringVariations.pascalStaticOverload.bind(undefined);
            verify(func(), "DevTests.CamelCasing.IOverloadCasingStatic.PascalStaticOverload() Called", "DevTests.CamelCasing.OverloadStringVariations.pascalStaticOverload.bind(undefined)");

            // Calling 'call' and 'apply' on static method with no overloads
            verify(Animals.Animal.getAnswer.call(Animals.Animal), 42, "Animals.Animal.getAnswer.call(Animals.Animal)");
            verify(Animals.Animal.getAnswer.apply(), 42, "Animals.Animal.getAnswer.apply()");
            verify(Animals.Animal.getAnswer.call(root), 42, "Animals.Animal.getAnswer.call(root)");
            verify(Animals.Animal.getAnswer.apply({}), 42, "Animals.Animal.getAnswer.apply({})");
            verify(Animals.Animal.getAnswer.call(null, 73), 42, "Animals.Animal.getAnswer.call(null, 73)");
            verify(Animals.Animal.getAnswer.apply(undefined), 42, "Animals.Animal.getAnswer.apply(undefined)");

            // Calling 'call' and 'apply' on static method with overloads 
            verify(DevTests.CamelCasing.OverloadStringVariations.pascalStaticOverload.apply(DevTests.CamelCasing.OverloadStringVariations),
                "DevTests.CamelCasing.IOverloadCasingStatic.PascalStaticOverload() Called",
                "DevTests.CamelCasing.OverloadStringVariations.pascalStaticOverload.apply(DevTests.CamelCasing.OverloadStringVariations)");
            verify(DevTests.CamelCasing.OverloadStringVariations.pascalStaticOverload.call(),
                "DevTests.CamelCasing.IOverloadCasingStatic.PascalStaticOverload() Called",
                "DevTests.CamelCasing.OverloadStringVariations.pascalStaticOverload.call()");
            verify(DevTests.CamelCasing.OverloadStringVariations.pascalStaticOverload.apply(root, [22]),
                "DevTests.CamelCasing.IOverloadCasingStatic.PascalStaticOverload(int) Called",
                "DevTests.CamelCasing.OverloadStringVariations.pascalStaticOverload.apply(root, [22])");
            verify(DevTests.CamelCasing.OverloadStringVariations.pascalStaticOverload.call({}),
                "DevTests.CamelCasing.IOverloadCasingStatic.PascalStaticOverload() Called",
                "DevTests.CamelCasing.OverloadStringVariations.pascalStaticOverload.call({})");
            verify(DevTests.CamelCasing.OverloadStringVariations.pascalStaticOverload.apply(null, [3]),
                "DevTests.CamelCasing.IOverloadCasingStatic.PascalStaticOverload(int) Called",
                "DevTests.CamelCasing.OverloadStringVariations.pascalStaticOverload.apply(null, [3])");
            verify(DevTests.CamelCasing.OverloadStringVariations.pascalStaticOverload.call(undefined, 50, 'foo'),
                "DevTests.CamelCasing.IOverloadCasingStatic.PascalStaticOverload(int) Called",
                "DevTests.CamelCasing.OverloadStringVariations.pascalStaticOverload.call(undefined, 50, 'foo')");

            // Assigning static methods to a variable
            logger.comment("var func = Animals.Animal.getAnswer;");
            func = Animals.Animal.getAnswer;
            verify(func(), 42, "func()");
            verify(func.bind(Animals.Animal)(), 42, "func.bind(Animals.Animal)()");
            verify(func.bind(root)(), 42, "func.bind(root)()");
            verify(func.call(Animals.Animal), 42, "func.call(Animals.Animal)");
            verify(func.apply({}), 42, "func.apply({})");

            logger.comment("func = DevTests.CamelCasing.OverloadStringVariations.pascalStaticOverload;");
            func = DevTests.CamelCasing.OverloadStringVariations.pascalStaticOverload;
            verify(func(), "DevTests.CamelCasing.IOverloadCasingStatic.PascalStaticOverload() Called", "func()");
            verify(func(107), "DevTests.CamelCasing.IOverloadCasingStatic.PascalStaticOverload(int) Called", "func(107)");
            verify(func.bind(DevTests.CamelCasing.OverloadStringVariations)(), "DevTests.CamelCasing.IOverloadCasingStatic.PascalStaticOverload() Called", "func.bind(DevTests.CamelCasing.OverloadStringVariations)()");
            verify(func.bind(root, 9)(), "DevTests.CamelCasing.IOverloadCasingStatic.PascalStaticOverload(int) Called", "func.bind(root, 9)()");
            verify(func.apply(DevTests.CamelCasing.OverloadStringVariations, [220]), "DevTests.CamelCasing.IOverloadCasingStatic.PascalStaticOverload(int) Called", "func.apply(DevTests.CamelCasing.OverloadStringVariations, [220])");
            verify(func.call({}), "DevTests.CamelCasing.IOverloadCasingStatic.PascalStaticOverload() Called", "func.call({})");
        }
    });

    runner.addTest({
        id: 2,
        desc: "Manipulation of instance methods",
        pri: '0',
        test: function () {
            var turkeyInstance = new Animals.Turkey();
            var fishInstance = new Animals.Fish();
            var fishInstance2 = new Animals.Fish();

            // Calling 'bind' on instance method with no overloads 
            var func = fishInstance.getNumFins.bind(fishInstance);
            var func2 = fishInstance.setNumFins.bind(fishInstance2);
            logger.comment("fishInstance.setNumFins.bind(fishInstance2)");
            func2(23);
            verify(fishInstance2.getNumFins(), 23, "fishInstance2.getNumFins()");
            verify(func(), 5, "fishInstance.getNumFins.bind(fishInstance)");

            func = fishInstance.getNumFins.bind(turkeyInstance);
            verify.exception(function () { func(); }, TypeError, "fishInstance.getNumFins.bind(turkeyInstance)");
            func = fishInstance.getNumFins.bind();
            verify.exception(function () { func(); }, TypeError, "fishInstance.getNumFins.bind()");
            func = fishInstance.getNumFins.bind(root);
            verify.exception(function () { func(); }, TypeError, "fishInstance.getNumFins.bind(root)");
            func = fishInstance.getNumFins.bind({});
            verify.exception(function () { func(); }, TypeError, "fishInstance.getNumFins.bind({})");
            func = fishInstance.getNumFins.bind(null);
            verify.exception(function () { func(); }, TypeError, "fishInstance.getNumFins.bind(null)");
            func = fishInstance.getNumFins.bind(undefined);
            verify.exception(function () { func(); }, TypeError, "fishInstance.getNumFins.bind(undefined)");

            // Calling 'bind' on instance method with overloads 
            func = turkeyInstance.toSandwich.bind(turkeyInstance);
            verify(func(), 1, "turkeyInstance.toSandwich.bind(turkeyInstance)");
            verify(func(0), 0, "turkeyInstance.toSandwich.bind(turkeyInstance)");
            func = turkeyInstance.toSandwich.bind(turkeyInstance, 0);
            verify(func(), 0, "turkeyInstance.toSandwich.bind(turkeyInstance, 0)");

            func = turkeyInstance.toSandwich.bind(fishInstance);
            verify.exception(function () { func(); }, TypeError, "turkeyInstance.toSandwich.bind(fishInstance)");
            func = turkeyInstance.toSandwich.bind();
            verify.exception(function () { func(); }, TypeError, "turkeyInstance.toSandwich.bind()");
            func = turkeyInstance.toSandwich.bind(root);
            verify.exception(function () { func(); }, TypeError, "turkeyInstance.toSandwich.bind(root)");
            func = turkeyInstance.toSandwich.bind({});
            verify.exception(function () { func(); }, TypeError, "turkeyInstance.toSandwich.bind({})");
            func = turkeyInstance.toSandwich.bind(null);
            verify.exception(function () { func(); }, TypeError, "turkeyInstance.toSandwich.bind(null)");
            func = turkeyInstance.toSandwich.bind(undefined);
            verify.exception(function () { func(); }, TypeError, "turkeyInstance.toSandwich.bind(undefined)");

            // Calling 'call' and 'apply' on instance method with no overloads
            logger.comment("fishInstance.setNumFins.call(fishInstance2, 203);");
            fishInstance.setNumFins.call(fishInstance2, 203);
            verify(fishInstance.getNumFins.apply(fishInstance2), 203, "fishInstance.getNumFins.call(fishInstance)");
            verify(fishInstance.getNumFins.call(fishInstance), 5, "fishInstance.getNumFins.call(fishInstance)");

            verify.exception(function () { fishInstance.getNumFins.apply(turkeyInstance); }, TypeError, "fishInstance.getNumFins.apply(turkeyInstance)");
            verify.exception(function () { fishInstance.getNumFins.call(); }, TypeError, "fishInstance.getNumFins.call()");
            verify.exception(function () { fishInstance.getNumFins.apply(root); }, TypeError, "fishInstance.getNumFins.apply(root)");
            verify.exception(function () { fishInstance.getNumFins.call({}); }, TypeError, "fishInstance.getNumFins.call({})");
            verify.exception(function () { fishInstance.getNumFins.apply(null); }, TypeError, "fishInstance.getNumFins.apply(null)");
            verify.exception(function () { fishInstance.getNumFins.call(undefined); }, TypeError, "fishInstance.getNumFins.call(undefined)");

            // Calling 'call' and 'apply' on instance method with overloads 
            verify(turkeyInstance.toSandwich.apply(turkeyInstance), 1, "turkeyInstance.toSandwich.apply(turkeyInstance)");
            verify(turkeyInstance.toSandwich.call(new Animals.Turkey(), 0), 0, "turkeyInstance.toSandwich.call(new Animals.Turkey(), 0)");
            verify(turkeyInstance.toSandwich.apply(turkeyInstance, [3]), 1, "turkeyInstance.toSandwich.apply(turkeyInstance, [3])");

            verify.exception(function () { turkeyInstance.toSandwich.call(fishInstance); }, TypeError, "turkeyInstance.toSandwich.call(fishInstance)");
            verify.exception(function () { turkeyInstance.toSandwich.apply(); }, TypeError, "turkeyInstance.toSandwich.apply()");
            verify.exception(function () { turkeyInstance.toSandwich.call(root); }, TypeError, "turkeyInstance.toSandwich.call(root)");
            verify.exception(function () { turkeyInstance.toSandwich.apply({}); }, TypeError, "turkeyInstance.toSandwich.apply({})");
            verify.exception(function () { turkeyInstance.toSandwich.call(null); }, TypeError, "turkeyInstance.toSandwich.call(null)");
            verify.exception(function () { turkeyInstance.toSandwich.apply(undefined); }, TypeError, "turkeyInstance.toSandwich.apply(undefined)");

            // Assigning instance methods to a variable
            logger.comment("var func = fishInstance.getNumFins;");
            func = fishInstance.getNumFins;
            verify.exception(function () { func(); }, TypeError, "func()");
            verify(func.bind(fishInstance)(), 5, "func.bind(fishInstance)()");
            verify.exception(function () { func.bind(root)(); }, TypeError, "func.bind(root)()");
            verify(func.apply(fishInstance), 5, "func.apply(fishInstance)");
            verify.exception(function () { func.call({}); }, TypeError, "func.call({})");

            logger.comment("var func = turkeyInstance.toSandwich;");
            func = turkeyInstance.toSandwich;
            verify.exception(function () { func(); }, TypeError, "func()");
            verify(func.bind(turkeyInstance)(0), 0, "func.bind(turkeyInstance)(0)");
            verify.exception(function () { func.bind(root)(); }, TypeError, "func.bind(root)()");
            verify(func.call(turkeyInstance), 1, "func.call(turkeyInstance)");
            verify.exception(function () { func.apply({}); }, TypeError, "func.apply({})");
        }
    });

    runner.addTest({
        id: 3,
        desc: "Manipulation of constructors",
        pri: '0',
        test: function () {
            var mother = new Animals.Animal();

            // Calling 'bind' on constructor with no overloads 
            logger.comment("new Animals.Fish.bind(Animals)()");
            var func = Animals.Fish.bind(Animals);
            verify.instanceOf(new func(), Animals.Fish);
            logger.comment("new Animals.Fish.bind()()");
            func = Animals.Fish.bind();
            verify.instanceOf(new func(), Animals.Fish);
            logger.comment("new Animals.Fish.bind(root, 'hello')()");
            func = Animals.Fish.bind(root, 'hello');
            verify.instanceOf(new func(), Animals.Fish);
            logger.comment("new Animals.Fish.bind({})()");
            func = Animals.Fish.bind({});
            verify.instanceOf(new func(), Animals.Fish);
            logger.comment("new Animals.Fish.bind(null)()");
            func = Animals.Fish.bind(null);
            verify.instanceOf(new func(), Animals.Fish);
            logger.comment("new Animals.Fish.bind(undefined)()");
            func = Animals.Fish.bind(undefined);
            verify.instanceOf(new func(), Animals.Fish);

            // Calling 'bind' on constructor with overloads 
            logger.comment("new Animals.Animal.bind(Animals)()");
            var func = Animals.Animal.bind(Animals);
            verify.instanceOf(new func(), Animals.Animal);
            logger.comment("new Animals.Animal.bind()()");
            func = Animals.Animal.bind();
            verify.instanceOf(new func(), Animals.Animal);
            logger.comment("new Animals.Animal.bind(root, 7)()");
            func = Animals.Animal.bind(root, 7);
            verify.instanceOf(new func(), Animals.Animal);
            logger.comment("new Animals.Animal.bind({}, mother, 46)()");
            func = Animals.Animal.bind({}, mother, 46);
            verify.instanceOf(new func(), Animals.Animal);
            logger.comment("new Animals.Animal.bind(null, 1, 2)()");
            func = Animals.Animal.bind(null, 1, 2);
            var instance = new func(3);
            verify.instanceOf(instance, Animals.Animal);
            verify(instance.getNumLegs(), 6, "instance.getNumLegs()");
            logger.comment("new Animals.Animal.bind(undefined)()");
            func = Animals.Animal.bind(undefined);
            verify.instanceOf(new func(), Animals.Animal);

            // Assigning constructors to a variable
            logger.comment("var func = Animals.Fish;");
            func = Animals.Fish;
            logger.comment("new func()");
            verify.instanceOf(new func(), Animals.Fish);
            logger.comment("new (func.bind(Animals))()");
            verify.instanceOf(new (func.bind(Animals))(), Animals.Fish);
            logger.comment("new (func.bind(root))()");
            verify.instanceOf(new (func.bind(root))(), Animals.Fish);

            logger.comment("var func = Animals.Animal;");
            func = Animals.Animal;
            logger.comment("new func()");
            verify.instanceOf(new func(), Animals.Animal);
            logger.comment("new (func.bind(Animals, mother))(10)");
            instance = new (func.bind(Animals, mother))(10);
            verify.instanceOf(instance, Animals.Animal);
            verify(instance.weight, 10, "instance.weight");
            logger.comment("new (func.bind(root))()");
            verify.instanceOf(new (func.bind(root))(), Animals.Animal);
        }
    });

    runner.addTest({
        id: 4,
        desc: "Manipulation of delegates",
        pri: '0',
        test: function () {
            var animalInstance = new Animals.Animal();

            function DelegateAsOutParamWithJSCallback(animal) {
                logger.comment("*** JSCallback as out delegate param called");
                var animalNames = animal.getNames();
                return animalNames.common;
            }

            var jsDelegate = animalInstance.methodDelegateAsOutParam(DelegateAsOutParamWithJSCallback);
            var nativeDelegate = animalInstance.getNativeDelegateAsOutParam();

            // Calling 'bind' on a JS Delegate
            var func = jsDelegate.bind(animalInstance);
            verify(func(animalInstance), 'Wolverine', "jsDelegate.bind(animalInstance)");
            func = jsDelegate.bind();
            verify(func(animalInstance), 'Wolverine', "jsDelegate.bind()");
            func = jsDelegate.bind(root, animalInstance);
            verify(func(), 'Wolverine', "jsDelegate.bind(root, animalInstance)");
            func = jsDelegate.bind({});
            verify(func(animalInstance), 'Wolverine', "jsDelegate.bind({})");
            func = jsDelegate.bind(null);
            verify(func(animalInstance), 'Wolverine', "jsDelegate.bind(null)");
            func = jsDelegate.bind(undefined);
            verify(func(animalInstance), 'Wolverine', "jsDelegate.bind(undefined)");

            // Calling 'bind' on a Native Delegate
            func = nativeDelegate.bind(animalInstance);
            verify(func(animalInstance), 'Wolverine', "nativeDelegate.bind(animalInstance)");
            func = nativeDelegate.bind();
            verify(func(animalInstance), 'Wolverine', "nativeDelegate.bind()");
            func = nativeDelegate.bind(root);
            verify(func(animalInstance), 'Wolverine', "nativeDelegate.bind(root)");
            func = nativeDelegate.bind({}, animalInstance);
            verify(func(42), 'Wolverine', "nativeDelegate.bind({}, animalInstance)");
            func = nativeDelegate.bind(null);
            verify(func(animalInstance), 'Wolverine', "nativeDelegate.bind(null)");
            func = nativeDelegate.bind(undefined, animalInstance, 12);
            verify(func(), 'Wolverine', "nativeDelegate.bind(undefined, animalInstance, 12)");

            // Calling 'call' and 'apply' on JS Delegate
            verify(jsDelegate.call(animalInstance, animalInstance), 'Wolverine', "jsDelegate.call(animalInstance, animalInstance)");
            verify(jsDelegate.call(root, animalInstance), 'Wolverine', "jsDelegate.call(root, animalInstance)");
            verify(jsDelegate.apply({}, [animalInstance]), 'Wolverine', "jsDelegate.apply({}, [animalInstance])");
            verify(jsDelegate.call(null, animalInstance, 73), 'Wolverine', "jsDelegate.call(null, animalInstance, 73)");
            verify(jsDelegate.apply(undefined, [animalInstance]), 'Wolverine', "jsDelegate.apply(undefined, [animalInstance])");

            // Calling 'call' and 'apply' on Native Delegate
            verify(nativeDelegate.apply(animalInstance, [animalInstance]), 'Wolverine', "nativeDelegate.apply(animalInstance, [animalInstance])");
            verify(nativeDelegate.apply(root, [animalInstance, "hello"]), 'Wolverine', "nativeDelegate.apply(root, [animalInstance, 'hello'])");
            verify(nativeDelegate.call({}, animalInstance), 'Wolverine', "nativeDelegate.call({}, animalInstance)");
            verify(nativeDelegate.apply(null, [animalInstance]), 'Wolverine', "nativeDelegate.apply(null, [animalInstance])");
            verify(nativeDelegate.call(undefined, animalInstance), 'Wolverine', "nativeDelegate.call(undefined, animalInstance)");

        }
    });

    Loader42_FileName = "Manipulation of Projected Functions";
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
