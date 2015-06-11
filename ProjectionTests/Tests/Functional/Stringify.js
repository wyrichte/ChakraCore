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

    runner.addTest({
        id: '0',
        desc: 'BLUE: 202724 - JSON.stringify() fails with long long values returned from WinRT',
        pri: '0',
        test: function () {
            var a = new Animals.Animal(0);
            var j = JSON.stringify(a.testBug202724_GetInt64());
            
            verify.defined(j, "JSON.stringify(a.testBug202724_GetInt64())");
        }
    });
    
    runner.addTest({
        id: '1',
        desc: 'BLUE: 202724 - JSON.stringify() fails with unsigned long long values returned from WinRT',
        pri: '0',
        test: function () {
            var a = new Animals.Animal(0);
            var j = JSON.stringify(a.testBug202724_GetUInt64());
            
            verify.defined(j, "JSON.stringify(a.testBug202724_GetUInt64())");
        }
    });
    
    runner.addTest({
        id: '2',
        desc: 'JSON.stringify() called with replacer function to ignore Int64 value',
        pri: '0',
        test: function () {
            var a = new Animals.Animal(0);
			var v = a.testBug202724_GetUInt64();
			var replacer = function(key, value) { 
				if(value == v) {
					return undefined;
				}
				return value;
			}
            var j = JSON.stringify({'v1':1, 'v2':v, 'v3':v+1}, replacer, ' ');
            
            verify.defined(j, "JSON.stringify() result");
        }
    });
    
    runner.addTest({
        id: '3',
        desc: 'JSON.stringify() called with replacer function which returns Int64 values',
        pri: '0',
        test: function () {
            var a = new Animals.Animal(0);
			var v = a.testBug202724_GetUInt64();
			var replacer = function(key, value) { 
				if(key == 'v1' || key == 'v2') {
					return v;
				}
				
				return value;
			}
            var j = JSON.stringify({'v1':1, 'v2':0, 'v3':v+1}, replacer, ' ');
            
            verify.defined(j, "JSON.stringify() result");
        }
    });
    
    runner.addTest({
        id: '4',
        desc: 'JSON.stringify() called with simple replacer and value contains Int64 value',
        pri: '0',
        test: function () {
            var a = new Animals.Animal(0);
			var v = a.testBug202724_GetUInt64();
			var replacer = function(key, value) { 
				return value;
			}
            var j = JSON.stringify({'v1':1, 'v2':v, 'v3':v+1}, replacer, ' ');
            
            verify.defined(j, "JSON.stringify() result");
        }
    });

	runner.addTest({
        id: '5',
        desc: 'JSON.stringify() called with Int64 space argument',
        pri: '0',
        test: function () {
            var a = new Animals.Animal(0);
			var v = a.testBug202724_GetUInt64();
			var replacer = function(key, value) { 
				return value;
			}
            var j = JSON.stringify({'v1':1, 'v2':v, 'v3':v+1}, replacer, v);
            
            verify.defined(j, "JSON.stringify() result");
        }
    });

	runner.addTest({
        id: '6',
        desc: 'JSON.stringify() called with Int64 space argument (converted to JavascriptNumber)',
        pri: '0',
        test: function () {
            var a = new Animals.Animal(0);
			var v = a.testBug202724_GetUInt64();
			var replacer = function(key, value) { 
				return value;
			}
            var j = JSON.stringify({'v1':1, 'v2':v, 'v3':v+1}, replacer, v+1);
            
            verify.defined(j, "JSON.stringify() result");
        }
    });

	runner.addTest({
        id: '7',
        desc: 'JSON.stringify() called with array replacer argument',
        pri: '0',
        test: function () {
            var a = new Animals.Animal(0);
			var v = a.testBug202724_GetUInt64();
			var replacer = [ 'v1', 'v2', 'v3' ];
            var j = JSON.stringify({'v1':1, 'v2':v, 'v3':v+1}, replacer, undefined);
            
            verify.defined(j, "JSON.stringify() result");
        }
    });

	runner.addTest({
        id: '8',
        desc: 'JSON.stringify() called with array replacer argument containing Int64 value',
        pri: '0',
        test: function () {
            var a = new Animals.Animal(0);
			var v = a.testBug202724_GetUInt64();
			var replacer = [ 'v2', v ];
            var j = JSON.stringify({'v1':v, 'v2':v, 'v3':v}, replacer, undefined);
            
            verify.defined(j, "JSON.stringify() result");
        }
    });

	runner.addTest({
        id: '9',
        desc: 'JSON.stringify() called with array replacer argument containing numeric values (Including Int64)',
        pri: '0',
        test: function () {
            var a = new Animals.Animal(0);
			var v = a.testBug202724_GetUInt64();
			var replacer = [ 2, v ];
            var j = JSON.stringify({1:v, 2:v, 3:v}, replacer, undefined);
            
            verify.defined(j, "JSON.stringify() result");
        }
    });

	runner.addTest({
        id: '10',
        desc: 'JSON.stringify() called with object containing mixture of numeric and Int64 keys and replacer values',
        pri: '0',
        test: function () {
            var a = new Animals.Animal(0);
			var v = a.testBug202724_GetUInt64();
			var replacer = [ 1, 2, v, 'v', 4294967296 ];
            var j = JSON.stringify({1:v, 2:v, 3:v, v:v, 4294967296:v}, replacer, undefined);
            
            verify.defined(j, "JSON.stringify() result");
        }
    });

	runner.addTest({
        id: '11',
        desc: 'JSON.stringify() called with array value object containing Int64 values',
        pri: '0',
        test: function () {
            var a = new Animals.Animal(0);
			var v = a.testBug202724_GetUInt64();
            var j = JSON.stringify([1, v-1, v, v+1], undefined, undefined);
            
            verify.defined(j, "JSON.stringify() result");
        }
    });

	runner.addTest({
        id: '12',
        desc: 'JSON.stringify() called with numeric key and replacer value (Int64 value binding)',
        pri: '1',
        test: function () {
            var a = new Animals.Animal(0);
			var v = a.testBug202724_GetUInt64();
			v = 200;
			var replacer = [ v ];
            var j = JSON.stringify({200:v}, replacer, undefined);
            
            verify.defined(j, "JSON.stringify() result");
        }
    });

	runner.addTest({
        id: '13',
        desc: 'JSON.stringify() called with replacer including Int64 value matching object key',
        pri: '0',
        test: function () {
            var a = new Animals.Animal(0);
			var v = a.testBug202724_GetUInt64(); // 4294967296
			var replacer = [ v ];
            var j = JSON.stringify({4294967296:v}, replacer, undefined);
            
            verify.defined(j, "JSON.stringify() result");
        }
    });

    Loader42_FileName = "JSON.Stringify / Int64 interaction tests";
})();

if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
