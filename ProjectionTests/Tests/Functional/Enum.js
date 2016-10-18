if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    var animalFactory;
    var myAnimal;

    var phylum = [
   "first : number = '0'",
   "acanthocephala : number = '0'",
   "acoelomorpha : number = '1'",
   "annelida : number = '2'",
   "arthropoda : number = '3'",
   "brachiopoda : number = '4'",
   "bryozoa : number = '5'",
   "chaetognatha : number = '6'",
   "chordata : number = '7'",
   "cnidaria : number = '8'",
   "ctenophora : number = '9'",
   "cycliophora : number = '10'",
   "echinodermata : number = '11'",
   "echiura : number = '12'",
   "entoprocta : number = '13'",
   "gastrotricha : number = '14'",
   "gnathostomulida : number = '15'",
   "hemichordata : number = '16'",
   "kinorhyncha : number = '17'",
   "loricifera : number = '18'",
   "micrognathozoa : number = '19'",
   "mollusca : number = '20'",
   "nematoda : number = '21'",
   "nematomorpha : number = '22'",
   "nemertea : number = '23'",
   "onychophora : number = '24'",
   "orthonectida : number = '25'",
   "phoronida : number = '26'",
   "placozoa : number = '27'",
   "platyhelminthes : number = '28'",
   "porifera : number = '29'",
   "priapulida : number = '30'",
   "rhombozoa : number = '31'",
   "rotifera : number = '32'",
   "sipuncula : number = '33'",
   "tardigrada : number = '34'",
   "xenoturbellid : number = '35'",
   "last : number = '35'"
   ];

    var enumStruct = [
    "current : number = '15'",
    "original : number = '27'"];

    var kitchenEnum = [
    "headChef : number = '0'",
    "assistantChef : number = '1'"];


    function verifyObject(actual, expected, t) {
        verify(typeof actual, t, 'type', false);
        var i = 0;
        for (p in actual) {
            var actualStr = p + " : " + typeof actual[p] + " = '" + actual[p] + "'";
            verify(actualStr, expected[i], actualStr);
            i++;
        }
        verify(i, expected.length, 'number of members', false);
    }

    runner.globalSetup(function () {
        animalFactory = Animals.Animal
        myAnimal = new Animals.Animal(1)
    });

    runner.addTest({
        id: 1,
        desc: 'BasicEnum',
        pri: '0',

        test: function () {
            verifyObject(Animals.Phylum, phylum, 'object');
        }

    });

    runner.addTest({
        id: 2,
        desc: 'BasicEnumValue',
        pri: '0',
        test: function () {
            verify(Animals.Phylum.gnathostomulida, 15, 'Animals.Phylum.gnathostomulida');
        }

    });

    runner.addTest({
        id: 3,
        desc: 'MarshalEnumValue',
        pri: '0',
        test: function () {
            verify(myAnimal.marshalPhylum(Animals.Phylum.gnathostomulida), 15, 'myAnimal.marshalPhylum(Animals.Phylum.gnathostomulida)');
        }

    });

    runner.addTest({
        id: 4,
        desc: 'MarshalEnumValueFromInt',
        pri: '0',
        test: function () {
            verify(myAnimal.marshalPhylum(5), 5, 'myAnimal.marshalPhylum(5)');
        }

    });

    runner.addTest({
        id: 5,
        desc: 'MarshalEnumStruct',
        pri: '0',
        test: function () {
            var s = { current: Animals.Phylum.gnathostomulida, original: Animals.Phylum.placozoa };
            verifyObject(s, enumStruct, 'object');
        }
    });


    runner.addTest({
        id: 6,
        desc: 'ChangeEnumField',
        pri: '0',
        test: function () {
            verify(Animals.Phylum.placozoa, 27, 'Animals.Phylum.placozoa');
            Animals.Phylum.placozoa = 1920954;
            verify(Animals.Phylum.placozoa, 27, 'Animals.Phylum.placozoa');
        }

    });

    runner.addTest({
        id: 7,
        desc: 'AddEnumField',
        pri: '0',
        test: function () {
            Animals.Phylum.popcorn = "Hedgehog";
            // Enums are not extensible so the expected object will not change
            verifyObject(Animals.Phylum, phylum, 'object');
        }

    });

    runner.addTest({
        id: 8,
        desc: 'DeleteBuiltInField',
        pri: '0',
        test: function () {
            delete Animals.Phylum.placozoa;
            verifyObject(Animals.Phylum, phylum, 'object');
        }

    });

    runner.addTest({
        id: 9,
        desc: 'AttemptNew',
        pri: '0',
        test: function () {
            try {
                var p = new Animals.Phylum();
            }
            catch (error) {
                var E_INVALIDARG = -2146827843;
                verify(error.number, E_INVALIDARG, 'error caught');
            }
        }

    });

    runner.addTest({
        id: 10,
        desc: 'PrintPhylum',
        pri: '0',
        test: function () {
            verify(Animals.Phylum + '', '[object Animals.Phylum]', 'Animals.Phylum');
        }

    });

    runner.addTest({
        id: 11,
        desc: 'KitchenEnum',
        pri: '0',
        test: function () {
            verifyObject(Fabrikam.Kitchen.ChefRole, kitchenEnum, 'object');
        }

    });

    runner.addTest({
        id: 12,
        desc: 'AssistantChef',
        pri: '0',
        test: function () {
            verify(Fabrikam.Kitchen.ChefRole.assistantChef, 1, 'Fabrikam.Kitchen.ChefRole.assistantChef');
        }

    });


    runner.addTest({
        id: 13,
        desc: 'FileAccessMode',
        pri: '0',
        test: function () {
            verify(Windows.Storage.FileAccessMode.read, 0, 'Windows.Storage.FileAccessMode.read');
            verify(Windows.Storage.FileAccessMode.readWrite, 1, 'Windows.Storage.FileAccessMode.readWrite');
            verify(Object.keys(Windows.Storage.FileAccessMode).length, 2, "Windows.Storage.FileAccessMode length");
        }

    });

    Loader42_FileName = "Enum tests";

})();

if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
