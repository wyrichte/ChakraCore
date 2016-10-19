if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {

    var ticksPerMillisecond = 10000;
    var ticksPerSecond = ticksPerMillisecond * 1000;
    var ticksPerMinute = ticksPerSecond * 60;
    var ticksPerHour = ticksPerMinute * 60;
    var ticksPerDay = ticksPerHour * 24;
    var jsEpochMilliseconds = 11644473600000;
    var jsEpochTicks = jsEpochMilliseconds * ticksPerMillisecond;
    var months = [0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334]

    var testClass;
    runner.globalSetup(function () {
        testClass = new DevTests.DateTimeAndTimeSpan.Tests();
    });

    verify.members = function verifyMembers(actual, expected, name) {
        // Only check instance and immediate prototype
        var instanceProperties = Object.getOwnPropertyNames(actual);
        var prototypeProperties = Object.getOwnPropertyNames(Object.getPrototypeOf(actual));
        var actualMembers = instanceProperties.concat(prototypeProperties);

        instanceProperties = Object.getOwnPropertyNames(expected);
        prototypeProperties = Object.getOwnPropertyNames(Object.getPrototypeOf(expected));
        var expectedMembers = instanceProperties.concat(prototypeProperties);
        for (var p in expectedMembers) {
            verify.notEqual(actualMembers.indexOf(expectedMembers[p]), -1, name + "." + expectedMembers[p] + " should be defined");
        }
        verify(actualMembers.length, expectedMembers.length, "Number of members of " + name);
    }

    verify.dateFilter = function (date, filter) {
        verify.instanceOf(date, Date);

        for (var field in filter) {
            verify(date['get' + field](), filter[field], 'Check if ' + field + ' matches');
        }
    }

    function getTicksFromDate(year, month, day, hours, minutes, seconds, milliseconds, offset) {
        var dateRep = Date.UTC(year, month, day, hours, minutes, seconds, milliseconds);
        var result = dateRep * ticksPerMillisecond;
        if (offset !== undefined) { result = result + offset; }
        return (result + jsEpochTicks);
    }

    // DateTime Tests
    runner.addTest({
        id: 1,
        desc: 'DateTime projected as Date',
        pri: '0',
        test: function () {
            // Verify Windows.Foundation.DateTime is not projected
            verify.notDefined(Windows.Foundation.DateTime, "Windows.Foundation.DateTime");

            var dateTime0 = testClass.produceDateTime(0);
            // Verify projected DateTime has typeof object
            verify(typeof dateTime0, "object", "typeof dateTime0");
            // Verify projected DateTime is instanceof Date
            verify.instanceOf(dateTime0, Date, true);
            // Verify projected DateTime has prototype of Date
            verify(Object.getPrototypeOf(dateTime0), Date.prototype, "Object.getPrototypeOf(dateTime0)");
            // Verify projected DateTime has equivalent members to Javascript Date
            verify.members(dateTime0, new Date(), "dateTime0");

            // Verify JS and WinRT epochs
            verify(dateTime0.getTime(), -jsEpochMilliseconds, "dateTime0.getTime()");
            var dateTimeJsEpoch = testClass.produceDateTime(jsEpochTicks);
            verify(dateTimeJsEpoch.getTime(), 0, "dateTimeJsEpoch.getTime()");

            var date0 = new Date(0);
            verify(testClass.verifyDateTime(date0, jsEpochTicks), true, "testClass.verifyDateTime(date0, jsEpochTicks)");
            var dateWinRTEpoch = new Date(-jsEpochMilliseconds);
            verify(testClass.verifyDateTime(dateWinRTEpoch, 0), true, "testClass.verifyDateTime(dateWinRTEpoch, 0)");
        }
    });

    runner.addTest({
        id: 2,
        desc: 'Simple marshaling of DateTime [in] and [out]',
        pri: '0',
        test: function () {
            // Verify Date as DateTime [in]
            var dateIn = new Date(Date.UTC(1756, 1, 27, 8, 24, 42, 763));
            var succeeded = testClass.verifyDateTime(dateIn, getTicksFromDate(1756, 1, 27, 8, 24, 42, 763, 0));
            verify(succeeded, true, "testsClass.verifyDateTime(dateIn, 1756, 1, 27, 8, 24, 42, 763, 0)");
            // Verify Date object round-tripped through WinRT
            verify(testClass.roundTripDateTime(dateIn).getTime(), dateIn.getTime(), "testClass.roundTripDateTime(dateIn).getTime()");

            // Verify DateTime [out] as Date
            var dateOut = testClass.produceDateTime(getTicksFromDate(1215, 7, 15, 18, 31, 2, 122, 0));
            verify.dateFilter(dateOut, { UTCFullYear: 1215, UTCMonth: 7, UTCDate: 15, UTCHours: 18, UTCMinutes: 31, UTCSeconds: 2, UTCMilliseconds: 122 });
            // Verify DateTime round-tripped through JS (no modification)
            dateOut = testClass.produceDateTime(2000, 1, 1, 0, 0, 0, 0, 1);
            succeeded = testClass.verifyDateTime(dateOut, 2000, 1, 1, 0, 0, 0, 0, 1);
            verify(succeeded, true, "testClass.verifyDateTime(dateOut, 2000, 1, 1, 0, 0, 0, 0, 1)");
        }
    });

    runner.addTest({
        id: 3,
        desc: 'Marshal invalid types as DateTime',
        pri: '0',
        test: function () {
            var invalidTypes = {
                object: { UniversalTime: 0 },
                int: 0,
                double: 4.2,
                numericString: jsEpochMilliseconds.toString(),
                "function": function () { return 7; },
                "null": null,
                "undefined": undefined,
                datePrototype: Date.prototype,
                dateArray: [new Date()],
                dateObject: { a: new Date() }
            };

            for (var type in invalidTypes) {
                logger.comment("Try to marshal " + type + " as DateTime[in]");
                verify.exception(function () {
                    return testClass.roundTripDateTime(invalidTypes[type]);
                }, TypeError, "Marshal " + type);
            }
        }
    });

    runner.addTest({
        id: 4,
        desc: 'Verify correctness of DateTime conversion to Date',
        pri: '0',
        test: function () {
            var time_positive = {
                millisecond: {
                    offset: ticksPerMillisecond,
                    filter: { UTCFullYear: 1601, UTCMonth: 0, UTCDate: 1, UTCHours: 0, UTCMinutes: 0, UTCSeconds: 0, UTCMilliseconds: 1 }
                },
                second: {
                    offset: ticksPerSecond,
                    filter: { UTCFullYear: 1601, UTCMonth: 0, UTCDate: 1, UTCHours: 0, UTCMinutes: 0, UTCSeconds: 1, UTCMilliseconds: 0 }
                },
                minute: {
                    offset: ticksPerMinute,
                    filter: { UTCFullYear: 1601, UTCMonth: 0, UTCDate: 1, UTCHours: 0, UTCMinutes: 1, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                hour: {
                    offset: ticksPerHour,
                    filter: { UTCFullYear: 1601, UTCMonth: 0, UTCDate: 1, UTCHours: 1, UTCMinutes: 0, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                day: {
                    offset: ticksPerDay,
                    filter: { UTCFullYear: 1601, UTCMonth: 0, UTCDate: 2, UTCHours: 0, UTCMinutes: 0, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                week: {
                    offset: ticksPerDay * 7,
                    filter: { UTCFullYear: 1601, UTCMonth: 0, UTCDate: 8, UTCHours: 0, UTCMinutes: 0, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                month: {
                    offset: getTicksFromDate(1601, 1, 1, 0, 0, 0, 0, 0),
                    filter: { UTCFullYear: 1601, UTCMonth: 1, UTCDate: 1, UTCHours: 0, UTCMinutes: 0, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                year: {
                    offset: getTicksFromDate(1602, 0, 1, 0, 0, 0, 0, 0),
                    filter: { UTCFullYear: 1602, UTCMonth: 0, UTCDate: 1, UTCHours: 0, UTCMinutes: 0, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                years_2: {
                    offset: getTicksFromDate(1603, 0, 1, 0, 0, 0, 0, 0),
                    filter: { UTCFullYear: 1603, UTCMonth: 0, UTCDate: 1, UTCHours: 0, UTCMinutes: 0, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                years_5: {
                    offset: getTicksFromDate(1606, 0, 1, 0, 0, 0, 0, 0),
                    filter: { UTCFullYear: 1606, UTCMonth: 0, UTCDate: 1, UTCHours: 0, UTCMinutes: 0, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                years_10: {
                    offset: getTicksFromDate(1611, 0, 1, 0, 0, 0, 0, 0),
                    filter: { UTCFullYear: 1611, UTCMonth: 0, UTCDate: 1, UTCHours: 0, UTCMinutes: 0, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                years_50: {
                    offset: getTicksFromDate(1651, 0, 1, 0, 0, 0, 0, 0),
                    filter: { UTCFullYear: 1651, UTCMonth: 0, UTCDate: 1, UTCHours: 0, UTCMinutes: 0, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                years_100: {
                    offset: getTicksFromDate(1701, 0, 1, 0, 0, 0, 0, 0),
                    filter: { UTCFullYear: 1701, UTCMonth: 0, UTCDate: 1, UTCHours: 0, UTCMinutes: 0, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                years_500: {
                    offset: getTicksFromDate(2101, 0, 1, 0, 0, 0, 0, 0),
                    filter: { UTCFullYear: 2101, UTCMonth: 0, UTCDate: 1, UTCHours: 0, UTCMinutes: 0, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                years_1000: {
                    offset: getTicksFromDate(2601, 0, 1, 0, 0, 0, 0, 0),
                    filter: { UTCFullYear: 2601, UTCMonth: 0, UTCDate: 1, UTCHours: 0, UTCMinutes: 0, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                years_5000: {
                    offset: getTicksFromDate(6601, 0, 1, 0, 0, 0, 0, 0),
                    filter: { UTCFullYear: 6601, UTCMonth: 0, UTCDate: 1, UTCHours: 0, UTCMinutes: 0, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                years_10000: {
                    offset: getTicksFromDate(11601, 0, 1, 0, 0, 0, 0, 0),
                    filter: { UTCFullYear: 11601, UTCMonth: 0, UTCDate: 1, UTCHours: 0, UTCMinutes: 0, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                years_25000: {
                    offset: getTicksFromDate(26601, 0, 1, 0, 0, 0, 0, 0),
                    filter: { UTCFullYear: 26601, UTCMonth: 0, UTCDate: 1, UTCHours: 0, UTCMinutes: 0, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                max: {
                    offset: testClass.getInt64Max(),
                    filter: { UTCFullYear: 30828, UTCMonth: 8, UTCDate: 14, UTCHours: 2, UTCMinutes: 48, UTCSeconds: 5, UTCMilliseconds: 477 }
                }
            };
            var time_negative = {
                millisecond: {
                    offset: -ticksPerMillisecond,
                    filter: { UTCFullYear: 1600, UTCMonth: 11, UTCDate: 31, UTCHours: 23, UTCMinutes: 59, UTCSeconds: 59, UTCMilliseconds: 999 }
                },
                second: {
                    offset: -ticksPerSecond,
                    filter: { UTCFullYear: 1600, UTCMonth: 11, UTCDate: 31, UTCHours: 23, UTCMinutes: 59, UTCSeconds: 59, UTCMilliseconds: 0 }
                },
                minute: {
                    offset: -ticksPerMinute,
                    filter: { UTCFullYear: 1600, UTCMonth: 11, UTCDate: 31, UTCHours: 23, UTCMinutes: 59, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                hour: {
                    offset: -ticksPerHour,
                    filter: { UTCFullYear: 1600, UTCMonth: 11, UTCDate: 31, UTCHours: 23, UTCMinutes: 0, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                day: {
                    offset: -ticksPerDay,
                    filter: { UTCFullYear: 1600, UTCMonth: 11, UTCDate: 31, UTCHours: 0, UTCMinutes: 0, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                week: {
                    offset: -(ticksPerDay * 7),
                    filter: { UTCFullYear: 1600, UTCMonth: 11, UTCDate: 25, UTCHours: 0, UTCMinutes: 0, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                month: {
                    offset: getTicksFromDate(1600, 11, 1, 0, 0, 0, 0, 0),
                    filter: { UTCFullYear: 1600, UTCMonth: 11, UTCDate: 1, UTCHours: 0, UTCMinutes: 0, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                year: {
                    offset: getTicksFromDate(1600, 0, 1, 0, 0, 0, 0, 0),
                    filter: { UTCFullYear: 1600, UTCMonth: 0, UTCDate: 1, UTCHours: 0, UTCMinutes: 0, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                years_2: {
                    offset: getTicksFromDate(1599, 0, 1, 0, 0, 0, 0, 0),
                    filter: { UTCFullYear: 1599, UTCMonth: 0, UTCDate: 1, UTCHours: 0, UTCMinutes: 0, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                years_5: {
                    offset: getTicksFromDate(1596, 0, 1, 0, 0, 0, 0, 0),
                    filter: { UTCFullYear: 1596, UTCMonth: 0, UTCDate: 1, UTCHours: 0, UTCMinutes: 0, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                years_10: {
                    offset: getTicksFromDate(1591, 0, 1, 0, 0, 0, 0, 0),
                    filter: { UTCFullYear: 1591, UTCMonth: 0, UTCDate: 1, UTCHours: 0, UTCMinutes: 0, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                years_50: {
                    offset: getTicksFromDate(1551, 0, 1, 0, 0, 0, 0, 0),
                    filter: { UTCFullYear: 1551, UTCMonth: 0, UTCDate: 1, UTCHours: 0, UTCMinutes: 0, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                years_100: {
                    offset: getTicksFromDate(1501, 0, 1, 0, 0, 0, 0, 0),
                    filter: { UTCFullYear: 1501, UTCMonth: 0, UTCDate: 1, UTCHours: 0, UTCMinutes: 0, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                years_500: {
                    offset: getTicksFromDate(1101, 0, 1, 0, 0, 0, 0, 0),
                    filter: { UTCFullYear: 1101, UTCMonth: 0, UTCDate: 1, UTCHours: 0, UTCMinutes: 0, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                years_1000: {
                    offset: getTicksFromDate(601, 0, 1, 0, 0, 0, 0, 0),
                    filter: { UTCFullYear: 601, UTCMonth: 0, UTCDate: 1, UTCHours: 0, UTCMinutes: 0, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                years_5000: {
                    offset: getTicksFromDate(-4601, 0, 1, 0, 0, 0, 0, 0),
                    filter: { UTCFullYear: -4601, UTCMonth: 0, UTCDate: 1, UTCHours: 0, UTCMinutes: 0, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                years_10000: {
                    offset: getTicksFromDate(-9601, 0, 1, 0, 0, 0, 0, 0),
                    filter: { UTCFullYear: -9601, UTCMonth: 0, UTCDate: 1, UTCHours: 0, UTCMinutes: 0, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                years_25000: {
                    offset: getTicksFromDate(-24601, 0, 1, 0, 0, 0, 0, 0),
                    filter: { UTCFullYear: -24601, UTCMonth: 0, UTCDate: 1, UTCHours: 0, UTCMinutes: 0, UTCSeconds: 0, UTCMilliseconds: 0 }
                },
                min: {
                    offset: testClass.getInt64Min(),
                    filter: { UTCFullYear: -27627, UTCMonth: 3, UTCDate: 19, UTCHours: 21, UTCMinutes: 11, UTCSeconds: 54, UTCMilliseconds: 523 }
                }
            };

            for (var time in time_positive) {
                logger.comment("Verify correctness of DateTime projected as Date with a positive offset of: " + time);
                var dateTime = testClass.produceDateTime(time_positive[time].offset);
                verify(dateTime.getTime(), Math.floor((time_positive[time].offset / ticksPerMillisecond) - jsEpochMilliseconds), "dateTime.getTime()");
                verify.dateFilter(dateTime, time_positive[time].filter);
            }
            for (var time in time_negative) {
                logger.comment("Verify correctness of DateTime projected as Date with a negative offset of: " + time);
                var dateTime = testClass.produceDateTime(time_negative[time].offset);
                verify(dateTime.getTime(), Math.ceil(time_negative[time].offset / ticksPerMillisecond) - jsEpochMilliseconds, "dateTime.getTime()");
                verify.dateFilter(dateTime, time_negative[time].filter);
            }
        }
    });

    runner.addTest({
        id: 5,
        desc: 'Verify correctness of Date conversion to DateTime',
        pri: '0',
        test: function () {
            var time_positive = {
                millisecond: {
                    time: Date.UTC(1601, 0, 1, 0, 0, 0, 1),
                    winRTTime: ticksPerMillisecond
                },
                second: {
                    time: Date.UTC(1601, 0, 1, 0, 0, 1, 0),
                    winRTTime: ticksPerSecond
                },
                minute: {
                    time: Date.UTC(1601, 0, 1, 0, 1, 0, 0),
                    winRTTime: ticksPerMinute
                },
                hour: {
                    time: Date.UTC(1601, 0, 1, 1, 0, 0, 0),
                    winRTTime: ticksPerHour
                },
                day: {
                    time: Date.UTC(1601, 0, 2, 0, 0, 0, 0),
                    winRTTime: ticksPerDay
                },
                week: {
                    time: Date.UTC(1601, 0, 8, 0, 0, 0, 0),
                    winRTTime: ticksPerDay * 7
                },
                month: {
                    time: Date.UTC(1601, 1, 1, 0, 0, 0, 0),
                    winRTTime: getTicksFromDate(1601, 1, 1, 0, 0, 0, 0, 0)
                },
                year: {
                    time: Date.UTC(1602, 0, 1, 0, 0, 0, 0),
                    winRTTime: getTicksFromDate(1602, 0, 1, 0, 0, 0, 0, 0)
                },
                years_2: {
                    time: Date.UTC(1603, 0, 1, 0, 0, 0, 0),
                    winRTTime: getTicksFromDate(1603, 0, 1, 0, 0, 0, 0, 0)
                },
                years_5: {
                    time: Date.UTC(1606, 0, 1, 0, 0, 0, 0),
                    winRTTime: getTicksFromDate(1606, 0, 1, 0, 0, 0, 0, 0)
                },
                years_10: {
                    time: Date.UTC(1611, 0, 1, 0, 0, 0, 0),
                    winRTTime: getTicksFromDate(1611, 0, 1, 0, 0, 0, 0, 0)
                },
                years_50: {
                    time: Date.UTC(1651, 0, 1, 0, 0, 0, 0),
                    winRTTime: getTicksFromDate(1651, 0, 1, 0, 0, 0, 0, 0)
                },
                years_100: {
                    time: Date.UTC(1701, 0, 1, 0, 0, 0, 0),
                    winRTTime: getTicksFromDate(1701, 0, 1, 0, 0, 0, 0, 0)
                },
                years_500: {
                    time: Date.UTC(2101, 0, 1, 0, 0, 0, 0),
                    winRTTime: getTicksFromDate(2101, 0, 1, 0, 0, 0, 0, 0)
                },
                years_1000: {
                    time: Date.UTC(2601, 0, 1, 0, 0, 0, 0),
                    winRTTime: getTicksFromDate(2601, 0, 1, 0, 0, 0, 0, 0)
                },
                years_5000: {
                    time: Date.UTC(6601, 0, 1, 0, 0, 0, 0),
                    winRTTime: getTicksFromDate(6601, 0, 1, 0, 0, 0, 0, 0)
                },
                years_10000: {
                    time: Date.UTC(11601, 0, 1, 0, 0, 0, 0),
                    winRTTime: getTicksFromDate(11601, 0, 1, 0, 0, 0, 0, 0)
                },
                years_25000: {
                    time: Date.UTC(26601, 0, 1, 0, 0, 0, 0),
                    winRTTime: getTicksFromDate(26601, 0, 1, 0, 0, 0, 0, 0)
                },
                max: {
                    time: Math.floor(testClass.getInt64Max() / ticksPerMillisecond) - jsEpochMilliseconds,
                    winRTTime: testClass.createInt64(2147483647, 4294961488)
                }
            };
            var time_negative = {
                millisecond: {
                    time: Date.UTC(1600, 11, 31, 23, 59, 59, 999),
                    winRTTime: -ticksPerMillisecond
                },
                second: {
                    time: Date.UTC(1600, 11, 31, 23, 59, 59, 0),
                    winRTTime: -ticksPerSecond
                },
                minute: {
                    time: Date.UTC(1600, 11, 31, 23, 59, 0, 0),
                    winRTTime: -ticksPerMinute
                },
                hour: {
                    time: Date.UTC(1600, 11, 31, 23, 0, 0, 0),
                    winRTTime: -ticksPerHour
                },
                day: {
                    time: Date.UTC(1600, 11, 31, 0, 0, 0, 0),
                    winRTTime: -ticksPerDay
                },
                week: {
                    time: Date.UTC(1600, 11, 25, 0, 0, 0, 0),
                    winRTTime: -(ticksPerDay * 7)
                },
                month: {
                    time: Date.UTC(1600, 11, 1, 0, 0, 0, 0),
                    winRTTime: getTicksFromDate(1600, 11, 1, 0, 0, 0, 0, 0)
                },
                year: {
                    time: Date.UTC(1600, 0, 1, 0, 0, 0, 0),
                    winRTTime: getTicksFromDate(1600, 0, 1, 0, 0, 0, 0, 0)
                },
                years_2: {
                    time: Date.UTC(1599, 0, 1, 0, 0, 0, 0),
                    winRTTime: getTicksFromDate(1599, 0, 1, 0, 0, 0, 0, 0)
                },
                years_5: {
                    time: Date.UTC(1596, 0, 1, 0, 0, 0, 0),
                    winRTTime: getTicksFromDate(1596, 0, 1, 0, 0, 0, 0, 0)
                },
                years_10: {
                    time: Date.UTC(1591, 0, 1, 0, 0, 0, 0),
                    winRTTime: getTicksFromDate(1591, 0, 1, 0, 0, 0, 0, 0)
                },
                years_50: {
                    time: Date.UTC(1551, 0, 1, 0, 0, 0, 0),
                    winRTTime: getTicksFromDate(1551, 0, 1, 0, 0, 0, 0, 0)
                },
                years_100: {
                    time: Date.UTC(1501, 0, 1, 0, 0, 0, 0),
                    winRTTime: getTicksFromDate(1501, 0, 1, 0, 0, 0, 0, 0)
                },
                years_500: {
                    time: Date.UTC(1101, 0, 1, 0, 0, 0, 0),
                    winRTTime: getTicksFromDate(1101, 0, 1, 0, 0, 0, 0, 0)
                },
                years_1000: {
                    time: Date.UTC(601, 0, 1, 0, 0, 0, 0),
                    winRTTime: getTicksFromDate(601, 0, 1, 0, 0, 0, 0, 0)
                },
                years_5000: {
                    time: Date.UTC(-4601, 0, 1, 0, 0, 0, 0),
                    winRTTime: getTicksFromDate(-4601, 0, 1, 0, 0, 0, 0, 0)
                },
                years_10000: {
                    time: Date.UTC(-9601, 0, 1, 0, 0, 0, 0),
                    winRTTime: getTicksFromDate(-9601, 0, 1, 0, 0, 0, 0, 0)
                },
                years_25000: {
                    time: Date.UTC(-24601, 0, 1, 0, 0, 0, 0),
                    winRTTime: getTicksFromDate(-24601, 0, 1, 0, 0, 0, 0, 0)
                },
                min: {
                    time: Math.ceil(testClass.getInt64Min() / ticksPerMillisecond) - jsEpochMilliseconds,
                    winRTTime: testClass.createInt64(2147483648, 5808)
                }
            };

            var time_range = {
                maxPlus1: time_positive.max.time + 1,
                minMinus1: time_negative.min.time - 1,
                year31000: Date.UTC(31000, 0),
                year28000BC: Date.UTC(-28000, 0)
            };

            for (var time in time_positive) {
                logger.comment("Verify correctness of Date projected as DateTime with a positive offset of: " + time);
                var date = new Date(time_positive[time].time);
                var succeeded = testClass.verifyDateTime(date, time_positive[time].winRTTime);
                verify(succeeded, true, "testClass.verifyDateTime(date, " + time_positive[time].winRTTime + ")");
            }
            for (var time in time_negative) {
                logger.comment("Verify correctness of Date projected as DateTime with a negative offset of: " + time);
                var date = new Date(time_negative[time].time);
                var succeeded = testClass.verifyDateTime(date, time_negative[time].winRTTime);
                verify(succeeded, true, "testClass.verifyDateTime(date, " + time_negative[time].winRTTime + ")");
            }
            for (var time in time_range) {
                logger.comment("Verify exception thrown when Date projected as DateTime with an out of range offset of: " + time);
                verify.exception(function () {
                    var date = new Date(time_range[time]);
                    return testClass.verifyDateTime(date, 0);
                }, RangeError, "Project DateTime with time: " + time);
            }
        }
    });

    runner.addTest({
        id: 6,
        desc: 'Equality of marshaled Date/DateTime objects',
        pri: '0',
        test: function () {
            // Verify DateTime instances differing only in the least significant 4 digits, will be projected as equal Date objects
            verify(testClass.produceDateTime(1), testClass.produceDateTime(9999), "projected DateTime(1) and DateTime(9999) are equivalent");
            verify(testClass.produceDateTime(300), testClass.produceDateTime(6734), "projected DateTime(300) and DateTime(6734) are equivalent");
            verify.notEqual(testClass.produceDateTime(9999), testClass.produceDateTime(10000), "projected DateTime(9999) and DateTime(10010) are not equivalent");

            // Verify equivalent Date instances marshal to equivalent DateTime instances
            verify(testClass.dateTimeCmp(new Date(-jsEpochMilliseconds), new Date(-jsEpochMilliseconds)), 0, "testClass.dateTimeCmp(new Date(-jsEpochMilliseconds), new Date(-jsEpochMilliseconds))");
            verify(testClass.dateTimeCmp(new Date(0), new Date(0)), 0, "testClass.dateTimeCmp(new Date(0), new Date(0))");
            verify(testClass.dateTimeCmp(new Date(Date.UTC(2061, 6, 15, 7, 42, 21, 544)), new Date(Date.UTC(2061, 6, 15, 7, 42, 21, 544))), 0, "testClass.dateTimeCmp(new Date(Date.UTC(2061, 6, 15, 7, 42, 21, 544)), new Date(Date.UTC(2061, 6, 15, 7, 42, 21, 544)))");
            verify(testClass.dateTimeCmp(new Date(0), new Date(-jsEpochMilliseconds)), jsEpochTicks, "testClass.dateTimeCmp(new Date(0), new Date(-jsEpochMilliseconds))");
        }
    });

    runner.addTest({
        id: 7,
        desc: 'Round trip specific tests',
        pri: '0',
        test: function () {
            testClass.dateTime = testClass.produceDateTime(1);
            var dateTime = testClass.dateTime;
            // Verify round trip of unmodified DateTime
            verify(testClass.cmpDateTimeToStored(dateTime), 0, "testClass.cmpDateTimeToStored(dateTime)");
            // Verify modified DateTime does not round trip after operation
            dateTime.setUTCSeconds(0);
            verify(dateTime.getUTCSeconds(), 0, "dateTime.getUTCMilliseconds()");
            verify(testClass.cmpDateTimeToStored(dateTime), 1, "testClass.cmpDateTimeToStored(dateTime)");

            testClass.dateTime = testClass.produceDateTime(-1);
            dateTime = testClass.dateTime;
            // Verify round trip of unmodified DateTime
            verify(testClass.cmpDateTimeToStored(dateTime), 0, "testClass.cmpDateTimeToStored(dateTime)");
            // Verify modified DateTime does not round trip after comparison
            dateTime.setUTCMinutes(0);
            verify(dateTime.getUTCMinutes(), 0, "dateTime.getUTCMilliseconds()");
            verify(testClass.cmpDateTimeToStored(dateTime), -1, "testClass.cmpDateTimeToStored(dateTime)");

            testClass.dateTime = testClass.produceDateTime(jsEpochTicks + 7342);
            dateTime = testClass.dateTime
            // Verify round trip of unmodified DateTime
            verify(testClass.cmpDateTimeToStored(dateTime), 0, "testClass.cmpDateTimeToStored(dateTime)");
            // Verify modified DateTime does round trip after comparison
            verify((dateTime > 0), false, "dateTime > 0");
            verify(testClass.cmpDateTimeToStored(dateTime), 0, "testClass.cmpDateTimeToStored(dateTime)");

            testClass.dateTime = testClass.produceDateTime(getTicksFromDate(2000, 0, 1, 0, 0, 0, 0, 16));
            dateTime = testClass.dateTime;
            // Verify round trip of unmodified DateTime
            verify(testClass.cmpDateTimeToStored(dateTime), 0, "testClass.cmpDateTimeToStored(dateTime)");
            // Verify modified DateTime does not round trip after comparison
            dateTime.setUTCMilliseconds(1000);
            verify(dateTime.getUTCMilliseconds(), 0, "dateTime.getUTCMilliseconds()");
            verify(testClass.cmpDateTimeToStored(dateTime), -(ticksPerSecond - 16), "testClass.cmpDateTimeToStored(dateTime)");

            // verify Date expandos are stripped when passed through WinRT
            var myDate = testClass.produceDateTime(0);
            var helpfulMsg = "Don't Panic.";
            myDate.note = helpfulMsg;
            verify(myDate.note, helpfulMsg, "myDate.note");
            var myDateReturned = testClass.roundTripDateTime(myDate);
            verify(myDateReturned, myDate, "testClass.roundTripDateTime(myDate)");
            verify.notDefined(myDateReturned.note, "myDateReturned.note");
        }
    });

    runner.addTest({
        id: 8,
        desc: 'Pass By Value',
        pri: '0',
        test: function () {
            var myDate = new Date(0);
            // Call method that modifies DateTime [in]
            var myDateReturned = testClass.resetDateTime(myDate);
            // Verify returned modified DateTime
            verify(myDateReturned.getTime(), -jsEpochMilliseconds, "myDateReturned.getTime()");
            // Verify original DateTime not affected
            verify(myDate.getTime(), 0, "myDate.getTime()");

            testClass.dateTime = new Date(0);
            verify(testClass.dateTime.getTime(), 0, "testClass.dateTime.getTime()");
            logger.comment("Attempt to call testClass.dateTime.setTime(-jsEpochMilliseconds)");
            testClass.dateTime.setTime(-jsEpochMilliseconds);
            verify(testClass.dateTime.getTime(), 0, "testClass.dateTime.getTime()");
        }
    });

    runner.addTest({
        id: 9,
        desc: 'Identical struct still marshals as struct',
        pri: '0',
        test: function () {
            verify(testClass.marshalDummyDateTime({ universalTime: 0 }).universalTime, 0, "testClass.marshalDummyDateTime({ universalTime: 0 }).universalTime");
            verify.exception(function () {
                return testClass.marshalDummyDateTime(new Date(0));
            }, TypeError, "Marshal Date as struct identical to DateTime");

            var dummyDateTime = testClass.marshalDummyDateTime({ universalTime: jsEpochTicks });
            verify.typeOf(dummyDateTime, "object");
            verify.instanceOf(dummyDateTime, Object);
            verify(dummyDateTime.universalTime, jsEpochTicks, "dummyDateTime.universalTime");
        }
    });

    // TimeSpan Tests
    runner.addTest({
        id: 10,
        desc: 'TimeSpan projected as Number',
        pri: '0',
        test: function () {
            // Verify Windows.Foundation.TimeSpan is not projected
            verify.notDefined(Windows.Foundation.TimeSpan, "Windows.Foundation.TimeSpan");

            var timeSpan0 = testClass.produceTimeSpan(0);
            // Verify projected TimeSpan has typeof object
            verify.typeOf(timeSpan0, "number");
            verify(timeSpan0, 0, "timeSpan0");
        }
    });

    runner.addTest({
        id: 11,
        desc: 'Simple marshaling of TimeSpan [in] and [out]',
        pri: '0',
        test: function () {
            // Verify Number as TimeSpan [in]
            var spanIn = jsEpochMilliseconds;
            var succeeded = testClass.verifyTimeSpan(spanIn, jsEpochTicks);
            verify(succeeded, true, "testsClass.verifyTimeSpan(spanIn, jsEpochTicks)");
            // Verify Number object round-tripped through WinRT
            verify(testClass.roundTripTimeSpan(spanIn), spanIn, "testClass.roundTripTimeSpan(spanIn)");

            // Verify TimeSpan [out] as Number
            var span = Date.UTC(2009, 9, 22, 8, 45, 17, 222) - Date.UTC(1975, 3, 4, 9, 30, 22, 543);
            var spanOut = testClass.produceTimeSpan(span * ticksPerMillisecond);
            verify(spanOut, span, "testClass.produceTimeSpan(" + span + " * ticksPerMillisecond)");
            // Verify TimeSpan round-tripped through JS (modified)
            spanOut = testClass.produceTimeSpan(span * ticksPerMillisecond);
            succeeded = testClass.verifyTimeSpan(spanOut, span * ticksPerMillisecond);
            verify(succeeded, true, "testClass.verifyTimeSpan(spanOut, span * ticksPerMillisecond);");
        }
    });

    runner.addTest({
        id: 12,
        desc: 'Marshal invalid types as TimeSpan',
        pri: '0',
        test: function () {
            var invalidTypes = {
                object: { duration: 0 },
                string: "Hello, World!",
                "function": function () { return 7; },
                "undefined": undefined,
                "NaN": NaN,
                multiElementArray: [30000, 2],
                object: { a: 20000 },
                arrayAsNumber: new Number([30000, 2]),
                objectAsNumber: new Number({ a: "foo" }),
                stringAsNumber: new Number("Hello, World!"),
            };

            for (var type in invalidTypes) {
                logger.comment("Try to marshal " + type + " as TimeSpan[in]");
                verify.exception(function () {
                    return testClass.roundTripTimeSpan(invalidTypes[type]);
                }, TypeError, "Marshal " + type);
            }
        }
    });

    runner.addTest({
        id: 13,
        desc: 'Marshal valid types as TimeSpan',
        pri: '0',
        test: function () {
            var types = {
                int: 242,
                double: 12345678.910,
                numericString: jsEpochMilliseconds.toString(),
                singleElementArray: [30000],
                intAsNumber: new Number(1234567),
                singleElementArrayAsNumber: new Number([30000]),
                numericStringAsNumber: new Number("123"),
                numberPrototype: Number.prototype,
                "null": null,
                enumValue0: Fabrikam.Kitchen.ChefRole.headChef,
                enumValue1: Fabrikam.Kitchen.ChefCapabilities.canDice
            };

            for (var type in types) {
                logger.comment("Try to marshal " + type + " as TimeSpan[in]");
                verify(testClass.verifyTimeSpan(types[type], types[type] * ticksPerMillisecond), true, "testClass.verifyTimeSpan(" + types[type] + ", " + (types[type] * ticksPerMillisecond) + ")");
            }
        }
    });

    runner.addTest({
        id: 14,
        desc: 'Verify correctness of TimeSpan conversion to Number',
        pri: '0',
        test: function () {
            var time_positive = {
                millisecond: ticksPerMillisecond,
                second: ticksPerSecond,
                minute: ticksPerMinute,
                hour: ticksPerHour,
                day: ticksPerDay,
                week: ticksPerDay * 7,
                month: getTicksFromDate(1601, 1, 1, 0, 0, 0, 0, 0),
                year: getTicksFromDate(1602, 0, 1, 0, 0, 0, 0, 0),
                years_2: getTicksFromDate(1603, 0, 1, 0, 0, 0, 0, 0),
                years_5: getTicksFromDate(1606, 0, 1, 0, 0, 0, 0, 0),
                years_10: getTicksFromDate(1611, 0, 1, 0, 0, 0, 0, 0),
                years_50: getTicksFromDate(1651, 0, 1, 0, 0, 0, 0, 0),
                years_100: getTicksFromDate(1701, 0, 1, 0, 0, 0, 0, 0),
                years_500: getTicksFromDate(2101, 0, 1, 0, 0, 0, 0, 0),
                years_1000: getTicksFromDate(2601, 0, 1, 0, 0, 0, 0, 0),
                years_5000: getTicksFromDate(6601, 0, 1, 0, 0, 0, 0, 0),
                years_10000: getTicksFromDate(11601, 0, 1, 0, 0, 0, 0, 0),
                years_25000: getTicksFromDate(26601, 0, 1, 0, 0, 0, 0, 0),
                max: testClass.getInt64Max()
            };
            var time_negative = {
                millisecond: -ticksPerMillisecond,
                second: -ticksPerSecond,
                minute: -ticksPerMinute,
                hour: -ticksPerHour,
                day: -ticksPerDay,
                week: -(ticksPerDay * 7),
                month: getTicksFromDate(1600, 11, 1, 0, 0, 0, 0, 0),
                year: getTicksFromDate(1600, 0, 1, 0, 0, 0, 0, 0),
                years_2: getTicksFromDate(1599, 0, 1, 0, 0, 0, 0, 0),
                years_5: getTicksFromDate(1596, 0, 1, 0, 0, 0, 0, 0),
                years_10: getTicksFromDate(1591, 0, 1, 0, 0, 0, 0, 0),
                years_50: getTicksFromDate(1551, 0, 1, 0, 0, 0, 0, 0),
                years_100: getTicksFromDate(1501, 0, 1, 0, 0, 0, 0, 0),
                years_500: getTicksFromDate(1101, 0, 1, 0, 0, 0, 0, 0),
                years_1000: getTicksFromDate(601, 0, 1, 0, 0, 0, 0, 0),
                years_5000: getTicksFromDate(-4601, 0, 1, 0, 0, 0, 0, 0),
                years_10000: getTicksFromDate(-9601, 0, 1, 0, 0, 0, 0, 0),
                years_25000: getTicksFromDate(-24601, 0, 1, 0, 0, 0, 0, 0),
                min: testClass.getInt64Min()
            };

            for (var time in time_positive) {
                logger.comment("Verify correctness of TimeSpan projected as Number with a positive offset of: " + time);
                var timeSpan = testClass.produceTimeSpan(time_positive[time]);
                verify(timeSpan, time_positive[time] / ticksPerMillisecond, "timeSpan");
            }
            for (var time in time_negative) {
                logger.comment("Verify correctness of TimeSpan projected as Number with a negative offset of: " + time);
                var timeSpan = testClass.produceTimeSpan(time_negative[time]);
                verify(timeSpan, time_negative[time] / ticksPerMillisecond, "timeSpan");
            }
        }
    });

    runner.addTest({
        id: 15,
        desc: 'Verify correctness of Number conversion to TimeSpan',
        pri: '0',
        test: function () {
            var max = Math.floor(testClass.getInt64Max() / ticksPerMillisecond);
            var min = Math.ceil(testClass.getInt64Min() / ticksPerMillisecond);
            var time_positive = {
                millisecond: Math.floor(getTicksFromDate(1601, 0, 1, 0, 0, 0, 1) / ticksPerMillisecond),
                second: Math.floor(getTicksFromDate(1601, 0, 1, 0, 0, 1, 0) / ticksPerMillisecond),
                minute: Math.floor(getTicksFromDate(1601, 0, 1, 0, 1, 0, 0) / ticksPerMillisecond),
                hour: Math.floor(getTicksFromDate(1601, 0, 1, 1, 0, 0, 0) / ticksPerMillisecond),
                day: Math.floor(getTicksFromDate(1601, 0, 2, 0, 0, 0, 0) / ticksPerMillisecond),
                week: Math.floor(getTicksFromDate(1601, 0, 8, 0, 0, 0, 0) / ticksPerMillisecond),
                month: Math.floor(getTicksFromDate(1601, 1, 1, 0, 0, 0, 0) / ticksPerMillisecond),
                year: Math.floor(getTicksFromDate(1602, 0, 1, 0, 0, 0, 0) / ticksPerMillisecond),
                years_2: Math.floor(getTicksFromDate(1603, 0, 1, 0, 0, 0, 0) / ticksPerMillisecond),
                years_5: Math.floor(getTicksFromDate(1606, 0, 1, 0, 0, 0, 0) / ticksPerMillisecond),
                years_10: Math.floor(getTicksFromDate(1611, 0, 1, 0, 0, 0, 0) / ticksPerMillisecond),
                years_50: Math.floor(getTicksFromDate(1651, 0, 1, 0, 0, 0, 0) / ticksPerMillisecond),
                years_100: Math.floor(getTicksFromDate(1701, 0, 1, 0, 0, 0, 0) / ticksPerMillisecond),
                years_500: Math.floor(getTicksFromDate(2101, 0, 1, 0, 0, 0, 0) / ticksPerMillisecond),
                years_1000: Math.floor(getTicksFromDate(2601, 0, 1, 0, 0, 0, 0) / ticksPerMillisecond),
                years_5000: Math.floor(getTicksFromDate(6601, 0, 1, 0, 0, 0, 0) / ticksPerMillisecond),
                years_10000: Math.floor(getTicksFromDate(11601, 0, 1, 0, 0, 0, 0) / ticksPerMillisecond),
                years_25000: Math.floor(getTicksFromDate(26601, 0, 1, 0, 0, 0, 0) / ticksPerMillisecond),
            };
            var time_negative = {
                millisecond: Math.ceil(getTicksFromDate(1600, 11, 31, 23, 59, 59, 999) / ticksPerMillisecond),
                second: Math.ceil(getTicksFromDate(1600, 11, 31, 23, 59, 59, 0) / ticksPerMillisecond),
                minute: Math.ceil(getTicksFromDate(1600, 11, 31, 23, 59, 0, 0) / ticksPerMillisecond),
                hour: Math.ceil(getTicksFromDate(1600, 11, 31, 23, 0, 0, 0) / ticksPerMillisecond),
                day: Math.ceil(getTicksFromDate(1600, 11, 31, 0, 0, 0, 0) / ticksPerMillisecond),
                week: Math.ceil(getTicksFromDate(1600, 11, 25, 0, 0, 0, 0) / ticksPerMillisecond),
                month: Math.ceil(getTicksFromDate(1600, 11, 1, 0, 0, 0, 0) / ticksPerMillisecond),
                year: Math.ceil(getTicksFromDate(1600, 0, 1, 0, 0, 0, 0) / ticksPerMillisecond),
                years_2: Math.ceil(getTicksFromDate(1599, 0, 1, 0, 0, 0, 0) / ticksPerMillisecond),
                years_5: Math.ceil(getTicksFromDate(1596, 0, 1, 0, 0, 0, 0) / ticksPerMillisecond),
                years_10: Math.ceil(getTicksFromDate(1591, 0, 1, 0, 0, 0, 0) / ticksPerMillisecond),
                years_50: Math.ceil(getTicksFromDate(1551, 0, 1, 0, 0, 0, 0) / ticksPerMillisecond),
                years_100: Math.ceil(getTicksFromDate(1501, 0, 1, 0, 0, 0, 0) / ticksPerMillisecond),
                years_500: Math.ceil(getTicksFromDate(1101, 0, 1, 0, 0, 0, 0) / ticksPerMillisecond),
                years_1000: Math.ceil(getTicksFromDate(601, 0, 1, 0, 0, 0, 0) / ticksPerMillisecond),
                years_5000: Math.ceil(getTicksFromDate(-4601, 0, 1, 0, 0, 0, 0) / ticksPerMillisecond),
                years_10000: Math.ceil(getTicksFromDate(-9601, 0, 1, 0, 0, 0, 0) / ticksPerMillisecond),
                years_25000: Math.ceil(getTicksFromDate(-24601, 0, 1, 0, 0, 0, 0) / ticksPerMillisecond),
            };

            var time_range = {
                maxPlus1: max + 1,
                minMinus1: min - 1,
                year31000: Math.floor(getTicksFromDate(31000, 0, 1, 0, 0, 0, 0) / ticksPerMillisecond),
                year28000BC: Math.ceil(getTicksFromDate(-28000, 0, 1, 0, 0, 0, 0) / ticksPerMillisecond),
                fullSpan: max-min,
            };

            for (var time in time_positive) {
                logger.comment("Verify correctness of Number projected as TimeSpan with a positive offset of: " + time);
                var span = time_positive[time];
                var succeeded = testClass.verifyTimeSpan(span, time_positive[time] * ticksPerMillisecond);
                verify(succeeded, true, "testClass.verifyTimeSpan(date, " + (time_positive[time] * ticksPerMillisecond) + ")");
            }
            for (var time in time_negative) {
                logger.comment("Verify correctness of Number projected as TimeSpan with a negative offset of: " + time);
                var span = time_negative[time];
                var succeeded = testClass.verifyTimeSpan(span, time_negative[time] * ticksPerMillisecond);
                verify(succeeded, true, "testClass.verifyTimeSpan(date, " + (time_negative[time] * ticksPerMillisecond) + ")");
            }
            for (var time in time_range) {
                logger.comment("Verify exception thrown when Number projected as TimeSpan with an out of range offset of: " + time);
                verify.exception(function () {
                    var span = time_range[time];
                    return testClass.verifyTimeSpan(span, 0);
                }, RangeError, "Project TimeSpan with time: " + time);
            }
        }
    });

    runner.addTest({
        id: 16,
        desc: 'Equality of marshaled TimeSpan objects',
        pri: '0',
        test: function () {
            // Verify TimeSpan instances differing only in the least significant 4 digits, will be projected as equal Number objects
            verify.notEqual(testClass.produceTimeSpan(1), testClass.produceTimeSpan(9999), "projected TimeSpan(1) and TimeSpan(9999) are not equivalent");
            verify.notEqual(testClass.produceTimeSpan(300), testClass.produceTimeSpan(6734), "projected TimeSpan(300) and TimeSpan(6734) are not equivalent");
            verify.notEqual(testClass.produceTimeSpan(9999), testClass.produceTimeSpan(10000), "projected TimeSpan(9999) and TimeSpan(10000) are not equivalent");

            // Verify equivalent Date instances marshal to equivalent TimeSpan instances
            verify(testClass.timeSpanCmp(0, 0), 0, "testClass.timeSpanCmp(0, 0)");
            verify(testClass.timeSpanCmp(Date.UTC(2061, 6, 15, 7, 42, 21, 544), Date.UTC(2061, 6, 15, 7, 42, 21, 544)), 0, "testClass.timeSpanCmp(Date.UTC(2061, 6, 15, 7, 42, 21, 544), Date.UTC(2061, 6, 15, 7, 42, 21, 544))");
            verify(testClass.timeSpanCmp(0, -jsEpochMilliseconds), jsEpochTicks, "testClass.timeSpanCmp(0, -jsEpochMilliseconds)");
        }
    });

    runner.addTest({
        id: 17,
        desc: 'Pass By Value - TimeSpan',
        pri: '0',
        test: function () {
            var mySpan = ticksPerDay * 365;
            // Call method that modifies TimeSpan [in]
            var mySpanReturned = testClass.resetTimeSpan(mySpan);
            // Verify returned modified TimeSpan
            verify(mySpanReturned, 0, "mySpanReturned");
            // Verify original TimeSpan not affected
            verify(mySpan, ticksPerDay * 365, "mySpan");
        }
    });

    runner.addTest({
        id: 18,
        desc: 'Identical struct still marshals as struct - TimeSpan',
        pri: '0',
        test: function () {
            verify(testClass.marshalDummyTimeSpan({ duration: ticksPerHour }).duration, ticksPerHour, "testClass.marshalDummyTimeSpan({ duration: ticksPerHour }).duration");
            verify.exception(function () {
                return testClass.marshalDummyTimeSpan(ticksPerMinute);
            }, TypeError, "Marshal Date as struct identical to TimeSpan");

            var dummyTimeSpan = testClass.marshalDummyTimeSpan({ duration: ticksPerMinute });
            verify.typeOf(dummyTimeSpan, "object");
            verify.instanceOf(dummyTimeSpan, Object);
            verify(dummyTimeSpan.duration, ticksPerMinute, "dummyTimeSpan.universalTime");
        }
    });

    runner.addTest({
        id: 19,
        desc: 'Struct with DateTime and TimeSpan fields',
        pri: '0',
        test: function () {
            var myEvent = { eventName: "Birthday Party", startTime: new Date(Date.UTC(1999, 5, 23, 18)), duration: (ticksPerHour * 4.5) };
            var myEventReturned = testClass.marshalEventLog(myEvent);

            verify.typeOf(myEventReturned, "object");
            verify.instanceOf(myEventReturned, Object);
            for (var field in myEvent) {
                verify.defined(myEventReturned[field], "field");
                verify(myEventReturned[field], myEvent[field], "myEventReturned." + field);
            }
        }
    });

    runner.addTest({
        id: 19,
        desc: 'FastSig Tests - +Date-String',
        pri: '0',
        test: function () {
            var newYears = "Happy New Year!";
            var notNewYears = "Not New Year's";

            var jan1UTC = Date.UTC(2012,0,1,0,0,0,0);
            var jan1Ticks = (jan1UTC * ticksPerMillisecond) + jsEpochTicks;

            var may20UTC = Date.UTC(2011,4,20,0,0,0,0);
            var may20Ticks = (may20UTC * ticksPerMillisecond) + jsEpochTicks;

            var year31000 = Date.UTC(31000, 0);
            var year28000BC = Date.UTC(-28000, 0);

            logger.comment("Verify function produces correct result with un-modified WinRT Date");
            var winRTDate = testClass.produceDateTime(jan1Ticks);
            verify(testClass.dateInStringOut(winRTDate), newYears, "testClass.dateInStringOut(winRTDate)");
            winRTDate = testClass.produceDateTime(may20Ticks);
            verify(testClass.dateInStringOut(winRTDate), notNewYears, "testClass.dateInStringOut(winRTDate)");

            logger.comment("Verify function produces correct result with modified WinRT Date");
            winRTDate = testClass.produceDateTime(jan1Ticks);
            winRTDate.setUTCDate(2);
            verify(testClass.dateInStringOut(winRTDate), notNewYears, "testClass.dateInStringOut(winRTDate)");
            winRTDate = testClass.produceDateTime(may20Ticks);
            winRTDate.setUTCFullYear(1956,0,1);
            verify(testClass.dateInStringOut(winRTDate), newYears, "testClass.dateInStringOut(winRTDate)");

            logger.comment("Verify function produces correct result with Javascript Date");
            var jsDate = new Date(jan1UTC);
            verify(testClass.dateInStringOut(jsDate), newYears, "testClass.dateInStringOut(jsDate)");
            jsDate = new Date(may20UTC);
            verify(testClass.dateInStringOut(jsDate), notNewYears, "testClass.dateInStringOut(jsDate)");

            logger.comment("Verify RangeError thrown when Javascript Date out of range of DateTime");
            jsDate = new Date(year31000);
            verify.exception(function () {
                return testClass.dateInStringOut(jsDate);
            }, RangeError, "Pass in Date with value greater than max DateTime");

            jsDate = new Date(year28000BC);
            verify.exception(function () {
                return testClass.dateInStringOut(jsDate);
            }, RangeError, "Pass in Date with value less than min DateTime");

            logger.comment("Verify RangeError thrown when modified WinRT Date out of range of DateTime");
            winRTDate = testClass.produceDateTime(jan1Ticks);
            winRTDate.setUTCFullYear(31000);
            verify.exception(function () {
                return testClass.dateInStringOut(winRTDate);
            }, RangeError, "Pass in WinRT Date with value greater than max DateTime");

            winRTDate = testClass.produceDateTime(jan1Ticks);
            winRTDate.setUTCFullYear(-28000);
            verify.exception(function () {
                return testClass.dateInStringOut(winRTDate);
            }, RangeError, "Pass in WinRT Date with value less than min DateTime");

            logger.comment("Verify TypeError thrown when non-Date type passed as DateTime");
            verify.exception(function () {
                return testClass.dateInStringOut(null);
            }, TypeError, "Marshal null as DateTime");
            verify.exception(function () {
                return testClass.dateInStringOut(Date.prototype);
            }, TypeError, "Marshal Date.prototype as DateTime");
        }
    });

    Loader42_FileName = "WinRT DateTime and TimeSpan Marshaling tests";
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
