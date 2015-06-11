WScript.RegisterCrossThreadInterfacePS();
//
// Test typeof using "typeofdata.js"
//
// Test these flags
//  -forceNative
//  -forceNative -off:fgpeeps
//

var PASS = "PASS";
var FAIL = "FAIL";;
var s_failCount = 0;
var s_totalCount = 0;

function log(label, o, expected, actual) {
    // Convert actual from boolean to PASS/FAIL
    if (actual !== PASS && actual !== FAIL) {
        actual = (actual ? PASS : FAIL);
    }

    s_totalCount++;

    if (expected != actual) {
        s_failCount++;
        WScript.Echo("Fail " + label + ":", o, "Expected:", expected, "Actual:", actual);
    }
}

// Br
function if_undefined (o, e) { if (typeof o == "undefined") { log("if_undefined ", o, e, PASS) } else { log("if_undefined ", o, e, FAIL) } }
function if_boolean   (o, e) { if (typeof o == "boolean"  ) { log("if_boolean   ", o, e, PASS) } else { log("if_boolean   ", o, e, FAIL) } }
function if_number    (o, e) { if (typeof o == "number"   ) { log("if_number    ", o, e, PASS) } else { log("if_number    ", o, e, FAIL) } }
function if_string    (o, e) { if (typeof o == "string"   ) { log("if_string    ", o, e, PASS) } else { log("if_string    ", o, e, FAIL) } }
function if_function  (o, e) { if (typeof o == "function" ) { log("if_function  ", o, e, PASS) } else { log("if_function  ", o, e, FAIL) } }
function if_object    (o, e) { if (typeof o == "object"   ) { log("if_object    ", o, e, PASS) } else { log("if_object    ", o, e, FAIL) } }
function ifn_undefined(o, e) { if (typeof o != "undefined") { log("ifn_undefined", o, e, PASS) } else { log("ifn_undefined", o, e, FAIL) } }
function ifn_boolean  (o, e) { if (typeof o != "boolean"  ) { log("ifn_boolean  ", o, e, PASS) } else { log("ifn_boolean  ", o, e, FAIL) } }
function ifn_number   (o, e) { if (typeof o != "number"   ) { log("ifn_number   ", o, e, PASS) } else { log("ifn_number   ", o, e, FAIL) } }
function ifn_string   (o, e) { if (typeof o != "string"   ) { log("ifn_string   ", o, e, PASS) } else { log("ifn_string   ", o, e, FAIL) } }
function ifn_function (o, e) { if (typeof o != "function" ) { log("ifn_function ", o, e, PASS) } else { log("ifn_function ", o, e, FAIL) } }
function ifn_object   (o, e) { if (typeof o != "object"   ) { log("ifn_object   ", o, e, PASS) } else { log("ifn_object   ", o, e, FAIL) } }

function ifs_undefined (o, e) { if (typeof o === "undefined") { log("ifs_undefined ", o, e, PASS) } else { log("ifs_undefined ", o, e, FAIL) } }
function ifs_boolean   (o, e) { if (typeof o === "boolean"  ) { log("ifs_boolean   ", o, e, PASS) } else { log("ifs_boolean   ", o, e, FAIL) } }
function ifs_number    (o, e) { if (typeof o === "number"   ) { log("ifs_number    ", o, e, PASS) } else { log("ifs_number    ", o, e, FAIL) } }
function ifs_string    (o, e) { if (typeof o === "string"   ) { log("ifs_string    ", o, e, PASS) } else { log("ifs_string    ", o, e, FAIL) } }
function ifs_function  (o, e) { if (typeof o === "function" ) { log("ifs_function  ", o, e, PASS) } else { log("ifs_function  ", o, e, FAIL) } }
function ifs_object    (o, e) { if (typeof o === "object"   ) { log("ifs_object    ", o, e, PASS) } else { log("ifs_object    ", o, e, FAIL) } }
function ifsn_undefined(o, e) { if (typeof o !== "undefined") { log("ifsn_undefined", o, e, PASS) } else { log("ifsn_undefined", o, e, FAIL) } }
function ifsn_boolean  (o, e) { if (typeof o !== "boolean"  ) { log("ifsn_boolean  ", o, e, PASS) } else { log("ifsn_boolean  ", o, e, FAIL) } }
function ifsn_number   (o, e) { if (typeof o !== "number"   ) { log("ifsn_number   ", o, e, PASS) } else { log("ifsn_number   ", o, e, FAIL) } }
function ifsn_string   (o, e) { if (typeof o !== "string"   ) { log("ifsn_string   ", o, e, PASS) } else { log("ifsn_string   ", o, e, FAIL) } }
function ifsn_function (o, e) { if (typeof o !== "function" ) { log("ifsn_function ", o, e, PASS) } else { log("ifsn_function ", o, e, FAIL) } }
function ifsn_object   (o, e) { if (typeof o !== "object"   ) { log("ifsn_object   ", o, e, PASS) } else { log("ifsn_object   ", o, e, FAIL) } }

