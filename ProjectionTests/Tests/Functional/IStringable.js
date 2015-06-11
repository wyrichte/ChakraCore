if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    var setTimeOutCalls = 0;
    var savedSetTimeout = undefined;
    if (typeof setTimeout !== 'undefined')
        savedSetTimeout = setTimeout;
    runner.globalSetup(function () {
        setTimeout = function () { setTimeOutCalls = setTimeOutCalls + 1; }
    });
    runner.globalTeardown(function () {
        setTimeout = savedSetTimeout;
    });

    function toStringObject(obj, name) {
		var str = "\n";
		
		str += "Implicit " + name + ".toString(): \n";
		try {
			str += obj;
		} catch(e) {
			str += "Error: " + e.message;
		}
		
		str += "\nExplicit " + name + ".toString(): \n";
		try {
			str += obj.toString();
		} catch(e) {
			str += "Error: " + e.message;
		}
		
		logger.comment(str);
	}

    runner.addTest({
        id: '1',
        desc: 'Non-IStringable class and proto',
        pri: '0',
        test: function () {
            var a = new Animals.Animal(1);
			toStringObject(a, "new Animals.Animal(1)");
			toStringObject(a.__proto__, "new Animals.Animal(1).__proto__");
			toStringObject(Animals.Animal, "Animals.Animal");
			toStringObject(Animals.Animal.prototype, "Animals.Animal.prototype");
        }
    });

    runner.addTest({
        id: '2',
        desc: 'IStringable class and proto',
        pri: '0',
        test: function () {
            var proto = DevTests.Repros.Stringables.SimpleStringable;
            var obj = new DevTests.Repros.Stringables.SimpleStringable();
			
			toStringObject(obj, "new DevTests.Repros.Stringables.SimpleStringable()");
			toStringObject(obj.__proto__, "new DevTests.Repros.Stringables.SimpleStringable().__proto__");
			toStringObject(DevTests.Repros.Stringables.SimpleStringable, "DevTests.Repros.Stringables.SimpleStringable");
			toStringObject(DevTests.Repros.Stringables.SimpleStringable.prototype, "DevTests.Repros.Stringables.SimpleStringable.prototype");
        }
    });

    Loader42_FileName = "IStringable tests";
})();

if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
