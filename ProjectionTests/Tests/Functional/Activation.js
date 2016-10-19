if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    // Tests around ABI Activation

    // Test that we cannot create an ABI instance which is not registered
    runner.addTest({
        id: 1,
        desc: 'UnregisteredClassActivation',
        pri: '0',
        test: function () {
            var exceptionCaught = false;
            var REGDB_E_CLASSNOTREG = Winery.WinRTErrorTests.RestrictedErrorAccess.errorCodes.classNotReg;

            try {
                var myDodoBird = new Animals.DodoBird();
            }
            catch (error) {
                verify(error.number, REGDB_E_CLASSNOTREG, 'error.number');
                exceptionCaught = true;
            }

            assert(exceptionCaught, 'Expected exception was caught');
        }

    });

    // Test that we cannot access namespaces that do not exist
    runner.addTest({
        id: 2,
        desc: 'UnregisteredNamespace',
        pri: '0',
        test: function () {
            var myGodzillaNamespace = Animals.Godzilla;
            verify(myGodzillaNamespace, undefined, 'Animals.Godzilla');
        }

    });

    // Test that we can create an ABI that is simple and factory activatable
    runner.addTest({
        id: 3,
        desc: 'SimpleAnimalActivation',
        pri: '0',
        test: function () {
            verify(Animals.Animal.constructor.name, "Animal","Animals.Animal.constructor.name");
            var myAnimal = new Animals.Animal();
            verify(myAnimal.getNumLegs(), 20, 'myAnimal.getNumLegs()');
        }

    });

    runner.addTest({
        id: 4,
        desc: 'FactoryAnimalActivation: 1 argument',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);
            verify(myAnimal.getNumLegs(), 1, 'myAnimal.getNumLegs()');
        }

    });

    runner.addTest({
        id: 5,
        desc: 'FactoryAnimalActivation: 2 arguments',
        pri: '0',
        test: function () {
            var animalMother = new Animals.Animal();
            var myAnimal = new Animals.Animal(animalMother, 4);
            verify(myAnimal.weight, 4, 'myAnimal.weight');
        }
    });

    runner.addTest({
        id: 6,
        desc: 'FactoryAnimalActivation: 4 arguments',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(5, 20, 33, 100);
            verify(myAnimal.getNumLegs(), 58, 'myAnimal.getNumLegs()');
        }

    });

    runner.addTest({
        id: 7,
        desc: 'FactoryAnimalActivation: 5 arguments',
        pri: '0',
        test: function () {
            var animalMother = new Animals.Animal();
            var myAnimal = new Animals.Animal(animalMother, 23, 4, 7, 16);
            verify(myAnimal.weight, 23, 'myAnimal.weight');
            verify(myAnimal.getNumLegs(), 27, 'myAnimal.getNumLegs()');
        }
    });

    runner.addTest({
        id: 8,
        desc: 'FactoryAnimalActivation: 6 arguments',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(5, 20, 33, 100, 1, 3);
            verify(myAnimal.getNumLegs(), 162, 'myAnimal.getNumLegs()');
        }

    });

    runner.addTest({
        id: 9,
        desc: 'FactoryAnimalActivation: 10 arguments',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(5, 20, 33, 100, 1, 3, 50, 25, 78, 250);
            verify(myAnimal.getNumLegs(), 212, 'myAnimal.getNumLegs()');
        }

    });

    runner.addTest({
        id: 10,
        desc: 'FishActivation: 0 arguments',
        pri: '0',
        test: function () {
            var myFish = new Animals.Fish();
            verify(myFish.getNumFins(), 5, 'myFish.getNumFins()');
        }

    });

    runner.addTest({
        id: 11,
        desc: 'FishActivation: 2 arguments',
        pri: '0',
        test: function () {
            var myFish = new Animals.Fish(false, 3);
            verify(myFish.getNumFins(), 5, 'myFish.getNumFins()');
        }

    });

    runner.addTest({
        id: 12,
        desc: 'ChefActivation: 0 arguments',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var myChef = new Fabrikam.Kitchen.Chef();
            }
            catch (error) {
                verify.instanceOf(error, Error);
                verify(error.description, "Chef: function called with too few arguments", 'error.description');
                exceptionCaught = true;
            }

            assert(exceptionCaught, 'Expected exception was caught');
        }

    });

    runner.addTest({
        id: 13,
        desc: 'ChefActivation: 3 arguments',
        pri: '0',
        test: function () {
            var myKitchen = new Fabrikam.Kitchen.Kitchen();
            var myChef = new Fabrikam.Kitchen.Chef('Bob', myKitchen, 42);
            verify(myChef.name, 'Bob', 'myChef.name');
        }

    });

    // Test that we can not activate a non-activatable class
    runner.addTest({
        id: 14,
        desc: 'NonActivatableClassActivation',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var myPuppy = new Animals.Pomapoodle();
            }
            catch (error) {
                verify.instanceOf(error, Error);
                verify(error.description, "Animals.Pomapoodle: type is not constructible", 'error.description');
                exceptionCaught = true;
            }

            assert(exceptionCaught, 'Expected exception was caught');
        }

    });

    // Test that we can call static methods on a non-activatable class
    runner.addTest({
        id: 15,
        desc: 'CallOnNonActivatableClass',
        pri: '0',
        test: function () {
            verify(Animals.Pomapoodle.eatCookies(4), 3, 'Animals.Pomapoodle.eatCookies(4)');
        }

    });

    // Test that we can call static methods on a non-activatable class
    runner.addTest({
        id: 16,
        desc: 'FactoryActivatableClassWithNoFactoryMethods',
        pri: '0',
        test: function () {
            var exceptionCaught = false;

            try {
                var empty = new Animals.EmptyClass();
            }
            catch (error) {
                verify.instanceOf(error, Error);
                verify(error.description, "Animals.EmptyClass: type is not constructible", 'error.description');
                exceptionCaught = true;
            }

            assert(exceptionCaught, 'Expected exception was caught');
        }

    });

    // Test the length of various runtimeclass constructor functions
    runner.addTest({
        id: 16,
        desc: 'Lengths of Constructor Functions',
        pri: '0',
        test: function () {
            // Factory and simple activatable
            verify(Animals.Animal.length, 0, 'Animals.Animal.length');
            // Simple Activatable
            verify(Animals.Fish.length, 0, 'Animals.Fish.length');
            // Factory Activatable
            verify(Fabrikam.Kitchen.Chef.length, 2, 'Fabrikam.Kitchen.Chef.length');
        }

    });

    Loader42_FileName = "Activation tests"
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
