// MSFT:18460583 - Exprgen:CAS:x64::DEBUG: ASSERT:  src->size <= SparseArraySegmentBase::INLINE_CHUNK_SIZE (chakra!Js::JavascriptArray::InitBoxedInlineSegments<Js::JavascriptNativeFloatArray>+103 [e:\a\_work\1\s\core\lib\runtime\library\javascriptarray.cpp @ 11844]) ASSERTI
// jshost -maxinterpretcount:1 -maxsimplejitruncount:1 45722_assert.js

function test0(){
    // Declare array of length 64, which is max inline head segment size
    var IntArr2 = [ 4616200933313889280,    -6512602339745534976,    -1036380016,    106,    -779830617033324416,    -637099590,    -1,    -1073741824,    -2063927323,    -134,    -1,    -1189756698,    65536,    -9124456825816517632,    -223,    -1010515036,    235,    -3,    -568685003,    -143,    -225,    8013349922097741824,    83,    -35,    -2147483648,    -3,    -2147483648,    -8662592711394399232,    1032798871,    485902825,    -319015415,    -803696419,    847400283,    -155636348,    74,    13,    65535,    -2147483649,    75,    -2147483646,    -5578323661635373056,    239,    180,    -1,    1073741823,    1330087704,    829051075,    -7269893513790476288,    -2147483649,    -2139781870,    148,    74454422,    1073741823,    1755358766,    -946904310,    217,    -199,    -1350291655,    8231004361096077312,    7422509546457244672,    -109,    690088459,    1980828667,    1  ];

    // Control value of x to force bailout below
    var x = shouldBailout ? "" : 0;

    // BailOutPrimitiveButString when shouldBailout. This would cause the assert that created this
    // bug because the head segment's size would become greater than INLINE_CHUNK_SIZE.
    var y = IntArr2[0] - x;

    // This function and if-block are never executed but seem to be needed to hit assert, perhaps
    // to restore the right args while boxing stack variables during bailout
    var func4 = function(){  };
    if(func4()) {
      print('inside if');
      IntArr2[0]++;
    }

    return y;
};

var shouldBailout = false;
// generate profile
test0();
// Run Simple JIT
test0();
// run code with bailouts enabled
shouldBailout = true;
test0();

print("PASSED");
