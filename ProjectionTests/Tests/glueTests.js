glueTests = {
    invalid: function() {
        runner.globalSetup(function () {
            logger.comment("Expecting all invalid");
        });

        var testCase = new TestCase();
        testCase.id = "";
        testCase.desc = "invalid id";
        testCase.test = function () {

            var x = 1;
            verify(x, 1, "Good! 1");
        }
        testCase.AddTest();

        var testCase = new TestCase();
        testCase.id = "1";
        testCase.desc = "";
        testCase.test = function () {

            var x = 1;
            verify(x, 1, "Good! 1");
        }
        testCase.AddTest();
        Loader42_FileName = "Invalid tests";
    },
    standard: function () {
        runner.globalSetup(function () {
            logger.comment("Expecting 18 pass, 6 fail, 24 total");
            logger.comment("setup");
        });

        runner.globalTeardown(function () {
            logger.comment("teardown");
        });

        var testCase = new TestCase();
        testCase.id = "1";
        testCase.desc = "normal";
        testCase.test = function () {

            var x = 1;
            verify(x, 1, "Good! 1");
        }
        testCase.AddTest();

        runner.addTest({
            id:2,
            desc: "addTest",
            pri:0,
            test: function () {
                verify(1, 1, "Good");
            }
        });

        runner.addTest({
            id:0,
            desc: "addTest (should not appear first)",
            pri:0,
            test: function () {
                verify(1, 1, "Good");
            }
        });

        runner.addTest({
            id:3,
            desc: "verify (expected fail)",
            pri:0,
            test: function () {
                verify(1, 2, "Fail");
            }
        });

        runner.addTest({
            id:4,
            desc: "verify",
            pri:0,
            test: function () {
                verify(1, 1, "Pass");
            }
        });

        runner.addTest({
            id:5,
            desc: "verify.equal (expected fail)",
            pri:0,
            test: function () {
                verify.equal(1, 2, "Fail");
            }
        });

        runner.addTest({
            id:6,
            desc: "verify.equal",
            pri:0,
            test: function () {
                verify.equal(1, 1, "Pass");
            }
        });

        runner.addTest({
            id:7,
            desc: "verify.notEqual (expected fail)",
            pri:0,
            test: function () {
                verify.notEqual(1, 1, "Fail");
            }
        });

        runner.addTest({
            id:8,
            desc: "verify.notEqual",
            pri:0,
            test: function () {
                verify.notEqual(1, 2, "Pass");
            }
        });

        runner.addTest({
            id:9,
            desc: "verify.noException (expected fail)",
            pri:0,
            test: function () {
                verify.noException(function() {
                    throw 1;
                }, "No Exception")
            }
        });

        runner.addTest({
            id:10,
            desc: "verify.noException",
            pri:0,
            test: function () {
                verify.noException(function() {
                    verify.equal(1, 1, "Pass");
                }, "No Exception")
            }
        });

        runner.addTest({
            id:11,
            desc: "verify.exception",
            pri:0,
            test: function () {
                verify.exception(function() {
                    throw new Error("foo");
                }, Error, "should get Exception");
            }
        });

        runner.addTest({
            id:12,
            desc: "verify.exception (expected fail)",
            pri:0,
            test: function () {
                verify.exception(function() {
                    verify.equal(1, 1, "Pass");
                }, Error, "should not get exception");
            }
        });
        runner.globalSetup(function () {
            logger.comment("setup2");
        });

        runner.globalTeardown(function () {
            logger.comment("teardown2");
        });


        runner.addTest({
            id:14,
            desc: "normal",
            test: function () {
                var x = 1;
                verify(x, 1, "Good");
            }
        });

        runner.addTest({
            id:15,
            desc: "basic async",
            test: function () {
                var x = 1;

                function callback() {
                    verify(x, 1, "Good");
                    runner.start(waitHandle);
                }

                window.setTimeout(callback, 100);
                var waitHandle = runner.wait();
            }
        });

        runner.addTest({
            id:16,
            desc: "timeout (expected failure)",
            test: function () {
                var x = 1;

                function callback() {
                    verify(x, 1, "good");
                    runner.start(waitHandle);
                }

                window.setTimeout(callback, 1000);

                var waitHandle = runner.wait(100);
            }
        });

        runner.addTest({
            id:17,
            desc: "normal after timeout",
            test: function () {
                var x = 1;
                verify(x, 1, "Good");
            }
        });

        runner.addTest({
            id:18,
            desc: "customize waiting time",
            test: function () {
                var x = 1;

                function callback() {
                    verify(x, 1, "Good");
                    runner.start(waitHandle);
                }

                setTimeout(callback, 2000);

                var waitHandle = runner.wait(3000);
            }
        });

        runner.addTest({
            id:19,
            desc:"multiple",
            test: function () {
                var x = 1;

                function callback() {
                    verify(x, 1, "good");
                    runner.start(waitHandle);
                }

                setTimeout(callback, 1000);
                setTimeout(callback, 1750);

                var waitHandle = runner.wait(2000, 2);
            }
        });

        runner.addTest({
            id:20,
            desc:"start before wait",
            test: function () {
                var x = 1;

                function callback() {
                    verify(x, 1, "good");
                    runner.start(waitHandle);
                }

                callback();

                var waitHandle = runner.wait();
            }
        });

        runner.addTest({
            id:21,
            desc: "normal after start before wait",
            test: function () {
                var x = 1;
                verify(x, 1, "Good");
            }
        });

        runner.addTest({
            id:22,
            desc:"start before and after wait",
            test: function () {
                var x = 1;

                function callback() {
                    verify(x, 1, "good");
                    runner.start(waitHandle);
                }

                callback();
                setTimeout(callback, 1000);
                var waitHandle = runner.wait(2000, 2);
            }
        });

        runner.addTest({
            id:23,
            desc: "normal after start before and after wait",
            test: function () {
                var x = 1;
                verify(x, 1, "Good");
            }
        });

        runner.addTest({
            id:24,
            desc: "bug",
            test: function () {
                runner.publish("bug", "12345", "FooBaz", "db");
                runner.publish("bug", "12345");
            }
        });
 
        Loader42_FileName = "glueTests.js";
    },
    noFilename: function() {
        runner.globalSetup(function () {
            logger.comment("Expecting 1 pass, 0 fail, 1 total");
        });

        var testCase = new TestCase();
        testCase.id = "1";
        testCase.desc = "normal";
        testCase.test = function () {

            var x = 1;
            verify(x, 1, "Good! 1");
        }
        testCase.AddTest();
    },
    setupError: function() {
        runner.globalSetup(function () {
            logger.comment("Expecting 0 pass, 0 fail, 0 total (no tests should run)");
            throw 1;
        });

        var testCase = new TestCase();
        testCase.id = "1";
        testCase.desc = "normal";
        testCase.test = function () {

            var x = 1;
            verify(x, 1, "Good! 1");
        }
        testCase.AddTest();
        Loader42_FileName = "setup error";
    },
    teardownError: function() {
        runner.globalSetup(function () {
            logger.comment("Expecting 1 pass, 0 fail, 1 total ");
        });

        runner.globalTeardown(function() {
            logger.comment("throwing error");
            throw 1;
        });

        var testCase = new TestCase();
        testCase.id = "1";
        testCase.desc = "normal";
        testCase.test = function () {

            var x = 1;
            verify(x, 1, "Good! 1");
        }
        testCase.AddTest();
        Loader42_FileName = "teardown error";
    },
    crossContext: function(){
        runner.globalSetup(function () {
            logger.comment("Expecting 2 pass, 0 fail, 2 total ");
        });    
       runner.addTest({
            id:1,
            desc: "cross context",
            test: function () {
                var cct = new CrossContextTest();
                cct.addChildFunction(init);
                cct.addChildFunction(init1);
                cct.childContent = "<div id='dd'></div>"
                  cct.test(function (child) {
                      child.init();
                      verify(child.a, 1, "child object");
                      child.init1();
                      verify(child.a, "aaa", "child object changed");
                      verify(child.dd.x, "prop", "child DOM expando property");
                  });

                function init(){
                    a=1;
                }

                function init1(){
                    a="aaa";
                    dd.x="prop";
                }
            }
        });
        
        runner.addTest({
            id:2,
            desc: "cross context with async",
            test: function () {
                var cct = new CrossContextTest();
                cct.addChildFunction(init);
                
                cct.test(function (child) {
                      child.init();
                      window.child = child;
                      setTimeout(function(){
                          verify(child.a, 123, "child object");
                          runner.start(waitHandle);}, 1000);
                      var waitHandle = runner.wait();
                  });

                function init(){
                    setTimeout(function (){
                         a=123;
                    }, 500);
                }    
            }
        });      
        Loader42_FileName = "cross context";    
    }
};

if(typeof WScript !== "undefined") {
    glueTests["standard"]();
    runner.run();
}
