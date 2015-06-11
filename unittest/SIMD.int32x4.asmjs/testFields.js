function asmModule(stdlib, imports) {
    "use asm";
    var i4 = stdlib.SIMD.int32x4;
    var i4splat = i4.splat;
    var i4fromFloat64x2 = i4.fromFloat64x2;
    var i4fromFloat64x2Bits = i4.fromFloat64x2Bits;
    var i4fromFloat32x4 = i4.fromFloat32x4;
    var i4fromFloat32x4Bits = i4.fromFloat32x4Bits;
    //var i4abs = i4.abs;
    var i4neg = i4.neg;
    var i4add = i4.add;
    var i4sub = i4.sub;
    var i4mul = i4.mul;
    //var i4swizzle = i4.swizzle;
    //var i4shuffle = i4.shuffle;
    var i4withX = i4.withX;
    var i4withY = i4.withY;
    var i4withZ = i4.withZ;
    var i4withW = i4.withW;
    var i4lessThan = i4.lessThan;
    var i4equal = i4.equal;
    var i4greaterThan = i4.greaterThan;
    var i4select = i4.select;
    var i4and = i4.and;
    var i4or = i4.or;
    var i4xor = i4.xor;
    var i4not = i4.not;
    //var i4shiftLeftByScalar = i4.shiftLeftByScalar;
    //var i4shiftRightByScalar = i4.shiftRightByScalar;
    //var i4shiftRightArithmeticByScalar = i4.shiftRightArithmeticByScalar;

    var f4 = stdlib.SIMD.float32x4;  
    var f4splat = f4.splat;
    var f4fromFloat64x2 = f4.fromFloat64x2;
    var f4fromFloat64x2Bits = f4.fromFloat64x2Bits;
    var f4fromInt32x4 = f4.fromInt32x4;
    var f4fromInt32x4Bits = f4.fromInt32x4Bits;
    var f4abs = f4.abs;
    var f4neg = f4.neg;
    var f4add = f4.add;
    var f4sub = f4.sub;
    var f4mul = f4.mul;
    var f4div = f4.div;
    var f4clamp = f4.clamp;
    var f4min = f4.min;
    var f4max = f4.max;
    var f4reciprocal = f4.reciprocal;
    var f4reciprocalSqrt = f4.reciprocalSqrt;
    var f4sqrt = f4.sqrt;
    //var f4swizzle = f4.swizzle;
    //var f4shuffle = f4.shuffle;
    var f4withX = f4.withX;
    var f4withY = f4.withY;
    var f4withZ = f4.withZ;
    var f4withW = f4.withW;
    var f4lessThan = f4.lessThan;
    var f4lessThanOrEqual = f4.lessThanOrEqual;
    var f4equal = f4.equal;
    var f4notEqual = f4.notEqual;
    var f4greaterThan = f4.greaterThan;
    var f4greaterThanOrEqual = f4.greaterThanOrEqual;

    var f4select = f4.select;
    var f4and = f4.and;
    var f4or = f4.or;
    var f4xor = f4.xor;
    var f4not = f4.not;

    var d2 = stdlib.SIMD.float64x2;  
    var d2splat = d2.splat;
    var d2fromFloat32x4 = d2.fromFloat32x4;
    var d2fromFloat32x4Bits = d2.fromFloat32x4Bits;
    var d2fromInt32x4 = d2.fromInt32x4;
    var d2fromInt32x4Bits = d2.fromInt32x4Bits;
    var d2abs = d2.abs;
    var d2neg = d2.neg;
    var d2add = d2.add;
    var d2sub = d2.sub;
    var d2mul = d2.mul;
    var d2div = d2.div;
    var d2clamp = d2.clamp;
    var d2min = d2.min;
    var d2max = d2.max;
    var d2reciprocal = d2.reciprocal;
    var d2reciprocalSqrt = d2.reciprocalSqrt;
    var d2sqrt = d2.sqrt;
    //var d2swizzle = d2.swizzle;
    //var d2shuffle = d2.shuffle;
    var d2withX = d2.withX;
    var d2withY = d2.withY;
    var d2lessThan = d2.lessThan;
    var d2lessThanOrEqual = d2.lessThanOrEqual;
    var d2equal = d2.equal;
    var d2notEqual = d2.notEqual;
    var d2greaterThan = d2.greaterThan;
    var d2greaterThanOrEqual = d2.greaterThanOrEqual;
    var d2select = d2.select;

    var fround = stdlib.Math.fround;

    var globImportF4 = f4(imports.g1);       // global var import
    var globImportI4 = i4(imports.g2);       // global var import
    var globImportD2 = d2(imports.g3);       // global var import

    var f4g1 = f4(-5033.2,-3401.0,665.34,32234.1);          // global var initialized
    var f4g2 = f4(1194580.33,-11201.5,63236.93,334.8);          // global var initialized

    var i4g1 = i4(1065353216, -1073741824, -1077936128, 1082130432);          // global var initialized
    var i4g2 = i4(353216, -492529, -1128, 1085);          // global var initialized

    var d2g1 = d2(0.12344,-1.6578);          // global var initialized
    var d2g2 = d2(5455.4395,-100324.688);          // global var initialized

    var gval = 1234;
    var gval2 = 1234.0;


    
    var loopCOUNT = 3;

    function func1(a)
    {
        a = a|0;
        var b = i4(5033,-3401,665,-32234);
        var c = i4(-34183, 212344, -569437, 65534);
        var d = i4(0, 0, 0, 0);


        var loopIndex = 0;
        while ( (loopIndex|0) < (loopCOUNT|0)) {

            b = i4(b.w, -b.z, b.y, -b.x);
            

            loopIndex = (loopIndex + 1) | 0;
        }

        return i4(b);
    }
    
    function func2(a)
    {
        a = a|0;
        var b = i4(5033,-3401,665,-32234);
        var c = i4(-34183, 212344, -569437, 65534);
        var d = i4(0, 0, 0, 0);
        var loopIndex = 0;

        for (loopIndex = 0; (loopIndex | 0) < (loopCOUNT | 0) ; loopIndex = (loopIndex + 1) | 0)
        {

            b = i4(i4g1.w, i4g1.w+i4g2.z, i4g1.y - i4g2.x, i4g2.x);
                

        }

        return i4(b);
    }

    function func3(a)
    {
        a = a|0;
        var b = i4(5033,-3401,665,-32234);
        var c = i4(-34183, 212344, -569437, 65534);
        var d = i4(0, 0, 0, 0);
        var loopIndex = 0;


        loopIndex = loopCOUNT | 0;
        do {

            globImportI4 = i4(globImportI4.x * globImportI4.y, 1 + globImportI4.y, i4g2.z / i4g2.w, i4g2.w);
    

            loopIndex = (loopIndex - 1) | 0;
        }
        while ( (loopIndex | 0) > 0);

        return i4(globImportI4);
    }
    
    function func4(a)
    {
        a = a|0;
        var b = i4(5033,-3401,665,-32234);
        var c = i4(-34183, 212344, -569437, 65534);
        var d = i4(0, 0, 0, 0);



        var loopIndex = 0;
        while ( (loopIndex|0) < (loopCOUNT|0)) {

            b = i4(b.w, b.z >> 2, b.y == b.x, b.x);
            a = b.signMask;
            

            loopIndex = (loopIndex + 1) | 0;
        }

        return a | 0;
    }
    
    function func5(a)
    {
        a = a|0;
        var b = i4(5033,-3401,665,-32234);
        var c = i4(-34183, 212344, -569437, 65534);
        var d = i4(0, 0, 0, 0);
        var loopIndex = 0;

        for (loopIndex = 0; (loopIndex | 0) < (loopCOUNT | 0) ; loopIndex = (loopIndex + 1) | 0)
        {

            b = i4(i4g1.w * 2, i4g2.z << 4, -i4g1.y, i4g2.x);
            a = b.signMask;

        }

        return a | 0;
    }

    function func6(a)
    {
        a = a|0;
        var b = i4(5033,-3401,665,-32234);
        var c = i4(-34183, 212344, -569437, 65534);
        var d = i4(0, 0, 0, 0);

        var loopIndex = 0;

        loopIndex = loopCOUNT | 0;
        do {

            globImportI4 = i4(-globImportI4.x, globImportI4.y, -i4g2.z, i4g2.w);
            a = globImportI4.signMask;

            loopIndex = (loopIndex - 1) | 0;
        }
        while ( (loopIndex | 0) > 0);

        return a | 0;
    }
    
    
    
    return {func1:func1, func2:func2, func3:func3, func4:func4, func5:func5, func6:func6};
}

var m = asmModule(this, {g1:SIMD.float32x4(90934.2,123.9,419.39,449.0), g2:SIMD.int32x4(-1065353216, -1073741824,-1077936128, -1082130432), g3:SIMD.float64x2(110.20, 58967.0, 14511.670, 191766.23431)});

var ret;
WScript.Echo("Func1");
ret = m.func1();
WScript.Echo(typeof(ret));
WScript.Echo(ret.toString());

WScript.Echo("Func2");
ret = m.func2();
WScript.Echo(typeof(ret));
WScript.Echo(ret.toString());

WScript.Echo("Func3");
ret = m.func3();
WScript.Echo(typeof(ret));
WScript.Echo(ret.toString());

WScript.Echo("Func4");
ret = m.func4();
WScript.Echo(typeof(ret));
WScript.Echo(ret.toString());

WScript.Echo("Func5");
ret = m.func5();
WScript.Echo(typeof(ret));
WScript.Echo(ret.toString());

WScript.Echo("Func6");
ret = m.func6();
WScript.Echo(typeof(ret));
WScript.Echo(ret.toString());
