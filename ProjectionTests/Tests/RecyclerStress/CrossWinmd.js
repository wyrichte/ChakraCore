if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    function verifyResult(actual, expected) {
        var i = 0;
        for (p in actual) {
            verify(p, expected[i][0], 'name');
            verify(actual[p], expected[i][1], p);
            i++;
        }

        verify(i, expected.length, 'number of members');
    }

    function verifyMultipleParam_names_newWeight_outAnimal(actual, expectedNames, expectedWeight) {
        verifyResult(actual.names, expectedNames);
        verify(actual.newWeight, expectedWeight, 'actual.newWeight');
        verifyResult(actual.outAnimal.GetNames(), expectedNames);
    }

    // name, value
    var animalGetNamesExpected = [
        ['common', 'Wolverine'],
        ['scientific', 'Gulo gulo'],
        ['alsoKnownAs', 'Skunk Bear']
    ];

    // name, value
    var animalDimensionsExpected = [
        ['length', 180],
        ['width', 360]
    ];


    runner.addTest({
        id: 1,
        desc: 'AccessOutInterfaceFromAnotherWinmd',
        pri: '0',
        test: function () {
            logger.comment('var myAnimal = new Animals.Animal(1);');
            var myAnimal = new Animals.Animal(1);

            logger.comment('var chef = myAnimal.likesChef()');
            var chef = myAnimal.likesChef();

            verify(chef.name, "Aarti Sequeira", "chef.name");
        }
    });


    Loader42_FileName = 'Cross Winmd tests';

})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
