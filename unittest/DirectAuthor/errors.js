var t01 = (case / 3);                         // ERRsyntax
var t02 = a. 31122;                           // ERRnoIdent
var t03 = { a: 12, get > () { return 3; } };  // ERRnoMemberIdent
var t04 = { a: 12, > : 32 };                  // ERRnoMemberIdent
var t05 = { a 12 };                           // ERRnoColon
var t06 = { get count(a) { return 3; } };     // ERRnoRparen
var t07 = function (23) { };                  // ERRnoIdent
var t08 = function (a { };                    // ERRnoRparen
var t09 = +23 = 4;                            // ERRsyntax
var  10 = 10;                                 // ERRnoIdent

try * { } catch(a) { };                       // ERRnoLcurly
try { } finally * { };                        // ERRnoLcurly
try { } catch( ) { };                         // ERRnoIdent
try { } catch(a) * { };                       // ERRnoLcurly
for (var a, b in c) { };                      // ERRsyntax
for (var i = 0; i < 10) { }                   // ERRnoSemic
switch (a) { default: 2; default: 3; }        // ERRdupDefault
label1: { continue label1; }                  // ERRbadContinue
label2: while(a > 0) { continue lable1; }     // ERRnoLabel
{ continue; }                                 // ERRbadContinue
{ break; }                                    // ERRbadBreak
{ return; }                                   // ERRbadReturn
{ a b }                                       // ERRnoSemic

function testStrictMode() {
  "use strict";

  function eval() {}                          // ERREvalUsage
  function arguments() { }                    // ERRArgsUsage
  var eval = 1;                               // ERREvalUsage
  var arguments = 1;                          // ERRArgsUsage
  var a = 01;                                 // ERRES5NoOctal
  var a = 01000000000000000000;               // ERRES5NoOctal
  var s = "\01";                              // ERRES5NoOctal
  var a = { get "\01"() {} };                 // ERRES5NoOctal
  var a = { get 01() {} };                    // ERRES5NoOctal
  var a = { get 01000000000000000000() {} };  // ERRES5NoOctal
  var a = { "\01": 1 };                       // ERRES5NoOctal
  var a = { 01: 1 };                          // ERRES5NoOctal
  var a = { 01000000000000000000: 1 };        // ERRES5NoOctal
  function bar(a, a) { }                      // ERRES5ArgSame
  delete a;                                   // ERRInvalidDelete
  try {} catch(eval) { };                     // ERREvalUsage
  try {} catch(arguments) { };                // ERRArgsUsage
  with (a) { };                               // ERRES5NoWith
  function bar() { "\01"; }                   // ERRES5NoOctal
}