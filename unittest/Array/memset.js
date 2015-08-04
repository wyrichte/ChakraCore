// Compares the value set by interpreter with the jitted code
// need to run with -mic:1 -off:simplejit -off:JITLoopBody -off:inline
// Run locally with -trace:memop -trace:bailout to help find bugs

var testCases = [
  function() {
    return {
      start: 0,
      end: 100,
      test: function testBasic(a) {
        for(var i = 0; i < 100; i++)
        {
          a[i] = 0;
        }
      }
    };
  },
  function() {
    return {
      start: 5,
      end: 101,
      test: function testReverse(a) {
        for(var i = 100; i >= 5; i--)
        {
          a[i] = 0;
        }
      }
    };
  },
  function(results) {
    return {
      runner: function testMultipleMemset(arrayGen) {
        var a = arrayGen(), b = arrayGen(), c = arrayGen();
        for(var i = 0; i < 10; i++)
        {
          a[i] = b[i] = c[i] = 0;
        }
        results.concat([a, b, c]);
      },
      check: function() {
        for(var i = 0; i < results.length; ) {
          compare_result(results[i], results[i + 2]);
          ++i;
          compare_result(results[i], results[i + 2]);
          ++i;
          compare_result(results[i], results[i + 2]);
          ++i;
        }
      }
    };
  },
  function() {
    return {
      start: 4,
      end: 30,
      test: function testUnroll(a) {
        for(var i = 4; i < 30; )
        {
          a[i] = 0;
          i++;
          a[i] = 0;
          i++;
        }
      }
    };
  },
  function() {
    return {
      start: 8,
      end: 10,
      test: function testMissingValues(a) {
        for(var i = 8; i < 10; i++)
        {
          a[i] = 0;
        }
      }
    };
  },
  function() {
    return {
      start: 0,
      end: 6,
      test: function testOverwrite(a) {
        a[5] = 3;
        for(var i = 0; i < 6; i++)
        {
          a[i] = 0;
        }
      }
    };
  },
  function() {
    return {
      start: 10,
      end: 50,
      test: function testNegativeConstant(a) {
        for(var i = 10; i < 50; i++)
        {
          a[i] = -1;
        }
      }
    };
  },
  function() {
    return {
      start: -50,
      end: 10,
      test: function testNegativeStartIndex(a) {
        for(var i = -50; i < 10; i++)
        {
          a[i] = -3;
        }
      }
    };
  }
];

var arrayGenerators = [
  function() {return new Array(10); }, // the one for the interpreter
  function() {return new Array(10); },
  function() {return []; }
  // causes bailouts right now: BailOut: function: testMultipleMemset ( (#1.2), #3) offset: #0036 Opcode: BailOnNotArray Kind: BailOutOnNotNativeArray
  // function() {return [1, 2, 3, 4, 5, 6, 7]; }
];

for(var testCase of testCases) {
  var results = [];
  var testInfo = testCase(results);
  for(var gen of arrayGenerators) {
    if(testInfo.runner) {
      testInfo.runner(gen);
    } else {
      var newArray = gen();
      testInfo.test(newArray);
      results.push(newArray);
    }
  }

  if(testInfo.check) {
    testInfo.check(results);
  } else {
    var base = results[0]; // result from the interpreter
    for(let i = 1; i < results.length; ++i) {
      compare_result(base, results[i], testInfo.start, testInfo.end);
    }
  }
}
var passed = true;
function compare_result(a, b, start, end) {
  for(var i = start; i < end; i++) {
    if(a[i] !== b[i])
    {
      WScript.Echo(i + " " + a[i] + " " + b[i]);
      passed = false;
      return false;
    }
  }
  return true;
}

if(passed)
{
  WScript.Echo("PASSED");
}
else
{
  WScript.Echo("FAILED");
}

