// This test case contains duplicated test262 tests that were broken by the
// regression mentioned in this bug. They have been slightly modified. We can
// remove this when test262 becomes a standard part of SNAP.

/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
//Copyright 2009 the Sputnik authors.  All rights reserved.
//This code is governed by the BSD license found in the LICENSE file.
/**
 * @path ch07/7.8/7.8.4/7.8.4-1-s.js
 * @description A directive preceeding an 'use strict' directive may not contain an OctalEscapeSequence
 * @onlyStrict
 */
function testcase()
{
  try
  {
    eval('"asterisk: \\052" /* octal escape sequences forbidden in strict mode*/ ; "use strict";');
    return false;
  }
  catch (e) {
    return (e instanceof SyntaxError);
  }
}
function testcase_functionscope()
{
  try
  {
    eval('function(){"asterisk: \\052" /* octal escape sequences forbidden in strict mode*/ ; "use strict";}();');
    return false;
  }
  catch (e) {
    return (e instanceof SyntaxError);
  }
}
WScript.Echo(testcase());
WScript.Echo(testcase_functionscope());
/**
 * NonEscapeSequence is not EscapeCharacter
 *
 * @path ch07/7.8/7.8.4/S7.8.4_A4.3_T2.js
 * @description EscapeCharacter :: DecimalDigits :: 7
 * @onlyStrict
 * @negative
 */
function testcase2()
{
  try
  {
    eval('"use strict";"\\7"');
    return false;
  }
  catch (e) {
    return (e instanceof SyntaxError);
  }
}
function testcase2_functionscope()
{
  try
  {
    eval('function(){"use strict";"\\7"}();');
    return false;
  }
  catch (e) {
    return (e instanceof SyntaxError);
  }
}
WScript.Echo(testcase2());
WScript.Echo(testcase2_functionscope());

/**
 * NonEscapeSequence is not EscapeCharacter
 *
 * @path ch07/7.8/7.8.4/S7.8.4_A4.3_T1.js
 * @description EscapeCharacter :: DecimalDigits :: 1
 * @onlyStrict
 * @negative
 */
function testcase3()
{
    try
    {
      eval('"use strict";"\\1"');
      return false;
    }
    catch (e) {
      return (e instanceof SyntaxError);
    }
}
function testcase3_functionscope()
{
    try
    {
      eval('function(){"use strict";"\\1"}();');
      return false;
    }
    catch (e) {
      return (e instanceof SyntaxError);
    }
}
WScript.Echo(testcase3());
WScript.Echo(testcase3_functionscope());

/**
 * @path ch14/14.1/14.1-14-s.js
 * @description semicolon insertion may come before 'use strict' directive
 * @noStrict
 */
function testcase4() {

  function foo()
  {
    "another directive"
    "use strict" ;
    return (this === undefined);
  }

  return foo.call(undefined);
}
WScript.Echo(testcase4());

/**
 * @path ch14/14.1/14.1-4-s.js
 * @description 'use strict' directive - not recognized if contains Line Continuation
 * @noStrict
 */
function testcase5() {

  function foo()
  {
    'use str\
ict';
     return (this !== undefined);
  }

  return foo.call(undefined);
}
WScript.Echo(testcase5());

/**
 * @path ch14/14.1/14.1-5-s.js
 * @description 'use strict' directive - not recognized if contains a EscapeSequence
 * @noStrict
 */
function testcase6() {

  function foo()
  {
    'use\u0020strict';
     return(this !== undefined);
  }

  return foo.call(undefined);
}
WScript.Echo(testcase5());

/**
 * @path ch14/14.1/14.1-8-s.js
 * @description 'use strict' directive - may follow other directives
 * @noStrict
 */
function testcase7() {

  function foo()
  {
     "bogus directive";
     "use strict";
     return (this === undefined);
  }

  return foo.call(undefined);
}
WScript.Echo(testcase7());

/**
 * @path ch14/14.1/14.1-16-s.js
 * @description 'use strict' directive - not recognized if it follow an empty statement
 * @noStrict
 */
function testcase8() {

  function foo()
  {
    ; 'use strict';
     return (this !== undefined);
  }

  return foo.call(undefined);
}
WScript.Echo(testcase8());

// Bug 379082 - duplicate
function b() {
    L: "use strict"; // no error
    with ({ }) { }
}

eval("L: \"use strict\"; with ({ }) { }"); // no error
