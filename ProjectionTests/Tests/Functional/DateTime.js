if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    var myElephant;
    var currentId = 1;
    var dateProto = (new Date()).prototype;

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

    function verifyDateFilter(date, filter) {
        verify(date.prototype === dateProto, true, 'Check object prototype is date');

        for (var field in filter)
        {
            verify(date['get' + field](), filter[field], 'Check if ' + field + ' matches');
        }
    }

    function doubleEqualWithPrecisionLoss(d1, d2)
    {
        return (Math.abs(d1 - d2) <= 1);
    }

    function writeLine(str) {
        logger.comment(str);
    }

    runner.globalSetup(function () {
        var elephantFactory = Animals.Elephant;
        myElephant = new elephantFactory(1);
    });

    runner.addTest({
        id: currentId++,
        desc: 'GetDateTime - Retrieve DateTime as out param',
        pri: '0',
        test: function () {
            // the initial age of the elephant is set to DateTime = 0
            var initialDate = myElephant.getAge();

            // Print out date in local time- this verifies the to*string functions too
            writeLine("Date is " + initialDate);
            writeLine("UTC Date is " + initialDate.toUTCString());

            // WinRT 0 = 1/1/1601, 00:00:00 UTC
            // Javascript's month number is 0 indexed, which is why month is 0
            // We use UTCFullYear since ES5 dictates that getYear returns Year - 1901- UTCFullYear was clearer
            verifyDateFilter(initialDate, { UTCDay: 1, UTCMonth: 0, UTCFullYear: 1601, UTCHours: 0, UTCMinutes: 0, UTCSeconds: 0 });
        }
    });

    runner.addTest({
        id: currentId++,
        desc: 'GetDateTime - Retrieve DateTime as out param, verify milliseconds',
        pri: '0',
        test: function () {
            // the initial age of the elephant is set to DateTime = 0
            var initialDate = myElephant.getAge();

            verify(initialDate.getTime(), -11644473600000, "Check number of milliseconds since ES5 epoch");
        }
    });

    runner.addTest({
        id: currentId++,
        desc: 'GetDateTime - Retrieve DateTime as out param and modify it',
        pri: '0',
        test: function () {
            // the initial age of the elephant is set to DateTime = 0
            var initialDate = myElephant.getAge();

            initialDate.setFullYear(2010, 1, 1);
            
            verify(initialDate.getTime(), 1265068800000, "Verify that the time retrieved is what we'd set");
        }
    });

    runner.addTest({
        id: currentId++,
        desc: 'GetDateTime - Retrieve DateTime as out param, and round-trip it',
        pri: '0',
        test: function () {
            // the initial age of the elephant is set to DateTime = 0
            var initialDate = myElephant.getAge();

            myElephant.setAge(initialDate);
            var modifiedDate = myElephant.getAge();
            
            verify(modifiedDate.getTime(), -11644473600000, "Verify that the DateTime round-tripped correctly");
        }
    });

    runner.addTest({
        id: currentId++,
        desc: 'GetDateTime - Retrieve DateTime as out param, and round-trip it, but first we modify it to invalidate it',
        pri: '0',
        test: function () {
            // the initial age of the elephant is set to DateTime = 0
            var initialDate = myElephant.getAge();

            initialDate.setFullYear(2010, 1, 1);
            
            myElephant.setAge(initialDate);
            var modifiedDate = myElephant.getAge();
            
            verify(modifiedDate.getTime(), 1265068800000, "Verify that the DateTime round-tripped correctly");
        }
    });

    runner.addTest({
        id: currentId++,
        desc: 'DateTime - Verify precision is maintained during round-tripping',
        pri: '0',
        test: function () {
            var timeInMilliseconds = 1265068800000; // 2-2-1641
            var timeInTicks = timeInMilliseconds * 10000 + 5; // Set an arbitrary number of ticks
            myElephant.setAgeTicks(timeInTicks);
            var age = myElephant.getAge();
            var jsEpochMilliseconds = 11644473600000;

            // Since we added 5 nanoseconds, that causes the number of milliseconds after truncation to be off by 1
            verify(doubleEqualWithPrecisionLoss(age.getTime(), timeInMilliseconds- jsEpochMilliseconds), true, "Verify that the time retrieved is correct");
            myElephant.setAge(age);
            verify(myElephant.getAgeTicks() == timeInTicks, true, "Verify that the time retrieved in ticks is correct");

            age.setFullYear(1641); // This should cause precision loss even though the referenced date doesn't really change
            myElephant.setAge(age);
            verify(myElephant.getAgeTicks() == timeInTicks, false, "Verify that the precision is lost");
        }
    });

    runner.addTest({
        id: currentId++,
        desc: 'DateTime - Check equality of retrieved dates',
        pri: '0',
        test: function () {
            // the initial age of the elephant is set to DateTime = 0
            myElephant.setAgeTicks(0);
            var date1= myElephant.getAge();
            var date2= myElephant.getAge();

            verify(date1 == date2, false, "Verify that the two ages are seperate objects");
            verify(date1.valueOf() == date2.valueOf(), true, "Verify that the value of the two ages is the same");

            date1.setUTCFullYear(1601); // Basically an identity operation but this will change the backing store
            verify(date1 == date2, false, "Verify that the two ages are seperate objects");
            verify(date1.valueOf() == date2.valueOf(), true, "Verify that the value of the two ages is the same"); // Verify that precision doesn't matter during compare
            
            var initialDate = new Date(1265068800000); // 2-2-2010
            myElephant.setAge(initialDate);
            var modifiedDate = myElephant.getAge();

            // TODO: Uncomment this when we handle implicitly converting JavascriptDate to JavascriptWinRTDate in the projection layer
            // verify(modifiedDate.getTime(), 1265068800000, "Verify that the DateTime round-tripped correctly");
        }
    });

    runner.addTest({
        id: currentId++,
        desc: 'DateTime - Verify valueOf returns double value',
        pri: '0',
        test: function () {
            var timeInMilliseconds = 1265068800000; // 2-2-1641
            var timeInTicks = timeInMilliseconds * 10000 + 5; // Set an arbitrary number of ticks
            myElephant.setAgeTicks(timeInTicks);
            var age = myElephant.getAge();
            var jsEpochMilliseconds = 11644473600000;

            age.setFullYear(1641); // This should cause precision loss even though the referenced date doesn't really change
            // Since we added 5 nanoseconds, that causes the number of milliseconds after truncation to be off by 1
            verify(doubleEqualWithPrecisionLoss(age.valueOf(), timeInMilliseconds- jsEpochMilliseconds), true, "Verify that the time retrieved is correct");
        }
    });

    runner.addTest({
        id: currentId++,
        desc: 'DateTime - Override builtin method test',
        pri: '0',
        test: function () {
            var timeInMilliseconds = 1265068800000; // 2-2-1641
            var timeInTicks = timeInMilliseconds * 10000 + 5; // Set an arbitrary number of ticks
            myElephant.setAgeTicks(timeInTicks);
            var age = myElephant.getAge();
            var jsEpochMilliseconds = 11644473600000;

            var oldValueOf = Date.prototype.valueOf;
            Date.prototype.valueOf = function() { return 1986; }

            age.setFullYear(1641); // This should cause precision loss even though the referenced date doesn't really change
            // Since we added 5 nanoseconds, that causes the number of milliseconds after truncation to be off by 1
            verify(age.valueOf() == 1986, true, "Verify that valueOf retrieved is correct");

            Date.prototype.valueOf = oldValueOf;
        }
    });
    
    runner.addTest({
        id: currentId++,
        desc: 'DateTime -verify infinity test',
        pri: '0',
        test: function () {
            var d = new Date(Infinity);
            var exceptionThrown = false;

            // Uncomment when bug 239440 is fixed
            try {
                // myElephant.setAge(d);
            } catch (e) {
                exceptionThrown = true;
            }

            // verify(exceptionThrown, true, "Verify converting infinity to DateTime");
        }
    });

    runner.addTest({
        id: currentId++,
        desc: 'DateTime - Verify instanceof test',
        pri: '0',
        test: function () {
            var timeInMilliseconds = 1265068800000; // 2-2-1641
            var timeInTicks = timeInMilliseconds * 10000;
            myElephant.setAgeTicks(timeInTicks);
            var age = myElephant.getAge();

            verify(age instanceof Date, true, "Verify that age is a date");
        }
    });

    runner.addTest({
        id: currentId++,
        desc: 'TimeSpan - Verify we marshal out a TimeSpan of 2000 as 0.2',
        pri: '0',
        test: function () {
            var timespan = myElephant.getTimeToGetToSixtyMPH();
            verify(timespan, 0.2, "Verify that we property marshal a TimeSpan of 2000 as 0.2");
            verify(typeof timespan, typeof 0, "Verify that we property marshal a TimeSpan is marshaled as a Number");
        }
    });

    runner.addTest({
        id: currentId++,
        desc: 'TimeSpan - Verify we can marshal in a TimeSpan',
        pri: '0',
        test: function () {
            myElephant.setTimeToGetToSixtyMPH(1);
            var timespan = myElephant.getTimeToGetToSixtyMPH();
            verify(timespan, 1, "Verify that we property marshal the TimeSpan value we had set");
        }
    });

    Loader42_FileName = "WinRT DateTime tests";
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
