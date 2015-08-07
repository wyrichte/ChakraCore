if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in
  // jc/jshost
  this.WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js");
}

var tests = [
  {
    name: "RegExp.test() - matches for the beginning of string, otherwise terminates if sticky = true",
    body: function () {
        var str = "abcababc";
        var re = /abc/y;
        assert.isTrue(re.test(str), "Sticky = true, RegExp.test() result");
        assert.isTrue(re.lastIndex == 3, "Sticky = true, lastIndex result on RegExp.test()");
        assert.isFalse(re.test(str), "Sticky = true, RegExp.test() result");
        assert.isTrue(re.lastIndex == 0, "Sticky = true, lastIndex result on RegExp.test()");
        assert.isTrue(re.test(str), "Sticky = true, RegExp.test() result");
        assert.isTrue(re.lastIndex == 3, "Sticky = true, lastIndex result on RegExp.test()");
    }
  },
  {
    name: "RegExp.exec() - matches for the beginning of string, otherwise terminates if sticky = true",
    body: function () {
        var str = "abcababc";
        var re = /abc/y;
        assert.isTrue(re.exec(str) == "abc", "Sticky = true, RegExp.exec() result");
        assert.isTrue(re.lastIndex == 3, "Sticky = true, lastIndex result on RegExp.exec()");
        assert.isTrue(re.exec(str) == null, "Sticky = true, RegExp.exec() result");
        assert.isTrue(re.lastIndex == 0, "Sticky = true, lastIndex result on RegExp.exec()");
    }
  },
  {
    name: "RegExp.match() - matches for the beginning of string, otherwise terminates if sticky = true",
    body: function () {
        var str = "abcababc";
        var re = /abc/y;
        assert.isTrue(str.match(re) == "abc", "Sticky = true, RegExp.match() result");
        assert.isTrue(re.lastIndex == 3, "Sticky = true, lastIndex result on RegExp.match()");
        assert.isTrue(str.match(re) == null, "Sticky = true, RegExp.match() result");
        assert.isTrue(re.lastIndex == 0, "Sticky = true, lastIndex result on RegExp.match()");
    }
  },
  {
    name: "RegExp.match() - matches for the beginning of string, otherwise terminates if sticky = true with lastindex set",
    body: function () {
        var str = "abcabcababc";
        var re = /abc/y;
        re.lastIndex = 3;
        assert.isTrue(str.match(re) == "abc", "Sticky = true, RegExp.match() result");
        assert.isTrue(re.lastIndex == 6, "Sticky = true, lastIndex result on RegExp.match()");
        assert.isTrue(str.match(re) == null, "Sticky = true, RegExp.match() result");
        assert.isTrue(re.lastIndex == 0, "Sticky = true, lastIndex result on RegExp.match()");
    }
  },  
  {
    name: "RegExp.search() - matches for the beginning of string, otherwise terminates if sticky = true",
    body: function () {
        var str = "ababcabc";
        var re = /abc/y;
        assert.isTrue(str.search(re) == -1, "Sticky = true, RegExp.search() result");
        assert.isTrue(re.lastIndex == 0, "Sticky = true, lastIndex result on RegExp.search()");
        assert.isTrue(str.search(re) == -1, "Sticky = true, RegExp.search() result");
        assert.isTrue(re.lastIndex == 0, "Sticky = true, lastIndex result on RegExp.search()");
    }
  },
  {
    name: "RegExp.replace() - matches for the beginning of string, otherwise terminates if sticky = true",
    body: function () {
        var str = "abcabcababc";
        var re = /abc/y;
        assert.isTrue(str.replace(re, "1") == "1abcababc", "Sticky = true, RegExp.replace() result");
        assert.isTrue(re.lastIndex == 3, "Sticky = true, lastIndex result on RegExp.replace()");
        assert.isTrue(str.replace(re, "1") == "abc1ababc", "Sticky = true, RegExp.replace() result");
        assert.isTrue(re.lastIndex == 6, "Sticky = true, lastIndex result on RegExp.replace()");
        assert.isTrue(str.replace(re, "1") == "abcabcababc", "Sticky = true, RegExp.replace() result");
        assert.isTrue(re.lastIndex == 0, "Sticky = true, lastIndex result on RegExp.replace()");
    }
  },
  {
    name: "RegExp.replace() - matches for the beginning of string, otherwise terminates if sticky = true, lastIndex set",
    body: function () {
        var str = "abcabcababc";
        var re = /abc/y;
        re.lastIndex = 4;
        assert.isTrue(str.replace(re, "1") == "abcabcababc", "Sticky = true, RegExp.replace() result");
        assert.isTrue(re.lastIndex == 0, "Sticky = true, lastIndex result on RegExp.replace()");
    }
  },  
  {
    name: "RegExp.replace() - matches for the beginning of string, otherwise terminates if sticky = true, global = true",
    body: function () {
        var str = "abcabcababc";
        var re = /abc/gy;
        assert.isTrue(str.replace(re, "1") == "11ababc", "Sticky = true, RegExp.replace() result");
        assert.isTrue(re.lastIndex == 0, "Sticky = true, lastIndex result on RegExp.replace()");
        assert.isTrue(str.replace(re, "1") == "11ababc", "Sticky = true, RegExp.replace() result");
        assert.isTrue(re.lastIndex == 0, "Sticky = true, lastIndex result on RegExp.replace()");
    }
  },  
  {
    name: "RegExp.replace() - matches for the beginning of string, otherwise terminates if global = true",
    body: function () {
        var str = "abcabcababc";
        var re = /abc/g;
        assert.isTrue(str.replace(re, "1") == "11ab1", "Sticky = true, RegExp.replace() result");
        assert.isTrue(re.lastIndex == 0, "Sticky = true, lastIndex result on RegExp.replace()");
        assert.isTrue(str.replace(re, "1") == "11ab1", "Sticky = true, RegExp.replace() result");
        assert.isTrue(re.lastIndex == 0, "Sticky = true, lastIndex result on RegExp.replace()");
    }
  },   
  {
    name: "RegExp.split() - ignores sticky flag",
    body: function () {
        var str = "a,ab,ba,b,";
        var re = /a,/y;
        var result = str.split(re, 5);
        assert.isTrue(result.length == 2, "Sticky = true, RegExp.split() result's length");
        assert.isTrue(re.lastIndex == 0, "Sticky = true, lastIndex result on RegExp.split()");
        assert.isTrue(result == ",ab,ba,b,", "Sticky = true, RegExp.split() result");
        
    }
  },  
  
];

testRunner.runTests(tests);