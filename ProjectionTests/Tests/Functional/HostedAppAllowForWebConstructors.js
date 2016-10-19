if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile('..\\projectionsglue.js'); } 
(function () {

    function getAnimal() {
        // Get an instance of an Animal from TestUtilities (since we can't 'new' one in a Web View host)
        var animalInstance = TestUtilities.AnimalToVar();
        verify(!!animalInstance, true, '!!animalInstance');
        return animalInstance;
    }

    runner.globalSetup(function () {
    });

    runner.addTest({
        id: 1,
        desc: 'Verify that a class without [AllowForWeb] doesn\'t have any constructor function created',
        pri: '0',
        test: function () {
            verify(Animals.Fish, undefined, 'Animals.Fish === undefined');
        }
    });

    runner.addTest({
        id: 2,
        desc: 'Verify that a class with [AllowForWeb] is constructable through namespace',
        pri: '0',
        test: function () {
            // Verify that a function is created to represent the constructor of the object
            verify(typeof Animals.Animal, 'function', 'typeof Animals.Animal');

            // Verify that object can be created by calling the constructor function
            var animal = new Animals.Animal();
            animal.setNumLegs(12345);
            verify(animal.getNumLegs(), 12345);
        }
    });

    runner.addTest({
        id: 3,
        desc: 'Verify that a class with [AllowForWeb] is constructable through constructor property',
        pri: '0',
        preReq: function () {
            return (typeof TestUtilities !== 'undefined');
        },
        test: function () {
            var animal = getAnimal();

            verify(animal.constructor, Animals.Animal, 'animal.constructor');

            var animal2 = new (animal.constructor)();
            animal2.setNumLegs(54321);
            verify(animal2.getNumLegs(), 54321);
        }
    });

    runner.addTest({
        id: 4,
        desc: 'Verify that an object instance of class with [AllowForWeb] is usable (basic)',
        pri: '0',
        preReq: function () {
            return (typeof TestUtilities !== 'undefined');
        },
        test: function () {
            var animal = getAnimal();

            animal.setNumLegs(12345);
            verify(animal.getNumLegs(), 12345);
        }
    });

    Loader42_FileName = 'AllowForWeb - Constructor tests for hosted app context';

})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
