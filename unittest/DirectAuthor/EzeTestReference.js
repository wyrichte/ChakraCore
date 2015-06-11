function TestCase() {
    this.useGlobalThis = true;

    this.id = 1;
    this.desc = "";
    this.reqs = [];
    this.test = function () { };
    this.AddTest = function () { 
        _$trace("in AddTest\n");
    this.test() };

    function assert () { };
    function verify () { };
}
TestCase.verify = function () { };
TestCase.useGlobalBaseline = true;

var BaselineTestCase = TestCase;

var IE7STANDARDSMODE = 7.0;
var IE8STANDARDSMODE = 8.0;
var IE9STANDARDSMODE = 9.0;
function getHOSTMode() { return IE9STANDARDSMODE };

function assert() { };
function verify() { };
function fail() { };

function Exception() { };
function getLocalizedError() { };

function SegmentArray() { };
function SegmentArrayWithPrototype() { };
function DelAllPrototype() { };
function reverse() { };
function shift() { };
function slice() { };

function CrossContextTest() {
    this.testWindow = true;

    this.addChildFunction = function (func) { func(); };
    this.addChildProperty = function (name, value) { Object.defineProperty(window, name, { value: value, writable: true, enumerable: true, configurable: true }) };

    this.test = function (func) { func(window); };
}

var log = { comment: function () { } };

var callback = { call: function () { } };

var X64MODE = 64;
var X64MODE = 86;
function getIEArchitecture() { return X86MODE }

var WScript = { Echo: function () { } };

function apLogFailInfo() { };