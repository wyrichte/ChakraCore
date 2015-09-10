// Compares the value set by interpreter with the jitted code
// need to run with -mic:1 -off:simplejit -off:JITLoopBody -off:inline
// Run locally with -trace:memop -trace:bailout to help find bugs

let testCases = [
  function() {
    return {
      start: 0,
      end: 100,
      test: function testBasic(a, src) {
        for(let i = 0; i < 100; i++) {
          a[i] = src[i];
        }
      }
    };
  },
  function() {
    let start = 5, end = 101;
    return {
      start: start,
      end: end,
      size: end - start,
      test: function testReverse(a, src) {
        // Currently this is not a valid memcopy pattern
        for(let i = end - 1; i >= start; i--) {
          a[i] = src[i];
        }
      }
    };
  },
  function() {
    let results = [];
    let start = 0, end = 10;
    return {
      start: start,
      end: end,
      runner: function testMultipleMemcopy(arrayGen, src) {
        let a = arrayGen(), b = arrayGen(), c = arrayGen();
        // Currently this is not a valid memcopy pattern
        for(let i = 0; i < 10; i++) {
          a[i] = b[i] = c[i] = src[i];
        }
        results.push([a, b, c]);
      },
      check: function() {
        let base = results[0];
        for(let i = 1; i < results.length; ++i) {
          for(let j = 0; j < 3; ++j) {
            compareResult(base[j], results[i][j], start, end);
          }
        }
      }
    };
  },
  function() {
    return {
      start: 0,
      end: 10,
      test: function preIncr(a, src) {
        let ri = -1;
        for(let i = 0; i < 10; ++i) {
          a[++ri] = src[i];
        }
      }
    };
  },
  function() {
    let int = true;
    return {
      size: 60,
      loop: 10, // Run this a few times to cause rejit
      test: function testNegativeStartIndex(a, src) {
        let start = int ? 0 : -50;
        let end = start + 60;
        for(let i = start, j = 0; i < end; i++, j++) {
          // This should bailout on MemOp because the index will be negative
          a[i] = src[j];
        }
        int = false;
      },
      check: function(results) {
        let base = results[0]; // result from the interpreter
        for(let i = 1; i < results.length; ++i) {
          compareResult(base, results[i], 0, 60, -50);
        }
      }
    };
  },
  function() {
    return {
      start: 0,
      end: 128,
      test: function bug4468518(a, src) {
        let x = 0;
        for(let i = 0; i < 128; i++) {
          let m = src[i];
          x += m;
          a[i] = m;
        }
        return x;
      }
    };
  }
];

function makeArray(size) {
  let a = new Array(size || 10);
  a.fill(1);
  return a;
}

let arrayGenerators = [
  makeArray,
  // causes bailouts right now
  // function() {return []; }
];

function makeSource(size) {
  let s = new Array(size);
  for(let i = 0; i < size; ++i) {
    s[i] = i;
  }
  return s;
}

for(let testCase of testCases) {
  let results = [];
  let testInfo = testCase();

  let src;
  if (testInfo.size !== undefined) {
    src = makeSource(testInfo.size);
  } else if(
    testInfo.start !== undefined &&
    testInfo.end !== undefined
  ) {
    src = makeSource(testInfo.end - testInfo.start);
  }

  function run(gen) {
    if(testInfo.runner) {
      let result = testInfo.runner(gen, src);
      results.push(result);
    } else {
      let newArray = gen(testInfo.size);
      newArray.fill(1);
      testInfo.test(newArray, src);
      results.push(newArray);
    }
  }

  // Run once for the interpreter
  run(makeArray);
  for(let gen of arrayGenerators) {
    if(testInfo.loop | 0) {
      for(let i = 0; i < testInfo.loop; ++i) {
        run(gen);
      }
    } else {
      run(gen);
    }
  }

  if(testInfo.check) {
    testInfo.check(results);
  } else {
    let base = results[0]; // result from the interpreter
    for(let i = 1; i < results.length; ++i) {
      compareResult(base, results[i], testInfo.start, testInfo.end);
    }
  }
}
let passed = true;
function compareResult(a, b, start, end, start2) {
  for(let i = start, j = start2 || start; i < end; ++i, ++j) {
    if(a[i] !== b[j]) {
      print(`a[${i}](${a[i]}) !== b[${j}](${b[j]})`);
      passed = false;
      return false;
    }
  }
  return true;
}

if(passed) {
  print("PASSED");
} else {
  print("FAILED");
}

