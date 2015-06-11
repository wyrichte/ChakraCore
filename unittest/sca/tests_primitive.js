//
// SCA tests for primitive
//
var tests_primitive = [
    null,
    undefined,
    true,
    false,

    // integers
    -1073741825,
    -1073741824,
    -1073741823,
    -234,
    -1,
    0,
    1,
    2,
    123,
    1073741822,
    1073741823,
    1073741824,

    // double
    123.456,
    -123.456,
    Number.POSITIVE_INFINITY,
    Number.NEGATIVE_INFINITY,
    Number.NaN,

    // int64/uint64
    { get: function(){ return SCA.makeInt64(0, 7); } },
    { get: function(){ return SCA.makeInt64(1, -1); } },
    { get: function(){ return SCA.makeInt64(-1, 0x8FFF0107); } },
    { get: function(){ return SCA.makeUint64(0, 7); } },
    { get: function(){ return SCA.makeUint64(1, -1); } },
    { get: function(){ return SCA.makeUint64(-1, 0x8FFF0107); } },

    // string
    "",
    "abc",
    "AbCd",
    "ABcdEFg\0Hij",
];
