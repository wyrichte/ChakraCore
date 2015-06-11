if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    var animalFactory;
    var myAnimal;

    var expected1 = [
	"stage : string = 'Stage test'",
	"bytesSent : number = '123'",
	"totalBytesToSend : number = '1234'",
	"bytesReceived : number = '321'",
	"totalBytesToReceive : number = '1234'",
	"retries : number = '2'" ];

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
        desc: 'TestOutBug258665_HttpProgress - complex generic type instantiation as null out',
        pri: '0',
        test: function () {
			try {
				var fromWinRT = myAnimal.testOutBug258665_HttpProgressAsOptEmpty();
				verify.equal(fromWinRT, null, "object");
			}
			catch(e) {
				logger.comment("CAUGHT EXCEPTION: " + e);
			}
        }
    });
	
    runner.addTest({
        id: 2,
        desc: 'TestOutBug258665_HttpProgress - simple generic type instantiation as null out',
        pri: '0',
        test: function () {
            var fromWinRT = myAnimal.testOutBug258665_HttpProgressAsOptIntEmpty();
            verify.equal(fromWinRT, null, "object");
        }
    });

    runner.addTest({
        id: 3,
        desc: 'TestOutBug258665_HttpProgress - complex generic type instantiation',
        pri: '0',
        test: function () {
            var fromWinRT = myAnimal.testOutBug258665_HttpProgress("foo");
            verifyObject(fromWinRT, expected1, "object");
        }
    });

    Loader42_FileName = "IReference Struct tests - bug 258665 repro";
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
