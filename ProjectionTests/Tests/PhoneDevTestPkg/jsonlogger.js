/// <disable>JS2085,JS2038,JS2015,JS2064,JS2003,JS2043,JS2055,JS2087</disable>
function JSONLogger() {
    function unicodeEscape(str) {
        var code, pref = {1: '\\x0', 2: '\\x', 3: '\\u0', 4: '\\u'};
        return str.replace(/\W/g, function(c) {
            if(c.charCodeAt(0)>0 && c.charCodeAt(0)<127) return c;
            return pref[(code = c.charCodeAt(0).toString(16)).length] + code;
        });
    }
    function printable(obj){    
      return unicodeEscape(""+obj);
    }
    var obj = {};
    var currentTest = null;
    function Test(test) {
        var that = this;
        this.id = test.id;
        this.desc = printable(test.desc);
        this.preReq = printable(test.preReq && test.preReq.toString());
        this.test = '{test function}';//test.test.toString();
        this.verifications = [];

        this.verify = function(passed, act, exp, msg) {
            that.verifications.push({
                type: 'verification',
                passed: printable(passed),
                act: printable(act),
                exp: printable(exp),
                msg: printable(msg)
            });
        }

        this.result = function(state, args) {
            that.verifications.push({
                type: 'result',
                result: state,
                other: args
            });
        }

        this.comment = function(str) {
            that.verifications.push({
                type: 'comment',
                comment: printable(str)
            });
        }

        this.bug = function(number, db, msg) {
            that.verifications.push({
                type: 'bug',
                number: number,
                db: db,
                msg: msg
            });
        }
        return this;
    }
    function dumpJson() {
        runner.__json_logger_obj = obj;
        try {
            runner.__json_logger_str = JSON.stringify(obj, null, 2);
        }
        catch (e) {
            runner.__json_logger_str = "Unexpected Exception happened during JSON.stringify: " + e.toString();
        }
    }

    this.start = function(filename, priority) {
        obj.filename = filename;
        obj.priority = priority;
        obj.startTime = new Date();
        obj.tests = [];
    };

    this.testStart = function(test) {
        currentTest = new Test(test);
    }

    this.verify = function(test, passed, act, exp, msg) {
        currentTest.verify(passed, act, exp, msg);
    };

    this.pass = function(test) {
        currentTest.result("pass");
    };

    this.fail = function(test) {
        currentTest.result("fail");
    };

    this.error = function(test, error) {
        if(currentTest)
            currentTest.result('error', error);
        else
            obj.tests.push({"error":error});
    };

    this.testEnd = function(test) {
        obj.tests.push(currentTest);
        currentTest = null;
    };

    this.end = function(filename, priority) {
        if(currentTest) {
            currentTest.result('error', 'test not finished');
            obj.tests.push(currentTest);
        }
        obj.endTime = new Date();

        dumpJson();
        
    };

    this.comment = function(str) {
        if(currentTest)
            currentTest.comment(str);
        else
            obj.tests.push({comment:str});
    };

    this.bug = function(number, msg, db) {
        db = db || "Windows 8 Bugs";
        currentTest.bug(number, db, msg);
    };
}
runner.subscribe(new JSONLogger());