// Cm
function is_undefined (o, e) { var a = (typeof o == "undefined"); log("is_undefined ", o, e, a); }
function is_boolean   (o, e) { var a = (typeof o == "boolean"  ); log("is_boolean   ", o, e, a); }
function is_number    (o, e) { var a = (typeof o == "number"   ); log("is_number    ", o, e, a); }
function is_string    (o, e) { var a = (typeof o == "string"   ); log("is_string    ", o, e, a); }
function is_function  (o, e) { var a = (typeof o == "function" ); log("is_function  ", o, e, a); }
function is_object    (o, e) { var a = (typeof o == "object"   ); log("is_object    ", o, e, a); }
function not_undefined(o, e) { var a = (typeof o != "undefined"); log("not_undefined", o, e, a); }
function not_boolean  (o, e) { var a = (typeof o != "boolean"  ); log("not_boolean  ", o, e, a); }
function not_number   (o, e) { var a = (typeof o != "number"   ); log("not_number   ", o, e, a); }
function not_string   (o, e) { var a = (typeof o != "string"   ); log("not_string   ", o, e, a); }
function not_function (o, e) { var a = (typeof o != "function" ); log("not_function ", o, e, a); }
function not_object   (o, e) { var a = (typeof o != "object"   ); log("not_object   ", o, e, a); }

function iss_undefined (o, e) { var a = (typeof o === "undefined"); log("iss_undefined ", o, e, a); }
function iss_boolean   (o, e) { var a = (typeof o === "boolean"  ); log("iss_boolean   ", o, e, a); }
function iss_number    (o, e) { var a = (typeof o === "number"   ); log("iss_number    ", o, e, a); }
function iss_string    (o, e) { var a = (typeof o === "string"   ); log("iss_string    ", o, e, a); }
function iss_function  (o, e) { var a = (typeof o === "function" ); log("iss_function  ", o, e, a); }
function iss_object    (o, e) { var a = (typeof o === "object"   ); log("iss_object    ", o, e, a); }
function nots_undefined(o, e) { var a = (typeof o !== "undefined"); log("nots_undefined", o, e, a); }
function nots_boolean  (o, e) { var a = (typeof o !== "boolean"  ); log("nots_boolean  ", o, e, a); }
function nots_number   (o, e) { var a = (typeof o !== "number"   ); log("nots_number   ", o, e, a); }
function nots_string   (o, e) { var a = (typeof o !== "string"   ); log("nots_string   ", o, e, a); }
function nots_function (o, e) { var a = (typeof o !== "function" ); log("nots_function ", o, e, a); }
function nots_object   (o, e) { var a = (typeof o !== "object"   ); log("nots_object   ", o, e, a); }

var is_test = [
    [
        if_undefined,
        if_boolean  ,
        if_number   ,
        if_string   ,
        if_function ,
        if_object   ,
    ],

    [
        ifs_undefined,
        ifs_boolean,
        ifs_number,
        ifs_string,
        ifs_function,
        ifs_object,
    ],

    [
        is_undefined,
        is_boolean,
        is_number,
        is_string,
        is_function,
        is_object,
    ],

    [
        iss_undefined,
        iss_boolean,
        iss_number,
        iss_string,
        iss_function,
        iss_object,
    ],
];

var not_test = [
    [
        ifn_undefined,
        ifn_boolean,
        ifn_number,
        ifn_string,
        ifn_function,
        ifn_object,
    ],

    [
        ifsn_undefined,
        ifsn_boolean,
        ifsn_number,
        ifsn_string,
        ifsn_function,
        ifsn_object,
    ],

    [
        not_undefined,
        not_boolean,
        not_number,
        not_string,
        not_function,
        not_object,
    ],

    [
        nots_undefined,
        nots_boolean,
        nots_number,
        nots_string,
        nots_function,
        nots_object,
    ],
];

var TESTS = 6;
var TOTAL_TESTS = TESTS * 8; // hard coded to verify all tests run

function test(data) {
    for (var i = 0; i < data.length; i++) {
        var o = data[i];

        for (var t = 0; t < TESTS; t++) {
            // same index, or null ~ "object"
            var match = (i == t || (i == 6 && t == 5));

            // is_... test should only PASS matching one
            is_test.forEach(function (tests) {
                tests[t](o, match ? PASS : FAIL);
            });

            // not_... test should only FAIL matching one
            not_test.forEach(function (tests) {
                tests[t](o, match ? FAIL : PASS);
            });
        }
    }
}

// Test local engine
WScript.LoadScriptFile("typeofdata.js");
test(this.g_data);

// Test other engine (HostDispatch)
var eng = WScript.LoadScriptFile("typeofdata.js", "crossthread");
test(eng.g_data);

if (s_failCount == 0
    && s_totalCount == TOTAL_TESTS * this.g_data.length * 2) {
    WScript.Echo("pass");
}
